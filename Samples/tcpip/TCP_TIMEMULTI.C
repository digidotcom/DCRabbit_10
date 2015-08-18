/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/*****************************************************

	tcp_timemulti.c

	  ** NOTE **

	  This sample will only work on a board with multiple
	  network interfaces i.e. 10 base T Ethernet plus a
	  serial PPP interface.  (PPP requires the optional PPP
	  module to be installed).

 Demonstrate how to talk to one or more "Time Protocol" servers (see
 RFC868 at http://www.faqs.org/rfcs/rfc868.html) in order for the
 Rabbit to get a reliable wall-clock time from the network.

 This demo is same as tcp_time.c, except that multiple
 interfaces may be specified.

 To verify that this demo is working, from a host connected to
 the network issue

   telnet xxx.xxx.xxx.xxx 13

 where xxx.xxx.xxx.xxx is any of the IP addresses printed in
 the interface table when the demo starts (whose interface is
 marked as being "up").  The telnet command
 should terminate after printing the time of day.

 ******************************************************/


/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG		4		// 4 = multiple interfaces

/*
 * Configurable items...
 */

// Set timezone offset and name
//#define TIMEZONE     (-7)	// Hours difference from UTC (whole hours only)
//#define TZNAME			"PDT"	// Standard name for above timezone

#define CLOSE_ENOUGH		60		// Discard servers who say the time is more than
										// this many seconds different from the others.
										// This could be reduced to 2 if servers are expected
										// to be tied to national standards.

//#define NO_TIMESERVER			// Disable time server code.


// Hostnames (or dotted quad IP addresses) of each time server to query.
// You can "weight" some servers as being more reliable by mentioning the
// same server in more than one entry.
// NOTE: the following addresses were obtained from hosts that were
// denoted as "open access" (http://www.eecis.udel.edu/~mills/ntp/servers.htm)
// to NTP.  We don't use NTP, but we don't think they'd mind if we used the
// old time service.  Please don't incorporate these addresses in any product
// without prior approval of the host adinistrator!
// The timestamps from these servers will be traceable to national standards.


// NOTE: the following list of servers is known (as of 2007) to respond to
// TIME/DAYTIME protocol queries.  However, since these protocols are being replaced
// with NTP, there is no guarantee of these services continuing to run.
// Please refer to the new sample SNTP_TIME.C for a more up-to-date way of
// obtaining network time.
char * const server_hosts[] =
{
   "time-b.nist.gov"
  ,"nist1.aol-ca.truetime.com"
  ,"timelord.uregina.ca"
};

#define NSERVERS		(sizeof(server_hosts)/sizeof(server_hosts[0]))

/********************************
 * End of configuration section *
 ********************************/


// If TIMEZONE not defined, default to "UTC" (0h).
#ifndef TIMEZONE
	#define TIMEZONE		0
	#define TZNAME			"UTC"
#endif

// Timezone was defined, but TZNAME was not
#ifndef TZNAME
	#define TZNAME			""
#endif

#memmap xmem
#use "dcrtcp.lib"



#define TIME_PORT			37		// Binary time service (machine-readable)
#define DAYTIME_PORT		13		// ASCII time service (human-readable)

// Returned timestamps from servers.  These are stored as the number of seconds
// since Jan 1, 1900 UT.  They are later converted to "Rabbit epoch" (1980).
// There were lots of furry rabbits at that time but, strangely, no silicon ones.
// Sadly, RFC868 does not allow determination of timezone, so everything is in
// terms of UTC a.k.a. "GMT".
longword timestamps[NSERVERS];

// Flag to indicate whether valid result obtained from corresponding server.
int use_this[NSERVERS];

// Number of valid results remaining.
int nservers;

// Base "reference" epoch (local ms) for when we started our timings.
longword base_stamp;

// Base epoch from SEC_TIMER corresponding to the above
longword base_tick;

// Amount to add to SEC_TIMER to make the result equal to the average time
// derived from the servers.
longword skew;

// Seconds between RFC868 epoch (1900) and Rabbit epoch (1980)
#define EPOCH_DIFF	2524521600UL


// UDP socket we use to query the servers
udp_Socket usock;
#define USOCK_BUFSIZE 64
long usock_buf;


char * format_timestamp(longword stamp, int local)
{
	static char b[40];
	char *p;
	struct tm daytime;

	if (local)
		mktm(&daytime, stamp + TIMEZONE*3600L);
	else
		mktm(&daytime, stamp);

	// print base time using strftime from <time.h>
	p = b + strftime( b, sizeof b, "%a, %b %d %Y %H:%M:%S", &daytime);

	// append timezone name and hour offset
	sprintf(p, " %s (%dh)",
							local ? TZNAME : "UTC",
							local ? TIMEZONE : 0
							);
	return b;
}


void query_the_servers(void)
{
	auto int rc;
	auto udp_Socket * socket;
	auto int i;
	auto longword rhost;
	auto word rport;
	auto int nrx;
	auto longword rtt_stamp;
	auto longword stamp;
	auto longword timer;
	auto int rt_count;

	#GLOBAL_INIT { usock_buf = xalloc(USOCK_BUFSIZE); }
	nservers = 0;
	socket = NULL;

	// Remember starting epoch for our queries
	base_stamp = MS_TIMER;
	base_tick = SEC_TIMER;

	for (i = 0; i < NSERVERS; i++) {

		use_this[i] = 0;

		// Close the UDP socket if it was in use by previous iteration of this loop.
		if (socket) {
			sock_close(socket);
			socket = NULL;
		}

		// Resolve the hostname (this blocks)
		rhost = resolve(server_hosts[i]);
		if (!rhost) {
			printf("Could not resolve %s.\n", server_hosts[i]);
			continue;	// Try next server
		}

		// Open UDP channel.
		rc = udp_extopen(&usock, IF_ANY, 0, rhost, TIME_PORT, NULL, usock_buf, USOCK_BUFSIZE);
		if (!rc) {
			printf("Could not open UDP socket to %s.\n", server_hosts[i]);
			sock_perror(&usock, NULL);
			exit(21);
		}

		socket = &usock;
		udp_set_dicmp(socket);	// Set to receive ICMP messages in-stream.

		// Wait for ARP to resolve the router address.  Wait up to 2 seconds.
		timer = _SET_TIMEOUT(2000);
		while (!sock_resolved(socket)) {
			tcp_tick(NULL);
			if (chk_timeout(timer))
				break;
		}
		if (!socket) {
			printf("Host %s not reachable: no router?\n", server_hosts[i]);
			continue;
		}

		rt_count = 0;

	retry:
		// All OK.  Send empty datagram to server after recording current
		// (local) timestamp.  This loops until the return is non-zero.

		rtt_stamp = MS_TIMER;	// Start time of issuing request
		rc = udp_send(socket, NULL, 0);
		if (rc < 0) {
			printf("Failed to send empty datagram to %s.\n", server_hosts[i]);
			continue;
		}

		for (;;) {
			tcp_tick(NULL);
			rc = udp_recvfrom(socket, (char *)&stamp, sizeof(stamp), &rhost, &rport);

			if (rc < -1) {
				printf("Failed to get response from %s:\n", server_hosts[i]);
				sock_perror(socket, NULL);
				if (rc == -3)
					printf("...probably not a TIME protocol server.\n");
				break;
			}
			if (rc >= 4)
				// Exit wait loop if got response.
				break;

			// 4 second timeout.  Retry up to 2 times, else ignore this server.  This raises
			// the possibility that we may get a response from a previous packet, thus
			// warping our perception of time.  We ignore this wrinkle in our demo, since
			// the protocol does not provide for correction of this other than waiting about
			// 4 minutes for stray packets to die in the network.
			if ((long)(MS_TIMER - rtt_stamp) > 4000) {
				rt_count++;
				if (rt_count < 2)
					goto retry;
				else {
					printf("Server %s timed out.\n", server_hosts[i]);
					sock_perror(socket, NULL);
					rc = -1;
					break;
				}
			}
		}

		if (rc < 0)
			continue;

		// Correct for round-trip time in milliseconds.
		rtt_stamp += MS_TIMER - rtt_stamp >> 1;

		//printf("Got %d bytes from host %s [%lX] port %u\n", rc, server_hosts[i], rhost, rport);

		stamp = intel(stamp);	// Convert to local ordering.

		// Add 1/2 round-trip time and correction back to common base epoch
		stamp -= (rtt_stamp - base_stamp)/ 1000;

		timestamps[i] = stamp;
		use_this[i] = 1;			// Mark as OK so far
		printf("Server %s says time is %s\n", server_hosts[i], format_timestamp(stamp - EPOCH_DIFF, 1));
		nservers++;
	}

	if (socket)
		udp_close(socket);

}


void discard_outliers(void)
{
	auto int i, j, k;
	auto int minservers;
	auto float diff;
	auto float wdiff;
	auto float mdiff;

	if (nservers >= 3)
		minservers = nservers + 1 >> 1;	// Half (rounded up) of the successful servers
	else
		minservers = nservers;

	while (nservers > minservers) {
		// Throw away the worst outliers first, until minservers remain.  The
		// "worst outlier" is the one whose sum-of-squares distance from the others
		// is greatest.
		mdiff = 0.0;
		for (i = 0; i < NSERVERS; i++)
			if (!use_this[i])
				continue;
			else {
				wdiff = 0.0;
				for (j = 0; j < NSERVERS; j++)
					if (!use_this[j] || i == j)
						continue;
					else {
						diff = (long)(timestamps[j] - timestamps[i]);
						wdiff += diff * diff;
					}
				if (wdiff > mdiff) {
					mdiff = wdiff;
					k = i;
				}
			}
		if (sqrt(mdiff) / nservers < CLOSE_ENOUGH)
			// Don't discard any more if within about 60 seconds of each other
			break;
		use_this[k] = 0;
		printf("Discarding server %d\n", k);
		nservers--;
	}
}

void compute_offset(void)
{
	auto int i;
	auto long stamp;
	auto long avg;


	// Average the timestamps of the remaining servers.  This can't be
	// done the obvious way, to avoid arithmetic overflow.

	for (i = 0; i < NSERVERS; i++)
		if (use_this[i]) {
			avg = timestamps[i];
			break;
		}

	stamp = 0;
	for (i++; i < NSERVERS; i++)
		if (use_this[i])
			stamp += timestamps[i] - avg;

	avg += stamp / nservers;

	// Convert to "Rabbit epoch" by subtracting the number of seconds between
	// 1900 and 1980.
	avg -= EPOCH_DIFF;

	// This is the amount to add to SEC_TIMER to get the "correct" time.
	skew = avg - base_tick;
}

#ifndef NO_TIMESERVER

// UDP socket for the TIME and DAYTIME service
udp_Socket usock_t;
udp_Socket usock_d;
long usock_t_buf;
long usock_d_buf;

// TCP sockets for the same 2 services
tcp_Socket tsock_t;
tcp_Socket tsock_d;
#define TSOCK_BUFSIZE	128
long tsock_t_buf;
long tsock_d_buf;


void ut_tick(void)
{
	auto longword rhost;
	auto word rport;
	auto longword stamp;

	if (udp_recvfrom(&usock_t, NULL, 0, &rhost, &rport) >= 0) {
		stamp = intel(SEC_TIMER + skew);
		udp_sendto(&usock_t, (char *)&stamp, sizeof(stamp), rhost, rport);
	}
}

void udt_tick(void)
{
	auto longword rhost;
	auto word rport;
	auto char b[64];

	if (udp_recvfrom(&usock_d, NULL, 0, &rhost, &rport) >= 0) {
		sprintf(b, "%s\r\n", format_timestamp(SEC_TIMER + skew, 1));
		udp_sendto(&usock_d, b, strlen(b), rhost, rport);
	}
}

#define TSTATE_CLOSED		0
#define TSTATE_LISTENING	1
#define TSTATE_CLOSING		2

void tt_tick(void)
{
	static int state;
	auto char b[64];
	auto longword stamp;

	#GLOBAL_INIT { state = TSTATE_CLOSED; tsock_d_buf = xalloc(8); }

	switch (state) {
		case TSTATE_CLOSED:
			tcp_extlisten(&tsock_t, IF_ANY, TIME_PORT, 0, 0, NULL, 0, tsock_d_buf, 8);
			state = TSTATE_LISTENING;
			// fall through
		case TSTATE_LISTENING:
			if (sock_established(&tsock_t)) {
				stamp = intel(SEC_TIMER + skew);
				sock_fastwrite(&tsock_t, (char *)&stamp, sizeof(stamp));	// Guaranteed to fit in tx buffer
				sock_close(&tsock_t);
				state = TSTATE_CLOSING;
			}
			break;
		case TSTATE_CLOSING:
			sock_fastread(&tsock_d, b, sizeof(b));	// Discard any junk sent by client
			if (!tcp_tick(&tsock_t))
				state = TSTATE_CLOSED;
			break;
	}


}

void tdt_tick(void)
{
	static int state;
	auto char b[64];
	auto int rc;

	#GLOBAL_INIT { state = TSTATE_CLOSED; tsock_d_buf = xalloc(TSOCK_BUFSIZE); }

	switch (state) {
		case TSTATE_CLOSED:
			tcp_extlisten(&tsock_d, IF_ANY, DAYTIME_PORT, 0, 0, NULL, 0, tsock_d_buf, TSOCK_BUFSIZE);
			state = TSTATE_LISTENING;
			// fall through
		case TSTATE_LISTENING:
			if (sock_established(&tsock_d)) {
				sprintf(b, "%s\r\n", format_timestamp(SEC_TIMER + skew, 1));
				sock_fastwrite(&tsock_d, b, strlen(b));	// Guaranteed to fit in tx buffer
				sock_close(&tsock_d);
				state = TSTATE_CLOSING;
			}
			break;
		case TSTATE_CLOSING:
			sock_fastread(&tsock_d, b, sizeof(b));	// Discard any junk sent by client
			if (!tcp_tick(&tsock_d))
				state = TSTATE_CLOSED;
			break;
	}
}

void become_server(void)
{
	// Become a four-port server, for both TCP and UDP versions of Time (RFC868)
	// and Daytime (RFC867).  The latter is an ascii (human readable) version of
	// Time.

	auto char buf[64];
	auto int rc;
	auto word rport;
	auto longword rhost;
	auto long stamp;

	#GLOBAL_INIT { usock_t_buf = xalloc(USOCK_BUFSIZE); usock_d_buf = xalloc(USOCK_BUFSIZE); }

	// Allow TCP SYN queueing.
	tcp_reserveport(TIME_PORT);
	tcp_reserveport(DAYTIME_PORT);

	// Open UDP sockets in passive mode
	udp_extopen(&usock_t, IF_ANY, TIME_PORT, 0xFFFFFFFF, 0, NULL, usock_t_buf, USOCK_BUFSIZE);
	udp_extopen(&usock_d, IF_ANY, DAYTIME_PORT, 0xFFFFFFFF, 0, NULL, usock_d_buf, USOCK_BUFSIZE);

	for (;;) {
		tcp_tick(NULL);
		// Tick all servers over
		ut_tick();
		udt_tick();
		tt_tick();
		tdt_tick();
	}


}
#endif

get_the_time(void)
{
	// Query the Time protocol port on each server.
	query_the_servers();

	if (!nservers) {
		printf("No servers could be found - using local clock\n");
		skew = 0;
	}
	else {
		// Now that we have responses from hopefully all of the servers, determine the
		// set that we are going to average.
		discard_outliers();

		// Compute the offset that we need to add to our SEC_TIMER to get the
		// correct averaged time.
		compute_offset();
	}

}

int main()
{
	int rc;
	int i;
	int ifaces_up;

	sock_init();
	// Wait for all interfaces to come up
	ifaces_up = 0;
	while (!ifaces_up) {
		tcp_tick(NULL);
		ifaces_up = 1;
	   for (i = 0; i < IF_MAX; i++) {
	      if (is_valid_iface(i)) {
	      	if (ifpending(i) == IF_COMING_UP) {
	      		ifaces_up = 0;
	      	}
	      }
	   }
	}

	ip_print_ifs();
	router_printall();

	// Find the time from the internet resources.
	get_the_time();

	printf("The time is: %s - RabbitTime!\n", format_timestamp(SEC_TIMER + skew, 1));

#ifndef NO_TIMESERVER
	// Now turn into a Time/Daytime protocol server.
	become_server();
#endif

	return 0;
}
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
 *
 *	Samples\TCPIP\sntp_time.c
 *
 * Demonstrate how to talk to one or more SNTP servers (see
 * RFC2030 at http://www.faqs.org/rfcs/rfc2030.html) in order for the
 * Rabbit to get a reliable wall-clock time from the network.
 *
 * This sample based on tcp_time.c, except that we use the more modern
 * and well supported SNTP protocol.  The results are much more accurate,
 * typically within a few 10's of ms.
 *
 * We use UDP to query to a number of SNTP servers.
 * The list of servers is specified in an initialized array.
 *
 * You might like to use this as a basis for adding a reliable wall-clock
 * time to an appliance which only has a crystal oscillator and, up until
 * now, no means for setting the actual date and time.
 *
 * Note that the RTC (Real Time Clock) is _not_ updated.  Instead,
 * variables called 'skew' and 'skew_coarse' are set such that
 *  (SEC_TIMER + skew_coarse + (skew/1024))
 * is the correct time (i.e. number of seconds since 1980).  The 10 LSBs of
 * 'skew' indicate the fraction of a second, to a resolution slightly better
 * than 1ms.
 *
 * After the initial setting is obtained from get_the_time(), the clock
 * will run at the local crystal oscillator rate, which may be +/- 100ppm
 * from the correct rate.  If keeping the time accurate is important,
 * you will need to call get_the_time() every few hours to update the
 * skew variables.
 *
 * How it works: It depends on some RFC1305 NTP/SNTP servers being set up.  The
 * nominated server(s) are queried for their UTC time.  The result
 * from each server is adjusted for the network delay.
 *
 * If no servers can be found, the current real-time clock is used
 * (SEC_TIMER).  This is a fall-back in case the network is unavailable.
 *
 *
 ******************************************************/
#class auto

// Uncomment the following macro definition in order to set Rabbit's RTC (Real-
// Time Clock) reasonably accurately, to within one second of the weighted
// average of the time server results. The RTC will be set to UTC time.
//#define SET_RTC

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// Set timezone offset and name
//#define TIMEZONE     (-7)	// Hours difference from UTC (whole hours only)
//#define TZNAME			"PDT"	// Standard name for above timezone

// Hostnames (or dotted quad IP addresses) of each time server to query.
// The timestamps from these servers will be traceable to national standards.
// We use the "pool" service, which basically selects a publicly available NTP
// server on a round-robin basis.  We also have some fixed favorites...


char * const server_hosts[] =
{
	"time-b.nist.gov",
	"nist1.aol-ca.truetime.com",
	"0.pool.ntp.org",
	"1.pool.ntp.org",
	"2.pool.ntp.org",
	"pool.ntp.org",
};

#define NSERVERS (sizeof(server_hosts)/sizeof(server_hosts[0]))

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

// Format of an NTP timestamp.  Note that in the actual network packet, the
// two fields are reversed.  We store them least-significant first in order
// to be compatible with the Rabbit little-endian convention.
typedef struct {
	unsigned long	frac;				// Fraction of a second (units of 2**-32 sec)
	unsigned long	sec;				// Seconds since 0h 1900 UTC
} NTP_ts;

// Format of the NTP datagram
typedef struct {
	char	flags;
	#define NTP_LI		0xC0			// Mask for leap-second warning indicator
		#define NTP_LONGMINUTE	0x40		// Last minute of today (UTC) is 61 seconds
		#define NTP_SHORTMINUTE	0x80		// Last minute of today (UTC) is 59 seconds
		#define NTP_ALARM    	0xC0		// Alarm condition, clock not sync'd
	#define NTP_VN		0x38			// Mask for protocol version number
		#define NTP_VERSION_3	0x18		// Version 3
		#define NTP_VERSION_4	0x20		// Version 4 (latest as of 2007)
	#define NTP_MODE	0x07			// Message mode as follows:
		#define NTP_CLIENT		0x03		// Client mode (unicast request)
		#define NTP_SERVER		0x04		// Server mode (unicast response)
	char	stratum;				// Server stratum: 0 for unavailable,
									// 1 for primary reference, 2-15 for secondary.
	char	poll_interval;
	char	precision;			// Signed integer indicating precision of local
									// clock, as power of 2 seconds.  E.g. -30 for
									// nanosecond precision.
	long	root_delay;			// See RFC1305
	unsigned long	root_dispersion;	// Ditto
	char	reference_identifier[4];	// Identifies primary source: either an IP
									// address for 2nd-15th stratum servers, or an
									// identifying string for stratum 0,1 (e.g. GPS,
									// PPS, WWV, USNO etc.)  <-- this for V3, V4 is a bit
									// different.
	// The various NTP timestamps...
	NTP_ts	reference;		// Time at which local clock last set or corrected.
	NTP_ts	originate;		// Client timestamp when sending request
	NTP_ts	receive;			// Server timestamp when receiving request
	NTP_ts	transmit;		// Server timestamp when sending reply

	// There are additional fields, but they are optional and we don't use them.
} NTP;



#define NTP_PORT		123		// Well-known UDP port number for NTP service

// Returned results from servers.
NTP  timestamps[NSERVERS];

// Flag to indicate whether valid result obtained from corresponding server.
int use_this[NSERVERS];

// Number of valid results remaining.
int nservers;

// Derived network round-trip, and clock offsets, in "milliseconds" (1/1024 sec).
long	ms_roundtrip[NSERVERS];
long	ms_offset[NSERVERS];


// Amount to add to SEC_TIMER to make the result equal to the average time
// derived from the servers.  This is basically stored in units of 1/1024 sec.
// "skew_coarse" is normally zero, and "skew" can accept up to +/- 2**21
// seconds of correction (about +/- 24.27 days).  Any value approaching (or
// exceeding) this indicates that the RTC is probably not set.  In this
// case, "skew_coarse" is set to a value which, when added to SEC_TIMER,
// brings the RTC close to the correct value reported by the NTP servers.
longword skew;				// Fine adjust (1/1024 sec units)
longword skew_coarse;	// Coarse adjust (1 sec units)

// Seconds between RFC868 epoch (1900) and Rabbit epoch (1980)
#define EPOCH_DIFF	2524521600UL


// UDP socket we use to query the servers.  We do one at a time, so only need
// minimal buffering.
udp_Socket usock;
#define USOCK_BUFSIZE (sizeof(NTP)+256)
long usock_buf;

///////////////////////////////////////////////////////////////////////

#ifdef SET_RTC
// Routine to update the RTC with the de-skewed SNTP time.
void set_deskewed_rtc(NTP_ts ntpt, int local, long offset)
{
	longword stamp;
	longword frac;
	longword g, f;

	stamp = ntpt.sec;
	frac = ntpt.frac;

	// g,f is sign extended offset.
	g = offset < 0 ? 0xFFC00000uL : 0;
	f = offset << 22;
	g |= (offset >> 10) & 0x3FFFFFuL;

	if (frac + f < frac)
		// Carry in addition
		stamp += g + 1;
	else
		stamp += g;

	stamp -= EPOCH_DIFF;

	if (local)
		stamp += TIMEZONE * 3600L;

	write_rtc(stamp);
}
#endif

// Useful routine for formatting the time, including an offset, which would normally
// be provided by the 'skew' variable once the NTP server(s) had responded.
char * format_timestamp(NTP_ts ntpt, int local, long offset)
{
	static char b[50];
	struct tm daytime;
	char *p;
	longword stamp;
	longword frac;
	longword g, f;

	stamp = ntpt.sec;
	frac = ntpt.frac;

	// g,f is sign extended offset.
	g = offset < 0 ? 0xFFC00000uL : 0;
	f = offset << 22;
	g |= (offset >> 10) & 0x3FFFFFuL;

	if (frac + f < frac)
		// Carry in addition
		stamp += g + 1;
	else
		stamp += g;
	frac += f;
	stamp -= EPOCH_DIFF;

	if (local)
		mktm(&daytime, stamp + TIMEZONE*3600L);
	else
		mktm(&daytime, stamp);

	// print base time using strftime from <time.h>
	p = b + strftime( b, sizeof b, "%a, %b %d %Y %H:%M:%S", &daytime);

	// append fractional seconds, timezone name and hour offset
	sprintf(p, ".%06lu %s (%dh)",
							(longword)(frac * 1000000.0/4924967296.0),
							local ? TZNAME : "UTC",
							local ? TIMEZONE : 0
							);
	return b;
}

// Get local time as an NTP timestamp (UTC)
NTP_ts get_local()
{
	static word uniq;
	longword sec, ms;
	NTP_ts ret;
	#GLOBAL_INIT { uniq = 0; }

	// We use a unique number on the (insignificant) LSBs of the timestamp.
	// This helps us match replies with requests.
	++uniq;

	// Need to disable interrupts, since we get consistent view of two values
	// which are updated asynchronously.
	asm ipset 1;

	sec = SEC_TIMER;
	ms = TICK_TIMER;	// This is really 1/1024 sec, not ms, but that's good.

	asm ipres;

	// Add in any coarse correction in order to bring the difference between
	// this and the correct time to less than 24 days.
	sec += skew_coarse;

	ret.sec = sec + EPOCH_DIFF;	// Change to 1900 epoch
	ret.frac = (ms & 1023) << 22 | uniq;	// Shift up to fraction, and OR in "uniquifier"

	return ret;
}


// Return difference between NTP timestamps (t2 - t1), in units of 1/1024 sec
long timediff_ms(NTP_ts * t2, NTP_ts * t1)
{
	static int warned;
	longword g, f;
	#GLOBAL_INIT { warned = 0; }
	g = t2->sec - t1->sec;
	f = t2->frac - t1->frac;
	if (t2->frac < t1->frac)
		--g;
	if ((long)g >= 0x1FFF00 || (long)g < -0x1FFF00) {
		if (!warned) {
			// This should not happen even if the RTC is not ever set, but may occur if
			// the skew_coarse variable is tampered with, or if very old timestams are
			// compared with new ones.
	      printf("\n**********************************************************************\n");
	      printf("Local RTC is not approximately correct, and could not be compensated.\n");
	      printf("Use samples\\rtclock\\setrtckb.c to set the RTC to within +/-24 days\n");
	      printf("of the correct UTC time.\n");
	      printf("**********************************************************************\n\n");
	      warned = 1;
		}
	}
	return g << 10 | f >> 22;
}


// Convert from network packet to host format
void ntoh_NTP_ts(NTP_ts * n)
{
	// Order of fields is also swapped!
	longword temp;
	temp = ntohl(n->sec);
	n->sec = ntohl(n->frac);
	n->frac = temp;
}
void ntoh_NTP(NTP * n)
{
	n->root_delay = ntohl(n->root_delay);
	n->root_dispersion = ntohl(n->root_dispersion);
	ntoh_NTP_ts(&n->reference);
	ntoh_NTP_ts(&n->originate);
	ntoh_NTP_ts(&n->receive);
	ntoh_NTP_ts(&n->transmit);
}

// Convert from host format to network packet
void hton_NTP_ts(NTP_ts * n)
{
	// Order of fields is also swapped!
	longword temp;
	temp = htonl(n->sec);
	n->sec = htonl(n->frac);
	n->frac = temp;
}
void hton_NTP(NTP * n)
{
	n->root_delay = htonl(n->root_delay);
	hton_NTP_ts(&n->reference);
	hton_NTP_ts(&n->originate);
	hton_NTP_ts(&n->receive);
	hton_NTP_ts(&n->transmit);
}


void query_the_servers(void)
{
	NTP  n;		// Query
	NTP  rep;	// Response
	NTP_ts T4;	// Time of arrival of most recent response
	NTP_ts T1;	// Time of sending
	auto char buf[16];
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
	auto int coarse_set;

	#GLOBAL_INIT { usock_buf = xalloc(USOCK_BUFSIZE); }
	nservers = 0;
	socket = NULL;
	coarse_set = 0;


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
		rc = udp_extopen(&usock, IF_DEFAULT, NTP_PORT, rhost, NTP_PORT, NULL, usock_buf, USOCK_BUFSIZE);
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
		if (!sock_resolved(socket)) {
			printf("Host %s not resolved\n", server_hosts[i]);
			continue;
		}

		rt_count = 0;

	retry:

		// Set up our request datagram
		memset(&n, 0, sizeof(n));
		n.flags = NTP_VERSION_3 | NTP_CLIENT;
		n.transmit = get_local();
		T1 = n.transmit;
		hton_NTP(&n);				// Convert to network order

		rc = udp_send(socket, &n, sizeof(n));
		if (rc < 0) {
			printf("Failed to send request to %s.\n", server_hosts[i]);
			continue;
		}

		rtt_stamp = _SET_TIMEOUT(4000);

		for (;;) {
			tcp_tick(NULL);
			rc = udp_recvfrom(socket, (char *)&rep, sizeof(rep), &rhost, &rport);

			if (rc < -1) {
				printf("Failed to get response from %s:\n", server_hosts[i]);
				sock_perror(socket, NULL);
				if (rc == -3)
					printf("...probably not an SNTP protocol server.\n");
				break;
			}
			if (rc >= (int)sizeof(NTP)) {
				T4 = get_local();		// Remember time of arrival
				// Exit wait loop if got response.
				break;
			}

			// 4 second timeout.  Retry up to 2 times, else ignore this server.
			if (_CHK_TIMEOUT(rtt_stamp)) {
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

		ntoh_NTP(&rep);

		// Quick check to see that response is to the last packet we sent.  The server
		// will reflect back in the 'originate' timestamp the stamp we set in 'transmit'.
		if (rep.originate.frac == T1.frac) {
			// Looks good...
	      timestamps[i] = rep;
	      use_this[i] = 1;        // Mark as OK so far

	      // Set coarse correction if not already done
	      if (!coarse_set) {
	      	coarse_set = 1;
	      	skew_coarse = rep.receive.sec - T1.sec;
	      	// Correct the stamps we saved before
	      	T1.sec += skew_coarse;
	      	T4.sec += skew_coarse;
	      }

			// Compute round-trip delay and local clock offset
			ms_roundtrip[i] = timediff_ms(&T4, &T1) - timediff_ms(&rep.receive, &rep.transmit);
			ms_offset[i] = (timediff_ms(&rep.receive, &T1) + timediff_ms(&rep.transmit, &T4)) >> 1;

	      printf("Server %s:\n", server_hosts[i]);
	      printf("   Time is    %s\n", format_timestamp(get_local(), 1, ms_offset[i]));
	      printf("   Round-trip %ldms\n", ms_roundtrip[i]);
	      printf("   Offset     %ldms\n", ms_offset[i]);

	      // Just for kicks, print some other data provided by the server.  We don't need
	      // this for most SNTP applications.
			printf("   Server stratum     %u\n", rep.stratum);
			rep.reference.frac = 0;	// Null term string (OK, since done with following field)
			printf("   Server ref ident   %s\n",
				rep.stratum > 1 ? inet_ntoa(buf, *(longword *)rep.reference_identifier) :
                              rep.reference_identifier);
         printf("   Server root delay       %10.6fs\n",
         	rep.root_delay * 1.0/65536);
         printf("   Server root dispersion  %10.6fs\n",
         	rep.root_dispersion * 1.0/65536);


	      nservers++;
		}
		else {
			printf("Dropping reply from server %s, not response to last query.\n", server_hosts[i]);
		}
	}

	if (socket)
		udp_close(socket);

}



get_the_time(void)
{
	word i;
	float q;
	float s;
	float r;
	#GLOBAL_INIT { skew = skew_coarse = 0; }

	// Query the Time protocol port on each server.
	query_the_servers();

	if (!nservers) {
		printf("No servers could be found - using local clock\n");
	}
	else {
		// Set fine skew to average of all response offsets
		// We weight the averages by the reciprocal of the RTT
		for (i = 0, q = 0.0, s = 0.0; i < NSERVERS; ++i)
			if (use_this[i]) {
				r = ms_roundtrip[i] >= 1 ? 1.0/ms_roundtrip[i] : 1.0;
				q += ms_offset[i] * r;
				s += r;
			}
		skew = (long)(q / s);
	}

	printf("Using weighted average offset of %ldms\n", skew);
}

int main(void)
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	printf("\nUncorrected RTC time is %s\n\n", format_timestamp(get_local(), 1, 0));

	// Find the time from the internet resources.
	get_the_time();

	printf("\n\n");

#ifdef SET_RTC
	set_deskewed_rtc(get_local(), 1, 0);
	printf("The RTC has been updated. To see the new 'uncorrected' RTC time, r");
	printf("estart this\nprogram (e.g. by pressing Ctrl+Q followed by Ctrl+F2 ");
	printf("and then F9).\n\n");
#endif

	for (;;)
		printf("\rCorrected RTC time is %s   ", format_timestamp(get_local(), 1, skew));
}
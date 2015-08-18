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
/**********************************************************************
 *		samples\tcpip\multi_echo.c
 *
 * 	This program demonstrates use of multiple interfaces (or indeed
 *    a single interface).
 *
 *    When running, you can:
 *      . ping any of the active interfaces
 *      . connect to a TCP socket on any interface, which will echo
 *        back any data sent to it.
 *      . send UDP packets to any interface, which will be echoed.
 *    Up to 10 simultaneous connections can be established to the
 *    TCP sockets, which all listen on port ECHO_PORT (default 7).
 *    The UDP socket also listens on port ECHO_PORT.
 *
 *    If connected via the programming cable, you can also press keys
 *    in the stdio window to perform various tests:
 *      . 'i', 'r' and 'c': print status information
 *      . '0' thru '9' toggle specified interface status (up/down)
 *      . 'p0' thru 'p9' ping the default router for the specified i/f
 *      . 'a' starts a dialog asking for a host/port to actively
 *        open a connection to.  The host can be a dotted quad or domain
 *        name (which will use the nameserver in the latter case, if
 *        defined).  To use this, you have to set up a listening socket
 *        on the specified host/port.  This becomes an echo connection
 *        just like the listening sockets.
 *    When a TCP connection is established, you can terminate it by
 *    quitting the host application, or you can send a single line
 *    starting with the characters 'quit', which will cause the socket
 *    to be closed by this host.  The string should be sent after a
 *    1 second or so break.
 *    The actively-opened connection can also be terminated by issuing
 *    another 'a' command.
 *
 **********************************************************************/
#class auto


// Definitions for this demo:
#define ECHO_PORT					7
#define NUM_LISTEN_SOCKS		4


/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1		// Specify the interface configuration to use


// Overrides for DCRTCP configuration:
#define USE_IF_CALLBACK		// Call the interface status callback
#define TCP_DATAHANDLER		// Pull in TCP socket data handler calls

/*
//#define DCRTCP_DEBUG
#define DCRTCP_VERBOSE
#define PPPLINK_DEBUG
#define PPPLINK_VERBOSE
#define PPPOE_DEBUG
#define PPPOE_VERBOSE
*/

#define NUM_SOCKS (NUM_LISTEN_SOCKS+1)
#define MAX_TCP_SOCKET_BUFFERS NUM_SOCKS
#define MAX_UDP_SOCKET_BUFFERS 1


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"


#define STATE_CLOSED		0		// Initial state, must be zero.
#define STATE_LISTEN		1
#define STATE_ESTAB		2
#define STATE_CLOSING	3
#define STATE_ACTOPEN	-2		// must be less than 0
#define STATE_DONT_USE	-1		// must be less than 0

tcp_Socket s[NUM_SOCKS];
tcp_Socket * asock;
char buf[100];
int state[NUM_SOCKS];
int * astate;
int socknum;
int rb;
word i;
longword oldip[IF_MAX];
longword ip;
longword seq, ping_id, ping_ip, act_ip;
char kb;
word aport;
udp_Socket usock;


/* Callback functions... */
int echo_handler(int event, udp_Socket * s, ll_Gather * g, _udp_datagram_info * udi)
{
	// This is explained in "samples\udp\udp_echo_dh.c"
	if (event != UDP_DH_INDATA)
		return 1;
	udp_sendto(s, g->data2, g->len2, udi->remip, udi->remport);
	return 1;
}

#ifdef TCP_DATAHANDLER
int t_handler(int event, tcp_Socket * s, ll_Gather * g, void * unused)
{
	printf("=== TCP handler: event = %d ===\n", event);
	return 0;
}
#else
	#define t_handler NULL
#endif

void updowncb(int iface, int up)
{
	printf("Callback: i/f %d is now %s\n", iface, up ? "up" : "down");
	ip_print_ifs();
}


void usage()
{
	printf("============================\n\n");
	printf("When running, press\n");
	printf("  ?     - reprint this help text\n");
	printf("  i     - print interface status\n");
	printf("  r     - print router status\n");
	printf("  c     - print ARP cache status\n");
	printf("  0-9   - toggle specified interface number up/down\n");
	printf("  p then 0-9   - ping router on specified interface number\n");
	printf("  a     - active open\n");
}

int main()
{
	auto int if_status;

	printf("Multiple Interface Echo Test\n");
	usage();
	printf("\nPress any key to proceed.\n");
	while (!kbhit());
	getchar();
	printf("Initializing TCP/IP stack...\n");

	for (i = 0; i < IF_MAX; i++)
		ifconfig(i, IFG_IPADDR, oldip+i,
#ifdef USE_IF_CALLBACK
					IFS_IF_CALLBACK, updowncb,
#endif
					IFS_END);

	sock_init();

	// Wait for the interface to come up
		while (IF_COMING_UP == (if_status = ifpending(IF_DEFAULT)) ||
		       IF_COMING_DOWN == if_status)
	{
		tcp_tick(NULL);
	}

	printf("...done.\n");

	memset(state, 0, sizeof(state));
	socknum = -1;
	seq = 1;
	ping_ip = 0;
	act_ip = 0;
	asock = s + (NUM_SOCKS - 1);
	astate = state + (NUM_SOCKS - 1);
	*astate = STATE_DONT_USE;

	if(!udp_extopen(&usock, IF_ANY, ECHO_PORT, -1L, 0, echo_handler, 0, 0)) {
		printf("udp_extopen failed!\n");
		exit(0);
	}


	for (;;) {
		socknum++;
		if (socknum == NUM_SOCKS)
			socknum = 0;

		tcp_tick(NULL);

		if (state[socknum] > STATE_CLOSED &&
		    !sock_alive(s+socknum)) {
		   if (socknum == NUM_LISTEN_SOCKS) {
		   	printf("Active socket closed\n");
				state[socknum] = STATE_DONT_USE;
		   }
		   else {
				printf("Socket %d closed\n", socknum);
				state[socknum] = STATE_CLOSED;
			}
			sock_perror(s+socknum, NULL);
		}

		for (i = 0; i < IF_MAX; i++) {
			ifconfig(i, IFG_IPADDR, &ip, IFS_END);
			if (oldip[i] != ip) {
				printf("IPaddr on i/f %d changed from %08lx to %08lx\n",
					i, oldip[i], ip);
				oldip[i] = ip;
				ifconfig(i, IFS_ICMP_CONFIG_RESET, IFS_END);
			}
		}

		if (kbhit()) {
			kb = getchar();
			if (kb == '?')
				usage();
			else if (kb == 'i')
				ip_print_ifs();
			else if (kb == 'r')
				router_printall();
			else if (kb == 'c')
				arpcache_printall();
			else if (kb == 'p') {
				printf("Press an interface number [TCP/IP now blocked]\n");
				while (!kbhit());
				kb = getchar();
				if (isdigit(kb)) {
					ifconfig(kb - '0', IFG_ROUTER_DEFAULT, &ping_ip, IFS_END);
					if (ping_ip) {
						printf("Pinging router %08lX...\n", ping_ip);
						_send_ping(ping_ip, seq++, 1, IPTOS_DEFAULT, &ping_id);
					}
					else
						printf("No router for interface %d\n", kb - '0');
				}
				else
					printf("No interface selected.\n");
			}
			else if (kb == 'a') {
				if (act_ip && *astate != STATE_DONT_USE) {
					printf("Closing active connection to %s...\n", inet_ntoa(buf, act_ip));
					sock_close(asock);
					while (tcp_tick(asock));
				}
				*astate = STATE_DONT_USE;
				printf("Enter a host name or IP address [TCP/IP now blocked]\n");
				gets(buf);
				printf("Resolving...\n");
				act_ip = resolve(buf);
				if (act_ip) {
					printf("Enter a port number to connect to (0-65535)\n");
					gets(buf);
					aport = (word)atol(buf);
					printf("Opening to %s:%u...\n", inet_ntoa(buf, act_ip), aport);
					*astate = STATE_ACTOPEN;
				}
				else
					printf("Could not resolve %s to IP address.\n", buf);

			}
			else if (isdigit(kb)) {
				// Toggle interface status
				kb -= '0';
	  			if (!(1u<<kb & IF_SET)) {
            	printf("Not a valid interface\n");
					continue;
            }
				if (ifstatus(kb)) {
					printf("Bringing interface %d down...\n", kb);
					ifconfig(kb, IFS_DOWN, IFS_END);
				}
				else {
					printf("Bringing interface %d up...\n", kb);
					ifconfig(kb, IFS_UP, IFS_END);
				}
			}
		}

		if (ping_ip) {
			if (_chk_ping(ping_ip , &ping_id) != 0xffffffffL) {
				printf("Got ping response from %08lX\n", ping_ip);
				ping_ip = 0;
			}
		}

		switch (state[socknum]) {
			case STATE_CLOSED:
				if (!tcp_extlisten(s + socknum, IF_ANY, ECHO_PORT, 0, 0, t_handler, 0, 0, 0)) {
					printf("Listen failed - socket %d\n", socknum);
					state[socknum] = STATE_DONT_USE;
					break;
				}
				printf("Listening on socket %d...\n", socknum);
				state[socknum] = STATE_LISTEN;
				break;
			case STATE_ACTOPEN:
				if (!tcp_extopen(s + socknum, IF_ANY, 0, act_ip, aport, t_handler, 0, 0)) {
					printf("Active open failed\n");
					state[socknum] = STATE_DONT_USE;
					break;
				}
				state[socknum] = STATE_LISTEN;
				break;
			case STATE_LISTEN:
				if (sock_established(s + socknum)) {
					printf("Connection %d estab.\n", socknum);
					state[socknum] = STATE_ESTAB;
				}
				break;
			case STATE_ESTAB:
				if ((rb = sock_fastread(s + socknum, buf, sizeof(buf))) > 0) {
					sock_fastwrite(s + socknum, buf, rb);
					if (!strncmp(buf, "quit", 4)) {
						printf("Peer on socket %d requests close\n", socknum);
						sock_close(s + socknum);
						state[socknum] = STATE_CLOSING;
					}
				}
				else if (rb < 0) {
					printf("Connection %d closed by peer.\n", socknum);
					sock_close(s + socknum);
					state[socknum] = STATE_CLOSING;
				}
				break;
			default:
				break;
		}
	}
}
/********************************************************************
   Samples\tcpip\multicast.c
   Rabbit Semiconductor, 2002

	This sample program sends and receives heartbeat packets via
	multicast UDP sockets.  Multiple sockets can be configured to
	operate at once.  IGMP can be enabled so that the multicast
	datagrams can be routed across subnets.

	Multicasting is useful when information needs to be transmitted
	to a group of other devices.  All of these devices can be
	listening on a single multicast group.  Note that multicasting
	will not work with TCP, since TCP is an end-to-end protocol.
	Multicasting is usually used in conjunction with UDP.

********************************************************************/
#class auto

/*
 * Defines the size of the buffer that is used to receive UDP
 * datagrams.
 */
#define BUFFER_SIZE	1024

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG		1

/*
 * Number of buffers allocated for UDP sockets, and hence the
 * total number of allowable UDP sockets.
 */
#define MAX_UDP_SOCKET_BUFFERS	5

/*
 * Enable the use of multicasting.  Note that this does not
 * enable IGMP, so that if multicast routing is required, this
 * will not suffice.
 */
#define USE_MULTICAST

/*
 * Enable the use of IGMP.  When multicast groups are joined
 * and left, reports will be issued that an IGMP router on the
 * local network can use.  If USE_IGMP is defined to 2, then
 * IGMPv2 will be used.  USE_IGMP can also be defined to be 1,
 * which means that IGMPv1 will be used.  Note however, that
 * the IGMPv2 client is compatible with both IGMPv1 and IGMPv2
 * routers, so there is not much reason to set USE_IGMP to
 * anything other than 2.
 *
 * This sample will work with the following line commented out,
 * but the multicast datagrams will not be routed across
 * subnets.
 */
#define USE_IGMP 2

#memmap xmem

#use "dcrtcp.lib"

/*
 * Define various structures and constants for this program
 */

// This is used to drive the state machine
typedef struct {
	int 			state;
	udp_Socket	sock;
	longword		timer;
	long			pkts_sent;
} MulticastState;

// Used for the configuration structure
typedef struct {
	char*	ip;			// IP address of the multicast group

	word	port;			// Port number (local and remote) of the
							// multicast socket

	byte	iface;		// Interface on which to start the socket

	int	interval;	// Interval in seconds between each
							// heartbeat packet we send (0 for no
							// heartbeat packets)
} MulticastConf;

// Configure the multicast groups to which we want to send
// datagrams.  See the above data structure for an
// explanation of the fields.
const MulticastConf conf[] =
{
	{ "224.1.2.3", 1234, IF_DEFAULT, 1},
	{ "224.1.2.4", 4321, IF_DEFAULT, 5},
	{ "224.1.2.4", 2345, IF_DEFAULT, 10}
};
#define  MULTICAST_COUNT	(sizeof(conf) / sizeof(MulticastConf))

#if MULTICAST_COUNT > MAX_UDP_SOCKET_BUFFERS
	#error "Too many multicast groups, need to increase MAX_UDP_SOCKET_BUFFERS."
#endif

// Defines the states of the state machine
enum {
	MCAST_OPENING,
	MCAST_RUNNING,
	MCAST_CLOSING,
	MCAST_CLOSED
};

// This holds the state for each multicast socket
MulticastState state[ MULTICAST_COUNT ];

// Initialize the state structure
void my_init(void)
{
	memset(state, 0, sizeof(state));
}

// Do processing on the multicast sockets
void my_tick(void)
{
	static char buffer[BUFFER_SIZE];
	auto int i;
	auto int retval;
	auto longword remip;
	auto word remport;
	auto char ipbuf[16];

	// Process each socket
	for (i = 0; i < MULTICAST_COUNT; i++) {
		switch (state[i].state) {
		case MCAST_OPENING:
			// Open the multicast socket
			retval = udp_extopen(&state[i].sock, conf[i].iface,
			                     conf[i].port, inet_addr(conf[i].ip),
			                     conf[i].port, NULL, 0, 0);
			if (retval == 0) {
				// Error opening the socket
				printf("Error opening socket %d!\n", i);
				state[i].state = MCAST_CLOSED;
			}
			else {
				// Set the interval timer, and advance to the next state
				state[i].timer = set_timeout(conf[i].interval);
				state[i].state = MCAST_RUNNING;
			}
			break;
		case MCAST_RUNNING:
			// Check for any incoming datagrams
			retval = udp_recvfrom(&state[i].sock, buffer, BUFFER_SIZE,
			                      &remip, &remport);
			if (retval < -1) {
				// Error reading from the socket
				printf("Error reading from socket %d!\n", i);
				state[i].state = MCAST_CLOSING;
			}
			else if (retval >= 0) {
				// NULL terminate the buffer and print a message
				buffer[retval] = '\0';
				inet_ntoa(ipbuf, remip);
				printf("Received on socket %d from %s port %u:\n", i, ipbuf,
				       remport);
				printf("%s\n", buffer);
			}

			// Check the timer--if it has expired, send another heartbeat packet
			if (chk_timeout(state[i].timer)) {
				state[i].timer = set_timeout(conf[i].interval);
				sprintf(buffer, "Heartbeat packet #%ld on socket %d\n",
				        state[i].pkts_sent, i);
				retval = udp_send(&state[i].sock, buffer, strlen(buffer));
				state[i].pkts_sent += 1;
				if (retval < 0) {
					printf("Error sending on socket %d!\n", i);
					state[i].state = MCAST_CLOSING;
				}
			}
			break;
		case MCAST_CLOSING:
			sock_close(&state[i].sock);
			state[i].state = MCAST_CLOSED;
			break;
		case MCAST_CLOSED:
			// Do nothing
			break;
		default:
			// There is no transition to a state other than the ones listed
			// above.  So, if we reach this "state", then something is
			// dreadfully wrong.
			printf("Error in state machine!  Not in a known state...\n");
			exit(1);
		}
	}
}

void main(void)
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	// Initialize our state structure
	my_init();

	for(;;) {
		// Drive the TCP/IP stack
		tcp_tick(NULL);
		// Drive our state machine
		my_tick();
	}
}
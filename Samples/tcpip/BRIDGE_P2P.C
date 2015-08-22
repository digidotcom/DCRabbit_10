/* BRIDGE_P2P.C

   Demonstrate Ethernet to Wifi bridge, using a "Point-to-Point" (P2P)
   link over the WiFi interface.  A bridge joins two Ethernet LANs into
   a single one, at least at the network and higher layers.

   Requires an RCM66XXW board (currently, the only Rabbit module which
   has both Ethernet and WiFi).  The interface board is also assumed, since
   the S1 switch and DS1 LED are used.  PE7 also is used to output a
   received signal strength indicator.

   This is a variation of BRIDGE.C, which should be examined for an overview
   of basic IP network bridging.  This sample assumes that two similar
   devices are running this program, and a dedicated point-to-point link is
   to be established between them.  BRIDGE.C does not assume P2P at the Wifi
   interface, and thus is suitable for bridging from this device to any other
   Wifi capable device (such as an access point).

   The main weakness with BRIDGE.C is that it does not implement any form of
   robust security when bridging via ad-hoc Wifi.  This is because there is
   no standard for supporting WPA or WPA-2 (RSN) over ad-hoc, and the Rabbit
   Wifi driver does not support it.

   This sample overcomes this security limitation, by negotiating a secure
   tunnel over the ad-hoc Wifi link.  Since this is not part of any standard,
   it requires two RCM66XXW devices running this code.  It will not
   interoperate with any access point, and is intended strictly for bridging
   two ethernet segments over a dedicated ad-hoc Wifi link.

   Another major difference between this sample and BRIDGE.C is that there
   is no IP address assignment on the Ethernet interface.  Thus, neither
   module take up any IP address on the Ethernet segments, requires DHCP,
   or handles ADDP configuration.  In fact, no such configuration is
   required at all (although see note below on router determination).

   When starting the ad-hoc link, the following steps are taken:

   1. Wifi interface brought up using linklocal procedure.  The resulting
      IP addresses are meaningful only to the local link and are not addressable
      from either ethernet segment.

   2. An announcement/discovery packet is broadcast (UDP port 2) over Wifi.
      On receipt of the AD packet, if it has our hardware address in the
      appropriate field, then that constitutes an acknowledgment (ACK), and the
      next step is taken.  Otherwise, each side continually retransmits
      the initial AD packet.  Each AD packet contains the linklocal IP
      address and hardware address of the sender (and the peer, if known),
      thus both sides have all necessary information to establish a TCP/IP
      socket connection.

   3. When each side has sent and received the ACK AD packet, it enters tunnel
      establishment state.  This sets up a TLS-secured socket to the peer.
      The peer with the highest numbered hardware address is the TLS server side
      (and listens on the socket), and the lower HWA is the client (and actively
      opens the socket).  The socket is opened using port 2 on both sides.
      No other sockets are used.

      Note: this sample uses pre-shared keys for TLS.  It is easy to change
      to using RSA certificates; see other TLS examples.  PSK is convenient
      since there is no requirement to use external tools to generate the
      certificates, and is faster at run-time.  The key itself needs to be
      set the same on both ends, of course; this requires modification of
      this code in the getPSK() function.

   4. When the socket is established and secured with TLS, it enters tunnel
      state.  In this state, IP and ARP packets which need to be bridged to
      the other side are sent via the tunnel.  There is no special wrapping
      of each packet, however any padding at the end of the packet is
      removed, so that the IP length field can be used to deduce the end
      of the current IP packet, and the ARP packets are all a known fixed
      length.  Note that the ethernet header is transmitted as well.

	Use of TLS security:
   ---------------------

   TLS requires key exchange i.e. both sides need to negotiate session
   keys securely.  This sample makes use of TLS-PSK (pre-shared key), although
   it is easy to use RSA certificates if desired.  The PSK is "burned in"
   in this sample; obviously a real application would need this to be
   configurable by the user (e.g. over the Ethernet or a serial cable).

   If you comment out the USE_TLS macro, then the TCP connection will be
   plaintext.  Use this only for experimentation (and ease of sniffing
   out any problems).

	Optimizing packet sizes:
   ------------------------

   Since the only traffic flowing over the ad-hoc Wifi link is a single
   TCP socket, the packet sizes over the air may be controlled by two
   mechanisms: the Wifi "fragmentation threshold" and the TCP MSS (Maximum
   Segment Size).  It is preferable to use the TCP MSS, since this performs
   packet sizing decisions at the highest layer, which is preferable.
   Unfortunately, MSS cannot be dynamically resized during a TCP connection,
   so an optimum value needs to be picked from the start.  This depends on
   the quality of the Wifi link.  If the expected quality is poor, then
   choose a smaller MSS.  512 bytes might be a good starting value, or
   choose up to 1460 for a very clean Wifi link.  The MTU on the Ethernet
   side is not a consideration; since TCP divides the data stream into
   segments automatically and transparently, there is no need to worry about
   MTU compatibility with the Ethernet side.

   TCP has a retransmission strategy, but Wifi itself has positive acknowledgment
   at its own layer.  Thus, there are two retry levels.  With the relatively
   high probability of requiring retransmissions, it is better to have this
   done at the Wifi level.  The TCP retry level should not be invoked very
   often, which is good for performance reasons.

   Since all IP packets are queued and sent over a single socket, the socket
   should be set with maximum sized receive and transmit buffers.  The
   Rabbit library supports up to 32767 bytes for each.

	Network interface configuration:
   --------------------------------

   As mentioned above, the Ethernet interface does not get a legitimate
   IP address assigned.  Thus, the Eth. interface does not come "up" in the
   normal sense.  Nevertheless, packets are still processed and transparently
   bridged as required.  The library was not designed with this use in mind,
   so in order to avoid bad library behavior, an IP address of 0.0.0.1 is
   assigned to Ethernet.  This is not valid "on the wire", but we never send
   it so it is harmless.

   The Wifi interface is assumed to be ad-hoc, and since we control both
   ends of the link -- by running the same hardware and program on each
   side -- we hard-code an SSID and channel.  Since the network configuration
   is basically hard-coded, we can avoid any configuration structure.
   We also avoid the scanning code, so this sample is much simpler than
   BRIDGE.C (except for the socket and TLS stuff).


	Router determination:
   ---------------------

   When sending packets out on the Ethernet interface, the correct destination
   hardware address must be used.  For local nodes (on the same Ethernet
   segment) this is done by looking up the cached ARP table.  Since we
   basically implement proxy ARP, we *should* be able to map the destination
   IP address to the correct destination hardware (Ethernet MAC) address.
   The only reason a local address may not be located is if the communicating
   hosts have used something other than ARP to determine each other's
   hardware addresses.  In this case, bridging will not work at all, and such
   hosts will probably not be able to communicate if they are on opposite sides
   of the bridge.

   If, on the other hand, the destination IP address is not local then we must
   forward the packet to a router.  This is problematic; we do not have
   a normal host IP address on the Ethernet side, and do not have any static
   configuration, so we do not initially know the router address.  To overcome
   this problem, we examine all DHCP acknowlegment packets which are received on
   the Ethernet side.  From these, we passively extract the router information.
   Because of this, we can only route successfully if the Ethernet side has
   an available DHCP server.  If all nodes are statically configured, then we
   never get to see any DHCP information, and thus do not discover the
   router.  If this is expected to be the case, then the program will need
   to be modified so that the IP address of the router can be manually
   configured.  It may also be possible to use ICMP router discovery and/or
   listen in to various router management packets like RIP or OSPF.

   For environments where a DHCP server may not be available, this sample
   uses the definitions FALLBACK_ROUTER and FALLBACK_NETMASK to determine the
   router/mask to use in the case that no DHCP packets are intercepted.
   These are hard-coded; a real application would need to make arrangements
   to have these items configurable by the end user.


	Mapping table persistence:
   --------------------------

   For minimum disruption to the network when one end of the tunnel is
   rebooted, some information is assumed to be preserved (in RAM) over
   a boot cycle.  In particular, the global data 'bt' and 'eth_router' is
   assumed to be preserved in battery-backed RAM.  This sample does not
   make any special arrangements since the battery is not a standard part
   of the RCM6600W interface board.  If you reboot by pressing the
   reset button, then the information will be preserved.  If you power
   off, then the tunnel will be disrupted until each host times out and
   refresh their ARP cache.  They may even need to restart their network
   interface in order for the tunnel to locate the router (via DHCP
   packet sniffing as described above).


   Antenna alignment and transmit power:
   -------------------------------------

	For a long-haul WiFi link, it is likely that directional antennae (such
   as dish or Yagi array) will be used at at least one end.  As an aid to
   alignment of the directional antenna, this sample outputs a PWM signal
   on PE7 which indicates the last received signal strength (RSSI) from the
   peer.  The absolute signal level (in dBm) is not available, however
   the signal level relative to the individual receiver circuit is
   available.  If the maximum receive level is arbitrarily set to 0dB,
   then the PWM is 100% for 0dB and above, and decreases by 3.2% for
   each 3dB below that; it will be 0% for -92dB.  A DC voltmeter attached
   to PE7 will thus read 3.3V for 0dBm or above, and 0V for -92bBm or below.
   This is about 100mV per 3dB.

   For this to work, the link signal strength must be at least sufficient to
   associate.  If there is no signal, then the PWM output will be zero.

   If there is no network activity, there may be insufficient received
   packets for an accurate RSSI to be determined in a timely manner.  This
   sample monitors the S1 switch on the interface board: while this switch
   is pressed (shorting PD1 to ground), the code will send dummy requests to
   the peer, about 16 times per second.  If a reply is received, its RSSI is
   recorded (and output on PE7) and the DS1 LED is flashed.

   Transmit power is fixed, and is set using the IFS_WIFI_TX_POWER parameter
   to ifconfig() in the associate() function.  The value ranges from 1
   (minimum) to 15 (maximum).  Usually, the maximum should be chosen for
   a long-haul link.  Although not shown in this sample, it would be possible
   to connect a potentiometer to PE0 and use the Rabbit 6000 analog input
   to implement a variable transmit power.  In debug mode, you can hit
   the '[' and ']' keys to change the transmit power.

   This sample assumes a long-haul link is to be set up, so the default
   transmit power is set to 15 (maximum available, limited by country
   regulation if applicable).  If the SW1 switch is held when booting,
   then the transmit power is set to 7.  This is useful for testing with
   both ends fairly close to each other.  The difference in received level
   will be about 8-10dB between the "long haul" and "local test" settings.


   LED indicator:
   --------------

   The DS1 LED on the interface board shows the basic link status as follows:

   1. No association: OFF
   2. Negotiating tunnel: short flashes at 1Hz
   3. Server waiting for client: long flashes at 1Hz
   4. Client waiting for server: 2Hz flashes
   5. Securing tunnel with TLS: 8Hz flashes
   6. Tunnel OK: ON.

   The distinction between client and server is somewhat arbitrary, but
   needed for the protocols to work.  This sample uses the highest hardware
   address to determine the client.

   States 2, 3, 4 and 5 in the above table are usually very short lived, so you
   may not see them unless there is a problem.  A likely problem is insufficient
   link quality, or mismatching TLS keys.

   While the S1 button is pressed, the LED is used to show receipt of
   'ping' packets.  S1 is also used at boot time to select between high
   (default) and low power transmission.  See previous section.

*/


/****************************************************************************
*****************************************************************************

	The following macros can be changed to configure this sample.

*****************************************************************************
****************************************************************************/

// Security.  Normally have this defined, unless debugging or packet sniffing.
// Not having TLS leaves the link wide open to eavesdropping and tampering.
#define USE_TLS

// Default router/mask in case of no DHCP server
#define FALLBACK_ROUTER 	IPADDR(192,168,20,1)
#define FALLBACK_NETMASK 	IPADDR(255,255,255,0)

// Define to override default MSS of 1460.  Pick smaller values if the
// Wifi link is expected to be sub-optimal.  Smaller packets over the air
// are less likely to be corrupted, and even though there must be more
// of them, the throughput can actually improve if interference is likely.
//#define TUNNEL_MSS	1024


// Hard-code Ad-hoc SSID and operating channel
#define TUNNEL_SSID	"EtherFiTunnel"
#define TUNNEL_CHAN	8

// Prevent a compile-time "IFC_WIFI_SSID has not been defined . . ." warning.
#define IFC_WIFI_SSID TUNNEL_SSID

// Since we establish on a fixed channel, increase the scan time
// so that we are more likely to pick up an existing IBSS.  Since IBSS beacon
// time is 100ms, need to look a bit harder to make sure both sides don't
// come up starting the IBSS.
#define _WIFI_SCAN_PERIOD 10	//	Scan period for each try (ms) - default 5
#define _WIFI_SCAN_LIMIT 50	// Number of periods per channel - default 3
#define IBSS_CREATE_TIMEOUT	10000	// ms to timeout IBSS creation
												// needs to be sufficient for linklocal

#define MAX_UDP_SOCKET_BUFFERS	1
#define USE_MULTICAST
#define USE_LINKLOCAL

// Extra buffers to avoid congestion
#define ETH_MAXBUFS	64

// Shorten default keepalive timer so that we are quicker to react to
// a rebooted peer.
#define KEEPALIVE_WAITTIME 	2
#define KEEPALIVE_NUMRETRYS   3

// Tweak TCP timeouts to account for variable Wifi link quality.  Since
// Wifi has its own packet retransmission, this can cause spikes in the
// observed round-trip time.  So don't let TCP be too hasty to retransmit.
#define RETRAN_STRAT_TIME  30
#define TCP_MINRTO	100
#define TCP_LAZYUPD	100

// Define socket buffer sizes (TCP).  Bigger the better, up to 32767.
#define BUFSIZE 32000

// UDP port number for discovery, TCP port number for socket.  Can be anything
// >=2 provided both sides use the same!
#define PORT	2

// Bridging table number of entries - should be prime number to get
// optimum hashing.
#define BR_TABSIZE	509


// Control messages and debugging.  Turn off all for a "production" setting.
#define BRIDGE_VERBOSE		// This sample
//#define ARP_VERBOSE
//#define ICMP_VERBOSE
//#define ZC_LL_VERBOSE		// Link-local verbose
//#define WIFIG_VERBOSE 1
//#define WIFI_MAC_MGMT_VERBOSE 1
//#define IP_VERBOSE
//#define _SSL_PRINTF_DEBUG 4
//#define SSL3_PRINTF_DEBUG
//#define SSL_SOCK_VERBOSE
//#define TCP_VERBOSE

//#define DCRTCP_DEBUG
//#define WIFIG_DEBUG
//#define TLS_DEBUG

/****************************************************************************
*****************************************************************************

	End of configuration macros.

   The following code should only be altered if major functional
   changes are desired.

*****************************************************************************
****************************************************************************/


// Dynamic configuration.
#define TCPCONFIG 6

// Required definitions, since we need to do some unusual stuff...
#define CUSTOM_ARP_HANDLER  bridge_arp_handler
#define CUSTOM_IP4_HANDLER  bridge_ip4_handler

// Required definition to have more liberal ARP rules.  This allows us to
// refresh our ARP table even if our interface was not completely configured
// (e.g. it was on the "wrong side" of the DHCP server).
#define _ARP_IGNORE_INVALID_SOURCE_IP

// Required, but only for dhcp parsing routines.  We don't use DHCP for
// configuration.
#define USE_DHCP

#use "rcm66xxw.lib"
#use "dcrtcp.lib"
#use "crc32.lib"
#ifdef USE_TLS
	// Set TLS options.  For clarity in demonstration, use pre-shared keys
   // with AES.
	#define SSL_USE_AES
	#define SSL_USE_PSK
	#define SSL_DONT_USE_RC4
	#define SSL_DONT_USE_RSA
	#use "ssl_sock.lib"
#endif

// Tunnelling state
enum {
	DOWN,					// Wifi interface down (must be value 0)
	ANNOUNCE,			// Link available, first phase of announce/discovery (AD)
   WAIT_ACK,			// Wait for AD packet with ACK
   LISTENING,			// Listening TCP socket (server)
   OPENWAIT,			// Waiting short time to open TCP socket (client)
	OPENING,				// Opening TCP socket (client)
#ifdef USE_TLS
   SECURING,  			// Establishing TLS session
#endif
	TUNNEL,				// Established tunnel, between received packets,
   						// waiting for Ethernet header.
	TUNNEL_ARP,			// Waiting for ARP packet
	TUNNEL_IP,			// Waiting for IPv4 header
	TUNNEL_IPDATA,		// Waiting for remainder of IPv4 packet
};

// Current tunnelling state
int bst = DOWN;


// The main socket for the tunnel (non-secured)
tcp_Socket tcp_sock;
#ifdef USE_TLS
	ssl_Socket *	ssl_sock = NULL;
#endif
void * sock = &tcp_sock;


// This is the database entry which allows mapping IP addresses to hardware
// addresses.  It is updated by looking at ARP packets, and used to
// determine the correct destination hardware addresses for packets
// received from the tunnel.
// Entries are indexed based on hash of the IP address.  The hash is simply
// the IP address as a byte-reversed number, modulo the table size.  E.g.
// for 10.10.6.100, the byte reversed number is 0x64060A0A, then modulo 511
// gives 0x1D.  A hash collision is resolved by adding 1 until a free table
// entry is located.  It's best if the table doesn't get more than 3/4 full.
typedef struct {
	longword ip;		// IP address
	byte	hwa[6];		// Hardware address
	int	iface;		// Interface (IF_ETH0 or IF_WIFI0) to
							// this node.  IF_ANY if entry not used.
   int	state;		// State of this entry as follows:
   #define BTS_UNUSED		0	// Unused (iface == IF_ANY in this case)
   #define BTS_TEST			1	// Need ARP probe ASAP
   #define BTS_CONFIRMING	2	// Sent ARP to confirm this entry
   #define BTS_OK				3	// Entry active
   #define BTS_NOARP			4	// No ARP response received
   longword crc;		// CRC of this entry (for checking at reboot), not
   						// including the following fields.

	longword last_used;
							// SEC_TIMER timestamp when this entry last used
   word	arp_timer;	// Short timer for ARP resolution
} BrEnt;

word	br_used = 0;			// Number of entries currently used.  Must never
									// get equal to BR_TABSIZE, otherwise a search
									// would not terminate.
long br_reorg_count = 0;	// Number of times bridging table re-organized.
longword	boot_time;			// Timestamp when booted
BrEnt far btx[BR_TABSIZE];	// Bridging table copy used when re-organizing.
int long_haul = 1;			// Set FALSE if SW1 held at boot time.

// Main bridging table.  This should be preserved over reboot e.g. in battery-
// backed RAM.
BrEnt far bt[BR_TABSIZE];

// Router information for Ethernet side, but only if valid CRC of the struct.
// This should also persist over reboot so as to avoid hosts having to
// re-acquire DHCP leases.
struct {
	int 		known;
	longword ip;
	longword netmask;
   longword crc;
} eth_router;


// Statistics:
typedef struct {
	// ARP packets received on Ethernet and tunnelled over Wifi:
	longword		arp_req;		// Requests
	longword		arp_req_unconfig;	// out of the above, those which had an
											// unconfigured source IP (0 or -1).
	longword		arp_rep;		// Replies
	longword		arp_rep_notfnd;	// out of the above, those whose source IP
											// was not found in our bridging table.
	longword		arp_other;	// Any other than request or reply
	longword		arp_bad;		// Invalid format
	longword		arp_nb;		// Non-tunnel mode
	longword		arp_fwd_err;// ARP packet dropped; tunnel congested
	longword		arp_fwd_ok;	// ARP tunnelled successfully

	// IP packets received on Ethernet and tunnelled over Wifi:
	longword		ip4_nb;		// Non-tunnel mode, or was a destination address
									// type that we can't forward.
   longword		ip4_bad;		// Malformed IP packet (less than length field)
	longword		ip4_br;		// Tunnelling attempted

	longword		ip4_fwd_err;// IP packet dropped; tunnel congested
	longword		bcast_fwd_err;	// Out of above, those which were broadcast
	longword		mcast_fwd_err;	// Out of above, those which were multicast
	longword		ip4_fwd_ok;	// IP tunnelled successfully
	longword		bcast_fwd_ok;	// Out of above, those which were broadcast
	longword		mcast_fwd_ok;	// Out of above, those which were multicast

	// DHCP and other types of broadcast or multicast:
	longword		dhcp_bcast;	// DHCP broadcast dest packets received
	longword		bcast;		// Other broadcast dest packets received
	longword		addp_mcast;	// ADDP multicast dest packets received
	longword		mcast;		// Other multicast dest packets received

   // ARP packets received from tunnel and forwarded over Ethernet:
	longword		tun_arp_req;		// Requests
	longword		tun_arp_req_unconfig;	// out of the above, those which had an
											// unconfigured source IP (0 or -1).
	longword		tun_arp_rep;		// Replies
	longword		tun_arp_rep_notfnd;	// out of the above, those whose source IP
											// was not found in our bridging table.
	longword		tun_arp_err;	// ARP packet dropped; ethernet congested
	longword		tun_arp_ok;		// ARP forwarded to ethernet successfully

	// IP packets received from tunnel and forwarded over Ethernet:
	longword		ip4_rout;	// Needed to route, not in bridging table
	longword		ip4_rout_err;	// Out of the above, those which could not be
										// routed because router HWA not known
	longword		tun_ip4_err;// IP packet dropped; ethernet congested
	longword		tun_bcast_err;	// Out of above, those which were broadcast
	longword		tun_mcast_err;	// Out of above, those which were multicast
	longword		tun_ip4_ok;	// IP forwarded successfully
	longword		tun_bcast_ok;	// Out of above, those which were broadcast
	longword		tun_mcast_ok;	// Out of above, those which were multicast

} Stats;

Stats st;


// Set when desired to send test packets
int send_test = 0;
// Set when received test packet reply
int test_reply = 0;
// Last received signal strength (raw value 0..92 in arbitrary base decibels)
int last_rx_signal = 0;
// Smoothed value of the above (weighted moving average of the last few)
float smoothed_rx_signal = 0.0;

void record_rx_signal(void)
{
	// This is not always entirely accurate, since the RSSI is not recorded
   // with each network packet at the time it arrives.  Thus, if other
   // packets arrive before the custom packet handlers get called, then
   // the rx_signal field may not reflect the packet we are handling.
   // Even so, this is good enough for optimizing a directional antenna etc.
	last_rx_signal = _wifi_rxFrame.rx_signal;
	smoothed_rx_signal = smoothed_rx_signal * 0.8 + last_rx_signal * 0.2;

   // Set PWM (on PE7) to indicate the range of signal
   // Largest signal is 92, smallest is 0.  Translate this to 100%..0% duty.
	WrPortI(PWM3R, &PWM3RShadow, (int)(255/92.0 * smoothed_rx_signal));
}



int entry_free(word i)
{
	return bt[i].state == BTS_UNUSED;
}

void update_crc(BrEnt far * b)
{
	// Note that we use the table size as the initial value.  This ensures
   // that resizing the table invalidates all entries, which is necessary
   // since the hash location depends on the table size.
	b->crc = crc32_calc(b, 14, BR_TABSIZE);
}

int crc_ok(const BrEnt far * b)
{
	return b->crc == crc32_calc(b, 14, BR_TABSIZE);
}

// Find table entry for a given IP address.  As side effect, if not found
// then sets global variable next_free to an appropriate entry for that IP.
// If matching entry found, updates its "last used" timer to prevent it
// being LRUd out.
BrEnt far * next_free;
BrEnt far * findIP(longword ipaddr)
{
	word j = (word)(intel(ipaddr) % BR_TABSIZE);
	BrEnt far * b = bt + j;
	for (;;) {
		next_free = b;
		if (entry_free(j))
			break;
		if (b->ip == ipaddr) {
	   	b->last_used = SEC_TIMER;
			return b;
		}
		++b;
		if (++j == BR_TABSIZE)
			j = 0, b = bt;
	}
	return NULL;
}

void reorg_table(void);

// Add new entry (or replace existing).
BrEnt far * last_added;
BrEnt far * addIP(longword ipaddr, byte far * hwa, int iface)
{
	BrEnt far * b = findIP(ipaddr);
	if (!b) {
		b = next_free;
		++br_used;
		// If br_used >= 3/4 table size, then need to reorganize
		// table by recreating it adding most recently used entries
		// first, until just half full.
		if (br_used > BR_TABSIZE*3/4) {
			reorg_table();
		}
	}
#ifdef BRIDGE_VERBOSE
	if (debug_on)
		printf("BARP: adding table entry for 0x%08lX on iface %d\n", ipaddr, iface);
#endif
	b->ip = ipaddr;
	_f_memcpy(b->hwa, hwa, sizeof(b->hwa));
	b->iface = iface;
   b->state = BTS_OK;
   update_crc(b);
	b->last_used = SEC_TIMER;
	return b;

}

#ifdef BRIDGE_VERBOSE
void bridge_printall(void);
#endif

int recent_first(void far * a, void far * b)
{
	longword au, bu;
	long c;
	// If state is BTS_UNUSED, entry is unused and should sort to the end.
	// Do this by setting last_used to boot_time (i.e. old as possible)
	if (((BrEnt far *)b)->state == BTS_UNUSED)
		bu = boot_time;
	else
		bu = ((BrEnt far *)b)->last_used;
	if (((BrEnt far *)a)->state == BTS_UNUSED)
		au = boot_time;
	else
		au = ((BrEnt far *)a)->last_used;
	c = (long)(bu - au);
	return c > 0 ? 1 : c < 0 ? -1 : 0;
}

void init_table(int force)
{
	word i;
   longword age;

	br_used = 0;

	for (i = 0; i < BR_TABSIZE; ++i) {
   	if (force) {
	      bt[i].state = BTS_UNUSED;
	      bt[i].iface = IF_ANY;
      }
      else {
      	if (crc_ok(bt + i) && bt[i].state == BTS_OK && bt[i].iface != IF_ANY) {
				age = boot_time - bt[i].last_used;
            if (age < 86400) {
            	// Less than a day ago, set to initiate ARP confirmation
               bt[i].state = BTS_TEST;
               ++br_used;
            }
            else
	         	bt[i].state = BTS_UNUSED;
         }
         else {
	         bt[i].state = BTS_UNUSED;
	         bt[i].iface = IF_ANY;
         }
      }
   }

	// Now check eth_router info
   if (eth_router.crc != crc32_calc(&eth_router, sizeof(eth_router)-4, 0)) {
   	// It's no good
      memset(&eth_router, 0, sizeof(eth_router));
#ifdef FALLBACK_ROUTER
		eth_router.ip = FALLBACK_ROUTER;
		eth_router.netmask = FALLBACK_NETMASK;
#endif

   }

#ifdef BRIDGE_VERBOSE
	bridge_printall();
#endif
}

void reorg_table(void)
{
	word i;
#ifdef BRIDGE_VERBOSE
	printf("BARP: re-organizing bridging table with %u entries\n", br_used);
#endif
	_f_memcpy(btx, bt, sizeof(btx));
	_f_qsort(btx, BR_TABSIZE, sizeof(btx[0]), recent_first);
	init_table(1);
	for (i = 0; i < BR_TABSIZE/2; ++i) {
		if (btx[i].state == BTS_UNUSED)
			break;
		addIP(btx[i].ip, btx[i].hwa, btx[i].iface)
		// put back original last used timer
		->last_used = btx[i].last_used;
	}
	++br_reorg_count;
}


void housekeeping(void)
{
	static ent = 0;
	// This is driven at about 16Hz when bridging mode.  Use it to keep the
   // bridging table up-to-date.

   // Note: 'break' from this switch to go to next entry; or 'return' to
   // process same entry next time.
   switch (bt[ent].state) {
   default:
   	break;
   case BTS_TEST:
   	// For now, always change from TEST to OK.
#ifdef BRIDGE_VERBOSE
		printf("BARP: ARP probe for old entry %08lX iface %d\n", bt[ent].ip, bt[ent].iface);
#endif
      bt[ent].state = BTS_OK;
		break;
   case BTS_CONFIRMING:
   	if (_CHK_SHORT_TIMEOUT(bt[ent].arp_timer)) {
#ifdef BRIDGE_VERBOSE
			printf("BARP: failed to re-locate %08lX iface %d\n", bt[ent].ip, bt[ent].iface);
#endif
      	bt[ent].state = BTS_NOARP;
         break;
      }
      return;
   }

   // Do next entry next time
   if (++ent == BR_TABSIZE)
   	ent = 0;
}

#ifdef BRIDGE_VERBOSE
void bridge_printall(void)
{
	word i;
   int st;
	char ipbuf[16];
	printf("Bridging table: %u out of %u entries.\n", br_used, BR_TABSIZE);
	if (br_used) {
		printf("status\tiface\tIP\t\tHWA\t\t\tAge (s)\n");
		printf("------\t-----\t---------------\t------------------\t---------\n");
	}
	for (i = 0; i < BR_TABSIZE; ++i) {
		if (!entry_free(i)) {
      	st = bt[i].state;
			printf("%s\t%u\t%.15s\t%02X:%02X:%02X:%02X:%02X:%02X\t%9lu\n",
         	st == BTS_OK ? "active" :
         	st == BTS_TEST ? "test" :
         	st == BTS_CONFIRMING ? "confing" :
         	                          "dead",
				bt[i].iface,
				inet_ntoa(ipbuf, bt[i].ip),
				bt[i].hwa[0],bt[i].hwa[1],bt[i].hwa[2],
				bt[i].hwa[3],bt[i].hwa[4],bt[i].hwa[5],
				SEC_TIMER - bt[i].last_used
				);
		}
	}

   if (eth_router.known) {
   	printf("Ethernet-side router/mask: %.15s/%.15s (%s)\n",
      	inet_ntoa(ipbuf, eth_router.ip),
         inet_ntoa(ipbuf, eth_router.netmask),
         findIP(eth_router.ip) ? "resolved" : "unresolved"
         );
   }
   else {
   	printf("Ethernet-side router/mask: unknown\n");
	#ifdef FALLBACK_ROUTER
		printf("... using fallback of %.15s/%.15s (%s)\n",
      	inet_ntoa(ipbuf, eth_router.ip),
         inet_ntoa(ipbuf, eth_router.netmask),
         findIP(eth_router.ip) ? "resolved" : "unresolved"
         );
	#endif
	}
}
#endif

/*--------------------------------------------------------------------------

   This routine is called when a packet is to be sent over the tunnel.
   Basically, it writes to the socket buffer and then lets tcp_tick()
   handle the details.

--------------------------------------------------------------------------*/
int tunnel(ll_Gather * g)
{
	// Since this is called from packet handler, cannot call tcp_tick().
   // If no buffer space, we have no choice but to drop it.
	word len = g->len1 + g->len2;
#ifdef BRIDGE_VERBOSE
	if (debug_on > 1)
      printf("Tunnel: sending pkt length %u (%u+%u)\n", len, g->len1, g->len2);
#endif
   if (sock_writable(sock) <= len) {
#ifdef BRIDGE_VERBOSE
      printf("Tunnel: dropping, writable only %d\n", sock_writable(sock)-1);
#endif
      return 1;
   }
   // Enough space
   sock_fastwrite(sock, g->data1, g->len1);
   sock_fastwrite(sock, g->data2, g->len2);
	return 0;
}

// Update table and rewrite ARP packet
void update_bridge_table(ether_ll_hdr * ep, arp_Header *in, int iface)
{
   longword his_ip, dst_ip;
	BrEnt far * b;

   his_ip = intel(in->srcIPAddr);
   dst_ip = intel(in->dstIPAddr);

#ifdef BRIDGE_VERBOSE
	if (debug_on) {
	   if (in->opcode == ARP_REQUEST)
	      printf("%s: who has %08lX? Tell %08lX  i/f %d\n",
         	iface == IF_ETH0 ? "Eth" : "Tunnel",
         	dst_ip, his_ip, iface);
	   else
	      printf("%s: %08lX replying to %08lX  i/f %d\n",
         	iface == IF_ETH0 ? "Eth" : "Tunnel",
         	his_ip, dst_ip, iface);
	}
#endif

   // Update bridging table, and modify for forwarding.
   // For a query, we populate a (possibly new) table entriy for his_ip,
   // which can be fully set up, whereas the entry for dst_ip is not
   // known until the reply.  If his_ip is zero or -1 (i.e. unconfigured)
   // then we don't make an entry for this query.  Note that this works
   // for gratuitous ARP, in which case only the entry for the node is
   // made (there will normally be no replies).  The query is always
   // tunnelled so it will be broadcast on the remote ethernet segment.
   // When a reply is received, we create an entry for dst_ip.  If his_ip
   // is unconfigured, the reply is broadcast on the other ethernet,
   // otherwise it is unicast or broadcast as received.
   if (in->opcode == ARP_REQUEST) {
   	if (iface == IF_ETH0)
			++st.arp_req;
      else
      	++st.tun_arp_req;
   	if (his_ip && (long)his_ip != -1L)
   		addIP(his_ip, (byte *)&in->srcEthAddr, iface);
   	else {
      	if (iface == IF_ETH0)
   			++st.arp_req_unconfig;
         else
         	++st.tun_arp_req_unconfig;
      }
      memset(ep->dest, 0xFF, 6);	// Broadcast link-layer
   }
   else {
   	if (iface == IF_ETH0)
			++st.arp_rep;
      else
      	++st.tun_arp_rep;
   	addIP(his_ip, (byte *)&in->srcEthAddr, iface);
   	memset(&in->dstEthAddr, 0, sizeof(in->dstEthAddr)); // assume unknown
	   // If received unicast, send on unicast to original requester (if in table)
	   // else send on broadcast.
		if (memcmp(ep->dest, "\xFF\xFF\xFF\xFF\xFF\xFF", 6)) {
			b = findIP(dst_ip);
			if (b) {
				_f_memcpy(ep->dest, b->hwa, 6);
			   _f_memcpy(&in->dstEthAddr, b->hwa, sizeof(in->dstEthAddr));
			}
			else {
   			if (iface == IF_ETH0)
					++st.arp_rep_notfnd;
            else
            	++st.tun_arp_rep_notfnd;
      		memset(ep->dest, 0xFF, 6);
      	}
		}
   }
}

/*--------------------------------------------------------------------------

	This function is responsible for handling incoming ARP packets.

	Anything incoming from the Wifi side is handled as per normal.

	Anything from the Ethernet side is forwarded over the tunnel.

	See tunnel_handler() for the "other side" i.e. processing of
	backets received on the Wifi tunnel.

--------------------------------------------------------------------------*/

far ll_prefix *bridge_arp_handler(far ll_prefix *LL,
                                    byte *hdrbuf, word *pflags)

{
   auto arp_Header *in;
   auto ether_ll_hdr * ep;
	auto ll_Gather g;

	*pflags = 0;

   if (LL->iface == IF_WIFI0) {
#ifdef BRIDGE_VERBOSE
		if (debug_on > 2)
			printf("Got Wifi ARP\n");
#endif
		*pflags |= CUSTOM_PKT_FLAG_PROCESS;	// Normal processing requested
      record_rx_signal();
      if (send_test)
      	test_reply = 1;
		return LL;
	}

	if (bst < TUNNEL) {
#ifdef BRIDGE_VERBOSE
		if (debug_on)
			printf("Got Eth ARP (not bridging)\n");
#endif
		++st.arp_nb;
_norm:
		return LL;
	}


   if (LL->len < LL->net_offs + sizeof(arp_Header)) {
#ifdef BRIDGE_VERBOSE
		printf("Got Eth ARP (bad)\n");
#endif
		++st.arp_bad;
   	return LL;	// Discard it, too short to contain ARP header
   }

   _pkt_buf2root(LL, in = (arp_Header *)(hdrbuf+LL->net_offs),
   								sizeof(arp_Header), LL->net_offs);
	// We construct outgoing ethernet header immediately before the received
	// ARP header, to avoid unnecessary copy.  14 bytes for ethernet header
	// plus one byte (unused) for interface number in the ep struct.  There
	// is guaranteed to be 14 bytes available (net_offs >= 14).
	ep = (ether_ll_hdr *)in - 1;
  	LL->net_proto = NET_PROTO_ARP;	// Got valid ARP packet

   if (in->hwType != arp_TypeEther ||      // have ethernet hardware,
       in->protType != IP_TYPE ||          // and IP network layer
       in->opcode != ARP_REQUEST && in->opcode != ARP_REPLY) { // Normal ARP (not RARP)
      // Can't forward these...
#ifdef BRIDGE_VERBOSE
		printf("Got Eth ARP (not Eth<->IP request or reply)\n");
#endif
		++st.arp_other;
   	return LL;
   }

  	ep->type = ARP_TYPE;
	update_bridge_table(ep, in, LL->iface);

   memset(&g, 0, sizeof(g));
   g.len1 = sizeof(arp_Header) + 14;	// Must be fixed length
   g.data1 = (char __far *)ep;
   if (tunnel(&g))
   	++st.arp_fwd_err;
   else
   	++st.arp_fwd_ok;

	return LL;
}


DHCPInfo di_sniff;

udp_Header far * is_dhcp(far ll_prefix *LL, byte *hdrbuf)
{
/* Check if packet is DHCP, and we are currently bridging.
   NOTE: also perform side effect on DHCP packets, to turn on broadcast
   flag.  If we don't do this, then server will reply unicast to the
   address in chaddr field, but then we (the bridge) won't be able to
   correctly forward the packet.  We could manipulate chaddr, but it's
   much easier - if a bit cheesy - to force the broadcast bit on.

   We also examine the packet in order to determine a router and
   netmask for the Ethernet side.  This is necessary so packets can be
   forwarded past the local network segment.
*/
	in_Header * ip;
   udp_Header far * up;
   DHCPPkt far * dp;
	int iface = LL->iface;
   word len, chk, dhcptype;
   word dstPort, srcPort;
   int isdhcp, modified;

   ip = (in_Header *)(hdrbuf + LL->net_offs);

   // Copy the UDP header to hdrbuf
   if (LL->len < LL->tport_offs + sizeof(udp_Header))
   	return NULL;	// Too short to contain UDP header

   up = (udp_Header far *)(LL->data1 + LL->tport_offs);

   len = ntohs(up->length);

   dstPort = ntohs(up->dstPort);
   srcPort = ntohs(up->srcPort);

   isdhcp = dstPort == IPPORT_BOOTPS && srcPort == IPPORT_BOOTPC ||
            dstPort == IPPORT_BOOTPC && srcPort == IPPORT_BOOTPS;

   if (isdhcp) {
   	modified = 0;
   	dp = (DHCPPkt far *)(up + 1);

      // Parse options
      dhcptype = dhcp_getopts(IF_ETH0, &di_sniff, dp, len - sizeof(udp_Header), 0, 0);
      if (dhcptype == DHCP_TY_ACK) {
      	eth_router.ip = di_sniff.router[0];
         eth_router.netmask = di_sniff.tent_subnet;
         eth_router.known = 1;
         eth_router.crc = crc32_calc(&eth_router, sizeof(eth_router)-4, 0);
#ifdef BRIDGE_VERBOSE
			if (debug_on)
				printf("Found DHCP ACK with router=%08lX netmask=%08lX\n",
            	eth_router.ip, eth_router.netmask);
#endif

      }

   	if (!(dp->bp_flags & DHCP_F_BROADCAST)) {
   		modified = 1;
			dp->bp_flags |= DHCP_F_BROADCAST;
		}
		// Fix up UDP checksum field, if necessary
		if (modified && (chk = intel16(up->checksum))) {
			#asm
			ld		hl,(sp+@sp+chk)
			ld		de,0x8000
			sub	hl,de
			jr		nc,.skip
			dec	hl
		.skip:
			test	hl
			jr		nz,.skip2
			dec	hl
		.skip2:
			ld		(sp+@sp+chk),hl
			#endasm
			up->checksum = intel16(chk);
		}
   }
   else
   	up = NULL;

   return up;

}

/*--------------------------------------------------------------------------

	This function is responsible for handling incoming IP packets.

	Anything incoming from the Wifi side is handled as per normal.

	Anything from the Ethernet side is forwarded over the tunnel.

	See tunnel_handler() for the "other side" i.e. processing of
	backets received on the Wifi tunnel.

--------------------------------------------------------------------------*/

far ll_prefix *bridge_ip4_handler(far ll_prefix *LL,
                                    byte *hdrbuf, word *pflags, in_Header * ip)
{
   ether_ll_hdr * ep;
	ll_Gather g;
	longword dst = intel(ip->destination);
	longword src = intel(ip->source);
	int iface = LL->iface;
	int bcast;				// Flag set when forwarding broadcast.
	int mcast;				// Flag set when forwarding multicast.
	BrEnt far * b;
   unsigned len;
   unsigned iphdrlen;

	*pflags = 0;
   if (iface == IF_WIFI0) {
#ifdef BRIDGE_VERBOSE
		if (debug_on > 2)
			printf("Got Wifi IP\n");
#endif
		*pflags |= CUSTOM_PKT_FLAG_PROCESS;	// Normal processing requested
      record_rx_signal();
		return LL;
	}


	if (bst < TUNNEL) {
		// Not in tunnel mode...
#ifdef BRIDGE_VERBOSE
		if (debug_on)
			printf("Got Eth IP (not bridging)\n");
#endif
      ++st.ip4_nb;
      return LL;
	}

	ep = (ether_ll_hdr *)ip - 1;
	bcast = mcast = 0;

	if (!dst || !src ||
			IS_ANY_BCAST_ADDR(dst) ||
			IS_ANY_NET_ADDR(dst)) {
		bcast = 1;
		// Check for broadcast DHCP packets i.e. UDP with ports IPPORT_BOOTPS and
		// IPPORT_BOOTPC...
		if (is_dhcp(LL, hdrbuf)) {
			++st.dhcp_bcast;
#ifdef BRIDGE_VERBOSE
	      if (debug_on)
	         printf("BIP: DHCP found, %08lX(on %d)->%08lX\n", src, iface, dst);
#endif
		}
		else
			++st.bcast;
	}
	else if (IS_MULTICAST_ADDR(dst)) {
		mcast = 1;
		if (dst == IPADDR(224,0,5,128)) {
			++st.addp_mcast;
#ifdef BRIDGE_VERBOSE
	      if (debug_on)
	         printf("BIP: ADDP found, %08lX(on %d)->%08lX\n", src, iface, dst);
#endif
		}
		else
			++st.mcast;
	}

	// It's something we can potentially forward
  	ep->type = IP_TYPE;

   // Get actual IP packet length.  LL->len may include extraneous data like
   // the ethernet CRC, so use the IP field instead.  Don't let it exceed
   // the real packet length, though.
   len = ntohs(ip->length);
   if (len > LL->len - LL->net_offs) {
   	// Malformed (shorter than declared length)
#ifdef BRIDGE_VERBOSE
		printf("Got Eth IP (bad length)\n");
#endif
      ++st.ip4_bad;
   	return LL;
   }

   iphdrlen = LL->tport_offs - LL->net_offs;

#ifdef BRIDGE_VERBOSE
	if (debug_on > 2)
		printf("BIP: %08lX(on %d)->%08lX proto=%u len=%u/%u\n",
      	src, iface, dst, ip->proto,
         len, LL->len - LL->net_offs);
#endif


   memset(&g, 0, sizeof(g));
   g.len1 = 14 + iphdrlen;
   g.data1 = (char __far *)ep;
   g.len2 = len - iphdrlen;
   g.data2 = (char __far *)(LL->data1 + LL->tport_offs);

   if (tunnel(&g)) {
   	++st.ip4_fwd_err;
   	if (bcast)
   		++st.bcast_fwd_err;
   	else if (mcast)
   		++st.mcast_fwd_err;
   }
   else {
   	++st.ip4_fwd_ok;
   	if (bcast)
   		++st.bcast_fwd_ok;
   	else if (mcast)
   		++st.mcast_fwd_ok;
   }

	return LL;
}

void clear_stats(void)
{
	memset(&st, 0, sizeof(st));
   memset(&_wifi_macStats, 0, sizeof(_wifi_macStats));
}


#ifdef BRIDGE_VERBOSE
void print_stats(void)
{
	printf("\n%u out of %u bridging table entries used\n\n", br_used, BR_TABSIZE);
	printf("\t\tDescr\n");
	printf("---------------\t------------------\n");
#define PS(field, descr) printf("%10lu\t%s\n", st.field, descr)
	//PS(arp_nb, "ARP when non-tunnel mode");
	PS(arp_req, "ARP requests to tunnel");
	PS(arp_req_unconfig, " - of above, with unconfigured source");
	PS(arp_rep, "ARP replies to tunnel");
	PS(arp_rep_notfnd, " - of above, not in bridging table");
	PS(arp_other, "ARP not reply or request");
	PS(arp_bad, "ARP format errors");
	PS(arp_fwd_err, "ARP dropped, tunnel congested");
	PS(arp_fwd_ok, "ARP tunnelled OK");

	PS(tun_arp_req, "ARP requests from tunnel");
	PS(tun_arp_req_unconfig, " - of above, with unconfigured source");
	PS(tun_arp_rep, "ARP replies from tunnel");
	PS(tun_arp_rep_notfnd, " - of above, not in bridging table");
	PS(tun_arp_err, "ARP dropped, Ethernet congested");
	PS(tun_arp_ok, "ARP forwarded OK");

	//PS(ip4_nb, "IPv4 when non-tunnel mode");
	PS(ip4_bad, "IPv4 apparently truncated");
	PS(ip4_br, "IPv4 to tunnel");
	PS(ip4_fwd_err, "IPv4 dropped, tunnel congested");
	PS(bcast_fwd_err, " - of above, with broadcast dest");
	PS(mcast_fwd_err, " - of above, with multicast dest");
	PS(ip4_fwd_ok, "IPv4 tunnelled OK");
	PS(bcast_fwd_ok, " - of above, with broadcast dest");
	PS(mcast_fwd_ok, " - of above, with multicast dest");
	PS(dhcp_bcast, "DHCP broadcast seen");
	PS(bcast, "Other broadcast seen");
	PS(addp_mcast, "ADDP multicast seen");
	PS(mcast, "Other multicast seen");

	PS(ip4_rout, "IPv4 routed");
	PS(ip4_rout_err, "of above, router not known");
	PS(tun_ip4_err, "IPv4 dropped, Ethernet congested");
	PS(tun_bcast_err, " - of above, with broadcast dest");
	PS(tun_mcast_err, " - of above, with multicast dest");
	PS(tun_ip4_ok, "IPv4 forwarded OK");
	PS(tun_bcast_ok, " - of above, with broadcast dest");
	PS(tun_mcast_ok, " - of above, with multicast dest");

	printf("\n");
   printf("Wifi (tunnel) statistics:\n");
   printf("   TX:\n");
   printf("    bytes              %u\n", _wifi_macStats.txBytes);
   printf("    directed frames    %u\n", _wifi_macStats.txFrames);
   printf("    broadcast frames   %u\n", _wifi_macStats.txBCFrames);
   printf("    RTS frames         %u\n", _wifi_macStats.txRTS);
   printf("    Retried frames     %u\n", _wifi_macStats.txRetries);
   printf("    Dropped:\n");
   printf("     retry limit       %u\n", _wifi_macStats.txDropRetry);
   printf("     broadcast error   %u\n", _wifi_macStats.txDropBC);
   printf("     not associated    %u\n", _wifi_macStats.txDropAssoc);
   printf("   RX:\n");
   printf("    bytes              %u\n", _wifi_macStats.rxBytes);
   printf("    directed frames    %u\n", _wifi_macStats.rxFrames);
   printf("    broadcast frames   %u\n", _wifi_macStats.rxBCFrames);
   printf("    RTS frames         %u\n", _wifi_macStats.rxRTS);
   printf("    retried frames     %u\n", _wifi_macStats.rxRetries);
   printf("    Dropped:\n");
   printf("     too large         %u\n", _wifi_macStats.rxDropSize);
   printf("     out of buffers    %u\n", _wifi_macStats.rxDropBuffer);
   printf("     invalid frame     %u\n", _wifi_macStats.rxDropInvalid);
   printf("     duplicate frame   %u\n", _wifi_macStats.rxDropDup);
   printf("     lifetime limit    %u\n", _wifi_macStats.rxDropAge);
   printf("     hardware overrun  %u\n", _wifi_macStats.rxDropOverrun);
	printf("\n");
}
#endif


void associate(const char * ssid, word ssid_len, int channel)
{
   ifdown(IF_WIFI0);
   while (ifpending(IF_WIFI0) != IF_DOWN)
   	tcp_tick(NULL);

   if (!ssid_len)
   	ssid_len = strlen(ssid);

#ifdef BRIDGE_VERBOSE
   printf("Creating IBSS %.*s on channel %d\n",
      ssid_len, ssid, channel);
#endif
	// just force linklocal address since there is nothing else
 	// on this network.
   ifconfig( IF_WIFI0,
	   	IFS_IPADDR, IPADDR(169,254,0,0),
	   	IFS_NETMASK, IPADDR(255,255,0,0),
	   	IFS_ROUTER_SET, IPADDR(0,0,0,0),
	      IFS_WIFI_MODE, IFPARAM_WIFI_ADHOC,
	      IFS_WIFI_SSID, ssid_len, ssid,
	      IFS_WIFI_CHANNEL, channel,
	      IFS_WIFI_ENCRYPTION, IFPARAM_WIFI_ENCR_NONE,
	      IFS_WIFI_AUTHENTICATION, IFPARAM_WIFI_AUTH_OPEN,
         // Following sets (fixed) transmit power
         IFS_WIFI_TX_POWER, long_haul ? 15 : 7,
	      IFS_END);

   ifup(IF_WIFI0);
}

int create_ibss()
{
   // Hard-coded configuration items...
   associate(TUNNEL_SSID, 0, TUNNEL_CHAN);
   return 0;
}


#ifdef BRIDGE_VERBOSE
/****************************************************************************
	print_macaddress

	Routine to print out mac_addr types.

****************************************************************************/
void print_macaddress(far unsigned char *addr)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2],
	       addr[3], addr[4], addr[5]);
}

/****************************************************************************
	wifi_rates_str

	Routine to convert _wifi_wln_scan_bss.rates_basic and .rates
   to a string listing speeds in Mbps.  A basic rate is suffixed with an
   asterisk (*).

****************************************************************************/
const int rates[] = { 10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540 };
char *wifi_rates_str(char *buffer, word basic, word oper)
{
	int i;
	char *p;

   p = buffer;
   for (i = 0; i < (sizeof(rates) / sizeof(rates[0])); i++) {
		if (basic & (1 << i))
			p += sprintf (p, "%.1f* ", rates[i] * 0.1);
      else if (oper & (1<<i))
			p += sprintf (p, "%.1f ", rates[i] * 0.1);
   }
   if (p == buffer) strcpy (p, "none");
   else sprintf (p, "Mbps");

   return buffer;
}

/****************************************************************************
	wifi_cap_str

	Routine to convert _wifi_wln_scan_bss.bss_caps
   to a string listing BSS capabilities.

****************************************************************************/
const char * cap_strs[] =
	{ "ESS", "IBSS", "POLLABLE", "POLLREQ", "PRIVACY", "SHORTPRE", "PBCC", "AGILITY" };
char *wifi_cap_str(char *buffer, word caps)
{
	int i;
	char *p;

   p = buffer;
   for (i = 0; i < (sizeof(cap_strs) / sizeof(cap_strs[0])); i++) {
		if (caps & (1 << i))
			p += sprintf (p, "%s ", cap_strs[i]);
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	wifi_erp_str

	Routine to convert _wifi_wln_scan_bss.erp_info
   to a string listing ERP capabilities.

****************************************************************************/
const char * erp_strs[] =
	{ "NONERP", "USEPROTECT", "BARKER" };
char *wifi_erp_str(char *buffer, word caps)
{
	int i;
	char *p;

   p = buffer;
   for (i = 0; i < (sizeof(erp_strs) / sizeof(erp_strs[0])); i++) {
		if (caps & (1 << i))
			p += sprintf (p, "%s ", erp_strs[i]);
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	wifi_auths_str

	Routine to convert wifi_status.authen bitfield to a string listing
   authentication types.

****************************************************************************/
char * const auths[] = { "open", "wep shared", "wep 802.1x", "wpa psk", "wpa 802.1x", "leap" };
char *wifi_auths_str (char *buffer, longword auth)
{
	int i;
   char *p;

   p = buffer;
   for (i = 0; i < (sizeof(auths) / sizeof(auths[0])); i++) {
		if (auth & (1 << i)) {
      	if (p != buffer) p += sprintf (p, ", ");
			p += sprintf (p, "%s", auths[i]);
      }
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	wifi_encrs_str

	Routine to convert wifi_status.encrypt bitfield to a string listing
   encryption types.

****************************************************************************/
char * const encrs[] = { "open", "wep", "tkip", "ccmp" };
char *wifi_encrs_str (char *buffer, longword auth)
{
	int i;
   char *p;

   p = buffer;
   for (i = 0; i < (sizeof(encrs) / sizeof(encrs[0])); i++) {
		if (auth & (1 << i)) {
      	if (p != buffer) p += sprintf (p, ", ");
			p += sprintf (p, "%s", encrs[i]);
      }
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	wifi_options_str

	Routine to convert _wifi_macParams.options bitfield to a string listing
   overall options.

****************************************************************************/
char * const opt_strs[] = { "antdiv", "shortpre", "vercert", "bonly", "rtsprot",
									"fixedrate", "multidomain" };
char *wifi_options_str (char *buffer, longword opts)
{
	int i;
   char *p;

   p = buffer;
   for (i = 0; i < (sizeof(opt_strs) / sizeof(opt_strs[0])); i++) {
		if (opts & (1uL << i)) {
      	if (p != buffer) p += sprintf (p, ", ");
			p += sprintf (p, "%s", opt_strs[i]);
      }
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	print_wifi

	Routine to print out status (wln_status type).

****************************************************************************/
char * pend_str(int p)
{
	switch (p) {
	case IF_DOWN: return "DOWN";
	case IF_COMING_DOWN: return "COMING DOWN";
	case IF_UP: return "UP";
	case IF_COMING_UP: return "COMING UP";
	default: return "Other";
	}
}

char * wifi_state_str(wln_state s)
{
	switch (s) {
	case WLN_ST_STOPPED: return "Driver not running";
	case WLN_ST_SCANNING: return "Scanning for BSS";
	case WLN_ST_ASSOC_ESS: return "Associated with ESS";
	case WLN_ST_AUTH_ESS: return "Authenticated with ESS";
	case WLN_ST_JOIN_IBSS: return "Joined ad-hoc IBSS";
	case WLN_ST_START_IBSS: return "Started ad-hoc IBSS";
	default: return "Unknown/illegal status";
	}
}

void print_wifi(void)
{
   wifi_status status_i;
   wifi_status * status;
	char buffer[80];	// 60 should actually be enough

	status = &status_i;
	ifconfig (IF_WIFI0, IFG_WIFI_STATUS, status, IFS_END);
   printf("\nMAC status:\n");
	printf("   state = %d (%s)\n",
		status->state, wifi_state_str(status->state));
	printf("   ssid = %s\n", status->ssid);
	printf("   ssid_len = %d\n", status->ssid_len);
	printf("   channel = %d\n", status->channel);
	printf("   bss_addr = ");
	print_macaddress(status->bss_addr);
	printf("\n");
	printf("   bss_caps = %04x\n", status->bss_caps);
	printf("   authen = %08lx (%s)\n", status->authen,
   	wifi_auths_str (buffer, status->authen));
	printf("   encrypt = %08lx (%s)\n", status->encrypt,
   	wifi_encrs_str (buffer, status->encrypt));
	printf("   tx_rate = %4.1f\n", status->tx_rate * 0.1);
	printf("   rx_rate = %4.1f\n", status->rx_rate * 0.1);
	printf("   rx_signal = %d\n", status->rx_signal);
	printf("   tx_power = %d\n", status->tx_power);

   // Also print current MAC parameters
   printf("\nMAC parameters:\n");
   printf("   Options: %s\n", wifi_options_str(buffer, _wifi_macParams.options));
	printf("   max tx_rate = %4.1f\n", _wifi_macParams.tx_rate * 0.1);
	printf("   RTS threshold = %d\n", _wifi_macParams.rts_thresh);
	printf("   frag threshold = %d\n", _wifi_macParams.frag_thresh);
   // Dynamic rate adjustment parameters:
   printf("   _wifi_rateCount = %d\n", _wifi_rateCount);
   printf("   _wifi_rateTxCount = %d\n", _wifi_rateTxCount);
   printf("   _wifi_rateTx = %d\n", _wifi_rateTx);
   printf("   _wifi_rateErrorCount = %d\n", _wifi_rateErrorCount);
   printf("   _wifi_rateIdx = %d\n", _wifi_rateIdx);
   printf("   _wifi_successThreshold = %d\n", _wifi_successThreshold);
   printf("   _wifi_success = %d\n", _wifi_success);

}
#endif


/****************************************************************************
*****************************************************************************

	Routines to implement Announce/Discovery Protocol (ADP).

   This is a made-up protocol just to get both ends of the tunnel known
   to each other.  Once the necessary information is known (i.e. the
   IP address of this node and the peer) then a TCP socket can be
   established to construct the actual tunnel.

*****************************************************************************
****************************************************************************/

// Announce/Discover (AD) packet format (broadcast on UDP port 2)
// Since this is sent plaintext, avoid any sensitive information such as
// any negotiation of security parameters - this must be left to TLS proper.
typedef struct
{
	long	magic;			// Magic number indicating tunnel establishment protocol
   							// version and also to eliminate "random" packets from
                        // being interpreted inappropriately.
#define MAGIC	0x5E77EA01L
	char	my_hwa[6];		// My HWA (on Wifi interface)
   char  your_hwa[6];	// Other peer's Wifi HWA.  This is initially zero,
   							// and is set to non-zero when we have received peer's
                        // AD packet and want to set up tunnel to it.  Non-zero
                        // in this field constitutes acknowledgment.
   longword	my_ip;		// My IP address (linklocal)
   longword	your_ip;		// Peer's IP address (linklocal)
} AD_pkt;

// Global packet buffer
AD_pkt adp;


udp_Socket	adp_sock;
word adp_timer;
word adp_retry = 0;

// Initialize the state machine
void start_tunnel(void)
{
	bst = ANNOUNCE;
  	//sock_abort(sock);
}

// Send an AD packet (adp global)
void send_adp(void)
{
   _udp_datagram_info udi;

   memset(udi.hwa, 0xFF, sizeof(udi.hwa));   // Assume broadcast hardware
   udi.remip = 0xFFFFFFFFuL;
   udi.remport = PORT;
   udi.len = PORT;   // Hack for socketless UDP
   udi.iface = IF_WIFI0;
   udi.flags = 0;    // Default TOS
#ifdef BRIDGE_VERBOSE
	if (debug_on)
   	printf("Sending AD packet\n");
#endif
   // Socketless UDP send
   udp_write(NULL, &adp, sizeof(adp), 0, &udi);

}

// Static pointers to socket buffers.  To avoid memory leaks, the
// same buffers are used over and over.
void far * rxbuf = NULL;
void far * txbuf = NULL;
#ifdef USE_TLS
// Also need plaintext buffers for TLS
void far * app_rxbuf = NULL;
void far * app_txbuf = NULL;
#endif

// Tweak socket settings (MSS and buffering) once opened.
void set_sockopts(tcp_Socket * s)
{
#ifdef TUNNEL_MSS
	s->mss = TUNNEL_MSS;
#endif
	// Since the library only supports up to 32k socket buffer, which needs
   // to be divided into two (for transmit and receive), bypass the library
   // and allocate bigger buffers.
   // We need the biggest buffers possible, since hosts will often open
   // multiple sockets at the TCP level, which we need to multiplex over
   // the single-socket tunnel.
	if (!rxbuf)
   	rxbuf = _sys_malloc(BUFSIZE);
	if (!txbuf)
   	txbuf = _sys_malloc(BUFSIZE);
#ifdef USE_TLS
	if (!app_rxbuf)
   	app_rxbuf = _sys_malloc(BUFSIZE);
	if (!app_txbuf)
   	app_txbuf = _sys_malloc(BUFSIZE);
#endif
   s->rd.buf = rxbuf;
   s->rd.maxlen = BUFSIZE;
   s->wr.buf = txbuf;
   s->wr.maxlen = BUFSIZE;
  	_tbuf_reset(&s->rd);
  	_tbuf_reset(&s->wr);


}

int adp_handler(int event, udp_Socket * s, ll_Gather * g, _udp_datagram_info * udi)
{
	AD_pkt far * a;
   int rc;

#ifdef BRIDGE_VERBOSE
	if (debug_on)
		printf("Got AD packet\n");
#endif
	if (event != UDP_DH_INDATA)
		return 0;
   if (bst < WAIT_ACK)
   	return 1;
   // g->data2/len2 is the UDP datagram received.  Map it to an AD_pkt struct.
   a = (AD_pkt far *)g->data2;
   if (g->len2 < sizeof(*a))
   	return 1;
   if (a->magic != MAGIC)
   	return 1;
	if (bst == WAIT_ACK) {
		_f_memcpy(adp.your_hwa, a->my_hwa, sizeof(adp.your_hwa));
      adp.your_ip = a->my_ip;
   }

	if (!_f_memcmp(a->your_hwa, adp.my_hwa, sizeof(a->your_hwa))) {
      // Got ACK for me, proceed to next state (open TCP socket)
#ifdef BRIDGE_VERBOSE
		if (debug_on)
			printf(" - with ACK\n");
#endif
		if (bst == WAIT_ACK) {
	      // Who has the higher HWA address?
	      if (memcmp(adp.my_hwa, adp.your_hwa, sizeof(adp.my_hwa)) > 0) {
	         bst = LISTENING;
#ifdef BRIDGE_VERBOSE
				printf("Listening (as server)\n");
#endif
				rc = tcp_extlisten(sock, IF_WIFI0, PORT, adp.your_ip, PORT,
            			NULL, 0, 0, 0);
            if (!rc) {
					exit(1);	// Should not happen
            }
            set_sockopts(sock);
   			adp_timer = _SET_SHORT_TIMEOUT(17000);
	      }
	      else {
	         bst = OPENWAIT;
      		send_adp();	// Send to ensure server side has ACK from us
            // Wait 500ms before opening to allow peer to listen first
   			adp_timer = _SET_SHORT_TIMEOUT(500);
	      }
      }
   }
   else if (!_f_memcmp(a->your_hwa, "\0\0\0\0\0\0", 6)) {
#ifdef BRIDGE_VERBOSE
		if (debug_on)
	  		printf(" - with no ACK\n");
#endif
   	// Peer does not yet know our address.  Respond with it.
      send_adp();
   }

   return 1;
}


/****************************************************************************
*****************************************************************************

	Routines for handling packets received from tunnel.

*****************************************************************************
****************************************************************************/

// A static struct where the next packet received over the tunnel is
// assembled.
struct {
	ether_ll_hdr 	eth;
   union {
   	arp_Header	arp;
      struct {
			in_Header	hdr;
         char		payload[MAX_MTU - sizeof(in_Header)];
      } ip;
   } u;
} tun;
word	ipdata_len;


/*--------------------------------------------------------------------------

	This function is responsible for handling ARP packets.

	tun.u.arp contains latest ARP header.  Forward it over the Ethernet
   interface after rewriting the sender address (so we become the proxy).

--------------------------------------------------------------------------*/

void handle_tunnel_arp()
{
	ll_Gather g;

#ifdef BRIDGE_VERBOSE
	if (debug_on > 2)
		printf("Got tunnelled ARP\n");
#endif
	update_bridge_table(&tun.eth, &tun.u.arp, IF_WIFI0);

   // Rewrite sender address, both in the ARP packet itself and in the
   // ethernet header.
   _f_memcpy(&tun.u.arp.srcEthAddr, my_eth_addr[IF_ETH0], 6);
   memcpy(tun.eth.src, my_eth_addr[IF_ETH0], 6);

   memset(&g, 0, sizeof(g));
   g.iface = IF_ETH0;
   g.len1 = sizeof(arp_Header) + 14;
   g.data1 = (char __far *)&tun;
   if (pkt_gather(&g)) {
#ifdef BRIDGE_VERBOSE
		printf("Tunnelled ARP: could not forward to Eth.\n");
#endif
   	++st.tun_arp_err;
   }
   else
   	++st.tun_arp_ok;
}


/*--------------------------------------------------------------------------

	This function is responsible for handling IP packets.

	tun.u.ip contains latest IP packet.  Forward it over the Ethernet
   interface after rewriting the sender address.  We also need to
   rewrite the destination address according to our proxy ARP table
   and/or the router information.

--------------------------------------------------------------------------*/

void handle_tunnel_ip()
{
	ll_Gather g;
	BrEnt far * b;
   int bcast, mcast;
	longword dst = ntohl(tun.u.ip.hdr.destination);
	longword src = ntohl(tun.u.ip.hdr.source);

#ifdef BRIDGE_VERBOSE
	if (debug_on > 2)
		printf("Got tunnelled IP (ip pkt len=%u)\n", ntohs(tun.u.ip.hdr.length));
#endif
	bcast = mcast = 0;

	if (!dst || !src ||
			IS_ANY_BCAST_ADDR(dst) ||
			IS_ANY_NET_ADDR(dst)) {
		bcast = 1;
	}
	else if (IS_MULTICAST_ADDR(dst)) {
		mcast = 1;
	}

   // Rewrite sender address in the Ethernet header.  Strictly, this should not
   // be necessary (since hosts should use ARP to resolve hardware
   // addresses) but do it in case hosts make use of the address.
   memcpy(tun.eth.src, my_eth_addr[IF_ETH0], 6);

   // Rewrite (unicast) dest according to lookup
   if (!bcast && !mcast) {
	   b = findIP(dst);
	   if (b) {
	      ++st.ip4_br;
	      _f_memcpy(tun.eth.dest, b->hwa, 6);
	   }
	   else {
      	// Not found directly, requires router...
	      ++st.ip4_rout;
	      if (!eth_router.ip || !(b = findIP(eth_router.ip))) {
	         ++st.ip4_rout_err;
	#ifdef BRIDGE_VERBOSE
				if (debug_on)
		         printf("BIP: not in table, dropped (no router)\n");
	#endif
	         return;
	      }

	      _f_memcpy(tun.eth.dest, b->hwa, 6);
	#ifdef BRIDGE_VERBOSE
	      if (debug_on > 1)
	         printf("BIP: not in table, use router\n");
	#endif
	   }
   }

   memset(&g, 0, sizeof(g));
   g.iface = IF_ETH0;
   g.len1 = 14 + ntohs(tun.u.ip.hdr.length);
   g.data1 = (char __far *)&tun;
   if (pkt_gather(&g)) {
#ifdef BRIDGE_VERBOSE
		printf("Tunnelled IP: could not forward to Eth.\n");
#endif
   	++st.tun_ip4_err;
   	if (bcast)
   		++st.tun_bcast_err;
   	else if (mcast)
   		++st.tun_mcast_err;
   }
   else {
   	++st.tun_ip4_ok;
   	if (bcast)
   		++st.tun_bcast_ok;
   	else if (mcast)
   		++st.tun_mcast_ok;
   }

}


/*--------------------------------------------------------------------------

	This function is only used when using TLS.  It returns the appropriate
   pre-shared key to the library.

--------------------------------------------------------------------------*/
#ifdef USE_TLS
int getPSK(ssl_Socket far * state,
   						const void __far * hint, size_t hint_length,
   						void __far * identity, size_t * identity_length,
                     void __far * key, size_t * key_length)
{
	// Set the key and its length.  Obviously, for a real application
   // this has to be unique and known only to the two ends of the
   // tunnel!
	_f_memset(key, 0x66, SSL_MAX_PSK);
   *key_length = SSL_MAX_PSK;

   if (state->is_client) {
	#ifdef BRIDGE_VERBOSE
   	printf("Client setting key identity 'KI'\n");
	#endif
   	// This is also somewhat arbitrary, but since we are implementing
      // a point-to-point link between two known entities, a fixed
      // key identifier is just fine.
	   _f_strcpy(identity, "KI");
	   *identity_length = 2;
   }
   else {
	#ifdef BRIDGE_VERBOSE
   	printf("Server got key identity '%.*ls'\n",
      	*identity_length, identity);
	#endif
   	// Should be same as set above (for the client case).
   }
   return 0;
}
#endif


/****************************************************************************
*****************************************************************************

	This function is the main tunnelling state machine.

*****************************************************************************
****************************************************************************/

void run_tunnel(void)
{
	int rc;

	switch (bst) {
   default:
   	break;
	case ANNOUNCE:	// Link available, first phase of announce/discovery (AD)
		udp_extopen(&adp_sock, IF_WIFI0, PORT, -1, 0,
						adp_handler, 1, 0);
	   adp.magic = MAGIC;
	   memcpy(adp.my_hwa, my_eth_addr[IF_WIFI0], sizeof(adp.my_hwa));
      adp.my_ip = _if_tab[IF_WIFI0].ipaddr;
      if (!adp_retry) {
		   memset(adp.your_hwa, 0, sizeof(adp.your_hwa));
	      adp.your_ip = 0;
      }
      if (++adp_retry == 6)
      	adp_retry = 0;
      send_adp();
   	adp_timer = _SET_SHORT_TIMEOUT(1000);
      bst = WAIT_ACK;
		break;
   case WAIT_ACK:			// Wait for AD packet with ACK
		if (_CHK_SHORT_TIMEOUT(adp_timer)) {
      	// Nothing received while in this state, go back to previous state
			bst = ANNOUNCE;
      }
      break;
   case LISTENING:		// Listening TCP socket (higher MAC address)
		if (_CHK_SHORT_TIMEOUT(adp_timer)) {
      	// Peer did not open to us - maybe it powered down.
         // Go back to initial state to be ready when it comes up again.
#ifdef BRIDGE_VERBOSE
			printf("Timed out establishing tunnel (server)\n");
#endif
         sock_abort(sock);
			bst = ANNOUNCE;
      }
      else if (!sock_waiting(sock)) {
      	if (sock_established(sock)) {
#ifdef BRIDGE_VERBOSE
				printf("Established tunnel (server, plaintext)\n");
#endif
	         // Send keepalives, otherwise one end may not realize the other
	         // end rebooted.
	         tcp_keepalive(sock, 15);
#ifdef USE_TLS
	         rc = sock_secure(&tcp_sock, &ssl_sock,
	                      BUFSIZE, app_rxbuf,
	                      BUFSIZE, app_txbuf,
	                      0,   // Is a server
	                      SSL_F_DISABLE_SSLV3 | SSL_F_NO_RESUME | SSL_S_ALLOW_PSK,
	                      0, 0, 0, 0,
	                      NULL,
	                      NULL,
	                      NULL
	                      );
	         if (!rc) {
            	// Set PSK callback function
					ssl_sock->getPSK = getPSK;
	            sock = ssl_sock;
		         bst = SECURING;
	         }
	         else {
            	// Server should always initialize.  Else must be a memory
               // allocation issue, so exit program.
	#ifdef BRIDGE_VERBOSE
	         	printf("Failed to initialize TLS server\n");
	#endif
	            exit(4);
	         }
#else
				bst = TUNNEL;
#endif
			}
         else if (!sock_alive(sock)) {
#ifdef BRIDGE_VERBOSE
				printf("Socket DOA\n");
#endif
				bst = ANNOUNCE;
         }
         else {
         	sock_abort(sock);
#ifdef BRIDGE_VERBOSE
				printf("Socket established, but closed\n");
#endif
				bst = ANNOUNCE;
         }
      }
      break;
   case OPENWAIT:			// Waiting short time to open TCP socket (lower MAC address)
		if (_CHK_SHORT_TIMEOUT(adp_timer)) {
      	// Short timeout before attempting active open has expired.
#ifdef BRIDGE_VERBOSE
			printf("Opening (as client)\n");
#endif
			rc = tcp_extopen(sock, IF_WIFI0, PORT, adp.your_ip, PORT, NULL, 0, 0);
         if (!rc)
         	exit(2);
         set_sockopts(sock);
			bst = OPENING;
   		adp_timer = _SET_SHORT_TIMEOUT(7000);
      }
      break;
	case OPENING:			// Opening TCP socket (lower MAC address)
		if (_CHK_SHORT_TIMEOUT(adp_timer)) {
      	// Failed to connect within timeout.
#ifdef BRIDGE_VERBOSE
			printf("Timed out establishing tunnel (client)\n");
#endif
         sock_abort(sock);
			bst = ANNOUNCE;
      }
   	else if (sock_established(sock)) {
#ifdef BRIDGE_VERBOSE
			printf("Established tunnel (client, plaintext)\n");
#endif
         // Send keepalives, otherwise one end may not realize the other
         // end rebooted.
         tcp_keepalive(sock, 15);
#ifdef USE_TLS
         rc = sock_secure(&tcp_sock, &ssl_sock,
                      BUFSIZE, app_rxbuf,
                      BUFSIZE, app_txbuf,
                      1,   // Is a client
                      SSL_F_DISABLE_SSLV3 | SSL_F_NO_RESUME | SSL_S_ALLOW_PSK,
                      0, 0, 0, 0,
                      NULL,
                      NULL,
                      NULL
                      );
         if (!rc) {
            // Set PSK callback function
            ssl_sock->getPSK = getPSK;
            sock = ssl_sock;
            bst = SECURING;
         }
         else {
            // Client should always initialize.  Else must be a memory
            // allocation issue, so exit program.
	#ifdef BRIDGE_VERBOSE
	         printf("Failed to initialize TLS client\n");
	#endif
            exit(4);
         }
#else
			bst = TUNNEL;
#endif
		}
		break;

#ifdef USE_TLS
	case SECURING:
     	if (sock_established(sock)) {
#ifdef BRIDGE_VERBOSE
			printf("Established tunnel (encrypted)\n");
#endif
			bst = TUNNEL;
		}
      else if (!sock_alive(sock)) {
#ifdef BRIDGE_VERBOSE
			printf("Failed to encrypt tunnel\n");
#endif
			bst = ANNOUNCE;
      }
   	break;
#endif

	case TUNNEL:			// Established tunnel, between received packets,
   							// waiting for Ethernet header.
   	if (sock_readable(sock) > 14) {
      	// Got Eth header
         sock_fastread(sock, &tun.eth, sizeof(tun.eth));
         if (tun.eth.type == ARP_TYPE)
         	bst = TUNNEL_ARP;
         else
         	bst = TUNNEL_IP;
      }
      break;
	case TUNNEL_ARP:		// Waiting for ARP packet
   	if (sock_readable(sock) > sizeof(arp_Header)) {
      	// OK, extract complete ARP packet (and Eth header)
         sock_fastread(sock, &tun.u.arp, sizeof(tun.u.arp));
         handle_tunnel_arp();
         bst = TUNNEL;
		}
      break;
	case TUNNEL_IP:		// Waiting for IPv4 header
   	if (sock_readable(sock) > sizeof(in_Header)) {
         sock_fastread(sock, &tun.u.ip.hdr, sizeof(tun.u.ip.hdr));
         ipdata_len = ntohs(tun.u.ip.hdr.length) - sizeof(tun.u.ip.hdr);
         bst = TUNNEL_IPDATA;
		}
      break;
	case TUNNEL_IPDATA:	// Waiting for remainder of IPv4 packet
   	if (sock_readable(sock) > ipdata_len) {
         sock_fastread(sock, &tun.u.ip.payload, ipdata_len);
         handle_tunnel_ip();
         bst = TUNNEL;
      }
   	break;
   }

   if (bst >= TUNNEL && (!sock_alive(sock) || !sock_readable(sock) || !sock_writable(sock))) {
#ifdef BRIDGE_VERBOSE
		printf("Tunnel socket failed\n");
#endif
   	sock_abort(sock);
      bst = ANNOUNCE;
   }

#ifdef USE_TLS
   if (bst <= ANNOUNCE && ssl_sock) {
   	// Need to clean up resources used by TLS
      sock_unsecure(ssl_sock);
      ssl_sock = NULL;
      sock = &tcp_sock;
   }
#endif
}


/****************************************************************************
*****************************************************************************

	Routines for test packet handling and RSSI indicator.

*****************************************************************************
****************************************************************************/

void do_send_test(void)
{
	// This is called only when WiFi interface is 'up'.  It sends an ARP
   // packet for the (known) peer address, thus should always elicit
   // a reply.
   // _arp_request() is an internal function.  Normally, ARP should be
   // handled via the API, but this is a special case.
	_arp_request(adp.your_ip, adp.my_ip, IF_WIFI0);
}



/****************************************************************************
*****************************************************************************

	Main application loop.

*****************************************************************************
****************************************************************************/

void main()
{
	int i, c;
	char ip[16], netmask[16];
	longword tmo;
	word iface, wpend, epend, prev_wpend, prev_epend, mask;
	int err;
	word led_timer, led_phase;
	unsigned long led_state;
#ifdef BRIDGE_VERBOSE
	int macstatus = -1;
#endif

	brdInit();
	boot_time = SEC_TIMER;

   // If switch *not* pressed (i.e. reads high), then long_haul assumed true.
   long_haul = BitRdPortI(PDDR, 1);

   // Set up PE7 as PWM output.  PE7 may be configured as PWM3 (alt out 2)
   WrPortI(PEAHR, &PEAHRShadow, 0x80);		// PE7 Alt out 2
   WrPortI(PEFR, &PEFRShadow, PEFRShadow | 0x80);	// PE7 alternate function
   WrPortI(PEDDR, &PEDDRShadow, PEDDRShadow | 0x80);	// Output
	WrPortI(PWL3R, &PWL3RShadow, 0);		// Normal PWM
   WrPortI(TAT9R, &TAT9RShadow, 1);		// High freq output



	clear_stats();

	debug_on = 0;

	// Init table to all unused (but keep valid old entries)
   init_table(0);

	// Start network and wait for interface to come up (or error exit).
	printf("MAC: %02X:%02X:%02X\n",
		SysIDBlock.macAddr[3],SysIDBlock.macAddr[4],SysIDBlock.macAddr[5]);


	if (sock_init()) {
		printf("sock_init() failed\n");
		exit(-NETERR_IFDOWN);
	}

	// Set up network.  Ethernet is set to dummy static addresses.
   ifconfig( IF_ETH0,
	   	IFS_IPADDR, IPADDR(0,0,0,1),
	   	IFS_NETMASK, IPADDR(255,255,255,255),
	   	IFS_ROUTER_SET, IPADDR(0,0,0,0),
	      IFS_END);


	prev_wpend = IF_DOWN;
	prev_epend = IF_DOWN;

	ifup(IF_ETH0);
	create_ibss();

	// This sample allows some keyboard control.  If running without a debug
   // connection, then these are simply ignored.
	printf("Press following keys:\n");
	printf("  t       print interface status and bridging table\n");
	printf("  m       print WiFi MAC status\n");
	printf("  p       print statistics\n");
	printf("  P       print and clear statistics\n");
	printf("  r       force bridging table re-organization\n");
	printf("  e       restart Ethernet interface\n");
	printf("  f       toggle 'fixed Tx rate' flag\n");
	printf("  <>      decrease/increase Tx rate (and set fixed)\n");
	printf("  []      decrease/increase Tx power\n");
	printf("  ,.      decrease/increase verbose level\n");

   //debug_on = 4;

	led_timer = _SET_SHORT_TIMEOUT(63);
	led_phase = 0;
	led_state = 0;

   for (;;) {
#ifdef BRIDGE_VERBOSE
   	if (kbhit()) {
   		c = getchar();
   		if (c == 't') {
   			ip_print_ifs();
   			router_printall();
				arpcache_printall();
				bridge_printall();
   		}
   		else if (c == 'm') {
		      print_wifi();
   		}
   		else if (c == 'e') {
   			printf("Ethernet restart...\n");
		      ifdown(IF_ETH0);
   		}
			else if (c == 'p' || c == 'P') {
				print_stats();
				if (c == 'P')
					clear_stats();

			}
			else if (c == 'r') {
				reorg_table();
				printf("Done reorg, now %u entries.\n", br_used);
			}
			else if (c == 'f') {
				_wifi_macParams.options ^= WLN_OPT_FIXEDRATE;
				printf("Fixed tx rate now %s.\n",
            	_wifi_macParams.options & WLN_OPT_FIXEDRATE ? "ON" : "OFF");
			}
         else if (c == '<') {
         	if (_wifi_rateTx > 0)
            	--_wifi_rateTx;
				_wifi_macParams.options |= WLN_OPT_FIXEDRATE;
            printf("Rate now %4.1f\n", _wifi_rateInfo[_wifi_rateTx].bps * 0.5);
         }
         else if (c == '>') {
         	if (_wifi_rateTx < _WIFI_RATE_NUM-1)
            	++_wifi_rateTx;
				_wifi_macParams.options |= WLN_OPT_FIXEDRATE;
            printf("Rate now %4.1f\n", _wifi_rateInfo[_wifi_rateTx].bps * 0.5);
         }
         else if (c == '[') {
         	if (_wifi_macStatus.tx_power > 0)
         	ifconfig(IF_WIFI0, IFS_WIFI_TX_POWER, _wifi_macStatus.tx_power - 1, IFS_END);
            printf("Tx power now %d\n", _wifi_macStatus.tx_power);
         }
         else if (c == ']') {
         	if (_wifi_macStatus.tx_power < 15)
         	ifconfig(IF_WIFI0, IFS_WIFI_TX_POWER, _wifi_macStatus.tx_power + 1, IFS_END);
            printf("Tx power now %d\n", _wifi_macStatus.tx_power);
         }
			else if (c == ',') {
         	if (debug_on)
         		--debug_on;
            printf("Debug now %d\n", debug_on);
         }
			else if (c == '.') {
         	++debug_on;
            printf("Debug now %d\n", debug_on);
         }
   	}
#endif

		tcp_tick(NULL);

		wpend = ifpending(IF_WIFI0);
		epend = ifpending(IF_ETH0);

	   if (wpend == IF_DOWN)
	   	ifup(IF_WIFI0);

	   if (epend == IF_DOWN)
	   	ifup(IF_ETH0);

#ifdef BRIDGE_VERBOSE
	   if (prev_epend != epend) {
	   	printf("Ethernet now %s\n", pend_str(epend));
	   	prev_epend = epend;
	   	if (epend == IF_UP)
				ip_print_ifs();
	   }
	   if (prev_wpend != wpend) {
	   	printf("WiFi now %s\n", pend_str(wpend));
	   	prev_wpend = wpend;
	   	if (wpend == IF_UP) {
	         ip_print_ifs();
	         print_wifi();
	      }
	   }
#endif

      if (wpend == IF_UP) {
         if (bst == DOWN) {
#ifdef BRIDGE_VERBOSE
            printf("Tunnel establishment...\n");
#endif
            start_tunnel();
         }
      }
      else {
         if (bst != DOWN) {
#ifdef BRIDGE_VERBOSE
            printf("Tunnel OFF\n");
#endif
	         bst = DOWN;
         }
      }

     	run_tunnel();

#ifdef BRIDGE_VERBOSE
      if (_wifi_macStatus.state != macstatus) {
      	macstatus = _wifi_macStatus.state;
      	printf("MAC status now %s\n", wifi_state_str(macstatus));
      }
#endif

      // If S1 depressed (low input), then issue 'ping' (actually, ARP)
      // packets to peer.  This helps determine radio link quality.
      send_test = !BitRdPortI(PDDR, 1);

		// Drive the DS1 LED
		if (_CHK_SHORT_TIMEOUT(led_timer)) {
			led_timer = _SET_SHORT_TIMEOUT(63);
			++led_phase;
			led_phase &= 0x1F;
         led_state = 0;
			if (wpend == IF_UP) {
	         if (send_test) {
            	if (test_reply) {
               	// LED on for this interval if received reply
						led_state = 0xFFFFFFFFuL;
                  test_reply = 0;
               }
					do_send_test();
            }
         	else switch (bst) {
	            default:
	               break;
	            case WAIT_ACK:
	               // Short flash 1Hz while negotiating tunnel
	               led_state = 0x00010001uL;
	               break;
	            case LISTENING:
	               // Long flash 1Hz when server waiting for client
	               led_state = 0x0FFF0FFFuL;
	               break;
	            case OPENWAIT:
	            case OPENING:
	               // Flash 2Hz when client opening to server
	               led_state = 0x0F0F0F0FuL;
	               break;
					#ifdef USE_TLS
	            case SECURING:
	               // Flash 8Hz when establishing TLS
	               led_state = 0x55555555uL;
	               break;
					#endif
	            case TUNNEL:
	            case TUNNEL_ARP:
	            case TUNNEL_IP:
	            case TUNNEL_IPDATA:
	               led_state = 0xFFFFFFFFuL;  // solid ON when tunnelling
	               if (epend == IF_UP)
	                  housekeeping();
	               break;
            }
         }

			BitWrPortI(PDDR, &PDDRShadow, (led_state & (1uL<<led_phase)) == 0, 0);
		}
   }
}
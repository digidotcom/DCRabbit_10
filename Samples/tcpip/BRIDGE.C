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
/* BRIDGE.C

   Demonstrate Ethernet to Wifi bridge.  A bridge makes two LANs appear to
   be a single one, at least at the network and higher layers.

   Requires an RCM66XXW board (currently, the only Rabbit module which
   has both Ethernet and WiFi).

	Basically, this acts as a proxy ARP between the Ethernet and Wifi
	network segments.  This board also has its own IP address (the
	same one on both interfaces) and is thus addressable as an ordinary
	network host.  In particular it uses DHCP to configure itself,
	and also supports ADDP and optional HTTP server.  If there is no
	DHCP server on either interface, then it uses LINKLOCAL procedures
	to assign itself an IP address in the 169.254/16 address range.

	Proxy ARP works for IP only, since the proxy handler needs to look
	inside IP headers to perform the appropriate hardware address
	translation.  Other protocols could be added given an understanding
	of their headers.

	Proxy ARP works by having the device answer ARP requests on each
	interface, putting itself as the hardware address for each destination
	IP on the other interface.  On receipt of
	each unicast IP packet, it looks up the real hardware address on
	the other interface, rewrites the ethernet header, and forwards
	the packet on the other interface.

	Broadcasts generally will not work if simply forwarded, since many
	protocols using broadcast or multicast rely on hardware addresses,
	and thus header rewriting will cause confusion.  If we know about
	specific protocols, then they can be successfully bridged.  This
	sample understands DHCP/BOOTP (broadcast) and ADDP (multicast) so
	devices can be automatically configured and discovered transparently.
	All other broadcast or multicast is forwarded in the hope that it
	will "work", but there are no guarantees.

	Routing poses special problems.  When a host wants to send to a
	host not on the subnet, it sends the packet to the next hop router
	instead.  The router hardware address is in the ethernet header,
	but the real destination host address is in the IP header.  There is
	no ARP exchange, thus the bridge device has to know which router to
	use for non-local IP addresses.  For this, it needs to understand the
	network topology and behave in some respects as a router itself,
	although an invisible one.

	In order to gain the necessary information for non-local packet
	handling, the bridge needs to obtain subnet masks and router
	addresses.  For this, it requires a DHCP server on exactly one
	of its interfaces.  (If a DHCP server exists on both interfaces,
	then they had both better agree.  In any case, only one is
	arbitrarily chosen.)  Since the bridge needs to select the actual
	next hop router, it uses its normal mechanism for this (i.e. the
	router table and redirect logic).

   NOTE: you can run this with a single RCM6600W (connecting to an
   access point using the customary WiFi configuration options) or you
   can use ad-hoc WiFi to connect to another RCM6600W running the same
   code as this one.  In the latter case, you are basically setting up
   a point-to-point bridge between two Ethernet segments.  Unfortunately,
   ad-hoc WiFi networking is not secure so it is recommend that you
   examine the companion sample to this one, bridge_p2p.c, for a solution
   to this problem.  If you are simply bridging an Ethernet segment to
   an existing infrastructure (via an access point) then adequate security
   is obtained by specifying WPA-2 by defining WIFI_USE_WPA and
   WIFI_AES_ENABLED.

   Expected throughput is about 4-6 megabytes per second (total for both
   directions).  Bottleneck for throughput appears to be transferring of data
   to and from the WiFi driver hardware, plus checksumming of IP traffic.
*/
#define SET_FACTORY	// If defined, override stored config and set to defaults
#define USE_HTTP		// If defined, include HTTP server.  For simplicity,
							// we only show some static pages.  See the HTTP samples
                     // for adding more features.

// Dynamic configuration.  We use DHCP and fall back to linklocal.
#define TCPCONFIG 6
#define NETCONFIG_OFFSET 0		// Offset at which to store config (768 bytes)
										// in userID block

/* Set the default WiFi scan, probe and ad-hoc flags.
   Do all tricks:
   . Initially, try passive scanning and match the best out of the list below.
   . Then, try actively probing.
   . Finally, create an ad-hoc (IBSS) network.
*/
#define USE_WIFI_BRIDGE_DFLT_CONFIG_FLAGS \
                            (CF_PASSIVE_SCAN | CF_ACTIVE_PROBE | CF_CREATE_IBSS)

/* Set some default SSIDs.
   The idea of this is to set up a list of known SSIDs, with each
   enabled entry going into config->wifi[0], config->wifi[1], etc., in
   turn.  In a real application, this list would be initially set up
   via a non-Wifi interface.  This list is only used when the user ID
   block is uninitialized; thereafter, it is read from the user block
   on startup.

   Entries with the ibss_create_chan field non-zero are special.
   They indicate entries where it is possible to create an ad-hoc
   network.  This is most useful for when a Wifi link is being
   created to join two ethernet segments into one logical ethernet
   segment.  In this case, the Wifi network does not have to
   correspond to any pre-existing SSID, so a mutually agreed ad-hoc
   network is most appropriate.

   At most 2 entries with non-zero ibss_create_chan field should be
   defined.  The first entry is the one which will normally be chosen
   to create the ad-hoc network.  The last entry is used only if the
   S1 switch (on the dev board) is held when rebooting.  S1 can be
   useful when initially establishing a radio link.  It forces the
   device to always start the ad-hoc network, in order to eliminate
   any possible race condition with the other end.

   If there is only one entry with ibss_create_chan non-zero, then that
   entry is both the first and last so the above considerations still
   apply.  If there are no ad-hoc entries, then only a link to
   an existing infrastructure network can be established.  This is
   useful for simply connecting an isolated ethernet segment to
   an existing access point and its infrastructure network.

   Uncomment one or more of the USE_BRIDGE_WIFI_DFLT_CONFIG_x macro
   definitions, below, to enable the respective known SSID entry or
   entries.  Edit each enabled entry's example .*_SSID, .*_ENCR, .*_KEY
   and .*_CHAN macro values to suit the local WiFi environment.
*/

#define USE_BRIDGE_WIFI_DFLT_CONFIG_A
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_A
	// Ensure the following two macros are defined to support infrastructure mode
	// with WPA2 CCMP security.
 #if !defined WIFI_USE_WPA
	#define WIFI_USE_WPA
 #endif
 #if !defined WIFI_AES_ENABLED
	#define WIFI_AES_ENABLED
 #endif
	#define BRIDGE_WIFI_DFLT_CONFIG_A_SSID "EtherFiBridge-AP-ccmp"
	#define BRIDGE_WIFI_DFLT_CONFIG_A_ENCR WLN_ENCR_CCMP
	// Given SSID "EtherFiBridge-AP-ccmp" and pass phrase "now is the time" then
	// the following binary key is suitable for WPA2 CCMP security.
	#define BRIDGE_WIFI_DFLT_CONFIG_A_KEY \
	           "0B9B4FB9C93D21C93F47511FBC4A1E87AD165541D421DADE10F6D40A2FF9FFE9"
	#define BRIDGE_WIFI_DFLT_CONFIG_A_CHAN 0
	// Ensure the IFC_WIFI_SSID macro is defined to quiet a compile time warning.
 #if !defined IFC_WIFI_SSID
	#define IFC_WIFI_SSID BRIDGE_WIFI_DFLT_CONFIG_A_SSID
 #endif
 #if !defined IFC_WIFI_WPA_PSK_HEXSTR
	#define IFC_WIFI_WPA_PSK_HEXSTR BRIDGE_WIFI_DFLT_CONFIG_A_KEY
 #endif
#endif

#define USE_BRIDGE_WIFI_DFLT_CONFIG_B
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_B
	// Ensure the following macro is defined to support infrastructure mode with
	// WPA TKIP security.
 #if !defined WIFI_USE_WPA
	#define WIFI_USE_WPA
 #endif
	#define BRIDGE_WIFI_DFLT_CONFIG_B_SSID "EtherFiBridge-AP-tkip"
	#define BRIDGE_WIFI_DFLT_CONFIG_B_ENCR WLN_ENCR_TKIP
	// Given SSID "EtherFiBridge-AP-tkip" and pass phrase "now is the time" then
	// the following binary key is suitable for WPA TKIP security.
	#define BRIDGE_WIFI_DFLT_CONFIG_B_KEY \
	           "08E6ABFA3A98020866D562D2599375332438C29C047B28D8EFBF789D2E692C7A"
	#define BRIDGE_WIFI_DFLT_CONFIG_B_CHAN 0
	// Ensure the IFC_WIFI_SSID macro is defined to quiet a compile time warning.
 #if !defined IFC_WIFI_SSID
	#define IFC_WIFI_SSID BRIDGE_WIFI_DFLT_CONFIG_B_SSID
 #endif
 #if !defined IFC_WIFI_WPA_PSK_HEXSTR
	#define IFC_WIFI_WPA_PSK_HEXSTR BRIDGE_WIFI_DFLT_CONFIG_B_KEY
 #endif
#endif

#define USE_BRIDGE_WIFI_DFLT_CONFIG_C
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_C
	// Note that the only supported security with ad-hoc is WEP.
	#define BRIDGE_WIFI_DFLT_CONFIG_C_SSID "EtherFiBridge-AdHoc-wep"
	#define BRIDGE_WIFI_DFLT_CONFIG_C_ENCR WLN_ENCR_WEP
	#define BRIDGE_WIFI_DFLT_CONFIG_C_KEY "00112233445566778899AABBCC"
	#define BRIDGE_WIFI_DFLT_CONFIG_C_CHAN 10
	// Ensure the IFC_WIFI_SSID macro is defined to quiet a compile time warning.
 #if !defined IFC_WIFI_SSID
	#define IFC_WIFI_SSID BRIDGE_WIFI_DFLT_CONFIG_C_SSID
 #endif
#endif

#define USE_BRIDGE_WIFI_DFLT_CONFIG_D
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_D
	#define BRIDGE_WIFI_DFLT_CONFIG_D_SSID "EtherFiBridge-AdHoc-open"
	#define BRIDGE_WIFI_DFLT_CONFIG_D_ENCR WLN_ENCR_OPEN
	#define BRIDGE_WIFI_DFLT_CONFIG_D_KEY  ""
	#define BRIDGE_WIFI_DFLT_CONFIG_D_CHAN 2
	// Ensure the IFC_WIFI_SSID macro is defined to quiet a compile time warning.
 #if !defined IFC_WIFI_SSID
	#define IFC_WIFI_SSID BRIDGE_WIFI_DFLT_CONFIG_D_SSID
 #endif
#endif

#define USE_BRIDGE_WIFI_DFLT_CONFIG_E
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_E
	#define BRIDGE_WIFI_DFLT_CONFIG_E_SSID "EtherFiBridge-AP-open"
	#define BRIDGE_WIFI_DFLT_CONFIG_E_ENCR WLN_ENCR_OPEN
	#define BRIDGE_WIFI_DFLT_CONFIG_E_KEY  ""
	#define BRIDGE_WIFI_DFLT_CONFIG_E_CHAN 0
	// Ensure the IFC_WIFI_SSID macro is defined to quiet a compile time warning.
 #if !defined IFC_WIFI_SSID
	#define IFC_WIFI_SSID BRIDGE_WIFI_DFLT_CONFIG_E_SSID
 #endif
#endif

#if !defined USE_BRIDGE_WIFI_DFLT_CONFIG_A && \
    !defined USE_BRIDGE_WIFI_DFLT_CONFIG_B && \
    !defined USE_BRIDGE_WIFI_DFLT_CONFIG_C && \
    !defined USE_BRIDGE_WIFI_DFLT_CONFIG_D && \
    !defined USE_BRIDGE_WIFI_DFLT_CONFIG_E
	#fatal "Must define one or more of the USE_BRIDGE_WIFI_DFLT_CONFIG_x macros."
#endif

#define _WIFI_SCAN_NUM			32		// Override default of 16 maximum scanned.
// Override scan intervals to at least 100ms per channel (default is 3*5)...
// (IBSS default beacon interval is 100ms, so we'd miss them with a short
// scan interval).
#define _WIFI_SCAN_PERIOD		5		// Scan period for each try (ms) - default 5
#define _WIFI_SCAN_LIMIT		24		// Number of periods per channel - default 3

#define IBSS_CREATE_TIMEOUT	10000	// ms to timeout IBSS creation
												// needs to be sufficient for linklocal
#define SCAN_TIMEOUT				6000	// ms for scan results timeout
												// needs to be at least 3 sec
#define DHCP_DEFAULT_TIMEOUT	16		// seconds to timeout DHCP (will fall back
												// to link-local address)
#define ASSOCIATE_TIMEOUT		(DHCP_DEFAULT_TIMEOUT * 1000 + 3000)
												// ms to timeout ifup() for wifi, must be a
												// few seconds longer than DHCP timeout
#if ASSOCIATE_TIMEOUT < 0 || ASSOCIATE_TIMEOUT > 32000
	// prevent an out of range (i.e. negative) ASSOCIATE_TIMEOUT value
	#fatal "The maximum allowed value for ASSOCIATE_TIMEOUT is 32000 ms."
#endif
#define ADDP_NAME					"EtherFiBridge"
#define ADDP_PASSWORD			"rabbit"
#define ADDP_CALLBACK_IF(iface,ip,mask,gw) addp_callback_if(iface,ip,mask,gw)
#define MAX_UDP_SOCKET_BUFFERS	2
#define USE_MULTICAST
#define USE_LINKLOCAL
#define USE_DHCP

#define BRIDGE_VERBOSE		// This sample
//#define ARP_VERBOSE
//#define ICMP_VERBOSE
//#define ZC_LL_VERBOSE		// Link-local verbose
//#define BOOTP_VERBOSE
//#define IP_VERBOSE
//#define ADDP_VERBOSE

//#define DCRTCP_DEBUG
//#define WIFIG_DEBUG
//#define ADDP_DEBUG

// Required definitions, since we need to do some unusual stuff...
#define CUSTOM_ARP_HANDLER  bridge_arp_handler
#define CUSTOM_IP4_HANDLER  bridge_ip4_handler

// Required definition to have more liberal ARP rules.  This allows us to
// refresh our ARP table even if our interface was not completely configured
// (e.g. it was on the "wrong side" of the DHCP server).
#define _ARP_IGNORE_INVALID_SOURCE_IP

#use "rcm66xxw.lib"
#use "idblock_api.lib"
#use dcrtcp.lib
#use "addp.lib"
#use "crc32.lib"

#ifdef USE_HTTP
#use "http.lib"
#ximport "samples/tcpip/http/pages/static.html"    index_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"    rabbit1_gif
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF)
SSPEC_MIMETABLE_END
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif)
SSPEC_RESOURCETABLE_END
#endif


/*
 * Data structure used to store current network configuration.  The <reserved>
 * element exists to allow for future versions of this program to add more
 * data to the configuration while maintaining backward compatability with the
 * old structure.
 */
#define N_SSID	5
typedef struct {
/*
SSID selection policy:

Picks one with best RSSI, that matches one of the entries in the
configuration SSID table.  If none exist, selects first one with
ibss_create_chan non-zero, and creates a new IBSS.  If none, then waits forever.

*/
	char	ssid_len;			// Length of following SSID (0 for blank entry)
	char	ssid[32];
	char	encr;					// One of
									// WLN_ENCR_OPEN,WLN_ENCR_WEP,WLN_ENCR_TKIP,
                           // WLN_ENCR_CCMP.
                           // Note: WLN_ENCR_TKIP, WLN_ENCR_CCMP are only
                           // supported for infrastructure i.e. one end has
                           // to be an access point.
	char	key[32];				// Length of key is 0, 10, 22 or 32 depending on
									// encr field.  WEP keys are ascii HEX, null term
									// in order to distinguish WEP-40 and WEP-104.
									// PSK keys are binary
	char	chan;					// If not zero, use on this channel only
	char	ibss_create_chan;	// If not zero, allow creation of IBSS.
	char	resv[28];			// bring up to 96
} wifi_t;

typedef struct {
	struct {
	   unsigned long ip;       // Static IP (or 0 for DHCP)
	   unsigned long netmask;  // Static netmask
	   unsigned long gateway;  // Static router
	} eth_static, wifi_static;
	unsigned flags;		// Flags as follows
	// At least one of the following 3 flags must be set.  If all are
	// zero, the WiFi interface is not started, and the device must
	// be configured via the Ethernet interface.

	// Passive scan will initially scan all channels for SSIDs which we
	// know about (i.e. configured in wifi list below).  This allows
	// automatic connection to the "best" (i.e. highest signal strength)
	// peer.  If turned off (0), then we start actively probing and/or
	// start/join an IBSS.
	#define CF_PASSIVE_SCAN	0x0001
	// Active probe will try associating with each SSID in turn until
	// the link comes up, or the list is exhausted.  In the latter case,
	// it proceeds to creating an IBSS, or going back to passive scan.
	#define CF_ACTIVE_PROBE	0x0002
	// Create IBSS if set.  The first entry in the wifi table which has a
	// non-zero ibss_create_chan field will be used.  This option is
	// only used if passive and active scans did not succeed.
	#define CF_CREATE_IBSS	0x0004

	//
	unsigned flags2;
	wifi_t	wifi[N_SSID];	// List of SSIDs (if ssid_len != 0)
	char		reserved[768-32-sizeof(wifi_t)*N_SSID];		// room for more stuff
	unsigned long crc32;
} config_t;

config_t config;

unsigned boot_mode;
#define BOOT_LAST_IBSS 0x0001		// Force create/join of last IBSS configured



// Bridging status (per interface)
enum {
	DOWN = 0,		// Interface down (must be value 0)
	UP = 1,
	WITH_DHCP = 2,
	BRIDGING = 4,

	DHCP_NB = UP | WITH_DHCP,				// Up with DHCP, but not bridging
	DHCP = UP | WITH_DHCP | BRIDGING,	// Up with DHCP, and bridging
	NO_DHCP_NB = UP,							// Up without DHCP, and not bridging
	NO_DHCP = UP | BRIDGING					// Up without DHCP, and bridging
};

// Bridging status (per interface)
int bst[IF_MAX] = { DOWN, };


// Assuming there are only two interfaces, this macro returns "the other one".
#define IF_OTHER(iface) ((iface) == IF_ETH0 ? IF_WIFI0 : IF_ETH0)

// This is the database entry which allows mapping IP addresses to hardware
// addresses.  It is updated by looking at ARP packets, and used to
// bridge IP packets across the interfaces.
// Entries are indexed based on hash of the IP address.  The hash is simply
// the IP address as a byte-reversed number, modulo the table size.  E.g.
// for 10.10.6.100, the byte reversed number is 0x64060A0A, then modulo 511
// gives 0x1D.  A hash collision is resolved by adding 1 until a free table
// entry is located.  It's best if the table doesn't get more than 3/4 full.
typedef struct {
	longword ip;		// IP address
	byte	hwa[6];		// Hardware address
	int	iface;		// Interface (IF_ETH0 or IF_WIFI0) with single hop to
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

#define BR_TABSIZE	509	// Bridging table number of entries - should be prime
									// number to get optimum hashing.
word	br_used = 0;			// Number of entries currently used.  Must never
									// get equal to BR_TABSIZE, otherwise a search
									// would not terminate.
long br_reorg_count = 0;	// Number of times bridging table re-organized.
longword	boot_time;			// Timestamp when last booted
BrEnt far bt[BR_TABSIZE];	// Bridging table.
BrEnt far btx[BR_TABSIZE];	// Bridging table copy used when re-organizing.

// Statistics per interface:
typedef struct {
	// ARP packets received:
	longword		arp_req;		// Bridged requests
	longword		arp_req_unconfig;	// out of the above, those which had an
											// unconfigured source IP (0 or -1).
	longword		arp_rep;		// Bridged replies
	longword		arp_rep_notfnd;	// out of the above, those whose source IP
											// was not found in our bridging table.
	longword		arp_other;	// Any other than request or reply
	longword		arp_bad;		// Invalid format
	longword		arp_me;		// Processed normally (for me)
	longword		arp_nb;		// Non-bridging mode
	// ARP forwarding
	longword		arp_fwd_err;// Forwarding packet dropped
	longword		arp_fwd_ok;	// Forwarded OK

	// IP packets received:
	longword		ip4_me;		// Processed normally (for me)
	longword		ip4_nb;		// Non-bridging mode, or was a destination address
									// type that we can't forward.
	longword		ip4_br;		// Successfully bridged (had cached HWA)
	longword		ip4_rout;	// Needed to route, not in bridging table
	longword		ip4_rout_err;	// Out of the above, those which could not be
										// routed because router HWA not known
	// IP forwarding
	longword		ip4_fwd_err;// Forwarding packet dropped
	longword		bcast_fwd_err;	// Out of above, those which were broadcast
	longword		mcast_fwd_err;	// Out of above, those which were multicast
	longword		ip4_fwd_ok;	// Forwarded OK
	longword		bcast_fwd_ok;	// Out of above, those which were broadcast
	longword		mcast_fwd_ok;	// Out of above, those which were multicast

	// DHCP and other types of broadcast or multicast forwarding
	longword		dhcp_bcast;	// DHCP broadcast dest packets received
	longword		bcast;		// Other broadcast dest packets received
	longword		addp_mcast;	// ADDP multicast dest packets received
	longword		mcast;		// Other multicast dest packets received
} Stats;

Stats st[IF_MAX];

int entry_free(word i)
{
	return bt[i].state == BTS_UNUSED;
}

void update_crc(BrEnt far * b)
{
	b->crc = crc32_calc(b, 14, 0);
}

int crc_ok(const BrEnt far * b)
{
	return b->crc == crc32_calc(b, 14, 0);
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

void bridge_printall(void);


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
   	// Issue ARP request for this entry
#ifdef BRIDGE_VERBOSE
		printf("BARP: ARP probe for old entry %08lX iface %d\n", bt[ent].ip, bt[ent].iface);
#endif
      _arp_request(bt[ent].ip, 0xFFFFFFFFuL, bt[ent].iface);
      bt[ent].state = BTS_CONFIRMING;
      bt[ent].arp_timer = _SET_SHORT_TIMEOUT(5000);
		return;
   case BTS_CONFIRMING:
   	if (_CHK_SHORT_TIMEOUT(bt[ent].arp_timer)) {
#ifdef BRIDGE_VERBOSE
			printf("BARP: ARP probe failed to re-locate %08lX iface %d\n", bt[ent].ip, bt[ent].iface);
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
}


/*
This function is responsible for handling incoming ARP packets.

It is modelled after the code in ARP.LIB, but adds the section at
end for maintaining the proxy/bridge table, and reforwarding the
ARP request on the other interface (after modifying the sender
hardware address).
*/

far ll_prefix *bridge_arp_handler(far ll_prefix *LL,
                                    byte *hdrbuf, word *pflags)

{
   auto arp_Header *in;
   auto ether_ll_hdr * ep;
	auto ll_Gather g;
   auto longword his_ip, dst_ip, new_ip;
   auto ATEntry *ate;
   auto ATHandle ath;
   auto word flags;
   auto int for_me = 0;
   auto word iface, other;
   auto int impersonate = 0;
	BrEnt far * b;
	Stats * s;

   iface = LL->iface;
	s = st + iface;

	*pflags = 0;
	if (!(bst[iface] & BRIDGING)) {
		++s->arp_nb;
_norm:
		// Normal processing
      if (for_me && !impersonate && in->opcode == ARP_REPLY) {
      	// A reply to an address I asked for, probably because refreshing
         // a table entry.
   		if (his_ip && (long)his_ip != -1L)
   			addIP(his_ip, (byte *)&in->srcEthAddr, iface);

      }
		*pflags |= CUSTOM_PKT_FLAG_PROCESS;	// Normal processing requested
		return LL;
	}


   if (LL->len < LL->net_offs + sizeof(arp_Header)) {
		++s->arp_bad;
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
		++s->arp_other;
   	goto _norm;
   }

   other = IF_OTHER(iface);


   his_ip = intel(in->srcIPAddr);
   dst_ip = intel(in->dstIPAddr);
   impersonate = IS_MY_ADDR(his_ip, iface);
   for_me = IS_MY_ADDR(dst_ip, iface);

	if (for_me || impersonate) {
		++s->arp_me;
		goto _norm;
	}

#ifdef BRIDGE_VERBOSE
	if (debug_on) {
	   if (in->opcode == ARP_REQUEST)
	      printf("BARP: who has %08lX? Tell %08lX  i/f %d\n", dst_ip, his_ip, iface);
	   else
	      printf("BARP: %08lX replying to %08lX  i/f %d\n", his_ip, dst_ip, iface);
	}
#endif

   ath = arpcache_search_iface(his_ip, 0, iface);
   if (ath > 0) {
   	// If the sender's IP address is in our cache, update the cache entry.
		ate = _arp_data + ATH2INDEX(ath);
		flags = ate->flags;
#ifdef BRIDGE_VERBOSE
		if (debug_on)
			printf("BARP: reloading because his IP address in cache\n");
#endif
		ath = arpcache_load(ath, (byte *)&in->srcEthAddr,
						(byte)iface, flags | ATE_RESOLVED, 0);
   }

   // Update bridging table, and forward over other interface after
   // changing the hardware address to ours.
   // For a query, we populate a (possibly new) table entriy for his_ip,
   // which can be fully set up, whereas the entry for dst_ip is not
   // known until the reply.  If his_ip is zero or -1 (i.e. unconfigured)
   // then we don't make an entry for this query.  Note that this works
   // for gratuitous ARP, in which case only the entry for the node is
   // made (there will normally be no replies).  The query is always
   // broadcast on the other interface.
   // When a reply is received, we create an entry for dst_ip.  If his_ip
   // is unconfigured, the reply is broadcast on the other interface,
   // otherwise it is unicast or broadcast as received.
  	ep->type = ARP_TYPE;
   if (in->opcode == ARP_REQUEST) {
		++s->arp_req;
   	if (his_ip && (long)his_ip != -1L)
   		addIP(his_ip, (byte *)&in->srcEthAddr, iface);
   	else
   		++s->arp_req_unconfig;
      memset(ep->dest, 0xFF, 6);	// Broadcast link-layer
   }
   else {
		++s->arp_rep;
   	addIP(his_ip, (byte *)&in->srcEthAddr, iface);
   	memset(&in->dstEthAddr, 0, sizeof(in->dstEthAddr)); // assume unknown
	   // If received unicast, send on unicast to original requester (if in table)
	   // else send on broadcast.
		if (!(LL->ll_flags & LL_BROADCAST)) {
			b = findIP(dst_ip);
			if (b) {
				_f_memcpy(ep->dest, b->hwa, 6);
			   _f_memcpy(&in->dstEthAddr, b->hwa, sizeof(in->dstEthAddr));
			}
			else {
				++s->arp_rep_notfnd;
      		memset(ep->dest, 0xFF, 6);
      	}
		}
		else
      	memset(ep->dest, 0xFF, 6);
   }
   // Overwrite sender HWA with ours - we are the proxy
   _f_memcpy(&in->srcEthAddr, my_eth_addr[other], sizeof(in->srcEthAddr));
   memcpy(ep->src, my_eth_addr[other], 6);
   memset(&g, 0, sizeof(g));
   g.iface = (byte)other;
   g.len1 = sizeof(arp_Header) + 14;
   g.data1 = (char __far *)ep;
   if (pkt_gather(&g))
   	++s->arp_fwd_err;
   else
   	++s->arp_fwd_ok;

	return LL;
}


udp_Header far * is_dhcp(far ll_prefix *LL, byte *hdrbuf)
{
/* Check if packet is DHCP, and we are currently bridging.
   NOTE: also perform side effect on DHCP packets, to turn on broadcast
   flag.  If we don't do this, then server will reply unicast to the
   address in chaddr field, but then we (the bridge) won't be able to
   correctly forward the packet.  We could manipulate chaddr, but it's
   much easier - if a bit cheesy - to force the broadcast bit on.
*/
	in_Header * ip;
   udp_Header far * up;
   DHCPPkt far * dp;
	int iface = LL->iface;
   word len, chk;
   word dstPort, srcPort;
   int isdhcp, modified;

   ip = (in_Header *)(hdrbuf + LL->net_offs);

   // Copy the UDP header to hdrbuf
   if (LL->len < LL->tport_offs + sizeof(udp_Header))
   	return NULL;	// Too short to contain UDP header

   up = (udp_Header far *)(LL->data1 + LL->tport_offs);

   len = intel16(up->length);

   dstPort = intel16(up->dstPort);
   srcPort = intel16(up->srcPort);

   isdhcp = dstPort == IPPORT_BOOTPS && srcPort == IPPORT_BOOTPC ||
            dstPort == IPPORT_BOOTPC && srcPort == IPPORT_BOOTPS;

   if (isdhcp) {
   	modified = 0;
   	dp = (DHCPPkt far *)(up + 1);
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


far ll_prefix *bridge_ip4_handler(far ll_prefix *LL,
                                    byte *hdrbuf, word *pflags, in_Header * ip)
{
   ether_ll_hdr * ep;
   udp_Header far * up;
	ll_Gather g;
	longword dst = intel(ip->destination);
	longword src = intel(ip->source);
	int iface = LL->iface;
	int other;				// The "other" interface we are bridging to
	int bcast;				// Flag set when forwarding broadcast.
	int mcast;				// Flag set when forwarding multicast.
  	ATHandle ath;
	BrEnt far * b;
	Stats * s = st + iface;
   unsigned len;
   unsigned iphdrlen;

	if (IS_MY_ADDR(dst, iface)) {
		++s->ip4_me;
		goto _norm;
	}

	if (!(bst[iface] & BRIDGING)) {
		// Not in bridging mode...
      ++s->ip4_nb;
_norm:
		// Set flag to process normally
      *pflags |= CUSTOM_PKT_FLAG_PROCESS;
      return LL;
	}

	other = IF_OTHER(iface);
	ep = (ether_ll_hdr *)ip - 1;
	bcast = mcast = 0;

	if (!dst || !src ||
			IS_ANY_BCAST_ADDR(dst) ||
			IS_ANY_NET_ADDR(dst)) {
		bcast = 1;
		// Check for broadcast DHCP packets i.e. UDP with ports IPPORT_BOOTPS and
		// IPPORT_BOOTPC...
		if ((up = is_dhcp(LL, hdrbuf))) {
			++s->dhcp_bcast;
#ifdef BRIDGE_VERBOSE
	      if (debug_on)
	         printf("BIP: DHCP found, %08lX(on %d)->%08lX\n", src, iface, dst);
#endif
		}
		else
			++s->bcast;
	}
	else if (IS_MULTICAST_ADDR(dst)) {
		mcast = 1;
		if (dst == ADDP_ADDR) {
			++s->addp_mcast;
#ifdef BRIDGE_VERBOSE
	      if (debug_on)
	         printf("BIP: ADDP found, %08lX(on %d)->%08lX\n", src, iface, dst);
#endif
		}
		else
			++s->mcast;
	}

	// It's something we can potentially forward
  	ep->type = IP_TYPE;

   // Get actual IP packet length.  LL->len may include extraneous data like
   // the ethernet CRC, so use the IP field instead.  Don't let it exceed
   // the real packet length, though.
   len = ntohs(ip->length);
   if (len > LL->len - LL->net_offs)
   	len = LL->len - LL->net_offs;

   iphdrlen = LL->tport_offs - LL->net_offs;

#ifdef BRIDGE_VERBOSE
	if (debug_on > 2)
		printf("BIP: %08lX(on %d)->%08lX proto=%u len=%u/%u\n",
      	src, iface, dst, ip->proto,
         len, LL->len - LL->net_offs);
#endif

	if (bcast) {
		// Handle broadcast forwarding.  Simply change source
		// HWA to us, and set all 1's dest.
		_f_memset(ep->dest, 0xFF, 6);
	}
	else if (mcast) {
		// Multicast; set HWA appropriately
		multicast_iptohw(ep->dest, dst);
	}
	else {
	   // Is dst in our bridging table?  If so, can simply rewrite the
	   // dst hardware address.  Otherwise, send to router on the other
	   // side.
	   b = findIP(dst);
	   if (b) {
	      ++s->ip4_br;
	      _f_memcpy(ep->dest, b->hwa, 6);
	#ifdef BRIDGE_VERBOSE
	      if (debug_on > 1)
	         printf("BIP: change dst hwa %02X:%02X:%02X\n", b->hwa[3], b->hwa[4], b->hwa[5]);
	#endif
	   }
	   else {
	      // Consult our usual ARP table; will probably return a router.
	      // If HWA not immediately available (cached), then must drop this
	      // packet since we don't have sufficient memory to buffer it.
	      ++s->ip4_rout;
	#ifdef BRIDGE_VERBOSE
	      if (debug_on > 1)
	         printf("BIP: not in table, arp resolve\n");
	#endif
	      ath = arpresolve_start_iface(dst, other);
	      if (ath <= 0) {
	         ++s->ip4_rout_err;
	         return LL;
	      }
	      ath = arpresolve_check(ath, dst);
	      if (ath <= 0) {
	         ++s->ip4_rout_err;
	         return LL;
	      }
	      arpcache_hwa(ath, ep->dest);
	   }

	}

   memcpy(ep->src, my_eth_addr[other], 6);
   memset(&g, 0, sizeof(g));
   g.iface = (byte)other;
   g.len1 = 14 + iphdrlen;
   g.data1 = (char __far *)ep;
   g.len2 = len - iphdrlen;
   g.data2 = (char __far *)(LL->data1 + LL->tport_offs);

   if (pkt_gather(&g)) {
   	++s->ip4_fwd_err;
   	if (bcast)
   		++s->bcast_fwd_err;
   	else if (mcast)
   		++s->mcast_fwd_err;
   }
   else {
   	++s->ip4_fwd_ok;
   	if (bcast)
   		++s->bcast_fwd_ok;
   	else if (mcast)
   		++s->mcast_fwd_ok;
   }
   if (bcast || mcast)
   	// We also need to process bcast/mcast normally, so our DHCP and ADDP
   	// work properly.
   	goto _norm;

	return LL;
}

void clear_stats(void)
{
	memset(st, 0, sizeof(st));
   memset(&_wifi_macStats, 0, sizeof(_wifi_macStats));
}


void print_stats(void)
{
	printf("\n%u out of %u bridging table entries used\n\n", br_used, BR_TABSIZE);
	printf("ETH0\t\tWIFI0\t\tDescr\n");
	printf("---------------\t---------------\t------------------\n");
#define PS(field, descr) printf("%10lu\t%10lu\t%s\n",  \
					st[IF_ETH0].field, st[IF_WIFI0].field, descr)
	PS(arp_me, "ARP for this device");
	PS(arp_nb, "ARP when non bridging mode");
	PS(arp_req, "bridged ARP requests");
	PS(arp_req_unconfig, "of above, with unconfigured source");
	PS(arp_rep, "bridged ARP replies");
	PS(arp_rep_notfnd, "of above, not in bridging table");
	PS(arp_other, "ARP not reply or request");
	PS(arp_bad, "ARP format errors");
	PS(arp_fwd_err, "ARP errors when forwarding");
	PS(arp_fwd_ok, "ARP forwarded OK");

	PS(ip4_me, "IPv4 for this device");
	PS(ip4_nb, "IPv4 when non bridging mode");
	PS(ip4_br, "IPv4 bridged");
	PS(ip4_rout, "IPv4 routed");
	PS(ip4_rout_err, "of above, router not resolved");
	PS(ip4_fwd_err, "(FERR) IPv4 errors when forwarding");
	PS(bcast_fwd_err, "of (FERR) above, with broadcast dest");
	PS(mcast_fwd_err, "of (FERR) above, with multicast dest");
	PS(ip4_fwd_ok, "(FOK) IPv4 forwarded OK");
	PS(bcast_fwd_ok, "of (FOK) above, with broadcast dest");
	PS(mcast_fwd_ok, "of (FOK) above, with multicast dest");
	PS(dhcp_bcast, "DHCP broadcast seen");
	PS(bcast, "Other broadcast seen");
	PS(addp_mcast, "ADDP multicast seen");
	PS(mcast, "Other multicast seen");
	printf("\n");
   printf("Wifi statistics:\n");
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
   printf("     decryption error  %u\n", _wifi_macStats.rxDropDecrypt);
   printf("     hardware overrun  %u\n", _wifi_macStats.rxDropOverrun);
   printf("     replay detected   %u\n", _wifi_macStats.rxDropReplay);
   printf("     MIC check failed  %u\n", _wifi_macStats.rxDropMICFail);
	printf("\n");
}



void net_tick(void)
{
   // Check for and respond to ADDP requests.
   addp_tick();
#ifdef USE_HTTP
   http_handler();
#else
   tcp_tick(NULL);
#endif
}


enum {
	SCAN_INIT,
	SCAN_COMPLETE,
	SCAN_SELECTED,
	SCAN_CREATE_IBSS,
	SCAN_WAITUP,
	SCAN_START_PROBE,

	SCAN_PROBE0,
	SCAN_PROBE1,
	SCAN_PROBE2,
	SCAN_PROBE3,
	SCAN_PROBE4, // one for each config'd SSID

	SCAN_END		// must be last
};
int scan_state = SCAN_INIT;
word scan_timer;


void associate(wifi_t * w_sel, int create, int join_ibss, int channel)
{
   ifdown(IF_WIFI0);
   while (ifpending(IF_WIFI0) != IF_DOWN)
   	net_tick();

	if (create) {
		// If creating IBSS, don't muck around with DHCP,
		// just force linklocal address since there is nothing else
		// on this network.
	   ifconfig( IF_WIFI0,
	   	IFS_DHCP, 0,
	   	IFS_IPADDR, IPADDR(169,254,0,0),
	   	IFS_NETMASK, IPADDR(255,255,0,0),
	   	IFS_ROUTER_SET, IPADDR(0,0,0,0),
	      IFS_END);
	}
	else if (config.wifi_static.ip) {
	   ifconfig( IF_WIFI0,
	   	IFS_DHCP, 0,
	   	IFS_IPADDR, config.wifi_static.ip,
	   	IFS_NETMASK, config.wifi_static.netmask,
	   	IFS_ROUTER_SET, config.wifi_static.gateway,
	      IFS_END);
	}
	else {
	   ifconfig( IF_WIFI0,
	   	IFS_DHCP, 1,
	   	IFS_DHCP_FALLBACK, 1,
	   	IFS_DHCP_FB_IPADDR, IPADDR(169,254,0,0),
	   	IFS_NETMASK, IPADDR(255,255,0,0),
	   	IFS_ROUTER_SET, IPADDR(0,0,0,0),
	      IFS_END);
	}

   ifconfig(IF_WIFI0,
   	IFS_WIFI_MODE,
   		create ? IFPARAM_WIFI_ADHOC :
   		join_ibss ? IFPARAM_WIFI_ADHOC_JOIN_ONLY :
   		IFPARAM_WIFI_ANY,
      IFS_WIFI_SSID, (int)w_sel->ssid_len, w_sel->ssid,
      IFS_WIFI_CHANNEL, (int)(channel ? channel : create ? w_sel->ibss_create_chan : w_sel->chan),
      IFS_WIFI_AUTHENTICATION, IFPARAM_WIFI_AUTH_OPEN,
      IFS_END);
   switch (w_sel->encr) {
   case WLN_ENCR_OPEN:
      ifconfig(IF_WIFI0,
         IFS_WIFI_ENCRYPTION, IFPARAM_WIFI_ENCR_NONE,
         IFS_END);
      break;
   case WLN_ENCR_WEP:
      ifconfig(IF_WIFI0,
         IFS_WIFI_ENCRYPTION, IFPARAM_WIFI_ENCR_WEP,
         IFS_WIFI_WEP_KEY_HEXSTR, 0, w_sel->key,
         IFS_END);
      break;
#ifdef WIFI_USE_WPA
   case WLN_ENCR_TKIP:
      ifconfig(IF_WIFI0,
         IFS_WIFI_ENCRYPTION, IFPARAM_WIFI_ENCR_TKIP,
         IFS_WIFI_AUTHENTICATION, IFPARAM_WIFI_AUTH_WPA_PSK,
         IFS_WIFI_WPA_PSK_BIN, w_sel->key,
         IFS_END);
      break;
   case WLN_ENCR_CCMP:
      ifconfig(IF_WIFI0,
         IFS_WIFI_ENCRYPTION, IFPARAM_WIFI_ENCR_CCMP,
         IFS_WIFI_AUTHENTICATION, IFPARAM_WIFI_AUTH_WPA_PSK,
         IFS_WIFI_WPA_PSK_BIN, w_sel->key,
         IFS_END);
      break;
#endif
   }
   ifup(IF_WIFI0);
}

int create_ibss(int last)
{
	wifi_t * w;
	int i;

	if (!(config.flags & CF_CREATE_IBSS) && !last) {
   #ifdef BRIDGE_VERBOSE
      printf("SCAN: IBSS creation not permitted by config\n");
   #endif
		scan_state = SCAN_END;
		return 1;	// None available
	}

	// Find first/last entry with ability to create IBSS
	if (last) for (i = N_SSID-1; i >= 0; --i) {
		w = config.wifi + i;
		if (w->ssid_len && w->ibss_create_chan)
			break;
	}
	else for (i = 0; i < N_SSID; ++i) {
		w = config.wifi + i;
		if (w->ssid_len && w->ibss_create_chan)
			break;
	}
	if (i < N_SSID && i >= 0) {
   #ifdef BRIDGE_VERBOSE
      printf("SCAN: creating IBSS %.*s on channel %u\n",
      	w->ssid_len, w->ssid, w->ibss_create_chan);
   #endif
	   scan_state = SCAN_CREATE_IBSS;
	   associate(w, 1, 0, 0);
	   scan_timer = _SET_SHORT_TIMEOUT(IBSS_CREATE_TIMEOUT);
	   return 0;
	}
	else {
   #ifdef BRIDGE_VERBOSE
      printf("SCAN: no IBSS specified\n");
   #endif
		scan_state = SCAN_END;
		return 1;	// None available
	}
}



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

/****************************************************************************
	rxsignal_cmp

	qsort comparison, based on rx signal strength.

   Inputs:
      a, b  -- far pointers to _wifi_wln_scan_bss, a struct populated by
               the WIFI SCAN, including rx_signal (relative receive signal
               strength).

	Return value: > 0 if a > b
					  < 0 if a < b
					  0   if a == b

****************************************************************************/
int rxsignal_cmp(far _wifi_wln_scan_bss *a, far _wifi_wln_scan_bss *b) {
	return b->rx_signal - a->rx_signal;
}

/****************************************************************************
	scan_callback

   Prints out the sorted results of a BSS scan.
   Called when WIFI SCAN is complete.

   The argument is a pointer to the wifi_scan_data structure generated by
   the scan.

   We use _f_qsort to sort the data since the data is `far.'  _f_qsort
   requires a comparison function, and we use the rxsignal_cmp() function
   above.

   Inputs:
      data  -- far pointer to wifi_scan_data structure, which contains a
               count of the number of responses, and an array of
               _wifi_wln_scan_bss structures, with the first `count'
               containing valid data for the responses.

****************************************************************************/
void scan_callback(far wifi_scan_data* data)
{
	uint8 i, j;
	int sel, join_ibss, channel;
	far _wifi_wln_scan_bss *bss;
   char ssid_str[33];
   char buffer[80];
   wifi_t * w;
   wifi_t * w_sel;

	scan_state = SCAN_COMPLETE;

	bss = data->bss;

	// Sort results by signal strength.  Need to use _f_qsort, since bss is
	// far data.
	_f_qsort(bss, data->count, sizeof(bss[0]), rxsignal_cmp);
	// Print out matching results and pick best
	sel = -1;
	channel = 0;
   for (i = 0; i < data->count; i++) {
   	wifi_ssid_to_str (ssid_str, bss[i].ssid, bss[i].ssid_len);
   	// Filter according to our SSID list
   	for (j = 0; j < N_SSID; ++j) {
   		w = config.wifi + j;
   		if (!w->ssid_len)
   			continue;
   		if (bss[i].ssid_len != w->ssid_len ||
   		    memcmp(bss[i].ssid, w->ssid, w->ssid_len))
   			continue;
#ifdef BRIDGE_VERBOSE
      	printf("Considering %s:\n", ssid_str);
#endif
   		if (w->chan && w->chan != bss[i].channel) {
#ifdef BRIDGE_VERBOSE
      		printf(" - reject, on ch %d but want %d\n", bss[i].channel, w->chan);
#endif
   			continue;
         }
   		switch (w->encr) {
   		case WLN_ENCR_OPEN:
   			if (bss[i].bss_caps & WLN_CAP_PRIVACY) {
#ifdef BRIDGE_VERBOSE
      			printf(" - reject, has PRIVACY caps but want open\n");
#endif
   				continue;
            }
   			break;
   		case WLN_ENCR_WEP:
   			if (!(bss[i].bss_caps & WLN_CAP_PRIVACY) ||
					 bss[i].wpa_info[0] != 0) {
#ifdef BRIDGE_VERBOSE
      			printf(" - reject, is not WEP\n");
#endif
   				continue;
            }
   			break;
   		case WLN_ENCR_TKIP:
   			if (!(bss[i].bss_caps & WLN_CAP_PRIVACY) ||
					 bss[i].wpa_info[0] != _WIFI_ELEM_VENDOR) {
#ifdef BRIDGE_VERBOSE
      			printf(" - reject, is not TKIP\n");
#endif
   				continue;
            }
   			break;
   		case WLN_ENCR_CCMP:
   			if (!(bss[i].bss_caps & WLN_CAP_PRIVACY) ||
					 bss[i].wpa_info[0] != _WIFI_ELEM_RSN) {
#ifdef BRIDGE_VERBOSE
      			printf(" - reject, is not CCMP\n");
#endif
   				continue;
            }
   			break;
   		}
   		break;
   	}
   	if (j == N_SSID)
   		continue;	// No match
   	if (sel < 0) {
   		// Pick first of those matching criteria (will have highest RSSI)
   		sel = i;
   		w_sel = w;
   		join_ibss = bss[i].bss_caps & WLN_CAP_IBSS;
   		channel = bss[i].channel;
   	}
#ifdef BRIDGE_VERBOSE
      printf("%c: chan %2d; rx_signal %d; MAC ",
      	i < 10 ? i+'0' : i+'a'-10,
      	bss[i].channel,
      	bss[i].rx_signal);
      print_macaddress(bss[i].bss_addr);
      printf("; SSID [%s]\n", ssid_str);
      printf("   Rates: %s\n", wifi_rates_str(buffer, bss[i].rates_basic, bss[i].rates));
      printf("   Capable: %s\n", wifi_cap_str(buffer, bss[i].bss_caps));
      printf("   ERP info: %s\n", wifi_erp_str(buffer, bss[i].erp_info));
      if (bss[i].bss_caps & WLN_CAP_PRIVACY) {
      	switch (bss[i].wpa_info[0]) {
      	case _WIFI_ELEM_RSN:
      		printf("   WPA2/RSN %s\n",
      			bss[i].wpa_info[19] == 2 ? "PSK" :
      			bss[i].wpa_info[19] == 1 ? "802.1X" :
      			"VENDOR");
      		break;
      	case _WIFI_ELEM_VENDOR:
      		printf("   WPA %s\n",
      			bss[i].wpa_info[23] == 2 ? "PSK" :
      			bss[i].wpa_info[23] == 1 ? "802.1X" :
      			"VENDOR");
      		break;
      	default:
      		printf("   WEP\n");
      		break;
      	}
      }
#endif
   }

   if (sel >= 0) {
#ifdef BRIDGE_VERBOSE
   	printf("SCAN: scan located SSID %.*s\n", (int)w_sel->ssid_len, w_sel->ssid);
#endif
		scan_state = SCAN_SELECTED;
		associate(w_sel, 0, join_ibss, channel);
   }
}


int active_probe(int n)
{
	wifi_t * w = config.wifi + n;
	while (!w->ssid_len) {
		if (++n >= N_SSID)
			return -1;
		++w;
	}
#ifdef BRIDGE_VERBOSE
	printf("SCAN: probing config %d (SSID %.*s)\n", n, (int)w->ssid_len, w->ssid);
#endif
	scan_state = SCAN_PROBE0 + n;
	associate(w, 0, w->ibss_create_chan, 0);
	scan_timer = _SET_SHORT_TIMEOUT(ASSOCIATE_TIMEOUT);
	return 0;
}


void start_scan(void)
{
	ifdown(IF_WIFI0);
	while (ifpending(IF_WIFI0) != IF_DOWN)
		net_tick();
	if (boot_mode & BOOT_LAST_IBSS) {
		// Force selection of last IBSS.
		create_ibss(1);
	}
	else if (config.flags & CF_PASSIVE_SCAN) {
	   ifconfig(IF_WIFI0,
	      IFS_WIFI_MODE, IFPARAM_WIFI_NO_JOIN,
	      IFS_WIFI_SCAN, scan_callback,
	      IFS_END);
	   scan_timer = _SET_SHORT_TIMEOUT(SCAN_TIMEOUT);
	   scan_state = SCAN_INIT;
	}
	else if (config.flags & CF_ACTIVE_PROBE) {
		scan_state = SCAN_START_PROBE;
	}
	else {
		create_ibss(0);
	}
}


// Called regularly while IF_WIFI0 is not yet UP.
void scan_tick(void)
{
	int n;

	switch (scan_state) {
	case SCAN_INIT:
		break;
	case SCAN_COMPLETE:
		// If timeout and nothing found, try each SSID entry in turn as
		// active probes.
		if (_CHK_SHORT_TIMEOUT(scan_timer)) {
		#ifdef BRIDGE_VERBOSE
			printf("SCAN: nothing found in scan, trying active probes\n");
		#endif
			if (!(config.flags & CF_ACTIVE_PROBE)) {
	      #ifdef BRIDGE_VERBOSE
	         printf("SCAN: skipping active probe\n");
	      #endif
	      	create_ibss(0);
	      	break;
			}
			if (active_probe(0)) {
	      #ifdef BRIDGE_VERBOSE
	         printf("SCAN: no SSIDs configured!\n");
	      #endif
	      	scan_state = SCAN_END;
	      }
		}
		break;
	case SCAN_SELECTED:
		// another few seconds to come up
		scan_timer = _SET_SHORT_TIMEOUT(ASSOCIATE_TIMEOUT);
		scan_state = SCAN_WAITUP;
		break;
	case SCAN_CREATE_IBSS:
		if (_CHK_SHORT_TIMEOUT(scan_timer)) {
		#ifdef BRIDGE_VERBOSE
			printf("SCAN: could not create IBSS\n");
		#endif
      	scan_state = SCAN_END;
		}
		break;
	case SCAN_WAITUP:
		if (_CHK_SHORT_TIMEOUT(scan_timer)) {
		#ifdef BRIDGE_VERBOSE
			printf("SCAN: selected SSID failed to associate, trying active probes\n");
		#endif
	case SCAN_START_PROBE:
			if (active_probe(0)) {
	      #ifdef BRIDGE_VERBOSE
	         printf("SCAN: no SSIDs configured!\n");
	      #endif
	      	scan_state = SCAN_END;
			}
		}
		break;
	default: // SCAN_PROBE0..4 etc.
		if (_CHK_SHORT_TIMEOUT(scan_timer)) {
			++scan_state;
			if (scan_state >= SCAN_END || active_probe(scan_state - SCAN_PROBE0)) {
				scan_state = SCAN_END;
			}
		}
		break;
	case SCAN_END:
		break;
	}

	if (_CHK_SHORT_TIMEOUT(scan_timer)) {
		// Redo the scan.  Keep looking!
   #ifdef BRIDGE_VERBOSE
      printf("SCAN: starting over\n");
   #endif
   	start_scan();
	}

}


// reset configuration to defaults
void default_config( config_t *c)
{
	int k = 0;
	memset( c, 0, sizeof(*c));

	// Set the default WiFi scan, probe and ad-hoc flags.
	c->flags = USE_WIFI_BRIDGE_DFLT_CONFIG_FLAGS;

	/* Set some default SSIDs.
      The idea of this is to set up a list of known SSIDs, with each enabled
      entry going into config->wifi[0], config->wifi[1], etc., in turn.  In a
      real application, this list would be initially set up via a non-Wifi
      interface.  This list is only used when the user ID block is
      uninitialized; thereafter, it is read from the user block on startup.

      Entries with the ibss_create_chan field non-zero are special.
      They indicate entries where it is possible to create an ad-hoc
      network.  This is most useful for when a Wifi link is being
      created to join two ethernet segments into one logical ethernet
      segment.  In this case, the Wifi network does not have to
      correspond to any pre-existing SSID, so a mutually agreed ad-hoc
      network is most appropriate.

      At most 2 entries with non-zero ibss_create_chan field should be
      defined.  The first entry is the one which will normally be chosen
      to create the ad-hoc network.  The last entry is used only if the
      S1 switch (on the dev board) is held when rebooting.  S1 can be
      useful when initially establishing a radio link.  It forces the
      device to always start the ad-hoc network, in order to eliminate
      any possible race condition with the other end.

      If there is only one entry with ibss_create_chan non-zero, then that
      entry is both the first and last so the above considerations still
      apply.  If there are no ad-hoc entries, then only a link to
      an existing infrastructure network can be established.  This is
      useful for simply connecting an isolated ethernet segment to
      an existing access point and its infrastructure network.
   */

#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_A
	strncpy(c->wifi[k].ssid, BRIDGE_WIFI_DFLT_CONFIG_A_SSID,
	        sizeof c->wifi[0].ssid - 1u);
	c->wifi[k].ssid_len = strlen(c->wifi[k].ssid);
	c->wifi[k].encr = BRIDGE_WIFI_DFLT_CONFIG_A_ENCR;
	switch (c->wifi[k].encr)
	{
	case WLN_ENCR_WEP:
		_f_memcpy(c->wifi[k].key, BRIDGE_WIFI_DFLT_CONFIG_A_KEY,
		          sizeof c->wifi[0].key);
		break;
	case WLN_ENCR_TKIP:
	case WLN_ENCR_TKIP | WLN_ENCR_CCMP:
	case WLN_ENCR_CCMP:
		hexstr2bin(BRIDGE_WIFI_DFLT_CONFIG_A_KEY, c->wifi[k].key,
		           sizeof c->wifi[0].key);
	default:
		//break;
	}
	c->wifi[k].ibss_create_chan = BRIDGE_WIFI_DFLT_CONFIG_A_CHAN;
	++k;
#endif
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_B
	strncpy(c->wifi[k].ssid, BRIDGE_WIFI_DFLT_CONFIG_B_SSID,
	        sizeof c->wifi[0].ssid - 1u);
	c->wifi[k].ssid_len = strlen(c->wifi[k].ssid);
	c->wifi[k].encr = BRIDGE_WIFI_DFLT_CONFIG_B_ENCR;
	switch (c->wifi[k].encr)
	{
	case WLN_ENCR_WEP:
		_f_memcpy(c->wifi[k].key, BRIDGE_WIFI_DFLT_CONFIG_B_KEY,
		          sizeof c->wifi[0].key);
		break;
	case WLN_ENCR_TKIP:
	case WLN_ENCR_TKIP | WLN_ENCR_CCMP:
	case WLN_ENCR_CCMP:
		hexstr2bin(BRIDGE_WIFI_DFLT_CONFIG_B_KEY, c->wifi[k].key,
		           sizeof c->wifi[0].key);
	default:
		//break;
	}
	c->wifi[k].ibss_create_chan = BRIDGE_WIFI_DFLT_CONFIG_B_CHAN;
	++k;
#endif
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_C
	strncpy(c->wifi[k].ssid, BRIDGE_WIFI_DFLT_CONFIG_C_SSID,
	        sizeof c->wifi[0].ssid - 1u);
	c->wifi[k].ssid_len = strlen(c->wifi[k].ssid);
	c->wifi[k].encr = BRIDGE_WIFI_DFLT_CONFIG_C_ENCR;
	switch (c->wifi[k].encr)
	{
	case WLN_ENCR_WEP:
		_f_memcpy(c->wifi[k].key, BRIDGE_WIFI_DFLT_CONFIG_C_KEY,
		          sizeof c->wifi[0].key);
		break;
	case WLN_ENCR_TKIP:
	case WLN_ENCR_TKIP | WLN_ENCR_CCMP:
	case WLN_ENCR_CCMP:
		hexstr2bin(BRIDGE_WIFI_DFLT_CONFIG_C_KEY, c->wifi[k].key,
		           sizeof c->wifi[0].key);
	default:
		//break;
	}
	c->wifi[k].ibss_create_chan = BRIDGE_WIFI_DFLT_CONFIG_C_CHAN;
	++k;
#endif
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_D
	_f_strncpy(c->wifi[k].ssid, BRIDGE_WIFI_DFLT_CONFIG_D_SSID,
	           sizeof c->wifi[0].ssid - 1u);
	c->wifi[k].ssid_len = _f_strlen(c->wifi[k].ssid);
	c->wifi[k].encr = BRIDGE_WIFI_DFLT_CONFIG_D_ENCR;
	switch (c->wifi[k].encr)
	{
	case WLN_ENCR_WEP:
		_f_memcpy(c->wifi[k].key, BRIDGE_WIFI_DFLT_CONFIG_D_KEY,
		          sizeof c->wifi[0].key);
		break;
	case WLN_ENCR_TKIP:
	case WLN_ENCR_TKIP | WLN_ENCR_CCMP:
	case WLN_ENCR_CCMP:
		hexstr2bin(BRIDGE_WIFI_DFLT_CONFIG_D_KEY, c->wifi[k].key,
		           sizeof c->wifi[0].key);
	default:
		//break;
	}
	c->wifi[k].ibss_create_chan = BRIDGE_WIFI_DFLT_CONFIG_D_CHAN;
	++k;
#endif
#if defined USE_BRIDGE_WIFI_DFLT_CONFIG_E
	strncpy(c->wifi[k].ssid, BRIDGE_WIFI_DFLT_CONFIG_E_SSID,
	        sizeof c->wifi[0].ssid - 1u);
	c->wifi[k].ssid_len = strlen(c->wifi[k].ssid);
	c->wifi[k].encr = BRIDGE_WIFI_DFLT_CONFIG_E_ENCR;
	switch (c->wifi[k].encr)
	{
	case WLN_ENCR_WEP:
		_f_memcpy(c->wifi[k].key, BRIDGE_WIFI_DFLT_CONFIG_E_KEY,
		          sizeof c->wifi[0].key);
		break;
	case WLN_ENCR_TKIP:
	case WLN_ENCR_TKIP | WLN_ENCR_CCMP:
	case WLN_ENCR_CCMP:
		hexstr2bin(BRIDGE_WIFI_DFLT_CONFIG_E_KEY, c->wifi[k].key,
		           sizeof c->wifi[0].key);
	default:
		//break;
	}
	c->wifi[k].ibss_create_chan = BRIDGE_WIFI_DFLT_CONFIG_E_CHAN;
	++k;
#endif
}

// Load configuration stored in UserBlock; if CRC-32 is bad, use defaults
// returns 0 if config in UserBlock is good, -1 if using default configuration
int load_config( config_t *c)
{
	int err;

#ifndef SET_FACTORY
	// On some serial boot boards, need to call readUserBlock until it returns
	// a value <= 0.
	do
	{
		err = readUserBlock( c, NETCONFIG_OFFSET, sizeof(*c));
	} while (err > 0);

	if (err)
	{
		printf( "Reading from UserBlock failed with error %d!\n", err);
	}
	else if (c->crc32 == crc32_calc( c, sizeof(*c) - 4, 0))
	{
		// CRC-32 good, config in user block is valid
		return 0;
	}
#endif
	// Couldn't read valid configuration, load default and return error
	default_config( c);
	return -1;
}

// Calculate CRC-32 and save configuration to UserBlock.  Returns 0 if
// successful, or a value less than 0 on failure.
int save_config( config_t *c)
{
	int err;

	// update CRC-32
	c->crc32 = crc32_calc( c, sizeof(*c) - 4, 0);

	// Save to UserBlock.  On some serial boot boards, need to call
	// writeUserBlock until it returns a value <= 0.
	do
	{
		err = writeUserBlock( NETCONFIG_OFFSET, c, sizeof(*c));
	} while (err > 0);

	if (err)
	{
		printf( "Writing to UserBlock failed with error %d!\n", err);
	}

	return err;
}

// callback for ADDP library to change network settings
int addp_callback_if( int iface, unsigned long ip_addr, unsigned long netmask,
	unsigned long gateway)
{
	char buffer[20];

#ifdef BRIDGE_VERBOSE
	printf("\nADDP information:\n");
	printf( "Interface %s - IP:%s",
		iface == IF_ETH0 ? "ETH" : "WIFI", inet_ntoa( buffer, ip_addr));
	printf( "\tmask:%s", inet_ntoa( buffer, netmask));
	printf( "\tgw:%s\n\n", inet_ntoa( buffer, gateway));
#endif

   // use static IP if it is non-zero, else if zero will use DHCP or linklocal
   if (iface == IF_ETH0) {
      config.eth_static.ip = ip_addr;
      config.eth_static.netmask = netmask;
      config.eth_static.gateway = gateway;
   }
   else {
      config.wifi_static.ip = ip_addr;
      config.wifi_static.netmask = netmask;
      config.wifi_static.gateway = gateway;
   }

	// save changes to the userblock
	return save_config( &config);
}








void main()
{
	int i, c;
	char ip[16], netmask[16];
	longword tmo;
	word iface, wpend, epend, prev_wpend, prev_epend, mask;
	int err, macstatus;
	word led_timer, led_phase;
	unsigned long led_state;

	brdInit();

	// If S1 pressed on boot, force this unit to create/join the
	// *last* IBSS defined in the configuration.
	boot_mode = 0;
	if (!BitRdPortI(PDDR, 1)) {
		boot_mode |= BOOT_LAST_IBSS;
		printf("Boot mode = LAST IBSS\n");
	}

	boot_time = SEC_TIMER;

	clear_stats();

	debug_on = 0;

	// Init table to all unused (but keep valid old entries)
   init_table(0);

	// Start network and wait for interface to come up (or error exit).
	printf("MAC: %02X:%02X:%02X\n",
		SysIDBlock.macAddr[3],SysIDBlock.macAddr[4],SysIDBlock.macAddr[5]);


	load_config( &config);

	if (sock_init()) {
		printf("sock_init() failed\n");
		exit(-NETERR_IFDOWN);
	}

// Network initialization.  Bring both interfaces up.  Both interfaces attempt
// to use DHCP, but fall back to linklocal (169.254/16).  At least one
// interface should have DHCP, otherwise we won't be able to bridge IP
// packets when the destination IP addr is not local (i.e. knowable using ARP).

	// Set up network, based on settings from stored configuration
	if (config.eth_static.ip) {
		// static IP
	   ifconfig( IF_ETH0,
	   	IFS_DHCP, 0,
	   	IFS_IPADDR, config.eth_static.ip,
	   	IFS_NETMASK, config.eth_static.netmask,
	   	IFS_ROUTER_SET, config.eth_static.gateway,
	      IFS_END);
	}
	else {
		// DHCP, fall back to link-local (169.254.x.x address) on DHCP timeout.
	   ifconfig( IF_ETH0,
	   	IFS_DHCP, 1,
	   	IFS_DHCP_FALLBACK, 1,
	   	IFS_DHCP_FB_IPADDR, IPADDR(169,254,0,0),
	   	IFS_NETMASK, IPADDR(255,255,0,0),
	   	IFS_ROUTER_SET, IPADDR(0,0,0,0),
	      IFS_END);
	}

   // start the ADDP responder and HTTP
   err = addp_init( IF_ANY, NULL);
   if (err)
   {
      printf( "%s: error %d calling %s\n", __FUNCTION__, err, "addp_init");
   }
#ifdef USE_HTTP
   http_init();
   tcp_reserveport(80);
#endif

	macstatus = -1;
	prev_wpend = IF_DOWN;
	prev_epend = IF_DOWN;

	// Let Ethernet come up, since we can always discover and configure over it.
	ifup(IF_ETH0);

	// Start scan on Wifi, but don't join yet.
	start_scan();

	// This sample allows some keyboard control.  If running without a debug
   // connection, then these are simply ignored.
	printf("Press following keys:\n");
	printf("  t       print interface status and bridging table\n");
	printf("  m       print WiFi MAC status\n");
	printf("  p       print statistics\n");
	printf("  P       print and clear statistics\n");
	printf("  r       force bridging table re-organization\n");
	printf("  s       (re)scan WiFi interface\n");
	printf("  e       restart Ethernet interface\n");
	printf("  x       toggle 'active probe' flag\n");
	printf("  f       toggle 'fixed Tx rate' flag\n");
	printf("  <>      decrease/increase Tx rate (and set fixed)\n");

   //debug_on = 3;

	led_timer = _SET_SHORT_TIMEOUT(63);
	led_phase = 0;
	led_state = 0;

   for (;;) {
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
   		else if (c == 's') {
   			printf("rescan...\n");
   			boot_mode &= ~BOOT_LAST_IBSS;
		      start_scan();
   		}
   		else if (c == 'e') {
   			printf("Ethernet restart...\n");
		      ifdown(IF_ETH0);
   		}
   		else if (c == 'x') {
   			config.flags ^= CF_ACTIVE_PROBE;
   			printf("Active probe now %s (press 's' to rescan)\n",
   				config.flags & CF_ACTIVE_PROBE ? "ACTIVE" : "SKIP");
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
            printf("Rate now %4.1g\n", _wifi_rateInfo[_wifi_rateTx].bps * 0.5);
         }
         else if (c == '>') {
         	if (_wifi_rateTx < _WIFI_RATE_NUM-1)
            	++_wifi_rateTx;
				_wifi_macParams.options |= WLN_OPT_FIXEDRATE;
            printf("Rate now %4.1f\n", _wifi_rateInfo[_wifi_rateTx].bps * 0.5);
         }
   	}

		net_tick();

		wpend = ifpending(IF_WIFI0);
		epend = ifpending(IF_ETH0);

	   // Wifi interface needs to scan first and pick one of the configured SSIDs.
	   // While this is happening, Ethernet will come up with DHCP or static config.
	   // It may time out without associating, or it may create our own IBSS.
	   if (wpend != IF_UP)
	   	scan_tick();

		// Never give up on Ethernet
	   if (epend == IF_DOWN)
	   	ifup(IF_ETH0);

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

      if (epend == IF_UP && wpend == IF_UP) {
         if (!(bst[IF_ETH0] & BRIDGING))
            printf("Bridging ON\n");
         bst[IF_ETH0] |= BRIDGING;
         bst[IF_WIFI0] |= BRIDGING;
      }
      else {
         if ((bst[IF_ETH0] & BRIDGING))
            printf("Bridging OFF\n");
         bst[IF_ETH0] &= ~BRIDGING;
         bst[IF_WIFI0] &= ~BRIDGING;
      }

      if (_wifi_macStatus.state != macstatus) {
      	macstatus = _wifi_macStatus.state;
      	printf("MAC status now %s\n", wifi_state_str(macstatus));
      }

		// Drive the DS1 LED
		if (_CHK_SHORT_TIMEOUT(led_timer)) {
			led_timer = _SET_SHORT_TIMEOUT(63);
			++led_phase;
			led_phase &= 0x1F;
			if (wpend == IF_UP && epend == IF_UP) {
				led_state = 0xFFFFFFFFuL;	// solid ON when bridging
            // Also do table housekeeping
            housekeeping();
         }
			else if (epend == IF_UP || wpend == IF_UP) {
				led_state = 0xF0000000uL;
				if (epend==IF_UP) {
					led_state |= 0x00F00000uL;	// 2 flash every 2 sec if eth but no wifi
					if (scan_state >= SCAN_PROBE0)
						led_state |= 0xCCCC;		// plus 4 Hz over 1 sec if active probing
					else if (scan_state == SCAN_INIT)
						led_state |= 0x55555uL;	// or 8 Hz flash if passive scanning
				}
				if (wpend==IF_UP)
					led_state |= 0x00FFFFFFuL;	// 1/4 sec off every 2 secs if wifi
															// but no ethernet
			}
			else
				led_state = 0x00010001uL;	// short flash every sec if alive but no network

			BitWrPortI(PDDR, &PDDRShadow, (led_state & (1uL<<led_phase)) == 0, 0);
		}
   }
}
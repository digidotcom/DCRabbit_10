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
/*

	Samples/tcpip/addp.c

	This program demonstrates Rabbit's implementation of the Advanced Device
	Discovery Protocol (ADDP).  This is the protocol used by Digi's
	Device_Discovery and Device_Setup_Wizard applications.

	View the library description in ADDP.LIB for documentation on the macros
	you can use to configure the library.

	Usage
	-----
	Compile to the target.  Choose "Close Connection" from the "Run" menu.
	Disconnect power and change to the DIAG connector on the programming cable.
	Open a terminal program using the programming cable's serial port on the PC.
	Set the baud rate to 57600.  Power up the target and monitor the debug
	output.

	Run Device_Discovery or Device_Setup_Wizard (located in Dynamic C's
	Utilities/ADDP directory) on a PC connected to the same local network as
	the Rabbit.  Change the network settings and confirm that the Rabbit
	reboots with the updated settings.  The default password in this sample is
	"rabbit".

	For simplicity, this sample does not include a web server, so you will not
	be able to use the "Open web interface" option from "Device Tasks"
	inside of the Device_Discovery application or the "Log on to web user
	interface of device" checkbox of the Device_Setup_Wizard application.

	WARNING: This sample writes network configuration settings to the UserBlock.
	Be sure to set NETCONFIG_OFFSET to an area of the UserBlock that can hold
	a 256-byte structure.
*/

/*** Begin Configuration ***/

/*
 * If you're already storing information in the UserBlock, select an offset
 * that won't overlap the existing information.  This sample program stores
 * a 256-byte configuration structure starting at the offset specified below.
 */
#define NETCONFIG_OFFSET 0

/*
 * Uncomment these defines to enable verbose output and debugging at various
 * network levels in order to monitor how the library works.
 */
#define ADDP_VERBOSE
//#define ADDP_DEBUG
//#define UDP_VERBOSE
//#define NET_VERBOSE
//#define NET_DEBUG

/*
 * The following lines configure the ADDP library for your product, instead of
 * using the default settings.
 */

#define ADDP_NAME			"John Doe's Rabbit"
#define ADDP_PASSWORD	"rabbit"
// #define ADDP_HWNAME		"My Hardware"

/*
 * The library defaults to 0x44494749 ("DIGI"), the cookie used by the
 * Windows-based Digi Device Discovery Utility and Digi Device Setup Wizard.
 * If using the ADDP DLLs or their sample program (Finder.exe) from [ADDP
 * Utility Source.zip], set the cookie to 0x44564B54 ("DVKT").
 */
//#define ADDP_COOKIE 0x44564B54		// Use "DVKT" instead of "DIGI".

/*
 * Default behavior is to reply to all requests with the correct cookie.  If
 * using the files from [Utilities/ADDP/ADDP Utility Source.zip], you can
 * generate and use a Globally Unique ID (GUID) so only your devices will
 * respond to requests from your utility.

 * Set the GUID here, in the format shown.  The GUID below is the one used
 * by Finder.exe from the "ADDP Utility Source.zip" file.
 */

/* GUID array for "bf6db409-c83d-44a3-a36d-21797d2f73f9" */
//const byte addp_guid[] = {
//	0xbf, 0x6d, 0xb4, 0x09,		0xc8, 0x3d,		0x44, 0xa3,
//	0xa3, 0x6d, 		0x21, 0x79, 0x7d, 0x2f, 0x73, 0xf9 };
//#define ADDP_GUID addp_guid

/*** End Configuration ***/

// This sample uses a function called "addp_callback_if" to receive change
// requests from ADDP.LIB.
#define ADDP_CALLBACK_IF(iface,ip,mask,gw) addp_callback_if(iface,ip,mask,gw)

/*
 * Redirect STDOUT to serial port D to monitor progress while testing
 * ADDP code.  Since ADDP clients will want to reset the Rabbit after making
 * changes, you cannot run this program from the Dynamic C debugger.
 */
#define  STDIO_DEBUG_SERIAL   SADR
#define	STDIO_DEBUG_BAUD		57600
#define	STDIO_DEBUG_ADDCR

/*
 * Number of buffers allocated for UDP sockets, and hence the
 * total number of allowable UDP sockets.
 */
#define MAX_UDP_SOCKET_BUFFERS	4

/*
 * Enable the use of multicasting.  Note that this does not
 * enable IGMP, so that if multicast routing is required, this
 * will not suffice.
 */
#define USE_MULTICAST

// When using DHCP, fallback on 169.254.x.x address if DHCP server not found
#define USE_LINKLOCAL

// start up with network down, set fields at runtime
#define TCPCONFIG 6
#define USE_DHCP

#use "idblock_api.lib"
#use "dcrtcp.lib"
#use "addp.lib"

// CRC-32 used to detect valid configuration stored in UserBlock
#use "crc32.lib"

/*
 * Data structure used to store current network configuration.  The <reserved>
 * element exists to allow for future versions of this program to add more
 * data to the configuration while maintaining backward compatability with the
 * old structure.
 */
typedef struct {
	unsigned long ip;
	unsigned long netmask;
	unsigned long gateway;
	char		reserved[240];		// room for more stuff
	unsigned long crc32;
} config_t;

config_t config;

// reset configuration to defaults
void default_config( config_t *c)
{
	memset( c, 0, sizeof(*c));
}

// Load configuration stored in UserBlock; if CRC-32 is bad, use defaults
// returns 0 if config in UserBlock is good, -1 if using default configuration
int load_config( config_t *c)
{
	int err;

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

	printf( "Interface %d - IP:%s", iface, inet_ntoa( buffer, ip_addr));
	printf( "\tmask:%s", inet_ntoa( buffer, netmask));
	printf( "\tgw:%s\n", inet_ntoa( buffer, gateway));

	if (iface != IF_DEFAULT)
	{
		printf( "Ignoring callback -- this sample only handles interface %d.\n",
			IF_DEFAULT);
		return -1;
	}

	if (ip_addr)
	{
		// use static IP
      config.ip = ip_addr;
      config.netmask = netmask;
      config.gateway = gateway;
	}
	else
	{
		// use DHCP
		config.gateway = config.netmask = config.ip = 0;
	}

	// save changes to the userblock
	return save_config( &config);
}

void main()
{
	char ipbuf[16];
	unsigned long ip;

	int oldstatus, newstatus;
	int err;

	load_config( &config);

	// Start network and wait for interface to come up (or error exit).
	sock_init();

	// Set up network, based on settings from stored configuration
	if (config.ip)
	{
		// static IP
	   ifconfig( IF_DEFAULT,
	   	IFS_DHCP, 0,
	   	IFS_IPADDR, config.ip,
	   	IFS_NETMASK, config.netmask,
	   	IFS_ROUTER_SET, config.gateway,
	      IFS_END);
	}
	else
	{
		// DHCP, fall back to link-local (169.254.x.x address) on DHCP timeout.
	   ifconfig( IF_DEFAULT,
	   	IFS_DHCP, 1,
	   	IFS_DHCP_FALLBACK, 1,
	   	IFS_DHCP_FB_IPADDR, IPADDR(169,254,0,0),
	   	IFS_NETMASK, IPADDR(255,255,0,0),
	   	IFS_ROUTER_SET, IPADDR(0,0,0,0),
	      IFS_END);
	}

	ifup( IF_DEFAULT);

	// start the ADDP responder
   err = addp_init( IF_ANY, NULL);
   if (err)
   {
      printf( "%s: error %d calling %s\n", __FUNCTION__, err, "addp_init");
   }

	oldstatus = -1;
	for (;;)
	{
		newstatus = ifstatus( IF_DEFAULT);
		if (newstatus != oldstatus)
		{
			printf( "status change: was %d, now %d\n", oldstatus, newstatus);
			oldstatus = newstatus;

         ifconfig( IF_DEFAULT, IFG_IPADDR, &ip, IFS_END);
         printf( "My IP Address is now %s.\n", inet_ntoa( ipbuf, ip));
		}

		// Check for and respond to ADDP requests.
      addp_tick();

      tcp_tick( NULL);
	}
}
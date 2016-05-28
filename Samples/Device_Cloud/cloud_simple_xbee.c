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

	Simple Device Cloud sample, with XBee functionality.

	This shows basic use of Device Cloud to view and change node settings on an
   XBee network.

	-----------------------------------------------------------------------
	Instructions:
	-----------------------------------------------------------------------

	Visit https://devicecloud.digi.com/ to create a test account if you have
	not already done so.  You can also access the Device Cloud documentation
	at that site.

	Compile and run this sample.  You may need to place the following
	definitions in the Options->ProjectOptions->Defines tab:

		ROOT_SIZE_4K = 7U
		XMEMCODE_SIZE = 0x90000
		_SYS_MALLOC_BLOCKS=20
		_FIRMWARE_NAME_=""
		_FIRMWARE_VERSION_=0x0101
		_PRIMARY_STATIC_IP="10.10.6.100"

   [Setting _SYS_MALLOC_BLOCKS=20 is required because the RCI XBee
   interface may use a large amount of RAM initially, which is freed
	after initialization is complete.]

	The first 3 options may not be necessary (and can be adjusted up or down
	to suit the particular board being used).  The _FIRMWARE_* macros are
	needed if you enable firmware updates by defining IDIGI_USE_RPU.
	_PRIMARY_STATIC_IP and related network configuration macros may also
	be defined to set an initial "factory default" configuration.  See
	tcpconfig.lib and the network programming manual for details.  The
	default Device Cloud network configuration uses DHCP, with a fallback to the
	given static IP address.

	When running, navigate to https://devicecloud.digi.com/, log in, and add the
	board using the '+' button - this is only necessary the first time
	you run Device Cloud on a given board.  If you defined IDIGI_USE_ADDP, then the
	board will automatically appear in the list of devices which may be
	added.  When the board is added, you can double click on it to view
	and change the network configuration settings.

	Some configuration settings may cause the board to reboot.  This sample
	implements this by calling exit(0).  When using the Dynamic C debugger,
	you can simply hit F9 to run the program again (no need to recompile).
	In a real deployment, exit(0) causes the board to reboot.

	When changing the network configuration using Device Cloud, the configuration
	is saved in non-volatile storage and will be used next time the program
	(or any other Device Cloud sample) is run.  The macro settings you define
	such as _PRIMARY_STATIC_IP are only used the first time the Device Cloud samples
	are run.  Thereafter, it uses any settings set via the Device Cloud server,
	and saved in flash.

	If you accidentally configure a bad network setting (e.g. by setting a
	non-existent gateway address), then the board will try to use the
	last-known-good configuration if the "current" configuration cannot
	connect to the Device Cloud server.

	-----------------------------------------------------------------------
*/

// Use these settings if you want to test with a static configuration.
// Otherwise, use Device Cloud defaults (requires DHCP and/or ADDP)
//#define USE_STATIC_TEST
#ifdef USE_STATIC_TEST
	#define TCPCONFIG 6
	#define _IDIGI_FORCE_FACTORY
#endif

// Uncomment to dump Device Cloud/RCI traffic (formatted)
//#define RCI_VERBOSE_XML

// Uncomment to show debug print
//#define RCI_ZIGBEE_VERBOSE

// Uncomment to allow single stepping in various library code
//#define IDIGI_DEBUG
//#define DCRTCP_DEBUG
//#define ADDP_DEBUG
//#define PKTDRV_DEBUG
//#define WPAN_APS_DEBUG
//#define XBEE_DEVICE_DEBUG
//#define XBEE_SXA_DEBUG
//#define XBEE_WPAN_DEBUG
//#define XBEE_ATCMD_DEBUG

#define IDIGI_IFACE_VERBOSE	// This prints interface status when it changes.

#define IDIGI_USE_XBEE		// Required to include XBee support
//#define IDIGI_USE_ADDP	// Uncomment to include ADDP support
//#define IDIGI_USE_TLS		// Uncomment to include TLS support

#define IDIGI_PRODUCT "cloud_simple_xbee.c"
#define IDIGI_VENDOR "Digi International Inc."
#define IDIGI_VENDOR_ID "1234"
#define IDIGI_FIRMWARE_ID "1.01.01"
#define IDIGI_CONTACT "support@digi.com"
#define IDIGI_LOCATION "Planet Earth"
#define IDIGI_DESCRIPTION "Simple Device Cloud+XBee demo"
#define IDIGI_SERVER "my.devicecloud.com"


// Store non-volatile configuration data in the userID block, via the
// Simple UserID Block FileSystem.  You can use SUBFS to also store a limited
// amount of non-Device Cloud application configuration data.
#define IDIGI_USE_SUBFS
#define SUBFS_RESERVE_START 0
#define SUBFS_RESERVE_END 0

#define ADDP_PASSWORD	"rabbit"

// Required only if using TLS, but not using any static Zserver resources.
#define SSPEC_NO_STATIC


#use "Device_Cloud.lib"





void main()
{
	int rc;
	char c;
   char mac[6];

	if (idigi_init())
		exit(1);

	printf("Hit <space> to print network interface status and MAC address\n");
	printf("    ?       to print XBee node table\n");
	printf("    a       to print memory usage\n");
   printf("\nNote: MAC is useful for manually adding device to Device Cloud.\n\n");

_restart:

	do {
		rc = idigi_tick();

		if (kbhit()) {
			c = getchar();
         if (c == ' ') {
				ip_print_ifs();
            printf("MAC address: %02X%02X%02X:%02X%02X%02X\n",
	            SysIDBlock.macAddr[0], SysIDBlock.macAddr[1], SysIDBlock.macAddr[2],
	            SysIDBlock.macAddr[3], SysIDBlock.macAddr[4], SysIDBlock.macAddr[5]
	            );

         }
         else if (c == '?')
         	sxa_node_table_dump();
			else if (c == 'a')
				_sys_malloc_stats();
		}

	} while (!rc);

	printf("Final rc = %d\n", rc);
	if (rc == -NETERR_ABORT) {
		// RCI reboot request was received.  Normally we would use this
		// to shut down cleanly and reboot the board.
		printf("Rebooting via exit(0)!\n");
		exit(0);
	}

	goto _restart;

}
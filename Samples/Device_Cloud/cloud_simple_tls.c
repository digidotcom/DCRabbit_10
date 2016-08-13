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

	Simple Device Cloud sample, with TLS security.

	This shows basic use of Device Cloud to view and change network settings.

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
		_SYS_MALLOC_BLOCKS=10
		_FIRMWARE_NAME_=""
		_FIRMWARE_VERSION_=0x0101
		_PRIMARY_STATIC_IP="10.10.6.100"

	The first 3 options may not be necessary (and can be adjusted up or down
	to suit the particular board being used).  The _FIRMWARE_* macros are
	needed if you enable firmware updates by defining CLOUD_USE_RPU.
	_PRIMARY_STATIC_IP and related network configuration macros may also
	be defined to set an initial "factory default" configuration.  See
	tcpconfig.lib and the network programming manual for details.  The
	default Device Cloud network configuration uses DHCP, with a fallback to the
	given static IP address.

	When running, navigate to https://devicecloud.digi.com/, log in, and add the
	board using the '+' button - this is only necessary the first time
	you run Device Cloud on a given board.  If you defined CLOUD_USE_ADDP, then the
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

// You can comment out either or both of the following, to make a more spartan
// demo.
#define CLOUD_USE_TLS		// Required to include TLS support
#define CLOUD_USE_ADDP		// Required to include ADDP support

#define CLOUD_PRODUCT "cloud_simple_tls.c"
#define CLOUD_VENDOR "Digi International Inc."
#define CLOUD_VENDOR_ID "1234"
#define CLOUD_FIRMWARE_ID "1.01.00"
#define CLOUD_CONTACT "support@digi.com"
#define CLOUD_LOCATION "Planet Earth"
#define CLOUD_DESCRIPTION "Simple Device Cloud demo"
#define CLOUD_SERVER "my.devicecloud.com"

// Store non-volatile configuration data in the userID block, via the
// Simple UserID Block FileSystem.  You can use SUBFS to also store a limited
// amount of non-Device Cloud application configuration data.
#define CLOUD_USE_SUBFS
#define SUBFS_RESERVE_START 0
#define SUBFS_RESERVE_END 0

#define ADDP_PASSWORD	"rabbit"

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

/*
// Selectively enable the following debugging/diagnostic options when
// developing new applications.
#define CLOUD_DEBUG
#define DCRTCP_DEBUG
#define ADDP_DEBUG
#define PKTDRV_DEBUG

#define CLOUD_VERBOSE
#define DNS_VERBOSE
#define RABBITWEB_VERBOSE
#define TLS_VERBOSE
#define SSL_CERT_VERBOSE
*/
#define CLOUD_IFACE_VERBOSE	// This prints interface status when it changes.


// Required because we're using TLS, but not using any static Zserver resources.
#define SSPEC_NO_STATIC

#use "Device_Cloud.lib"





void main()
{
	int rc;

	if (cloud_init())
		exit(1);

	printf("Hit any key to print network interface status and MAC address\n\n");
   printf("Note: MAC is useful for manually adding device to Device Cloud.\n\n");

_restart:

	do {
		rc = cloud_tick();

		if (kbhit()) {
			getchar();
			ip_print_ifs();
         printf("MAC address: %02X%02X%02X:%02X%02X%02X\n",
            SysIDBlock.macAddr[0], SysIDBlock.macAddr[1], SysIDBlock.macAddr[2],
            SysIDBlock.macAddr[3], SysIDBlock.macAddr[4], SysIDBlock.macAddr[5]
            );
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
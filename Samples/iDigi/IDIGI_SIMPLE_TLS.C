/*

	Simple iDigi sample, with TLS security.

	This shows basic use of iDigi to view and change network settings.

	-----------------------------------------------------------------------
	Instructions:
	-----------------------------------------------------------------------

	Visit http://developer.idigi.com to create a test account if you have
	not already done so.  You can also access the iDigi documentation
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
	needed if you enable firmware updates by defining IDIGI_USE_RPU.
	_PRIMARY_STATIC_IP and related network configuration macros may also
	be defined to set an initial "factory default" configuration.  See
	tcpconfig.lib and the network programming manual for details.  The
	default iDigi network configuration uses DHCP, with a fallback to the
	given static IP address.

	When running, navigate to developer.idigi.com, log in, and add the
	board using the '+' button - this is only necessary the first time
	you run iDigi on a given board.  If you defined IDIGI_USE_ADDP, then the
	board will automatically appear in the list of devices which may be
	added.  When the board is added, you can double click on it to view
	and change the network configuration settings.

	Some configuration settings may cause the board to reboot.  This sample
	implements this by calling exit(0).  When using the Dynamic C debugger,
	you can simply hit F9 to run the program again (no need to recompile).
	In a real deployment, exit(0) causes the board to reboot.

	When changing the network configuration using iDigi, the configuration
	is saved in non-volatile storage and will be used next time the program
	(or any other iDigi sample) is run.  The macro settings you define
	such as _PRIMARY_STATIC_IP are only used the first time the iDigi samples
	are run.  Thereafter, it uses any settings set via the iDigi server,
	and saved in flash.

	If you accidentally configure a bad network setting (e.g. by setting a
	non-existent gateway address), then the board will try to use the
	last-known-good configuration if the "current" configuration cannot
	connect to the iDigi server.

	-----------------------------------------------------------------------
*/

// You can comment out either or both of the following, to make a more spartan
// demo.
#define IDIGI_USE_TLS		// Required to include TLS support
#define IDIGI_USE_ADDP		// Required to include ADDP support

#define IDIGI_PRODUCT "IDIGI SIMPLE_TLS.C"
#define IDIGI_VENDOR "Digi International Inc."
#define IDIGI_VENDOR_ID "1234"
#define IDIGI_FIRMWARE_ID "1.01.00"
#define IDIGI_CONTACT "support@digi.com"
#define IDIGI_LOCATION "Planet Earth"
#define IDIGI_DESCRIPTION "Simple iDigi demo"
#define IDIGI_SERVER "sd1-na.idigi.com"

// Store non-volatile configuration data in the userID block, via the
// Simple UserID Block FileSystem.  You can use SUBFS to also store a limited
// amount of non-iDigi application configuration data.
#define IDIGI_USE_SUBFS
#define SUBFS_RESERVE_START 0
#define SUBFS_RESERVE_END 0

#define ADDP_PASSWORD	"rabbit"

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

/*
// Selectively enable the following debugging/diagnostic options when
// developing new applications.
#define IDIGI_DEBUG
#define DCRTCP_DEBUG
#define ADDP_DEBUG
#define PKTDRV_DEBUG

#define IDIGI_VERBOSE
#define DNS_VERBOSE
#define RABBITWEB_VERBOSE
#define TLS_VERBOSE
#define SSL_CERT_VERBOSE
*/
#define IDIGI_IFACE_VERBOSE	// This prints interface status when it changes.


// Required because we're using TLS, but not using any static Zserver resources.
#define SSPEC_NO_STATIC

#use "idigi.lib"





void main()
{
	int rc;

	if (idigi_init())
		exit(1);

	printf("Hit any key to print network interface status and MAC address\n\n");
   printf("Note: MAC address is useful for manually adding device to iDigi.\n\n");

_restart:

	do {
		rc = idigi_tick();

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
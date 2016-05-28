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

	Simple Device Cloud sample, to show how to put data to the Device Cloud server.

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

	This sample puts a file to the Device Cloud server.  You can verify the file
	contents by going to the Management->Storage page, then double click on
	the device ID which corresponds to this board.  This will list any
	files uploaded to the server.


*/
#define IDIGI_USE_ADDP		// Required to include ADDP support
#define IDIGI_USE_DS			// Required to include Data Services support

// Uncomment the following to enable TLS security.  In this case, we also
// send the data to the server via a secure channel.
//#define IDIGI_USE_TLS		// Required to include TLS support

#define IDIGI_PRODUCT "cloud_put_data.c"
#define IDIGI_VENDOR "Digi International Inc."
#define IDIGI_VENDOR_ID "1234"
#define IDIGI_FIRMWARE_ID "1.01.00"
#define IDIGI_CONTACT "support@digi.com"
#define IDIGI_LOCATION "Planet Earth"
#define IDIGI_DESCRIPTION "Simple Device Cloud DS demo"
#define IDIGI_SERVER "my.devicecloud.com"

// Store non-volatile configuration data in the userID block, via the
// Simple UserID Block FileSystem.  You can use SUBFS to also store a limited
// amount of non-Device Cloud application configuration data.
#define IDIGI_USE_SUBFS
#define SUBFS_RESERVE_START 0
#define SUBFS_RESERVE_END 0


#define ADDP_PASSWORD	"rabbit"

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

/*
// Enable the following debugging/diagnostic options when
// developing new applications.
#define IDIGI_DEBUG
#define DCRTCP_DEBUG
#define ADDP_DEBUG
#define PKTDRV_DEBUG
#define IDIGI_VERBOSE
#define DNS_VERBOSE
#define RABBITWEB_VERBOSE
*/

#define IDIGI_IFACE_VERBOSE	// This prints interface status when it changes.

// Required only if using TLS, but not using any static Zserver resources.
#define SSPEC_NO_STATIC

#use "Device_Cloud.lib"

// The data we are going to send.  This is a simple string, but arbitrary
// data can be sent if the appropriate content type parameter is set in
// the cloud_upload() call.  Whatever the form of the data, it needs to be
// in memory, and unchanging for the duration of the PUT.
const char * test_data =
"This data has been put here by the Rabbit!\r\n";

const char * test_data2 =
"<MyXML>\n"
"<foo>This is the foo data</foo>\n"
"<bar><baz>12345</baz><qux parm=\"attribute value\">QUX data</qux></bar>\n"
"</MyXML>\n"
;

// Static DataSvcsState_t instances are required for each simultaneous PUT
// transfer.
DataSvcsState_t dss;
DataSvcsState_t dss2;

void main()
{
	int rc;
	int sendfile;

	if (cloud_init())
		exit(1);

	// cloud_init() does not necessarily gain a connection to the Device Cloud server,
	// hence we can't use cloud_upload() straight away.  If necessary, the following
	// loop can be used to wait for a connection.
	while (cloud_status() == IDIGI_COMING_UP)
		cloud_tick();

	if (cloud_status() != IDIGI_UP) {
		printf("Device Cloud failed to connect to server!\n");
		exit(1);
	}

	// Start sending the plaintext data
   // Append this data to the file each time
	rc = cloud_upload(&dss, "myFile.txt", "text/plain",
					test_data, strlen(test_data), IDIGI_DS_OPTION_APPEND);
	sendfile = 1;
	if (rc) {
		printf("Could not start PUT service #1! rc=%d\n", rc);
		if (rc == -EPERM)
			printf("...Data service not enabled.  On Device Cloud web UI, select\n" \
			       "  Configuration -> mgmtglobal -> Data Service Enabled.\n");
		else
			printf("...See documentation for cloud_upload().\n");
		sendfile = 0;
	}
   if (sendfile) {
	   // Start sending the XML data
	   // Replace the file each time
	   rc = cloud_upload(&dss2, "myFile.xml", "text/xml",
	               test_data2, strlen(test_data2), 0);

	   if (rc)
	      printf("Could not start PUT service #2! rc=%d\n", rc);
      else
      	sendfile |= 2;
   }

_restart:

	do {
		if (sendfile & 1) {
			rc = cloud_ds_tick(&dss);
			if (rc != -EAGAIN) {
				printf("\nPUT #1 completed, rc = %d\n", rc);
				if (rc / 100 == 2)	// 2xx code
					printf("Success!\n\n");
				else
					printf("Failed!\n\n");
				sendfile &= ~1;
			}
		}
		if (sendfile & 2) {
			rc = cloud_ds_tick(&dss2);
			if (rc != -EAGAIN) {
				printf("\nPUT #2 completed, rc = %d\n", rc);
				if (rc / 100 == 2)	// 2xx code
					printf("Success!\n\n");
				else
					printf("Failed!\n\n");
				sendfile &= ~2;
			}
		}

		// We must tick the server normally while waiting for data service
      // completion.
		rc = cloud_tick();

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
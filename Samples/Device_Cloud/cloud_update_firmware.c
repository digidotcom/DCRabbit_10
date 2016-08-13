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

	Simple Device Cloud sample.

	This shows basic use of Device Cloud to view and change network settings and
	update firmware.

	-----------------------------------------------------------------------
	Instructions:
	-----------------------------------------------------------------------

	It is recommended to run one of the other Device Cloud samples first.  Those
	samples have general instructions for connecting to an Device Cloud server,
	which are not repeated here.

	This looks rather long and complicated, however much of it is simply to
	show more details of the process.  Normally, the end-user view will be
	much simpler!

   0. Read the documentation for the Remote Program Update facility
      (Application Note 421 and/or the board_update.lib library source).
      Include any necessary configuration at the top of this program.
	1. Compile this program to a .bin file.  In the project options dialog,
      "Targetless" tab, select the appropriate board which you are
	   going to use.  Save the options and compile using Compile->Compile to
	   .bin file.
	2. Rename the .bin file (it will be called CLOUD_UPD_FIRMWARE.BIN in the
	   same directory as this sample code) to, say, NEW_FIRMWARE.BIN
	3. Change something in this sample that is visible in the Device Cloud user
	   interface.  For example, change CLOUD_DESCRIPTION to some other
	   string.  This is just so that you can see which firmware is actually
	   running.
	4. Compile this sample as normal to the target.
	5. Disconnect the programming cable, and reconnect just the "Diagnostic"
	   connector at the end of the cable.
	6. Start a terminal program, and direct it to the same COM port as used to
	   program the target, and set it to 115200bps, 8N1.
	7. Hit the reset button on the target, or power it off and on.
	8. You should see it connecting to the Device Cloud server (assuming you have
	   correctly configured the network).  The terminal program should show
	   some initial messages including the device ID.
	9. Connect to the Device Cloud server using a modern web browser such as Firefox
	   or IE7, and log into your test account.
	10. If the target board is not already in the list of devices, add it
	   using the '+' button.  Note that device discovery should locate the
	   target board using ADDP.  You can select it and it will be added
	   automatically to the Device Cloud account.
	11. Note the 'Description' field will contain the value you defined for
	   CLOUD_DESCRIPTION.  (If it doesn't, maybe you were running other samples
	   before, so try hitting the "refresh" button).
	12. Hit the "firmware" button.  The dialog box will allow you to specify a
	   file to upload to the target.  Select the firmware file you renamed in
	   step (2) and hit "Update Firmware".
	13. Observe, in the terminal window, that the board starts accepting the
	   new firmware image, and eventually reboots.  When it reboots, hit
	   "refresh" in the Device Cloud user interface.  You should see the original
	   description (i.e. "Update me!").
	14. You can also try uploading a bad file.  The target should reject it
	   and the upload will fail (unless, of course, you picked a valid .bin
	   file!).
*/

#define  STDIO_DEBUG_SERIAL   SADR
#define	STDIO_DEBUG_BAUD		115200
#define	STDIO_DEBUG_ADDCR


//#define CLOUD_USE_TLS			// Include security (TLS) support
#define CLOUD_USE_ADDP			// Include ADDP support
#define CLOUD_USE_RPU			// Required to include remote program update
										// This is not always sufficient.  Read the
										// Remote Program Update documentation to
										// properly configure your board for
										// firmware updates.  In particular, set
										// the appropriate BU_TEMP_USE_* macro

#define CLOUD_PRODUCT "cloud_update_firmware.c"
#define CLOUD_VENDOR "Digi International Inc."
#define CLOUD_VENDOR_ID "1234"
#define CLOUD_FIRMWARE_ID "1.01.00"
#define CLOUD_CONTACT "support@digi.com"
#define CLOUD_LOCATION "Planet Earth"
#define CLOUD_DESCRIPTION "Update me!"
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
// Enable the following debugging/diagnostic options when
// developing new applications.
#define CLOUD_DEBUG
#define DCRTCP_DEBUG
#define ADDP_DEBUG
#define PKTDRV_DEBUG
#define CLOUD_VERBOSE
#define DNS_VERBOSE
#define ARP_VERBOSE
#define UDP_VERBOSE
#define RABBITWEB_VERBOSE
*/

#define CLOUD_IFACE_VERBOSE	// This prints interface status when it changes.
#define EDP_VERBOSE 2	// This shows firmware upload progress.  Normally
								// this would not be defined.


// Required only if using TLS, but not using any static Zserver resources.
#define SSPEC_NO_STATIC

#use "Device_Cloud.lib"





void main()
{
	int rc;

	// cloud_init() does everything required to start the network and the
	// Device Cloud connection.
	if (cloud_init())
		exit(1);

_restart:

	do {
		// Insert any work your application needs to do at this point.

		// Drive Device Cloud and the network, until it indicates a special condition.
		rc = cloud_tick();
	} while (!rc);

	printf("Final rc = %d\n", rc);
	if (rc == -NETERR_ABORT) {
		// RCI reboot request was received.  Normally we would use this
		// to shut down cleanly and reboot the board.
		printf("Rebooting via exit(0)!\n");
		exit(0);
	}
	if (rc == -NETERR_NONE) {
		// Device network has been reconfigured by Device Cloud web interface etc.
		// The network is still up, so applications may use this opportunity
		// to shut down any in-progress connections cleanly.  The next call
		// to cloud_tick() will close down the network, reconfigure it,
		// then bring it back up.
		printf("Network reconfiguration in progress...\n");
	}
	goto _restart;

}
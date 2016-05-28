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

   Simple iDigi sample, to demonstrate FAT filesystem.

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
   default iDigi network configuration uses DHCP, with a fallback to the
   given static IP address.

   When running, navigate to https://devicecloud.digi.com/, log in, and add the
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


   To do something with this sample, go to the iDigi user interface
   and select the "Web Services Console".

   1. Seelct your target device using the SCI Targets button.

   2. Select Examples->SCI->File System Service->Upload File and the following
      XML will be available.

   3. Edit the path name to make the XML look like the following.  The
      critical thing is the initial "/A/" in the path name, since that
      is significant to the Rabbit filesystem.  And yes, it IS case sensitive.

   4. The file data can be changed (if you know Base64!) but the default is
      the string "file contents" when decoded.  It is always the decoded
      contents which are stored on the Rabbit.

<sci_request version="1.0">
  <file_system>
    <targets>
      <device id="00000000-00000000-0090C2FF-FF000000"/>
    </targets>
    <commands>
      <put_file path="/A/fooble.txt">
        <data>ZmlsZSBjb250ZW50cw==</data>
      </put_file>
    </commands>
  </file_system>
</sci_request>

	5. Hit the "Send" button: this will create a file called fooble.txt on
   	the first FAT partition (A).

   6. Then, you can retrieve the file using

<sci_request version="1.0">
  <file_system>
    <targets>
      <device id="00000000-00000000-0090C2FF-FF000000"/>
    </targets>
    <commands>
      <get_file path="/A/fooble.txt"/>
    </commands>
  </file_system>
</sci_request>

	7. Examine the results on the right hand upper pane (Web Services
   Responses).

   8. Other related filesystem requests can be set using the
   		Examples->SCI->File System Service
   	menu in the user interface, including listing and deleting files.
      You can put multiple elements inside the <command> element, in
      order to execute a sequence of operations.

*/
#define IDIGI_USE_FAT       // Required to include FAT filesystem support
//#define IDIGI_USE_ADDP     // Uncomment to include ADDP support

// Uncomment the following to enable TLS security.  In this case, we also
// send/receive file data via a secure channel.
//#define IDIGI_USE_TLS

#define IDIGI_PRODUCT "cloud_fat.c"
#define IDIGI_VENDOR "Digi International Inc."
#define IDIGI_VENDOR_ID "1234"
#define IDIGI_FIRMWARE_ID "1.01.00"
#define IDIGI_CONTACT "support@digi.com"
#define IDIGI_LOCATION "Planet Earth"
#define IDIGI_DESCRIPTION "Simple iDigi FAT demo"
#define IDIGI_SERVER "my.devicecloud.com"

// Store non-volatile configuration data in the userID block, via the
// Simple UserID Block FileSystem.  You can use SUBFS to also store a limited
// amount of non-iDigi application configuration data.
#define IDIGI_USE_SUBFS
#define SUBFS_RESERVE_START 0
#define SUBFS_RESERVE_END 0


#define ADDP_PASSWORD   "rabbit"

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

/*
// Selectively enable the following debugging/diagnostic options when
// developing new applications.
#define IDIGI_DEBUG
#define ZSERVER_DEBUG
#define IDIGI_VERBOSE
#define ZSERVER_VERBOSE
#define DCRTCP_DEBUG
#define ADDP_DEBUG
#define PKTDRV_DEBUG
#define DNS_VERBOSE
#define RABBITWEB_VERBOSE
*/

#define IDIGI_IFACE_VERBOSE   // This prints interface status when it changes.

// Required only if using TLS or FAT, but not using any static Zserver resources.
#define SSPEC_NO_STATIC

#use "Device_Cloud.lib"


void main()
{
   int rc;

   if (idigi_init())
      exit(1);

   // Add a rule to allow the iDigi server to access everything on the
   // first partition.  idigi_get_group() returns the group mask of
   // the special "idigi" user ID.  (Without this rule, iDigi won't be
   // able to access any files).
   sspec_addrule("/A/", NULL,
   			idigi_get_group(), idigi_get_group(), SERVER_IDIGI, 0, NULL);

_restart:

   do {
      rc = idigi_tick();
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
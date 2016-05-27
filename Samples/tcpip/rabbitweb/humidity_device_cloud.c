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
/*******************************************************************************
        Samples\tcpip\RabbitWeb\humidity_device_cloud.c

        This works exactly the same as humidity.c, except that Device Cloud
        functionality is included to take care of network configuration.
        See the various samples in Samples\Device_Cloud for more details.

*******************************************************************************/

// This macro used in this sample to select iDigi functionality.  If commented
// out, then works exactly like humidity.c (i.e. just the web interface)
#define USE_IDIGI

// This is needed to pull in the ZHTML interpreter code.
#define USE_RABBITWEB 1

#ifdef USE_IDIGI

	#define IDIGI_PRODUCT "humidity_device_cloud.c"
	#define IDIGI_VENDOR "Digi International Inc."
	#define IDIGI_VENDOR_ID "1234"
	#define IDIGI_FIRMWARE_ID "1.01.00"
	#define IDIGI_CONTACT "support@digi.com"
	#define IDIGI_LOCATION "Planet Earth"
	#define IDIGI_DESCRIPTION "Simple iDigi demo"
	#define IDIGI_SERVER "my.devicecloud.com"

	// Store non-volatile configuration data in the userID block, via the
	// Simple UserID Block FileSystem.  You can also use SUBFS to store a limited
	// amount of non-iDigi application configuration data.
	#define IDIGI_USE_SUBFS
	#define SUBFS_RESERVE_START 0
	#define SUBFS_RESERVE_END 0

	#define ADDP_PASSWORD   "rabbit"
	#define IDIGI_IFACE_VERBOSE   // This prints interface status when it changes.

	#use "Device_Cloud.lib"

#else
// not USE_IDIGI...

	/*
	 * NETWORK CONFIGURATION
	 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
	 * compile-time network configuration.
	 */
	#define TCPCONFIG 1

	#use "dcrtcp.lib"

#endif	// USE_IDIGI


#use "http.lib"

/*
 * These pages contain the ZHTML portion of the demonstration.  The first has
 * a status page, while the second has a configuration interface.
 */
#ximport "samples/tcpip/rabbitweb/pages/humidity_monitor.zhtml"	monitor_zhtml
#ximport "samples/tcpip/rabbitweb/pages/humidity_admin.zhtml"		admin_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", monitor_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/admin/index.zhtml", admin_zhtml)
SSPEC_RESOURCETABLE_END

// The following line defines an "admin" group, which will be used to protect
// certain variables.  This must be defined before we register the variables
// below.
#web_groups admin

// The #web lines below register C variables with the web server, such that
// they can be used with the RabbitWeb HTTP server enhancements.

// The following lines creates a "hum" variable to keep the current humidity
// value, and allows read-only access to it.
int hum;
#web hum groups=all(ro)

// This creates a humidity alarm variable, which can be modified by the admin
// group, but is only readable by everybody else.
int hum_alarm;
#web hum_alarm ((0 < $hum_alarm) && ($hum_alarm <= 100)) groups=all(ro),admin

// The following two sets of registrations are for an alarm interval variable
// and an alarm email address.
int alarm_interval;
char alarm_email[50];
#web alarm_interval ((0 < $alarm_interval) && ($alarm_interval < 30000)) \
                    groups=all(ro),admin
#web alarm_email groups=all(ro),admin

void main(void)
{
   int userid;

	// Initialize the values
	hum = 50;
	hum_alarm = 75;
	alarm_interval = 60;
	strcpy(alarm_email, "somebody@nowhere.org");

#ifdef USE_IDIGI
	// Start iDigi services
	if (idigi_init())
		exit(1);
#else
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
#endif
   http_init();

   // If the browser specifies a directory (instead of a proper resource name)
   // default to using "index.zhtml" in that directory, if it exists.
   // If we don't use this function, the default is "index.html" which won't
   // work for this sample.
   http_set_path("/", "index.zhtml");

	tcp_reserveport(80);

	// The following line limits access to the "/admin" directory to the admin
	// group.  It also requires basic authentication for the "/admin"
	// directory.
   sspec_addrule("/admin", "Admin", admin, admin, SERVER_ANY,
                 SERVER_AUTH_BASIC, NULL);
	// The following two lines create an "admin" user and adds it to the admin
	// group.
   userid = sauth_adduser("harpo", "swordfish", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);

	// This drives the HTTP server.
   while(1) {
#ifdef USE_IDIGI
		idigi_tick();
#endif
      http_handler();
   }
}


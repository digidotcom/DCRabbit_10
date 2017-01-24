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
        Samples\TcpIp\HTTP\static_device_cloud.c

        This works exactly the same as static.c, except that Device Cloud
        functionality is included to take care of network configuration.
        See the various samples in Samples\Device_Cloud for more details.
*******************************************************************************/

// This macro used in this sample to select Device Cloud functionality.  If commented
// out, then works exactly like static.c (i.e. just the web interface)
#define USE_DEVICE_CLOUD

/*
 *  The TIMEZONE compiler setting gives the number of hours from
 *  local time to Greenwich Mean Time (GMT).  For pacific standard
 *  time this is -8.  Note:  for the time to be correct it must be set
 *  with tm_rd which is documented in the Dynamic C user manual.
 */

#define TIMEZONE        -8


#ifdef USE_DEVICE_CLOUD

	#define CLOUD_PRODUCT "static_device_cloud.c"
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

	#define ADDP_PASSWORD   "rabbit"
	#define CLOUD_IFACE_VERBOSE   // This prints interface status when it changes.
#define CLOUD_VERBOSE

	#use "Device_Cloud.lib"

#else
// not USE_DEVICE_CLOUD...

	#define TCPCONFIG 1
	#use "dcrtcp.lib"

#endif

#use "http.lib"

/*
 *  ximport is a Dynamic C language feature that takes the binary image
 *  of a file, places it in extended memory on the controller, and
 *  associates a symbol with the physical address on the controller of
 *  the image.
 *
 */

#ximport "samples/tcpip/http/pages/static.html"    index_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"    rabbit1_gif

/*
 *  http_types gives the HTTP server hints about handling incoming
 *  requests.  The server compares the extension of the incoming
 *  request with the http_types list and returns the second field
 *  as the Content-Type field.  The third field defines a custom
 *  function to handle that mime type.
 *
 *  You can get a list of mime types from Netscape's browser in:
 *
 *  Edit->Preferences->Navigator->Applications
 *
 */

/* the default mime type for files without an extension must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF)
SSPEC_MIMETABLE_END

/*
 *  The resource table associates ximported files with URLs on the webserver.
 */

SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif)
SSPEC_RESOURCETABLE_END

void main()
{

	/*
	 *  sock_init initializes the TCP/IP stack.
	 *  http_init initializes the web server.
	 */

#ifdef USE_DEVICE_CLOUD
	// Start Device Cloud services
	if (cloud_init())
		exit(1);
#else
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
#endif
   http_init();

	/*
	 *  tcp_reserveport causes the web server to ignore requests when there
	 *  isn't an available socket (HTTP_MAXSERVERS are all serving index_html
	 *  or rabbit1.gif).  This saves some memory, but can cause the client
	 *  delays when retrieving pages.
	 */

   tcp_reserveport(80);

	/*
	 *  http_handler needs to be called to handle the active http servers.
	 */

   while (1) {
#ifdef USE_DEVICE_CLOUD
		cloud_tick();
#endif
      http_handler();
   }
}
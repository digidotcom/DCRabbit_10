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
        Samples\FileSystem\updpages.c

        Requires that you run this on a board with a compatible
        storage medium (serial flash, NAND flash or SD card).

        This sample shows how to combine HTTP, FTP and zserver functionality
        to create an application with web content which can be updated via FTP.

        This defines some static and dynamic resources, but you can also use
        FTP to upload completely new resources to the FAT filesystem.

        Run this, then issue an FTP command

          ftp 10.10.6.100

        (or whatever IP address you configure).

        Log in as "root" with password "super".  You can use FTP "put"
        commands to upload files.  Then, use a web browser to confirm
        that the new files are accessible via HTTP.

*******************************************************************************/
/*
 *    By default, have compiler make function variables storage class
 *    "auto" (allocated on the stack).
 */
#class auto

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1


/*
 *  The TIMEZONE compiler setting gives the number of hours from
 *  local time to Greenwich Mean Time (GMT).  For pacific standard
 *  time this is -8.  Note:  for the time to be correct it must be set
 *  with tm_rd which is documented in the Dynamic C user manual.
 */

#define TIMEZONE        -8


/********************************
 * End of configuration section *
 ********************************/

/*
 *  memmap forces the code into xmem.  Since the typical stack is larger
 *  than the root memory, this is commonly a desirable setting.  Another
 *  option is to do #memmap anymem 8096 which will force code to xmem when
 *  the compiler notices that it is generating within 8096 bytes of the
 *  end.
 *
 *  #use the Dynamic C TCP/IP stack library and the HTTP application library
 */
#memmap xmem


#define INPUT_COMPRESSION_BUFFERS 4
#use "zimport.lib"


//	#define FAT_DEBUG
//	#define FAT_VERBOSE

// Necessary for zserver use of FAT filesystem
#define FAT_USE_FORWARDSLASH

// Set FAT library to blocking mode
#define FAT_BLOCK

#use "fat16.lib"

/*
 * Some GUI FTP clients make multiple connections, so allow for three at a
 * time.  This supports the FileZilla default setting of one connection for
 * directory listings and two simultanous file transfers.
 */
#define FTP_MAXSERVERS 3

/*
 * Each FTP connection uses two TCP sockets, one for control and one for
 * data.  Configure the TCP/IP stack with enough pre-allocated buffers for
 * two sockets per FTP Server connection, plus two for HTTP.
 */
#define MIN_TCP_SOCKET_BUFFERS (2 * FTP_MAXSERVERS + 2)

#if defined(MAX_TCP_SOCKET_BUFFERS) && MAX_TCP_SOCKET_BUFFERS < MIN_TCP_SOCKET_BUFFERS
	#undef MAX_TCP_SOCKET_BUFFERS
#endif
#ifndef MAX_TCP_SOCKET_BUFFERS
	#define MAX_TCP_SOCKET_BUFFERS MIN_TCP_SOCKET_BUFFERS
#endif

//#define ZSERVER_DEBUG
//#define ZSERVER_VERBOSE
//#define HTTP_DEBUG
//#define HTTP_VERBOSE
//#define FTP_DEBUG
//#define FTP_VERBOSE

#use "dcrtcp.lib"
#use "http.lib"
#use "ftp_server.lib"

/*
 *  ximport is a Dynamic C language feature that takes the binary image
 *  of a file, places it in extended memory on the controller, and
 *  associates a symbol with the physical address on the controller of
 *  the image.
 *
 */

#ximport "samples/tcpip/http/pages/static.html"    index_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"    rabbit1_gif
#zimport "samples/tcpip/http/pages/zimport.shtml"			zimport_shtml
#zimport "samples/tcpip/http/pages/alice-rabbit.jpg"		alice_jpg

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

// For new applications, use the following macros instead of the old
// structure initializations...
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".shtml", "text/html", shtml_handler),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".jpg", "image/jpeg"),
	SSPEC_MIME(".gif", "image/gif")
SSPEC_MIMETABLE_END


/*
 *  Define user groups, and static resource table
 */

#define USER_GROUP	0x0001
#define ADMIN_GROUP	0x0002
#define ALL_GROUPS	(USER_GROUP|ADMIN_GROUP)
#define NO_GROUPS		0

// The flash resource table is now initialized with these macros...
// All user groups have read access, but none have write (even if write access was allowed,
// it would still not work because these resources are in flash memory).
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_P_XMEMFILE("/index.html", index_html, \
	         "static", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD),
	SSPEC_RESOURCE_P_ZMEMFILE("/index.shtml", zimport_shtml, \
	         "static", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD),
	SSPEC_RESOURCE_P_XMEMFILE("/rabbit1.gif", rabbit1_gif, \
	         "static", ALL_GROUPS, NO_GROUPS, SERVER_ANY, SSPEC_DEFAULT_METHOD)
SSPEC_RESOURCETABLE_END

long text_size;
long image_size;

void main()
{
	auto char buf[128];
   auto int rc, i, uid, handle;

	text_size = 12345;
	image_size = xgetlong(alice_jpg) & ZIMPORT_MASK;

	printf("Initializing filesystems...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
   if (rc)
   	printf("Failed to initialize, rc=%d\nProceeding anyway...\n", rc);

	sspec_addxmemfile("/alice.jpg", alice_jpg, SERVER_HTTP | SERVER_COMPRESSED);
	sspec_addvariable("text_size", &text_size, INT32, "%ld", SERVER_HTTP);
   sspec_addvariable("image_size", &image_size, INT32, "%ld", SERVER_HTTP);



	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

   /*
    *  http_init initializes the web server.
    *  ftp_init initializes the FTP server.
    */

   http_init();
   ftp_init(NULL);

   // Create a permissions rule for FAT
   sspec_addrule("/A", "fat-A-realm", ALL_GROUPS, ADMIN_GROUP, SERVER_ANY, 0, NULL);
   sspec_addrule("/E", "fat-E-realm", ALL_GROUPS, ADMIN_GROUP, SERVER_ANY, 0, NULL);

   // Add users and ensure users are in the correct group(s).
   uid = sauth_adduser("root", "super", SERVER_ANY);
   sauth_setwriteaccess(uid, SERVER_ANY);
   sauth_setusermask(uid, ALL_GROUPS, NULL);

   uid = sauth_adduser("admin", "work", SERVER_HTTP | SERVER_FTP);
   sauth_setwriteaccess(uid, SERVER_HTTP | SERVER_FTP);
   sauth_setusermask(uid, ADMIN_GROUP, NULL);

   uid = sauth_adduser("anonymous", "", SERVER_FTP);
   sauth_setusermask(uid, USER_GROUP, NULL);
   ftp_set_anonymous(uid);	// This FTP user does not require password, but cannot write anything

   uid = sauth_adduser("foo", "bar", SERVER_HTTP);
   sauth_setusermask(uid, USER_GROUP, NULL);

	// First, let's list the current working directory as seen by the 1st HTTP server instance.
   // The CWD for HTTP is always the root directory.
   printf("Root directory listing for the HTTP server...\n");
   for (handle = 0; handle >= 0; handle >= 0 ? printf(buf) : 0)
      handle = sspec_dirlist(handle, buf, sizeof(buf),
                                    http_getcontext(0), SSPEC_LIST_LONG);

   printf("\n");


   /*
    *  tcp_reserveport causes the web server to ignore requests when there
    *  isn't an available socket (HTTP_MAXSERVERS are all serving index_html
    *  or rabbit1.gif).  This saves some memory, but can cause the client
    *  delays when retrieving pages.
    */

   tcp_reserveport(80);

   /*
    *  http_handler needs to be called to handle the active http servers.
    *  ftp_tick needs to be called to handle the active FTP servers.
    */

   printf("Press any key to safely shut-down server.\n");

   while (1) {
      http_handler();
      ftp_tick();
      if (kbhit())
      {
      	// Unmount all of the mounted FAT partitions & devices before exit
  	   	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS;
     	   											 i += FAT_MAX_PARTITIONS) {
	      	if (fat_part_mounted[i]) {
	      		fat_UnmountDevice(fat_part_mounted[i]->dev);
   	   	}
      	}
        	exit(rc);
      }
   }
}
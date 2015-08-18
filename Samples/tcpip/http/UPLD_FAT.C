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
        Samples\TCPIP\HTTP\UPLD_FAT.C

        Demonstrate the HTTP file upload facility, using the default
        handlers.  This sample uses the FAT filesystem so that there is
        somewhere we can save the uploaded file.
        The login to upload files is user: admin  password: upload
*******************************************************************************/
#class auto

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 *  This value is required to enable FAT blocking mode for ZServer.lib
 */
#define FAT_BLOCK

// Necessary for zserver.
#define FAT_USE_FORWARDSLASH

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1


/*
 * TCP/IP modification - reduce TCP socket buffer
 * size, to allow more connections. This can be increased,
 * with increased performance, if the number of sockets
 * are reduced.  Note that this buffer size is split in
 * two for TCP sockets--1024 bytes for send and 1024 bytes
 * for receive.
 */
#define TCP_BUF_SIZE 2048

/*
 * Web server configuration
 */

/*
 * only one socket and server are needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1
#define MAX_UDP_SOCKET_BUFFERS 1

/*
#define HTTP_VERBOSE
#define ZSERVER_VERBOSE
#define HTTP_DEBUG
#define ZSERVER_DEBUG
#define DCRTCP_DEBUG
#define REMOTEFS_DEBUG
*/

//#define HTTP_TIMEOUT		6


/********************************
 * End of configuration section *
 ********************************/

#define USE_HTTP_UPLOAD		// Required for this demo, to include upload code.

#define DISABLE_DNS		// No name lookups required

// Define this because we are using a static rule table.  Flash rules, OK?
#define SSPEC_FLASHRULES

#memmap xmem

#use "dcrtcp.lib"
#use "fat16.lib"		// Must use FAT before http/zserver
#use "http.lib"


#ximport "samples/tcpip/http/pages/upload.html"    index_html


// Define a group bit for updating resources...
#define ADMIN_GROUP	0x0002

// This table maps file extensions to the appropriate "MIME" type.  This is
// needed for the HTTP server.
SSPEC_MIMETABLE_START
	SSPEC_MIME(".htm", MIMETYPE_HTML),
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END

// This is the access permissions "rule" table.  It associates userid
// information with files in the FAT filesystem.  This is necessary because
// FAT filesystems do not support the concept of owner userids.  Basically,
// this is a simple prefix-matching algorithm.
SSPEC_RULETABLE_START
	// You need to be in user group 0x0002 to write resources starting with "/A/new" --
   // in this case, /A/new.htm and /A/new2.htm.  The 2nd parameter ("newPages") is
   // used as the resource realm, which your browser will use to prompt you for a
   // userid and password.  Only the HTTP server can access these, and basic (i.e. plaintext)
   // user/password authentication is specified.  Everyone can read these pages (0xFFFF),
   // but only users in group ADMIN_GROUP can write them.
	SSPEC_MM_RULE("/A/new", "newPages", 0xFFFF, ADMIN_GROUP, SERVER_HTTP, SERVER_AUTH_BASIC, NULL)
SSPEC_RULETABLE_END


// The flash resource table is now initialized with these macros...
SSPEC_RESOURCETABLE_START
// "/index.html" - this is the web page which is initially presented.  It contains the
//    form to be filled out.
SSPEC_RESOURCE_XMEMFILE("/index.html", index_html),
// "upload.cgi" - resource name as specified in the HTML for handling the POST data
// http_defaultCGI - name of the default upload CGI handler in HTTP.LIB
// "newPages" - the realm associated with upload.cgi
// ADMIN_GROUP - read permission must be limited to this group.  The CGI itself only needs read permission,
//          but it should be the same group bits as specified for the write permission for the
//          target resource files (see ruletable entry above).
// 0x0000 - no write permission.  Writing to a CGI does not make sense, only writing to files.
// SERVER_HTTP - only the HTTP server accesses CGIs.
// SERVER_AUTH_BASIC - should use the same authentication technique as for the files.
SSPEC_RESOURCE_P_CGI("upload.cgi", http_defaultCGI,
                       "newPages", ADMIN_GROUP, 0x0000, SERVER_HTTP, SERVER_AUTH_BASIC)
SSPEC_RESOURCETABLE_END


void main()
{
	int rc, i;
	char buf[20];
   int uid;

	printf("Initializing network...\n");
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	printf("Initializing filesystem...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
	// Verify that PARTITION_A (i.e partition 0 on device 0) is mounted.
	if (NULL == PARTITION_A)
	{
		// This sample program cannot succeed without access to PARTITION_A.
		printf("The required PARTITION_A was not mounted; exiting now.\n");
		exit(rc);
	}
   if (rc)
   	printf("Failed to initialize, rc=%d\nProceeding anyway...\n", rc);

   printf("Setting up userids...\n\n");
   // Create a user ID
   uid = sauth_adduser("admin", "upload", SERVER_HTTP);
   if (uid < 0)
   	printf("Failed to create userid, rc=%d\nProceeding anyway...\n", uid);
	else {
	   // Ensure that that user has a group mask of ADMIN_GROUP i.e. this user is a member of "ADMIN".
	   sauth_setusermask(uid, ADMIN_GROUP, NULL);
      sauth_setwriteaccess(uid, SERVER_HTTP);	// Also need to assign individual write access.
      printf("Userid created successfully: use 'admin' with password 'upload'\n\n");
   }

   http_init();
   tcp_reserveport(80);

   printf("Ready: point your browser to http://%s/\n\n", inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
   printf("\nPress any key to bring down the server cleanly.\n");

   while (1) {
      http_handler();
      if (kbhit())
      {
		   // Cycle through all partitions and unmount as needed
		   for(i = 0; i < SSPEC_MAX_PARTITIONS; i++) {
		   	// See if the partition is registered
    			if (sspec_fatregistered(i)) {
		      	// The partition was registered, lets unmount it
					fat_UnmountDevice(sspec_fatregistered(i)->dev);
		      }
		   }
         exit(0);
      }
   }
}










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
  FAT_SERVE2.C

  Sample program to demonstrate use of ZSERVER.LIB and FAT
  filesystem functionality.

  This is the 2nd of a 2-part sample.  The first part (which
  you should have run already) is FAT_SETUP.C

  This sample sets up a web server which accesses the files
  which were copied to the FAT filesystem by the previous
  sample.

  It is similar to FAT_SERVE.C, except that access permissions
  have been added.

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
 *  This value is required to enable FAT blocking mode for ZServer.lib
 */
#define FAT_BLOCK

// Necessary for zserver.
#define FAT_USE_FORWARDSLASH
#define HTTP_NO_FLASHSPEC	// It's all done from the filesystem

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


/********************************
 * End of configuration section *
 ********************************/

// Define this because we are using a static rule table
#define SSPEC_FLASHRULES

//#define ZSERVER_DEBUG
//#define ZSERVER_VERBOSE

#memmap xmem
#use "fat16.lib"		// Must use FAT before http/zserver
#use "dcrtcp.lib"
#use "http.lib"


// This is the access permissions "rule" table.  It associates userid
// information with files in the FAT filesystem.  This is necessary because
// FAT filesystems do not support the concept of owner userids.  Basically,
// this is a simple prefix-matching algorithm.
#define ALICES_GROUP 0x0002

SSPEC_RULETABLE_START
	// You need to be in user group ALICES_GROUP to retrieve resources starting with "/A/alice" --
   // in this case, /A/alice.htm and /A/alice.jpg.  The 2nd parameter ("wonderland") is
   // used as the resource realm, which your browser will use to prompt you for a
   // userid and password.  Only the HTTP server can retrieve these, and basic (i.e. plaintext)
   // user/password authentication is specified.
	SSPEC_MM_RULE("/A/alice", "wonderland", ALICES_GROUP, 0, SERVER_HTTP, SERVER_AUTH_BASIC, NULL)
SSPEC_RULETABLE_END

// This table maps file extensions to the appropriate "MIME" type.  This is
// needed for the HTTP server.
SSPEC_MIMETABLE_START
	SSPEC_MIME(".htm", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
	SSPEC_MIME(".jpg", "image/jpeg")
SSPEC_MIMETABLE_END


int main()
{
	int rc;
   char buf[20];
   int uid;

	printf("Initializing filesystem...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
   if (rc)
   	printf("Failed to initialize, rc=%d\nProceeding anyway...\n", rc);

   printf("Setting up userids...\n");
   // Create a user ID
   uid = sauth_adduser("alice", "duchess", SERVER_HTTP);
	// Ensure that that user has a group mask of ALICES_GROUP
   sauth_setusermask(uid, ALICES_GROUP, NULL);

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
   http_set_path("/A/", "static.htm");	// Set a root directory (the FAT first partition) and
   												// default resource name.
	tcp_reserveport(80);

   printf("Now try connecting via your web browser.\n");
   printf("Try a URL of http://%s/\n", inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
   printf("\nPress any key to bring down the server cleanly.\n");

   while (1) {
      http_handler();
      if (kbhit())
      {
		   // You should always unmount the device on exit to flush cache entries
			fat_UnmountDevice(sspec_fatregistered(0)->dev);
        	exit(0);
      }
   }
	return 0;
}
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
        ftp_fat.c

        Demonstration of a simple FTP server, using the ftp library, that
        allows file uploads and deletion.  This sample uses the FAT library
        to store and retrieve files on a mass storage device (NAND flash,
        serial flash, SD card) connected to the Rabbit.

        If the FAT device already has files on it, then the files will
        be visible in this sample.  File and directory names are limited
        to standard 8.3 file name/extension limitations.  The root
        directory of the first FAT partition is referenced as directory
        "/A" and subsequent partitions on the same device will be B, C
        and D respectively.  If the module has two storage devices, the
        partitions on the second device will be directories E, F, G and H.

        The user "anonymous" may download the file "rabbitA.gif", but
        not "rabbitF.gif".  The user "foo" (with password "bar") may
        download "rabbitF.gif", and also "rabbitA.gif".  User "foo" can
        also upload files, which will be stored in the FAT filesystem.

        When a file is uploaded, you must call it
        /A/filename.ext
        where A is the directory reference for the partition and
              filename.ext meets standard 8.3 naming conventions

        something like
          put  my.file  /A/myfile.txt
        or maybe
          cd /A
          put  my.file  myfile.txt

        Each user may execute the "dir" or "ls" command to see a listing of
        the available files.  The listing shows only the files that the
        logged-in user can access.

        When you initially connect, an ls command will show a directory
        called /A as well as the memory based files.  You can CD to that
        directory in order to list the FAT files therein (and upload new
        ones if you are the foo user).

        This sample also shows how to create an xmem file resource
        without using #ximport.  A small "README" file is generated.

        You should always exit this sample by pressing a key on the
        keyboard.  This brings the sample down politely and insures
        that any unwritten cache entries are committed to the storage
        device.  Since the FAT filesystem uses a cache layer this is a
        very important step.  It should also be considered when you
        write your own application that a polite exit path is needed
        by the FAT filesystem.

*******************************************************************************/
#class auto

/*
 *  This value is required to enable FAT blocking mode for ZServer.lib
 */
#define FAT_BLOCK

// Necessary for zserver.
#define FAT_USE_FORWARDSLASH
#define HTTP_NO_FLASHSPEC	// It's all done from the filesystem

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your        *
 * preferences.                    *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

/*
 * FTP and FAT configuration
 *
 * See the sspec_* and sauth_* functions below to see how to set up
 * files and users for the FTP server.
 */

/*
 * Some GUI FTP clients make multiple connections, so allow for three at a
 * time.  This supports the FileZilla default setting of one connection for
 * directory listings and two simultanous file transfers.
 */
#define FTP_MAXSERVERS 3

/*
 * Each FTP connection uses two TCP sockets, one for control and one for
 * data.  Configure the TCP/IP stack with enough pre-allocated buffers for
 * two sockets per FTP Server connection.
 */
#define MIN_TCP_SOCKET_BUFFERS (2 * FTP_MAXSERVERS)

/*
 * This is the size of the structure that keeps track of files for
 * the FTP server in the dynamic resource table.  Unlike the
 * ftp_server_full.c sample, there is no need to include one
 * count for each FAT file, since zserver manages these without
 * requiring an additional dynamic resource table entry.  This is
 * set just high enough to contain the "fixed" entries added in
 * main().
 */
#define SSPEC_MAXSPEC 10

/*
 * This must be defined to notify the library that there is no
 * static resource table (used to be known as "flashspec").
 * This macro is required for FTP-only applications since
 * Dynamic C 8.50.
 */
#define SSPEC_NO_STATIC

/*
 * Define this because we are using a static rule table.
 */
#define SSPEC_FLASHRULES


/*
 * Optionally define debugging macros (for single-stepping in library)
 */

#define FTP_DEBUG
#define ZSERVER_DEBUG

/********************************
 * End of configuration section *
 ********************************/

#if defined(MAX_TCP_SOCKET_BUFFERS) && MAX_TCP_SOCKET_BUFFERS < MIN_TCP_SOCKET_BUFFERS
	#undef MAX_TCP_SOCKET_BUFFERS
#endif
#ifndef MAX_TCP_SOCKET_BUFFERS
	#define MAX_TCP_SOCKET_BUFFERS MIN_TCP_SOCKET_BUFFERS
#endif

#memmap xmem
#use "fat16.lib"
#use "dcrtcp.lib"
#use "ftp_server.lib"

#ximport "samples/tcpip/http/pages/rabbit1.gif" rabbit1_gif


/* The empty string at the end is used to calculate string length. */
xstring xtext { "Welcome to the Rabbit FTP server.  Download a file.\r\n", "" };


/**
 * 	Duplicate a string with a four-byte size in front of it.
 * 	Allocates xmem to store size and string.  The "xstring"
 * 	directive does NOT store the size.  To use sspec_addxmemfile(),
 * 	this is required.  Thus the copy which prepends a four-byte size
 * 	value (not including NUL char).  The final xmem2mem() copies the
 * 	string just after the size.
 *
 * 	RETURN: 	physical addr of 4 byte size (string follows, without
 * 			its NULL char termination).
 */
long
xmem_strdup( long xstr )
{
	auto long	xsrc, xnext;
	auto long 	siz;
	auto long 	xdest;

	xmem2root( & xsrc, xstr+0, 4 );
	xmem2root( & xnext, xstr+4, 4 );
	siz = xnext - xsrc - 1;   					// Don't store NULL char.

	xdest = xalloc( (int)siz + 4 );
	root2xmem( xdest, & siz, 4 );
	xmem2xmem( xdest+4, xsrc, (int)siz );

	return( xdest );
}   /* end xmem_strdup() */

/* -------------------------------------------------------------------- */


#define ANON_GROUP	0x0001		// Group mask for anonymous user(s)
#define FOO_GROUP		0x0002		// Group mask for user(s) which can perform uploads

// This is the access permissions "rule" table.  It associates userid
// information with files in the filesystem.  This is necessary because
// FAT does not support the concept of owner userids.  Basically,
// this is a simple prefix-matching algorithm.
SSPEC_RULETABLE_START
	// You need to be in user group FOO_GROUP to write resources starting with "/A"
   // The directory "/A" links to the root directory of the first FAT partition.
   // Everyone can read these files (0xFFFF), but only users in group FOO_GROUP can
   // write (or create) them.
	SSPEC_MM_RULE("/A", "foo", 0xFFFF, FOO_GROUP, SERVER_FTP, SERVER_AUTH_BASIC, NULL),
   	// Note: the 2nd string parameter is not relevant to FTP, however it gets
      // printed in the "group" field of the directory listing.  Thus, we may as
      // well set it to "foo".  If set to "" or NULL, then the group field prints
      // as "anon" which may be confusing to the user (since it is not related to
      // the anonymous user).
      // SERVER_AUTH_BASIC means plaintext userid/password matching - this is the only
      // authentication currently supported by FTP.

   // Just for fun, we add another rule for files starting with "/A/file9".  This
   // allows write-only access i.e. foo can upload it, but nobody can read it back.
	SSPEC_MM_RULE("/A/file9", "foo", 0, FOO_GROUP, SERVER_FTP, SERVER_AUTH_BASIC, NULL),

SSPEC_RULETABLE_END


void main()
{
	int anon_user, foo_user, i;
	faraddr_t  xdest;
	long len;
	char buf[20];

	// Note: sspec_automount automatically initializes all known filesystems.
   // We assume that the first partition on the device will be a valid FAT12
   // or FAT16 partition which will be mounted on '/A'.
   if (sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL)) {
   	printf("Failed to setup FAT.  Please refer to FAT samples/documentation.\n");
   	exit(1);
   }

   /*
    * Setup dynamic resources & users.  These have nothing to do with FAT files.
    */

	// Set up anonymous user
	anon_user = sauth_adduser("anonymous", "", SERVER_FTP);
	sauth_setusermask(anon_user, ANON_GROUP, NULL);
   // FTP makes special arrangements for this user...
	ftp_set_anonymous(anon_user);

   // Set up foo user.
	foo_user = sauth_adduser("foo", "bar", SERVER_FTP);
	sauth_setusermask(foo_user, FOO_GROUP, NULL);
	sauth_setwriteaccess(foo_user, SERVER_FTP);	  // This allows write access

	// Set up the first few files.  The sspec_setuser() function has slightly
   // changed semantics from DC8.50.  It sets the access permissions to be
   // readable to all groups which the specified user is a member of (and
   // only those).  Write permissions denied to all.
   // Note that we have initially set each user to be in its own, unique, group.
	sspec_setuser(sspec_addxmemfile("rabbitA.gif", rabbit1_gif, SERVER_FTP), anon_user);
	sspec_setuser(sspec_addxmemfile("test1", rabbit1_gif, SERVER_FTP), anon_user);
	sspec_setuser(sspec_addxmemfile("test2", rabbit1_gif, SERVER_FTP), anon_user);

	/*
	 *  This shows how to set up an xmem file (without #ximport).
	 *  Copy the string from one place in XMEMORY to another.
	 */
	xdest = xmem_strdup( xtext );
	sspec_setuser( sspec_addxmemfile( "README", xdest, SERVER_FTP ), anon_user );

	// Set up the next set of files.
	sspec_setuser(sspec_addxmemfile("rabbitF.gif", rabbit1_gif, SERVER_FTP), foo_user);
	sspec_setuser(sspec_addxmemfile("test3", rabbit1_gif, SERVER_FTP), foo_user);
	sspec_setuser(sspec_addxmemfile("test4", rabbit1_gif, SERVER_FTP), foo_user);

   // Having done all the sspec_setuser() calls (for fixed files), since the
   // user "foo" is able to do everything that "anonymous" can, we now make
   // foo a member of _both_ groups.
	sauth_setusermask(foo_user, FOO_GROUP | ANON_GROUP, NULL);

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	ftp_init(NULL); /* use default handlers */

	tcp_reserveport(FTP_CMDPORT);	// Port 21

   printf("Ready: FTP to %s\n\n", inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));

   printf("Press any key to quit and unmount partitions.\n");

	while(!kbhit()) {
		ftp_tick();
	}

   // Unmount all FAT devices before exiting
   printf("\nUnmounting FAT devices, please wait...\n");
  	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS;
         											 i += FAT_MAX_PARTITIONS) {
     	if (fat_part_mounted[i]) {
     		fat_UnmountDevice(fat_part_mounted[i]->dev);
     	}
   }
}
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
        ftp_server.c

        Demonstration of a simple FTP server, using the ftp library.
        The user "anonymous" may download the file "rabbitA.gif", but
        not "rabbitF.gif".  The user "foo" (with password "bar") may
        download "rabbitF.gif", and also "rabbitA.gif", since files owned
        by the anonymous user are world-readable.

        Each user may execute the "dir" or "ls" command to see a listing of
        the available files.
*******************************************************************************/
#class auto

// #define FTP_DEBUG

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
 * FTP configuration
 *
 * See the sspec_* and sauth_* functions below to see how to set up
 * files and users for the FTP server.
 */

/*
 * We are not defining any static resources.  Defining the following macro
 * tells ZSERVER.LIB (used by the FTP server) not to look for these static
 * resources.
 */
#define SSPEC_NO_STATIC

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
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
 * 			its NUl char termination).
 */
long
xmem_strdup( long xstr )
{
	auto long	xsrc, xnext;
	auto long 	siz;
	auto long 	xdest;

	xmem2root( & xsrc, xstr+0, 4 );
	xmem2root( & xnext, xstr+4, 4 );
	siz = xnext - xsrc - 1;   					// Don't store NUL char.

	xdest = xalloc( (int)siz + 4 );
	root2xmem( xdest, & siz, 4 );
	xmem2xmem( xdest+4, xsrc, (int)siz );

	return( xdest );
}   /* end xmem_strdup() */


/* -------------------------------------------------------------------- */

void main()
{
	int user;
	faraddr_t  xdest;

	// Set up anonymous user, and unowned files (i.e. files accessible by
	// anyone including the anonymous user).
	user = sauth_adduser("anonymous", "", SERVER_FTP);
	ftp_set_anonymous(user);

	sspec_addxmemfile("rabbitA.gif", rabbit1_gif, SERVER_FTP);
	sspec_addxmemfile("test1", rabbit1_gif, SERVER_FTP);
	sspec_addxmemfile("test2", rabbit1_gif, SERVER_FTP);

	/*
	 *  Copy the string from one place in XMEMORY to another.
	 */
	xdest = xmem_strdup( xtext );
	sspec_addxmemfile( "README", xdest, SERVER_FTP );

	// Set up the user "foo", and some files owned by foo.
	user = sauth_adduser("foo", "bar", SERVER_FTP);

	sspec_setuser(sspec_addxmemfile("rabbitF.gif", rabbit1_gif, SERVER_FTP), user);
	sspec_setuser(sspec_addxmemfile("test3", rabbit1_gif, SERVER_FTP), user);
	sspec_setuser(sspec_addxmemfile("test4", rabbit1_gif, SERVER_FTP), user);

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
	ftp_init(NULL); /* use default handlers */

	tcp_reserveport(FTP_CMDPORT);	// Port 21

	while(1) {
		ftp_tick();
	}
}
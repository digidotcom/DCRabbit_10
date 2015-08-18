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
        Samples\TcpIp\RabbitWeb\auth.c

        Demonstrates the use of authentication features for variables in the
        RabbitWeb HTTP enhancements.  See also the

        samples\tcpip\rabbitweb\pages\auth.zhtml

        page that demonstrates the corresponding ZHTML scripting features for
        authentication.

        This sample includes three web pages, which are really the same web
        page referenced from locations that have different authentication
        requirements.  They are "/index.zhtml", "/basic/index.zhtml", and
        "/digest/index.zhtml", which require no, basic, and digest
        authentication, respectively.

        There are also three users (in decreasing order of access):  "admin",
        "user", and "guest".  The passwords for each of these users are set to
        the same as the username.

        Experimenting with the web pages and matching up what happens with the
        #web statements in the program should give a good overview of how
        variable authorization works in the RabbitWeb HTTP enhancements.

*******************************************************************************/

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
 * HTTP_MAXBUFFER is used for each web server to hold received and
 * transferred information.  In particular, it holds each header line
 * received from the web client.  Digest authentication generally
 * results in a large WWW-Authenticate header that can be larger than
 * the default HTTP_MAXBUFFER value of 256.  512 bytes should be
 * sufficient.
 */
#define HTTP_MAXBUFFER				512

/********************************
 * End of configuration section *
 ********************************/

/*
 * This is needed to be able to use the RabbitWeb HTTP enhancements and the
 * ZHTML scripting language.
 */
#define USE_RABBITWEB 1

/*
 * Enable both basic and digest HTTP authentication.
 */
#define USE_HTTP_BASIC_AUTHENTICATION 1
#define USE_HTTP_DIGEST_AUTHENTICATION 1

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"

/*
 * This page contains the ZHTML portion of the authentication demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/auth.zhtml"	auth_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server.  Note that "/index.zhtml"
 * will be used only for no authentication, "/basic/index.zhtml" will force
 * the use of basic authentication, and "/digest/index.zhtml" will force the
 * use of digest authentication, even though they all refer to the same
 * #ximported file. */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", auth_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", auth_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/basic/index.zhtml", auth_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/digest/index.zhtml", auth_zhtml)
SSPEC_RESOURCETABLE_END

// The following variable will be read-only
int foo_ro;
// The following variable will be read-write
int foo_rw;
// Each of the following variables are specifically for basic and digest
// authentication.  The _ro variables are readonly for the "user" group and
// read-write for the "admin" groups, whereas the _rw variables are accessible
// to both groups.
int foo_basic_ro;
int foo_basic_rw;
int foo_digest_ro;
int foo_digest_rw;
// The following can be accessed from both basic and digest authentication
// methods.
int foo_basic_digest_ro;
int foo_basic_digest_rw;

// The following is a #web_groups statement, which defines a list of up to 16
// user groups.  These group symbols ("user", "admin", and "guest") will be used
// later to add specific users to these groups.
#web_groups user,admin,guest

// The following is a read-only variable, so no guard is needed.  All users
// (including *no* user, in the case of a page without authentication) are
// simply given read-only access to it via the "groups=all(ro)" part of the
// #web statements
#web foo_ro groups=all(ro)
// The following is read-write (which is the default).
#web foo_rw ($foo_rw > 0)

// The following makes this variable only accessible via basic authentication,
// and is writable only by the admin group.
#web foo_basic_ro ($foo_basic_ro > 0) auth=basic groups=admin,user(ro)
// The following makes this variable only accessible via basic authentication,
// and is writable by both the admin and user groups.
#web foo_basic_rw ($foo_basic_rw > 0) auth=basic groups=admin,user

// The following makes this variable only accessible via digest authentication,
// and is writable only by the admin group.
#web foo_digest_ro ($foo_digest_ro > 0) auth=digest groups=admin,user(ro)
// The following makes this variable only accessible via digest authentication,
// and is writable by both the admin and user groups.
#web foo_digest_rw ($foo_digest_rw > 0) auth=digest groups=admin,user

// The following makes this variable only accessible via both basic and digest
// authentication, and is writable only by the admin group.
#web foo_basic_digest_ro ($foo_basic_digest_ro > 0) \
   auth=basic,digest groups=admin,user(ro)
// The following makes this variable only accessible via both basic and digest
// authentication, and is writable by both the admin and user groups.
#web foo_basic_digest_rw ($foo_basic_digest_rw > 0) \
   auth=basic,digest groups=admin,user

void main(void)
{
	// This variable is used in setting up the web site users
	int userid;

	// Initialize the #web-registered variables
	foo_ro = 1;
	foo_rw = 2;
	foo_basic_ro = 3;
	foo_basic_rw = 4;
	foo_digest_ro = 5;
	foo_digest_rw = 6;
	foo_basic_digest_ro = 7;
	foo_basic_digest_rw = 8;

	// Create the rules that allow access to the different web server pages.
   // This rule is for the "/basic" web pages.  Note that it allows all user
   // groups, although basic authentication must be used.
   sspec_addrule("/basic", "Basic only", 0xffff, 0xffff, SERVER_ANY,
                 SERVER_AUTH_BASIC, NULL);
   // This rule overrides the above rules for the "/digest" web pages.  Note,
   // again, it allows all user groups, although digest authentication must be
   // used.
   sspec_addrule("/digest", "Digest only", 0xffff, 0xffff, SERVER_ANY,
                 SERVER_AUTH_DIGEST, NULL);

	// Set up the "admin" user.  The password is "admin".  The second line below
	// adds the user to the "admin" group.
	userid = sauth_adduser("admin", "admin", SERVER_ANY);
   sauth_setusermask(userid, admin, NULL);

	// The following lines set up the "user" and "guest" users.
	userid = sauth_adduser("user", "user", SERVER_ANY);
   sauth_setusermask(userid, user, NULL);
	userid = sauth_adduser("guest", "guest", SERVER_ANY);
   sauth_setusermask(userid, guest, NULL);

	// Initialize the TCP/IP stack and HTTP server
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();

	// This yields a performance improvement for an HTTP server
	tcp_reserveport(80);

   while (1) {
		// Drive the HTTP server
      http_handler();
   }
}


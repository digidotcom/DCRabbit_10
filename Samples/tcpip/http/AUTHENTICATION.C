/*******************************************************************************
        Samples\TcpIp\HTTP\authentication.c
        Rabbit Semiconductor, 2002

        This program provides three users, "foo", "foo2", and "foo3",
        for whom the passwords are "bar", "bar2", and "bar3",
        respectively.  Each of the three users can be toggled
        between being valid and invalid by pressing '1', '2', or '3'
        on the keyboard, respectively.

        Authentication modes can be set among basic, digest, and no
        authentication by pressing the 'b', 'd', or 'n' keys,
        respectively.

        This program is a modification of STATIC.C that demonstrates
        HTTP Digest Authentication and multiple user support.  Digest
        authentication achieves authentication without cleartext
        transmitting of passwords.  Instead, it uses a digest algorithm
        based on MD5.  Note, however, that this does NOT encrypt the
        web pages--it simply avoids sending the password itself in
        cleartext.  If possible, digest authentication should be used
        in place of basic authentication.

        Most modern browsers support digest authentication, including
        the most recent versions of Internet Explorer, Netscape/Mozilla,
        and Opera.  Netscape and Mozilla ignore the "stale" flag in
        digest authentication, and hence will sometimes ask for the
        username/password again when it is not strictly necessary.
        Also note that Netscape 4.x does NOT support digest
        authentication.

*******************************************************************************/

/***********************************
 * Configuration                   *
 * -------------                   *
 * The following fields may need   *
 * to be altered to match your     *
 * preferences.                    *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 					1

/*
 * HTTP_MAXBUFFER is used for each web server to hold received and
 * transferred information.  In particular, it holds each header line
 * received from the web client.  Digest authentication generally
 * results in a large WWW-Authenticate header that can be larger than
 * the default HTTP_MAXBUFFER value of 256.  512 bytes should be
 * sufficient.
 */
#define HTTP_MAXBUFFER				512

/*
 * By default, digest authentication is turned off.  Note that you
 * can set USE_HTTP_BASIC_AUTHENTICATION to 0 to remove the code for
 * basic authentication at compile time.
 */
#define USE_HTTP_DIGEST_AUTHENTICATION	1

/*
 * If you want to associate multiple users with a particular
 * resource (web page), then you need to set the following macro.
 * This macro defines how many users can be associated with a web
 * page, and defaults to 1.
 */
#define SSPEC_USERSPERRESOURCE 	3

/*
 * If you are using only the ZSERVER.LIB functions to manage the table
 * of HTTP pages, then you can disable code support for the
 * http_flashspec[] array.
 */
#define HTTP_NO_FLASHSPEC

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
#use "dcrtcp.lib"
#use "http.lib"

/*
 *  ximport is a Dynamic C language feature that takes the binary image
 *  of a file, places it in extended memory on the controller, and
 *  associates a symbol with the physical address on the controller of
 *  the image.
 */
#ximport "samples/tcpip/http/pages/static.html"		index_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"		rabbit1_gif

/*
 *  SSPEC_MIMETABLE_* gives the HTTP server hints about handling incoming
 *  requests.  The server compares the extension of the incoming
 *  request with this list and returns the second field
 *  as the Content-Type field.
 *  You can get a list of mime types by placing the cursor on
 *  the word MIME and pressing Ctrl-H.
 */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF)
SSPEC_MIMETABLE_END

void main(void)
{
	int user1;
	int user2;
	int user3;
	int user1_enabled;
	int user2_enabled;
	int user3_enabled;
	int page1;
	int page2;
	int ch;

	printf("Press '1', '2', or '3' to disable/enable the three users.\n");
	printf("Press 'b', 'd', or 'n' to set the authentication to basic, digest, or none.\n\n");

	/*
	 *  sock_init initializes the TCP/IP stack.
	 *  http_init initializes the web server.
	 */
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
	http_init();

	/*
	 * HTTP_DIGEST_AUTH is the default authentication type when
	 * digest authentication has been enabled, so this line is not
	 * strictly necessary.  The other possible values are
	 * HTTP_BASIC_AUTH and HTTP_NO_AUTH.
	 */
	http_setauthentication(HTTP_DIGEST_AUTH);
	printf("Using digest authentication\n");

	/*
	 * The following lines add three users, a web page, and an image, and
	 * associates the users with the web page.  The userx_enabled
	 * variables are used to keep track of which users are current
	 * enabled.
	 */
	user1_enabled = 1;
	user2_enabled = 1;
	user3_enabled = 1;
	user1 = sauth_adduser("foo", "bar", SERVER_HTTP);
	user2 = sauth_adduser("foo2", "bar2", SERVER_HTTP);
	user3 = sauth_adduser("foo3", "bar3", SERVER_HTTP);

	page1 = sspec_addxmemfile("/", index_html, SERVER_HTTP);
	sspec_adduser(page1, user1);
	sspec_adduser(page1, user2);
	sspec_adduser(page1, user3);
	sspec_setrealm(page1, "Admin");

	page2 = sspec_addxmemfile("index.html", index_html, SERVER_HTTP);
	sspec_adduser(page2, user1);
	sspec_adduser(page2, user2);
	sspec_adduser(page2, user3);
	sspec_setrealm(page2, "Admin");

	sspec_addxmemfile("rabbit1.gif", rabbit1_gif, SERVER_HTTP);

	/*
	 *  tcp_reserveport causes the web server to ignore requests when there
	 *  isn't an available socket (HTTP_MAXSERVERS are all serving index_html
	 *  or rabbit1.gif).  This saves some memory, but can cause the client
	 *  delays when retrieving pages (versus increasing HTTP_MAXSERVERS).
	 */
	tcp_reserveport(80);

	while (1) {
		/*
		 * Watch for user keypresses
		 */
		if (kbhit()) {
			ch = getchar();
			switch (ch) {
			case '1':
				/*
				 * Handle the keypress for User 1
				 */
				user1_enabled = !user1_enabled;
				if (user1_enabled) {
					/*
					 * sspec_adduser() adds a user to a resource
					 */
					sspec_adduser(page1, user1);
					sspec_adduser(page2, user1);
					printf("User 1 enabled\n");
				}
				else {
					/*
					 * sspec_removeuser() removes a user from a resource
					 */
					sspec_removeuser(page1, user1);
					sspec_removeuser(page2, user1);
					printf("User 1 disabled\n");
				}
				break;
			case '2':
				user2_enabled = !user2_enabled;
				if (user2_enabled) {
					sspec_adduser(page1, user2);
					sspec_adduser(page2, user2);
					printf("User 2 enabled\n");
				}
				else {
					sspec_removeuser(page1, user2);
					sspec_removeuser(page2, user2);
					printf("User 2 disabled\n");
				}
				break;
			case '3':
				user3_enabled = !user3_enabled;
				if (user3_enabled) {
					sspec_adduser(page1, user3);
					sspec_adduser(page2, user3);
					printf("User 3 enabled\n");
				}
				else {
					sspec_removeuser(page1, user3);
					sspec_removeuser(page2, user3);
					printf("User 3 disabled\n");
				}
				break;
			case 'b':
				http_setauthentication(HTTP_BASIC_AUTH);
				printf("Using basic authentication\n");
				break;
			case 'd':
				http_setauthentication(HTTP_DIGEST_AUTH);
				printf("Using digest authentication\n");
				break;
			case 'n':
				http_setauthentication(HTTP_NO_AUTH);
				printf("Using no authentication\n");
				break;
			}
		}

		/*
		 * http_handler needs to be called to handle the active http servers
		 */
		http_handler();
	}
}
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
        Samples\TcpIp\HTTP\static.c

        A very basic example using the HTTP library. This program
        completely intilizes the library, outputing a basic static
        web page.
*******************************************************************************/
/*
 * 	By default, have compiler make function variables storage class
 * 	"auto" (allocated on the stack).
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
#use "dcrtcp.lib"
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
 *  You can get a list of mime types by placing the cursor on
 *  the word MIME and pressing Ctrl-H.
 *
 */

/* the default mime type for '/' must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF)
SSPEC_MIMETABLE_END

/*
 *  The resource table assocates the file image we brought in with ximport
 *  and associates it with its name on the webserver.  In this example
 *  the file "samples/http/pages/static.html" will be sent to the
 *  client when they request either "http://yoururl.com/" or
 *  "http://yoururl.com/index.html"
 *
 */

SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/index.html", index_html),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif)
SSPEC_RESOURCETABLE_END

void main()
{

	/*
	 *  sock_init initializes the TCP/IP stack.
	 *  http_init initializes the web server.
	 */

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
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
      http_handler();
   }
}
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
/*****

static web server over PPPoE
Once connected, the server will print outs it's IP address on STDIO
You can then access the static page from that address: 'http://x.x.x.x/'

You will need to set the username and password for your account

********/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 0		// 0 means no predefined configuration
#define USE_PPPOE 0x01	// PPPoE over the first ethernet interface (ETH0).
#define USE_ETHERNET USE_PPPOE	// From DC9.0, you need to explicitly define USE_ETHERNET == USE_PPPOE

/*
#define DCRTCP_DEBUG
#define PPP_DEBUG
#define PPP_VERBOSE
#define PPPOE_DEBUG
#define PPPOE_VERBOSE
#define IP_VERBOSE
#define NET_VERBOSE
*/

#define PAPNAME "charlie"
#define PAPPASSWORD "pword"

#define HTTP_MAXSERVERS 2
#define MAX_TCP_SOCKET_BUFFERS 2

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"


#define TIMEZONE        -8

#define LCP_TIMEOUT 5000



//#define FRAGSUPPORT		//make sure FRAGSUPPORT is on

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
 *  SSPEC_MIMETABLE_* gives the HTTP server hints about handling incoming
 *  requests.  The server compares the extension of the incoming
 *  request with this list and returns the second field
 *  as the Content-Type field.
 *
 *  The default mime type for '/' must be first
 *
 */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
SSPEC_MIMETABLE_END

/*
 *  The static resource table assocates the file image we brought in with ximport
 *  and associates it with its name on the webserver.  In this example
 *  the file "samples/http/pages/static.html" will be sent to the
 *  client when they request either "http://yoururl.com/" or
 *  "http://yoururl.com/index.html"
 *
 */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/index.html", index_html),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
SSPEC_RESOURCETABLE_END


void main()
{
	auto unsigned long t;
	auto int if_status;
	auto char buffer[100];

	sock_init();

	ifconfig(IF_PPPOE0,
				IFS_PPP_ACCEPTIP, 1,
				IFS_PPP_ACCEPTDNS, 1,
				IFS_PPP_REMOTEAUTH, PAPNAME, PAPPASSWORD,
            IFS_DEBUG, 5,
				IFS_UP,
				IFS_END);

	// Wait for it to come up (or fail)
	while (IF_COMING_UP == (if_status = ifpending(IF_PPPOE0)) ||
	       IF_COMING_DOWN == if_status)
		tcp_tick(NULL);


	if(ifstatus(IF_PPPOE0))
		printf("PPPoE established\n");
	else
		printf("PPPoE failed\n");

   // Get the IP address assigned
   ifconfig(IF_PPPOE0, IFG_IPADDR, &t, IFS_END);
	printf("Point your browser to http://%s\n", inet_ntoa( buffer, t));

	http_init();

/*
 *  tcp_reserveport causes the web server to maintain pending requests
 * whenever there is not a listen socket available
 *
 */

   tcp_reserveport(80);

/*
 *  http_handler needs to be called to handle the active http servers.
 */

   while (1) {
      http_handler();
   }

}
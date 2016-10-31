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
ppp_answer.c

Static web server over PPP, handles peers that call in.
Once connected, the server will print outs it's IP address on STDIO.
You can then access the static page from that address: 'http://x.x.x.x/'.

You will need to choose a PPP serial port, change the default addresses and
also change the send/expect sequence at the beginning of the program.
********/

// select the PPP serial port, as appropriate for the target board
//  defining MY_PPP_SERIAL_PORT to 1 selects A, 2 selects B, ..., 6 selects F
#define MY_PPP_SERIAL_PORT 3	// our default is serial port C

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 0		// No predefined config for this sample

//#define PPP_DEBUG		// uncomment for PPP debugging
//#define PPP_VERBOSE	// uncomment for PPP detail

#define HTTP_MAXSERVERS 2
#define MAX_TCP_SOCKET_BUFFERS 2

// set up the selected PPP serial port
#if 1 == MY_PPP_SERIAL_PORT
	// for PPP on serial port A
	#warnt "Choosing serial port A disallows debugging via the programming port."
	#define USE_PPP_SERIAL 0x01
	#define MY_PPP_INTERFACE IF_PPP0
#elif 2 == MY_PPP_SERIAL_PORT
	// for PPP on serial port B
	#define USE_PPP_SERIAL 0x02
	#define MY_PPP_INTERFACE IF_PPP1
#elif 3 == MY_PPP_SERIAL_PORT
	// for PPP on serial port C
	#define USE_PPP_SERIAL 0x04
	#define MY_PPP_INTERFACE IF_PPP2
#elif 4 == MY_PPP_SERIAL_PORT
	// for PPP on serial port D
	#define USE_PPP_SERIAL 0x08
	#define MY_PPP_INTERFACE IF_PPP3
#elif 5 == MY_PPP_SERIAL_PORT && _CPU_ID_ >= R3000
	// for PPP on serial port E
	#define USE_PPP_SERIAL 0x10
	#define MY_PPP_INTERFACE IF_PPP4
#elif 6 == MY_PPP_SERIAL_PORT && _CPU_ID_ >= R3000
	// for PPP on serial port F
	#define USE_PPP_SERIAL 0x20
	#define MY_PPP_INTERFACE IF_PPP5
#else
	#error "Invalid PPP serial port selection!"
#endif

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
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif")
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
	auto int if_status;
	auto char buffer[100];

	sock_init();

	while(1)
	{
			//set up to act like an ISP,
			//local address is 10.1.10.1, peer is set to 10.1.10.2
			//IFS_PPP_LOCALAUTH sets up expected name and password
			//Note - SENDEXPECT sets up auto answer on first ring, and waits
			//indefinitely for "CONNECT"
			ifconfig(MY_PPP_INTERFACE,
						IFS_PPP_INIT,
						IFS_PPP_SPEED, 19200L,
						IFS_PPP_USEMODEM, 1,
						IFS_PPP_MODEMESCAPE, 1,
						IFS_PPP_SENDEXPECT, "ATS0=1 &CONNECT",
						IFS_PPP_HANGUP, "ATH",
						IFS_IPADDR, aton("10.1.10.1"),
						IFS_PPP_ACCEPTIP, 0,
						IFS_PPP_SETREMOTEIP, aton("10.1.10.2"),
						IFS_PPP_ACCEPTDNS, 0,
						IFS_PPP_LOCALAUTH, "rabbit", "carrots",
						IFS_UP,
						IFS_END);

			while (IF_COMING_UP == (if_status = ifpending(MY_PPP_INTERFACE)) ||
			       IF_COMING_DOWN == if_status)
			{
				tcp_tick(NULL); //wait for PPP to come up
			}

			if(ifstatus(MY_PPP_INTERFACE))
			{
				printf("PPP established\n");
				printf("IP address is %s\n", inet_ntoa( buffer, gethostid()));

				http_init();
			   tcp_reserveport(80);

  			 	while (ifstatus(MY_PPP_INTERFACE))
  			 	{
     	 			http_handler();
   			}
   		}
			else
			{
				printf("PPP failed\n");
			}

		ifconfig(MY_PPP_INTERFACE, IFS_DOWN, IFS_END);
		while(ifstatus(MY_PPP_INTERFACE))
		{
			tcp_tick(NULL); //wait for PPP to terminate
		}
	}
}
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

/* the default mime type for '/' must be first */
const HttpType http_types[] =
{
   { ".html", "text/html", NULL},
   { ".gif", "image/gif", NULL}
};

/*
 *  http_flashspec assocates the file image we brought in with ximport
 *  and associates it with its name on the webserver.  In this example
 *  the file "samples/http/pages/static.html" will be sent to the
 *  client when they request either "http://yoururl.com/" or
 *  "http://yoururl.com/index.html"
 *
 */

const HttpSpec http_flashspec[] =
{
   { HTTPSPEC_FILE,  "/",              index_html,    NULL, 0, NULL, NULL},
   { HTTPSPEC_FILE,  "/index.html",    index_html,    NULL, 0, NULL, NULL},
   { HTTPSPEC_FILE,  "/rabbit1.gif",   rabbit1_gif,   NULL, 0, NULL, NULL},
};

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
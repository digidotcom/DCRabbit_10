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

   multiweb.c

   Static web server over PPP and/or ethernet.
   This demonstrates the new multiple-interface functionality.

   When connected via PPP (serial port C by default), the server will print out
   its IP address on STDIO.  The static ethernet address is always printed out.

   You can then access the static page from the IP address(es) printed on STDIO
   at the start of execution.

   If ADD_PPP_SERVER is defined then you should read through the remainder of
   the information in this comment block to determine what other macros need be
   defined or edited.  Otherwise, you may skip past anything that is conditional
   on the ADD_PPP_SERVER macro.

   Once connected, both HTTP servers will print out their IP addresses on STDIO.
   You can then access the static page from either address: <http://x.x.x.x/>.

   You will need to change the default macro definitions for DIALUP_NAME,
   DIALUP_NUMBER and DIALUP_PASSWORD.

   You also may need to alter the choice of serial port, whether or not CTS/RTS
   flow control is enabled via the DIALUP_FLOWCONTROL macro and the choice of
   flow control (CTS, RTS) I/O bits and ports, and the send/expect sequence
   defined in the DIALUP_SENDEXPECT macro.

   This sample assumes you are using hardware modem flow control i.e. CTS and
   RTS.  The CTS port and bit number are defined in the IFS_PPP_CTSPIN
   parameter, defaulting to parallel port C, bit 1.  Likewise, RTS is set via
   IFS_PPP_RTSPIN and defaults to parallel port C, bit 0.

   Changing either or both I/O bits and ports used for CTS / RTS is simply a
   matter of updating the MY_xTS_BIT and MY_xTS_PORT macro definitions
   appropriately.

   Note that no real checking of these CTS / RTS macro definitions is performed,
   so it is entirely possible to make a non-working choice.  In fact, the
   default selections are not appropriate for some Rabbit boards.  Please check
   your hardware manual in order to make choices that are appropriate for your
   board.

   When using a Rabbit SBC or an RCM on a protyping board, it can be convenient
   to choose the CTS / RTS flow control lines based on a second RS-232 port, if
   available.  In many cases, the second RS-232 serial port will already have an
   RS-223 tranceiver chip installed, saving some time and effort.  The second
   serial port's RX line is repurposed as the CTS, and the TX line as the RTS.
   The standard I/O pins used for serial ports are:
      RXA (CTS) on PC7
      TXA (RTS) on PC6
      RXB (CTS) on PC5
      TXB (RTS) on PC4
      RXC (CTS) on PC3
      TXC (RTS) on PC2
      RXD (CTS) on PC1
      TXD (RTS) on PC0
      RXE (CTS) on PG7 (Rabbit 3000) or PC7 (Rabbit 4000)
      TXE (RTS) on PG6 (Rabbit 3000) or PC6 (Rabbit 4000)
      RXF (CTS) on PG3 (Rabbit 3000) or PC3 (Rabbit 4000)
      TXF (RTS) on PG2 (Rabbit 3000) or PC2 (Rabbit 4000)

   It is important to ensure that your modem is set up correctly for your choice
   of CTS/RTS hardware flow control enabled vs. disabled on the Rabbit board.
   Please consult your modem's manual to determine what its setup requirements
   are for your choice of flow control enabled vs. disabled.

********/

// Define the following to include a PPP interface if e.g. you are dialing
// an ISP.  Otherwise, if you have a dedicated serial link to a PC, and no
// modem, you should not define this.
//#define ADD_PPP_SERVER

#ifdef ADD_PPP_SERVER
	#define DIALUP_FLOWCONTROL 1	// 0 to disable, 1 to enable HW flow control
	#define DIALUP_NAME "username"
	#define DIALUP_NUMBER "5551212"
	#define DIALUP_PASSWORD "password"

	// select the PPP serial port, as appropriate for the target board
	//  defining MY_PPP_SERIAL_PORT to 1 selects A, 2 selects B, ..., 6 selects F
	#define MY_PPP_SERIAL_PORT 3  // our default is serial port C

	// select the CTS and RTS bits and ports
	// our defaults are PC1 (standard RXD) and PC0 (standard TXD), respectively
	#define MY_CTS_BIT 1
	#define MY_CTS_PORT PCDR
	#define MY_RTS_BIT 0
	#define MY_RTS_PORT PCDR

	//Uncomment to get PPP detail
	//#define PPP_VERBOSE
	//#define PPP_DEBUG
	//#define PPPLINK_VERBOSE
	//#define PPPLINK_DEBUG
	//#define CHAT_VERBOSE
	//#define CHAT_DEBUG

	// The following macro sets up the initial login screen navigation.  This is
	// necessary for access to many ISPs, since when you dial in with the modem,
	// the ISP does user authentication via an old-fashioned login screen.
	//
	// %0 gets userid substituted, %1 is password -- obtained from values passed
	// for IFS_PPP_REMOTEAUTH.  The string below gets compiled to
	// "ATZ #ok @ATDT5551212 #CONNECT '' #ogin: @%0 #word: @%1 ~"
	//
	// This is a send/expect sequence used to establish a modem connection to the
	// ISP (ready for PPP to take over).  Expanded out:
	//  ATZ         - send the modem reset command (usually default config 0).
	//                This helps get the modem into a known initial state.
	//  #ok         - wait for 'ok' string (the leading '#' means case-
	//                insensitive i.e. will accept 'OK', 'Ok' etc.
	//  @ATDT5551212 - After 1.5 seconds pause, send the dialout command. The
	//                pause allows time for the modem's reset to complete for
	//                those modems which respond with an early 'ok' to the prior
	//                reset command.
	//  #CONNECT    - wait for 'connect' message
	//  ''          - Send nothing (quotes are a place-holder).  Note that the
	//                CRLF is appended to any send-string _except_ an empty
	//                string like this (but you can suppress the CRLF by
	//                prefixing the send-string with '#').
	//  #ogin:      - Wait for a 'login:' message.  We don't actually look for
	//                the initial 'L' since the first character can sometimes be
	//                lost.
	//  @%0         - send the userid (i.e. the value supplied by the
	//                IFS_PPP_REMOTEAUTH parameter.  If the character-based login
	//                prompt expects a different userid than the one passed to
	//                the following PPP authentication phase, then you can't use
	//                %0.  In this case, put the character-based login ID
	//                directly in the send/expect string.  The initial '@'
	//                character causes us to pause for 1.5 seconds before sending
	//                the string.  This is not theoretically required, however
	//                some ISPs have software which tries to discourage machine-
	//                based hacking attempts.  If we respond too quickly, the ISP
	//                thinks we are typing at inhumanly fast rates, and deems us
	//                to be a hacker.
	//  #word:      - Wait for the 'password:' prompt.
	//  @%1         - Send the password string.  See considerations above for the
	//                logon ID string.
	//  ~           - Wait for ASCII tilde char (0x7E).  This is handy, since
	//                this is the first character sent by PPP when it starts up
	//                on the peer.  Some peers send an ascii 'PPP' string, but
	//                this is not usually so reliable as a means of detecting PPP
	//                startup.  Both methods are provided in the choice of send /
	//                expect macros, below.  If one doesn't work (i.e. CHAT:
	//                times out waiting for the '~' or the 'PPP') try the other
	//                macro definition.
	// Comment out exactly one of the following two send / expect macro
	// definitions.  It may be necessary to edit these send / expect macro
	// definitions to suit.
	#define DIALUP_SENDEXPECT "ATZ #ok @ATDT" DIALUP_NUMBER " #CONNECT '' #ogin: @%0 #word: @%1 ~"
	//#define DIALUP_SENDEXPECT "ATZ #ok @ATDT" DIALUP_NUMBER " #CONNECT '' #ogin: @%0 #word: @%1 PPP"

	#if DIALUP_FLOWCONTROL
		#define DIALUP_SPEED 57600L	// higher serial rate, flow control enabled
	#else
		#define DIALUP_SPEED 19200L	// lower serial rate, flow control disabled
	#endif

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

	#ifndef PFDR
	   #define PFDR 0
	#endif
	#ifndef PGDR
	   #define PGDR 0
	#endif

	#if PADR == MY_CTS_PORT
	   // parallel port A
	 #if MY_CTS_PORT == MY_RTS_PORT
	   #error "Parallel port A can't be shared between inputs and outputs."
	 #endif
	   #define MY_CTS_BIT_SETUP WrPortI(SPCR, &SPCRShadow, 0x80);
	#elif PBDR == MY_CTS_PORT
	   // parallel port B
	   #define MY_CTS_BIT_SETUP BitWrPortI(PBDDR, &PBDDRShadow, 0, MY_CTS_BIT);
	#elif PCDR == MY_CTS_PORT
	   // parallel port C
	   #define MY_CTS_BIT_SETUP BitWrPortI(PCFR, &PCFRShadow, 0, MY_CTS_BIT);
	                            // no DDR setup is possible
	#elif PDDR == MY_CTS_PORT
	   // parallel port D
	   #define MY_CTS_BIT_SETUP BitWrPortI(PDFR, &PDFRShadow, 0, MY_CTS_BIT); \
	                            BitWrPortI(PDDDR, &PDDDRShadow, 0, MY_CTS_BIT);
	#elif PEDR == MY_CTS_PORT
	   // parallel port E
	   #define MY_CTS_BIT_SETUP BitWrPortI(PEFR, &PEFRShadow, 0, MY_CTS_BIT); \
	                            BitWrPortI(PEDDR, &PEDDRShadow, 0, MY_CTS_BIT);
	#elif PFDR && PFDR == MY_CTS_PORT
	   // parallel port F
	   #define MY_CTS_BIT_SETUP BitWrPortI(PFFR, &PFFRShadow, 0, MY_CTS_BIT); \
	                            BitWrPortI(PFDDR, &PFDDRShadow, 0, MY_CTS_BIT);
	#elif PGDR && PGDR == MY_CTS_PORT
	   // parallel port G
	   #define MY_CTS_BIT_SETUP BitWrPortI(PGFR, &PGFRShadow, 0, MY_CTS_BIT); \
	                            BitWrPortI(PGDDR, &PGDDRShadow, 0, MY_CTS_BIT);
	#else
	   #error "Invalid MY_CTS_PORT selection!"
	#endif

	#if PADR == MY_RTS_PORT
	   // parallel port A
	   #define MY_RTS_BIT_SETUP BitWrPortI(PADR, &PADRShadow, 0, MY_RTS_BIT); \
	                            WrPortI(SPCR, &SPCRShadow, 0x84);
	   #define MY_RTS_PORT_SHADOW PADRShadow
	#elif PBDR == MY_RTS_PORT
	   // parallel port B
	   #define MY_RTS_BIT_SETUP BitWrPortI(PBDR, &PBDRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PBDDR, &PBDDRShadow, 1, MY_RTS_BIT);
	   #define MY_RTS_PORT_SHADOW PBDRShadow
	#elif PCDR == MY_RTS_PORT
	   // parallel port C
	   #define MY_RTS_BIT_SETUP BitWrPortI(PCFR, &PCFRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PCDR, &PCDRShadow, 0, MY_RTS_BIT);
	                            // no DDR setup is possible
	   #define MY_RTS_PORT_SHADOW PCDRShadow
	#elif PDDR == MY_RTS_PORT
	   // parallel port D
	   #define MY_RTS_BIT_SETUP BitWrPortI(PDFR, &PDFRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PDDR, &PDDRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PDDDR, &PDDDRShadow, 1, MY_RTS_BIT);
	   #define MY_RTS_PORT_SHADOW PDDRShadow
	#elif PEDR == MY_RTS_PORT
	   // parallel port E
	   #define MY_RTS_BIT_SETUP BitWrPortI(PEFR, &PEFRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PEDR, &PEDRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PEDDR, &PEDDRShadow, 1, MY_RTS_BIT);
	   #define MY_RTS_PORT_SHADOW PEDRShadow
	#elif PFDR && PFDR == MY_RTS_PORT
	   // parallel port F
	   #define MY_RTS_BIT_SETUP BitWrPortI(PFFR, &PFFRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PFDR, &PFDRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PFDDR, &PFDDRShadow, 1, MY_RTS_BIT);
	   #define MY_RTS_PORT_SHADOW PFDRShadow
	#elif PGDR && PGDR == MY_RTS_PORT
	   // parallel port G
	   #define MY_RTS_BIT_SETUP BitWrPortI(PGFR, &PGFRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PGDR, &PGDRShadow, 0, MY_RTS_BIT); \
	                            BitWrPortI(PGDDR, &PGDDRShadow, 1, MY_RTS_BIT);
	   #define MY_RTS_PORT_SHADOW PGDRShadow
	#else
	   #error "Invalid MY_RTS_PORT selection!"
	#endif
#endif


/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG		1		// use the predefined static configuration for eth0

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
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF)
SSPEC_MIMETABLE_END


// The static resource table is initialized with these macros...
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/index.html", index_html),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif)
SSPEC_RESOURCETABLE_END


void main()
{
	auto unsigned long t;
	auto char buffer[100];
	auto word i;
	auto int if_status;

	debug_on = 1;

#ifdef ADD_PPP_SERVER
	MY_CTS_BIT_SETUP	// set up the CTS handshake input
	MY_RTS_BIT_SETUP	// set up the RTS handshake output
#endif

	sock_init();

#ifdef ADD_PPP_SERVER
	// now configure PPP for dialing in to ISP and bring it up
	ifconfig(MY_PPP_INTERFACE,
				IFS_PPP_INIT,
				IFS_PPP_SPEED, DIALUP_SPEED,
				IFS_PPP_RTSPIN, MY_RTS_PORT, &MY_RTS_PORT_SHADOW, MY_RTS_BIT,
				IFS_PPP_CTSPIN, MY_CTS_PORT, MY_CTS_BIT,
				IFS_PPP_FLOWCONTROL, DIALUP_FLOWCONTROL,
				IFS_PPP_SENDEXPECT, DIALUP_SENDEXPECT,
				IFS_PPP_HANGUP, "ATH #ok",
				IFS_PPP_MODEMESCAPE, 1,
				IFS_PPP_ACCEPTIP, 1,
				IFS_PPP_ACCEPTDNS, 1,
				IFS_PPP_REMOTEAUTH, DIALUP_NAME, DIALUP_PASSWORD,
				IFS_UP,
				IFS_END);

	while (IF_COMING_UP == (if_status = ifpending(MY_PPP_INTERFACE)) ||
	       IF_COMING_DOWN == if_status)
	{
		tcp_tick(NULL);
	}
	if(ifstatus(MY_PPP_INTERFACE))
	{
		printf("PPP established\n");
	}
	else
	{
		printf("PPP failed\n");
	}
#endif

	ip_print_ifs();
	router_printall();

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


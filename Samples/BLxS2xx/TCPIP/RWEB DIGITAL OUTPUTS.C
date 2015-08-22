/***************************************************************************

   RabbitWeb Digital Output Sample

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
	This program demonstrates using the digOut API function to
   control digital sinking type outputs to toggle LEDs ON and
   OFF on the demo board provided with your kit.

	Connections:
	============

	****WARNING****: When using the J7 connector, be sure to insulate or cut
      the exposed wire from the wire leads you are not using.  The only
      connection required from J7 for any of the sample programs is +5v.

	1. DEMO board jumper settings:
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller J10 pin 5 GND to the DEMO board GND.

	3. Connect a wire from the controller J7 pin 6 (+5V) to the DEMO board +V.

   4. Connect the following wires from the controller J10 to the DEMO
      board screw terminal:

      From J10 pin 9 DIO0 to LED1
      From J10 pin 4 DIO1 to LED2
      From J10 pin 8 DIO2 to LED3
      From J10 pin 3 DIO3 to LED4

	Instructions:
	=============
   1. Get help (ctrl-H) on TCPCONFIG for instructions on setting up
      networking for this server sample.
   2. Compile and run this program.
   3. With your WEB browser access the WEB page, using the URL shown.
   4. View LEDS on Web page and the controller to see that they match
      when changing them via the WEB page control button.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto
#memmap xmem

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

#define USE_RABBITWEB 1

#use "BLxS2xx.lib"
#use "dcrtcp.lib"
#use "http.lib"

// Import files to xmem to make them available through server
#ximport "samples/BLxS2xx/tcpip/pages/leddemo.zhtml"		leddemo_zhtml
#ximport "samples/BLxS2xx/tcpip/pages/rabbit1.gif"			rabbit1_gif
#ximport "samples/BLxS2xx/tcpip/pages/ledon.gif"			ledon_gif
#ximport "samples/BLxS2xx/tcpip/pages/ledoff.gif"			ledoff_gif
#ximport "samples/BLxS2xx/tcpip/pages/button.gif"			button_gif

// first entry in mimetable is used for index.html file
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html"),
   SSPEC_MIME(".gif", "image/gif"),
   SSPEC_MIME(".css", "text/css")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", leddemo_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/leddemo.zhtml", leddemo_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledon.gif", ledon_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledoff.gif", ledoff_gif),
	SSPEC_RESOURCE_XMEMFILE("/button.gif", button_gif)
SSPEC_RESOURCETABLE_END

#define IOCOUNT 4
int output[IOCOUNT];

// create a zweb variable for IOCOUNT
const int iocount = IOCOUNT;
#web iocount

void update_outputs()
{
	int i;

   for (i = 0; i < IOCOUNT; ++i) {
		digOut (i, output[i]);
   }
}

#web output[@]
#web_update output[@] update_outputs

void main()
{
	unsigned long ip;
	char ipbuf[16];

	int i;

   // Initialize the controller
	brdInit();

	for (i = 0; i < IOCOUNT; i++) {
      // Set output to be a general digital output
      setDigOut (i, 1);

   	// set outputs alternating between 0 and 1
		output[i] = (i % 2);
   }

   // update state of I/O pins to match settings in program
	update_outputs();

   printf ("Starting TCP/IP stack...\n");
	sock_init_or_exit(1);

	// start web server and queue requests on port 80 when busy
	http_init();
	tcp_reserveport(80);

   // show that the server is up and the IP address to connect to it
   ifconfig (IF_DEFAULT, IFG_IPADDR, &ip, IFS_END);
   printf ("Connect your browser to http://%s/\n", inet_ntoa (ipbuf, ip));

   // process WEB page requests and update the LED's
   while (1)
   {
      http_handler();
   }
}
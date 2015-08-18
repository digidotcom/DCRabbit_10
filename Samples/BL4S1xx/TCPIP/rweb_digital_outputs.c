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
   rweb_digital_outputs.c

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
	This program demonstrates using the digOut API function to
   control digital sinking type outputs to toggle LEDs ON and
   OFF on the demo board provided with your kit.

	Connections:
	============
	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller GND, to the DEMO board GND.

	3. Connect a wire from the controller +5V to the DEMO board +5V.

   4. Connect the following wires from the controller to the DEMO board:
   		From OUT0 to LED1
   		From OUT1 to LED2
   		From OUT2 to LED3
   		From OUT3 to LED4

	Instructions
	============
	1. Compile and run this program.
	2. Connect to the Rabbit with a web browser, using the URL shown.
	3. To check other outputs that are not connected to the demo board,
      you can use a voltmeter to see the output change states.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// default functions to xmem
#memmap xmem

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// Make use of RabbitWeb's scripting commands in HTML files
#define USE_RABBITWEB 1

#use "dcrtcp.lib"
#use "http.lib"

#ximport "Samples/BL4S1xx/tcpip/pages/leddemo.zhtml"		leddemo_zhtml
#ximport "Samples/BL4S1xx/tcpip/pages/ledon.gif"			ledon_gif
#ximport "Samples/BL4S1xx/tcpip/pages/ledoff.gif"			ledoff_gif
#ximport "Samples/BL4S1xx/tcpip/pages/button.gif"			button_gif

// first entry in mimetable is used for URLs without extensions (including "/")
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

   for (i = 0; i < IOCOUNT; i++) {
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
		output[i] = (i & 1);
   }

   // update state of I/O pins to match settings in program
	update_outputs();

   printf ("Starting TCP/IP stack...\n");
	sock_init_or_exit(1);

	// start web server and queue requests on port 80 when busy
	http_init();
	tcp_reserveport(80);

   ifconfig (IF_ETH0, IFG_IPADDR, &ip, IFS_END);
   printf ("Connect your browser to http://%s/\n", inet_ntoa (ipbuf, ip));

   while (1) {
   	http_handler();
   }
}
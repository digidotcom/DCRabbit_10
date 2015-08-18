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
/**********************************************************

   rweb_io_sample.c

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
	This program demonstrates using RabbitWeb to display the status of multiple
	I/O lines (analog and digital) in a web browser, and allow the user to
	change the digital outputs by clicking on buttons on the page.

	It uses an IFRAME (invisible frame) to refresh the I/O readings every two
	seconds.  Since the web browser doesn't have to re-render the entire page,
	updates are quick and flicker free.

	Connections for unipolar mode of operation, 0 - 20V
   ----------------------------------------------------
   1. Connect the positive power supply lead to an input channel.
	2.	Connect the negative power supply lead to AGND on the controller.

	Note: If you always see "not calibrated" on your analog inputs, you need to
	run the adc_cal_se_unipolar.c sample to calibrate the inputs.

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
   		From DOUT0 to LED1
   		From DOUT1 to LED2
   		From DOUT2 to LED3
   		From DOUT3 to LED4
			From IN0 to SW1
			From IN1 to SW2
			From IN2 to SW3
			From IN3 to SW4

	Instructions:
	=============
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

// Configuration options:

// Set DEMO_GAIN appropriately for the voltage range on the analog inputs.
// See the function help (Ctrl-H) on ADC_GAINS for valid gain settings.
#define DEMO_GAIN GAIN_X2

// End of Configuration Options

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// Make use of RabbitWeb's scripting commands in HTML files
#define USE_RABBITWEB 1

// Allow for multiple connections, to better support web browsers that
// download multiple files at the same time.
#define HTTP_MAXSERVERS 4

#use "dcrtcp.lib"
#use "http.lib"

#ximport "samples/BL4S1xx/tcpip/pages/iosample.html"		iosample_html
#ximport "samples/BL4S1xx/tcpip/pages/iovalues.html"		iovalues_html
#ximport "samples/BL4S1xx/tcpip/pages/iosample.css"		iosample_css
#ximport "samples/BL4S1xx/tcpip/pages/rabbitlogo.png"		rabbitlogo_png

// first entry in mimetable is used for URLs without extensions (including "/")
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser for *.html files
	SSPEC_MIME_FUNC(".html", "text/html", zhtml_handler),
	SSPEC_MIME(".png", "image/png"),
	SSPEC_MIME(".css", "text/css")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", iosample_html),
	SSPEC_RESOURCE_XMEMFILE("/iovalues.html", iovalues_html),
	SSPEC_RESOURCE_XMEMFILE("/iosample.css", iosample_css),
	SSPEC_RESOURCE_XMEMFILE("/rabbitlogo.png", rabbitlogo_png)
SSPEC_RESOURCETABLE_END

// RabbitWeb doesn't support arrays of strings, but it can handle an array
// of structures with a single string as it's member.
typedef struct {
	char s[20];
} string20_t;

// info on each I/O position around the board
typedef struct {
	char	label[6];
	int	type;
	int	channel;
} ioinfo_t;

// io reading for each position around the board, as a formatted string.  The
// string is large enough to handle the "high" and "low" text that includes
// html entities for up and down arrows.
typedef struct {
	char	display[20];
} io_t;

enum {
	IOTYPE_OUT = 0,		// iovalues.html depends on IOTYPE_OUT being 0
	IOTYPE_IN,
	IOTYPE_AIN,
	IOTYPE_BLANK
};

// This table defines the I/O lines on the mocked-up board used on the
// HTML page.  Order is left-to-right, top-to-bottom.
const ioinfo_t ioinfo[] = {
	{ "AIN0", IOTYPE_AIN, 0 },		{ "GND", IOTYPE_BLANK, 0 },		// 0, 1
	{ "AIN1", IOTYPE_AIN, 1 },		{ "+K", IOTYPE_BLANK, 0 },			// 2, 3
	{ "AIN2", IOTYPE_AIN, 2 },		{ "IN0", IOTYPE_IN, 0 },			// 4, 5
	{ "AIN3", IOTYPE_AIN, 3 },		{ "IN1", IOTYPE_IN, 1 },			// 6, 7
	{ "AIN4", IOTYPE_AIN, 4 },		{ "IN2", IOTYPE_IN, 2 },			// 8, 9
	{ "AIN5", IOTYPE_AIN, 5 },		{ "IN3", IOTYPE_IN, 3 },			// 10, 11
	{ "AIN6", IOTYPE_AIN, 6 },		{ "OUT0", IOTYPE_OUT, 0 },			// 12, 13
	{ "AIN7", IOTYPE_AIN, 7 },		{ "OUT1", IOTYPE_OUT, 1 },			// 14, 15
	{ "GND", IOTYPE_BLANK, 0 },	{ "OUT2", IOTYPE_OUT, 2 },			// 16, 17
	{ " ", IOTYPE_BLANK, 0 },		{ " ", IOTYPE_BLANK, 0 },			// 18, 19
	{ "IN11", IOTYPE_IN, 11 },		{ "OUT3", IOTYPE_OUT, 3 },			// 20, 21
	{ "IN10", IOTYPE_IN, 10 },		{ "OUT4", IOTYPE_OUT, 4 },			// 22, 23
	{ "IN9", IOTYPE_IN, 9 },		{ "OUT5", IOTYPE_OUT, 5 },			// 24, 25
	{ "IN8", IOTYPE_IN, 8 },		{ "OUT6", IOTYPE_OUT, 6 },			// 26, 27
	{ "IN7", IOTYPE_IN, 7 },		{ "OUT7", IOTYPE_OUT, 7 },			// 28, 29
	{ "IN6", IOTYPE_IN, 6 },		{ "GND",	IOTYPE_BLANK, 0 },		// 30, 31
	{ "IN5", IOTYPE_IN, 5 },		{ "+K1", IOTYPE_BLANK, 0 },		// 32, 33
	{ "IN4", IOTYPE_IN, 4 },		{ "+K2", IOTYPE_BLANK, 0 },		// 34, 35
	{ "GND", IOTYPE_BLANK, 0 },	{ "+5V", IOTYPE_BLANK, 0 }			// 36, 37
};
#define IOCOUNT (sizeof(ioinfo)/sizeof(ioinfo[0]))
#web ioinfo[@]
// create a zweb variable for IOCOUNT
const int iocount = IOCOUNT;
#web iocount

io_t io[IOCOUNT];
#web io[@]

// These strings include the HTML entities for Up Arrow (&uarr;) and
// Down Arrow (&darr;).  Left (_L) and Right (_R) versions are used for
// each side of the on-page board.
#define STR_HIGH_R	"&uarr;high"
#define STR_LOW_R		"&darr;low"
#define STR_HIGH_L	"high&uarr;"
#define STR_LOW_L		"low&darr;"

// Array to hold the eight HTML buttons used for changing the digital outputs.
string20_t outbtn[BL_DIGITAL_OUT];
#web outbtn[@].s

// Array used to hold the button values when a user submits the page.
string20_t btn[BL_DIGITAL_OUT];
#web btn[@].s

// Global output stores the state of each digital output.
int output[BL_DIGITAL_OUT];

// update_outputs() updates the digital outputs to match the settings in
// the global variable <output>.
void update_outputs()
{
	int i;

   for (i = 0; i < BL_DIGITAL_OUT; i++) {
		digOut (i, output[i]);
   }
}

// io_read() reads the analog and digital I/O.  Each call to io_read
// updates all of the digital inputs and outputs, and reads a single
// analog input.
// After a read has completed, it increments a static variable to point
// to the next channel for the next pass.
void io_read()
{
	int i;
	float voltage;
	const ioinfo_t *info;
	io_t *iop;

	for (i = 0; i < IOCOUNT; i++) {
		info = &ioinfo[i];		// pointer to configuration structure
		iop = &io[i];				// pointer to formatted string on HTML page

		switch (info->type) {
			case IOTYPE_OUT:		// digital output
	         strcpy (iop->display,
	         	output[info->channel] ? STR_HIGH_R : STR_LOW_R);
	         strcpy (outbtn[info->channel].s,
	         	output[info->channel] ? STR_LOW_R : STR_HIGH_R);
				break;

			case IOTYPE_IN:		// digital input
	         strcpy (iop->display,
	         	digIn(info->channel) ? STR_HIGH_R : STR_LOW_R);
				break;

			case IOTYPE_AIN:		// analog input
            voltage = anaInVolts (info->channel, DEMO_GAIN);
            if (voltage <= BL_ERRCODESTART) {
               if (voltage == BL_TIMEOUT) {
                  strcpy (iop->display, "timeout");
               } else if (voltage == BL_OVERFLOW) {
                  strcpy (iop->display, "overflow");
               } else if (voltage == BL_NOT_CAL) {
                  strcpy (iop->display, "not calibrated");
               } else if (voltage == BL_WRONG_MODE) {
                  strcpy (iop->display, "wrong mode");
               } else {
                  strcpy (iop->display, "error");
               }
            } else {
               // we got a valid result back
               sprintf (iop->display, "%.02fV", voltage);
            }
				break;
		}
	}
}

// When a user clicks on one of the digital output buttons, RabbitWeb calls
// handle_buttons().  handle_buttons() scans the <btn> array to find the
// button that the user clicked on.
void handle_button()
{
	int i;

	for (i = 0; i < BL_DIGITAL_OUT; i++) {
		if (*(btn[i].s)) {
			output[i] = strstr (btn[i].s, "high") ? 1 : 0;
		}
	}
	update_outputs();
	memset (btn, 0, sizeof(btn));
	io_read();
}
#web_update btn[@].s handle_button

void main()
{
	unsigned long ip;
	char ipbuf[16];
	int channel;

   // Initialize the controller
	brdInit();

	memset (io, 0, sizeof(io));
	memset (btn, 0, sizeof(btn));

	// configure the outputs
	for (channel = 0; channel < BL_DIGITAL_OUT; channel++) {
      // Set output to be a general digital output
      setDigOut (channel, 1);

   	// set outputs alternating between 0 and 1
		output[channel] = (channel & 1);
   }

   // update state of I/O pins to match settings in program
	update_outputs();

	// configure the digital inputs
   for (channel = 0; channel < BL_DIGITAL_IN; channel++) {
		setDigIn (channel);
   }

   // Configure analog inputs for Single-Ended unipolar mode of operation.
   // (Max voltage range is 0 - 20v)
	for (channel = 0; channel < BL_ANALOG_IN; ++channel)
   {
   	anaInConfig (channel, SE0_MODE);
   }

   printf ("Starting TCP/IP stack...\n");
	sock_init_or_exit(1);

	// start web server and queue requests on port 80 when busy
	http_init();
	tcp_reserveport(80);

   ifconfig (IF_ETH0, IFG_IPADDR, &ip, IFS_END);
   printf ("Connect your browser to http://%s/\n", inet_ntoa (ipbuf, ip));

	// main loop -- update inputs and handle web requests
   while (1) {
	   io_read();
   	http_handler();
   }
}
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
/**************************************************************************

	digout_bank.c

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
   This program demonstrates using the digOutBank API function to
   control digital sinking type outputs to toggle LED's ON and OFF
   on the demo board provided with your kit.  The banking functions
   allow banks of IO to be input or output more efficiently.

	Connections:
	============
	1. DEMO board jumper settings:
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller GND, to the DEMO board GND.

   3. Connect a wire from the controller +5V to the DEMO board +V.

   4. Connect the following wires from the controller to the DEMO board:
	   	From OUT0 to LED1
	   	From OUT1 to LED2
	   	From OUT2 to LED3
	   	From OUT3 to LED4

	Instructions:
	=============
	1. Compile and run this program.
	2. Select a output channel via the STDIO window (per the channels you
      connected to the LEDs) to toggle a LED on the demo board.
      NOTE: Since the digital outputs use sinking drivers, they are
            enabled (and the LED will light) when the port is at logic 0.
	3. To check other outputs that are not connected to the demo board,
      you can use a voltmeter to see the output change states.
      Note: a pull-up must be connected to outputs to see I/O changes.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include series BL4S1xx library
#use "BL4S1xx.LIB"

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

///////////////////////////////////////////////////////////////////////////

void main()
{
	char *ptr;
	char display[128];
	char channel_bank;
	int channel;
   char mask;

   // Initialize the controller
	brdInit();

	//Display user instructions and channel headings
	DispStr(8, 1, " <<< Digital Outputs 0-7 >>>");
	DispStr(8, 3, " OUT0\t OUT1\t OUT2\t OUT3\t OUT4\t OUT5\t OUT6\t OUT7");
	DispStr(8, 4, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 7,
          "Connect the Demo Bd. LEDs to the outputs that you want to demo.");
	DispStr(8, 8, "(See instructions in sample program for complete details)");
	DispStr(8, 14, "<-PRESS 'F4' to return to Edit Mode->");

   // Configure all outputs to be general digital outputs that are high
	channel_bank = 0xFF;
	for(channel = 0; channel < BL_DIGITAL_OUT; ++channel)
	{
   	// Set output to be general digital output
		setDigOut(channel, 1);
   };

	// loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		// update sinking drivers
      mask = 0x01;
		ptr = display;							         //initialize the string pointer
		for(channel = 0; channel < 8; ++channel)	//output to channels 0 - 7
		{
			// format and display level
			ptr += sprintf(ptr, "  %d\t", channel_bank & mask ? 1 : 0);
         mask = mask << 1;
		}
		DispStr(8, 5, display);							//update output status
      digOutBank(0, channel_bank);

		// Display the channels that the user can toggle
		sprintf(display, "Toggle output channel (0 - %d) = ", BL_DIGITAL_OUT - 1);
		DispStr(8, 11, display);
     	do
		{
         while(!kbhit());  // allow user to debug easier
			channel = getchar() - '0';
 		} while(channel < 0 || channel >= BL_DIGITAL_OUT);

		// Toggle the channel selected
      channel_bank = channel_bank ^ (0x01 << channel);
		sprintf(display, "Toggled channel %d", channel);
		DispStr(8, 10, display);
   }
}
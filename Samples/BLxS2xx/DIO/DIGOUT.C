/**************************************************************************

	digout.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
	This program demonstrates using the digOut API function to
   control digital sinking type outputs to toggle LED's ON and
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

   4. Connect the following wires from the controller J10 to the demo
	    board screw terminals:

	   	From DIO0 to LED1
	   	From DIO1 to LED2
	   	From DIO2 to LED3
	   	From DIO3 to LED4

	Instructions:
	=============
	1. Compile and run this program.

	2. Select a output channel via the STDIO window (per the channels you
      connected to the LEDs) to toggle a LED on the demo board.
      NOTE: Since the configurable outputs use sinking drivers, they are
            enabled (and the LED will light) when the port is at logic 0.

	3. To check other outputs that are not connected to the demo board,
      you can use a voltmeter to see the output change states or change
      the wiring to connect other outputs to a LED.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

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
	char channels[8];
	int channel;
	int output_level;

   // Initialize the controller
	brdInit();

   // Configure all outputs to be general digital outputs that are high
	for(channel = 0; channel < 8; ++channel)
	{
   	// Set output to be general digital output
		setDigOut(channel, 1);
		// Set output to be off
		channels[channel] = 1;
   }

	//Display user instructions and channel headings
	DispStr(8, 1, " <<< Configurable Outputs...Setup as Digital Outputs >>>");
	DispStr(8, 3, "DOUT0\tDOUT1\tDOUT2\tDOUT3\tDOUT4\tDOUT5\tDOUT6\tDOUT7");
	DispStr(8, 4, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 7, "Connect the Demo Bd. LEDs to the outputs that you want to "\
   				  "demo.");
	DispStr(8, 8, "(See instructions in sample program for complete details)");

	//loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		// update configurable outputs
		ptr = display;							         //initialize the string pointer
		for(channel = 0; channel < 8; channel++)	//output to channels 0 - 7
		{
			output_level = channels[channel];		//output logic level to channel
			digOut(channel, output_level);
			ptr += sprintf(ptr, "%d\t", output_level);  //format and display level
		}
		DispStr(8, 5, display);							//update output status

		// Display the channels that the user can toggle
		sprintf(display, "Toggle output channel (0 - 7) = ");
		DispStr(8, 11, display);
     	do
		{
      	while(!kbhit());  // allow user to debug easier
			channel = getchar();
         channel -= '0';    // Convert the ascii value to a integer
 		} while(channel < 0 || channel >= 8);

		// Toggle the channel selected
      channels[channel] = channels[channel] ^ 0x01;
		sprintf(display, "Toggled channel %d", channel);
		DispStr(8, 10, display);
   }
}
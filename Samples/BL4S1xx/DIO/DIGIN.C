/**************************************************************************

	digin.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
	This program demonstrates using the digIn API function to read
   digital inputs. Using the DEMO board provided in your kit,
   you will be able to see a digital input toggle from HIGH to LOW
   when a push button on the demo board is pressed.

	Connections:
	============
	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.

	2. Connect a wire from the controller GND, to the DEMO board GND.

	3. Connect a wire from the controller +5V to the DEMO board +V.

	Instructions:
	=============
	1. Connect SW1 - SW4 from the demo board to any four inputs on the
	   controller, choose from IN0 - IN11.
	2. Compile and run this program.
	3. Press any one of the DEMO board switches SW1 - SW4 and you should
	   see the input go LOW on the channel the switch is connected to.
	4. Move connections from the demo board to other inputs and repeat
	   steps 1 - 3.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// update digital inputs for the given channel range
void update_input(int start_channel, int end_channel, int col, int row)
{
	char *ptr;
	char display[80];
	int reading;
   int channel;

	// display the input status for the given channel range
	ptr = display;							         //initialize the string pointer
   for (channel = start_channel; channel <= end_channel; ++channel)
   {
		reading = digIn(channel);	   			//read channel
      ptr += sprintf(ptr, "  %d\t", reading);	//format reading in memory
	}
	DispStr(col, row, display);				   //update input status
}

///////////////////////////////////////////////////////////////////////////

void main()
{

	auto int key, channel;

   // Initialize the controller
	brdInit();

   // Configure all inputs to be general digital inputs
	for(channel = 0; channel < BL_DIGITAL_IN; ++channel)
	{
		setDigIn(channel);
   }

	//Display user instructions and channel headings
	DispStr(8, 1, " <<< Digital inputs 00 - 11 >>>");
	DispStr(8, 3, " IN0 \t IN1 \t IN2 \t IN3 \t IN4 \t IN5 \t IN6 \t IN7 ");
	DispStr(8, 4, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 7, " IN8 \t IN9 \t IN10\t IN11");
	DispStr(8, 8, "-----\t-----\t-----\t-----");

	DispStr(8, 13,
   	 "Connect the Demo Bd. switches to the inputs that you want to toggle.");
	DispStr(8, 14, "(See instructions in sample program for complete details)");
	DispStr(8, 16, "<-PRESS 'F4' to return to Edit Mode->");

	//loop until user presses the "Q" key
	for(;;)
	{
		// update input channels 0 - 7, located at col = 8 row = 5.
		update_input(0, 7, 8, 5);

		// update input channels 8 - 11, located at col = 8 row = 9.
		update_input(8, 11, 8, 9);
   }
}
///////////////////////////////////////////////////////////////////////////



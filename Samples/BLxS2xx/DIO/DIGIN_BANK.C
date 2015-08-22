/**************************************************************************
	digin_bank.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
	This program demonstrates using the digInBank API function to read
   digital inputs. Using the demo board provided in your kit, you will
   be able to see a digital input toggle from HIGH to LOW when a
   push button on the demo board is pressed.  The banking functions allow
   banks of IO to be input or output more efficiently.

	Connections:
	============
	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.

	2. Connect a wire from the controller J10 pin 5 GND to the DEMO board GND.

	3. Connect a wire from the controller J7 pin 6 (+5V) to the DEMO board +V.

	4. Make sure +5V pull-ups are enabled on the BLxS2xx board,
      JP9 OR JP8 (pins 3&4) for the connector you use (JP9 for connector J10,
      JP8 for connector J9).

	Instructions:
	=============
	1. Connect SW1 - SW4 from the demo board to any four inputs on
	   connectors J10 or J9, choose from DIO0 - DIO15.
	2. Compile and run this program.
	3. Press any one of the DEMO board switches SW1 - SW4 and you should
	   see the input go LOW on the channel the switch is connected to.
	4. Move connections from the demo board to other inputs and repeat
	   steps 1 - 4.

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

// update digital inputs for the given channel range
void update_input(int bank, int col, int row)
{
	char *ptr;
	char display[80];
	int data, reading, index;
   char mask;

   data = digInBank(bank);

	// display the input status for the given channel range
   mask = 0x01;
	ptr = display;							         //initialize the string pointer
	for(index = 0; index < 8; ++index)	      //update channel status
   {
   	reading = (data & mask) ? 1 : 0;
		ptr += sprintf(ptr, "%d\t", reading);	//format reading and add to string
   	mask <<= 1;
   }
	DispStr(col, row, display);				   //update input status
}

///////////////////////////////////////////////////////////////////////////

void main()
{

	int key, channel;

   // Initialize the controller
	brdInit();

   // Set configurable I/O 0-15 to be general digital inputs
	for(channel = 0; channel < 16; ++channel)
	{
		setDigIn(channel);
   }

	//Display user instructions and channel headings
	//Display user instructions and channel headings
	DispStr(8, 1, " <<< Configurable I/O set as inputs 00 - 15 >>>");
	DispStr(8, 3, "DIN00\tDIN01\tDIN02\tDIN03\tDIN04\tDIN05\tDIN06\tDIN07");
	DispStr(8, 4, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 7, "DIN08\tDIN09\tDIN10\tDIN11\tDIN12\tDIN13\tDIN14\tDIN15");
	DispStr(8, 8, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 13, "Connect the Demo Bd. switches to the inputs that you what "\
   					"to toggle.");
	DispStr(8, 14, "(See instructions in sample program for complete details)");
	DispStr(8, 16, "<-PRESS F4 to return to Edit Mode.");

	//loop until user presses F4
	for(;;)
	{
		// update bank 0, located at col = 8 row = 5.
		update_input(0, 8, 5);

		// update bank 1, located at col = 8 row = 9.
		update_input(1, 8, 9);
   }
}
///////////////////////////////////////////////////////////////////////////



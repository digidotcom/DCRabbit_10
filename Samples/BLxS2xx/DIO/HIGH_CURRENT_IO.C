/**************************************************************************
	high_current_io.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
	This program demonstrates the use of the high-current outputs by
   toggling LED's ON and OFF on the demo board provided with your kit.

	Attention!!!
	============
	1. The high-current output HOUT0 is configured for sourcing to
      provide power to the DEMO board.

   2. Outputs HOUT1-HOUT2 are configured to demonstrate tristate
      operation for control of LEDS on the Demo board.

   3. Output HOUT3 is configured to demonstrate sinking operation
      for control of a LED on the Demo board.


   High-Current +K1 Power Connection:
   =================================

	****WARNING****: When using the J7 connector, be sure to insulate or cut
      the exposed wire from the wire leads you are not using.  The only
      connection required from J7 for any of the sample programs is +5v.

   1. Connect J7 pin 3 or 6 (+5v) to +K1 (J3 pin 7 or 10)


   Connections:
	============
	1. DEMO board jumper settings:
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller J3 pin 2 GND to the DEMO board GND.

	3. Connect LED1 - LED3 from the DEMO board to digital outputs
   	HOUT1 - HOUT3 on connector J3.

   4. Connect HOUT0 on connector J3 to +V on the Demo board. (this
   	is going to be used for the power supply on the demo board)

	Instructions:
	=============
	1. Compile and run this program.

	2. The program will prompt you for your channel selection, select
	   Output Channel HOUT0.

	3. After you have made the channel selection you'll be prompted to
	   select the logic level, set the HOUT0 channel to a high logic
	   level to give the LED's a voltage source.

	4. Select output channel HOUT1 - HOUT3 via the STDIO window to
      toggle a LED on the demo board.

**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

// Set the STDIO cursor location and display a string
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
	char channels[4];
	int channel;
	int output_level;

   // Initialize the controller
	brdInit();

   // Configure the high-current outputs for the following operation:
   // HOUT0............Sourcing operation
   // HOUT1-HOUT2......Tristate operation
   // HOUT3............Sinking operation
   digOutConfig_H(0x01);			// Set Hout0 Sourcing and Hout3 Sinking
   digOutTriStateConfig_H(0x06);  // Set Hout1 & Hout2 for Tristate

	// Display user instructions and channel headings
	DispStr(8, 1, " <<< Sourcing output channel   = HOUT0         >>>");
	DispStr(8, 2, " <<< Tristate output channels  = HOUT1-HOUT2   >>>");
   DispStr(8, 3, " <<< Sinking output channel    = HOUT3         >>>");
	DispStr(8, 5, "HOUT0\tHOUT1\tHOUT2\tHOUT3");
	DispStr(8, 6, "-----\t-----\t-----\t-----");

	DispStr(8, 9, "Connect the Demo Bd. LED's to the outputs that you want to demo.");
	DispStr(8, 10, "(See instructions in sample program for complete details)");
	DispStr(8, 16, "<-PRESS 'Q' TO QUIT->");

	// Set the channel array to desired default values.
   // HOUT0.............Set to 0 for sourcing transistor to be OFF.
   // HOUT1-HOUT2.......Set to 2 for both sinking/sourcing transistors to be OFF.
   // HOUT3.............Set to 1 for sinking transistor to be OFF.

   channels[0] = 0;	// Set value for Hout0 Sourcing Output to be OFF
   channels[1] = 2;  // Set value for Hout1 Tristate Output to be OFF
   channels[2] = 2;  // Set value for Hout2 Tristate Output to be OFF
   channels[3] = 1;  // Set value for Hout3 Sinking  Output to be OFF

	// Loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		// Update outputs
		ptr = display;							         //initialize the string pointer
		for(channel = 0; channel <= 3; ++channel)	//output to channels 0 - 3
		{
			output_level = channels[channel];		//output logic level to channel
         if(channel == 1 || channel == 2)
         {
         	digOutTriState_H(channel, output_level);
         } else {
         	digOut_H(channel, output_level);
         }

         //format logic level for display
			ptr += sprintf(ptr, "%d\t", output_level);
		}
		DispStr(8, 7, display);							//update output status


		// Wait for user to make output channel selection or exit program
		sprintf(display, "Select output channel 0 - 3 = ");
		DispStr(8, 12, display);
     	do
		{
      	while(!kbhit());  // allow user to debug easier
			channel = getchar();
			if (channel == 'Q' || channel == 'q')	// check if it's the q or Q key
			{
      		exit(0);
     		}
		}while(channel < '0' && channel > '3');

		// Convert the ascii hex value to an interger
		channel = channel - '0';

      // Display the channel that ths user has selected
      printf("%d", channel);

      // Display proper logic level range for given ouput channel
      // configuration.
      if(channel == 0 || channel == 3)
      {
         // Sinking or Sourcing logic level selection
			sprintf(display, "Select logic level (0 - 1) = ");
      }
      else
      {
         // Tristate logic level selection
			sprintf(display, "Select logic level (0 - 2) = ");
     	}
      DispStr(8, 13, display);
		while(1)
		{
      	while(!kbhit());  // allow user to debug easier
			output_level = getchar();
         // check if it's the q or Q key
			if (output_level == 'Q' || output_level == 'q')
			{
      		exit(0);
     		}
     		output_level -= '0';

         // Check for valid logic level for given output type
         if(channel == 0 || channel == 3)
         {
				if((output_level >= 0) && (output_level <= 1))
            {
            	break;
            }
         }
         else if(channel == 1 || channel == 2)
         {
            if((output_level >= 0) && (output_level <= 2))
            {
            	break;
            }
        	}
      }
      // Save and display output level
     	channels[channel] = output_level;
      printf("%d", output_level);

  		// Clear channel and logic level selection prompts
  		DispStr(8, 12, "                                                     ");
  		DispStr(8, 13, "                                                     ");
   }
}
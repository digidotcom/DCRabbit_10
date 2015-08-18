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
/********************************************************************

	controlled.c

	This program is used with RCM4200 series controllers with
	prototyping boards.

	Description
	===========
	This sample program demonstrates controlling port outputs
	from STDIO by toggling LED's on the prototyping board.

	I/O control			On proto-board
	--------------		----------------------
	Port B bit 2		DS2, LED
	Port B bit 3		DS3, LED

	Instructions
	============
	1. Compile and run this program.

	2. The program will prompt you for an LED, DS2 or DS3 selection.
		Make a selection from your PC keyboard.

	3. After you have made the LED selection you'll be prompted to
	   select an ON or OFF state. Logic LOW will light up the LED.

*********************************************************************/
#class auto

#use RCM42xx.LIB

#define DS2 2
#define DS3 3

#define DS2_BIT 2
#define DS3_BIT 3

////////
// output function to control protoboard LED's
////////
void pbLedOut(int led, int onoff)
{
	if(led == DS2)
   	BitWrPortI(PBDR, &PBDRShadow, onoff, DS2_BIT);
	else
   	BitWrPortI(PBDR, &PBDRShadow, onoff, DS3_BIT);
}

////////
// Set the STDIO cursor location and display a string
////////
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

///////////////////////////////////////////////////////////////////////////
main()
{

	auto char s[128];
	auto char display[128];
	auto char channels[8];
	auto int output_status, channel;
	auto int output_level;
	auto unsigned int outputChannel;

	// Initialize I/O pins
	brdInit();

	// Display user instructions and channel headings
	DispStr(8, 2, "<<< Proto-board LED's   >>>");
	DispStr(8, 4, "DS2\tDS3");
	DispStr(8, 5, "-----\t-----");

	DispStr(8, 10, "From PC keyboard:");
	DispStr(8, 21, "< Press 'Q' To Quit >");

	for(channel = DS2; channel <=DS3 ; channel++)
	{
		channels[channel] = 1;		// Indicate output is OFF
		pbLedOut(channel, 1);
	}

	// Loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		// Update high current outputs
		display[0] = '\0';								//initialize for strcat function
		for(channel = DS2; channel <= DS3; channel++)	//output to DS2 and DS3 only
		{
			output_level = channels[channel];		//output logic level to channel
			pbLedOut(channel, output_level);
			sprintf(s, "%s\t", output_level?"OFF":"ON ");		//format logic level for display
			strcat(display,s);							//add to display string
		}
		DispStr(8, 6, display);							//update output status

		// Wait for user to make output channel selection or exit program
		sprintf(display, "Select 2=DS2 or 3=DS3 to toggle LED's");
		DispStr(8, 12, display);
     	do
		{
			channel = getchar();
			if (channel == 'Q' || channel == 'q')		// check if it's the q or Q key
			{
      		exit(0);
     		}
     		channel = channel - 0x30;		// convert ascii to integer
		} while (!((channel >= DS2) && (channel <= DS3)));

		// Display the channel that the user has selected
		sprintf(display, "Selected DS%d to toggle               ", channel);
		DispStr(8, 12, display);

		// Wait for user to select logic level or exit program
		sprintf(display, "Select 1=OFF or 0=ON");
		DispStr(8, 13, display);
		do
		{
			output_level = getchar();
			if (output_level == 'Q' || output_level == 'q')		// check if it's the q or Q key
			{
      		exit(0);
     		}
     		output_level = output_level - 0x30;
		} while (!((output_level >= 0) && (output_level <= 1)));
		sprintf(display, "Selected %s         ", output_level?"OFF":"ON ");
     	DispStr(8, 13, display);
     	channels[channel] = output_level;

  		// Clear channel and logic level selection prompts
  		DispStr(8, 12, "                                                  ");
  		DispStr(8, 13, "                                                  ");
   }
}
///////////////////////////////////////////////////////////////////////////




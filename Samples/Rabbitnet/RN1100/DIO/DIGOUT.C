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

	digout.c

	This sample program is for RabbitNet RN1100 Digital I/O boards.

   Description
	===========
	This program demonstrates the use of the outputs configured	as SINKING
	and SOURCING type output.

	The sample program requires the use	the DEMO board that was provided in
   your development kit so you can see	the LED's toggle ON/OFF via the outputs.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1100 as the search criteria.

	Connections
	===========
	Connect +K1 and GND (connector J4) to external power source +5V.

	When the controller is plugged into to the demo board the
	following connections are readily available.

	RN1100, J3		 Demo Board
	----------		 ----------
		OUT00 <----> DS1
		OUT01 <----> DS2
		OUT02 <----> DS3
		OUT03 <----> DS4
        +K1 <----> +5V


	Instructions
	============
	1. Compile and run this program.

	2. The program will prompt you for your channel selection, select
	   Output Channel OUT00.

	3. After you have made the channel selection you'll be prompted to
	   select the logic level, set the OUT00 channel to a high logic
	   level.

	4. At this point your ready to start toggling the LEDS, DS1-DS4, by
		selecting channels OUT01 - OUT03, and changing the logic level.

	5. To check other outputs use a voltmeter to verify that the channels
   have changed to the level that you have set it to.

**************************************************************************/
#class auto

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board

//configure to sinking safe state
#define OUTCONFIG	0xFFFF


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

	auto char s[128], status;
	auto char display[128];
	auto char channels[16];
	auto int output_status, channel, device0;
	auto int output_level;
	auto unsigned int outputChannel;
	auto rn_search newdev;

	brdInit();                 //initialize controller
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   //search for device match
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

	status = rn_digOutConfig(device0, OUTCONFIG);  //configure to sinking safe state

	// Set the channel array to reflect the output channel default value
	outputChannel = OUTCONFIG;
	for(channel = 0; channel <=15 ; channel++)
	{
		// Set outputs to be OFF, for both sinking
		// and sourcing type outputs.
		channels[channel] = outputChannel & 0x0001;
		outputChannel = outputChannel >> 1;
	}

	// Display user instructions and channel headings
	DispStr(8, 1, "\t\t<<< Sinking output channels  = OUT00-OUT15   >>>");
	DispStr(8, 4, "OUT00\tOUT01\tOUT02\tOUT03\tOUT04\tOUT05\tOUT06\tOUT07");
	DispStr(8, 5, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 9, "OUT08\tOUT08\tOUT10\tOUT11\tOUT12\tOUT13\tOUT14\tOUT15");
	DispStr(8,10 , "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 14, "Connect the Demo Bd. LED's to the outputs that you want to demo.");
	DispStr(8, 15, "(See instructions in sample program for complete details)");
	DispStr(8, 21, "<-PRESS 'Q' TO QUIT->");

	// Loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		// Update high current outputs
		display[0] = '\0';								//initialize for strcat function
		for(channel = 0; channel <= 7; channel++)	//output to channels 0 - 7
		{
			output_level = channels[channel];		//output logic level to channel
			status = rn_digOut(device0, channel, output_level, 0);
			sprintf(s, "%d\t", output_level);		//format logic level for display
			strcat(display,s);							//add to display string
		}
		DispStr(8, 6, display);							//update output status


		display[0] = '\0';
		for(channel = 8; channel <= 15; channel++)	//output to channels 8 - 9
		{
			output_level = channels[channel];			//output logic level to channel
			status = rn_digOut(device0, channel, output_level, 0);
			sprintf(s, "%d\t", output_level);
			strcat(display,s);
		}
		DispStr(8, 11, display);

		// Wait for user to make output channel selection or exit program
		sprintf(display, "Select output channel 0 - 15 (Input Hex 0-F) = ");
		DispStr(8, 17, display);
     	do
		{
			channel = getchar();
			if (channel == 'Q' || channel == 'q')		// check if it's the q or Q key
			{
      		exit(0);
     		}
		}while(!isxdigit(channel));

		// Convert the ascii hex value to a integer
		if( channel >= '0' && channel <='9')
		{
			channel = channel - 0x30;
		}
      else
      {
  			if( channel >= 'A' && channel <='F')
	      	channel = channel - 0x3C;
         else
	      	channel = channel - 0x57;
      }

		// Display the channel that ths user has selected
		sprintf(display, "Select output channel 0 - 15 (Input Hex 0-F) = %d", channel);
		DispStr(8, 17, display);

		// Wait for user to select logic level or exit program
		sprintf(display, "Select logic level = ");
		DispStr(8, 18, display);
		do
		{
			output_level = getchar();
			if (output_level == 'Q' || output_level == 'q')		// check if it's the q or Q key
			{
      		exit(0);
     		}
     		output_level = output_level - 0x30;

		} while(!((output_level >= 0) && (output_level <= 1)));

		sprintf(display, "Select logic level = %d", output_level);
     	DispStr(8, 18, display);
     	channels[channel] = output_level;

  		// Clear channel and logic level selection prompts
  		DispStr(8, 17, "                                                  ");
  		DispStr(8, 18, "                                                  ");
   }
}
///////////////////////////////////////////////////////////////////////////


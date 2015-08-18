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

	digbankout.c

	This sample program is for RabbitNet RN1100 Digital I/O boards.

   Description
	===========
	This program demonstrates the use of bank outputs configured as SINKING
	and SOURCING type outputs.

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

	2. The program will prompt you for your output bank selection, select
	   1 for the bank of OUT00 to OUT07.

	3. After you have made the selection you'll be prompted to a hex byte
		value, enter AA.

	4.	Note that LED's DS1 and DS3 will be lit.

**************************************************************************/
#class auto

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board

#define BANK1 1		//OUT00 to OUT07
#define BANK2 2		//OUT08 and OUT15
#define OUTCONFIG		0xFFF	   //configure to sinking safe state

// screen foreground colors
#define	BLACK		"\x1b[30m"
#define	RED		"\x1b[31m"
#define	GREEN		"\x1b[32m"
#define	BLUE		"\x1b[34m"

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *color, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s%s", x, y, color, s);
}

//------------------------------------------------------------------------
// Set to initially disable outputs OUT9-OUT0.
//------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////

void main()
{
	auto char s[4];
	auto char display[128];
	auto int channel, output_level, output1, output2, device0, status;
	auto int up4, lo4;
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

	// Display user instructions and channel headings
	DispStr(8, 1, GREEN, "\t<<< Sinking output channels  = OUT00-OUT15   >>>");
	DispStr(8, 4, BLACK, "OUT00\tOUT01\tOUT02\tOUT03\tOUT04\tOUT05\tOUT06\tOUT07");
	DispStr(8, 5, BLACK, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 9, BLACK, "OUT08\tOUT08\tOUT10\tOUT11\tOUT12\tOUT13\tOUT14\tOUT15");
	DispStr(8,10, BLACK, "-----\t-----\t-----\t-----\t-----\t-----\t-----\t-----");

	DispStr(8, 14, RED, "Connect the Demo Bd. LED's to the outputs that you want to demo.");
	DispStr(8, 15, RED, "(See instructions in sample program for complete details)");
	DispStr(8, 21, RED, "<-PRESS 'Q' TO QUIT->");


	//intialize output values
	output1 = 0xff;		// bank 1
	output2 = 0xff;		// bank 2

	// Loop until user presses the upper/lower case "Q" key
	for(;;)
	{
		rn_digBankOut(device0, BANK1, output1, 0);
		output_level=output1;
		display[0] = '\0';									//initialize for strcat function
		//display output values
		for(channel = 0; channel <= 7; channel++)
		{
			sprintf(s, "%d\t", output_level&0x0001);	//format logic level for display
			strcat(display,s);								//add to display string
			output_level >>= 1;								//output logic level to channel
		}
		DispStr(8, 6, BLUE, display);

		rn_digBankOut(device0, BANK2, output2, 0);
		output_level=output2;
		display[0] = '\0';
		//display output values
		for(channel = 8; channel <= 15; channel++)
		{
			sprintf(s, "%d\t", output_level&0x0001);	//format logic level for display
			strcat(display,s);								//add to display string
			output_level >>= 1;								//output logic level to channel
		}
		DispStr(8, 11, BLUE, display);

		// Wait for user to make output channel selection or exit program
		sprintf(display, "Enter '1' to change outputs 0 to 7 or '2' for 8 to 15 .... Bank ");
		DispStr(8, 17, RED, display);
		gets(s);
		if (s[0] == 'Q' || s[0] == 'q')		// check if it's the q or Q key
		{
     		exit(0);
  		}

		channel = atoi(s);

		// Wait for user to select logic level or exit program
		sprintf(display, "Enter hex byte value (ie: 3F or 0A)    ");
		DispStr(8, 18, RED, display);
		gets(s);

		if (s[0] > 0x39)
			up4 = toupper(s[0])-0x37;
		else
			up4 = s[0]-0x30;
		up4 <<= 4;

		if (s[1] > 0x39)
			lo4 = toupper(s[1])-0x37;
		else
			lo4 = s[1]-0x30;

		if (channel == BANK1)
			output1 = up4|lo4;
		else
			output2 = up4|lo4;

  		// Clear channel and logic level selection prompts
  		DispStr(8, 17, BLACK, "                                                                 ");
  		DispStr(8, 18, BLACK, "                                                                 ");
   }
}
///////////////////////////////////////////////////////////////////////////




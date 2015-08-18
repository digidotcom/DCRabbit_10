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

	digbankin.c

	This sample program is for RabbitNet RN1100 Digital I/O boards.


   Description
	===========
	This program demonstrates the use of the digital input bank.

	Using the provided DEMO board in your controller kit, you will see
   an input channel toggle from HIGH to LOW when pressing a push button on
	the DEMO board.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1100 as the search criteria.

	Connections
	===========
	Connect RN1100, +K1 and GND (connector J4) to external power
   source +5V.  Place JP3 jumper across VCC for pull-up.
   On the Demo board, place H2 jumpers across pins 4-6 and 3-5.

	When the controller is plugged into to the demo board the
	following connections are readily available.

	RN1100, J3		Demo Board
	----------		----------
			IN00 <->	S1
			IN01 <->	S2
			IN02 <->	S3
			IN03 <->	S4
          +K1 <-> +5V
          GND <-> GND

	Instructions
	============
	1. Compile and run this program.
	2. Press any one of the DEMO board switches S1 - S4 and you should
	   see the input go LOW on the channel the switch is connected to.
	3. IN04 to IN23 can be viewed by touching the line with a GND signal.

**************************************************************************/
#class auto

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board

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

// update digital inputs for the given channel range
void update_input(int handle, int bank, int col, int row)
{
	auto char s[40], reading;
	auto char display[80];
	auto int i;

	// display the input status for the given channel range
	display[0] = '\0';							//initialize for strcat function

	rn_digBankIn(handle, bank, &reading, 0);
	for (i=0; i<=7; i++)
	{
		sprintf(s, "%d\t", reading&1);			//format reading in memory
		strcat(display,s);						//append reading to display string
		reading = reading >> 1;
	}

	DispStr(col, row, BLUE, display);			//update input status
}


///////////////////////////////////////////////////////////////////////////

void main()
{
	auto int key, reading, channel,device0;
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

	//Display user instructions and channel headings
	DispStr(8, 1, GREEN, "\t\t<<< Digital inputs 0 - 23 >>>");
	DispStr(8, 3, BLACK, "IN00\tIN01\tIN02\tIN03\tIN04\tIN05\tIN06\tIN07");
	DispStr(8, 4, BLACK, "----\t----\t----\t----\t----\t----\t----\t----");

	DispStr(8, 7, BLACK, "IN08\tIN09\tIN10\tIN11\tIN12\tIN13\tIN14\tIN15");
	DispStr(8, 8, BLACK, "----\t----\t----\t----\t----\t----\t----\t----");

	DispStr(8,11, BLACK, "IN16\tIN17\tIN18\tIN19\tIN20\tIN21\tIN22\tIN23");
	DispStr(8,12, BLACK, "----\t----\t----\t----\t----\t----\t----\t----");


	DispStr(8, 16, RED, "Connect the Demo Bd. switches to the inputs that you what to toggle.");
	DispStr(8, 17, RED, "(See instructions in sample program for complete details)");
	DispStr(8, 19, RED, "<-PRESS 'Q' TO QUIT->");

	//loop until user presses the "Q" key
	for(;;)
	{
		// update input channels 0 - 7   (display at col = 8 row = 5)
		update_input(device0, 1, 8, 5);

		// update input channels 8 - 15  (display at col = 8 row = 9)
		update_input(device0, 2, 8, 9);

		// update input channels 16 - 23  (display at col = 8 row = 13)
		update_input(device0, 3, 8, 13);

		if(kbhit())
		{
			key = getchar();
			if (key == 'Q' || key == 'q')		// check if it's the q or Q key
			{
				while(kbhit()) getchar();
      		exit(0);
     		}
		}
   }
}
///////////////////////////////////////////////////////////////////////////



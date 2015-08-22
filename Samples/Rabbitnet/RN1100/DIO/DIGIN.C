/**************************************************************************

	digin.c
   Z-World, 2003

	This sample program is for RabbitNet RN1100 Digital I/O boards.

	Description
	===========
	This program demonstrates the use of the digital inputs.
	Using the provided DEMO board in your controller kit, you will see
   an input channel toggle from High to Low when pressing a push button on
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
	   see the input go Low on the channel the switch is connected to.
	3. IN04 to IN23 can be viewed by touching the line with a GND signal.

**************************************************************************/
#class auto

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board


// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// update digital inputs for the given channel range
void update_input(int handle, int start_channel, int end_channel, int col, int row)
{
	auto char s[40];
	auto char display[80];
	auto char reading;

	// display the input status for the given channel range
	display[0] = '\0';							//initialize for strcat function
	while(start_channel <= end_channel)		//read channels
	{
		rn_digIn(handle, start_channel++, &reading, 0);
		sprintf(s, "%d\t", reading);			//format reading in memory
		strcat(display,s);						//append reading to display string
	}
	DispStr(col, row, display);				//update input status
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
	device0 = rn_find(&newdev);
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

	//Display user instructions and channel headings
	DispStr(8, 1, "\t\t<<< Digital inputs 0 - 23: >>>");
	DispStr(8, 3, "IN00\tIN01\tIN02\tIN03\tIN04\tIN05\tIN06\tIN07");
	DispStr(8, 4, "----\t----\t----\t----\t----\t----\t----\t----");

	DispStr(8, 7, "IN08\tIN09\tIN10\tIN11\tIN12\tIN13\tIN14\tIN15");
	DispStr(8, 8, "----\t----\t----\t----\t----\t----\t----\t----");

	DispStr(8,11, "IN16\tIN17\tIN18\tIN19\tIN20\tIN21\tIN22\tIN23");
	DispStr(8,12, "----\t----\t----\t----\t----\t----\t----\t----");


	DispStr(8, 16, "Connect the Demo Bd. switches to the inputs that you what to toggle.");
	DispStr(8, 17, "(See instructions in sample program for complete details)");
	DispStr(8, 19, "<-PRESS 'Q' TO QUIT->");

	//loop until user presses the "Q" key
	for(;;)
	{
		// update input channels 0 - 7   (display at col = 8 row = 5)
		update_input(device0, 0, 7, 8, 5);

		// update input channels 8 - 15  (display at col = 8 row = 9)
		update_input(device0, 8, 15, 8, 9);

		// update input channels 16 - 23  (display at col = 8 row = 13)
		update_input(device0, 16, 23, 8, 13);

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



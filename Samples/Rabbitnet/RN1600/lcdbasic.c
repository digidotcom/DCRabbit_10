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
	lcdbasic.c

	This sample program is intended for RN1600 RabbitNet
   Keypad/Display Interface card.

   Description
   ============
   This program demonstrates use of the 2x20 display
   found in the development kit.  It will demonstrate various
   display functions.

   Note: Backlight function will work on displays that have
   backlight capability.

	Instructions
   ============
   0. Install the 2x20 display onto J5.
	1. Compile and run this program.
   2. Observe the display screen.

*********************************************************************/
#class auto
#memmap xmem  // Required to reduce root memory usage

/////
//local macros
/////
#define ON 1
#define OFF 0
#define DISPROWS 2     //number of lines in display
#define DISPCOLS 20    //number of columns in display

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1600			//RN1600 KDIF card


char * const dispdemo[] = {"Backlight ON",
								"Backlight OFF",
                        "Cursor On",
                        "Cursor Off",
                        "Cursor BLINK On",
                        "Cursor BLINK Off",
                        "Buzzer On",
                        "Buzzer Off",
                        "Display On",
                        "Display Off",
                        "Display Shift"};


nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


/***************************************************************************
***************************************************************************/
void main (	void	)
{
 	auto int device0, light, blank;
   auto rn_search newdev;
   auto unsigned int i;

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------
	brdInit();					// Initialize the controller
   rn_init(RN_PORTS, 1);   // Initialize controller RN ports

   // Verify that the Rabbitnet display board is connected
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

   // Initialize Display and buzzer drivers
   // Note: Functions brdInit() and rn_init() must executed before initializing
   //       display and buzzer drivers.
	rn_dispInit(device0, DISPROWS, DISPCOLS);

	rn_dispPrintf (device0, 0, "Starting...");
   rn_keyBuzzerAct(device0, 100, 0);
   msDelay(500);
	rn_keyBuzzerAct(device0, 100, 0);
   msDelay(1000);

	while(1)
	{
   	rn_dispClear(device0, 0);

      //show backlight
		rn_dispPrintf (device0, 0, "%s", dispdemo[1]);
		rn_dispBacklight(device0, OFF, 0);		   	//turn off backlight
      msDelay(2000);
	   rn_dispGoto(device0, 5, 1, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[0]);
		rn_dispBacklight(device0, ON, 0);	//turn on backlight
      msDelay(2000);

      //show cursor styles
   	rn_dispClear(device0, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[2]);
  		rn_dispCursor(device0, RNDISP_CURON, 0);
      msDelay(2000);
	   rn_dispGoto(device0, 5, 1, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[3]);
	  	rn_dispCursor(device0, RNDISP_CUROFF, 0);
      msDelay(2000);
   	rn_dispClear(device0, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[4]);
  		rn_dispCursor(device0, RNDISP_CURBLINKON, 0);
      msDelay(2000);
	   rn_dispGoto(device0, 0, 1, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[5]);
	  	rn_dispCursor(device0, RNDISP_CURBLINKOFF, 0);
      msDelay(2000);

      //show buzzer
   	rn_dispClear(device0, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[6]);
     	rn_keyBuzzer(device0, ON, 0);    //on
      msDelay(500);
	   rn_dispGoto(device0, 5, 1, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[7]);
     	rn_keyBuzzer(device0, OFF, 0);    //off
      msDelay(2000);

      //show display off but still working
   	rn_dispClear(device0, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[9]);
      msDelay(2000);
	  	rn_dispOnoff(device0, OFF, 0);
	   rn_dispGoto(device0, 0, 0, 0);
      for (i='A'; i<='Z'; i++)
      {
      	rn_dispPutc(device0, i, 0);
      }
      msDelay(1000);
  		rn_dispOnoff(device0, ON, 0);
      msDelay(2000);

      //show display shift right then left
   	rn_dispClear(device0, 0);
		rn_dispPrintf (device0, 0, "%s", dispdemo[10]);
      msDelay(1000);
	   rn_dispGoto(device0, 15, 1, 0);
      rn_dispData(device0, '>', 1, 0);
	   rn_dispCmd(device0, 0x07, 1, 0);  //enable display shift
      for (i=0; i<20; i++)
      {
	  		rn_dispCmd(device0, 0x1C, 200, 0);  //display shift right
      }
	   rn_dispGoto(device0, 5, 1, 0);
      rn_dispData(device0, '<', 1, 0);
      for (i=0; i<19; i++)
      {
		   rn_dispCmd(device0, 0x18, 200, 0);  //display shift left
      }
   	rn_dispCmd(device0, 0x06, 1, 0);  //disable display shift
      msDelay(2000);

   }
}
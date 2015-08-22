/*******************************************************************
	Flashled.c

 	Digi International, Copyright (C) 2008.  All rights reserved.

   This program is used with RCM56xxW or RCM66xxW series controllers and
   interface board.

   Description
   ===========
   This program uses Dynamic C's costatement feature to flash
   the LED on the interface board.

	I/O control       On Base-board
	--------------    ----------------------
	Port D bit 0		DS1 (LED)

   Jumper settings (Interface Board)
   ----------------------------
   JP1   1-2 program mode
         5-6 enables DS1 (LED)
         7-8 enables S1  (button), optional

         2    4    6   8
         o    o    o   o
         |         |
         o    o    o   o
         1    3    5   7

   Instructions
   ============
   1.  Make sure jumpers are connected as shown above.
   2.  Compile and run this program.
   3.  DS1 (LED) flashes on/off.

*******************************************************************/

#if RCM6600W_SERIES
	#use "rcm66xxw.lib"
#else
	#use "rcm56xxw.lib"
#endif

#define LEDON  0
#define LEDOFF 1

#define DS1 0

main()
{
	brdInit();

	while (1)
	{
		costate
		{
	      BitWrPortI(PDDR, &PDDRShadow, LEDON, DS1);
			waitfor(DelayMs(50));       // On for 50 ms.
	      BitWrPortI(PDDR, &PDDRShadow, LEDOFF, DS1);
			waitfor(DelayMs(100));      // Off for 100 ms.
		}
	}
}
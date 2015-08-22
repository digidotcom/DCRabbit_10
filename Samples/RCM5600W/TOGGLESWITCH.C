/*******************************************************************

	toggleswitch.c

 	Digi International, Copyright (C) 2008.  All rights reserved.

   This program is used with RCM56xxW or RCM66xxW series controllers and
   interface board and optionally with a digital I/O accessory board,
   available with the deluxe dev kit.

   See "Add Additional Boards" in the User's Manual for instructions on
   how to attach the accessory board.

   Description
   ===========
   This program monitors switch S1 and LED DS1 on the interface board, or
   if DIGITAL_IO_ACCESSORY is defined, monitors switches S1-S4 and
   LEDs DS1-DS4 on the digital I/O accessory board, while a corresponding
   button switch is pressed.

   Use the following jumper placements on the interface board:

	I/O control       On Interface Board
	--------------    ------------------
	Port D bit 0		DS1, LED

   Jumper settings (Interface Board)
   ---------------------------------
   JP1   1-2 program mode
         5-6 enables DS1 (LED)
         7-8 enables S1  (button)

         2    4    6   8
         o    o    o   o
         |         |   |
         o    o    o   o
         1    3    5   7

   Use the following jumper placements on the digital I/O
   accessory board:

	I/O control       On Digital I/O board
	--------------    ---------------------
	Port A bits 4-7  	LEDs DS1-DS4
	Port B bits 4-7	Button switches S1-S4

   Jumper settings (Digital I/O board)
   -----------------------------------
   JP7   2-4, 3-5

      1 o    o 2
             |
      3 o    o 4
        |
      5 o    o 6

      7 o    o 8


   JP5   1-2, 3-4, 5-6, 7-8
   JP8   1-2, 3-4, 5-6, 7-8

         2    4    6   8
         o    o    o   o
         |    |    |   |
         o    o    o   o
         1    3    5   7

   Instructions
   ============
   1.  Connect jumpers as shown above for the interface or accessory board.
   2.  Compile and run this program.
   3.  Press digital switches S1-S4 to light up LEDs DS1-DS4.

*******************************************************************/

// Comment the following define to use only interface board with its one
// button and corresponding LED, uncomment to use the digital I/O accessory
// board with its four buttons and corresponding LEDs.
//#define DIGITAL_IO_ACCESSORY

#ifdef DIGITAL_IO_ACCESSORY
   #define DS1 4
   #define DS2 5
   #define DS3 6
   #define DS4 7
   #define S1  4
   #define S2  5
   #define S3  6
   #define S4  7
#else
   #define DS1 0
   #define DS4 0
   #define S1  1
   #define S4  1
#endif

#if RCM6600W_SERIES
	#use "rcm66xxw.lib"
#else
	#use "rcm56xxw.lib"
#endif

#ifdef DIGITAL_IO_ACCESSORY
InitIO()
{
   // Make Port B pins for switch inputs
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S1);
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S2);
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S3);
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S4);

   // Set Port A pins for LEDs low
   BitWrPortI(PADR, &PADRShadow, 1, DS1);
   BitWrPortI(PADR, &PADRShadow, 1, DS2);
   BitWrPortI(PADR, &PADRShadow, 1, DS3);
   BitWrPortI(PADR, &PADRShadow, 1, DS4);
}
#endif

main()
{
   int i;

   brdInit();
#ifdef DIGITAL_IO_ACCESSORY
	InitIO();
#endif

   while(1)
   {
      for(i = S1; i <= S4; i++)
      {
#ifdef DIGITAL_IO_ACCESSORY
         if(!BitRdPortI(PBDR, i))
#else
         if(!BitRdPortI(PDDR, i))
#endif
         {
            // Light corresponding LED
#ifdef DIGITAL_IO_ACCESSORY
            BitWrPortI(PADR, &PADRShadow, 0, i);
#else
            BitWrPortI(PDDR, &PDDRShadow, 0, i-1);
#endif
         }
         else
         {
#ifdef DIGITAL_IO_ACCESSORY
            BitWrPortI(PADR, &PADRShadow, 1, i);  // LED off
#else
            BitWrPortI(PDDR, &PDDRShadow, 1, i-1);
#endif
         }
		}
   }
}


/*******************************************************************

	SwitchLEDs.c

 	Digi International, Copyright (C) 2008.  All rights reserved.

   This program is used with RCM57xx or RCM67xx series controllers
   with digital I/O accessory  boards.

   Description
   ===========
   This program monitors switches S1, S2, S3 and S4 on the
   Digital I/O accessory boards and lights LEDs DS1-DS4 while
   a corresponding button switch is pressed.

	I/O control       On Digital I/O board
	--------------    ----------------------
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
   1.  Make sure jumpers are connected as shown above.
   2.  Compile and run this program.
   3.  Press digital switches S1-S4 to light up LEDs DS1-DS4.

*******************************************************************/

#define DS1 4
#define DS2 5
#define DS3 6
#define DS4 7
#define S1  4
#define S2  5
#define S3  6
#define S4  7

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

   // Make Port A bit-wide output
   BitWrPortI(SPCR, &SPCRShadow, 1, 2);
   BitWrPortI(SPCR, &SPCRShadow, 0, 3);

}

main()
{
   int i;

   InitIO();

   while(1)
   {
	   for(i = S1; i <= S4; i++)
   	{
         // Bits for switches and LEDs correspond
   		if(BitRdPortI(PBDR, i))
	      {
			   BitWrPortI(PADR, &PADRShadow, 1, i);
      	}
	      else
         {
			   BitWrPortI(PADR, &PADRShadow, 0, i);
      	}
	   }
   }
}


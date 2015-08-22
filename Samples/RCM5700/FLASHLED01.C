/*******************************************************************
	Flashled01.c

 	Digi International, Copyright (C) 2008.  All rights reserved.

   This program is used with RCM57xx or RCM67xx series controllers
   with base boards.

   Description
   ===========
   This program uses Dynamic C's costatement feature to flash
   the DS1 LED on the base board. Flashled01a.c does the same
   thing using standard C constructs.

	I/O control       On Base-board
	--------------    ----------------------
	Port D bit 0		DS1, LED

   Jumper settings (Base-board)
   ----------------------------
   JP1   5-6

         2    4    6   8
         o    o    o   o
         |         |
         o    o    o   o
         1    3    5   7

   Instructions
   ============
   1.  Make sure jumper is connected as shown above.
   2.  Compile and run this program.
   3.  DS1 flashes on/off.

*******************************************************************/

main()
{
   // Initialize DS1 LED (PDO) to output
 	BitWrPortI(PDDDR, &PDDDRShadow, 1, 0);
   // Make sure PD0 not set to alternate function
 	BitWrPortI(PDFR,  &PDFRShadow, 0, 0);

	while (1)
	{
		costate
		{
 			WrPortI(PDDR, NULL, 0);
			waitfor(DelayMs(50));      // On for 50 ms.
      	WrPortI(PDDR, NULL, 1);
			waitfor(DelayMs(100));  	// Off for 100 ms.
		}
	}
}
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
/*******************************************************************

	Flashled02.c

   This program is used with RCM57xx or RCM67xx series controllers
   with base boards.

   Description
   ===========
   This program uses costatements to detect and debounce
   S1 being pressed. It flashes DS1 continuously and changes
   the flashing speed when S1 is pressed

   This program is functionally equivalent to Flashled02a.c, but
   uses Dynamic C's convenient costatements to implement the
   same simple state machine functionality.

	I/O control       On Base-board
	--------------    ----------------------
	Port D bit 0		DS1, LED
	Port D bit 1		S1, button switch

   Jumper settings (Base-board)
   ----------------------------
   JP1   5-6, 7-8

         2    4    6   8
         o    o    o   o
         |         |   |
         o    o    o   o
         1    3    5   7

   Instructions
   ============
   1.  Make sure jumpers are connected as shown above.
   2.  Compile and run this program.
   3.  Press S1 on the base board to change speed.

*******************************************************************/

#define DS1 0
#define S1  1

main()
{
   int slow;

   // Set DS1 (PD0) off
 	BitWrPortI(PDDR, &PDDRShadow,  0, DS1);
   // Initialize DS1 LED to output
 	BitWrPortI(PDDDR, &PDDDRShadow, 1, DS1);

   // Initialize S1 switch (PD1) to input
 	BitWrPortI(PDDDR, &PDDDRShadow, 0, S1);

   // Make sure PD0, PD1 not set to alternate function
 	BitWrPortI(PDFR,  &PDFRShadow,  0, DS1);
 	BitWrPortI(PDFR,  &PDFRShadow,  0, S1);

   slow = 0;

	while (1)
	{
		costate
		{
         waitfor(BitRdPortI(PDDR, S1)); // Wait for switch unpressed
			waitfor(DelayMs(50));		  	 // Unpressed switch detected
			if (!BitRdPortI(PDDR, S1))
			{
            slow = !slow;               // Switch press detected
			}
		}

      costate
      {
	   	BitWrPortI(PDDR, &PDDRShadow,  1, DS1);  // LED off
         waitfor(DelayMs(slow*200 + 50));
      	BitWrPortI(PDDR, &PDDRShadow,  0, DS1);  // LED on
         waitfor(DelayMs(slow*200 + 50));
      }
	}
}
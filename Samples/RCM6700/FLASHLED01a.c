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

	Flashled01a.c

   This program is used with RCM57xx or RCM67xx series controllers
   with base boards.

   Description
   ===========
   This program uses a state machine with standard C constructs
   to flash LED DS1 on the baseboard. Flashled01.c does the same
   thing using Dynamic C's costatement features.

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

#define STATE1 1
#define STATE2 2
#define STATE3 3
#define STATE4 4

main()
{
   unsigned long t0;
   int state;

   // Initialize DS1 LED (PDO) to output
 	BitWrPortI(PDDDR, &PDDDRShadow, 1, 0);
   // Make sure PD0 not set to alternate function
 	BitWrPortI(PDFR,  &PDFRShadow, 0, 0);

   state = STATE1;

	while (1)
	{
		switch(state)
      {
         case STATE1:
	 			WrPortI(PDDR, NULL, 0);     // LED on
            t0 = MS_TIMER;              // Intialize timer
            state = STATE2;             // Next state
            break;
         case STATE2:
				if (MS_TIMER - t0 > 50)     // Check timer
            {
            	 state = STATE3;         // Next state if expired
            }
            break;
         case STATE3:
	      	WrPortI(PDDR, NULL, 1);     // LED off
            t0 = MS_TIMER;              // Intialize timer
            state = STATE4;             // Next state
            break;
         case STATE4:
				if (MS_TIMER - t0 > 100)    // Check timer
            {
            	 state = STATE1;         // back to first state if expired
            }
            break;
         default:
		}
	}
}
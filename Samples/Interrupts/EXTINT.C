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

	EXTINT.C

	This program demonstrates how to use the external interrupt lines to
   trigger an interrupt service routine (ISR) in your code for the Rabbit
   4000-6000 CPU.

   Instructions:
   -------------
   1. Verify that interrupt lines PE0 & PE1 are NOT floating, if they are
   then you will need to terminate the interrupt lines with a pull-up or
   pull-down resistor ( >10k ).

   2. To demonstrate the interrupt(s), drive the interrupt line(s) with
   a signal source to generate pulses to be counted by the interrupt
   routine for display. The program will count a minimum of 20 pulses
   on each interrupt line before exiting.

	Notes:
   -----
   1. Signal source can be as simple as a jumper wire connected to GND.
   2. The "interrupt" keyword is used to create an ISR in C code.

**************************************************************************/
#class auto

char count0, count1;

void my_isr0();
void my_isr1();

void main()
{
	count0 = 0;
	count1 = 0;

	WrPortI(PEDDR, &PEDDRShadow, 0x00);	// set port E as all inputs

	SetVectExtern(0, my_isr0);
	SetVectExtern(1, my_isr1);
   // re-setup ISR's to show example of retrieving ISR address using GetVectExtern
	SetVectExtern(0, GetVectExtern(0));
	SetVectExtern(1, GetVectExtern(1));

	WrPortI(I0CR, &I0CRShadow, 0x09);		// enable external INT0 on PE0, rising edge, priority 1
	WrPortI(I1CR, &I1CRShadow, 0x09);		// enable external INT1 on PE1, rising edge, priority 1


	while ((count0 < 20) && (count1 < 20)) {

		// output the interrupt count every second
		costate {
			printf("counts = %3d  %3d\n", count0, count1);
			waitfor(DelaySec(1));
		}
	}

	WrPortI(I0CR, &I0CRShadow, 0x00);				// disable external interrupt 0
	WrPortI(I1CR, &I1CRShadow, 0x00);				// disable external interrupt 1

	// final counts
	printf("counts = %3d  %3d\n", count0, count1);
}


/****** interrupt routines  ******/

nodebug root interrupt void my_isr0()
{
	count0++;
}

nodebug root interrupt void my_isr1()
{
	count1++;
}
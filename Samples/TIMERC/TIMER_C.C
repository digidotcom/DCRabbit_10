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
/**********************************************************************\
	timer_c.c

   This example is for the Rabbit 4000 processor to demonstrate the
   new Timer C.

   Timer C is clocked by pclk/2 or pclk/16 or the output of Timer A1.
   The counter counts from zero to the value in the divider registers
   and then returns to zero.

   There are four outputs from Timer C. They are controlled by set and
   reset registers.  When the Timer C value equals the value in a set
   register, the output is set to one.  When the value reaches the
   value in the reset register, the output returns to 0.

   This sample outputs four signals on parallel port E pins 0-3. The
   signals will have the same frequency but different cascading start
   times. The Rabbit 4000 can output the Timer C signals on parallel
   ports C and D pins 0-3 as well.
              __          __          __
   Timer C0: |  |__ __ __|  |__ __ __|  |__ __ __ ...
                 __          __          __
   Timer C1:  __|  |__ __ __|  |__ __ __|  |__ __ ...
                    __          __          __
   Timer C2:  __ __|  |__ __ __|  |__ __ __|  |__ ...
                       __          __          __
   Timer C3:  __ __ __|  |__ __ __|  |__ __ __|  |...

   Instructions:
		1: Choose an output frequency by changing the FREQ value
      2: Hook up an oscilloscope to PE0, 1, 2 and/or 3
      3: Compile and run the program.  Observe the output.

   Alternatively, hook up some LED's to PE0-3 and lower the FREQ
   value (< 5) so that the flashing LED's are visible.

   See also: Samples\Rabbit4000\DMA\DMA_TIMERC.C

\**********************************************************************/

// The Frequency in Hz
#define FREQ 1000000ul // 1MHz

const unsigned int base_reg[4] = { TCS0LR, TCS1LR, TCS2LR, TCS3LR };

// This function writes to the channel specific set and reset registers.
void timerc_setup(int channel, unsigned int set, unsigned int res) {
   WrPortI(base_reg[channel] + (TCS0LR-TCS0LR), NULL, (char) set);
   WrPortI(base_reg[channel] + (TCS0HR-TCS0LR), NULL, (char) (set >> 8));
   WrPortI(base_reg[channel] + (TCR0LR-TCS0LR), NULL, (char) res);
   WrPortI(base_reg[channel] + (TCR0HR-TCS0LR), NULL, (char) (res >> 8));
}

int main()
{
	unsigned long div;
   unsigned int req_ta1, actual_ta1;

   // The baud clock is 16 counts per bit and freq_divider corresponds
   // to a baud rate of 19200.
   div = (unsigned long)((19200ul*16ul*(unsigned long)freq_divider)/FREQ);

   // If FREQ is too low, we need to use timer A1 to compensate.
   req_ta1 = 0;
  	while(div / (1L + req_ta1) > 0xFFFF) {
   	req_ta1++;
   }

   // Clock by pclk/2 with no interrupts
	WrPortI(TCCR, &TCCRShadow, 0x00);
   if (req_ta1 > 0) {
      // set up timer A1, TAT1R_SetValue will also set TCCR to use Timer A1
      while (1)
      {
      	actual_ta1 = TAT1R_SetValue( TAT1R_CTIMER_REQ, req_ta1);
      	if (actual_ta1 == req_ta1)
      	{
      		break;
      	}
      	req_ta1 = actual_ta1;
      }
		div /= (1L + actual_ta1);
   }

	// Set up the divide registers
	WrPortI(TCDLR, NULL, (char)(div));
   WrPortI(TCDHR, NULL, (char)(div >> 8));

   // Port E Setup:
	WrPortI(PEDDR, &PEDDRShadow, PEDDRShadow | 0x0F);
   WrPortI(PEFR,  &PEFRShadow, PEFRShadow | 0x0F);
   WrPortI(PEALR, &PEALRShadow, 0xAA);

   // Channel setup:
   timerc_setup(0, (int)(div * 0.00), (int)(div * 0.25));
   timerc_setup(1, (int)(div * 0.25), (int)(div * 0.50));
   timerc_setup(2, (int)(div * 0.50), (int)(div * 0.75));
   timerc_setup(3, (int)(div * 0.75), (int)(div * 0.00));

   // Start Timer C
   WrPortI(TCCSR, &TCCSRShadow, 0x01);

   while(1);
}


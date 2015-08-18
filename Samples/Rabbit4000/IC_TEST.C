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
/**********************************************************
	ic_test.c

	This program is used with Rabbit 4000 series controllers
	with RCM4xxx prototyping boards.

	Description
	===========
	This program demonstrates a simple application of input
	capture peripheral.  It measures the duration of a low pulse
	on PC5 and displays to STDIO.  Port C bit 5 is channel 1 pulse
	capture pin.

	Note:  Interrupts are not used in the demo.

	Instructions
	============
	1. Hook up a pulled-up switch to PC5.

		If using a RCM4xxx prototyping board, jumper J2 as follows:
			PB5 --- PC5

	2. Compile and run this program.
	3. Press and release switch S3.
	4. Observe the amount of time you hold down the switch in STDIO.

*************************************************************************/
#class auto

#define VERBOSE_ROLLOVER 0	// nonzero to report input capture rollover
#if VERBOSE_ROLLOVER
	#warnt "Reporting rollover increases chances of losing counts at fast rates."
#endif	// VERBOSE_ROLLOVER

void main(void)
{
	char capture_status;
	unsigned multiplier;
	unsigned long pulse_width;

	// Set PC5 as an input.
	WrPortI(PCDDR, &PCDDRShadow, PCDDRShadow & 0xDF);

	// PC5 is pulse capture pin for channel 1
	// Available pins are Ports C, D and E pins 1,3,5 and 7
	WrPortI(ICS1R, NULL, 0x22);

	// use freq_divider as basis for input capture prescaler
	//  (freq_divider-1) runs at 19200*16Hz; for better range multiply by the
	//  largest value that keeps the result within the 8-bit unsigned range
	for (multiplier = 1; 256u >= freq_divider*(1+multiplier); ++multiplier);
	WrPortI(TAT8R, NULL, (freq_divider*multiplier)-1);	// (TA8 prescaler)
	WrPortI(ICCSR, NULL, 0x0C);	// zero out counters
	WrPortI(ICCR, NULL, 0x00);		// normal operation; no interrupts

	// run counter start to stop
	// start is falling edge, stop is rising edge
	// latch counter on stop
	WrPortI(ICT1R, NULL, 0x59);

	pulse_width = 0;

	while(1)
	{
		// listen for capture states
		capture_status = RdPortI(ICCSR);
		if(capture_status & 0x10)
		{
			// channel 1 stop occurred
			// read capture value, LSB first
			pulse_width += RdPortI(ICL1R);
			pulse_width += RdPortI(ICM1R) * 256L;

			// 307.2 = 19200 * 16 / 1000 (see TA8 setting above)
			printf("channel 1 pulse: %.3fms\n", (float)(pulse_width * multiplier) / 307.2);
			if (RdPortI(ICCSR) & 0x20) {
				// already seen a start condition, so resynchronize input capture
				//  with the first pulse following completion of the above printf
				while (!(RdPortI(ICCSR) & 0x10));	// wait for next stop condition
			}
			WrPortI(ICCSR, NULL, 0x04);	// zero out counter
			pulse_width = 0;
		}
		else if (capture_status & 0x04) {
#if VERBOSE_ROLLOVER
			printf("Input capture counter rolled over.\n");
#endif	// VERBOSE_ROLLOVER
			pulse_width += 0x10000L;
		}
	}
}


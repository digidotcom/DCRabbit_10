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
/******************************************************************************
	timer_b.c

	This example program produces a pulse on port A every time timer B is
	zero (the ISR reloads match register B1 with zero each time it is
	called).  This produces a pulse train with a frequency of 7200 Hz with
	a 14MHz CPU clock.  Since the pulse is produced when the ISR is called,
	the timing between pulses is NOT precise!

 ******************************************************************************

	Timer B is a 10-bit counter that can be clocked with perclk/2, perclk/16,
	or with the output of timer A1.  perclk is the peripheral clock, which
	is usually set to the main oscillator frequency (but may be set to that
	frequency divided by eight if the CPU is also using that clock/8).

	For a 14MHz CPU clock, the timer B rollover times are as follows
	(timer A1 defaults to being clocked with perclk/2):

		Timer B Input	Timer B resol.		Rolls over every	Rollover freq.
		-------------	--------------		----------------	--------------
		  perclk/2		0.135 microsec		138.88 microsec		7200 Hz
		  perclk/16		1.085 microsec		  1.11 millisec		 900 Hz

	Keep these things in mind when using timer B:

		1.  The A/D and D/A channels on the Jackrabbit use timer B -- you
			 cannot use timer B if you plan on doing any analog/digital
			 conversion!

		2.  If you plan on using timer A1 as timer B input, be warned that
			 the serial port buad rates are based on A1 and will need to be
			 modified as well.

	Timer B contains two match registers, B1 and B2.  These can be programmed
	to produce an interrupt when timer B matches the register and/or to
	update output values on ports D and E.

	Watch out for the timer B match registers -- there are two bytes for
	each register, and the bits of the upper byte are not in the
	"expected" locations:

				   Match register MSB		Match register LSB
			  |------------------------|------------------------|
		Bit: | 9  8  x  x  x  x  x  x	| 7  6  5  4  3  2  1  0 |
			  |------------------------|------------------------|
			    ^  ^

 ******************************************************************************
	Uses:

	Because of the high rollover frequency, timer B is well-suited for
	producing a pulse (or updating an output) of precise length, or at
	a precise time in the future (but within the rollover time).  It
	is not so well-suited for producing a train of pulses at a particular
	frequency since (a) the pulse frequency must be greater than the
	rollover frequency for the match registers to work, and (b) the next
	timer B match must be recalcuated each time (but see below).

	If a simple fraction of the rollover frequency is acceptable, then
	timer B can be used.  For example, a short pulse at 7200 Hz can be
	produced by an ISR that always reloads the match register with zero, so
	that the interrupt is always produced when timer B is zero.  Similarly,
	a more complicated ISR could set the match register to zero the first time,
	512 the second time, zero the third time, 512 the fourth time, and so on
	to produce a pulse train with a frequency of 2*7200 = 14.4 kHz.

	Pulse-width modulation is another good use for timer B.  If one of the
	two rollover frequencies above is acceptable for your application, then
	a pulse can always be turned on when timer B is zero, and turned off at
	a specific match value that produces the desired duty cycle (0-100%), i.e.

							match = duty cycle * 1024

	(in reality, the time required for an ISR to load a new match register
	will prevent the very lowest and highest duty cycles from working).

******************************************************************************/
#class auto

int	count;
void  timerb_isr();

void main()
{
	count = 0;

	WrPortI(SPCR, &SPCRShadow, 0x84);	// enable port A for outputs
	SetVectIntern(0x0B, timerb_isr);	   		// set up ISR
	SetVectIntern(0x0B, GetVectIntern(0xB));	// re-setup ISR to show example of retrieving ISR address

	WrPortI(TBCR, &TBCRShadow, 0x01);	// clock timer B with (perclk/2) and
													//     set interrupt level to 1

	WrPortI(TBM1R, NULL, 0x00);			// set initial match!
	WrPortI(TBL1R, NULL, 0x00);

	WrPortI(TBCSR, &TBCSRShadow, 0x03);	// enable timer B and B1 match interrupts

	while (count < 30000) {

		printf("count = %d\n", count);

	}

	WrPortI(TBCSR, &TBCSRShadow, 0x00);	// disable timer B and its interrupts
}


////////////////////////////////////////////////////////////////////////////////
// interrupt routine for timer B
//
//  This is called whenever _either_ B1 or B2 matches the timer.  If you are
//  using both of them, you need to check the TBCSR register to see which one
//  triggered the interrupt (you need to read that register anyway to clear
//  the flag).
//
////////////////////////////////////////////////////////////////////////////////

#asm
timerb_isr::
	push	af							; save registers
	push	hl

	ioi	ld a, (TBCSR)			; load B1, B2 interrupt flags (clears flag); this
										; should be done as soon as possible in the ISR

	ld		hl, (count)
	inc	hl							; increment counter
	ld		(count), hl

	ld		hl, PADR
	ioi	ld (hl), 0xFF			; send a pulse to port A
	ioi	ld (hl), 0x00

	ld		a, 00h
	ioi	ld (TBM1R), a			; set up next B1 match (at timer=0000h)
	ioi	ld (TBL1R), a			; NOTE:  you _need_ to reload the match
										;	register after every interrupt!

done:
	pop	hl							; restore registers
	pop	af

	ipres								; restore interrupts
	ret								; return
#endasm
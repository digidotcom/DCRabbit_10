/******************************************************************************
	timer_b2.c
	Z-World, 2000

	This example program produces a precicely-timed pulse of duration 34.7
	microseconds (rollover/4) on port E, bit 7 every 138.88 microseconds
 	(timer B rollover) with a 14MHz CPU clock -- other boards will
 	produce different times depending on the CPU frequency.  The port E bit is
 	updated when the match occurs, not when the ISR is called, which produces
 	the precise timing.  The ISR uses the lowest bit of the count variable
 	to determine whether to turn the pulse on or off.

	Note that if the ISR call is delayed (typically due to another interrupt
	handler such as the serial ISR), two timer B matches will go by before
	the ISR is called (try running this demo with and without polling).
	This will produce an incorrect-width pulse, however the ISR below will
	realign the match and pulse values in the next cycle.

	NOTE: if your board is using parallel port E for another purpose (for
	example, the OP6800), then running this sample may cause unusual results
	(odd behavior, program crash, etc.).

******************************************************************************/
#class auto

int	count;			// keep track of times ISR is called

void  timerb_isr();

void main()
{
	count = 0;

	WrPortI(SPCR, NULL, 0x84);						// enable port A as outputs

	// make sure port E is standard mode
	WrPortI(PEFR, &PEFRShadow, PEFRShadow & 0x7F);
	// set port E bit 7 as output
	WrPortI(PEDDR, &PEDDRShadow, PEDDRShadow | 0x80);

	// set upper nibble of port E to clock out on B1 match!
	WrPortI(PECR, &PECRShadow, 0x20);

	// Set up vector to ISR
	SetVectIntern(0x0B, timerb_isr);
	// Re-setup ISR to show example of retrieving ISR address
	SetVectIntern(0x0B, GetVectIntern(0xB));

	WrPortI(TBCR, &TBCRShadow, 0x01);	// clock timer B with (perclk/2) and
													//     set interrupt level to 1

	WrPortI(TBM1R, NULL, 0x00);
	WrPortI(TBL1R, NULL, 0x00);			// set initial match

	WrPortI(PEB7R, NULL, 0x80);			// set initial output value


	WrPortI(TBCSR, &TBCSRShadow, 0x03);	// enable timer B and B1 match interrupts

	while (count < 30000) {

		printf("count = %d\n", count);

	}

	WrPortI(TBCSR, &TBCSRShadow, 0x00);	// disable timer B and its interrupt
}


/******************************************************************************
	interrupt routine for timer B

	This is called whenever _either_ B1 or B2 matches the timer.  If you are
	using both of them, you need to check the TBCSR register to see which one
	triggered the interrupt (you need to read that register anyway to clear
	the flag).
 ******************************************************************************/

#asm
timerb_isr::
	push	af							; save registers
	push	hl

	ioi	ld a, (TBCSR)			; load B1, B2 interrupt flags (clears flag)

	ld		hl, (count)
	inc	hl							; increment counter
	ld		(count), hl

	ld		a, 01h
	and	l							; mask off all but lowest bit of counter
	jr		z, match_0100
match_0000:
	ld		a, 40h
	ioi	ld (TBM1R), a			; set up next B1 match (at timer=0100h)
	ld		a, 00h					; 01h bit-mangled to for TBM1R layout
	ioi	ld (TBL1R), a			; NOTE:  you _need_ to reload the match
										;	register after every interrupt!
	ld		a, 80h
	ioi	ld (PEDR), a			; load output value for the next match
										;		(NOT the one loaded above!)
	jr		done
match_0100:
	ld		a, 00h
	ioi	ld (TBM1R), a			; set up next B1 match (at timer=0000h)
	ioi	ld (TBL1R), a			; NOTE:  you _need_ to reload the match
										;	register after every interrupt!
	ld		a, 00h
	ioi	ld (PEDR), a			; load output value for the next match
										;		(NOT the one loaded above!)

done:
	pop	hl							; restore registers
	pop	af

	ipres								; restore interrupts
	ret								; return
#endasm

/******************************************************************************/
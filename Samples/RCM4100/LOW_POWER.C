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
/*****************************************************

   Samples\RCM4100\Low_Power.c

	Please Note:
	This program will lose communication with Dynamic C once it begins
	running. Dynamic C will display a target communication error message.
	Dynamic C cannot continue debugging once the target starts running
	off the 32 kHz oscillator. To prevent this, run without polling (Alt-F9).

	This sample program shows how to implement a function in RAM in order
	to reduce power consumption by the Rabbit microprocessor.  There are
	four features which create the lowest possible power drain by the
	processor:
	1) Run the CPU from the 32 KHz crystal
	2) Turn off the high frequency crystal oscillator
	3) Run from RAM
	4) Insure that internal I/O instructions do not use CS0

	This sample will show LEDs toggling on the RCM4100 prototyping
   boards. If you don't have LEDs hooked to	PB2-PB3, you can
 	use a scope. PB2 will blink the fastest.  After switching to
   low-power, all the LEDs blink together.

*******************************************************************/
#class auto

// local function prototypes
void delay_ms(unsigned long delay_mS);

/*******************************************************************
This function must be in root in order for it to be copied to RAM.
Due to the granularity of the LXPC (12K byte window), this function
cannot be larger than 4K bytes or cross a 4K byte boundary.
*******************************************************************/
#asm root
RamFunction::
			ipset	3					; disable interrupts

; These two statements cause CS0 to NOT get activated when doing
; internal I/O operations thereby further reducing power by insuring
; that the flash is never accessed. They CANNOT be executed from flash.
			ld		a, 0xC5			; use CS1 for
	ioi	ld		(MB0CR), a		;  memory bank 0

; These instructions are loops which shows that the Rabbit is executing
; the function. The first 4 bits of Port A are toggled at varying speeds.
			ld    c,10
Loop0:
			ld 	b,15
Loop1:
			ld    d,255

Loop2:	; time delay loop
			dec   d
			jr    nz,Loop2

			ld    a,b
			xor   0xff
#if _BOARD_TYPE_==RCM4110
	ioi	ld		(PBDR), a
#else
	ioi	ld		(PADR), a	; write b to port A
#endif
			djnz	Loop1

			dec   c
			jr    nz,Loop0

			; done looping, now restore CSO
			ld		a, MB0CR_SETTING			; use CS0 for
	ioi	ld		(MB0CR), a		;  memory bank 0

	ipres		; restore the interrupt priority level
	ret
RamFunctionEnd::
#endasm

#define DS2 2
#define DS3 3

#define ON  0		//state to turn on led
#define OFF 1		//state to turn off led
///////////////////////////////////////////////////////////
// Initializes the two pins used for DS2 and DS3
// See brdInit() for other port initializing
///////////////////////////////////////////////////////////
void initPort()
{
#asm
	ioi	ld	a, (PBDR)
	res	DS2, a
	ioi	ld (PBDR), a
	ioi	ld	a, (PBDDR)
	set	DS2, a
	ioi	ld (PBDDR), a

	ioi	ld	a, (PBDR)
	res	DS3, a
	ioi	ld (PBDR), a
	ioi	ld	a, (PBDDR)
	set	DS3, a
	ioi	ld (PBDDR), a
#endasm
}

///////////////////////////////////////////////////////////
// DS2 led on protoboard is controlled by port B2
// turns on if state = 0
// turns off if state = 1
///////////////////////////////////////////////////////////
void DS2led(int state)
{
	if (state == ON)
	{
#asm
	ioi	ld	a, (PBDR)		;read port to keep other bit values
	res	DS2, a				;clear bit
	ioi	ld (PBDR), a		;write data to port
#endasm
	}
	else
	{
#asm
	ioi	ld	a, (PBDR)			;read port to keep other bit values
	set	DS2, a					;set bit
	ioi	ld (PBDR), a			;write data to port
#endasm
	}
}

root main()
{
	auto unsigned long physaddr;			// Physical memory address to write to
	auto unsigned long FunctionSize;		//	Number of bytes in the Low Power function
	static unsigned int XPCvalue;			// LXPC value where function is copied to RAM
	static unsigned int Offset;			// Offset in LXPC area where function is copied

	initPort(); //WrPortI(SPCR, NULL, 0x84);		// make port A all outputs

	// Allocate space in RAM for the RAM function and copy it to RAM
	FunctionSize = (long)RamFunctionEnd - (long)RamFunction;

	// Using xalloc ensures that unused RAM is allocated
	physaddr = xalloc(FunctionSize);

	// Copy RamFunction to RAM
	root2xmem(physaddr, (void *)RamFunction, (int)FunctionSize);

	// Calculate lxpc stuff
	XPCvalue = (unsigned int)((physaddr - 0xE000L) >> 12); // get lxpc value
	Offset = (unsigned int)(physaddr & 0xFFF) + 0xE000;    // and offset

   //necessary delay for packet transfer from target before calling
   //use32kHzOsc function.
   delay_ms(2);
	// main osc off, use 32 KHz oscillator, disable periodic interrupt
	// we now loose communications with Dynamic C - if the low power
	// function executes for any length of time
	use32kHzOsc();

	// Disable the watch dog timer
	Disable_HW_WDT();

#asm
			// call that routine in SRAM - THIS MUST BE EXECUTED FROM ROOT
			ld		hl, retadr			; put the return address
			push	hl						;	on the stack so that function can use RET
			ld		hl, (XPCvalue)		;  get calculated lxpc value
			ld		lxpc, hl				;  load it into the lxpc
			ld		hl, (Offset);		;  load offset into LXPC area
			jp 	(hl);					;  jump to the function
retadr:
#endasm

	// Main osc. on, use main oscillator, enable periodic interrupt
	useMainOsc();

	// This loop toggles all PA0-PA3 all at once to indicate that we
	//  have returned from the low power function.
	while (1)
	{
		costate
		{
			waitfor(DelayMs(300));
			DS2led(ON); //WrPortI ( PADR, NULL, 15 );
			waitfor(DelayMs(300));
			DS2led(OFF);WrPortI ( PADR, NULL, 0 );
		}
	}

}

void delay_ms(unsigned long delay)
{
	auto unsigned long time0;

	for (time0 = MS_TIMER; MS_TIMER - time0 < delay; ) ;
}
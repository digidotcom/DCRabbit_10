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

     Samples\Low_Power.c

	Please Note:
   This program will lose communication with Dynamic C once it begins
   running. Dynamic C will display a target communication error message.
   Dynamic C cannot continue debugging once the target starts running off
   the 32 kHz oscillator. To prevent this, compile to flash, and then run
   without polling (Alt-F9).

   This sample program shows how to implement a function in RAM in order
   to reduce power consumption by the Rabbit microprocessor.  There are
   four features which create the lowest possible power drain by the
   processor:

	1) Run the CPU from the 32 KHz crystal
	2) Turn off the high frequency crystal oscillator
	3) Run from RAM
	4) Insure that internal I/O instructions do not use CS0

   While in low-power mode, the program will blink LEDs on the RabbitCore
   Prototyping Board or the Minicore Digital I/O Accessory Board (no LEDs
   will blink on the BL4S100 or BLxS200 family of single-board computers).
   If your core is not connected to either the RabbitCore Prototyping
   Board or the Minicore Digital I/O Accessory Board, hook up LEDs to
   PB2-PB3 (non-Minicore, non-SBC) or PA4-PA7 (Minicore), or use a scope.
   (Note that the RabbitCore Prototyping Board only has LEDs attached to
   PB2 and PB3, and the Minicore Digital I/O Accessory Board has LEDs on
   PA4-PA7, and no visual feedback will be given for the SBCs.)

   After switching back to regular power, the LEDs will blink in unison.

*******************************************************************/
#class auto

// local function prototypes
void delay_ms(unsigned long delay_mS);

/*******************************************************************
This function must be in root in order for it to be copied to RAM.
Due to the granularity of the XPC (8K byte window), this function
cannot be larger than 4K bytes. Moreover, you may only use RELATIVE jumps
in this code.
*******************************************************************/
#if (RCM5600W_SERIES || RCM5700_SERIES)
	// Minicore Digital I/O Accessory Board
	#define PxDR PADR
	#define PxDDR SPCR      // PA direction controlled by SPCR
   #define LED_MASK 0x84   // Sets PA0-7 to outputs
#elif (BL4S100_SERIES || BLXS200_SERIES)
	#define PxDR PBDR
	#define PxDDR PBDDR
   #define LED_MASK 0x00   // No LEDs on the SBCs
#else
   // RabbitCore Prototyping Board
	#define PxDR PBDR
	#define PxDDR PBDDR
   #define LED_MASK (RdPortI(PBDDR) | 0x0C)   // LEDs on PB2 and PB3
#endif

#asm root
RamFunctionStart::
; These two statements cause CS0 to NOT get activated when doing
; internal I/O operations thereby further reducing power by insuring
; that the flash is never accessed. They CANNOT be executed from flash.
	ioi	ld		a, (MB0CR)		; Get MB0CR setting
         ld		e, a				; Save it for later.
			ld		a, 0xC5			; use CS1 for
	ioi	ld		(MB0CR), a		;  memory bank 0

; The following loops show that the Rabbit is executing the function. The
; pins controlling the LEDs are toggled at varying speeds.
			ld    c,10        ; loop through 10 times
Loop0:
			ld 	b,15        ; set the bit pattern (start with all 1's)
Loop1:
			ld    d,255       ; delay counter

Loop2:	; time delay loop
			dec   d
			jr    nz, Loop2

			ld    a,b
			xor   0xff
#if (RCM5600W_SERIES || RCM5700_SERIES)
	// Minicore Digital I/O Accessory Board: a = b << 4
			sla	a;
			sla	a;
			sla	a;
			sla	a;
#endif
	ioi	ld		(PxDR), a	; write bit pattern from b to port
			djnz	Loop1       ; decrement b and jump if nz

			dec   c
			jr    nz, Loop0

			; done looping, now restore CSO
			ld		a, e				; remember previous MB0CR setting
	ioi	ld		(MB0CR), a		;  memory bank 0

	llret
RamFunctionEnd::
#endasm

#define RAM_FUNCTION_SIZE 0x80
far char RamFunction[RAM_FUNCTION_SIZE];

#define XMEM_SEGMENT(x) -0xE000+(x) >> 12
#define XMEM_OFFSET(x) 0xE000 + ((x) & 0xFFF)

main()
{
   auto unsigned int actual_size;

	WrPortI(PxDDR, NULL, LED_MASK);		// make all outputs

   actual_size = (unsigned int)RamFunctionEnd - (unsigned int)RamFunctionStart;

   if (actual_size > RAM_FUNCTION_SIZE) {
   	printf("The ram function is %d bytes too large. Increase its size and recompile.\n",
             actual_size - RAM_FUNCTION_SIZE);
      exit(1);
   }

	_f_memcpy(RamFunction, RamFunctionStart, actual_size);

   //necessary delay for packet transfer from target before calling
   //use32kHzOsc function.
   delay_ms(2);
	// main osc off, use 32 KHz oscillator, disable periodic interrupt
	// we now loose communications with Dynamic C - if the low power
	// function executes for any length of time
	use32kHzOsc();

	// Disable the watch dog timer
	Disable_HW_WDT();

   // Call the RamFunction.  Note that when calling code in far data,
   // llcall should always be used (not lcall), since the segment may be
   // larger than 0xff.  The called code must also use llret to return.
   // Additionally, all interrupts should be disabled to prevent an lcall
   // from clobbering the lxpc.
   #asm
      ipset	3	; disable interrupts
      #pragma nowarn  // Suppress warning about calling a non-label
   	llcall XMEM_SEGMENT(RamFunction), XMEM_OFFSET(RamFunction)
      ipres		; restore the interrupt priority level
   #endasm

	// Main osc. on, use main oscillator, enable periodic interrupt
	useMainOsc();

   // This loop blinks the LEDs all at once to indicate that we have
   // returned from the low power function.
	while (1)
	{
		costate
		{
			waitfor(DelayMs(300));
			WrPortI ( PxDR, NULL, 0xFF );
			waitfor(DelayMs(300));
			WrPortI ( PxDR, NULL, 0 );
		}
	}

}

void delay_ms(unsigned long delay)
{
	auto unsigned long time0;

	for (time0 = MS_TIMER; MS_TIMER - time0 < delay; ) ;
}
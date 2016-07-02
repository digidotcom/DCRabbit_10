/*
   Copyright (c) 2016 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
// The cold loader expects that the following settings were tripleted in
// during bootstap:
// - Memory setup correct including MBxCR registers, SEGSIZE, STACKSEG, and
//   DATASEG
// - Watchdog disabled (0x80 0x09 0x51, 0x80 0x09 0x54)
// - Set serial port A async, 8 bit, pport C input (SACR = 0, 0x80 0xc4 0x00)
// - Enable timer A with cpuclk/2 (TACSR = 1, 0x80 0xa0 0x01)
// Note that any IO register settings that can be tripleted in during bootstrap
// rather than being setup while the coldloader will result in a smaller
// coldloader and less bytes to transfer.  For instance, consider loading 0
// into SACR.  If done during bootstrap, this is a 3-byte operation
// (0x80 0xc4 0x00) whereas if done in code:
// xor a
// ioi ld (SACR),a
// it is a 15 byte operation since the code requires 5 bytes, and each byte
// requires two bytes of address to tell the processor where to place the code
// in memory.

// This source file should be used in conjunction with makecold.exe, which is
// built from makecold.cpp.  The makecold.cpp source file contains several
// options including the building a coldloader to work with either 8 or 16 bit
// memories.  Note that if compiling a coldloader for 16-bit RAM, this
// file must be compiled with 16-bit support enabled, and makecold.exe must also
// be recompiled for 16-bit support.

// After this program is compiled to a .bin file, check it in a hex editor to
// make sure that the compiler has not inserted any extraneous bytes of code.

// This file is used to build coldload.bin.  This file should only be
// compiled under version 7.20 or later of the compiler to ensure that
// the file is not zero padded.  When compiling to .bin file, you should
// turn pointer checking off, restrict watch expressions, and exclude
// the bios.

// Define the following macro to 1 if compiling the coldloader to run on a
// 16-bit RAM.  Note that makecold.exe must be compiled to match the setting
// of this macro.
//#define BOOTSTRAP_FROM_16BIT	1

// WARNING: This cold loader must not use any Rabbit 4000 specific instructions.
// A good heuristic is all 8-bit loads not involving 'a' and all 8 bit
// arithmetic intsructions are on the 7F page. The best thing to do is look at
// the list file and search for the 7F prefix. But besides that, be extra
// careful that you're not using a 4000 only instruction.

// The '@' symbol designates parameters for the triplet utility. Right now it
// is designed to take a starting address, assumes that the binary is contiguous
// and takes io register settings in sequence and prefixes them to the triplets
// The utility knows macro names used in Dynamic C.

//@ SEGSIZE			0xC1
//@ STACKSEG		0x80
//@ DATASEG			0x7F
//@ PEAHR			0x01
//@ PEFR				0x10
//@ MACR				0x20
//@ SACR				0x00
//@ TACSR			0x01

//@ start			0x1000
//@ MACR				0x00
//@ MB0CR			0x4D
//@ SEGSIZE			0xC6
//@ DATASEG			0x3A
//@ SPCR				0x80

//#define  FREQADRS					0x3F02	// frequency divisor address

// Make sure the assembler doesn't add nop's for forward referenced labels...
#define call
#define real_call(expr) db 0xCD $ dw expr

#ifdef __SEPARATE_INST_DATA__
#if __SEPARATE_INST_DATA__
#error "Turn off separate I&D space."
#endif
#endif

#define LIGHTS_ON \
  	ld		a, 0xFF $ \
   ioi	ld		(PBDDR), a $\
	xor	a $\
	ioi	ld		(PBDR), a

void _get_byte();

#asm
main::

	; The processor comes up in 8-bit mode, which presents an interesting
   ; situation when 16-bit memories are present.  Until 16-bit mode has been
   ; set, the processor will effectively execute even addresses twice and not
   ; execute odd addresses.  The following code will start up the 16-bit bus
   ; on CS1.

	ld		hl, 0x0021
   nop
   dec	hl
   dec	hl
   dec	hl
   dec	hl  ; hl = 0x1D (MACR address)
	; We want 0x20 loaded into a, so we take advantage of the 0x1e (ld e,n)
   ; opcode.  By executing 0x1e twice, 0x1e is loaded into the e register.
   ; Incrementing e twice results in 0x20 in e.
	ld		e,0x1e
   inc	e
   inc	e
   ; Load 0x20 into a
   ld		a,e
   ld		a,e
	; ioi prefix
   db		0xd3
   db		0xd3
   ld		(hl),a
   ; The prior instruction writes to IO, but this one to memory with
   ; a value of 0x20 which is
   ld		(hl),a            ; MACR = 0x20 - start 16-bit bus on CS1
   nop
   nop							; 24 bytes

   // I wonder if these can be moved to the IO
   ld		a,0x45
   ioi	ld (MB0CR),a

   // MB2CR is the only register used in a strange way to load the cold loader.
   ld		a,0x40
   ioi	ld (MB2CR),a

   ld		a,0x40
   ioi	ld (MB3CR),a

   ld    sp, coldloadend+0x200 ; set up stack in low root segment

	ld    bc,0000h        		; our counter
	ld    de,07FFh        		; mask for RTC bits

; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; !!!!!WARNING: Time critical code for crystal frequency       !
; !!!!!detection begins here. Adding or removing code from the !
; !!!!!following loops will affect the frequency computation.  !
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
wait_for_zero:
	ioi ld    (RTC0R),a     	; fill RTC registers
	ioi ld    hl,(RTC0R)			; get lowest two RTC regs
	and   hl,de						; mask off bits
	jr    nz,wait_for_zero		; wait until bits 0-9 are zero

timing_loop:
	inc   bc							; increment counter
	push  bc 						; save counter

   ld 	b, 98h
	ld    hl, PCFR

delay_loop:
	ioi ld    (hl), 0xC0			; Set PCFR for for txa
	djnz  delay_loop

	pop bc 							; restore counter
	ioi ld    (RTC0R),a			; fill RTC registers
	ioi ld    hl,(RTC0R)			; get lowest two RTC regs
	bit   2,h						; test bit 10
	jr    z, timing_loop			; repeat until bit set

; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; !!!!!Time critical code for crystal frequency detection ends	!
; !!!!!here!                                                   !
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	;ld    h,b
   db		0x60
	;ld    l,c
   db		0x69
	ld    de, 0x0008
	add   hl, de			; add 8 (equiv. to rounding up later)

	rr    hl
	rr    hl
	rr    hl
	rr    hl					; divide by 16
	ld    a, l				; this is our divider!

	dec   a
ioi	ld	(TAT4R), a		; set timer A4 running at 57600 baud

	real_call(_get_byte)
	ld		c, a				; pilot BIOS's size LSB

	real_call(_get_byte)
	ld		b, a				; pilot BIOS's size MSB

   ld		hl, 0x6000

_load_pilot_loop:
	real_call(_get_byte)
	ld		(hl), a
	inc	hl							; increment the copy-to-RAM index
   dwjnz _load_pilot_loop

	jp		0x6000			   	; start running pilot bios

_get_byte::
pollrxbuf:
	ioi	ld    a,(SASR)			; check byte receive status
	bit   7,a
	jr    z, pollrxbuf 			; wait until byte received
	ioi 	ld    a,(SADR)			; get byte
	ret

coldloadend::
	; The coldloader triplet utility looks for this sequence to trim any
   ; constants the compiler emits automatically from the binary.
	dw 0x7676
	dw 0x7676
#endasm


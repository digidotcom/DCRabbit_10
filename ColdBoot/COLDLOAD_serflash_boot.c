/*
   Copyright (c) 2016 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	Rabbit 5000 and new processors can boot directly off of a serial flash
	without the assistance of a PIC (which was needed by the Rabbit 4000).
	Bootstrap mode is entered and bytes are read in sets of three from the
	serial flash starting from flash offset 0. These triplets have the same
	format as triplets read during bootstrap via dynamic C, namely [High
	byte, Low byte, Value]. This boot loader must be turned into triplets by
	the triplet utility BL_triplets.exe and then embedded at address 0 of
	the program (the start of the BIOS). Note that the boot loader will not
	execute as regular code, as it embeds a jump over itself (otherwise we
	could not safely place it at the start of the BIOS).

	The strategy here is to have this boot loader placed into high physical
	memory. This is done using the dataseg register, which will make the
	data segment start at address 0x4000 and then make this equate to
	physical address 0x14000 (an arbitrary value greater than 64k). The boot
	loader will then load 64k from the serial flash into internal RAM (this
	is why the boot loader must live above 64k, as it also resides in
	internal RAM). 64k is sufficient to allow the BIOS to run as it would
	for other serial boot flash boards.

	Note that processor execution always begins at address 0, so you must
	place a jump to 0x4000 at address 0. This is done automatically by the
	boot load triplet generator.

	The triplet generator will pad the end of the boot loader with 0x00, up
	to the length of the boot loader (for now it's 512 bytes). If the boot
	loader ever needs to be expanded in the future, this value can be easily
	changed in BL_triplets.cpp.

	Use COMPILE_coldload_serflash_boot.dcp to compile this .C to a .BIN.
	Use BL_triplets.exe to convert to the .LIB:

	BL_triplets.exe ..\COLDLOAD_serflash_boot
		..\..\DistribCommon\Lib\Rabbit4000\BIOSLIB\serial_flash_boot_loader.lib
*/

/*
	The odd notation that follows is used by BL_triplet.exe to build
	serial_flash_boot_loader.lib from COLDLOAD_serflash_boot.bin.
*/

//@ GCSR    0x08
//@ WDTTR   0x51
//@ WDTTR   0x54
//@ MB0CR   0xC3

// Set up PD4 (SPI TX), PD5 (SPI RX) and PD6 (Chip Select)
//@ PDDR    0x40
//@ PDDCR   0x00
//@ PDFR    0x10
//@ PDDDR   0x50

// Configure the clock
//@ TAT5R   0x00
//@ TACSR   0x01

// Configure serial port B for SPI
// Note that Rabbit 5000 needs SxER before SxCR; 6000 works with either ordering
//@ SBER    0x18
//@ SBCR    0x1C

// Set up PB0 (SPI Clock) as an output
//@ PBDDR   0x01

//@ SEGSIZE 0xD4
//@ DATASEG 0x10
//
//@ start   0x4000

// Note that since this is a special bootloader, we don't end with setting
// register SPCR to 0x80.  The BL_triplets.exe program adds that in.

#define SBF_DR          SBDR
#define _AR_OFFS        1
#define _SR_OFFS        3
#define _CR_OFFS        4
#define SBF_SPI_CONTROL 0x1C
#define SPI_RXMASK      0x40
#define SPI_TXMASK      0x80

#asm
main::
		//since this code will run immediately after hardware reset,
		//interrupts will be off
		ld		a, 0x80		; it is unsafe to set MMIDR to 0x80 through I/O triplets
ioi	ld		(MMIDR), a
		ld		a, 0xC0		; make sure rabbit 4000 instructions are enabled
ioi	ld		(EDMR), a

//==============================================================
//send the continuous read command
.dropDataLoop:
ioi	ld		a, (SBF_DR)				; clear receive data buffer / FIFO
ioi	ld		a, (SBF_DR+_SR_OFFS)	; check status register, is more data waiting?
		and	0xAC						; test Rx full/overrun, Tx full/sending bits
		jr		nz, .dropDataLoop		; if TX busy or RX not empty go re-check . . .

		ld		hl, PDDR
ioi	res	6, (hl)					; reset PD6 to enable sbf cs
		ld		de, hl					; de is now PDDR

		ld		hl, SBF_DR + _SR_OFFS
;; The industry standard 0x03 continuous read command followed by three address
;; bytes will not work for boards equipped with Atmel serial flash devices with
;; revision levels earlier than "D" (i.e. revisions prior to AT45DB041D,
;; AT45DB081D, AT45DB161D, AT45DB321D, AT45DB642D). The 0x03 command is also
;; limited to 33 MHz operation, maximum. In contrast, the Atmel-specific
;; "legacy" 0xE8 continuous read command is supported for all AT45DBxxxx serial
;; flash device revisions to date, at up to 66 MHz operation, maximum.
		; load the sequence 0xE8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		ld		bc, 8						; command + 3 address + 4 don't care bytes
		ld		a, 0xE8					; command byte
.sbfStreamOutL:
ioi	ld		(SBF_DR), a				; send command, address or don't care byte
;; The Rabbit's SPI transmitter idle status bit is actually set 1/2 bit-time
;; early. To compensate for this, we start a simultaneous receive plus transmit
;; operation and wait for the receive operation to complete. We discard the
;; received byte before proceeding.
		ld		a, SBF_SPI_CONTROL | SPI_RXMASK | SPI_TXMASK
ioi	ld		(SBF_DR+_CR_OFFS), a	; start simultaneous receive+transmit operation
		align	2							; even-align the following ioi bit X, (hl)
.sbfWaitRxTxDone:
ioi	bit	7, (hl)					; check for Rx buffer not empty
		jr		z, .sbfWaitRxTxDone	; if Zero flag set, go wait some more . . .

ioi	ld		a, (SBF_DR)				; clear Rx buffer not empty flag (discard byte)
		xor	a							; remaining bytes are all 0x00
		dwjnz	.sbfStreamOutL

		; receive the program bytes, starting at physical address 0
		ld		px, 0
		; BC already has 0 from dwjnz loop, above
;		ld		bc, 0		; the following loop will run 64k times until bc = 0 again
		; don't forget, HL still has SxSR and DE is holding PDDR for later

.ReadLoop:
		ld		a, SBF_SPI_CONTROL | SPI_RXMASK
ioi	ld		(SBF_DR+_CR_OFFS), a	; start a byte receive operation
		; the following pre-increment shifts instruction cycles into idle time
		ld		px, px+1					; pre-increment the destination address
		align	2							; even-align the following ioi bit X, (hl)
.ReadWait:
ioi	bit	7, (hl)					; test receiver status bit
		jr		z, .ReadWait			; spin if no receive data yet

ioi	ld		a, (SBF_DR)				; get byte from SxDR
		; the following PX-1 offset is part of the loop efficiency optimization
		ld		(px-1), a				; store in RAM (decrement pre-incremented addr)
		; post-inc of PX moved to pre-inc, above, to maximize loop efficiency
		dwjnz	.ReadLoop

		ex		de, hl					; hl = PDDR, de = SBDR + _SR_OFFS
ioi	set	6, (hl)					; set PD6 to disable sbf cs
		; read is complete, jump to newly loaded code
		jp		0x0000

coldloadend::
		; The coldloader triplet utility looks for this sequence to trim any
		; constants the compiler emits automatically from the binary.
		db 0x76, 0x76, 0x76, 0x76
#endasm
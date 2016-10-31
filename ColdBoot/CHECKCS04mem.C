/*
   Copyright (c) 2016 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*****************************************************************************
  checkCS04mem.c

  This tripletized utility is needed by Dynamic C to determine if no memory
  (flash or RAM) is on CS0. It is needed to indentify products that use
  external RAM on CS1 and serial boot flash, as opposed to products that
  have RAM on CS0 and are therefore assumed to use serial boot flash.

  This code will only work on CS3, because it takes advantage of the fact
  that code running on internal RAM don't put anything on the data bus.
  Since internal RAM will work as 8-bit, there is no need for weird 8/16
  code as is used in checkRamCS0.bin and precoldload.bin.

  If a device is found on CS0, STATUS is raised

  Use the project file compile_coldload.dcp to compile this file.

  Assumptions before running this code:
     MECR  = 0x00
     MMIDR = 0x80
     MB0CR = 0x43
     MB1CR = 0x01
     MB2CR = 0x00
     MB3CR = 0x01

******************************************************************************/

// start signals the end of io register values and gives the starting offset
// to the triplet utility.

//@ start  0x0000
//@ SPCR     0x80

#ifdef __SEPARATE_INST_DATA__
#if __SEPARATE_INST_DATA__
#error "Turn off separate I&D space."
#endif
#endif

#asm
main::
		ld	 a,0xC0
ioi	ld  (EDMR),a      ; Use R4K instruction set

      nop
      nop

      ld  py,0x00040000 ; Use the base of quad. 1 to force values on the bus.
                        ;  DC maps those quadrants to CS1/0E0/WE0, an unused
                        ;  combination, to ensure that they aren't mapped to
                        ;  internal RAM and something gets on the bus without
                        ;  destroying an existing value in an actual device.

		ld  px,0x00080000	; Using base of quadrant 2, 0x00080000

		ld  (py),a		   ; 0xC0 on bus
 		ld  hl,(px)		   ; Read from bottom of quadrant 3
							;    If no device on CS, reads 0xC0C0 from bus
							;    If 16-bit ext. RAM, reads word L:L
							;    Else reads word @PX (0xFFFF for virgin flash)

		ld  de,hl		; Save read value
		inc hl
		ld  (px),hl		; write vlaue + 1 back, If 16-bit ext. RAM, writes L+1:L+1

		ld  a,0xff		; 0xff on bus
		ld  (py),a

		ld  hl,(px)		; Read again, if no device there, reads 0xFFFF
							;  If flash there, reads same value as 1rst read
							;  If RAM there, reads different value than 1rst read
							;  If 16-bit ext. RAM, reads L+1:L+1
							;  If virgin flash might read 0x0000

		sub hl,de		; RAM or virgin flash = non-zero, flash = 0,
							;     nothing = 0x3F3F

		ld  a,0x3F		; H = 0x3F means no device
		cp  h
		jr  z, noDevFound
		ld	 a,0x30     ; Raise  STATUS pin
ioi	ld  (GOCR),a
noDevFound:
		jr  noDevFound

		db 0x76, 0x76, 0x76, 0x76      ; End of program marker for tripletizer utility.
#endasm




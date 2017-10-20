/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
// A macro to be used in assembler code.  On entry, AHL contains
// linear 24-bit address.  Converts this to segmented form
// with LXPC value in JK, and logical part in HL (in range 0xE000-0xEFFF).
// 118 clocks
// high 8 bits of linear address put in bc
#define _LIN2SEG	\
	push bcde  $\
   ld   c,a   $\
	add  hl,hl $\
   rl   bc    $\
   add  hl,hl $\
   rl   bc    $\
   add  hl,hl $\
   rl   bc    $\
   add  hl,hl $\
   rl   bc    $\
   ld   a,c   $\
	sub  0x0E  $\
   ld   c,a   $\
   ld   a,b   $\
	sbc  a,0   $\
   and  0x0F  $\
   ld   b,a   $\
   ld   de,hl $\
   push bcde  $\
   pop  jkhl  $\
   or   a     $\
   rr   hl    $\
   scf        $\
   rr   hl    $\
   scf        $\
   rr   hl    $\
   scf        $\
   rr   hl    $\
   ld   a,c   $\
   pop  bcde

   // FIXX - last ld  a,c is temporary so _LIN2SEG returns seg in both A and JK

// Trashes flags and af'.
// _GEN_LIN2SEG allows you to normalize to a specific range  The value
// to normalize can also be put into registers b, c, d, or e
/*
#define _GEN_LIN2SEG(X) add hl,hl $ rla $ add hl,hl $ rla $ add hl,hl $ rla $\
	add hl,hl $ rla $ sub X $ ex af, af' $ ld a, X $ rra $ rr hl $ rra $ rr hl $\
	rra $ rr hl $ rra $ rr hl $ ex af, af'
*/
#define _GEN_LIN2SEG(X)	\
	push bcde   $\
   ld	  d,0    $\
   ld   e,a    $\
	add  hl,hl  $\
   rl   de     $\
   add  hl,hl  $\
   rl   de     $\
   add  hl,hl  $\
   rl   de     $\
   add  hl,hl  $\
   rl   de     $\
   ld   a,e    $\
	sub  X      $\
   ld   e,a    $\
   ld   a,d    $\
	sbc  a,0    $\
   ld   d,a    $\
   ex   jk',hl $\
   ld   hl,de  $\
   ex   jk,hl  $\
   ex   jk',hl $\
	ld	  a,X    $\
   rra         $\
   rr   hl     $\
   rra         $\
   rr   hl     $\
   rra         $\
   rr   hl     $\
   rra         $\
   rr   hl     $\
   ld   a,e    $\
   pop  bcde

// Assembler macro to call the _xmem_mvc (move chars) routine.  Expects A to contain
// the relevant XPC value; HL or DE contains a logical address in 0xE000-0xEFFF range,
// BC contains number of bytes to move (<=4k).  DE or HL contains an address in
// root data.  One and only one of HL or DE must point in the xmem window; the other
// must point to root data.  Data is moved from (HL) to (DE).
// On return, HL and DE set to their values on entry, plus byte count.  BC zero.
#define _XMEM_MVC	db 0xCF $ dw _xmem_mvc $ db 0

// These macros do the same as _XMEM_MVC, except the source or destination is an
// internal (ioi) or external (ioe) I/O address.  Note that the memory address always
// increments, but the I/O address stays fixed.
#define _XMEM_MVC_IOE2MEM	db 0xCF $ dw _xmem_mvc_ioe2mem $ db 0
#define _XMEM_MVC_IOI2MEM	db 0xCF $ dw _xmem_mvc_ioi2mem $ db 0
#define _XMEM_MVC_MEM2IOE	db 0xCF $ dw _xmem_mvc_mem2ioe $ db 0
#define _XMEM_MVC_MEM2IOI	db 0xCF $ dw _xmem_mvc_mem2ioi $ db 0

// Assembler macro to call the _xmem_clc (compare chars) routine.  Expects A to contain
// the relevant XPC value; HL or DE contains a logical address in 0xE000-0xEFFF range,
// BC contains number of bytes to compare (<=4k) in "inner djnz form" (see xmemcmp()
// function for details).  DE or HL contains an address in
// root data.  Data is compared between (HL) and (DE).
// On return, HL and DE point past fist mismatch char, or end of string.  BC is
// remaining char count in inner djnz form; zero iff exact match.
// A set to 0 if exact match, non-zero if not.  Cy flag set iff first mismatch byte
// at (DE) is less than (HL).  Cy flag clear if exact match, or (HL) < (DE).
// Comparison is in an unsigned sense.
#define _XMEM_CLC	db 0xCF $ dw _xmem_clc $ db 0

#define _IOELSIDR	db 0xDB,0xED,0xD0
#define _IOILSIDR	db 0xD3,0xED,0xD0
#define _IOELDISR	db 0xDB,0xED,0x90
#define _IOILDISR	db 0xD3,0xED,0x90


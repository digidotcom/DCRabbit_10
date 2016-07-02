// WARNING: This cold loader must not use any Rabbit 4000 specific
// mainpage instructions.
// A good heuristic is all 8-bit loads not involving 'a' and all 8 bit
// arithmetic intsructions are on the 7F page. The best thing to do is look at
// the list file and search for the 7F prefix. But besides that, be extra
// careful that you're not using a 4000 only instruction.

// The '@' symbol designates parameters for the triplet utility. Right now it
// is designed to take a starting address, assumes that the binary is contiguous
// and takes io register settings in sequence and prefixes them to the triplets
// The utility knows macro names used in Dynamic C.

//@ SEGSIZE  0xC6
//@ STACKSEG 0x74
//@ DATASEG  0x3A

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
		ld		b, 1				; If 16-bit, CPU will see 0x0606: ld b, 6
   	djnz	label          ; 8bit won't branch (--b == 0), else will.
   	ld		a, 0x30        ; Didn't branch, must be 8-bit
ioi	ld		(GOCR), a      ; Signal Dynamic C accordingly...
		jr		@pc            ; Loop forever
align 0x14
label:							; Must be at location 0x14 for djnz to
									; appear the same in either mode.
		ld		hl, 0x1821     ; 0x21 = ld hl, 0x18 = jr for next instruction
      jr		label          ; Loop forever

coldloadend::
	; The coldloader triplet utility looks for this sequence to trim any
   ; constants the compiler emits automatically from the binary.
	dw 0x7676
	dw 0x7676
#endasm


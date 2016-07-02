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
    //*** Write 0 to Phys. addr. 8xxxx (Assuming
    //   correct memory mapping) and verify write.
		ld		h, 0xD0
		xor   a			; A = 0
      ld    b, a     ; For upper nibble of GOCR later
		ld    (hl), a  ; Write 0 to mem. @ D0xx
		ld    a, (hl)  ; Read back
		or    a        ; Set zero flag if zero
    	jr	 	nz, infLoop

    	//*** Write FF or FE to same address and verify write
    	dec   a        ; A = 0xFF
    	ld    (hl), a  ; Write to mem. @ D0xx
    	ld    a, (hl)  ; Read back
    	inc   a        ; Executes twice on 16 bit
    	jr	 	nz, infLoop

    	//****** It's RAM on CS0, Turn STATUS on *****
    	ld    c, 0x0E
    	ld    a, 0x30
    	ioi   ld (bc), a     ; Load 0x30 GOCR to turn on STATUS
infLoop:
    	jr	 	infLoop

coldloadend::
	; The coldloader triplet utility looks for this sequence to trim any
   ; constants the compiler emits automatically from the binary.
	dw 0x7676
	dw 0x7676
#endasm


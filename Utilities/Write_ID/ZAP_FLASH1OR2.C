//
//	zap_flash1or2.c --- erase entire flash (1 or 2 or both)
//
// CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION!
//
// This utility erases the ENTIRE flash or flashes, including the ID and
//  User Blocks if ZAP_FIRST_FLASH is defined!
//
// CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION!
//

// uncomment one or both of the following lines
//#define ZAP_FIRST_FLASH
//#define ZAP_SECOND_FLASH

#ifndef _RAM_
#error "This program must be run in RAM!"
#endif

#ifndef ZAP_FIRST_FLASH
	#ifndef ZAP_SECOND_FLASH
	#error "CAUTION!  This utility erases the ENTIRE flash or flashes,"
	#error "including the ID and User Blocks if ZAP_FIRST_FLASH is defined!"
	#endif
#endif

void ZapFlashAtMB3CR(void);

root void main(void)
{
	;
#ifdef ZAP_FIRST_FLASH
	// assumes first flash is on CS0, sets maximum wait states
	WrPortI(MB3CR, &MB3CRShadow, 0x00);
	ZapFlashAtMB3CR();
#endif
#ifdef ZAP_SECOND_FLASH
	// assumes second flash is on CS2, sets maximum wait states
	WrPortI(MB3CR, &MB3CRShadow, 0x02);
	ZapFlashAtMB3CR();
#endif
	;
}

root void ZapFlashAtMB3CR(void)
{
	;
#asm
#ifdef _ENABLE_16BIT_FLASH_
      ld    c,0x0c
      ld    b,0

      ld    de,0xaaaa
      ld    px,bcde
      ld    hl,0x00aa
      ld    (px),hl        ; 0xaa -> 0x5555

      ld    de,0x5554
      ld    px,bcde
      ld    hl,0x0055
      ld    (px),hl        ; 0x55 -> 0x2aaa

      ld    de,0xaaaa
      ld    px,bcde
      ld    hl,0x0080
      ld    (px),hl        ; 0x80 -> 0x5555

      ld    de,0xaaaa
      ld    px,bcde
      ld    hl,0x00aa
      ld    (px),hl        ; 0xaa -> 0x5555

      ld    de,0x5554
      ld    px,bcde
      ld    hl,0x0055
      ld    (px),hl        ; 0x55 -> 0x2aaa

      ; write chip erase command to destination address
      ld    de,0xaaaa
      ld    px,bcde
      ld    hl,0x0010
      ld    (px),hl        ; 0x10 -> 0x5555

      ; wait for flash
      ld    de,0x0000
      ld    px,bcde
   .flash_wait1:
      ld    a,(px)                ; read byte at flash address 0 into a
      ld    h,a
      ld    a,(px)                ; read byte at flash address 0 into a
      xor   h                     ; xor two read together
      bit   6,a                   ; check if bit 6 is set
      jr    nz,.flash_wait1       ; if bit did not toggle, read again
#else
		ld		a, 0B7h
		ld		xpc, a
		ld		a, 0AAh
		ld		(0E555h), a				;AA to 5555
		ld		a, 0B4h
		ld		xpc, a
		ld		a, 055h
		ld		(0EAAAh), a				;55 to 2AAA
		ld		a, 0B7h
		ld		xpc, a
		ld		a, 080h
		ld		(0E555h), a				;80 to 5555

		ld		a, 0B7h
		ld    xpc, a
		ld    a, 0AAh
		ld    (0E555h), a				;AA to 5555
		ld		a, 0B4h
		ld    xpc, a
		ld    a, 055h
		ld    (0EAAAh), a				;55 to 2AAA
		ld		a, 0B7h
		ld    xpc, a
		ld    a, 010h
		ld    (0E555h), a				;10 to 5555

		; now wait until the flash chip has completed its erasure
		ld		hl, 0E000h
.busyWait:
		ld		a, (hl)
		xor	(hl)
		jr		nz, .busyWait
#endif
#endasm
	;
}
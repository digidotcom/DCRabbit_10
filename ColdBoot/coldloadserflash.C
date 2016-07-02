//The cold loader takes care of the following tasks:
//- Run a timing calculation to determine the clock speed of the board.
//- Using the clock speed to determine 57600 baud, receive the pilot BIOS.
//- Store (clock speed / 7200) at bytes 0 and 1 of RAM.

//TIMING:
//The strategy here is to see how many times the timer A interrupt will fire
//in a constant amount of real time.  We use the periodic interrupt to measure
//some amount of time (1/16 of a second happens to be convenient) and just count
//the number of timer A interrupts that occur.  If timer A is driven by pclk/2
//and timer A1 is set to go off every 225 counts, your final count value will
//equal the processor's clock speed / 7200.  To communicate with Dynamic C and
//receive the pilot you need to find the timer A divider value to achieve 57600
//baud.  The math ends up being very simple:
//
//timer A value = (count + 128)/256 - 1
//
//The 128/256 is for rounding, since there will be some variance and you are
//unlikely to land exactly on the correct count number.
//
//PRODUCING THE COLD LOADER:
//This source file should be used in conjunction with triplet.exe, which is
//built from triplet.cpp.
//The '@' symbol designates parameters for the triplet utility. Right now it
//is designed to take a starting address, assumes that the binary is contiguous
//and takes io register settings in sequence and prefixes them to the triplets
//The utility knows macro names used in Dynamic C.
//
//After this program is compiled to a .bin file, check it in a hex editor to
//make sure that the compiler has not inserted any extraneous bytes of code.
//
//WARNING: This cold loader must not use any Rabbit 4000 specific
//mainpage instructions.
//A good heuristic is all 8-bit loads not involving 'a' and all 8 bit
//arithmetic intsructions are on the 7F page. The best thing to do is look at
//the list file and search for the 7F prefix. But besides that, be extra
//careful that you're not using a 4000 only instruction.
//

//@ SEGSIZE    0xE6
//@ STACKSEG   0x74				// 0xE000 logical is at 0x082000 physical
//@ DATASEG    0xFE				// 0x6000 logical is at 0x004000 physical
//@ SACR       0x00
//@ TAT1R      0xE0
//@ TACSR      0x03

// start signals the end of io register values and gives the starting offset
// to the triplet utility.

//@ start  0x0000

//These are the replacement for the interrupt vector loading.  If the code
//changes, the locations of the interrupt vectors will likely change as well.
//Currently the vectors live at 0x50 and 0x62; compile the loader and update
//these addresses as necessary.  Interrupt vectors can be easily located by
//doing a search for 0xF5 (push AF).
//@ patch   0x5000   0xC3
//@ patch   0x5001   0x5C
//@ patch   0x5002   0x00
//@ patch   0x50A0   0xC3
//@ patch   0x50A1   0x6E
//@ patch   0x50A2   0x00

//@ SPCR     0x80

// Make sure the assembler doesn't add nop's for forward referenced labels...
#define call
#define real_call(expr) db 0xCD $ dw expr

#ifdef __SEPARATE_INST_DATA__
#if __SEPARATE_INST_DATA__
#error "Turn off separate I&D space."
#endif
#endif

#define INTVEC_BASE 0x5000    //we need to define this because the memory layout
                              //isn't defined yet; 0x5000 was chosen arbitrarily

void _get_byte();

#asm
main::
      ; Change DATASEG on Rabbit 6000 to store pilot in top 64KB
		; of internal RAM (0xEAxxx + 0x06000 = 0x0F0000)
ioi	ld		a, (GCPU)		; Check CPU type
		and   0x1F				; and only remap to upper 64K if Rabbit 6000
		cp    0x04
		jr    nz, .not6000
		ld    a, 0xEA
ioi   ld    (DATASEG), a      ; map data segment so 0x6000 logical is 0x0FE000

.not6000:
		; set up stack in low root segment (used by ISRs to preserve AF)
      ld    sp, coldloadend+0x200 

      ;set up interrupt vector table
      ld    a, 0xff & (INTVEC_BASE >> 8)
      ld    iir, a

      ;set up periodic interrupt parameters
      ld    b, 129   ;we need one extra since the first interrupt doesn't count
      bool  hl
      db    0x6C     ; Rabbit 3000 opcode for ld L, h

		; HL is set to 0, GCSR is 0, so save 4 bytes by using (hl)
ioi   ld    a, (hl)   ;reading GCSR clears periodic interrupt
      ld    a, 0x09     ;enable periodic interrupt
ioi   ld    (hl), a
      ipset 0

      ;wait for timing measurement to complete (b == 0)
		; Timer A ISR increments HL, periodic ISR decrements b
_timer_wait_loop:
      ld    a, b
      or    a
      jr    nz, _timer_wait_loop

      ld    a, 0x08     ;disable periodic interrupt
ioi   ld    (GCSR), a

      ;save the baud rate count for later use
      ;the pilot will use this value to accurately calculate baud rate divisors
      ld    (0), hl

      ld    c, 128   ;after the loop, b will be 0
      add   hl, bc
      dec   h        ;by using the value in H, you effectively divide by 256
      ld    a, h
ioi   ld    (TAT4R), a     ; set timer A4

      real_call(_get_byte)    ; pilot BIOS's size LSB
      ld    c, a

      real_call(_get_byte)    ; pilot BIOS's size MSB
      ld    b, a

      ld    hl, 0x6000

_load_pilot_loop:
      real_call(_get_byte)
      ld    (hl), a
      inc   hl                   ; increment the copy-to-RAM index
      dwjnz _load_pilot_loop

      jp    0x6000               ; start running pilot bios

_get_byte::
pollrxbuf:
ioi   ld    a, (SASR)       ; check byte receive status
      bit   7, a
      jr    z, pollrxbuf    ; wait until byte received
ioi   ld    a, (SADR)       ; get byte
      ret


_periodic_timer_isr::
      push  af       ;protect a and flags
ioi   ld    a, (GCSR)      ;clear interrupt
      ld    a, 0x01     ;enable timer A interrupt at priority 1
      djnz  isr_exit
last_iteration:
      xor   a           ;turn off timer A interrupt
isr_exit:
ioi   ld    (TACR), a   ;will set this every time the ISR runs, but that's ok
      pop   af
      ipres
      ret


_timerA_timer_isr::
      push  af       ;protect a and flags
ioi   ld    a, (TACSR)     ;clear interrupt
      inc   hl       ;hl holds the number of times this ISR has run
      pop   af
      ipres
      ret


coldloadend::
   ; The coldloader triplet utility looks for this sequence to trim any
   ; constants the compiler emits automatically from the binary.
   db 0x76, 0x76, 0x76, 0x76
#endasm


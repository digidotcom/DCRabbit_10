/*************************************************************
Pilot BIOS source code for 16-bit memory width.

This is what Dynamic C uses to load the regular BIOS/program to RAM/Flash
*************************************************************/
#include "legacy_keywords.h"

// Note:  The "PILOT_BIOS" macro should be placed in the
//  Project Options' Defines box.

// *** CAUTION ***
// The pilot BIOS makes global use of px, py, and pz.
// Moreover, the serial ISR uses the alternate registers for processing,
// and operates at IP 1.

#pragma DATAWAITSUSED off

#memmap root

#ifndef _RAM_
   #fatal "Must compile pilot BIOS to RAM."
#endif

#ifdef __SEPARATE_INST_DATA__
   #if __SEPARATE_INST_DATA__
      #fatal "Turn off separate I&D space."
   #endif
#endif

#undef MSB_BIT
#define MSB_BIT 23

#undef MECR_VALUE
#define MECR_VALUE 0x80
#define MSB_OFFSET (((MECR_VALUE >> 5) + 1) & 7)

#if MSB_OFFSET == 0
	#define MSB_ID_MASK_BIT 2
#elif MSB_OFFSET == 1
	#define MSB_ID_MASK_BIT 3
#elif MSB_OFFSET == 2
	#define MSB_ID_MASK_BIT 4
#elif MSB_OFFSET == 3
	#define MSB_ID_MASK_BIT 5
#elif MSB_OFFSET == 4
	#define MSB_ID_MASK_BIT 6
#elif MSB_OFFSET == 5
	#define MSB_ID_MASK_BIT 7
#endif

#undef _FLASH_SIZE_
#define _FLASH_SIZE_ 0x800

// the serial port to use
#define _PB_SxCR        SACR
#define _PB_SxSR        SASR
#define _PB_SxDR        SADR
#define _PB_INT_VECTOR  SERA_OFS
#define _PB_PCFRVAL     0xC0
#define _PB_BAUDTIMER   TAT4R

struct _dkcHeader {     // structure for a communication header
   char  count;            // number of bytes to transmit/receive
   int  address;        // address
   char  XPCval;           // XPC
   char chksum;         // checksum
   char *cPointer;      // content pointer
   int  length;
};

// These types of definitions are usually defined through some path from
// default.h, but the pilot BIOS project does not use that file.
typedef unsigned char uint8;
typedef unsigned int uint16;
typedef uint16 word;
typedef unsigned long uint32;
typedef uint32 faraddr_t;

#use "idblock.lib"
#use "tc_defs.lib"
#use "mutil.lib"
#use "util.lib"
#use "flashwr.lib"

// Pilot BIOS' own shadow registers
char MB0CRShadow,MB1CRShadow,MB2CRShadow,MB3CRShadow,MECRShadow;

// identification of the hardware
int	PB_RAM_SIZE;
int	PB_FLASH_ID;
char	PB_DIV19200;
char	PB_FREQDIVIDER;
char 	PB_USEDOUBLER;
unsigned long	_PB_CPUID;
unsigned long  PB_IDBLOCK_PADDR; // physical address of the system id block

root int _PB_Init() ;  // initialization for the Pilot BIOS finite state machine
root _PB_WriteMem();
root _PB_ReadProgPort();
root _PB_WriteProgPort();
root _PB_SetIntVecTab();
root int _PB_getcrc(char *Data, char count, int accum);

#define HIXPC 				0x71
#define LOXPCBASE 		0xF9

// These are addresses of memory locations whose contents are changed by the
// cold loader. It is very important that these values are consistent with the
// values used by the cold loader.
#define DIVADRS 			0x3F00
#define REGBIOSFLAG		0x3F01
//#define FREQADRS			0x3F02
#define BEGREGBIOS      0x0000

/******* variable definitions ***********/
#define SYSSTLENGTH 0x300
char sysStack[SYSSTLENGTH];

struct _dkcHeader curHeader;     // current header information

// the current packet
_TC_PacketHeader  _PB_Header;
_TC_PacketFooter  _PB_Footer;
char              _PB_Buffer[0x1000];  // packet body

// If flashWriteCount is non-zero, the pilot BIOS is writing data to flash.
unsigned int      flashWriteCount;

// vars for the ISR - it receives into this temp buffer
// this _must_ use powers of two - the pilot code depends on the wrapping.
// This is the circular buffer used by the serial ISR to buffer incoming data
// verbatim.
char  _PB_RXBuffer[0x1000];

// BUFFER_MASK *must* be one less than the circular buffer size
#define BUFFER_MASK 0xFFF

unsigned int _PB_RXWritePointer; // offset into _PB_Buffer we are receiving into
unsigned int _PB_RXReadPointer;  // offset into _PB_Buffer we are reading from

// State machine variables
void* _PB_Mode;            // mode (rx/tx) that the FSM is currently in
void* _PB_State;           // this is the address (state) of the comm FSM
char  _PB_esc;             // escaped character marker
int   _PB_length;          // length remaining to read/write
char* _PB_ptr;             // pointer into the currently receiving/transmitting
                           // buffer
int   _PB_checksum;        // running checksum, computed on a byte-by-byte basis
int   _PB_save_checksum;   // temporary storage of the checksum

// flags to queue the changing of the BAUD rate after a SETBAUDRATE packet */
char  _PB_setbaudrate;     // true if a baud rate change has been queued
char  _PB_newdivider;      // the new divider to set the baud rate too
char  _PB_waittx;          // true if we are waiting for a TX to finish before
                           // changing the baud rate

#asm root nodebug
premain::
main::
	db	0xCC	; The cold loader explicitly checks for this byte.

ioi	ld a, (MMIDR)
   or		0x80
   ioi	ld (MMIDR),a
   ld    a, 0xC0
   ioi   ld (EDMR),a
   ld		a, 0x22
   ioi	ld (MACR),a
		nop $ nop		; *always* follow any MACR update with two NOPs
   ld		a, 0xD6
   ioi	ld (SEGSIZE), a
   ld    a, 0x80
   ioi   ld (MECR), a
   ld    (MECRShadow),a

;**** Grab the 19200 baud frequency divider
	// The cold loader will have, through auto-baud, detected the timer A value.
   // The pilot can calculate the 19200 divider from it. Add one, then multiply
   // by six.
ioi	ld		a, (TAT4R)
	inc	a
   ld		b, a
   add	a, b	; *2
   add	a, b	; *3
	ld		(PB_DIV19200), a

;**** TEST RAM SIZE ***********
;	Dynamic C needs to know how much RAM is available

	ld		a, 0x80
ioi	ld		(MECR), a
	ld		c, 1

   ld		px, 0x3fffff
   ld		a, (px)
   ld		L, a
   ex		jk', hl ; Original high byte stored in k'
   ld		py, 0x7fff

.loop:
   ld		a, (py) ; Current address
   ld		b, a
   ld		a, (px) ; Hi address.
   cp		a, b
   jr		nz, .not_it

   cpl
   ld		(px), a
   ld		b, a
   ld		a, (py)

   cp		a, b
   jr		z, .done

.not_it:
	inc   c
   jr		z, .errdone
   ld		jkhl, py
   ld		a, c
   ld		bcde, 0
   ld		d, 0x80
   add	jkhl, bcde
   ld		py, jkhl
   ld		c, a
	jr    .loop

.errdone:
	ld    c, 0
.done:
	ex		jk', hl
   ld		a, L
   ld		(px), a
	ld		a, c
	ld		(PB_RAM_SIZE),a
   xor   a
	ld		(PB_RAM_SIZE+1),a

;**** END TEST RAM SIZE ********

	; the pilot BIOS's stack is not set up yet, use inline code
	;  to hit the watchdog instead of " call bioshitwd"
	ld		a, 0x5A
	ioi	ld (WDTCR), a

	ipset 1

._PB_SetProgPort:
	;	comm port A
	;	parallel port c, bit 6,7 use alternate pin assignment

   clr	hl
   ld		(flashWriteCount), hl

	ld 	a, _PB_PCFRVAL
	ioi   ld (PCFR),a

	ld		a,0xc3			;	jump instruction
	ld		(_PBINTVEC_BASE+_PB_INT_VECTOR),a
	ld		hl,._PB_SerialISR
	ld		(_PBINTVEC_BASE+_PB_INT_VECTOR+1),hl

	;	at port c, async, 8-bit, interrupt priority 1
	ld 	a,00000001b
	ioi   ld (_PB_SxCR),a

._PB_GetCPUID:
	ioi	ld a, (GROM)			; read CPU identification parameters
	and	0x1F						; (formatted for unsigned long read by compiler)
	ld		(_PB_CPUID+2), a
	ioi	ld a, (GRAM)
	and	0x1F
	ld		(_PB_CPUID+3), a
	ioi	ld a, (GREV)
	and	0x1F
	ld		(_PB_CPUID+0), a
	ioi	ld a, (GCPU)
	and	0x1F
	ld		(_PB_CPUID+1), a

._PB_SetSP:
   ; initialize stack pointer
   ld    hl, sysStack+sizeof(sysStack)
   ld    sp, hl

   ;; mark program flash initialization not yet done (this is a safety
   ;;  feature in case future pilot BIOS changes introduce a path where
   ;;  _readIDBlock gets called before _InitFlashDriver is called)
   clr   hl
   ld    (_InitFlashDriverOK), hl

  	ld		bcde, 0
   ld		bc, 0x80
   push	bcde
   push  bcde
   call	_InitFlashDriver
   add	sp, 8
   ;The return value of _InitFlashDriver is not checked.  Even if an error
   ;occurred during init, we still want to allow the pilot to continue
   ;so that we can perform a compile to RAM for debugging.

   ld		hl, (_flash_info + _FlashIdEntry + id)
   ld		(PB_FLASH_ID), hl
   ld    hl, 0x04          ; Only check the MB2 quadrant. When we get 8meg
                           ; flashes, this will have to change.
   call  _readIDBlock      ;return value in HL
	bool	hl
	jr		z, ._PB_idBlockOk

._PB_idBlockBad:				; clear SysIDBlock struct if not on flash or in RAM
	ld		hl, SysIDBlock
	ld		b, SysIDBlock+marker+6-SysIDBlock
	xor	a
.blockEraseLoop:
	ld		(hl), a
	inc	hl
	djnz	.blockEraseLoop

._PB_idBlockOk:

_PB_CheckClockDoubler::
	call	_PB_getDoublerSetting
	ld		a, l
	ld		(PB_USEDOUBLER), a	; nonzero = enable doubler (later)

	ld		a, (PB_DIV19200)
	bool	hl							; still contains result of _PB_getDoublerSetting
	jr		z, .dont_double
.do_double:
	sla	a							; multiply by two
.dont_double:
	ld		(PB_FREQDIVIDER), a	; use this value for baud rate calcs

._PB_SetComm:
      call  _PB_InitBaudRateChange
      call  ._PB_InitTC
      call  _PB_SetIntVecTab
      ipset 0
_PB_Loop::
	ld		a, 0x5A						; hit watchdog timer
	ioi	ld (WDTCR), a
      call  ._PB_Entry           ; enter the FSM
      ld    a, (_PB_setbaudrate)
      or    a
      jr    z, _PB_Loop
      call  _PB_RunQueuedBaudRateChange   ; run any queued baud rate changes
      jp    _PB_Loop

._PB_InitTC:
      call  _PB_Init                ; initialize comm module
      call  _PB_InitRXRing          ; init the RX ring pointers
      ret

bioshitwd::
	push	af
	ld		a, 0x5A						; hit watchdog timer
	ioi	ld (WDTCR), a
	pop	af
	ret

//*********** Serial interrupt handler ************
; Beware! Use alternate registers in the pilot with caution.
._PB_SerialISR:
      ex    af, af'

ioi   ld    a,(_PB_SxSR)         ; what type of int is this?
      bit   7,a
      jr    z,._PBNotRXInt

      exp
      exx

      ; this is a receive interrupt
ioi   ld    a,(_PB_SxDR)         ; get the byte (and clear the interrupt)
      ld    hl, (_PB_RXWritePointer) ; Load current circular buffer offset to hl
      ldl   pw, _PB_RXBuffer
      ld    (pw+hl), a         ; Store the byte.
      inc   hl                 ; Update the write offset, wrap at size of buffer
      ld    de, BUFFER_MASK
      and   hl, de
      ld    (_PB_RXWritePointer), hl

      ; Duplicated for speed. Saves a branch.
      exx
      exp
      ex    af, af'
      ipres
      ret

._PBNotRXInt:
      ; TX is handled by POLLING - just drop these interrupts
ioi   ld    (_PB_SxSR),a
      ex    af, af'
      ipres
      ret

_PB_ReadProgPort::
;  destroys A
;  returns byte read (if any) in A
;  returns with Z set if nothing is read

      ;  check if there is anything available
ioi   ld    a,(_PB_SxSR)
      bit   SS_RRDY_BIT,a        ; if a received byte ready?
      ret   z                    ; nope, return with z set
      ;  otherwise, a byte *is* ready, read from data port
ioi   ld    a,(_PB_SxDR)
      ret                        ; return with z *not* set

_PB_WriteProgPort::
;  assumes byte to transmit is in C
;  destroys A
;  returns with Z reset if not transmitted

      ;  check if the port is ready
ioi   ld    a,(_PB_SxSR)

      bit   SS_TFULL_BIT,a       ; can I transmit now?
      ret   nz                   ; nope, return with nz set
      ; otherwise, the transmit buffer is ready, write to it!
      ld    a,c                  ; move byte to transmit to a
ioi   ld    (_PB_SxDR),a
      ret                        ; return with z *not* set

_PB_CanProgPortTransmit::
;  destroys A
;  returns with Z reset if the transmitter is busy,
;  and Z set if it is avaliable to transmit
ioi   ld    a,(_PB_SxSR)
      bit   SS_TFULL_BIT,a       ; can I transmit now?
      ret

_PB_IsProgPortTxBusy::
; destroys A
; returns with Z reset if the transmitter is doing anything
; and Z set if the transmitter is completely idle
ioi   ld    a,(_PB_SxSR)
      bit   2,a
      ret   nz
      bit   3,a
      ret

_PB_SetIntVecTab::
      ;  set up interrupt vector table
      ld    a,0xff & (_PBINTVEC_BASE >> 8)   ; R register has 0x20, so interrupt
                                             ; table starts at 2000
      ld    iir,a
      ld    eir,a
      ret

_PB_InitRXRing::
      clr   hl
      ld    (_PB_RXWritePointer), hl
      ld    (_PB_RXReadPointer), hl
      ret

_PB_Init::  ;  initialize the communication module
      ld    hl,._PB_ModeRX
      ld    (_PB_Mode),hl        ; start receiving initially
      ld    hl,._PB_RXNothing
      ld    (_PB_State),hl       ; default to the Nothing state, will drop bytes
      xor   a
      ld    (_PB_esc),a          ; do not unescape the next byte
      clr   hl
      ld    (_PB_length),hl      ; nothing to receive
      ld    (_PB_ptr),hl         ; no buffer to receive into
      ret

._PB_Read::   ; Read out of the RX ring instead of calling the ReadPort function
; This block of code writes a byte to flash if pending. px, py and pz are
; set up when a flash write message and payload are received.
      ld    hl, (flashWriteCount)
      test  hl
      jr    z, _PB_SkipFlashWrite
      dec   hl
      ld    (flashWriteCount), hl
      bit   0, L ; odd? wait till next byte to write.
      jr    nz, _PB_SkipFlashWrite
      ld    hl, (pz)
      push  py
      push  pz
      call  _ProgramFlashUnit
      pop   pz
      pop   py
      ld    py, py+2
      ld    pz, pz+2
_PB_SkipFlashWrite:

      ld    de, (_PB_RXWritePointer)
      ld    hl, (_PB_RXReadPointer)
      cp    hl, de               ; do the pointers match?
      ret   z

      ; data is good - update the read pointer
      ldl   pw, _PB_RXBuffer
      ld    a, (pw+hl)
      inc   hl
      ld    de, BUFFER_MASK
      and   hl, de
      ld    (_PB_RXReadPointer), hl
      ret   nz ; Return nz if that was the result of the AND operation (common)
      inc   hl
      test  hl
      ret   ; Otherwise hl was zero, so inc, test and return with nz set.

._PB_Write:    ; this is the abstraction of the actual write mechanism
      jp    _PB_WriteProgPort

._PB_CanTransmit: ; abstraction of the above
      jp    _PB_CanProgPortTransmit

._PB_IsTransmiterIdle:  ; abstraction of the above
      jp    _PB_IsProgPortTxBusy

; Computes a checksum
; Uses the 8-bit Fletcher checksum algorithim. See RFC1145 for more info
;
; assumes the following:
;   hl == the checksum variable
;   a  == the value to add to the checksum
_PB_Checksum::
      ld    c,a                        ; save the value in a
      add   a,h
      adc   a,0x00
      ld    h,a                        ; A = A + D[i]
      add   a,l
      adc   a,0x00
      ld    l,a                        ; B = B + A
      ld    a,c                        ; restore a
      ret

; Converts a physical address stored in bc,de to a logical address.
; The resulting xpc will be in jk, and the offset will be in hl.
_PB_PhysicalToLogical::
      push  bc       ; save the address for later
      push  de
	ld		a,0xf0
	and	d			; a = d & 0xf0
	or		c			; a = c | d
   srl   4,bcde
   srl   4,bcde
   srl   4,bcde
   ld    jkhl,0x0e
   ex    jkhl,bcde
   sub   jkhl,bcde
   ld    a,h
   and   0x0f
   ld    h,a
   ex    jk,hl
	pop	hl
   ld    d,h
   ld    e,L
   ld    a,0x0f
   and   h
   or    0xe0
   ld    h,a
      pop   bc       ; restore the address
      ret

;//****** START IN RAM - just jump to the beginning again
_PB_StartRegBiosRAM::
      xor   a           ;disable clock doubler
ioi   ld    (GCDR), a
      jp    BEGREGBIOS

;//****** START IN FLASH - Code to fix the MBxCR mapings, and jump to 0x0000

_PB_StartRegBiosFLASH::

   ipset 3
	call	bioshitwd
   ld		a,0x20
   ioi	ld (MACR),a
		nop $ nop		; *always* follow any MACR update with two NOPs
	xor	a
  	ioi	ld (GCDR), a
   ld		a,0x00
   ioi	ld (MECR),a
	ioi	ld a,(EDMR)
   and	0x3f
   ioi	ld (EDMR),a
   ld    a,0xb2
   ld    xpc,a
   ld		a,0x51
   ioi	ld (WDTTR),a
   ld		a,0x54
   ioi	ld (WDTTR),a
   ld		a,0xB0
   ioi	ld (MB3CR),a
      ; Pilot bios download performance relies on a wait-first strategy for the
      ; flash writing functionality, so that the pilot can continue buffering
      ; input while the flash write is completing. So, ensure flash writing is
      ; complete before jumping into the regular BIOS in flash.
      ld    hl,0xe000
.wloop:
      ld    a, (hl)        ; get an initial flash toggle-bit status value
      cp    (hl)           ; did the toggle-bit change?
      jr    neq, .wloop    ; if yes (incomplete write), go check again . . .

      cp    (hl)           ; did the toggle bit change? (SST work around)
      jr    neq, .wloop    ; if yes (incomplete write), go check again . . .

      jp    (hl)           ; run the application's BIOS (in parallel flash)

;//****** Lookup the Baud Rate divider, based on div19200
; expects:
;     ix: points to the (long) baud rate
; returns (if nz is set):
;     a: the new divider
; returns (if z is set):
;     <nothing - baud rate is unacceptable>
_PB_LookupBaudRateDivider::
      ld    hl, _PB_BaudTable
._PB_LookupLoop:
      ex    de, hl
      ld    hl, _PB_EndBaudTable
      or    a
      sbc   hl, de              ; does (hl == _PB_EndBaudTable)?
      ret   z                   ; return w/ z set if we hit the end of the table
      ex    de, hl

      ld    iy, hl               ; iy has the table entry
	call	._PB_CompareTableEntry
	jr		z,._PB_FoundTableEntry

	ld		de,5						; de == size of one table entry
	add	hl,de
	jr		._PB_LookupLoop		; try the next entry

._PB_FoundTableEntry:				; iy has the table entry!
	ld		a,(iy+4)					; get the divider value
	ld		e,a						; put it in 'e'
	ld		a,(PB_FREQDIVIDER)	; a has the original divider
	ld		d,a						; put it in 'd'
	xor	a
	ld		b,a						; the result will be in 'b'

._PB_DivLoop:	; compute (d/e)
	; are we done? (is the old divider 0?)
	xor	a
	or		d
	jr		z,._PB_DivFinished

	; do one subtraction
	ld		a,d
	sub	e
	jr		c,._PB_BadDiv
	ld		d,a

	; increment the result
	inc	b
	jr		._PB_DivLoop

._PB_BadDiv:	; couldn't divide it evenly
	xor	a
	or		a							; set z
	ret

._PB_DivFinished:
	ld		a,b
	dec	a							; subtract 1 from the divider to get the actuall value
	ld		b,a						; keep in in b for a sec...

	ld		d,0xff
	or		d							; set nz

	ld		a,b						; the return value
	ret

_PB_BaudTable::
;        baud rate, in hex     divider relative to 19200
;        (little endian!)
;        --------------------  -------------------------
      db    0x00,0xE1,0x00,0x00,  3
      db    0x00,0xC2,0x01,0x00,  6
      db    0x00,0x84,0x03,0x00,  12
      db    0x00,0x08,0x07,0x00,  24
_PB_EndBaudTable::
      db    0x00,0x00,0x00,0x00,  0x00

._PB_CompareTableEntry:	; compare a single entry - ix & iy should point to the longs
								; _MUST_ not clobber hl!
      ld    a,(ix+0)
      cp    (iy+0)
      ret   nz
      ld    a,(ix+1)
      cp    (iy+1)
      ret   nz
      ld    a,(ix+2)
      cp    (iy+2)
      ret   nz
      ld    a,(ix+3)
      cp    (iy+3)
      ret

_PB_InitBaudRateChange::
      ; init the baud rate change values
      xor   a
      ld    (_PB_setbaudrate),a
      ld    (_PB_newdivider),a
      ld    (_PB_waittx),a
      ret

_PB_RunQueuedBaudRateChange::   ; poll to see if we need to change the baud rate
      ld    a,(_PB_waittx)
      or    a
      ret   nz                      ; return if the transmitter is still active

      ; test the serial port
      call  ._PB_IsTransmiterIdle
      ret   nz                      ; the transmitter is busy - return

      ; the transmitter is completely idle! set the baud rate!
      push  ip
      ipset 3                       ; ints off for safety!

      ; enable doubler if possible
      ld    a, (PB_USEDOUBLER)
ioi   ld    (GCDR), a

      ld    a,(_PB_newdivider)
ioi   ld    (_PB_BAUDTIMER),a    ; set the baud rate!

      call  _PB_InitBaudRateChange  ; reset everything back to normal
      call  ._PB_InitTC             ; make sure the FSM is reset as well

      call  ._PB_Read                  ; flush any byte currently there...

      pop   ip
      ret

;//****** Begin Pilot BIOS FSM ****************************

._PB_Entry:    ;this is the entry point of the communication FSM
      ld    hl,(_PB_Mode)        ; load address of current mode
      jp    (hl)                 ; and jump indirect to it

._PB_ModeRX:   ; receiving - get the byte that was received
      call  ._PB_Read
      ret   z                    ; nothing was available - return

      cp    TC_FRAMING_START
      jr    z,._PB_RXHaveStart   ;was the character the beginning of a new packet?

      ld    b,a                  ; save the read character
      ld    a,(_PB_esc)
      or    a                    ; should the next character be escaped
      jr    z,._PB_RXNoEsc

      ld    a,b
      xor   0x20                 ; unescape the character
      ld    b,a
      xor   a
      ld    (_PB_esc),a          ; mark the next character as not-escaped
      jr    ._PB_RXHaveCharacter ; continue with the un-escaped character

._PB_RXNoEsc: ; the character is in b, and does not need to be escaped
      ld    a,b                  ; character is in BOTH a AND b
      cp    TC_FRAMING_ESC
      jr    nz,._PB_RXHaveCharacter ; not the ESC character

      ld    a,0x01
      ld    (_PB_esc),a          ; mark the next character as one that needs to
                                 ; be escaped
      ret                        ; this character is done - return

._PB_RXHaveCharacter:   ; the good character is in b
      ld    a,b                  ; the character is in BOTH a AND b
      ld    hl,(_PB_checksum)    ; get the running checksum
      call  _PB_Checksum
      ld    (_PB_checksum),hl    ; save the running checksum

      ld    hl,(_PB_ptr)         ; is there any place to receive into?
      test  hl
      jr    z,._PB_RXSkipStore

      ; ptr is non-NULL -- store the character
      ld    a,b
      ld    (hl),a               ; store the character
      inc   hl
      ld    (_PB_ptr),hl         ; increment the ptr and store it back
      ld    hl,(_PB_length)      ; get the length
      dec   hl
      ld    (_PB_length),hl      ; decrement by 1 and store it again
      test  hl
      jr    z,._PB_RXSkipStore  ; if length is 0 (post decrement), enter the FSM
      ret                    ; otherwise, return, so more characters can be read

._PB_RXSkipStore:
      ld    a,b
      ld    hl,(_PB_State)
      jp    (hl)                 ; normal character - enter the RX state machine
                     ; the received byte is in 'a' if it matters

._PB_RXNothing:   ; a START character has not been received - do nothing
      ret

._PB_RXHaveStart: ; received a START character! reset everything for the new packet!
      clr   hl
      ld    a, h
      ld    (_PB_esc), a            ; do not escape anything, initially
      ld    (_PB_checksum), hl      ; init the checksum to 0
      ld    hl, TC_HEADER_SIZE-2
      ld    (_PB_length), hl     ; store the length of the header (minus the
                                 ; header_checksum)
      ld    hl, _PB_Header
      ld    (_PB_ptr), hl        ; mark where to store the header
      ld    hl, ._PB_RXHaveHeader
      ld    (_PB_State), hl         ; start receiving the header
      ret

._PB_RXHaveHeader: ; have the header of the packet - store the checksum and get
                   ; the header_checksum to compare
      ld    hl, (_PB_checksum)
      ld    (_PB_save_checksum), hl ; save the header_checksum for later
      ld    hl, 2
      ld    (_PB_length), hl     ; save the length (only 2, as only the
                                 ; header_checksum needs to be received)
      ld    hl, ._PB_RXHaveHeaderChecksum
      ld    (_PB_State), hl
      ret

._PB_RXHaveHeaderChecksum:
      ld    hl, (_PB_save_checksum)
      ex    de, hl
      ld    hl, (_PB_Header+header_checksum)
      or    a
      sbc   hl, de
      jr    nz, ._PB_RXBadHeaderChecksum  ; do the checksums match?

      ; they did - start receiving the body of the packet
      ld    hl, (_PB_Header+length)
      ld    (_PB_length), hl     ; length of the body
      ld    a, l
      or    h
      jr    z, ._PB_RXHaveBody      ; is the length 0? if so, go straight to
                                    ; getting the footer
      ld    hl, _PB_Buffer
      ld    (_PB_ptr), hl        ; point at the buffer...
      ld    hl, ._PB_RXHaveBody
      ld    (_PB_State), hl
      ret

._PB_RXBadHeaderChecksum: ; the header checksum failed - flush the packet
._PB_RXBadChecksum: ; the main checksum failed - flush the packet
      call  _PB_Init ; reset everything and wait for a START command again
      ret

._PB_RXHaveBody: ; the body of the packet has been received - get the footer
      ld    hl,(_PB_checksum)
      ld    (_PB_save_checksum),hl  ; save a copy of the checksum for later
      ld    hl,TC_FOOTER_SIZE
      ld    (_PB_length),hl      ; store the footer's length
      ld    hl,_PB_Footer
      ld    (_PB_ptr),hl         ; point at the storage for the footer
      ld    hl,._PB_RXHaveFooter
      ld    (_PB_State),hl
      ret

._PB_RXHaveFooter: ; the footer has been received - verify the checksum
      ld    hl,(_PB_save_checksum)
      ex    de,hl
      ld    hl,(_PB_Footer+checksum)
      or    a
      sbc   hl,de
      jr    nz,._PB_RXBadChecksum   ; the checksums didn't match

      ; the checksums matched - the entire packet has been received
      ld    ix,_PB_Buffer        ; point at the packet buffer
                                 ; the handlers will use this
      ld    a,(_PB_Header+type)
      cp    TC_TYPE_SYSTEM
      jp    nz,._PB_NakPacket    ; if the type is not SYSTEM then NAK the packet

._PB_SystemSubtype:
// This block is essentially a "flush" for when the flash write from the
// previously acknowledged write has not yet completed. px, py, and pz should
// still be loaded appropriately from the previous system write handler.
      ld    bc, (flashWriteCount)
      test  bc
      jr    z, ._PB_NoPendingWrite
      ; If there was a half-way write, then make it even before the array write.
      bit   0, c
      jr    z, ._PB_no_odd_correction
      inc   bc
._PB_no_odd_correction:
      push  bc                       ; Size
      push  pz                       ; Source buffer
      push  py                       ; Destination offset
      call  _ProgramFlashBlock
      add   sp, 10
      clr   hl
      ld    (flashWriteCount), hl

._PB_NoPendingWrite:
      ld    a,(_PB_Header+subtype)
      cp    TC_SYSTEM_NAK
      jr    z,._PB_HandleNAK
      cp    TC_SYSTEM_NOOP
      jr    z,._PB_HandleNOOP
      cp    TC_SYSTEM_READ
      jr    z,._PB_HandleREAD
      cp    TC_SYSTEM_WRITE
      jp    z,._PB_HandleWRITE
      cp    TC_SYSTEM_INFOPROBE
      jp    z,._PB_HandleINFOPROBE
      cp    TC_SYSTEM_STARTBIOS
      jp    z,._PB_HandleSTARTBIOS
      cp    TC_SYSTEM_SETBAUDRATE
      jp    z,._PB_HandleSETBAUDRATE
      cp    TC_SYSTEM_ERASEFLASH
      jp    z,._PB_HandleERASEFLASH
      cp    TC_SYSTEM_FLASHDATA
      jp    z,._PB_HandleFLASHDATA
      ; unknown subtype - NAK it!
      jp    ._PB_NakPacket

._PB_HandleNAK: ; Handling of NAKs is not supported here - just drop the packet
      call  _PB_Init
      ret

._PB_HandleNOOP: ; Reflect the packet back at them...
      jp    ._PB_AckPacket

._PB_HandleREAD: ; read a block of data, and reply with it
      ld    a,(ix)            ; get the TYPE of the READ
      cp    TC_SYSREAD_PHYSICAL
      jp    nz,._PB_NakPacket ; Not a physical-address READ! Not supported!

      ld    bcde, (ix+3)      ; get the physical address
      ld    px, bcde          ; px is the source!

      ld    hl, (ix+1)        ; get the length of the read
      ld    bc, hl
      ld    (ix), hl          ; build the ACK header
      ld    de, 6
      add   hl, de            ; get the total length of the packet
      ld    (_PB_Header+length), hl ; ...and store it in the packet

      ld    jkhl, (ix+3)
      ld    (ix+2), jkhl      ; move the physical address down one byte

      ld    de, 6
      add   ix, de            ; move ix such that it points at the destination
      ldl   py, ix            ; py has the destination
                        ; px has the source -- bc already has the length
      copy                    ; copy the data into the packet

      jp    ._PB_AckPacket    ; ACK the READ

._PB_HandleWRITE: ; write the given data out to memory
      ld    hl,lxpc
      push  hl
      ld    a,(ix)            ; get the WRITE type
      cp    TC_SYSWRITE_PHYSICAL
      jp    nz,._PB_NakPacket ; only PHYSICAL address are supported!

      ld    bcde, (ix+3)      ; get the physical address of the buffer
                              ; bc:de has the physical address

      ld    a, b              ; get flags
      and   0x3               ; mask out the unused bits
      cp    2
      jr    z, ._PB_WRITEFlash
      cp    0
      jr    z, ._PB_WRITERam
      ; 1 means fast ram which this pilot does not support, NAK this packet
      jp    ._PB_NakPacket

._PB_WRITERam:                ; write it to RAM...
      ld    py, bcde          ; py points to the destination
      ld    hl, ix
      ld    bcde, 7
      add   hl, de
      ldl   px, hl            ; px now points to the source data
      ld    hl, (ix+1)
      ld    bc, hl            ; bc has the length of the data
      copy
      jp    ._PB_WRITEAck     ; ack the packet

._PB_WRITEFlash:              ; write it to flash...
      ld    bcde, (ix+3)      ; get the physical address of the buffer
                              ; bc:de has the physical address

      // We get flash addresses as > 0x00800000 from
      // DC LMS, translate to flash offset and add to start of flash
      ld    b, 0
      ld    jkhl, 0
      ld    jk, 0x80          ; Flash top address = 0x800000
      add   jkhl, bcde
      ld    py, jkhl          ; Destination in py, this is a flash offset.

      ld    hl, (ix+1)
      bit   0, L
      jr    z, ._PB_already_even
      inc   hl
._PB_already_even:
      ld    (flashWriteCount), hl

      ldl   pz, _PB_Buffer + 7 ; The first 7 bytes are subtype, size, address.

._PB_WRITEAck:
      clr   hl
      ld    (_PB_Header+length),hl  ; no body to the ACK packet

      pop   hl
      ld    lxpc,hl        ; restore the xpc
      jp    ._PB_AckPacket    ; reply as an ACK

._PB_WRITENak:
      pop   hl
      ld    lxpc,hl
      jp    ._PB_NakPacket

._PB_HandleINFOPROBE: ; return a block of configuration data
      ld    hl, (IDBlockAddr)
      ld    (ix), hl
      ld    hl, (IDBlockAddr+2)
      ld    (ix+2), hl
      ld    bc, 4
      add   ix, bc
      ld    hl,(PB_FLASH_ID)  ; get the flash id
      ld    (ix),hl           ; ...and store it in the packet
      inc   ix
      inc   ix                ; move to the next field in the packet
      ld    HL,(PB_RAM_SIZE)  ; get the RAM size
      ld    (ix),hl           ; ...and store it in the packet
      inc   ix                ; move to the div19200 field
      inc   ix                ; move to the div19200 field
      ld    a,(PB_DIV19200)   ; get the 19200 baud divider
      ld    (ix),a            ; ...and store it in the packet
      inc   ix                ; move to the IDBlock field
      ld    a, (_PB_CPUID)    ; the current CPUID value
      ld    (ix),a            ; store the 4-byte CPUID value
      inc   ix
      ld    a, (_PB_CPUID+1)
      ld    (ix),a
      inc   ix
      ld    a, (_PB_CPUID+2)
      ld    (ix),a
      inc   ix
      ld    a, (_PB_CPUID+3)
      ld    (ix),a
      inc   ix                ; move past the CPUID value
      ld    hl,ix
      ex    de,hl                      ; destination is in DE
      ld    hl,SysIDBlock              ; hl is the source
      ld    bc,sizeof(SysIDBlockType)  ; bc is the length
      ldir                             ; copy the IDBlock

      ld    hl,sizeof(SysIDBlockType)
      ex    de,hl
      ld    hl, 13; sizeof(PB_FLASH_ID)+sizeof(PB_RAM_SIZE)+sizeof(PB_DIV19200)
                                       ;+sizeof(_PB_CPUID)
      add   hl,de                      ; hl == length of the packet - IDBlock+3
      ld    (_PB_Header+length),hl     ; store the length in the outgoing packet
      jp    ._PB_AckPacket             ; send the ACK back

._PB_HandleSTARTBIOS: ; start executing the main BIOS
      ld    a,(ix)                     ; get the start_mode
      cp    TC_STARTBIOS_RAM
      jp    z,_PB_StartRegBiosRAM      ; should we run in RAM?
      cp    TC_STARTBIOS_FLASH
      jp    z,_PB_StartRegBiosFLASH    ; should we run in FLASH?
      ;cp    TC_STARTBIOS_FASTRAM       ; this pilot does not support this message.
      ;jp    z,_PB_StartRegBiosFASTRAM  ; should we run in FAST RAM?
      jp    ._PB_NakPacket             ; unknown START type - nak it

._PB_HandleSETBAUDRATE: ; test the incoming baud rate and maybe set ours
      ; all replys have length 0
      clr   hl
      ld    (_PB_Header+length),hl     ; the reply is of 0 length

      call  _PB_LookupBaudRateDivider  ; lookup the divider
      jp    z,._PB_NakPacket           ; was the divider acceptable?

      ; queue the divider to be set after the packet is finished
      ld    (_PB_newdivider),a         ; store the divider for later
      ld    a,0xff
      ld    (_PB_setbaudrate),a      ; queue the baud rate change
      ld    (_PB_waittx),a           ; mark that we are waiting for TX to finish
      jp    ._PB_AckPacket           ; ACK the packet

._PB_HandleERASEFLASH: ; erase the entire FLASH
      ld    jkhl, (ix)               ; get physical address passed from compiler
      ld    bcde, 0
      ld    bc, 0x80
      add   jkhl, bcde                ; Actual address
      push  jkhl                      ; 2nd parameter
      push  bcde                      ; Erase from beginning.
      ; Before we go further, check if the user block would be clobbered.

      ld    bcde, (UserBlockAddr)
      cp    jkhl, bcde
      jr    c, ._PB_EraseApproved
      add   sp, 8
      jr    ._PB_NakPacket

._PB_EraseApproved:
      call  _EraseFlashRange
      add   sp, 8

._PB_HEFAck:
._PB_HandleFLASHDATA:
      clr   hl
      ld    (_PB_Header+length),hl     ; our ACK has no data
      jr    ._PB_AckPacket             ; ...and send the ACK


._PB_NakPacket: ; OR the subtype w/ TC_NAK and send them an empty packet back
      ld    a,(_PB_Header+subtype)
      or    TC_NAK
      ld    (_PB_Header+subtype),a
      clr   hl
      ld    (_PB_Header+length),hl
      jr    ._PB_Reply

._PB_AckPacket: ; OR the subtype w/ TC_ACK and send the packet back
      ld    a,(_PB_Header+subtype)
      or    TC_ACK
      ld    (_PB_Header+subtype),a

._PB_Reply: ; the reply is in the header/buffer/footer buffers - send it!
      ld    hl,._PB_ModeTX
      ld    (_PB_Mode),hl        ; move to the TX mode
      ld    hl,._PB_SendHeaderChecksum
      ld    (_PB_State),hl       ; after sending the header, build and send the
                                 ; header_checksum
      ld    hl,TC_HEADER_SIZE-2  ; 2 for the header_checksum
      ld    (_PB_length),hl
      ld    hl,_PB_Header
      ld    (_PB_ptr),hl         ; point at the buffer

      ld    c,TC_FRAMING_START   ; start with a START character
._PB_TXStart:
      call  ._PB_Write
      jr    nz,._PB_TXStart         ; loop to start the transmission

      clr   hl
      ld    (_PB_checksum),hl    ; start the checksum at zero

      ld    a,TC_FRAMING_ESC
      ld    (_PB_esc),a          ; init the ESC marker to non-escape-mode
      ret

._PB_ModeTX: ; the transmit mode - send the current buffer
      call  ._PB_Read               ; this is only HALF-DUPLEX, so flush any
                                    ; received character
      call  ._PB_CanTransmit
      ret   nz                   ; return if the transmitter is still busy -
                                 ; another INT will happen later
      ld    a,(_PB_esc)          ; is an escaped character pending?
      cp    TC_FRAMING_ESC
      jr    z,._PB_TXNoEsc

      ld    c,a
._PB_TXSendEscapedChar:
      call  ._PB_Write
      jr    nz,._PB_TXSendEscapedChar  ; loop while the escaped character is sent
      ld    a,TC_FRAMING_ESC
      ld    (_PB_esc),a          ; mark the next character as non-escaped
      jr    ._PB_TXFinishCharacter  ; finishes the character that was started
                                    ; last int
._PB_TXNoEsc: ; no escaped character was pending - get a character to transmit
      ld    hl,(_PB_ptr)
      ld    a,(hl)               ; get the next character to send...
      inc   hl
      ld    (_PB_ptr),hl            ; increment the pointer and store it again

      ld    hl,(_PB_checksum)    ; add the new character to the running checksum
      call  _PB_Checksum
      ld    (_PB_checksum),hl    ; save the new checksum

      cp    TC_FRAMING_ESC
      jr    z,._PB_TXEsc            ; it's an ESC character - escape it
      cp    TC_FRAMING_START
      jr    z,._PB_TXEsc            ; it's a START character - escape it

      ld    c,a
._PB_TXSendChar:
      call  ._PB_Write
      jr    nz,._PB_TXSendChar      ; loop while sending the data character

._PB_TXFinishCharacter:
      ld    hl,(_PB_length)      ; get the length remaining
      dec   hl
      ld    (_PB_length),hl      ; store length-1
      ld    a,h
      or    l
      ret   nz                  ; return if the length is still >0

      ld    hl,(_PB_State)
      jp    (hl)                ; a section is done - jump to the proper handler

._PB_TXEsc: ; escape the character
      xor   0x20
      ld    (_PB_esc),a          ; save the escaped character for next time
      ld    c,TC_FRAMING_ESC
._PB_TXSendEscChar:
      call  ._PB_Write
      jr    nz,._PB_TXSendEscChar   ; loop while sending the ESC char
      ret   ; all done for now...

._PB_SendHeaderChecksum: ; send the header_checksum, from the current
                         ; running-checksum
      ld    hl,(_PB_checksum)
      ld    (_PB_Header+header_checksum),hl  ; store the header_checksum
      ld    hl,2
      ld    (_PB_length),hl      ; store the length of the header_checksum
      ld    hl,._PB_SendBody
      ld    (_PB_State),hl
      ret

._PB_SendBody: ; send the body of the packet, if any
      ld    hl,(_PB_Header+length)
      ld    a,h
      or    l
      jr    z,._PB_SendFooter   ; is the length 0? if so, skip to the footer
      ld    (_PB_length),hl     ; store the length of the body to send
      ld    hl,_PB_Buffer
      ld    (_PB_ptr),hl        ; point at the data-buffer, for the body-portion
      ld    hl,._PB_SendFooter
      ld    (_PB_State),hl
      ret

._PB_SendFooter: ; generate the checksum out of the running-checksum,
                 ; and send the footer
      ld    hl,(_PB_checksum)    ; get the running-checksum
      ld    (_PB_Footer+checksum),hl   ; ...and store it in the footer
      ld    hl,_PB_Footer
      ld    (_PB_ptr),hl         ; send out of the footer
      ld    hl,TC_FOOTER_SIZE
      ld    (_PB_length),hl      ; set the length of the footer
      ld    hl,._PB_SendDone
      ld    (_PB_State),hl       ; when finished, move to the DONE state
      ret

._PB_SendDone: ; all done sending! reset everything and move back to the beginning!
      xor   a
      ld    (_PB_waittx),a       ; mark that the TX has finished
      call  _PB_Init
      ret

_PB_getDoublerSetting::
	; this function is a modified duplicate of the function _getDoublerSetting
	; found in CPUPARAM.LIB starting with DynC 7.25.  It was copied here since
	; the BIOS normally receives a macro called _CPU_ID_ containing the CPU type
	; and revision number, but the pilot BIOS reads that information itself.
	xor	a							; clear carry flag
	ld		a, (PB_DIV19200)

	ld		de, 0x0100				; Rabbit 3000 CPU ID
	ld		hl, (_PB_CPUID)		; target's CPU ID
	sbc	hl, de					; is target's CPU ID >= Rabbit 3000 CPU ID?
	jr		nc, ._PB_gds_R3000	; yes, go get Rabbit 3000 doubler setting

	; Rabbit 2000-specific section
._PB_gds_R2000:
	; Rabbit 2000 products automatically have the clock doubler
	;  disabled if the base oscillator is more than 12.9024 MHz.
	ld		hl, 7					; 20 nS low time setting
	cp		13						; is base oscillator 7.3728 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 20 nS low time setting
	ld		L, 4					; 14 nS low time setting
	cp		19						; is base oscillator 11.0592 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 14 nS low time setting
	ld		L, 2					; 10 nS low time setting
	cp		22						; is base oscillator 12.9024 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 10 nS low time setting
	ld		L, 0					; disabled clock doubler setting
	jr		._PB_gds_done		; > 12.9024 MHz, go return disabled doubler setting

	; Rabbit 3000-specific section
._PB_gds_R3000:
	; Rabbit 3000 products automatically have the clock doubler
	;  disabled if the base oscillator is more than 25.8048 MHz.
	ld		hl, 15				; 20 nS low time setting
	cp		7						; is base oscillator 3.6864 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 20 nS low time setting
	ld		L, 10					; 15 nS low time setting
	cp		25						; is base oscillator 14.7456 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 15 nS low time setting
	ld		L, 4					; 9 nS low time setting
	cp		31						; is base oscillator 18.432 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 9 nS low time setting
	ld		L, 2					; 7 nS low time setting
	cp		37						; is base oscillator 22.1184 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 7 nS low time setting
	ld		L, 1					; 6 nS low time setting
	cp		43						; is base oscillator 25.8048 MHz or lower?
	jr		c, ._PB_gds_done	; yes, go return 6 nS low time setting
	ld		L, 0					; disabled clock doubler setting
									; > 25.8048 MHz, return disabled doubler setting
._PB_gds_done:
	ret

#endasm
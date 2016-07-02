/*************************************************************
Pilot BIOS source code

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

#undef MECR_VALUE

//  MECR_VALUE must be 0x80
#define MECR_VALUE 0x80
#define MSB_OFFSET (((MECR_VALUE >> 5) + 1) & 7)

#undef MSB_BIT

#if MSB_OFFSET == 0
   #define MSB_BIT 18
#elif MSB_OFFSET == 1
   #define MSB_BIT 19
#elif MSB_OFFSET == 2
   #define MSB_BIT 20
#elif MSB_OFFSET == 3
   #define MSB_BIT 21
#elif MSB_OFFSET == 4
   #define MSB_BIT 22
#elif MSB_OFFSET == 5
   #define MSB_BIT 23
#endif

#undef _FLASH_SIZE_
#define _FLASH_SIZE_  _cexpr(0x0040 << MSB_OFFSET)

#define RAM_CS_TO_USE 0x45 // 2 ws, /OE1/WE1/CS1

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
char MB0CRShadow,MB1CRShadow,MB2CRShadow,MB3CRShadow;

// identification of the hardware
int   PB_RAM_SIZE;
int   PB_FLASH_ID;
char  PB_DIV19200;
unsigned long  _PB_CPUID;

root int _PB_Init() ;  // initialization for the Pilot BIOS finite state machine
root _PB_WriteMem();
root _PB_ReadProgPort();
root _PB_WriteProgPort();
root _PB_SetIntVecTab();
root int _PB_getcrc(char *Data, char count, int accum);

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

// flags to queue the changing of the BAUD rate after a SETBAUDRATE packet
char  _PB_setbaudrate;     // true if a baud rate change has been queued
char  _PB_newdivider;      // the new divider to set the baud rate too
char  _PB_waittx;          // true if we are waiting for a TX to finish before
                           // changing the baud rate

// Fast RAM compile mode variables. Unfortunately, the pilot BIOS must be aware
// of fast RAM(s) memory mapping conventions for booting directly into fast RAM
// execution mode when the application is compiled with SUPPRESS_FAST_RAM_COPY
// defined TRUE (nonzero). Note that Rabbit boards may be equipped with up to
// two fast RAM devices.
char  fastram1_mb2cr_setting;
char  fastram2_mb3cr_setting;
long  fastram1_size;
long  fastram2_size;

int   _PB_coldload_count;     // this is X * (oscillator speed / 7200)
                              // depending on the clock doubler setting, X may
                              // be 2 or 4
int   _PB_max_error_count;    // used during baud rate tolerance checking

#asm root nodebug
premain::
main::
      // This version of the PILOT bios is Rabbit 4000 specific.
      ld    a, 0x80
ioi   ld    (MMIDR), a
      ld    a, 0xC0
ioi   ld    (EDMR), a

      ;  initialize stack pointer prior to doing any calls
      ;  though the stack pointer was set by the cold loader, it's best just to
      ;  set it to a value you can trust
      ld    hl, sysStack+sizeof(sysStack)
      ld    sp, hl

      ; Note that the oscillator speed / 7200 was calculated by the cold loader
      ; and stored at address 0.

      ; Get the 19200 baud divider, which is sent back to Dynamic C in order
      ; to set the macro _FREQ_DIV_19200_.  The calculation is:
      ; ((count * 3) + 128) / 256
      ld    hl, (0)
      ld    de, hl
      add   hl, de
      add   hl, de
      ld    de, 128
      add   hl, de
      ld    a, h         ;divide by 256
      ld    (PB_DIV19200), a

      ; If possible, the clock doubler will be enabled for the duration of the
      ; pilot in order to improve the resolution of timer A.  The doubler will
      ; be disabled prior to starting the BIOS.
      ld    hl, (0)
      call  _PB_getDoublerSetting
      ld    hl, (0)
      cp    a, 0     ;will the doubler be off?  if so, don't double count value
      jr    z, _PB_set_doubler
      add   hl, hl   ;double the count value to account for the doubler
_PB_set_doubler:
ioi   ld    (GCDR), a
      ; For the duration of the pilot, run timer A off of pclk.  Combined with
      ; the clock doubler, this gives four times the normal resolution for
      ; timer A, ideally allowing higher baud rates with unconventional
      ; oscillator frequencies.
      xor   a
ioi   ld    (TAPR), a
      add   hl, hl   ;double the count value to account for timer A using pclk
      ld    (_PB_coldload_count), hl
      ;adjust TAT4R for the new timer rate.  We still want 57600 baud for now.
      ;The calculation is: (count + 128) / 256
      ld    de, 128
      add   hl, de
      ld    a, h  ;divide by 256
      dec   a     ;put divider - 1 into timer scaling register
ioi   ld    (TAT4R), a

      ;calculate the max allowable error (this is the same for all baud rates)
      ;refer to _PB_LookupBaudRateDivider for an explanation of the process
      ld    jk, 0
      ld    hl, (_PB_coldload_count)
      srl   4, jkhl
      srl   1, jkhl
      ld    de, hl      ;DE = count/32
      srl   1, jkhl
      add   hl, de      ;HL = count/32 + count/64
      ld    de, hl
      ld    hl, (_PB_coldload_count)
      ld    L, h
      ld    h, 0
      add   hl, de      ;HL = count/32 + count/64 + count/256
      ld    (_PB_max_error_count), hl


;**** START TEST RAM SIZE ***********
;  Dynamic C needs to know how much RAM is available
      ld    a, 0x80         ;4 meg quadrants
ioi   ld    (MECR), a

//******* Also, internal RAM detection *********
// If there's an internal RAM (CS3), we're running in it now.
// But if there's an external RAM too, that's the one to measure
// (new configurations might change this in the future). So test
// for external RAM on CS1WE1, and set MB0 to use it if it's there.
//
// Specifically, this test was originally added to handle the RCM5700.
      // *** CS1WE1 test ***
.test_CS1WE1:
      ld    a, 0x05
      call  _PB_GetRamSize
      ld    a, c           ; size result (in 64 K blocks) is in the C register
      or    a
      jr    nz, ._PB_SaveRAMsize

      ; Use current MB0CR value (CS3WE0) for argument to _PB_GetRamSize
ioi   ld    a, (MB0CR)
      call  _PB_GetRamSize

._PB_SaveRAMsize:
      clr   hl
      ld    L, c           ; size result (in 64 K blocks) is in the C register
      add   hl, hl         ; pilot BIOS's reported RAM size is in 32K blocks
      ld    (PB_RAM_SIZE), hl

      //********* Look for fast RAM chips *********
      // The fast RAM search attempts to find up to two fast RAM devices in the
      // following order of chip select/enable: CS0WE0, CS0WE1, CS2WE1, CS2WE0.
      ld    jkhl, 0        ; initialize fast RAM sizes to zero
      ld    a, L           ; initialize fast RAM MBxCR settings to zero
      ld    (fastram1_size), jkhl
      ld    (fastram2_size), jkhl
      ld    (fastram1_mb2cr_setting), a
      ld    (fastram2_mb3cr_setting), a

      // *** CS0WE0 TEST ***
.test_CS0WE0:
      ld    a, 0x00        ; test CS0WE0 at 4 wait states
      call  _PB_GetRamSize
      ld    a, c           ; size result (as 64 K blocks) is in the C register
      or    a
      jr    z, .test_CS0WE1

      ld    a, 0xC0        ; set up to run in CS0WE0 at 0 wait states
      call  .set_mbXcr

      // *** CS0WE1 TEST ***
.test_CS0WE1:
      ld    a, 0x04        ; test CS0WE1 at 4 wait states
      call  _PB_GetRamSize
      ld    a, c           ; size result (as 64 K blocks) is in the C register
      or    a
      jr    z, .test_CS2WE1

      ld    a, 0xC4        ; set up to run in CS0WE1 at 0 wait states
      call  .set_mbXcr

      // *** CS2WE1 TEST ***
.test_CS2WE1:
      ld    a, 0x06        ; test CS2WE1 at 4 wait states
      call  _PB_GetRamSize
      ld    a, c           ; size result (as 64 K blocks) is in the C register
      or    a
      jr    z, .test_CS2WE0

      ld    a, 0xC6        ; set up to run in CS2WE1 at 0 wait states
      call  .set_mbXcr

      // *** CS2WE0 TEST ***
.test_CS2WE0:
      ld    a, 0x02        ; test CS2WE0 at 4 wait states
      call  _PB_GetRamSize
      ld    a, c           ; size result (as 64 K blocks) is in the C register
      or    a
      jr    z, .test_end

      ld    a, 0xC2        ; set up to run in CS2WE0 at 0 wait states
      call  .set_mbXcr
      jr    .test_end

      // Local subroutine to save either first or second fast RAM information.
      // (Note: Does not return after saving second fast RAM information.)
      // On entry, A has the MBxCR setting and C has the number of 64K blocks.
.set_mbXcr:
      ; convert 64K blocks in C into bytes in BCDE
      ld    b, 0
      ld    de, 0
      ; if already found first fast RAM then go save second fast RAM information
      ld    jkhl, (fastram1_size)
      test  jkhl
      jr    nz, .set_mb3cr

      ld    (fastram1_size), bcde
      ld    (fastram1_mb2cr_setting), a
      ret

.set_mb3cr:
      ld    (fastram2_size), bcde
      ld    (fastram2_mb3cr_setting), a
      ; remove the stacked return address (does not return)
      add   sp, 2

.test_end:
;  **** END TEST RAM SIZE ********

      ipset 1
._PB_SetProgPort:
      ;  comm port A
      ;  parallel port c, bit 6,7 use alternate pin assignment
      clr   hl
      ld    (flashWriteCount), hl

      ld    a,_PB_PCFRVAL
ioi   ld    (PCFR),a

      ld    a,0xc3         ;  jump instruction
      ld    (_PBINTVEC_BASE+_PB_INT_VECTOR),a
      ld    hl,._PB_SerialISR
      ld    (_PBINTVEC_BASE+_PB_INT_VECTOR+1),hl

      ;  at port c, async, 8-bit, interrupt priority 1
      ld    a, 0x01
ioi   ld    (_PB_SxCR),a

._PB_GetCPUID:
ioi   ld    a, (GROM)         ; read CPU identification parameters
      and   0x1F              ; (formatted for unsigned long read by compiler)
      ld    (_PB_CPUID+2), a
ioi   ld    a, (GRAM)
      and   0x1F
      ld    (_PB_CPUID+3), a
ioi   ld    a, (GREV)
      and   0x1F
      ld    (_PB_CPUID+0), a
ioi   ld    a, (GCPU)
      and   0x1F
      ld    (_PB_CPUID+1), a

      ;; mark program flash initialization not yet done (this is a safety
      ;;  feature in case future pilot BIOS changes introduce a path where
      ;;  _readIDBlock gets called before _InitFlashDriver is called)
      clr   hl
      ld    (_InitFlashDriverOK), hl

      ld    bcde, 0
      ld    bc, 0x0004 << MSB_OFFSET
      push  bcde
      push  bcde
      call  _InitFlashDriver
      add   sp, 8
      ;The return value of _InitFlashDriver is not checked.  Even if an error
      ;occurred during init, we still want to allow the pilot to continue
      ;so that we can perform a compile to RAM for debugging.

      ld    hl, (_flash_info + _FlashIdEntry + id)
      ld    (PB_FLASH_ID), hl
      ld    hl, 0x04          ; Only check the MB2 quadrant. When we get 8meg
                              ; flashes, this will have to change.
      call  _readIDBlock      ;return value in HL
      bool  hl
      jr    z, ._PB_idBlockOk

._PB_idBlockBad:           ; clear SysIDBlock struct if not on flash or in RAM
      ld    hl, SysIDBlock
      ld    b, SysIDBlock+marker+6-SysIDBlock
      xor   a
.blockEraseLoop:
      ld    (hl), a
      inc   hl
      djnz  .blockEraseLoop

._PB_idBlockOk:
._PB_SetComm:
      call  _PB_InitBaudRateChange
      call  ._PB_InitTC
      call  _PB_SetIntVecTab
      ipset 0
_PB_Loop::
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
      ld    hl, (pz)
      push  py
      push  pz
      call  _ProgramFlashUnit
      pop   pz
      pop   py
      ld    py, py+1
      ld    pz, pz+1
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
; The resulting xpc will be in a, and the offset will be in hl.
_PB_PhysicalToLogical::
      push  bc       ; save the address for later
      push  de

      ld    a,0x0f
      and   c
      ld    c,a      ; c = c & 0x0f
      ld    a,0xf0
      and   d        ; a = d & 0xf0
      or    c        ; a = c | d
      rlca           ; transpose the two nibbles
      rlca
      rlca
      rlca           ; a = xpc + 0xe
      sub   0xE      ; a = xpc

      ld    hl,0x0fff
      and   hl,de
      ex    de,hl
      ld    hl,0xe000
      add   hl,de

      pop   de
      pop   bc       ; restore the address
      ret

;//****** START IN RAM - just jump to the beginning again
_PB_StartRegBiosRAM::
      ld    a, 0x01     ;restore Timer A to running off of pclk/2
ioi   ld    (TAPR), a
      xor   a           ;disable clock doubler
ioi   ld    (GCDR), a
      jp    BEGREGBIOS

;//****** START IN FLASH - Code to fix the MBxCR mappings, and jump to 0x0000

_PB_StartRegBiosFLASH::
      ld    a', 0x00
      jr    _PB_StartRegBios

_PB_StartRegBiosFASTRAM::
      ld    a', (fastram1_mb2cr_setting)

_PB_StartRegBios:
      ipset 3
      ld    a, 0x01     ;restore Timer A to running off of pclk/2
ioi   ld    (TAPR), a
      xor   a           ;disable clock doubler
ioi   ld    (GCDR), a
ioi   ld    a, (MB0CR)
ioi   ld    (MB1CR), a
      ld    hl, 0x3FE   ;start of MB1 given 4 meg quadrants
ioi   ld    (DATASEGL), hl ; After this, we're running from MB1CR
      ex    af, af'
ioi   ld    (MB0CR), a     ; Not pulling the rug, because we're in MB1CR
      ; Pilot bios download performance relies on a wait-first strategy for the
      ; flash writing functionality, so that the pilot can continue buffering
      ; input while the flash write is completing. So, ensure flash writing is
      ; complete before jumping into the regular BIOS in flash.
      ld    hl, _cexpr(BEGREGBIOS)  ; get the application's BIOS start address
.wloop:
      ld    a, (hl)        ; get an initial flash toggle-bit status value
      cp    (hl)           ; did the toggle-bit change?
      jr    neq, .wloop    ; if yes (incomplete write), go check again . . .

      cp    (hl)           ; did the toggle bit change? (SST work around)
      jr    neq, .wloop    ; if yes (incomplete write), go check again . . .

      jp    (hl)           ; run the application's BIOS (in parallel flash)

; ****** Lookup the Baud Rate divider
; Looks for the given baud rate in the baud rate table, then uses the
; shifter value from that table to calculate the required timer A
; divider for achieving that baud rate.  Also checks if the error from
; using that divider will exceed 5% (typical tolerance for serial);
; if so, the baud rate is unacceptable.
;
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
      ld    jkhl, (ix)
      ld    bcde, (iy)
      cp    jkhl, bcde
      ld    hl, iy

      jr    z, ._PB_FoundTableEntry

      ld    de, 5                 ; de == size of one table entry
      add   hl, de
      jr    ._PB_LookupLoop      ; try the next entry

._PB_FoundTableEntry:            ; iy has the table entry!
      ;to do the timer value calculation we want to go:
      ;
      ;(clock speed + 16 * baud) / (32 * baud) - 1
      ;
      ;note that the 16*baud/32*baud (0.5) is for rounding
      ;
      ;we have a cleverly-devised count value from the cold loader
      ;we also have a shifter value from the baud table
      ;the equation becomes a much cheaper:
      ;
      ;(count + 1<<shifter)>>(shifter + 1) - 1
      ;
      ld    a, (iy+4)               ; get the shifter value
      ld    b, a
      ld    de, 1             ;make DE = (1<<shifter)
._PB_BaudFirstShift:
      rl    de                ;a rotate is fine since the top bits are 0
      djnz  ._PB_BaudFirstShift
      ld    jkhl, 0        ;clear jk
      ld    hl, (_PB_coldload_count)
      add   hl, de         ;HL = (count + 1<<shifter)
      inc   a              ;A = (shifter + 1)
      ld    b, a
._PB_BaudSecondShift:
      srl   1, jkhl           ;we can't safely use a rotate here
      djnz  ._PB_BaudSecondShift
      ;at this point, L contains the value we want for doing error evaluation
      ;calculate the implied clock speed based on this 8-bit result
      ;we will use this for error checking
      ;error checking works as follows:
      ;
      ;diff = abs[(TAT4R + 1)<<(shifter + 1) - count]
      ;max_error = count>>5 + count>>6 + count>>8
      ;if (diff > max_error) fail
      ;
      ;Note that 1/32 + 1/64 + 1/256 is close to 1/20, which is 5%.
      ;All you're doing is seeing if the serial port will be running
      ;within the acceptable 5% tolerance range.  Also note that max_error
      ;was calculated at the beginning of the pilot, as it is constant.
      ;
      ld    de, hl      ;save HL for later (L has divider + 1)
      ld    h, 0        ;only L should be nonzero at the start of the shift
      ld    b, a        ;A still contains (shifter + 1)
._PB_BaudCheckValueShift:
      sll   1, jkhl
      djnz  ._PB_BaudCheckValueShift
      ;HL now contains the implied count value
      ld    b, d     ;save HL's old value (we'll need DE); there is no ld BC, DE
      ld    c, e
      ld    de, (_PB_coldload_count)
      cp    hl, de      ;cp implied count, actual count
      jr    gt, ._PB_BaudFindDiff     ;check if implied count < actual count
      ex    de, hl      ;since implied count < actual count, swap them first
._PB_BaudFindDiff:
      sub   hl, de
      ;difference is in HL
      ld    de, (_PB_max_error_count)
      cp    hl, de      ;cp difference in counts, max allowable difference
      jr    lt, ._PB_BaudRateOk
      ld    d, 0     ;set z (this will indicate that the baud rate is bad)
      and   d
      ret
._PB_BaudRateOk:
      ld    hl, bc   ;BC has HL's old value
      dec   hl       ;L = (count + 1<<shifter)>>(shifter+1) - 1
      ld    d, 0xff  ;set nz (this will indicate that the baud rate is ok)
      or    d        ;
      ld    a, L     ;A is the return value
      ret            ;return value in A

_PB_BaudTable::
;        baud rate, in hex     part of the maths (the shifter)
;        (little endian!)      (shifter value can't be zero)
;        --------------------  -------------------------
      db    0x00,0xE1,0x00,0x00,  7          ;57600
      db    0x00,0xC2,0x01,0x00,  8          ;115200
      db    0x00,0x84,0x03,0x00,  9          ;230400
      db    0x00,0x08,0x07,0x00,  10         ;460800
_PB_EndBaudTable::
      db    0x00,0x00,0x00,0x00,  0x00


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
      ld    a,xpc
      push  af                ; save the xpc

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
      ; 1 means fast ram

ioi   ld    hl, (MB2CR)
      ex    jk', hl     ; Save settings.
      ld    a, (fastram1_mb2cr_setting)
      ld    l, a
      ld    a, (fastram2_mb3cr_setting)
      ld    h, a
ioi   ld    (MB2CR), hl
      // The fast RAM test will have already configured the MB2CR and MB3CR for
      // fast RAM write if valid.
      ld    b, 0              ; mask flag
      // Now check what address range is needed.
      ld    jkhl, (fastram1_size)
      cp    jkhl, bcde
      jr    c, next_range
      jr    z, next_range

      ld    jkhl, 0
      ld    jk, 0x80          ; Base address = 800000
      add   jkhl, bcde
      jr    ._PB_WRITEFastRam

next_range:
      ld    jkhl, (fastram2_size)
      cp    jkhl, bcde
      jp    c, ._PB_NakPacket
      jp    z, ._PB_NakPacket

      ld    jkhl, 0
      ld    jk, 0xC0
      add   jkhl, bcde

._PB_WRITEFastRam:
      ld    py, jkhl          ; py has destination
      ld    hl, ix
      ld    bcde, 7
      add   hl, de
      ldl   px, hl            ; px has source
      ld    hl, (ix+1)
      ld    bc, hl            ; bc has the length of the data
      copy

      ex    jk', hl     ; Restore settings.
ioi   ld    (MB2CR), hl
      jr    ._PB_WRITEAck     ; ack the packet

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
      ld    (flashWriteCount), hl

      ldl   pz, _PB_Buffer + 7 ; The first 7 bytes are subtype, size, address.

._PB_WRITEAck:
      clr   hl
      ld    (_PB_Header+length),hl  ; no body to the ACK packet

      pop   af
      ld    xpc,a             ; restore the xpc
      jp    ._PB_AckPacket    ; reply as an ACK

._PB_WRITENak:
      pop   af
      ld    xpc,a
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
      ld    a,(PB_RAM_SIZE)   ; get the RAM size
      ld    (ix),a            ; ...and store it in the packet
      inc   ix                ; move to the div19200 field
      xor   a
      ld    (ix),a            ; ...and store it in the packet
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
      cp    TC_STARTBIOS_FASTRAM
      jp    z,_PB_StartRegBiosFASTRAM  ; should we run in FAST RAM?
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

//gets GCDR setting based on clock speed
_PB_getDoublerSetting::
      ;go through the table and find which entry is appropriate
      ld    de, hl
      ld    ix, _PB_clock_doubler_table
      ld    bc, 3
._PB_DoublerSearch:
      ld    hl, (ix)
      cp    hl, de      ;compare the table entry with the parameter
      jr    gt, ._PB_DoublerFoundEntry  ;if table entry > parameter, you're done
      add   ix, bc   ;IX += 3
      test  hl       ;did HL read the last entry (which is zero)?
      jr    nz, ._PB_DoublerSearch
      xor   a        ;don't turn the doubler on
      ret
._PB_DoublerFoundEntry:
      ld    a, (ix+2)
      ret

_PB_clock_doubler_table::
      ;first two bytes are clock speed, third is GCDR setting
      ;all entries have some minor error (0x20) added since some
      ;are common oscillator speeds.  If the timing measurement
      ;is just slightly high we still want to use the correct
      ;GCDR setting, and adding the error covers this.
      ;This table is based on the settings from _getDoublerSetting
      ;in cpuparam.lib.  Adjust or expand as deemed necessary.
      ;     Clock speed | GCDR setting
      db    0x20, 0x04,       0x0F     ;7.3728 MHz
      db    0x20, 0x06,       0x0D     ;11.0592 MHz
      db    0x20, 0x09,       0x09     ;16.5888 MHz
      db    0x20, 0x0B,       0x06     ;20.2752 MHz
      db    0x20, 0x10,       0x03     ;29.4912 MHz
      db    0xCA, 0x1C,       0x01     ;52.8384 MHz
      db    0x20, 0x26,       0x12     ;70.0416 MHz
_PB_clock_doubler_table_end:
      db    0x00, 0x00,       0x00     ;end of table


_PB_GetRamSize::            ; Returns size in 64K units, max. 4 MB (64x64K)
ioi   ld    hl, (MB2CR)
      push  hl
      ld    h, a
      ld    L, a
ioi   ld    (MB2CR), hl

      ld    c, 1           ; start testing at offset 0x0ffff
      ld    px, _cexpr(0x00080000<<MSB_OFFSET)
      ld    a, (px-1)
      ld    L, a
      ex    jk', hl ; Original high byte stored in k'
      ld    py, 0x00010000 + (0x00040000<<MSB_OFFSET)
      ipset 3
.loop:
      ld    a, (py-1) ; Current test address
      ld    b, a

// This curious piece of code and the push pop
// below prevent a false positive by assuring
// that the test will fail if nothing is
// connected to the CS/OE being tested.
      pop    iy          ; the stack is guaranteed to be
      push   iy          ; mapped

      ld    a, (px-1)   ; Hi address.
      cp    a, b
      jr    nz, .not_it

      cpl               ; Flip bits, write and read to confirm
      ld    (px-1), a
      ld    b, a

      pop   iy
      push  iy

      ld    a, (py-1)
      cp    a, b
      cpl
      ld    (py-1),a    ; restore original byte
      jr    z, .done

.not_it:
      ld    a, c
      inc   a
      ;; Stop checking size at just over 4 MBytes.
      ;; (This enforces a 4 MByte maximum supported RAM size.)
      cp    65          ; 65x64K == 4M+64K
      jr    z, .errdone

      ld    jkhl, py
      ld    bcde, 0
      inc   c           ; add 64K

      add   jkhl, bcde
      ld    py, jkhl

      ld    c, a
      jr    .loop

.errdone:
      ld    c, 0
.done:
      ex    jk', hl
      ld    a, L
      ld    (px-1), a
      pop   hl
ioi   ld    (MB2CR), hl
      ipres
      ret

_PB_EndOfpilot_c::
#endasm
;   Copyright (c) 2016 Digi International Inc.
;
;   This Source Code Form is subject to the terms of the Mozilla Public
;   License, v. 2.0. If a copy of the MPL was not distributed with this
;   file, You can obtain one at http://mozilla.org/MPL/2.0/.
;**************************************************************
;   PIC boot loader for RCM4200
;
;   Uses clocked serial to send initial loader
;   if SMODE pin isn't detected high on GP0. Otherwise,
;   just sleeps.
;
;   Brian Murtha
;
;**************************************************************

;**** TEST MODE *********************************
; 0 = Production mode, load secondary Rabbit loader code
; 1 = Load toggle PB2 & PB3 program to memory & run
;************************************************
#define TESTMODE 0

;************************************************
; Define memory ctrl pin bits & WS to use for MB0CR
;  High nibble = C -> 0 WS
;  High nibble = 8 -> 1 WS
;  High nibble = 4 -> 2 WS
;  High nibble = 0 -> 4 WS
;  Low nibble  = 0 -> CS0/OE0
;  Low nibble  = 5 -> CS1/OE1
;  Low nibble  = 6 -> CS2/OE1
;************************************************
#define CSOEWS 0x80   ; Use CS0, 1 WS

#include p10f202.inc

;  Disable Watch dog and code protection, 
;    make GP3 master reset (MCLR)
   __CONFIG _MCLRE_ON & _CP_OFF & _WDT_OFF

;************************************************
; Macro for 2 cycle, one instruction nop to save code space
TwoCycleNOP macro
    local localLab
     goto localLab
localLab
  endm

;************************************************
; Macro to write a Rabbit I/O byte
;   LOWADDR = I/O addr.
;   VALUE = value to write
IO_WR macro LOWADDR,VALUE
    movlw   0x80            ; High address always 0x80
    movwf	transmitByte
    call    SendByte
    movlw   LOWADDR
    movwf	transmitByte
    call    SendByte
    movlw   VALUE
    movwf	transmitByte
    call    SendByte
  endm

;**** Variables **************
  cblock 0x18
    Delay	        ; File register for delay
    BitCount        ; Bit loop counter
    ByteCount       ; Byte loop counter
    dbOnCount       ; Debounce on count
    dbOffCount      ; Debounce off count
    transmitByte    ; Byte to be transmitted
  endc

  org 0x0   		; Reset vector is at 0x0000
  goto Main         ; Locate callable routines 1rst
                    ; because CALL takes 1 byte arg.

;*****************************************************************
;  This is a trick to access a table of constant bytes by offset.
;
;  Reg W should have the offset into the table before calling.
;
;  0x8nm is the opcode for retlw nm, which loads the byte nm
;  into W and returns to the address placed on the stack by call
;  to BLPROG.
;
;  Use utility or Dynamic C with hex dump and stupid editor
;  tricks to create table from serial flash loader binary image
;  by appending each byte with 0x8 and adding commas.
;
;  An 0xFF byte terminates the table, so FF must not be used
;  in initial loader program!
;
;*****************************************************************
BLPROG                ; Start of table
      addwf	PCL,f     ; Add contents of W to program counter

#if TESTMODE==1
    ;**********************************************
    ; Test program to load to memory. Hits WDT
    ; and toggles PB2 & PB3 continuously.

    ; ld a,0xcc  ioi ld (PBDDR),a    ; address 0 here
	dw 0x83E,0x8CC
    dw 0x8D3,0x832,0x847,0x800

    ; ld a,0x5a  ioi ld (WTDCR),a
	dw 0x83E,0x85A
	dw 0x8D3,0x832,0x808,0x800

	; ld a,0xc0  ioi ld (PBDR),a
	dw 0x83E,0x8C8
	dw 0x8D3,0x832,0x840,0x800

    ; Delay Loop
    dw 0x801,0x899,0x899,0x80B,0x878,0x87F,0x8B1,0x820,0x8FA

    ; ipset 3
	dw 0x8ED,0x85E

	; ld a,0xc8  ioi ld (PBDR),a
	dw 0x83E,0x8C4
	dw 0x8D3,0x832,0x840,0x800

    ; Delay Loop
    dw 0x801,0x899,0x899,0x80B,0x878,0x87F,0x8B1,0x820,0x8FA

	; jp  6e00,  where we just copied the loader to
    dw 0x8c3,0x800,0x86e

    dw 0x8ff  ; Table terminator
#endif

#if TESTMODE==0 ;****** Production loader code image from PIC_resident_loader.c ******************
 dw 0x83E,0x880,0x8D3,0x832,0x810,0x800

 dw 0x8AF,0x8D3,0x832,0x8AB,0x800

 dw 0x8D3,0x832,0x854,0x800,0x8D3,0x832,0x865,0x800,0x83C,0x8D3,0x832,0x847,0x800,0x8D3,0x832,0x8A0
 dw 0x800,0x83E,0x804,0x8D3,0x832,0x819,0x800,0x817,0x8D3,0x832,0x800,0x800,0x817,0x8D3,0x832,0x851
 dw 0x800,0x8D3,0x832,0x855,0x800,0x83E,0x840,0x8D3,0x832,0x80A,0x800,0x83E,0x803,0x8D3,0x832,0x80F
 dw 0x800

 dw 0x8DD,0x821,0x808,0x802         ; ld ix,0x208     // Adjustments for board revs
 dw 0x8D3,0x8DD,0x836,0x800,0x830   ; ioi ld (ix),0x0020

 dw 0x821,0x8D5,0x800,0x83E,0x818,0x8D3,0x877,0x81F,0x82B,0x8D3,0x877,0x82B,0x83E,0x8C0,0x8D3
 dw 0x832,0x820,0x804,0x87F,0x845,0x8D3,0x8DD,0x8CB,0x800,0x8EE,0x811,0x8D1,0x800,0x83E,0x8E8,0x810
 dw 0x8F4,0x806,0x808,0x8D3,0x8CB,0x856,0x820,0x8FB,0x8D3,0x812,0x8AF,0x810,0x8F6,0x8D3,0x8CB,0x856
 dw 0x820,0x8FB,0x83E,0x8F2,0x8ED,0x867,0x8D3,0x83A,0x8D0,0x800,0x806,0x808,0x811,0x800,0x8E0,0x83E
 dw 0x84C,0x8D3,0x832,0x8D4,0x800,0x8D3,0x8CB,0x87E,0x828,0x8FB,0x8D3,0x83A,0x8D0,0x800,0x812,0x813
 dw 0x87A,0x87F,0x8B3,0x820,0x8EA,0x8ED,0x877,0x83C,0x83C,0x8ED,0x867,0x810,0x8DF,0x8D3,0x8DD,0x8CB
 dw 0x800,0x8AE,0x8EB,0x8E9,0x8FF
#endif

;***************************************************************
; Subroutine to send the byte in transmitByte file register
;
;  This is full speed SPI. To make it slower, add equal numbers
;  of NOPs or TwoCyclNOP's to DELAY SPOT 1 and DELAY SPOT 2,
;  or call a delay routine there.
;
;  If program gets to be larger than 256 - sizeof(this routine),
;  then this routine can be changed to a macro and inlined
;***************************************************************
SendByte
    movlw   8
    movwf   BitCount
    bcf     GPIO,2              ; Make sure CLK is low
    bcf	    GPIO,1              ; Start data low
    bcf     STATUS,0            ; Clear carry flag
BitLoop

    ;--- DELAY SPOT 1

    rrf     transmitByte,1      ; Rotate LSB to carry flag
    btfss   STATUS,0            ; Skip next instr. if flag not set
    goto    bitlow
    nop                         ; Makes cycles = if we skip goto
    bsf     GPIO,1              ; Pull data high
    bcf     GPIO,2              ; Make sure CLK is low
    goto    CLKHigh
bitlow
    bcf     GPIO,1              ; Pull data low
    bcf     GPIO,2              ; Make sure CLK is low
    TwoCycleNOP                 ; 2 cycles to make same as goto
CLKHigh

    ;--- DELAY SPOT 2

    TwoCycleNOP                 ; Keep hi&lo CLK cycles same len.
    TwoCycleNOP
    TwoCycleNOP
    bsf     GPIO,2              ; Make sure CLK is low
    decfsz	BitCount,1
    goto    BitLoop
    TwoCycleNOP
    TwoCycleNOP

    ;--- DELAY SPOT 3
    nop
    bcf	    GPIO,1              ; Start pins low
    bcf     GPIO,2              ; Make sure CLK is low

    retlw   0

;**** Begin program ***********
Main
    andlw	0xFE				; Clear Fosc/4 output enable.
    movwf	OSCCAL              ;   Must always do this first

    bcf		GPIO,2              ; Make sure CLK is low

    movlw	b'11000000'			; Disable disable pull-ups and 
    OPTION                      ;   disable wakeup on pin change. 
    movlw	b'11111111'			; Configure all pin as input
    tris	GPIO				; 

    movwf	Delay               ; Reuse w value from tris load

    bcf		GPIO,1              ; Start Data low

StartDelayLoop 					; Slight delay for thing s to settle 
    decfsz	Delay,f            
    goto	StartDelayLoop

;**** Debounce SMODE pin in case of transients
;  Just get two identical reads 1 ms apart and sleep if low,
;  do bootload if high
Debounce
    clrf	dbOnCount
    clrf	dbOffCount
    btfss	GPIO,0
    incfsz	dbOnCount,f
    btfsc	GPIO,0
    incfsz	dbOffCount,f
Delay1mS
    movlw	.71 		; delay ~1000uS
    movwf	Delay
    decfsz	Delay,f		; this loop does 215 cycles
    goto	$-1
    decfsz	Delay,f		; This loop does 786 cycles
    goto	$-1

    btfss	GPIO,0
    incfsz	dbOnCount,f
    btfsc	GPIO,0
    incfsz	dbOffCount,f
    movf	dbOnCount,w
    xorlw	0x02
    btfsc	STATUS,2     ; Skip next instr. if 0 flag clear
    goto	DoBootLoad   ; Do boot load if it was on twice
    movf	dbOffCount,w
    xorlw	0x02
    btfss	STATUS,2     ; Skip next instr. if 0 flag set
    goto	Debounce     ; Try again if not on or off 2 times

_Sleep
    movlw	b'11111111'	 ; Configure all pins as input
    tris	GPIO		
    movf	GPIO,0       ; Read pins before sleep. Really needed 
    sleep                ;  only if we're waking up on pin changes

DoBootLoad
    movlw	b'11111001'	 ; Configure pins 1 & 2 as output
    tris	GPIO		

   ;********************************************
   ; Map Fast RAM to quadrant 0
   IO_WR  0x14, CSOEWS ; MB0CR
   ; Map Fast RAM to quadrant 1
   IO_WR  0x15, CSOEWS ; MB1CR
   ; Map Fast RAM to quadrant 2
   IO_WR  0x16, CSOEWS ; MB2CR
   ; Map Fast RAM to quadrant 3
   IO_WR  0x17, CSOEWS ; MB3CR

   ; Map 6000 to FF000, D000 to C0000
   IO_WR  0x13, 0xd6   ; SEGSIZE
   IO_WR  0x12, 0xF9   ; DATASEG

   ; Disable secondary WDT
   IO_WR  0x0C, 0x5a   ; SWDTR
   IO_WR  0x0C, 0x52   ; SWDTR
   IO_WR  0x0C, 0x44   ; SWDTR

    ;clrf   ByteCount   ; Init. count to 0
    clrw
    movwf   ByteCount    ; Init. count to 0


;**** Send code bytes using triplets
ProgramByteLoop
    movlw   0x6e
    movwf   transmitByte        ; 6E is always high addr. byte
    call    SendByte            ; Send high address byte
    movf    ByteCount,0         ; Put index in W
	movwf	transmitByte        ; Index is low address byte
    call    SendByte            ; Send low address byte
    movf    ByteCount,0         ; Put index in W
	call	BLPROG              ; Returns with code byte in W
	movwf	transmitByte        ; Load code byte to W
    call    SendByte            ; send code byte
    incf	ByteCount,f	        ; Increment byte count
    movf    ByteCount,0         ; Put index in W
	call	BLPROG              ; returns with code byte in W
    xorlw   0xFF                ; Test for end byte = FF
    btfsc   STATUS,2            ; Skip next instr. if 0 flag clear
    goto    EndBootStrap
    goto    ProgramByteLoop

;**** Send command to Rabbit to end triplet mode ****
EndBootStrap
    clrw
    movwf   transmitByte    ; 0 is high addr. byte
    call    SendByte
    clrw
    movwf   transmitByte    ; 0 is low addr. byte
    call    SendByte
    movlw   0xc3            ; C3 is jp op code
    movwf   transmitByte
    call    SendByte

    clrw
    movwf   transmitByte    ; 0 is high addr. byte
    call    SendByte
    movlw   0x01
    movwf   transmitByte
    call    SendByte
    clrw                    ; 0 is low jp addr.
    movwf   transmitByte
    call    SendByte

    clrw
    movwf   transmitByte   ; 0 is high addr. byte
    call    SendByte
    movlw   0x02
    movwf   transmitByte
    call    SendByte
    movlw   0x6e           ; 6E is high jp addr.
    movwf   transmitByte
    call    SendByte

    ; Writing 0x80 to SPCR ends bootstrap mode
    IO_WR  0x24, 0x80     
    goto    _Sleep
 end

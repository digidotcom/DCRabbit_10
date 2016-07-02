/************************************************

 This is the PIC-resident loader for the
 RCM4200

 This code should be DW'ed into the PIC program
 as 12 bit words where each has a high nibble of
 8 appended in front of it.

 Sorry, but there's no utility to convert this into a
 12 bit code table for the PIC code, but it's easy enough
 to do:

 1) compile this in a normal DC environment.
 2) get the address of main using a flyover watch expression
 3) use the hex memory dump window to dump at the main addr.
 4) copy the hex dump from the ld a,0x80 (3E 80) to the
    terminating FF byte.
 5) paste into a new editor window
 6) use column mode (Shift-Alt) to clean off ASCII on right
 7) use global search and replace to replace spaces
    between dump bytes with. ", 0x8"
 8) add "dw 0x8" to the byte at the begining of each row.
 9) copy and past table into the correct location in the
    PIC *.ASM code.

 10) Build the PIC code using the Bootload01.MCP project
     file if using the free MPLAB from MicroChip,
     which also has the needed include file.
 11) Load the generated HEX file using the Pic Kit 2 or
     other PIC programming utility.

 It's important that there be no absolute jumps in the
 code. It's important that the total length be < 253
 bytes to fit properly in the PIC10F202 code.
 This code must be adapted if the CS pin for serial
 differs.

*************************************************/

#define SDIVISOR 0x00
#define CSENABLE  asm  ioi set 5,(ix)
#define CSDISABLE  asm ioi res 5,(ix)

#define SF_SPI_CONTROL_VALUE 0x0C
#define SF_SPI_TXMASK 0x80
#define SF_SPI_RXMASK 0x40
#ifndef NAPCR
 #define NAPCR 0x0208
#endif

main(){
 ;;
#asm
      //**** Init serial Flash CS pin
      ld 	a,0x80
ioi   ld 	(MMIDR),a

      xor    a          ; A = 0, carry flag cleared
ioi   ld    (TAT5R),a   ; 0 for SCLCK divisor
ioi   ld    (PCDCR),a   ; Set TXB to push-pull
ioi   ld    (PDFR),a

      inc   a           ; A = 0x01
ioi   ld    (PBDDR),a   ; Set PB0 to output for SCLCK
ioi   ld    (TACSR),a   ; Start clock

    	ld		a, 0x04     ; Set OE0 to toggle 1/2 clock early
ioi  	ld		(MTCR), a
      rla               ; A = 0x08
ioi	ld		(GCSR), a   ; Normal oscillator, processor and peri.
                        ;  from main clock, no periodic interrupt

      rla               ; A = 0x10
ioi   ld    (PCDDR),a   ; Set TXB to output
ioi   ld    (PCFR),a    ; Set PC4 to TXB

      ld    a, 0x40
ioi	ld		(GCM0R),a   ; Enable spectrum spreader

    	ld		a, 0x03
ioi  	ld		(GCDR), a

      ld 	ix,NAPCR
ioi   ld    (ix),0x30    ; intitial state needed for RCM4300

      ld    hl,SBER

      //**** Initialize clocked serial port
      ld    a, 0x18     ; Reverse bits
ioi   ld    (hl),  a    ; HL = SBER
      rra               ; A = 0xC = SF_SPI_CONTROL_VALUE
      dec   hl          ; HL = SBCR
ioi   ld    (hl),  a
      dec   hl          ; HL = SBSR

      ld    a,0xc0
ioi   ld    (EDMR),a
      ld    b,L         ; B = 0xD3

delayLoop:              ;** baud CLK needs delay
      CSENABLE
      ld    de, SBAR
		ld		a, 0xe8		; Continuous read command
      djnz  delayLoop

      ld    b,0x08      ; command + 3 addr bytes + 4 don't care

SPIWrite:
; wait for the buffer to be available
Tx00:
ioi   bit   2,(hl)      ; test trans bit
		jr		nz, Tx00 	; jump if not done yet

ioi	ld		(de), a	   ; send it
      xor   a           ; zeros for address and don't care bits
		djnz	SPIWrite    ; jump if not done

Tx01:
ioi   bit   2,(hl)      ; test trans bit
		jr		nz, Tx01

      //**** Receive and copy the whole thing *********

      //**** Intitialize outer loop
      ld    a,0xf2
      ld    xpc,a

ioi   ld		a,(SBDR)	  	; Clear receive buffer
      ld    b,8         ; 8 x 0x2000 = 64K
OuterLoop:
      ld    de,0xe000
InnerLoop:
	   ld		a, SF_SPI_CONTROL_VALUE | SF_SPI_RXMASK
ioi   ld		(SBCR), a  	; Load RX control val. - rec. byte

Rx0Wait :               ; Wait for the receiver to complete
ioi   bit   7,(hl)      ; Test receiver bit
		jr		z, Rx0Wait  ; Jump if not done yet
ioi	ld		a,(SBDR)	  	; Get the byte

      ld 	(de),a      ; Load to memory
      inc   de
      ld    a,d
      or    e           ; See if DE rolled over
      jr  	nz,InnerLoop
      ld    a,xpc
      inc   a
      inc   a
      ld    xpc,a       ; xpc for next 0x2000 chunk
      djnz  OuterLoop

      CSDISABLE

      ex    de,hl       ; DE = 0
      jp    (hl)        ; Jump to address 0000 to run

      db 0xff  ; End of prog. terminator PIC uses

      ; Make end of prog. easy to spot in hex dump
      db "<<<<---- THE END IS HERE !!!                   "

#endasm
}
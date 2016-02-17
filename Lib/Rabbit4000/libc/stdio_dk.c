/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	STDIO's interface to the debug kernel
*/

/*** BeginHeader */
#ifdef STDIO_PRINTF_DEBUG
	#define _stdio_dk_debug __debug
#else
	#define _stdio_dk_debug __nodebug
#endif
/*** EndHeader */

/*** BeginHeader kbhit, _stream_stdin_read, _stream_stdout_write */
int kbhit();
size_t _stream_stdin_read( void __far *cookie, void __far *buffer, size_t bytes);
size_t _stream_stdout_write( void __far *cookie, const void __far *buffer,
																						size_t bytes);
/*** EndHeader */

#ifdef STDIO_DEBUG_SERIAL
// parameters for debug output of STDIO to a serial port -- to use this
// feature, see the example program in SAMPLES/STDIO_SERIAL.C .

#if (STDIO_DEBUG_SERIAL == SADR)
	#define	SDS_SxDR					SADR
	#define	SDS_SxSR					SASR
	#define	SDS_TATxR				TAT4R
	#define	SDS_TATxRShadow		TAT4RShadow
   #define	SDS_PxFR					PCFR
   #define	SDS_PxFRShadow			PCFRShadow
	#define	SDS_PxFRMask			0x40
#endif
#if (STDIO_DEBUG_SERIAL == SBDR)
 #if (RCM4300_SERIES)
   #fatal "Serial Port B not available for debug output on RCM43x0 modules."
 #endif
	#define	SDS_SxDR					SBDR
	#define	SDS_SxSR					SBSR
	#define	SDS_TATxR				TAT5R
	#define	SDS_TATxRShadow		TAT5RShadow
   #define	SDS_PxFR					PCFR
   #define	SDS_PxFRShadow			PCFRShadow
	#define	SDS_PxFRMask			0x10
#endif
#if (STDIO_DEBUG_SERIAL == SCDR)
	#define	SDS_SxDR					SCDR
	#define	SDS_SxSR					SCSR
	#define	SDS_TATxR				TAT6R
	#define	SDS_TATxRShadow		TAT6RShadow
   #define	SDS_PxFR					PCFR
   #define	SDS_PxFRShadow			PCFRShadow
	#define	SDS_PxFRMask			0x04
#endif
#if (STDIO_DEBUG_SERIAL == SDDR)
	#define	SDS_SxDR					SDDR
	#define	SDS_SxSR					SDSR
	#define	SDS_TATxR				TAT7R
	#define	SDS_TATxRShadow		TAT7RShadow
	#if BL4S100_SERIES
      #define  SDS_PxFR             PCFR
      #define  SDS_PxFRShadow       PCFRShadow
      #define  SDS_ALTREG           PCALR
      #define  SDS_ALTShadow        PCALRShadow
      #define  SDS_PxFRMask         0x55
      #define  SDS_ALTVAL           0x00
	#else
      #define  SDS_PxFR             PCFR
      #define  SDS_PxFRShadow       PCFRShadow
      #define  SDS_PxFRMask         0x01
   #endif
#endif

#if (STDIO_DEBUG_SERIAL == SEDR)
   #define  SDS_SxDR             SEDR
   #define  SDS_SxSR             SESR
   #define  SDS_TATxR            TAT2R
   #define  SDS_TATxRShadow      TAT2RShadow
	#if BLXS200_SERIES
      #define  SDS_PxFR             PDFR
      #define  SDS_PxFRShadow       PDFRShadow
      #define  SDS_ALTREG           PDAHR
      #define  SDS_ALTShadow        PDAHRShadow
	#else
      #define  SDS_PxFR             PCFR
      #define  SDS_PxFRShadow       PCFRShadow
      #define  SDS_ALTREG           PCAHR
      #define  SDS_ALTShadow        PCAHRShadow
   #endif
   #define  SDS_ALTVAL           0x30
   #define  SDS_PxFRMask         0x40
   #define  SDS_AndDDR           0x7F
   #define  SDS_OrDDR            0x40
#endif

#if (STDIO_DEBUG_SERIAL == SFDR)
   #define  SDS_SxDR             SFDR
   #define  SDS_SxSR             SFSR
   #define  SDS_TATxR            TAT3R
   #define  SDS_TATxRShadow      TAT3RShadow
	#if BLXS200_SERIES
      #define  SDS_PxFR             PDFR
      #define  SDS_PxFRShadow       PDFRShadow
      #define  SDS_ALTREG           PDALR
      #define  SDS_ALTShadow        PDALRShadow
	#elif BL4S100_SERIES
      #define  SDS_PxFR             PEFR
      #define  SDS_PxFRShadow       PEFRShadow
      #define  SDS_ALTREG           PEALR
      #define  SDS_ALTShadow        PEALRShadow
	#else
      #define  SDS_PxFR             PCFR
      #define  SDS_PxFRShadow       PCFRShadow
      #define  SDS_ALTREG           PCALR
      #define  SDS_ALTShadow        PCALRShadow
   #endif
   #define  SDS_ALTVAL           0x30
   #define  SDS_PxFRMask         0x04
   #define  SDS_AndDDR           0xF7
   #define  SDS_OrDDR            0x04
#endif

#ifndef SDS_SxSR
#fatal "Unknown value for STDIO_DEBUG_SERIAL, must be SxDR where x = A,B,...,F."
#endif

#ifndef STDIO_DEBUG_BAUD
#fatal "If STDIO_DEBUG_SERIAL enabled, must set STDIO_DEBUG_BAUD as well."
#endif

#funcchain _GLOBAL_INIT		_stdioSerialInit

__xmem _stdio_dk_debug
void _stdioSerialInit()
{
	unsigned long	divisor;

#ifndef STDIO_DEBUG_FORCEDSERIAL
	if (!(OPMODE & 0x08)) // init serial port if NOT in debug mode
#endif	//STDIO_DEBUG_FORCEDSERIAL
	{
	#if (STDIO_DEBUG_SERIAL == SADR)
	 		// disable interrupts (enforce polled I/O) for serial port A
			WrPortI(SACR, &SACRShadow, SACRShadow & 0xFC);
	#endif

	// Calculate the TIMER A divisor.  Round to the closest integer.
	divisor = (freq_divider * 19200L + ((STDIO_DEBUG_BAUD + 1) / 2))
				/ STDIO_DEBUG_BAUD - 1L;
   WrPortI(SDS_TATxR, &SDS_TATxRShadow, (char) divisor);
	WrPortI(SDS_PxFR, &SDS_PxFRShadow, SDS_PxFRShadow | SDS_PxFRMask);

	#if (BLXS200_SERIES && _BOARD_TYPE_ == RCM4010)
	      // Switch jumperless port routing for BL4S210 SBC serial port
	      WrPortI(SPCR,  &SPCRShadow,  0x8C);
	      WrPortI(PEFR,  &PEFRShadow,  (PEFRShadow | 1));
	      WrPortI(IB0CR, &IB0CRShadow, 0x48);
	      WrPortE(0x30, NULL, 0x4C);
	#endif

	#ifdef SDS_ALTREG
			WrPortI(SDS_ALTREG, &SDS_ALTShadow, SDS_ALTVAL);
	#endif

	#if BL4S100_SERIES
   	#if STDIO_DEBUG_SERIAL == SDDR
   		// set PC0/2/4/6 as TX
         WrPortI(PCDDR, &PCDDRShadow, SDS_PxFRMask);
      	WrPortI(SDS_PxFR, &SDS_PxFRShadow, SDS_PxFRMask);
   	#endif
		#if (STDIO_DEBUG_SERIAL == SFDR)
      	WrPortI(SDS_PxFR, &SDS_PxFRShadow, SDS_PxFRMask);
         WrPortI(SFCR, &SFCRShadow, SFCRShadow | 0x20);		// use PE3 for Rx
      #endif
	#endif

   #if BLXS200_SERIES && \
      ((STDIO_DEBUG_SERIAL == SEDR) || (STDIO_DEBUG_SERIAL == SFDR))
         WrPortI(PDDDR, &PDDDRShadow, (PDDDRShadow & SDS_AndDDR) | SDS_OrDDR);
         WrPortI(SDS_PxFR, &SDS_PxFRShadow, SDS_PxFRMask);
      	#if STDIO_DEBUG_SERIAL == SEDR
         	WrPortI(SECR, &SECRShadow, SECRShadow | 0x10);
      	#elif STDIO_DEBUG_SERIAL == SFDR
         	WrPortI(SFCR, &SFCRShadow, SFCRShadow | 0x10);
      	#endif
   #endif


   // Don't assemble characters during break (i.e., no cable attached)
	// Always for Port A; only if either RS232_NOCHARASSYINBRK or the appropriate
	//  RS232_NOCHARASSYINBRK_A..F is defined for others.
   #if STDIO_DEBUG_SERIAL == SADR
      BitWrPortI(SAER, &SAERShadow, 1, 1);
   #endif

   #if STDIO_DEBUG_SERIAL == SBDR \
       && (defined RS232_NOCHARASSYINBRK || defined RS232_NOCHARASSYINBRK_B)
      BitWrPortI(SBER, &SBERShadow, 1, 1);
   #elif STDIO_DEBUG_SERIAL == SCDR \
         && (defined RS232_NOCHARASSYINBRK || defined RS232_NOCHARASSYINBRK_C)
      BitWrPortI(SCER, &SCERShadow, 1, 1);
   #elif STDIO_DEBUG_SERIAL == SDDR \
         && (defined RS232_NOCHARASSYINBRK || defined RS232_NOCHARASSYINBRK_D)
      BitWrPortI(SDER, &SDERShadow, 1, 1);
   #elif STDIO_DEBUG_SERIAL == SEDR \
         && (defined RS232_NOCHARASSYINBRK || defined RS232_NOCHARASSYINBRK_E)
      BitWrPortI(SEER, &SEERShadow, 1, 1);
   #elif STDIO_DEBUG_SERIAL == SFDR \
         && (defined RS232_NOCHARASSYINBRK || defined RS232_NOCHARASSYINBRK_F)
      BitWrPortI(SFER, &SFERShadow, 1, 1);
   #endif

	}	// serial port initialization close brace

}

#endif	// defined STDIO_DEBUG_SERIAL



/* START FUNCTION DESCRIPTION ********************************************
kbhit                      <STDIO.LIB>

SYNTAX: int kbhit();

KEYWORDS: keyboard, input, stdio

DESCRIPTION:  Returns non-zero if there are any characters available on stdin.

Like printf, this function is only useful for debugging in the
Dynamic C IDE. This function will always return 0 if the the Dynamic C
stdio window is not already open and focused. Any call to printf opens
the stdio window. The user should make sure only one process calls this
function at a time.

RETURN VALUE: See description.
END DESCRIPTION **********************************************************/
/* START _FUNCTION DESCRIPTION ********************************************
_kbhit                      <STDIO.LIB>

SYNTAX: int _kbhit();

KEYWORDS: keyboard, input, stdio

DESCRIPTION:  Returns non-zero if a key on the PC has been hit while
the stdio window in the Dynamic C IDE has the focus. Returns zero
otherwise

Unlike kbhit(), this function ONLY checks the debug kernel for characters.

Like printf, this function is only useful for debugging in the
Dynamic C IDE. This function will always return 0 if the the Dynamic C
stdio window is not already open and focused. Any call to printf opens
the stdio window. The user should make sure only one process calls this
function at a time.

RETURN VALUE: See description.
END DESCRIPTION **********************************************************/

_stdio_dk_debug
int _kbhit()
{
#ifdef STDIO_DEBUG_SERIAL
	#ifndef STDIO_DEBUG_FORCEDSERIAL
		if (! (OPMODE & 0x08))
	#endif	//STDIO_DEBUG_FORCEDSERIAL
		{
			// check STDIO serial port status register for available byte
			return (BitRdPortI( SDS_SxSR, 7));
		}
#endif
	return dkCharReady;
}

_stdio_dk_debug
int kbhit()
{
	// first see if there are any characters in the unget buffer
	return (_stdio_files[0].unget_idx || _kbhit());
}



_stdio_dk_debug
size_t _stream_stdin_read( void __far *cookie, void __far *buffer, size_t bytes)
{
#ifdef STDIO_DEBUG_SERIAL
	#if BLXS200_SERIES && STDIO_DEBUG_SERIAL == SEDR
		static int serEStartup = 1;
	#endif
#endif

	// If 0 bytes requested, return 0 bytes read.
	if (! bytes)
	{
		return 0;
	}

	// Wait for a key in the buffer -- stdin blocks and never returns EOF
	while (! _kbhit())
	{
		#asm
			rst 0x28				; allow debug kernel to stop execution inside loop
		#endasm
	}

#ifdef STDIO_DEBUG_SERIAL
	#ifndef STDIO_DEBUG_FORCEDSERIAL
		if (!(OPMODE & 0x08))
	#endif	//STDIO_DEBUG_FORCEDSERIAL
		{
		#if BLXS200_SERIES && STDIO_DEBUG_SERIAL == SEDR
			if (serEStartup)
			{
				serEStartup = 0;        // one-time FIFO cleanup
				// Remove chars from PC7 (programming cable) left on FIFO when
				// switch to PD7 was made.
				while (BitRdPortI( SESR, 7))
				{
					RdPortI( SEDR);
				}
			}
		#endif
			// DEVNOTE: Because of kbhit() test above, no need to check SDS_SxSR
			*(char __far *)buffer = RdPortI( SDS_SxDR);

			return 1;
		}
#endif

	// code for getting a character from Dynamic C (debug kernel)
	dkGetCharFromStdio();
	*(char __far *)buffer = dkCharData;

	return 1;
}


_stdio_dk_debug
size_t _stream_stdout_write( void __far *cookie, const void __far *buffer,
																						size_t bytes)
{
	int saveix;
	// hand data off to debug kernel
	// maximum of TC_SYSBUF_SIZE-TC_HEADER_RESERVE bytes at a time
	// This is a modified version of dkSendStdio and _stdioSerialOut.

	#asm
		ld		(sp+@SP+saveix), ix
		ld		a, (OPMODE)
		and	0x08
#ifndef STDIO_DEBUG_SERIAL
		jr		z, .exit					; skip the debug kernel code if in run mode
#else
	#ifndef STDIO_DEBUG_FORCEDSERIAL
		jr		nz, .dkDoMsg			; use the debug kernel for stdout
	#endif
		ld		hl, (sp+@sp+bytes)
		test	hl
		jr		z, .exit
		ld		bc, hl					; number of bytes to send
		ld		px, (sp+@sp+buffer)	; source of bytes to send

.waitforclear:
ioi	ld		a, (SDS_SxSR)
		bit	3, a
		jr		nz, .waitforclear

		ld		a, (px)
	#ifdef STDIO_DEBUG_ADDCR
		cp		'\n'
		jr		nz, .notNL
		ld		a, '\r'
ioi	ld		(SDS_SxDR), a
.sentCR:
ioi	ld		a, (SDS_SxSR)
		bit	3, a
		jr		nz, .sentCR
		ld		a, '\n'		; reload newline
.notNL:
	#endif		// STDIO_DEBUG_ADDCR defined
ioi	ld		(SDS_SxDR), a
		ld		px, px+1
		dwjnz	.waitforclear
.done:
		jr		.exit
#endif		// STDIO_DEBUG_SERIAL defined

.dkDoMsg:
#if DK_ENABLE_DEBUGKERNEL
	call	dkProlog

__dk_sschecklock:
	ld		hl,dkLocks
.getlock:
	bit	DKF_STDIO_LOCK,(hl)
	set   DKF_STDIO_LOCK,(hl)
	jr		nz, .getlock

__dk_ssgotlock:
	ld		hl,(sp+@SP+bytes)			; make sure 0 < hl <= max TC payload
	test	hl
	jr		z, __dk_ssdone
	ld		de,TC_SYSBUF_SIZE-TC_HEADER_RESERVE
	cp		hl,de
	jr		ltu, .sizeok
	ld		hl,de
	ld		(sp+@SP+bytes),hl			; update number of bytes actually sent
.sizeok:
	ld		bc,hl
	ld		d,TC_TYPE_DEBUG			; d has packet type
	ld		e,TC_DEBUG_STDIO			; e has packet subtype
	ld		px,(sp+@SP+buffer)		; load address of buffer to send

	call	dkBuildResponseF			; send packet (px=buffer, bc=length, de=type)

	ld		a,(dkStatusFlags)			; get status flags
	bit	DKF_STAT_INWATCH,a		; check if watch is executing
	jr		z,__dk_ssnotinwatch		; jump if not executing watch expression
   ld		hl,dkLocks					; this is a watch, clear stdio lock
   res	DKF_STDIO_LOCK,(hl)
	jr		__dk_ssdone

__dk_ssnotinwatch:
	call	dkSetStdioTimer

__dk_sswaitforlock:
	call	dkPollSerialPort
	call	dkCheckStdioTimeOut
	bool	hl
	jr		nz,__dk_ssdone
	ld		hl,dkLocks
	bit	DKF_STDIO_LOCK,(hl)
	jr		nz,__dk_sswaitforlock

__dk_ssdone:
	call	dkEpilog
	jr		.exit

#endif		// DK_ENABLE_DEBUG_KERNEL
.exit:
	ld		ix, (sp+@SP+saveix)
	#endasm

	return bytes;
}



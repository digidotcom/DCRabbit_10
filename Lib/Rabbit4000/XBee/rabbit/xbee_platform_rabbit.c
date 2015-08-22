/*
	Copyright (c)2010 Digi International Inc., All Rights Reserved

	This software contains proprietary and confidential information of Digi
	International Inc.  By accepting transfer of this copy, Recipient agrees
	to retain this software in confidence, to prevent disclosure to others,
	and to make no use of this software other than that for which it was
	delivered.  This is a published copyrighted work of Digi International
	Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
	prohibited.

	Restricted Rights Legend

	Use, duplication, or disclosure by the Government is subject to
	restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
	Technical Data and Computer Software clause at DFARS 252.227-7031 or
	subparagraphs (c)(1) and (2) of the Commercial Computer Software -
	Restricted Rights at 48 CFR 52.227-19, as applicable.

	Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
*/
/**
	@addtogroup hal_rabbit
	@{
	@file xbee_platform_rabbit.c
	Platform-specific functions for use by the XBee Driver on
	Rabbit/Dynamic C platform.
*/

/*** BeginHeader */
#include "xbee/platform.h"
#use "jslong.c"
/*** EndHeader */

/*** BeginHeader xbee_seconds_timer */
#define xbee_seconds_timer() (SEC_TIMER + 0)
/*** EndHeader */
__nodebug
uint32_t (xbee_seconds_timer)( void)
{
	return SEC_TIMER;
}

/*** BeginHeader xbee_millisecond_timer */
#define xbee_millisecond_timer() (MS_TIMER + 0)
/*** EndHeader */
__nodebug
uint32_t (xbee_millisecond_timer)( void)
{
	return MS_TIMER;
}

/*** BeginHeader _xbee_checksum_inline */
uint8_t _xbee_checksum_inline( const void __far *bytes, uint_fast8_t length,
	uint_fast8_t initial);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION *******************************************
_xbee_checksum_inline                   <xbee_platform_rabbit.c>

SYNTAX:
   __nodebug uint8_t _xbee_checksum_inline( const void __far *bytes, 
                                            uint_fast8_t length, 
                                            uint_fast8_t initial)

DESCRIPTION:


**************************************************************************/
__nodebug
uint8_t _xbee_checksum_inline( const void __far *bytes, uint_fast8_t length,
	uint_fast8_t initial)
{
	#asm __nodebug
		; px already contains address of buffer
		ld		hl, (sp+@SP+length)
		ld		bc, hl
		ld		hl, (sp+@SP+initial)
		test	bc
		jr		z, .done			; no bytes to sum
		ld		a, L				; copy checksum from L to a

		; Loop through the bytes using BC as index.  Since index will run
		; from <count> down to 1 (instead of zero), need to decrement px
		; before looping.
		ld		px, px - 1
	.loop:
		ld		hl, (px + bc)
		sub	a, L
		dwjnz	.loop

		ld		L, a				; copy checksum back to L
	.done:
		ld		h, 0				; zero out top byte
		lret
	#endasm
}

/*** BeginHeader _swapcpy */
/*** EndHeader */
// documented in xbee/byteorder.h
void _swapcpy( void FAR *dst, const void FAR *src, uint_fast8_t bytes)
{
	/*
		Use PX as pointer into destination and PY as pointer into source.
		Increment PX after each read, and use PY+HL addressing on the source.
		HL starts with the number of bytes to copy; by pre-decrementing it
		we'll start with the last byte of the source to copy.
	*/
	#asm
		; px already contains dst
		ld		py, (sp+@SP+src)
		ld		hl, (sp+@SP+bytes)
		test	hl
		jr		z, .done
		ld		bc, hl
	.loop:
		dec	hl								; move to next byte of source
		ld		a, (py + hl)
		ld		(px), a
		ld		px, px + 1					; move to next byte of destination
		dwjnz .loop
	.done:
	#endasm
}


/*** BeginHeader */
#define xbee_readline(buffer, length) getsn_tick(buffer, length)
/*** EndHeader */



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
	@addtogroup util_byteorder
	@{
	@file swapbytes.c
	For platforms without swap32() and swap16() functions for swapping byte
	order for 32 and 16-bit values.

	@note Should be written in assembly for embedded targets.
*/

#include "xbee/platform.h"

#ifndef HAVE_SWAP_FUNCS

/* START FUNCTION DESCRIPTION ********************************************
swap32                                  <swapbytes.c>

SYNTAX:
   uint32_t swap32( uint32_t value)

DESCRIPTION:
     Swap the byte order of a 32-bit value.


PARAMETER1:  value - value to swap

RETURNS:  new 32-bit value with opposite endianness of <value>

**************************************************************************/
uint32_t swap32( uint32_t value)
{
	return  (value & 0x000000FF) << 24
			| (value & 0x0000FF00) << 8
			| (value & 0x00FF0000) >> 8
			| (value & 0xFF000000) >> 24;
}

/* START FUNCTION DESCRIPTION ********************************************
swap16                                  <swapbytes.c>

SYNTAX:
   uint16_t swap16( uint16_t value)

DESCRIPTION:
     Swap the byte order of a 16-bit value.


PARAMETER1:  value - value to swap

RETURNS:  new 16-bit value with opposite endianness of <value>

**************************************************************************/
uint16_t swap16( uint16_t value)
{
	return ((value & 0x00FF) << 8) | (value >> 8);
}

#endif /* HAVE_SWAP_FUNCS */


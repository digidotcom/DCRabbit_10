/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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


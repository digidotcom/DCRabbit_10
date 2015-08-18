/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup util_byteorder
	@{
	@file swapcpy.c
	Used in endian conversions for values larger than 32 bits.
*/

/*** BeginHeader _swapcpy */
/*** EndHeader */
#include "xbee/byteorder.h"
// documented in xbee/byteorder.h
void _swapcpy( void FAR *dst, const void FAR *src, uint_fast8_t bytes)
{
	if (bytes)
	{
		src = (const uint8_t FAR *)src + bytes;
		do
		{
			src = (const uint8_t FAR *)src - 1;
			*(uint8_t FAR *)dst = *(const uint8_t FAR *)src;
			dst = (uint8_t FAR *)dst + 1;
		} while (--bytes);
	}
}



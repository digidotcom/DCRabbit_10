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



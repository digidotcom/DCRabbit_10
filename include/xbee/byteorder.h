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
/* START FUNCTION DESCRIPTION ********************************************
swap16                                  <byteorder.h>

MACRO SYNTAX:
     swap16( uint16_t value)

DESCRIPTION:
     Convert a uint16_t in big-endian byte order to host byte order.
























     <dst> in host byte order (equivalent to memcpy() on big-endian
     platforms).



     <dst_le> in little-endian byte order (always swaps byte order).



     in big-endian byte order (equivalent to memcpy() on big-endian
     platforms).



     in little-endian byte order (equivalent to memcpy() on little-endian
     platforms).



     <dst_be> in big-endian byte order (always swaps byte order).



     <dst> in host byte order (equivalent to memcpy() on little-endian
     platforms).

**************************************************************************/

#ifndef __XBEE_ENDIAN_H
#define __XBEE_ENDIAN_H
	#include <string.h>			// for memcpy

	// xbee/platform will load the platform's endian.h or at least define
	// the macros LITTLE_ENDIAN, BIG_ENDIAN and BYTE_ORDER.
	#include "xbee/platform.h"

	// On DOS, swap16() and swap32() are already defined as macros, so don't
	// define them here.
	#ifndef swap16
		uint16_t swap16( uint16_t value);
	#endif
	#ifndef swap32
		uint32_t swap32( uint32_t value);
	#endif

/* START _FUNCTION DESCRIPTION *******************************************
_swapcpy                                <byteorder.h>

SYNTAX:
   void _swapcpy( void FAR *dst,  const void FAR *src,  uint_fast8_t count)

DESCRIPTION:

     Function similar to memcpy() but reverses byte order during copy.
     Copy <count> from <src> to <dst> while reversing the order.  Assumes
     that <src> and <dst> do not overlap.


PARAMETER1:  dst - destination buffer
PARAMETER2:  src - source buffer
PARAMETER3:  count - number of bytes to copy

**************************************************************************/
	void _swapcpy( void FAR *dst, const void FAR *src, uint_fast8_t count);

	#if BYTE_ORDER == LITTLE_ENDIAN
		#define memcpy_letoh( dst, src_le, count)	_f_memcpy( dst, src_le, count)
		#define memcpy_htole( dst_le, src, count)	_f_memcpy( dst_le, src, count)

		#define memcpy_betoh( dst, src_be, count)	_swapcpy( dst, src_be, count)
		#define memcpy_htobe( dst_be, src, count)	_swapcpy( dst_be, src, count)
	#else
		#define memcpy_letoh( dst, src_le, count)	_swapcpy( dst, src_le, count)
		#define memcpy_htole( dst_le, src, count)	_swapcpy( dst_le, src, count)

		#define memcpy_betoh( dst, src_be, count)	_f_memcpy( dst, src_be, count)
		#define memcpy_htobe( dst_be, src, count)	_f_memcpy( dst_be, src, count)
	#endif

	#define memcpy_betole( dst_le, src_be, count)	\
						_swapcpy( dst_le, src_be, count)
	#define memcpy_letobe( dst_be, src_le, count)	\
						_swapcpy( dst_be, src_le, count)

	// define byte-swapping macros if the platform hasn't already done so
	#ifndef htobe16
		#if BYTE_ORDER == LITTLE_ENDIAN
			// host to big-endian
			#define htobe16(x)	swap16(x)
			#define htobe32(x)	swap32(x)

			// big-endian to host
			#define be16toh(x)	swap16(x)
			#define be32toh(x)	swap32(x)

			// host to little-endian
			#define htole16(x)	(x)
			#define htole32(x)	(x)

			// little-endian to host
			#define le16toh(x)	(x)
			#define le32toh(x)	(x)
		#else
			// host to little-endian
			#define htole16(x)	swap16(x)
			#define htole32(x)	swap32(x)

			// little-endian to host
			#define le16toh(x)	swap16(x)
			#define le32toh(x)	swap32(x)

			// host to big-endian
			#define htobe16(x)	(x)
			#define htobe32(x)	(x)

			// big-endian to host
			#define be16toh(x)	(x)
			#define be32toh(x)	(x)
		#endif
	#endif

#endif

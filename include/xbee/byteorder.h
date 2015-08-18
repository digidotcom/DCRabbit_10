/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Rabbit processor is little endian.  These macros can be used in portable
	code to convert from host-byte-order to little-endian or big-endian byte
	order.
*/

#ifndef __ENDIAN_H
#define __ENDIAN_H

	#define LITTLE_ENDIAN	1234
	#define BIG_ENDIAN		4321
	#define PDP_ENDIAN		3412

	#define BYTE_ORDER		LITTLE_ENDIAN

	// intel() and intel16() are defined in math.lib

	// These functions aren't defined in the ANSI C or POSIX specifications,
	// and the names vary from platform to platform.	We define the most
	// commonly found names

	// Typically found on Linux
	#define bswap16(x)		intel16(x)
	#define bswap32(x)		intel(x)

	// Typically found on BSD
	#define swap16(x)			intel16(x)
	#define swap32(x)			intel(x)

	// Found on both Linux and BSD systems
	#define htole16(x)		(x)
	#define le16toh(x)		(x)
	#define htole32(x)		(x)
	#define le32toh(x)		(x)

	#define htobe16(x)		intel16(x)
	#define be16toh(x)		intel16(x)
	#define htobe32(x)		intel(x)
	#define be32toh(x)		intel(x)
#endif


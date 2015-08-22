/*
	inttypes.h

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

/*
	ANSI C99 7.8 Format conversion of integer types <inttypes.h>

	Note that Dynamic C does not support 64-bit integral values, so this file
	does not include macros for 64-bit wide types.
*/

#ifndef __INTTYPES_H
#define __INTTYPES_H

#include <stdint.h>

#if _DATA_MODEL_IS_FAR			// All far model
	#define _PTR_PREFIX	"l"
#else
	#define _PTR_PREFIX	""
#endif

// 7.8.1 Macros for format specifiers

// The fprintf macros for signed integers:
#define PRId8			"hhd"
#define PRIi8			"hhi"
#define PRIdLEAST8	"hhd"
#define PRIiLEAST8	"hhi"
#define PRIdFAST8		"hhd"
#define PRIiFAST8		"hhi"

#define PRId16			"d"
#define PRIi16			"i"
#define PRIdLEAST16	"d"
#define PRIiLEAST16	"i"
#define PRIdFAST16	"d"
#define PRIiFAST16	"i"

#define PRId32			"ld"
#define PRIi32			"li"
#define PRIdLEAST32	"ld"
#define PRIiLEAST32	"li"
#define PRIdFAST32	"ld"
#define PRIiFAST32	"li"

#define PRIdMAX		PRId32
#define PRIiMAX		PRIi32

#define PRIdPTR		_PTR_PREFIX "d"
#define PRIiPTR		_PTR_PREFIX "i"

// The fprintf macros for unsigned integers:
#define PRIo8			"hho"
#define PRIu8			"hhu"
#define PRIx8			"hhx"
#define PRIX8			"hhX"
#define PRIoLEAST8	"hho"
#define PRIuLEAST8	"hhu"
#define PRIxLEAST8	"hhx"
#define PRIXLEAST8	"hhX"
#define PRIoFAST8		"hho"
#define PRIuFAST8		"hhu"
#define PRIxFAST8		"hhx"
#define PRIXFAST8		"hhX"

#define PRIo16			"o"
#define PRIu16			"u"
#define PRIx16			"x"
#define PRIX16			"X"
#define PRIoLEAST16	"o"
#define PRIuLEAST16	"u"
#define PRIxLEAST16	"x"
#define PRIXLEAST16	"X"
#define PRIoFAST16	"o"
#define PRIuFAST16	"u"
#define PRIxFAST16	"x"
#define PRIXFAST16	"X"

#define PRIo32			"lo"
#define PRIu32			"lu"
#define PRIx32			"lx"
#define PRIX32			"lX"
#define PRIoLEAST32	"lo"
#define PRIuLEAST32	"lu"
#define PRIxLEAST32	"lx"
#define PRIXLEAST32	"lX"
#define PRIoFAST32	"lo"
#define PRIuFAST32	"lu"
#define PRIxFAST32	"lx"
#define PRIXFAST32	"lX"

#define PRIoMAX		PRIo32
#define PRIuMAX		PRIu32
#define PRIxMAX		PRIx32
#define PRIXMAX		PRIX32

#define PRIoPTR		_PTR_PREFIX "o"
#define PRIuPTR		_PTR_PREFIX "u"
#define PRIxPTR		_PTR_PREFIX "x"
#define PRIXPTR		_PTR_PREFIX "X"

// The fscanf macros for signed integers:
#define SCNd8			"hhd"
#define SCNi8			"hhi"
#define SCNdLEAST8	"hhd"
#define SCNiLEAST8	"hhi"
#define SCNdFAST8		"d"			// on Rabbit, fast8 is a 16-bit int
#define SCNiFAST8		"i"			// on Rabbit, fast8 is a 16-bit int

#define SCNd16			"d"
#define SCNi16			"i"
#define SCNdLEAST16	"d"
#define SCNiLEAST16	"i"
#define SCNdFAST16	"d"
#define SCNiFAST16	"i"

#define SCNd32			"ld"
#define SCNi32			"li"
#define SCNdLEAST32	"ld"
#define SCNiLEAST32	"li"
#define SCNdFAST32	"ld"
#define SCNiFAST32	"li"

#define SCNdMAX		SCNd32
#define SCNiMAX		SCNi32

#define SCNdPTR		_PTR_PREFIX "d"
#define SCNiPTR		_PTR_PREFIX "i"

// The fscanf macros for unsigned integers:
#define SCNo8			"hho"
#define SCNu8			"hhu"
#define SCNx8			"hhx"
#define SCNX8			"hhX"
#define SCNoLEAST8	"hho"
#define SCNuLEAST8	"hhu"
#define SCNxLEAST8	"hhx"
#define SCNXLEAST8	"hhX"
#define SCNoFAST8		"o"			// on Rabbit, fast8 is a 16-bit int
#define SCNuFAST8		"u"			// on Rabbit, fast8 is a 16-bit int
#define SCNxFAST8		"x"			// on Rabbit, fast8 is a 16-bit int
#define SCNXFAST8		"X"			// on Rabbit, fast8 is a 16-bit int

#define SCNo16			"o"
#define SCNu16			"u"
#define SCNx16			"x"
#define SCNX16			"X"
#define SCNoLEAST16	"o"
#define SCNuLEAST16	"u"
#define SCNxLEAST16	"x"
#define SCNXLEAST16	"X"
#define SCNoFAST16	"o"
#define SCNuFAST16	"u"
#define SCNxFAST16	"x"
#define SCNXFAST16	"X"

#define SCNo32			"lo"
#define SCNu32			"lu"
#define SCNx32			"lx"
#define SCNX32			"lX"
#define SCNoLEAST32	"lo"
#define SCNuLEAST32	"lu"
#define SCNxLEAST32	"lx"
#define SCNXLEAST32	"lX"
#define SCNoFAST32	"lo"
#define SCNuFAST32	"lu"
#define SCNxFAST32	"lx"
#define SCNXFAST32	"lX"

#define SCNoMAX		SCNo32
#define SCNuMAX		SCNu32
#define SCNxMAX		SCNx32
#define SCNXMAX		SCNX32

#define SCNoPTR		_PTR_PREFIX "o"
#define SCNuPTR		_PTR_PREFIX "u"
#define SCNxPTR		_PTR_PREFIX "x"
#define SCNXPTR		_PTR_PREFIX "X"

// 7.8.2 Functions for greatest-width integer types

#include <stdlib.h>			// for ldiv_t, ldiv(), labs(), strtol(), strtoul()

// 7.8.2.1 The imaxabs function
#define imaxabs			labs

// 7.8.2.2 The imaxdiv function
typedef ldiv_t imaxdiv_t;
#define imaxdiv			ldiv

// 7.8.2.3 The strtoimax and strtoumax functions
#define strtoimax			strtol
#define strtoumax			strtoul

// 7.8.2.4 The wcstoimax and wcstoumax functions

// Dynamic C does not suppor the wchar_t type, so these functions are undefined.


#endif
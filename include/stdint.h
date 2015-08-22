/*
	stdint.h

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
	ANSI C99 7.18 Integer types <stdint.h>

	Note that Dynamic C does not support 64-bit integral values, so this file
	does not include macros for 64-bit wide types.
*/

#ifndef __STDINT_H
#define __STDINT_H

// 7.18.1.1 Exact-width integer types
typedef signed char		int8_t;
typedef unsigned char	uint8_t;
typedef short				int16_t;
typedef unsigned short	uint16_t;
typedef long				int32_t;
typedef unsigned long	uint32_t;

// 7.18.2.1 Limits of exact-width integer types
#define INT8_MAX			127
#define INT8_MIN			-128
#define UINT8_MAX			255

#define INT16_MAX			32767
#define INT16_MIN			(-INT16_MAX-1)
#define UINT16_MAX		65535u

#define INT32_MAX			2147483647
#define INT32_MIN			(-INT32_MAX-1)
#define UINT32_MAX		4294967295ul


// 7.18.1.2 Minimum-width integer types
typedef int8_t				int_least8_t;
typedef uint8_t			uint_least8_t;
typedef int16_t			int_least16_t;
typedef uint16_t			uint_least16_t;
typedef int32_t			int_least32_t;
typedef uint32_t			uint_least32_t;

// 7.18.2.2 Limits of minimum-width integer types
#define INT_LEAST8_MAX		INT8_MAX
#define INT_LEAST8_MIN		INT8_MIN
#define UINT_LEAST8_MAX		UINT8_MAX
#define INT_LEAST16_MAX		INT16_MAX
#define INT_LEAST16_MIN		INT16_MIN
#define UINT_LEAST16_MAX	UINT16_MAX
#define INT_LEAST32_MAX		INT32_MAX
#define INT_LEAST32_MIN		INT32_MIN
#define UINT_LEAST32_MAX	UINT32_MAX

// 7.18.4.1 Macros for minimum-width integer constants
#define INT16_C(x)		(x)
#define UINT16_C(x)		(x ## U)
#define INT32_C(x)		(x ## L)
#define UINT32_C(x)		(x ## UL)


// 7.18.1.3 Fastest minimum-width integer types
// Note that on the Rabbit, a 16-bit int is faster than an 8-bit int
typedef int16_t			int_fast8_t;
typedef uint16_t			uint_fast8_t;
typedef int16_t			int_fast16_t;
typedef uint16_t			uint_fast16_t;
typedef int32_t			int_fast32_t;
typedef uint32_t			uint_fast32_t;

// 7.18.2.3 Limits of fastest minimum-width integer types
#define INT_FAST8_MAX	INT16_MAX
#define INT_FAST8_MIN	INT16_MIN
#define UINT_FAST8_MAX	UINT16_MAX
#define INT_FAST16_MAX	INT16_MAX
#define INT_FAST16_MIN	INT16_MIN
#define UINT_FAST16_MAX	UINT16_MAX
#define INT_FAST32_MAX	INT32_MAX
#define INT_FAST32_MIN	INT32_MIN
#define UINT_FAST32_MAX	UINT32_MAX


// 7.18.1.4 Integer types capable of holding object pointers
// Note that in an all-far model, these types should be 32 bits
#if _DATA_MODEL_IS_FAR
	typedef int32_t         intptr_t;
	typedef uint32_t        uintptr_t;
#else
	typedef int16_t         intptr_t;
	typedef uint16_t        uintptr_t;
#endif

// 7.18.2.4 Limits of integer types capable of holding object pointers
#if _DATA_MODEL_IS_FAR
	#define INTPTR_MIN      INT32_MIN
	#define INTPTR_MAX      INT32_MAX
	#define UINTPTR_MAX     UINT32_MAX
#else
	#define INTPTR_MIN      INT16_MIN
	#define INTPTR_MAX      INT16_MAX
	#define UINTPTR_MAX     UINT16_MAX
#endif

// 7.18.1.5 Greatest-width integer types
typedef int32_t			intmax_t;
typedef uint32_t			uintmax_t;

// 7.18.2.5 Limits of greatest-width integer types
#define INTMAX_MAX		INT32_MAX
#define INTMAX_MIN		INT32_MIN
#define UINTMAX_MAX		UINT32_MAX

// 7.18.4.2 Macros for greatest-width integer constants
#define INTMAX_C(x)		INT32_C(x)
#define UINTMAX_C(x)		UINT32_C(x)


// 7.18.3 Limits of other integer types
// stddef.h defines ptrdiff_t as short and size_t as unsigned short
#define PTRDIFF_MIN		INT16_MIN
#define PTRDIFF_MAX		INT16_MAX
#define SIZE_MAX			UINT16_MAX

// Dynamic C does not support wchar_t or wint_t, so these limits are undefined.
// WCHAR_MIN WCHAR_MAX
// WINT_MIN WINT_MAX

// sig_atomic_t is defined in signal.h as an int
#define SIG_ATOMIC_MIN	INT16_MIN
#define SIG_ATOMIC_MAX	INT16_MAX


#endif	// __STDINT_H



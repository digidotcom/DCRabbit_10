/*
	assert.h

	Copyright (c) 2009-10 Digi International Inc., All Rights Reserved

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
	C90 - 7.2 Diagnostics

	Note that this file may be included in a single scope multiple times and
	needs to change the definition of the assert macro depending on whether
	NDEBUG is set or not.

	Because of this requirement, this .h file DOES NOT HAVE GUARD MACROS.
*/

#undef assert

void assert( int expression);

// Assert macro, see comments for _dc_assert below for more information
// We follow the ANSI standard for the NDEBUG macro, but differ in what
// the macro is defined as in that case to save code space (ANSI specifies
// that assert is defined as ((void)0) when NDEBUG is defined, but this
// generates a NOP in Dynamic C, so we just define it to be nothing.
#ifdef NDEBUG // ANSI standard
	#define assert(ignore)
#else
	#define assert(exp)       ((exp) ? (void)0 : \
                          _dc_assert(#exp, __FILE__, __LINE__))
#endif

#ifndef __DC_ASSERT_LIB
	#use "assert.c"
#endif


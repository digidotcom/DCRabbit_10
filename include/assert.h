/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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


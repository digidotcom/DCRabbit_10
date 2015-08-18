/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	C90 - 7.1.6 Common Definitions
*/

#ifndef __STDDEF_H
#define __STDDEF_H

	/*
		Note regarding ptrdiff_t: Calculating the difference between two pointers
		is only valid if both point to the same array.  Dynamic C does not
		support arrays larger than 32KB, and the result of subtracting two
		pointers (even far pointers) is always a 16-bit integer.
	*/
	typedef short				ptrdiff_t;
	typedef unsigned short	size_t;
	typedef unsigned long	size32_t;				// Dynamic C extension
	typedef char				wchar_t;

	#define NULL				(void *) 0

	// use compiler's built-in offsetof()
	#define offsetof(s, f)	__offsetof(s, f)

	// alternate version of offsetof, using pointer arithmetic
//	#define offsetof(s, f)	((unsigned int)((char*)(&((s*)0)->f) - ((char*)0)))

#endif
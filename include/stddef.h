/*
	stddef.h
	Copyright (c) 2009 Digi International Inc., All Rights Reserved

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
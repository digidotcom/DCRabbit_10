/*
	dc.h

	Copyright (c) 1999-2009 Digi International Inc., All Rights Reserved

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

#ifndef __DCHH_LIB
#define __DCHH_LIB

#ifndef CoData
	#define CoData FuncBlk
#endif

// typedef and macro for easier use of xstrings
typedef const char __far * const __far *xstring_t;
#define XSTRING( xaddr, index)	(((xstring_t)xaddr)[index])

#nointerleave
#nouseix    	 // default do not use ix register as frame pointer

#define FALSE       0
#define TRUE        1

#define EOF			(-1)

// Use CONCAT(a, b) to concat two macros.
// Use CONCAT(a, CONCAT(b, c)) to concat three macros.  ect.
#define _CONCAT(a, b) a ## b
#define CONCAT(a, b) _CONCAT(a, b)

#define ASM_NOIX __nodebug speed __nouseix __root
#define ASM_IX __nodebug speed __useix

// define target pseudo-overflow flag
#define ovf lo			// overflow
#define novf lz 		// no overflow
#define ddip 1      	// default disable interrupt priority

// disable/restore interrupts assembly sequence
#define diasmseq(priority)	push ip $ ipset priority
#define riasmseq           pop ip

// pointers are __near by default (0) or __far by default (1)
#define _DATA_MODEL_IS_FAR 0

#endif


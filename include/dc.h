/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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


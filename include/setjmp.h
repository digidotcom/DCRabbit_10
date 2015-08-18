/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Restore calling environment.  Conforms to C90 standard.
*/

#ifndef __SETJMP_H
#define __SETJMP_H

typedef struct  {
			char    *retsp;
			char    *retix;
			char    *retaddr;			// must follow retix (32-bit push)
			int     retlxpc;
         word    retstackseg;
	}   jmp_buf[1];

__root int setjmp( jmp_buf env);
__root void longjmp( jmp_buf env, int val);

#use "setjmp.c"

#endif
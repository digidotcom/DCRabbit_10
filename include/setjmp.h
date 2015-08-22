/*
	setjmp.h
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
/*
	ctype.h

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
	C90 - 7.3 Character Handling
*/

#ifndef __CTYPE_H
#define __CTYPE_H

// 7.3.1 Character testing functions
__root int isalnum(int c);
__root int isalpha(int c);
__root int iscntrl(int c);
__root int isdigit(int c);
__root int isgraph(int c);
__root int islower(int c);
__root int isprint(int c);
__root int ispunct(int c);
__root int isspace(int c);
__root int isupper(int c);
__root int isxdigit(int c);

// 7.3.2 Character case mapping functions
__root int toupper(int c);
__root int tolower(int c);

// link the library with the actual functions
#use "ctype.c"

#endif


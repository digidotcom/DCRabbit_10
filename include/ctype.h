/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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


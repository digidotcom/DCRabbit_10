/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	From 4.4BSD, POSIX.1-2001
*/

#ifndef __STRINGS_H
#define __STRINGS_H

	int strcasecmp( const char __far *, const char __far *);
	int strncasecmp( const char __far *, const char __far *, size_t);

	#use "string.lib"
#endif
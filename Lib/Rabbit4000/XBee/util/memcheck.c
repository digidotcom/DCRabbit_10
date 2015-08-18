/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <stddef.h>
#include "xbee/platform.h"

// documented in xbee/platform.h
int memcheck( const void FAR *src, int c, size_t length)
{
	const uint8_t FAR *s;

	for (s = src; length--; ++s)
	{
		if (*s != c)
		{
			return *s - c;
		}
	}

	return 0;
}


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


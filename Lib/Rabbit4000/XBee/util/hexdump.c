/*
	Copyright (c)2009-2010 Digi International Inc., All Rights Reserved

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
/**
	@addtogroup util
	@{
	@file util/hexdump.c

	ANSI C hex_dump() implementation if not available natively on a given platform.
*/
/*** BeginHeader hex_dump */
/*** EndHeader */
#include "xbee/platform.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// See xbee/platform.h for function documentation.
void hex_dump( const void FAR *address, uint16_t length, uint16_t flags)
{
	char linebuf[80];
	char *p, *q, *hex, *chars;
   unsigned char ch;
   uint16_t i;
   const char FAR *data = address;

   hex = linebuf;
   if (flags & HEX_DUMP_FLAG_OFFSET)
   {
		hex += 6;			// 0000:<sp>
   }
   else if (flags & HEX_DUMP_FLAG_ADDRESS)
   {
		hex += 8;			// 000000:<sp>
   }
   else if (flags & HEX_DUMP_FLAG_TAB)
   {
		*hex++ = '\t';
   }
   // start printing ASCII characters at position <chars>
   chars = hex + (16 * 3 + 3);

   for(i = 0; i < length; )
   {
   	if (flags & HEX_DUMP_FLAG_OFFSET)
   	{
			sprintf( linebuf, "%04x: ", i);
   	}
		else if (flags & HEX_DUMP_FLAG_ADDRESS)
		{
			sprintf( linebuf, "%" PRIpFAR ": ", data);
		}
   	p = hex;
   	q = chars;
      do {
      	ch = *data++;
      	if ((i & 15) == 8)
      	{
      		// insert space between two sets of 8 bytes
				*p++ = ' ';
				*q++ = ' ';
      	}
      	p[0] = "0123456789abcdef"[ch >> 4];
      	p[1] = "0123456789abcdef"[ch & 0x0F];
      	p[2] = ' ';
      	p += 3;
         *q++ = isprint(ch) ? ch : '.';
      } while ((++i < length) && (i & 15));
		// add missing spaces between hex and printed chars
		memset( p, ' ', chars - p);
		#ifdef __DC__
			q[0] = '\n';
			q[1] = '\0';							// null terminate ASCII characters
			fputs( linebuf, stdout);
			// only necessary to flush stdout on Rabbit platform
			fflush( stdout);
		#else
			*q = '\0';								// null terminate ASCII characters
			puts( linebuf);
		#endif
   }
}

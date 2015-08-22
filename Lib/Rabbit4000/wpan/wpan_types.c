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
	@addtogroup wpan_types
	@{
	@file wpan_types.c
	Data types and macros used by all WPAN (802.15.4) devices.

*/

/*** BeginHeader */
#include <ctype.h>
#include <string.h>
#include "wpan/types.h"
/*** EndHeader */

/*** BeginHeader _WPAN_IEEE_ADDR_UNDEFINED */
/*** EndHeader */
/// @internal address pointed to by macro #WPAN_IEEE_ADDR_UNDEFINED
const addr64 _WPAN_IEEE_ADDR_UNDEFINED =
								{ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

/*** BeginHeader _WPAN_IEEE_ADDR_BROADCAST */
/*** EndHeader */
/// @internal address pointed to by macro #WPAN_IEEE_ADDR_BROADCAST
const addr64 _WPAN_IEEE_ADDR_BROADCAST =
								{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF } };

/*** BeginHeader _WPAN_IEEE_ADDR_COORDINATOR */
/*** EndHeader */
/// @internal address pointed to by macro #WPAN_IEEE_ADDR_COORDINATOR
const addr64 _WPAN_IEEE_ADDR_COORDINATOR =
								{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };


/*** BeginHeader addr64_format */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
addr64_format                           <wpan_types.c>

SYNTAX:
   char FAR *addr64_format( char FAR *buffer,  const addr64 FAR *address)

DESCRIPTION:
     Format a 64-bit address as a null-terminated, printable string
     (e.g., "00-13-A2-01-23-45-67").

     To change the default separator ('-'), define
     ADDR64_FORMAT_SEPARATOR to any character.  For example:

     #define ADDR64_FORMAT_SEPARATOR ':'


PARAMETER1:  buffer - Pointer to a buffer of at least #ADDR64_STRING_LENGTH
              (8 2-character bytes + 7 separators + 1 null = 24) bytes.

PARAMETER2:  address - 64-bit address to format.


RETURNS:  <address> as a printable string (stored in <buffer>).


                     add a parameter for other formats/flags
                     - uppercase vs. lowercase hex
                     - compact format (0013a200-405e0ef0)
                     - format used by the Python framework (with [!]?)


**************************************************************************/
char FAR *addr64_format( char FAR *buffer, const addr64 FAR *address)
{
	int i;
	const uint8_t FAR *b;
	char FAR *p;
	uint_fast8_t ch;

	// format address into buffer
	p = buffer;
	b = address->b;
	for (i = 8; ; )
	{
		ch = *b++;
		*p++ = "0123456789abcdef"[ch >> 4];
		*p++ = "0123456789abcdef"[ch & 0x0F];
		if (--i)
		{
			*p++ = ADDR64_FORMAT_SEPARATOR;
		}
		else
		{
			*p = '\0';
			break;
		}
	}

	// return start of buffer
	return buffer;
}

/*** BeginHeader addr64_equal */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
addr64_equal                            <wpan_types.c>

SYNTAX:
   bool_t addr64_equal( const addr64 FAR *addr1,  const addr64 FAR *addr2)

DESCRIPTION:
     Compare two 64-bit addresses for equality.


PARAMETER1:  addr1 - address to compare
PARAMETER2:  addr2 - address to compare


RETURNS:  TRUE	<addr1> and <addr2> are not NULL and point to
                     identical addresses
          FALSE	NULL parameter passed in, or addresses differ

**************************************************************************/
bool_t addr64_equal( const addr64 FAR *addr1, const addr64 FAR *addr2)
{
	// This is marginally faster than calling memcmp.  Make sure neither
	// parameter is NULL and then do two 4-byte compares.
	return (addr1 && addr2 &&
		addr1->l[0] == addr2->l[0] && addr1->l[1] == addr2->l[1]);
}


/*** BeginHeader addr64_is_zero */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
addr64_is_zero                          <wpan_types.c>

SYNTAX:
   bool_t addr64_is_zero( const addr64 FAR *addr)

DESCRIPTION:
     Test a 64-bit address for zero.


PARAMETER1:  addr - address to test


RETURNS:  TRUE	\c addr is NULL or points to an all-zero address
          FALSE	\c addr points to a non-zero address

SEE ALSO:  WPAN_IEEE_ADDR_ALL_ZEROS

**************************************************************************/
bool_t addr64_is_zero( const addr64 FAR *addr)
{
	return ! (addr && (addr->l[0] || addr->l[1]));
}

/*
	Do we need functions to return the high and low 32-bit halves of an addr64
	in host byte order?

	addr64_high32h and addr64_low32h?

	ntoh64 and hton64?

	I would prefer to ALWAYS store the bytes in an addr64 structure in network
	byte order to reduce the possibility of us passing a struct with the wrong
	byte order.  That doesn't mean we shouldn't provide helper functions to
	get or set the 32-bit halves in host byte order.

*/

/*** BeginHeader addr64_parse */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
addr64_parse                            <wpan_types.c>

SYNTAX:
   int addr64_parse( addr64 *address_be,  const char FAR *str)

DESCRIPTION:
     Parse a text string into a 64-bit IEEE address.

     Converts a text string with eight 2-character hex values, with an optional
     separator between any two values.  For example, the following formats are
     all valid:
     - 01-23-45-67-89-ab-cd-ef
     - 01234567-89ABCDEF
     - 01:23:45:67:89:aB:Cd:EF
     - 0123 4567 89AB cdef


PARAMETER1:  address - converted address (stored big-endian)
PARAMETER2:  str - string to convert, starting with first hex character


RETURNS:  -EINVAL  - invalid parameters passed to function; if <address> is
                     not NULL, it will be set to all zeros
          0        - string converted

**************************************************************************/
int addr64_parse( addr64 *address_be, const char FAR *str)
{
	uint_fast8_t i;
	uint8_t *b;
	int ret;

	i = 8;			// bytes to convert
	if (str != NULL && address_be != NULL)
	{
		// skip over leading spaces
		while (isspace( (uint8_t) *str))
		{
			++str;
		}
		for (b = address_be->b; i; ++b, --i)
		{
			ret = hexstrtobyte( str);
			if (ret < 0)
			{
				break;
			}
			*b = (uint8_t)ret;
			str += 2;					// point past the encoded byte

			// skip over any separator, if present
			if (*str && ! isxdigit( (uint8_t) *str))
			{
				++str;
			}
		}
	}

	if (i == 0)			// successful conversion
	{
		return 0;
	}

	// conversion not complete
	if (address_be != NULL)
	{
		*address_be = *WPAN_IEEE_ADDR_ALL_ZEROS;	// zero out address on errors
	}
	return -EINVAL;
}





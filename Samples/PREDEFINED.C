/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/***************************************************************************

	Samples\predefined.c

    This program demonstrates shows how the contents of the predefined
    macros __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__,
    _DC_GMT_TIMESTAMP_ and CC_VER can be displayed.

***************************************************************************/
#class auto


void main()
{
   printf("Function %s in file %s\n", __FUNCTION__, __FILE__);
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("(%lu seconds since January 1, 1980)\n", _DC_GMT_TIMESTAMP_);
   printf("Current line number: %d\n", __LINE__);
   printf("Current line number: %d\n", __LINE__);

   printf("Compiler version %d.%02x\n", CC_VER >> 8, CC_VER & 0x0FF);
}
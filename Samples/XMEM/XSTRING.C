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
/*****************************************************

	xstring.c

	Example of using the xstring compiler directive.  Dynamic C stores "xstrings"
	as a far array of pointers to strings stored in far memory:

		typedef const char far * const far *xstring_t;

	This program demonstrates how to access the strings of an xstring
	declaration by casting to the xstring_t type, or using a helper macro.

	See the Dynamic C manual for more information about how Dynamic C stores
	xstrings in XMEM.

******************************************************/
#class auto

xstring	mystrings {
	"The quick brown fox",
	"jumps over",
	"the lazy dog."
};

void main()
{
	int	i;

	printf( "Method 1:\n");
	for (i = 0; i < 3; i++)
	{
		printf( "Element %d of mystrings = \"%ls\"\n", i,
																	((xstring_t)mystrings)[i]);
	}

	printf( "Method 2:\n");
	for (i = 0; i < 3; i++)
	{
		printf( "Element %d of mystrings = \"%ls\"\n", i, XSTRING( mystrings, i));
	}
}
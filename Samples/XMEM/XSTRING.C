/*****************************************************

	xstring.c
	Digi International, Copyright ©2001-10.  All rights reserved.

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
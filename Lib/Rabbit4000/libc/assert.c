/*
	assert.c

	Copyright (c) 2006-10 Digi International Inc., All Rights Reserved

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
	Code for helper function referenced by assert macro in assert.h.
*/

/*** BeginHeader _dc_assert */
#define __DC_ASSERT_LIB
void _dc_assert(char* exp, char* filenm, int line);
/*** EndHeader */

/* START _FUNCTION DESCRIPTION ********************************************
_dc_assert                                                       <assert.h>

SYNTAX: void _dc_assert(char* exp, char* filenm, int line);

DESCRIPTION: Internal helper function for assert macro. This function
             takes a stringified expression, a file name, and a line
             number and prints them to stdout, then exits the program
             with an assertion failure exception.

             To disable assertions, add the line:
             #define NDEBUG
             to the top of RabbitBios.c.

PARAMETER 1: Stringified expression that failed the assertion test
PARAMETER 2: Name of the file where the assertion is
PARAMETER 3: Line in the file where the assertion is

RETURN VALUE: None

END DESCRIPTION **********************************************************/

// Assert function called by assert macro
__nodebug
void _dc_assert(char* exp, char* filenm, int line)
{
	// Print error message
	printf("Assertion failure: %s\nFile: %s, Line %d\n", exp, filenm, line);

	// Throw assertion exception and exit
   exception(-ERR_ASSERTFAILURE);
   exit(-ERR_ASSERTFAILURE);
}
/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
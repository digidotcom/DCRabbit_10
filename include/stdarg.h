/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
	Variable Arguments.  Conforms to C90 standard.

	Example (from Wikipedia http://en.wikipedia.org/wiki/Stdarg.h):

	#include <stdio.h>
	#include <stdarg.h>

	void printargs(int arg1, ...) // print all int type args, finishing with -1
	{
	  va_list ap;
	  int i;

	  va_start(ap, arg1);
	  for (i = arg1; i != -1; i = va_arg(ap, int))
	    printf("%d ", i);
	  va_end(ap);
	  putchar('\n');
	}

	int main(void)
	{
	   printargs(5, 2, 14, 84, 97, 15, 24, 48, -1);
	   printargs(84, 51, -1);
	   printargs(-1);
	   printargs(1, -1);
	   return 0;
	}
*/

#ifndef __STDARG_H
#define __STDARG_H

typedef char *va_list;

// Initialize <ap> for subsequent use by va_arg() and va_end() (C89)
#define va_start( ap, last)	((ap) = (va_list)(&last + 1))

// Pop arg of <type> from parameter list (C89)
// <type> must match the type of the next agrument *as promoted according to
// default argument promotions*.  This means that for a "char" parameter, you
// must use "int" as the type.
#define va_arg( ap, type)		(ap += sizeof(type), *(type *)(ap - sizeof(type)))

// Done accessing parameters (C89)
#define va_end( ap)

// Copy one va_list to another (C99)
#define va_copy( dest, src)	((dest) = (src))

#endif

/* START FUNCTION DESCRIPTION ********************************************
va_start                                                        <stdarg.h>

SYNTAX:	void va_start( va_list ap, parameterN)

DESCRIPTION:	This macro is used in variadic functions (those that take a
					variable number of arguments, where the argument list ends
					with "...").  It must be invoked before any access to the
					unnamed arguments.  It initializes ap for subsequent use by
					va_arg() and va_end().

PARAMETER 1:	A variable declared as type va_list.

PARAMETER 2:	The name of the last named argument (the one before the "..."
					in the parameter list).

RETURN VALUE:	None

SEE ALSO:	va_arg, va_end

END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
va_arg                                                          <stdarg.h>

SYNTAX:	TYPE va_arg( va_list ap, TYPE)

DESCRIPTION:	This macro is used in variadic functions (those that take a
					variable number of arguments, where the argument list ends
					with "...").  It pulls the next argument from the stack as
					type <TYPE> and advances <ap> to point to the next argument.

					Due to the way variables are promoted when passed to variadic
					functions, you must use type int instead of char, and type
					double instead of float.

					It is acceptable to use unions and structs as the type.

PARAMETER 1:	A variable declared as type va_list and initialized with
					va_start.

PARAMETER 2:	The type of the next argument (e.g., int, char *).

RETURN VALUE:	Returns the value of the next argument to the function and
					advances the va_list <ap> to point at the next argument on the
					stack.

SEE ALSO:	va_start, va_end

END DESCRIPTION **********************************************************/

/* START FUNCTION DESCRIPTION ********************************************
va_end                                                          <stdarg.h>

SYNTAX:	void va_end( va_list ap)

DESCRIPTION:	This macro is used in variadic functions (those that take a
					variable number of arguments, where the argument list ends
					with "...").  Before returning from a function, va_end should
					be called for any va_list variable initialized with va_start.

					Once passed to va_end(), a given va_list should not be passed
					to va_arg().  It is OK to pass it to va_start() again.

PARAMETER 1:	A variable declared as type va_list and initialized with
					va_start.

RETURN VALUE:	None

SEE ALSO:	va_start, va_arg

END DESCRIPTION **********************************************************/
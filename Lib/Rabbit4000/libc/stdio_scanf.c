/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*** BeginHeader */
#include <stdio.h>
#include <stdarg.h>

#ifdef STDIO_SCANF_DEBUG
	#define _stdio_scanf_debug __debug
#else
	#define _stdio_scanf_debug __nodebug
#endif
/*** EndHeader */

/* START FUNCTION DESCRIPTION ********************************************
fscanf                                                          <stdio.h>

SYNTAX:	int scanf( const char far *format, ...)
			int vscanf( const char far *format, va_list arg)
			int sscanf( const char far *s, const char far *format, ...)
			int _f_sscanf( const char far * str, const char far * format, ...)
			int vsscanf( const char far *s, const char far *format, va_list arg)
			int fscanf( FILE far *stream, const char far *format, ...)
			int vfscanf( FILE far *stream, const char far *format, va_list arg)

	Note that use of vfscanf() requires you to #include <stdarg.h> in your
	program before creating a va_list variable.

DESCRIPTION:	The formatted input functions scan and parse input text into
					separate fields.

					scanf() scans stdin, takes variable arguments
					vscanf() scans stdin, takes a va_list
					sscanf() scans a character buffer, takes variable arguments
					_f_sscanf() is like sscanf, but all arguments are far pointers
					vsscanf() scans a character buffer, takes a va_list
					fscanf() scans any readable file stream, takes variable arguments
					vfscanf() is the underlying function called by the others

PARAMETER <stream>:	The stream to read from.

PARAMETER <s>:			A string to use as the data source (instead of a stream).

PARAMETER <...>:		Variable arguments to match the conversion specifiers in
							<format>.

PARAMETER <arg>:		A va_list object initialized by the va_start() macro and
							pointing to the arguments to receive the converted input.
							vfscanf() does not call the va_end() macro.

PARAMETER <format>:	A string that specifies the admissible input sequences and
							how they are to be converted for assignment, using
							subsequent arguments as pointers to the objects to receive
							the converted input.

FORMAT:		The format is composed of zero or more directives: one or more
				white-space characters, an ordinary character (neither % nor a
				white-space character), or a conversion specification.  Each
				conversion specification is introduced by the character %.  After
				the %, the following appear in sequence:

				*	An optional assignment-suppressing character *.

				*	An optional decimal integer greater than zero that specifies the
					maximum field width (in characters).

				*	An optional 'F' to indicate that the argument for the specifier
					is a far pointer.

				*	An optional length modifier that specifies the size of the
					receiving object.

	         	l (lowercase L):	The corresponding argument for n, d, i, o, u
	         				and x conversion specifiers is a pointer to a long int
	         				or unsigned long int.  The argument for e, f and g
	         				specifiers is a pointer to a double (instead of a
	         				float).

					ll:	Since Dynamic C does not support the "long long" type,
								this modifier has the same meaning as a single 'l'.

	         	h:		Since a "short int" and an "int" are the same size, this
	         				modifier is ignored.

	         	hh:	The corresponding argument for n, d, i, o, u and x
	         				conversion specifiers is a pointer to a signed or
	         				unsigned char.

					j, t:	Same behavior as a single 'l'.  'j' refers to the intmax_t
								or uintmax_t type and 't' refers to the ptrdiff_t type.

					L, q:	Since Dynamic C does not support the "long double" type,
								these modifiers are ignored.

					z:		Since the size_t type is the same as the int type, this
								modifier is ignored.

				*	A conversion specifier character that specifies the type of
					conversion to be applied.

				The fscanf function executes each directive of the format in turn
				until reaching the end, or a directive fails.  The fscanf function
				can return early on an input failure (unavailability of input
				characters) or matching failure (inappropriate input).

				A directive composed of one or more white-space characters reads
				all whitespace from the input.

				A directive that is an ordinary character reads the next character
				from the source.  If the character differs, it is returned to the
				source and generates a matching failure.

				A directive that is a conversion specification (starting with %)
				defines a set of matching input sequences, as described below
				for each specifier.  A conversion is executed in the following
				steps.

				*	Unless the specifier is [, c or n, skip input white-space
					characters (as specified by the isspace function).

				*	Unless the specifier is n, an input item is read from the source.
					An input item is defined as the longest matching sequence of
					input characters, limited by a specified field width.  The first
					character, if any, after the input item remains unread.

				*	If the length of the input item is zero, it generates a matching
					failure, unless an error prevented input from the source (e.g.,
					stream at EOF) in which case it generates an input failure.

				*	Except in the case of a %% directive, the input item (or, in the
					case of a %n directive, the count of input characters) is
					converted to a type appropriate to the conversion specifier.

				*	Unless assignment suppression was indicated by a *, the result
					of the conversion is placed in the object pointed to by the
					next argument to the function (or next variable argument in
					the va_list).

				*	Trailing white space (including newline characters) is left
					unread unless matched by a directive.  The success of literal
					matches and suppressed assignments is not directly determinable
					other than via the %n directive.

SPECIFIERS:
				%	The %% directive matches a single % character.  No conversion
					assignment occurs.

				n	The %n directive doesn't consume characters from the source.
					The corresponding argument is a pointer to an integer where
					fscanf will write the number of characters read from the input
					source so far.  Execution of the %n directive does not increment
					the assignment count returned at completion of the function.

	d,i,o,u,x	The following specifiers match an optionally signed integer with
					a format identical to the subject sequence of the strtol (if
					signed) or strtoul (if unsigned) function with the given base.
					The corresponding argument is a pointer to an integral type.

	                  specifier   type        base  signed?
	                     d     	decimal      10     yes
	                     i        (any)         0     yes
	                     o        octal         8     no
	                     u        decimal      10     no
	                     x        hexadecimal  16     no
                        p        pointer      16     no

		e,f,g		The e, f and g specifiers match an optionally signed floating
					point number with a format identical to the subject sequence
					of the strtod function.  The corresponding argument is a pointer
					to a floating type.

				c	Matches a sequence of characters of exactly the field width
					(or 1 if the width isn't specified).

				s	Matches a sequence of non-white-space characters.

				[	Matches a non-empty sequence of characters from a set of
					expected characters (the scanset).  The specifier includes all
					subsequent characters in the format string, up to and including
					the matching right bracket (]).

					The characters between the brackets (the scanlist) compose the
					scanset, unless the first character is a circumflex (^), in
					which case the scanset contains all characters NOT in the
					scanlist between the circumflex and matching right bracket.

					If the specifier starts with [] or [^], the right bracket is
					in the scanlist and the next following right bracket character
					is the matching right bracket that ends the specification.

					If a dash (-) character is in the scanlist and is not the first
					(after optional circumflex) nor the last, it indicates a range
					of characters, including the character immediately before and
					after the dash.

	E,F,G,X		The conversion specifiers E, F, G and X are equivalent to the
					lowercase specifiers e, f, g and x.

					The function will return -EINVAL for an unrecognized specifier.

RETURN VALUE:	EOF if an input failure occurs before any conversion.
					Otherwise, returns the number of input items assigned, which
					can be fewer than provided for, or even zero, in the event
					of an early matching failure.

Dynamic C differences from the C99 standard:

				*	We don't support the a and A specifiers for parsing a
					floating point value written in hexadecimal.

				*	We support the 'F' modifier to designate a far pointer.

				*	We recognize (but ignore) the 'q' prefix as an alias for 'L'
					(long double).

				*	Since our int is equivalent to a short int, the optional 'h'
					prefix is ignored.

				*	Since we don't support the "long long" type, the optional 'll'
					prefix is treated the same as a single 'l'.

				*	Since we don't support the "long double" type, the optional 'L'
					prefix is ignored.

				*	Since we don't support multibyte characters, we ignore the
					optional 'l' prefix on the [, c and s specifiers.

END DESCRIPTION **********************************************************/

/*** BeginHeader _scanf */
#ifndef __STDIO_SCANF_H
	#define __STDIO_SCANF_H

// All of these declarations should be private to this file.

typedef struct _scanf_state _scanf_state_t;

// function for getting the next character or putting a character back
// if ch is 0 to 255, char to put back
// if ch is _SCANF_GET, function gets a character and returns it
#define _SCANF_GET	256
typedef int (*_scanf_chars_fn)( _scanf_state_t *state, int ch);

int _scanf( _scanf_chars_fn chars_fn, const void __far *data, word flags_in,
	const char __far *f, va_list arg);

struct _scanf_state {
	_scanf_chars_fn	chars_fn;
	void __far const		*data;
	int					conversions;
	int					chars_read;
	word					flags;
		#define _SCANF_FLAG_NONE			0x0000
	   #define _SCANF_FLAG_NOASSIGN		0x0001
	   #define _SCANF_FLAG_ALLOC			0x0002
	   #define _SCANF_FLAG_CHAR			0x0004
	   #define _SCANF_FLAG_SHORT			0x0008
	   #define _SCANF_FLAG_LONG			0x0010
	   #define _SCANF_FLAG_LONGLONG		0x0020

	   #define _SCANF_FLAG_FAR				0x1000
	   #define _SCANF_FLAG_ALLFAR			0x2000
//	   #define _SCANF_FLAG_UNUSED			0x4000
	   #define _SCANF_FLAG_FINISH			0x8000

		// can only pass these flags into _scanf()
	   #define _SCANF_GLOBAL_FLAGS		(_SCANF_FLAG_ALLFAR)
};

#endif
/*** EndHeader */

/* START _FUNCTION DESCRIPTION ********************************************
_scanf_get_int                                             <stdio_scanf.c>

SYNTAX:	int _scanf_get_int( _scanf_state_t *state, int code, int width,
																				void far *store)

DESCRIPTION:	Helper function used by _scanf() to extract an integer value
					(char, short, int or long) from the character source.  It copies
					characters to a temporary buffer and passes that to strtol()
					or strtoul() in order to limit the number of characters removed
					from the source.

					If an invalid character is read (i.e., one that is not part
					of a valid string representation of an integer), it is pushed
					back to the source ("ungotten").

PARAMETER 1:	Current scanf state.

PARAMETER 2:	One of the following specifiers: x, X, p, d, u, i, o

PARAMETER 3:	Width of the field, or 0 for no limit.  This function is limited
					to 128-characters per integer value.

PARAMETER 4:	An address for storing the result, or NULL if assignment has
					been suppressed.

					Note that this function will use state->flags to cast the result
					of the conversion to the correct size.

RETURN VALUE:	0 on success,
					-1 if source is out of characters (EOF),
					+1 if it wasn't able to match any digits.

					Note that it is possible to match a sign (+ or -) (that isn't
					pushed back) and then return 1 if the sign isn't followed by a
					digit.

END DESCRIPTION **********************************************************/
int _scanf_get_int( _scanf_state_t *state, int code, int width, void __far *store)
{
	char buffer[128];			// scan up to 128-byte integer
	_scanf_chars_fn	chars_fn = state->chars_fn;
	char *p, *bufend;
	int ch;
	int base;
	int signed_flag = 0;

	switch (code)
	{
      case 'x': case 'X':
      case 'p':
      	base = 16;
      	break;

      case 'd':
      	signed_flag = 1;
      case 'u':
      	base = 10;
      	break;

      case 'i':
      	signed_flag = 1;
      	base = 0;
      	break;

      case 'o':
      	base = 8;
      	break;

		default:
			// shouldn't happen
			return -1;
	}

	p = buffer;
	if (width && width < (sizeof buffer - 1))
	{
		bufend = buffer + width;
	}
	else
	{
		bufend = buffer + sizeof buffer - 1;		// leave 1 byte for null
	}

	ch = chars_fn( state, _SCANF_GET);
	// start with optional sign
	if ((ch == '-' || ch == '+') && p < bufend)
	{
		*p++ = ch;
		ch = chars_fn( state, _SCANF_GET);
	}

	// match optional prefix
	if (ch == '0' && p < bufend)
	{
		*p++ = ch;
		ch = chars_fn( state, _SCANF_GET);
		if ((ch == 'x' || ch == 'X') && (base == 0 || base == 16) && p < bufend)
		{
			base = 16;
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
		}
		else if (base == 0)
		{
			base = 8;
		}
	}

	// copy digits
	if (base == 8)
	{
		while (strchr( "01234567", ch) && p < bufend)
		{
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
		}
	}
	else if (base == 16)
	{
		while (isxdigit( ch) && p < bufend)
		{
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
		}
	}
	else
	{
		while (isdigit( ch) && p < bufend)
		{
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
		}
	}

	// push last character back
	chars_fn( state, ch);

	// 0 characters copied, or only a sign (+ or -) was copied
	if (p == buffer || (p - buffer == 1 && (*buffer == '+' || *buffer == '-')))
	{
		// didn't read any digits
		return (ch == EOF) ? -1 : 1;
	}

	if (! store)
	{
		return 0;
	}

	*p = '\0';		// add null terminator
	if (signed_flag)
	{
		long val = strtol( buffer, NULL, base);

		if (state->flags & (_SCANF_FLAG_LONG | _SCANF_FLAG_LONGLONG))
		{
			*(long __far *)store = val;
		}
		else if (state->flags & _SCANF_FLAG_CHAR)
		{
			*(signed char __far *)store = (signed char) val;
		}
		else
		{
			*(int __far *)store = (int) val;
		}
	}
	else
	{
		unsigned long val = strtoul( buffer, NULL, base);

		if (state->flags & (_SCANF_FLAG_LONG | _SCANF_FLAG_LONGLONG))
		{
			*(unsigned long __far *)store = val;
		}
		else if (state->flags & _SCANF_FLAG_CHAR)
		{
			*(unsigned char __far *)store = (unsigned char) val;
		}
		else
		{
			*(unsigned int __far *)store = (unsigned int) val;
		}
	}

	++state->conversions;
	return 0;
}

#ifndef STDIO_DISABLE_FLOATS
/* START _FUNCTION DESCRIPTION ********************************************
_scanf_get_float                                           <stdio_scanf.c>

SYNTAX:	int _scanf_get_float( _scanf_state_t *state, int width,
																					void far *store)

DESCRIPTION:	Helper function used by _scanf() to extract a floating point
					value from the character source.  It copies characters to a
					temporary buffer and passes that to strtod() in order to limit
					the number of characters removed from the source.

					If an invalid character is read (i.e., one that is not part
					of a valid string representation of a float), it is pushed
					back to the source ("ungotten").

PARAMETER 1:	Current scanf state.

PARAMETER 2:	Width of the field, or 0 for no limit.  This function is limited
					to 128-characters per floating point value.

PARAMETER 3:	An address for storing the result, or NULL if assignment has
					been suppressed.

RETURN VALUE:	0 on success,
					-1 if source is out of characters (EOF),
					+1 if it wasn't able to match any digits.

					Note that it is possible to match a sign (+ or -) and decimal
					point (that aren't pushed back) and then return 1 if the
					decimal point isn't followed by a digit.

END DESCRIPTION **********************************************************/
int _scanf_get_float( _scanf_state_t *state, int width, void __far *store)
{
	char buffer[128];			// scan up to 128-byte float
	_scanf_chars_fn	chars_fn = state->chars_fn;
	char *p, *bufend;
	int ch;
	int digit = 0;

	p = buffer;
	if (width && width < (sizeof buffer - 1))
	{
		bufend = buffer + width;
	}
	else
	{
		bufend = buffer + sizeof buffer - 1;		// leave 1 byte for null
	}

	ch = chars_fn( state, _SCANF_GET);
	// start with optional sign
	if (ch == '-' || ch == '+')
	{
		*p++ = ch;
		ch = chars_fn( state, _SCANF_GET);
	}

	// copy digits
	while (isdigit( ch) && p < bufend)
	{
		*p++ = ch;
		ch = chars_fn( state, _SCANF_GET);
		digit = 1;
	}

	// optional decimal point
	if (ch == '.' && p < bufend)
	{
		*p++ = ch;
		ch = chars_fn( state, _SCANF_GET);

		// followed by more digits
		while (isdigit( ch) && p < bufend)
		{
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
			digit = 1;
		}
	}
	// optional exponent
	if (digit && (ch == 'e' || ch == 'E') && p < bufend)
	{
		*p++ = ch;
		ch = chars_fn( state, _SCANF_GET);

		// optional sign for exponent
		if (ch == '-' || ch == '+')
		{
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
		}
		digit = 0;		// exponent requires at least one digit
		while (isdigit( ch) && p < bufend)
		{
			*p++ = ch;
			ch = chars_fn( state, _SCANF_GET);
			digit = 1;
		}
	}

	// push last, unmatched character back
	chars_fn( state, ch);

	if (! digit)
	{
		// didn't read any digits
		return (ch == EOF) ? -1 : 1;
	}

	if (! store)
	{
		return 0;
	}

	*p = '\0';		// add null terminator and convert

	// Note that Dynamic C supports float and double as same precision, and
	// does not support long double.
	*(double __far *)store = strtod( buffer, NULL);

	++state->conversions;
	return 0;
}
#endif

/* START _FUNCTION DESCRIPTION ********************************************
_scanf                                                     <stdio_scanf.c>

SYNTAX:	int _scanf( _scanf_chars_fn chars_fn, const void far *data, word flags_in,
																const char far *f, va_list arg)

DESCRIPTION:	Underlying function used from sscanf, fscanf and similar
					functions to do "formatted scanning".  This function does all
					of the work -- all other scanf functions just set up the
					va_list and pass the correct _scanf_chars_fn to this function.

					With the exception of not handling multi-byte characters,
					this implementation should be compliant with the C99 standard.

PARAMETER 1:	Function to use as a source of characters for scanning.
					See _scanf_chars_stream or _scanf_chars_string for this
					function's API.

PARAMETER 2:	Context pointer to pass to <chars_fn>.  For _scanf_chars_stream,
					this is a (FILE far *).  For _scanf_chars_string, this is a
					(char far *).

PARAMETER 3:	Either _SCANF_FLAG_NONE or _SCANF_FLAG_ALLFAR if the variable
					arguments passed are all far pointers.

PARAMETER 4:	Format string.  See fscanf() for documentation on the supported
					specifiers.

PARAMETER 5:	Pointer to variable arguments.  Note that this function does
					NOT call va_end().

RETURN VALUE:	EOF if an input failure occurs before any conversion, otherwise
					the function returns the number of input items assigned.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int _scanf( _scanf_chars_fn chars_fn, const void __far *data, word flags_in,
	const char __far *f, va_list arg)
{
	_scanf_state_t	state;
   char __far *dest;
   word fwid;
   int ch;

	// If parser encounters %[^abc], match_start points to 'a', match_end points
	// to ']' and match_negate is set if the pattern started with '^'.
   const char __far *match_start, *match_end;
   const char __far *scan;		// used to scan from match_start to match_end
   int mch;							// match character (for scanning ranges)
   int match_negate;				// set to non-zero if match starts with '^'
   int found;						// character was in scanset

	state.chars_fn = chars_fn;
	state.data = data;
	state.flags = flags_in & _SCANF_GLOBAL_FLAGS;
   state.chars_read = 0;
   state.conversions = 0;

   while (*f)
   {
   	while (*f)
   	{
   		// "A directive composed of white-space character(s) is executed by
   		// reading input up to the first non-white-space character (which
   		// remains unread), or until no more characters can be read."
   		if (isspace( *f))
   		{
   			// remove whitespace from stream
   			do {
   				ch = state.chars_fn( &state, _SCANF_GET);
   			} while (isspace( ch));
   			state.chars_fn( &state, ch);				// put back first non-space

   			// remove all whitespace from format
   			while (isspace(*f))
   			{
   				++f;
   			}
   		}
   		else
   		{
   			if (*f == '%')
   			{
   				++f;
   				if (*f != '%')
   				{
						break;			// start of format specifier
					}

					// This is a "%%" specifier -- eat white space and try to match.
					// "Input white-space characters (as specified by the isspace
					// function) are skipped, unless the specification includes a
					// [, c, or n specifier."
	   			do {
	   				ch = state.chars_fn( &state, _SCANF_GET);
	   			} while (isspace( ch));
	   			state.chars_fn( &state, ch);			// put back first non-space
   			}
   			if ((ch = state.chars_fn( &state, _SCANF_GET)) != *f)
   			{
   				// character in source doesn't match character in format
					if (ch == EOF)
		   		{
						return state.conversions ? state.conversions : EOF;
		   		}
   				state.chars_fn( &state, ch);
   				return state.conversions;
   			}
   			// advance to next character in format
   			++f;
   		}

   	}

	   // process flags
      state.flags &= _SCANF_GLOBAL_FLAGS;		// reset non-global flags

		// An optional assignment-suppressing character *.
      if (*f == '*')
      {
			state.flags |= _SCANF_FLAG_NOASSIGN;
			++f;
      }

		// An optional decimal integer greater than zero that specifies the
		// maximum field width (in characters).
      for (fwid = 0; isdigit( *f); ++f)
      {
			fwid = fwid * 10 + (*f - '0');
      }

		// An optional 'F' (Dynamic C extension) to indicate a far pointer.
		if (*f == 'F')
		{
				++f;
				state.flags |= _SCANF_FLAG_FAR;
		}

		// An optional length modifier that specifies the size of the receiving
		// object.
		switch (*f)
		{
			case 'h':
				++f;
				if (*f == 'h')
				{
					++f;
					state.flags |= _SCANF_FLAG_CHAR;					// hh
				}
				else
				{
					state.flags |= _SCANF_FLAG_SHORT;				// h
				}
				break;

			case 'l':
				++f;
				if (*f == 'l')
				{
					++f;
					state.flags |= _SCANF_FLAG_LONGLONG;			// ll
				}
				else
				{
					state.flags |= _SCANF_FLAG_LONG;					// l
				}
				break;

			case 'j':		// intmax_t or uintmax_t
			case 't':		// ptrdiff_t
				++f;
				state.flags |= _SCANF_FLAG_LONG;
				break;

			case 'z':		// size_t (same as default size, int)
				++f;
				break;

			case 'L':		// long double
			case 'q':		// long double (extension, not ANSI)
				++f;
				state.flags |= _SCANF_FLAG_LONGLONG;
				break;
		}

		if (! *f)		// end of format string
		{
         return state.conversions;
		}

      // "Input white-space characters (as specified by the isspace function)
      //	are skipped, unless the specification includes a [, c, or n specifier."
      if (! strchr( "[cn", *f))
      {
  			do {
  				ch = state.chars_fn( &state, _SCANF_GET);
  			} while (isspace( ch));
	      if (ch == EOF)
	      {
	         return state.conversions ? state.conversions : EOF;
	      }
  			state.chars_fn( &state, ch);			// put back first non-space
		}

      if (state.flags & _SCANF_FLAG_NOASSIGN)
      {
			dest = NULL;
      }
      else if (state.flags & _SCANF_FLAG_FAR)
      {
      	dest = va_arg( arg, char __far *);
      }
      else
      {
      	dest = va_arg( arg, char *);
      }

		// A conversion specifier character that specifies the type of conversion
		// to be applied.
		// "The conversion specifiers A, E, F, G, and X are also valid and
		// behave the same as, respectively, a, e, f, g, and x."
		switch (*f)
		{
			case 'n':
				/* "No input is consumed.  The corresponding argument shall be a
					pointer to integer into which is to be written the number of
					characters read from the input stream so far by this call to
					the scanf function.  Execution of a %n directive does not
					increment the assignment count."
				*/
				if (! dest)
				{
					// %*n -- ignore
				}
				else if (state.flags & _SCANF_FLAG_LONG)
				{
					*(long __far *)dest = state.chars_read;
				}
				else
				{
					*(word __far *)dest = state.chars_read;
				}
				break;

			case '[':
				/*	"Matches a nonempty sequence of characters from a set of expected
					characters (the scanset).  The corresponding argument shall be
					a pointer to the initial character of an array large enough to
					accept the sequence and a terminating null character, which will
					be added automatically.  The conversion specifier includes all
					subsequent characters in the format string, up to and including
					the matching right bracket (])."
				*/
				++f;		// point to first character of scanlist (after bracket)

				// A circumflex (^) in the first position negates the meaning of
				// the scanlist -- looking for characters NOT in the list.
				if ( (match_negate = (*f == '^')) )
				{
					++f;		// scanlist starts after circumflex
				}
				match_start = f;

				//	"If the conversion specifier begins with [] or [^], the right
				//	bracket character is in the scanlist..."
				f = _f_strchr( *f == ']' ? f + 1 : f, ']');
				if (! f)
				{
					// format string has open "%[" specifier
					return state.conversions;
				}
				match_end = f;

				// Match characters against list.
            --fwid;  // room for null - note that if fwid was zero, it will
                     // roll to 65535, which is what we want i.e. "infinity".
	         while (fwid--)
	         {
            	ch = state.chars_fn( &state, _SCANF_GET);
            	if (ch == EOF)
            	{
						break;
            	}
					for (found = 0, scan = match_start;
						!found && scan < match_end;
						++scan)
					{
						if (*scan == '-'
							&& (scan != match_start)
							&& (scan != match_end - 1) )
						{
							// scan a range of characters
							++scan;
							for (mch = scan[-2]; !found && mch <= *scan; ++mch)
							{
								found = (ch == mch);
							}
						}
						else
						{
							found = (ch == *scan);
						}
					}
					if (found ^ match_negate)
					{
	               if (dest)
	               {
	                  *dest++ = ch;
	               }
					}
					else
					{
						// put back char that doesn't match scanset
            		state.chars_fn( &state, ch);
            		break;
            	}
				}
            if (dest)
            {
					*dest = '\0';				// add null terminator
					++state.conversions;
            }
         	if (ch == EOF)
         	{
					return state.conversions ? state.conversions : EOF;
         	}
				break;

	      case 'c':
	         /* "Matches a sequence of characters of the number specified by the
	            field width (1 if no field width is present in the directive).
	            The corresponding argument shall be a pointer to the initial
	            character of an array large enough to accept the sequence.  No
	            null character is added."
	         */
	         if (!fwid)
	         {
	            fwid = 1;
	         }
	         while (fwid--)
	         {
            	ch = state.chars_fn( &state, _SCANF_GET);
            	if (ch == EOF)
            	{
						break;
            	}
            	if (dest)
            	{
               	*dest++ = ch;
               }
            }
            if (dest)
            {
            	++state.conversions;
            }
           	if (ch == EOF)
           	{
					return state.conversions ? state.conversions : EOF;
           	}
	         break;

         case 's':
            /* "Matches a sequence of non-whitespace characters.  The corresponding
               argument shall be a pointer to the initial character of an array
               large enough to accept the sequence and a terminating null
               character, which will be added automatically."
            */
            --fwid;  // room for null - note that if fwid was zero, it will
                     // roll to 65535, which is what we want i.e. "infinity".
	         while (fwid--)
	         {
            	ch = state.chars_fn( &state, _SCANF_GET);
            	if (isspace( ch) || ch == EOF)
            	{
            		state.chars_fn( &state, ch);			// put back first space
						break;
            	}
            	if (dest)
            	{
               	*dest++ = ch;
               }
            }
            if (dest)
            {
					*dest = '\0';				// add null terminator
					++state.conversions;
            }
				break;

         case 'd':               // signed decimal integer
         case 'i':               // signed integer
         case 'o':               // octal
         case 'u':               // unsigned decimal
         case 'x': case 'X':     // hex
         case 'p':               // pointer to void
         	ch = _scanf_get_int( &state, *f, fwid, dest);
            if (ch)
            {
					return (ch > 0) ? state.conversions : EOF;
            }
            break;

#ifndef STDIO_DISABLE_FLOATS
			case 'a':	case 'A':
         case 'e':	case 'E':
         case 'f':	case 'F':
         case 'g':	case 'G':
				ch = _scanf_get_float( &state, fwid, dest);
            if (ch)
            {
            	return (ch > 0) ? state.conversions : EOF;
            }
            break;
#endif
         default:
            //printf("F: sscanf unsupported conversion %c\n", *f);
            return -EINVAL;
		}
		++f;
   }

	return state.conversions;
}


/*** BeginHeader _scanf_chars_stream */
int _scanf_chars_stream( _scanf_state_t *state, int ch);
/*** EndHeader */
// scanf_chars_fn for reading from a stream
/* START _FUNCTION DESCRIPTION ********************************************
_scanf_chars_stream                                        <stdio_scanf.c>

SYNTAX:	int _scanf_chars_stream( _scanf_state_t *state, int ch)

DESCRIPTION:	Function to passed to _scanf() by fscanf() and similar functions.
					Source of characters is a stream (FILE *).

PARAMETER 1:	Current scanf state.

PARAMETER 2:	Character to push back, or _SCANF_GET to get the next character.

RETURN VALUE:	The character "gotten" from the stream, or 0 if <ch> was
					not _SCANF_GET.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int _scanf_chars_stream( _scanf_state_t *state, int ch)
{
	if (ch == _SCANF_GET)
	{
		++state->chars_read;
		return fgetc( (FILE __far *)state->data);
	}
	else if (ch >= 0 && ch <= 255)
	{
		--state->chars_read;
		ungetc( ch, (FILE __far *)state->data);
	}
	return 0;
}

/*** BeginHeader _scanf_chars_string */
int _scanf_chars_string( _scanf_state_t *state, int ch);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_scanf_chars_string                                        <stdio_scanf.c>

SYNTAX:	int _scanf_chars_string( _scanf_state_t *state, int ch)

DESCRIPTION:	Function to passed to _scanf() by sscanf() and similar functions.
					Source of characters is a null-terminated string.

PARAMETER 1:	Current scanf state.

PARAMETER 2:	Character to push back, or _SCANF_GET to get the next character.

RETURN VALUE:	The character "gotten" from the stream, or 0 if <ch> was
					not _SCANF_GET.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int _scanf_chars_string( _scanf_state_t *state, int ch)
{
	if (ch == _SCANF_GET)
	{
		ch = *((char __far *)state->data + state->chars_read);
		if (ch)
		{
			++state->chars_read;
			return ch;
		}
		return EOF;
	}
	else if (ch >= 0 && ch <= 255)
	{
		assert( state->chars_read != 0);
		--state->chars_read;
	}
	return 0;
}

/*** BeginHeader vfscanf */
#define vfscanf( stream, format, arg)	\
	_scanf( _scanf_chars_stream, stream, _SCANF_FLAG_NONE, format, arg)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
vfscanf                                                           <stdio.h>

SYNTAX:	int vfscanf( FILE far *stream, const char far *format, va_list arg)

		See function help for fscanf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int (vfscanf)( FILE __far *stream, const char __far *format, va_list arg)
{
	return _scanf( _scanf_chars_stream, stream, _SCANF_FLAG_NONE, format, arg);
}


/*** BeginHeader vscanf */
#define vscanf( format, arg)		vfscanf( stdin, format, arg)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
vscanf                                                           <stdio.h>

SYNTAX:	int vscanf( const char far *format, va_list arg)

		See function help for fscanf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int (vscanf)( const char __far *format, va_list arg)
{
	return vfscanf( stdin, format, arg);
}

/*** BeginHeader scanf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
scanf                                                            <stdio.h>

SYNTAX:	int scanf( const char far *format, ...)

		See function help for fscanf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int scanf( const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = vfscanf( stdin, format, ap);
	va_end( ap);

	return retval;
}


/*** BeginHeader fscanf */
/*** EndHeader */
// Function description appears at the top of this file.
_stdio_scanf_debug
int fscanf( FILE __far *stream, const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = vfscanf( stream, format, ap);
	va_end( ap);

	return retval;
}


/*** BeginHeader vsscanf */
#define vsscanf( s, fmt, arg)		_scanf( _scanf_chars_string, s, 0, fmt, arg)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
vsscanf                                                          <stdio.h>

SYNTAX:	int vsscanf( const char far *s, const char far *format, va_list arg)

		See function help for fscanf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int (vsscanf)( const char __far *s, const char __far *format, va_list arg)
{
	return _scanf( _scanf_chars_string, s, _SCANF_FLAG_NONE, format, arg);
}

/*** BeginHeader sscanf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
sscanf                                                           <stdio.h>

SYNTAX:	int sscanf( const char far *s, const char far *format, ...)

		See function help for fscanf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int sscanf( const char __far *s, const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = _scanf( _scanf_chars_string, s, _SCANF_FLAG_NONE, format, ap);
	va_end( ap);

	return retval;
}

/*** BeginHeader _f_sscanf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_f_sscanf                                                        <stdio.h>

SYNTAX:	int _f_sscanf( const char far *s, const char far *format, ...)

		See function help for fscanf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_scanf_debug
int _f_sscanf( const char __far *s, const char __far * format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = _scanf( _scanf_chars_string, s, _SCANF_FLAG_ALLFAR, format, ap);
	va_end( ap);

	return retval;
}



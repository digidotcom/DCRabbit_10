/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*** BeginHeader */
#include <stdio.h>
#include <stdarg.h>

#ifdef STDIO_PRINTF_DEBUG
	#define _stdio_printf_debug __debug
#else
	#define _stdio_printf_debug __nodebug
#endif
/*** EndHeader */

/*** BeginHeader printf */
/*** EndHeader */
// DEVNOTE: there may now be serialization issues with printf -- if a call
// to printf is interrupted by another task calling printf, those characters
// will be embedded in the previous printf's string.
/* START FUNCTION DESCRIPTION ********************************************
printf                                                           <stdio.h>

SYNTAX:	int printf( const char far *format, ...)
			int vprintf( const char far *format, va_list arg)
			int fprintf( FILE far *stream, const char far *format, ...)
			int vfprintf( FILE far *stream, const char far *format, va_list arg)

			int sprintf( char far *s, const char far *format, ...)
			int vsprintf( char far *s, const char far *format, va_list arg)
			int snprintf( char far *s, size_t size, const char far *format, ...)
			int vsnprintf( char far *s, size_t size, const char far *format,
																						va_list arg)

	Note that use of functions with a va_list parameter require you to
	#include <stdarg.h> in your program before creating a va_list variable.

DESCRIPTION:	The printf family of functions are used for formatted output.

						printf		output to stdout (variable arguments)
						vprintf		output to stdout (va_list for arguments)
						fprintf		output to a stream (variable arguments)
						vfprintf		output to a stream (va_list for arguments)

						sprintf		output to a char buffer (variable arguments)
						vsprintf		output to a char buffer (va_list for arguments)
						snprintf		length-limited version of sprintf
						vsnprintf	length-limited version of vsprintf

					As of Dynamic C 7.25, it is possible to redirect printf output
					to a serial port during run mode by defining a macro to specify
					which serial port.  See the sample program SAMPLES/STDIO_SERIAL.C
					for more information.

					The macro STDIO_DISABLE_FLOATS can be defined if it is not
					necessary to format floating point numbers.  If this macro is
					defined, %e, %f and %g will not be recognized. This can save
					thousands of bytes of code space.

PARAMETER <stream>:	When specified, formatted output is written to this stream.

PARAMETER <s>:			When specified, formatted output is written to this
							character buffer.  With [v]sprintf, the buffer must be
							large enough to hold the longest possible formatted string.
							With [v]snprintf, no more than <size> bytes (including
							null terminator) are written to <s>.

PARAMETER <size>:		The maximum number of characters to encode into the output
							buffer.  Because the output buffer is guaranteed to be
							null-terminated, no more than (<size>-1) non-null
							characters can be encoded into the output buffer.

PARAMETER <arg>:		A va_list object initialized by the va_start() macro and
							pointing to the arguments referenced in the format string.
							The vprintf() functions don't call the va_end() macro.

PARAMETER <...>:		Variable arguments referenced in the format string.

PARAMETER <format>:	A string that specifies how subsequent arguments (passed
							as variable arguments or in a va_list) are converted for
							output.

FORMAT:		The format is composed of zero or more directives: ordinary
	         characters (not %) which are copied unchanged to the output stream;
	         and conversion specifications, each of which results in fetching
	         zero or more subsequent arguments.  Each conversion specification
	         is introduced by the character %.  The % is followed with another
	         % (to copy a % to the output stream) or the following sequence:

	         *	Zero or more flags (in any order) that modify the meaning of the
	         	conversion specification

	         *	An optional minimum field width.  If the converted value has
	         	fewer characters than the field width, it will be padded with
	         	spaces (by default) on the left (or right, if the left adjustment
	         	flag has been given) to the field width.  The field width takes
	         	the form of an asterisk (*, described later) or a decimal
	         	integer.

					If the macros __ANSI_PRINTF__ or __ANSI_STRICT__ are defined,
					the field width will expand, if necessary, to show the complete
					value displayed.  If the macros are not defined, legacy Dynamic
					C behavior is to display asterisks (****) instead of the field
					contents to represent overflow.

	         *	An optional precision, with behavior based on the conversion
	         	specifier (listed after each :

	         	d, i, o, u, x and X:  The minimum number of digits to appear.
	         	e, E and F:  The number of digits to appear after the
	         				decimal-point character.
					g, G:  The maximum number of significant digits.
	         	s:  The maximum number of characters to be written from a string.

					If a precision appears with any other conversion specifier, the
					behavior is undefined.

	         	The precision takes the form of a period (.) followed by either
	         	an asterisk (*, described later) or by an optional decimal
	         	integer.  If only the period is specified, the precision defaults
	         	to zero.

				*	An optional 'F' to indicate that the following s, p or n
					specifier is a far pointer.

	         *	An optional length modifier with the following meanings:

	         	l (lowercase L):	The following d, i, o, u, x or X conversion
	         				specifier applies to a long int or unsigned long int.
	         				The following 'n' conversion specifier points to a long.
	         				For legacy support, also specifies that the following
	         				's' or 'p' specifier is a far pointer.

					ll:	Since Dynamic C does not support the "long long" type,
								this modifier has the same meaning as a single 'l'.

	         	h:		Since a "short int" and an "int" are the same size, this
	         				modifier is ignored.

	         	hh:	The following d, i, o, u, x or X conversion specifier
	         				applies to a signed char or an unsigned char.
	         				The following 'n' conversion specifier points to a
	         				signed char.

					j, t:	Same behavior as a single 'l'.  'j' refers to the intmax_t
								or uintmax_t type and 't' refers to the ptrdiff_t type.

					L, q:	Since Dynamic C does not support the "long double" type,
								these modifiers are ignored.

					z:		Since the size_t type is the same as the int type, this
								modifier is ignored.

	         *	Finally, the character that specifies the type of conversion
	         	to be applied.

WIDTH &		As noted above, an asterisk can indicate a field width, or
PRECISION:	precision, or both.  In this case, int arguments supply the field
				width and/or precision.  The argument to be converted follows the
				precision which follows the width.  A negative width is taken as a
				'-' flag followed by a positive field width.  A negative precision
				is taken as if the precision were omitted.

				For integral values (d, i, o, u, x, X), the result of converting a
				zero value with a precision of zero is no characters.

FLAGS:		-		The result of the conversion will be left-justified within
						the field (without this flag, conversion is right-justified).
						This flag overrides the behavior of the 0 flag.

				0		For d, i, o, u, x, X, e, E, f, g and G conversions, leading
						zeros (following any indication of sign or base) are used to
						pad to the field width; no space padding is performed.  This
						flag is ignored for non-floating point conversions
						(d, i, o, u, x, X) with a specified precision.

			space		If the first character of a signed conversion is not a sign,
						or if a signed conversion results in no characters, a space
						will be prefixed to the result.

				+		The result of a signed conversion will always begin with a
						plus or minus sign (without this flag, only negative values
						begin with a sign).  This flag overrides the behavior of
						the space ( ) flag.

				#		The result is converted to an "alternate form".  For octal
						(o), it increases the precision to force the first digit of
						the result to be a zero.  For hexadecimal (x, X), it prefixes
						a non-zero result with 0x or 0X.

						(Currently not supported) For floating point (e, E, f, g
						and G), the result will always contain a decimal-point
						character, even if no digits follow it.  For g and G
						conversions, trailing zeros are not removed from the result.

CONVERSION:	d, i, o, u, x, X		The precision specifies the minimum number
						of digits to appear.  If the value being converted can be
						represented in fewer digits, it will be expanded with leading
						zeros.  The default precision is 1.

						d, i		Signed integer in the style [-]dddd.

						u			Unsigned integer in the style dddd.

						o			Unsigned octal.

						x			Unsigned hexadecimal using lowercase a-f.

						X			Unsigned hexadecimal using uppercase A-F.

				f, e, E, g, G		Takes a double (floating point) argument.  The
						precision specifies the number of digits after the decimal
						point.  If the precision is missing, it defaults to 6 (for
						the f, e and E conversions) or 1 (for g and G).  If the
						precision is 0 and the '#' flag is not specified, no decimal
						point character appears.  The value is rounded to the
						appropriate number of digits.

						f			Uses the style [-]ddd.ddd.  If a decimal point
									appears, at least one digit appears before it.

						e, E		Uses the style [-]d.ddde±dd (or [-]d.dddE±dd).
									There is one digit before the decimal point.  The
									exponent always contains at least two digits.  If
									the value is zero, the exponent is zero.

						g, G		The style used depends on the value converted.
									Style 'e' (or 'E') will only be used if the exponent
									is less than -4 or greater than or equal to the
									precision.  Trailing zeros are removed from the
									fractional portion of the result.  A decimal point
									appears only if it is followed by a digit.

				c		The int argument is converted to an unsigned char and the
						resulting character is written.

				s		The argument is a pointer to a character array.  Characters
						from the string are written up to (but not including) a null
						terminator.  If the precision is specified, no more than that
						many characters are written.  The array must contain a null
						terminator if the precision is not specified or is greater
						than the size of the array.

				p		The argument is a void pointer, displayed using the 'X'
						format.

				n		The argument is a pointer to a signed integer.  The number
						of characters written by the printf call so far is written
						to that address.  Use %Fn if the parameter is a far pointer.
						Use %ln if it's a pointer to a long.

RETURN VALUE:	The number of characters transmitted, or a negative value if an
					output error occurred.

					For sprintf/vsprintf, the count does not include the null
					terminator written to the character buffer.

					For snprintf/vsnprintf, the count reflects the number of non-null
					characters that would have been written if the buffer was large
					enough.  The actual number of characters written (including the
					null terminator) won't exceed <size>.

Dynamic C differences from the C99 standard:

				*	Floating point types (f, e, E, g, G) do not support the '#'
					flag.

				*	We don't support the a and A specifiers for printing a floating
					point value in hexadecimal.

				*	To avoid buffer overflows or unexpected truncation, values that
					don't fit in the specified width are displayed as asterisks (*).
					To get true ANSI behavior, define the macro __ANSI_STRICT__.

				*	Since our int is equivalent to a short int, the optional 'h'
					prefix is ignored.

				*	Since we don't support the "long long" type, the optional 'll'
					prefix is treated the same as a single 'l'.

				*	Since we don't support the "long double" type, the optional 'L'
					prefix is ignored.

				*	We support the 'F' modifier on the 'p', 's' and 'n' conversion
					specifiers to designate a far pointer/address.

				*	We support the 'l' prefix on the 'p' and 's' conversion
					specifiers to designate a far pointer/address (deprecated).

END DESCRIPTION **********************************************************/
_stdio_printf_debug
int printf( const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = vfprintf( stdout, format, ap);
	va_end( ap);

	return retval;
}


/*** BeginHeader vprintf */
// define masking macro
#define vprintf( format, arg)		vfprintf( stdout, format, arg)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
vprintf                                                          <stdio.h>

SYNTAX:	int vprintf( const char far *format, va_list arg)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
// parens around function name are necessary due to masking macro
_stdio_printf_debug
int (vprintf)( const char __far *format, va_list arg)
{
	return vfprintf( stdout, format, arg);
}


/*** BeginHeader fprintf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
fprintf                                                          <stdio.h>

SYNTAX:	int fprintf( FILE far *stream, const char far *format, ...)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_printf_debug
int fprintf( FILE __far *stream, const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = vfprintf( stream, format, ap);
	va_end( ap);

	return retval;
}


/*** BeginHeader vfprintf */
/*** EndHeader */
_stdio_printf_debug __root __nouseix
void _vfprintf_putc( int c, char __far *buf, int *count, void *instanceParam)
{
	// It would appear that putc() functions called by doprnt shouldn't
	// touch IX.  We'll preserve and restore it here, since a lot can
	// happen when we call fputc.
	int saveix;

	asm ld (sp+@SP+saveix), ix

	// instanceParam is a near pointer to a far FILE pointer
	if (fputc( c, *(FILE __far **) instanceParam) != EOF)
	{
		++*count;
	}

	asm ld ix, (sp+@SP+saveix)
}

/* START FUNCTION DESCRIPTION ********************************************
vfprintf                                                         <stdio.h>

SYNTAX:	int vfprintf( FILE far *stream, const char far *format, va_list arg)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
// Note that C90 spec says that fprintf must be able to produce at least
// 509 characters in any single conversion (i.e., function call).
_stdio_printf_debug
int vfprintf( FILE __far *stream, const char __far *format, va_list arg)
{
	int count = 0;

	doprnt( _vfprintf_putc, format, arg, &stream, NULL, &count);
	if (stream->flags & _FILE_FLAG_AUTOFLUSH)
	{
		fflush( stream);
	}

	return count;
}


/*** BeginHeader sprintf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
sprintf                                                          <stdio.h>

SYNTAX:	int sprintf( char far *s, const char far *format, ...)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_printf_debug
int sprintf( char __far *s, const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = vsprintf( s, format, ap);
	va_end( ap);

	return retval;
}


/*** BeginHeader vsprintf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
vsprintf                                                         <stdio.h>

SYNTAX:	int vsprintf( char far *s, const char far *format, va_list arg)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_printf_debug __root __nouseix
void __qe(int c, char __far *buf, int *cnt, void *instanceParam)
{
	buf[(*cnt)++] = c;
}

_stdio_printf_debug
int vsprintf( char __far *s, const char __far *format, va_list arg)
{
	int count = 0;

	doprnt(__qe, format, arg, NULL, s, &count);
	s[count] = 0;			// add null terminator

	return count;
}

/*** BeginHeader snprintf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
snprintf                                                         <stdio.h>

SYNTAX:	int snprintf( char far *s, size_t num, const char far *format, ...)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_printf_debug
int snprintf( char __far *s, size_t num, const char __far *format, ...)
{
	va_list ap;
	int retval;

	va_start( ap, format);
	retval = vsnprintf( s, num, format, ap);
	va_end( ap);

	return retval;
}


/*** BeginHeader vsnprintf */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
vsnprintf                                                        <stdio.h>

SYNTAX:	int vsnprintf( char far *s, size_t num, const char far *format,
																					va_list arg)

		See function help for printf() for a description of this function.

END DESCRIPTION **********************************************************/
_stdio_printf_debug __root
void __qe_snprintf(int c, char __far *buf, int *cnt, void *instanceParam)
{
	int saveix;
	/*
		instanceParam is a pointer to the number of bytes remaining in the
		destination buffer and snprintf() guarantees a null-terminated buffer.
		Hence, if *(int *)instanceParam <= 1 then we can't write any more
		non-null characters into buf[].
	*/
	/*
		DEVNOTE: this function might work better in assembly
	*/
	#asm
		ld		(sp+@SP+saveix), ix
		ld		a, l								; char to write in a
		ld		iy, (sp+@SP+cnt)				; &bytes_written
		ld		ix, (sp+@SP+instanceParam)	; &bytes_left
		ld		hl, (ix)							; bytes_left
		test	hl
		jr		z, .no_write					; cnt == 0, no more room

		jp		m, .no_write					; cnt < 0, ensure no more room!

		dec	hl
		ld		(ix), hl							; store updated <bytesleft>
		test	hl
		jr		nz, .write_ch					; bytesleft now zero, so...
		xor	a									; write null terminator instead
	.write_ch:
		ld		px, (sp+@SP+buf)
		ld		hl, (iy)
		ld		(px+hl), a
		jr		.inc_count
	.no_write:
		ld		hl, (iy)
	.inc_count:
		inc	hl
		ld		(iy), hl
		ld		ix, (sp+@SP+saveix)
	#endasm

	/*
   if (*(int *)instanceParam > 0)
   {
   	// with snprintf, last character of destination buffer must be null
   	buf[*cnt] = (*(int *)instanceParam == 1) ? '\0' : c;
	   --*(int *)instanceParam;
	}
	++*cnt;
	*/
}

_stdio_printf_debug
int vsnprintf(char __far *buffer, size_t len, const char __far *format, va_list arg)
{
	int count = 0;

	doprnt(__qe_snprintf, format, arg, &len, buffer, &count);

	// If len is 0, __qe_snprintf added the null terminator or there was
	// never any room for the terminator (length passed in was 0).
	if (len > 0)
	{
		buffer[count] = '\0';		// add null terminator
	}

	return count;
}





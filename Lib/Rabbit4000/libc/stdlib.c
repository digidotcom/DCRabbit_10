/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*** BeginHeader */
#ifdef STDLIB_DEBUG
	#define _stdlib_debug __debug
#else
	#define _stdlib_debug __nodebug
#endif
/*** EndHeader */

/*****************************
 *
 *		7.10.1 String conversion functions
 *
 */

/*** BeginHeader _xtoxErr */
extern int _xtoxErr;
/*** EndHeader */
#warns Use of the global _xtoxErr has been deprecated.  To check for errors
#warns in string conversions, call strtod, strtol or strtoul directly and
#warns check the value of the tail pointer.
int _xtoxErr;

/* START FUNCTION DESCRIPTION ********************************************
strtod                                                          <stdlib.h>

NEAR SYNTAX: double _n_strtod( const char *s, char **tailptr);
FAR SYNTAX: double _f_strtod( const char far * s, char far * far * tailptr);

	Unless USE_FAR_STRING_LIB is defined, strtod is defined to _n_strtod.

DESCRIPTION:	Converts the initial portion of <s> to a floating point value.
					Skips leading spaces and converts a sequence of digits with
					optional leading '+' or '-', optional decimal point, and
					optional exponent.

PARAMETER 1:	Character string representation of a floating point number.

PARAMETER 2:	Address of a character pointer to store the address of the first
					character after the converted value.  Ignored if NULL.

RETURN VALUE:	The floating point number represented by <s>.

					If no conversion could be performed, zero is returned.

					If the correct value is outside the range of representable
					values, plus or minus HUGE_VAL is returned (according to the
					sign of the value), and the global <errno> is set to ERANGE.

					If the correct value would cause underflow, zero is returned and
					<errno> is set to ERANGE.

SEE ALSO:	strtol (signed long), strtoul (unsigned long)

KEYWORDS: convert

(Note that the float and double types have the same 32 bits of precision.)

NOTE:		For Rabbit 4000+ users, this function supports FAR pointers.
			The macro USE_FAR_STRING_LIB will change all calls to functions in
			this library to their far versions by default. The user may also
			explicitly call the far version with _f_strfunc where strfunc is
			the name of the string function.

			Because FAR addresses are larger, the far versions of this function
			will run slightly slower than the near version.  To explicitly call
			the near version when the USE_FAR_STRING_LIB macro is defined and all
			pointers are near pointers, append _n_ to the function name, e.g.
			_n_strtod. 	For more information about FAR pointers, see the
			Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

			Warning: The far version of strtod is _NOT_ backwards compatible
			with near pointers due to the use of a double pointer. The problem
			is that char ** tailptr is a 16-bit pointer pointing to another
			16-bit pointer. The far version, char far * far * tailptr, is a
			32-bit pointer pointing to a 32-bit pointer.  If you pass a double
			near pointer as the argument to the double far pointer function,
			the double dereference (**tailptr) of the double pointer will
			attempt to access a 32-bit address pointed to by the passed near
			pointer.  The compiler does not know the contents of a pointer and
			will assume the inner pointer is a 32-bit pointer. For more infor-
			mation, please see the Dynamic C documentation about FAR pointers.

				[ ] = 1 byte
				[ ][ ][x][x] indicates a NEAR address (16 bit) upcast to FAR

			Passing a 'char far * far * ptr' as tailptr:
			ADDRESS:				DATA:
				[ ][ ][x][x]		[y][y][y][y]			(tailptr)
				[y][y][y][y]		[z][z][z][z]			(*tailptr)
				[z][z][z][z]		[Correct contents]	(**tailptr)

			Passing a 'char ** ptr' as tailptr: Note the first pointer can be
			upcast to FAR but the compiler cannot upcast the internal pointer.
			ADDRESS:				DATA:
				[ ][ ][x][x]		[ ][ ][y][y]			(tailptr)
				[ ][ ][y][y]		[?][?][z][z]			(*tailptr)
				[?][?][z][z]		[Incorrect contents]	(**tailptr)

END DESCRIPTION **********************************************************/

/*** BeginHeader _f_strtod */
/*** EndHeader */
#define _STRTOD_MAX_SIGDIGS_  10    // Ignore non-significant digits
#define _STRTOD_MAX_DIGITS_   4     // Use int's to gather chunks of digits
#define _STRTOD_MAX_EXPONENT_ 38    // Update this when HUGE_VAL changes!
#define _STRTOD_MIN_EXPONENT_ -38   // Update this when HUGE_VAL changes!
#if (float)3.00e38 != HUGE_VAL
	#fatal "Both _STRTOD_MAX_EXPONENT_ and the HUGE_VAL test must be updated!
#endif
const float _STRTOD_TEN_POW_[] = {
   1, 1e1, 1e2, 1e3, 1e4, //1e5, 1e6, 1e7, 1e8,
};
const float _STRTOD_ONE_TENTH_POW_[] = {
   1, 1e-1, 1e-2, 1e-3, 1e-4, //1e-5, 1e-6, 1e-7, 1e-8,
};

/* START FUNCTION DESCRIPTION ********************************************
_f_strtod                                                       <stdlib.h>

SYNTAX:	double _f_strtod(const char far * s, char far * far * tailptr)

See function help for strtod() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
double _f_strtod(const char __far * s, char __far * __far * tailptr)
{
   auto float sum, tenths;
   auto float lastsum, lasttenths;
   auto int over_under_flow = 0; // 0 == success, 1 == overflow, -1 == underflow
   auto long exp;
   auto int negative;
   auto int i_digits;   // Collect digits in an int for speed
   auto char tmpdig;
   auto const char __far *s_ptr;
   auto char __far *tail;
   auto int sigdigs;

   // Skip over leading white space
   while (isspace(*s))
   {
      s++;
   }

   // Look for a sign ...
   switch (*s)
   {
   case '-' :
      s++;
      negative = -1;
      break;
   case '+' :
      s++;
   default :
      negative = 0;
      break;
   }

   // Get significant digits, if any, before the decimal point
   sigdigs = 0;
   lastsum = 0.;
   sum = 0.;
   s_ptr = s;
   while (isdigit(*s))
   {
      if (sigdigs > _STRTOD_MAX_SIGDIGS_)
      {
         break;
      }
      i_digits = 0;
      for(; s_ptr - s < _STRTOD_MAX_DIGITS_ && isdigit(*s_ptr); ++s_ptr)
      {
         i_digits *= 10;
         i_digits += (tmpdig = *s_ptr - '0');
         // Start counting significant digits at first non-zero digit (tmpdig)
         if (sigdigs || tmpdig)
         {
            ++sigdigs;
            if (sigdigs > _STRTOD_MAX_SIGDIGS_)
            {
               ++s_ptr;
               break;
            }
         }
      }

      sum *= _STRTOD_TEN_POW_[(unsigned)(s_ptr - s)];
      sum += (float) i_digits;
      s = s_ptr;
   } //while
   s = s_ptr;

   // It is assumed that significant pre-decimal point digits will never cause
   //  overflow. If there are "insignificant" pre-decimal point digits yet to be
   //  processed then the possibility of overflow becomes a concern.
   lastsum = sum;
   // The rest of the pre-decimal point digits, if any, are insignificant
   while (isdigit(*s))
   {
      for (s_ptr = s; s_ptr - s < _STRTOD_MAX_DIGITS_ && isdigit(*s_ptr);
           ++s_ptr)
         ;
      sum *= _STRTOD_TEN_POW_[(unsigned)(s_ptr - s)];
      if (!over_under_flow && lastsum >= sum)
      {  // Overflow, sum has stalled at or wrapped around maximum!
         over_under_flow = 1;
      }
      lastsum = sum;
      s = s_ptr;
   }

   // Check for an optional decimal point
   if (*s == '.')
   {
      // Get the significant digits after the decimal point
      ++s;
      tenths = 1.;
      // If no significant pre-decimal point digits, then many "insignificant"
      //  post-decimal point digits present the possibility of underflow.
      lasttenths = tenths;
      while(isdigit(*s))
      {
         if (sigdigs > _STRTOD_MAX_SIGDIGS_)
         {
            break;
         }
         i_digits = 0;
         for(s_ptr = s; s_ptr - s < _STRTOD_MAX_DIGITS_ && isdigit(*s_ptr);
               ++s_ptr)
         {
            i_digits *= 10;
            i_digits += (tmpdig = *s_ptr - '0');
            if (sigdigs || tmpdig)
            {
               ++sigdigs;
               if (sigdigs > _STRTOD_MAX_SIGDIGS_)
               {
                  break;
               }
            }

         }
         tenths *= _STRTOD_ONE_TENTH_POW_[(unsigned)(s_ptr-s)];
         if (!sigdigs && !over_under_flow && lasttenths <= tenths)
         {  // Underflow, tenths has stalled at or wrapped around minimum!
            over_under_flow = -1;
         }
         lasttenths = tenths;
         sum += i_digits * tenths;
         s = s_ptr;
      }
      // The rest of the digits are insignificant
      for( ; isdigit(*s); ++s);
   }

   // Check for optional exponent
   if (tolower(*s) == 'e')
   {
      exp = _f_strtol(++s, &tail, 10);
      if (!over_under_flow)
      {
         if (_STRTOD_MAX_EXPONENT_ < exp)
         {
            over_under_flow = 1;
         }
         else if (_STRTOD_MIN_EXPONENT_ > exp)
         {
            over_under_flow = -1;
         }
      }
      if (s != tail)
      {
         if (!over_under_flow && exp)
         {  // not already over/underflowed and a non-zero exponent
            lastsum = sum;
            sum *= _pow10((int) exp);
            if (0L < exp)
            {  // check for overflow
               if (lastsum >= sum)
               {  // Overflow, sum has stalled at or wrapped around maximum!
                  over_under_flow = 1;
               }
            }
            else
            {  // check for underflow
               if (lastsum <= sum || (lastsum > 0.0 && sum == 0.0))
               {  // Underflow, sum has stalled at or wrapped around minimum!
                  over_under_flow = -1;
               }
            }
         }
         s = tail;
      }
      else
      {
         // Wasn't really an exponent, after all.
         --s;
      }
   }

   // If necessary, pick the appropriate over/underflow return value, set errno.
   if (over_under_flow)
   {
      if (0 < over_under_flow)
      {  // overflow
         sum = HUGE_VAL;
      }
      else
      {  // underflow
         sum = 0.;
      }
      errno = ERANGE;
   }

   // Update the tail pointer
   if (tailptr)
   {
      *tailptr = (char __far *)s;
   }

   // Multiply in the sign
   return negative ? -sum : sum;
}

/*** BeginHeader _n_strtod */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_n_strtod                                                       <stdlib.h>

SYNTAX:	double _n_strtod(const char *sptr, char **tailptr)

See function help for strtod() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
double _n_strtod(const char *sptr, char **tailptr)
{
	auto char __far *far_tail;
	auto double retval;

	retval = _f_strtod( (const char __far *) sptr, &far_tail);
	if (tailptr)
	{
		*tailptr = (char *) (word) (unsigned long) far_tail;
	}

	return retval;
}

/* START FUNCTION DESCRIPTION ********************************************
strtol                                                          <stdlib.h>

NEAR SYNTAX:	long _n_strtol( const char *sptr,
																	char **tailptr, int base);
FAR SYNTAX:		long _f_strtol( const char far *sptr,
														char far * far *tailptr, int base);

	Unless USE_FAR_STRING_LIB is defined, strtol is defined to _n_strtol.

DESCRIPTION:	Converts the initial portion of <sptr> to a signed long value.
					Skips leading spaces and optional sign ('+' or '-') character
					before converting a sequence of characters resembling an integer
					represented in some radix determined by the value of <base>.

PARAMETER 1:	Character string representation of a signed long value.

PARAMETER 2:	Address of a character pointer to store the address of the first
					character after the converted value.  Ignored if NULL.

PARAMETER 3:	Radix to use for the conversion, can be zero (see below) or
					between 2 and 36.  The number to convert must contain letters
					and digits appropriate for expressing an integer of the given
					radix.

					The letters from a (or A) to z (or Z) correspond to the values
					10 to 35.  Only letters whose values are less than that of
					<base> are permitted.

					If <base> is zero:
						A leading 0x or 0X is skipped and <base> is set to 16.
						A leading 0 is skipped and <base> is set to 8.
						Without a leading 0, <base> is set to 10.


RETURN VALUE:	The signed long number represented by <sptr>.

					If no conversion could be performed, zero is returned.  In
					addition, the global <errno> is set to EINVAL if <radix> was
					invalid.

					If the correct value is outside the range of representable
					values, LONG_MAX or LONG_MIN is returned (according to the sign
					of the value), and the global <errno> is set to ERANGE.

SEE ALSO:	strtod (floating point), strtoul (unsigned long)

KEYWORDS: convert

NOTE:		For Rabbit 4000+ users, this function supports FAR pointers.
			The macro USE_FAR_STRING_LIB will change all calls to functions in
			this library to their far versions by default. The user may also
			explicitly call the far version with _f_strfunc where strfunc is
			the name of the string function.

			Because FAR addresses are larger, the far versions of this function
			will run slightly slower than the near version.  To explicitly call
			the near version when the USE_FAR_STRING_LIB macro is defined and all
			pointers are near pointers, append _n_ to the function name, e.g.
			_n_strtoul. 	For more information about FAR pointers, see the
			Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

			Warning: The far version of strtoul is _NOT_ backwards compatible
			with near pointers due to the use of a double pointer. The problem
			is that char ** tailptr is a 16-bit pointer pointing to another
			16-bit pointer. The far version, char far * far * tailptr, is a
			32-bit pointer pointing to a 32-bit pointer.  If you pass a double
			near pointer as the argument to the double far pointer function,
			the double dereference (**tailptr) of the double pointer will
			attempt to access a 32-bit address pointed to by the passed near
			pointer.  The compiler does not know the contents of a pointer and
			will assume the inner pointer is a 32-bit pointer. For more infor-
			mation, please see the Dynamic C documentation about FAR pointers.

				[ ] = 1 byte
				[ ][ ][x][x] indicates a NEAR address (16 bit) upcast to FAR

			Passing a 'char far * far * ptr' as tailptr:
			ADDRESS:				DATA:
				[ ][ ][x][x]		[y][y][y][y]			(tailptr)
				[y][y][y][y]		[z][z][z][z]			(*tailptr)
				[z][z][z][z]		[Correct contents]	(**tailptr)

			Passing a 'char ** ptr' as tailptr: Note the first pointer can be
			upcast to FAR but the compiler cannot upcast the internal pointer.
			ADDRESS:				DATA:
				[ ][ ][x][x]		[ ][ ][y][y]			(tailptr)
				[ ][ ][y][y]		[?][?][z][z]			(*tailptr)
				[?][?][z][z]		[Incorrect contents]	(**tailptr)

END DESCRIPTION **********************************************************/

/*** BeginHeader _f_strtol */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_f_strtol                                                       <stdlib.h>

SYNTAX:	long _f_strtol(const char far *sptr,
								char far * far * tailptr, int base)

See function help for strtol() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
long _f_strtol(const char __far *sptr, char __far * __far * tailptr, int base)
{
	auto unsigned long sum, premul_limit;
	auto unsigned long max_unsigned;
	auto int char_limit;
	auto int negative;
	auto int overflow;
   auto const char __far *original_sptr = sptr;
   auto int saw_digit = FALSE;
	auto int ch;

	// check base parameter
	if (base > 36 || base < 0 || base == 1)
	{
		if (tailptr)
		{
			*tailptr = (char __far *)sptr;
		}
		errno = EINVAL;
		return 0;
	}

	// eat up whitespace
	while (isspace( *sptr))
	{
		sptr++;
	}

	// determine sign (if any)
	switch (*sptr)
	{
		case '-':
			sptr++;
			negative = 1;
			break;
		case '+':
			sptr++;
			// fall through to default case
		default:
			negative = 0;
			break;
	}

	// determine base
	if (*sptr == '0')
	{
		if ((base == 0 || base == 16) && (toupper( sptr[1]) == 'X'))
		{
			// skip over leading 0x (and 0X) for hex conversions
			sptr += 2;
			base = 16;
		}
		else if (base == 0)
		{
			base = 8;
		}
	}
	else if (base == 0)
	{
		base = 10;
	}

	max_unsigned = negative ? 0x80000000ul : 0x7FFFFFFFul;
	premul_limit = (long) (max_unsigned / base);
	char_limit = (int) (max_unsigned % base);

	// start conversion
	for (sum = 0, overflow = 0;;)
	{
		ch = *sptr++;
		if (isdigit( ch))
		{
			ch -= '0';
		}
		else if (isalpha( ch))
		{
			ch = toupper(ch) + (10 - 'A');
		}
		else
		{
			// invalid character
			break;
		}

      if (ch >= base)
      {
      	// character is too big for this base
         break;
      }

      saw_digit = TRUE;
      
		// check for overflow -- if either condition is true, the multiply/add
		// to calculate the new sum will overflow
      if (overflow ||
      	sum > premul_limit || (sum == premul_limit && ch > char_limit))
      {
      	overflow = 1;
      }
		else
		{
      	sum = sum * base + ch;
      }
	}

	// only update tailptr if not NULL
   if (tailptr)
   {
      *tailptr = (char __far *)(saw_digit ? sptr - 1 : original_sptr);
   }

	if (overflow)
	{
		errno = ERANGE;
      return (long) max_unsigned;
	}
	else
	{
      return negative ? -(long) sum : (long) sum;
	}
}


/*** BeginHeader _n_strtol */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_n_strtol                                                       <stdlib.h>

SYNTAX:	long _n_strtol( const char * sptr, char ** tailptr, int base)

See function help for strtol() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
long _n_strtol( const char * sptr, char ** tailptr, int base)
{
	auto char __far *far_tail;
	auto long retval;

	retval = _f_strtol( (const char __far *) sptr, &far_tail, base);
	if (tailptr)
	{
		*tailptr = (char *) (word) (unsigned long) far_tail;
	}

	return retval;
}


/* START FUNCTION DESCRIPTION ********************************************
strtoul                                                         <stdlib.h>

NEAR SYNTAX:	unsigned long _n_strtoul(const char *sptr,
																		char **tailptr, int base)
FAR SYNTAX:		unsigned long _f_strtoul(const char far *sptr,
															char far * far *tailptr, int base)

	Unless USE_FAR_STRING_LIB is defined, strtoul is defined to _n_strtoul.

DESCRIPTION:	Converts the initial portion of <sptr> to an unsigned long value.
					Skips leading spaces and optional sign ('+' or '-') character
					before converting a sequence of characters resembling an integer
					represented in some radix determined by the value of <base>.

					If the sign is '-', result is negated before being returned.

PARAMETER 1:	Character string representation of an unsigned long value.

PARAMETER 2:	Address of a character pointer to store the address of the first
					character after the converted value.  Ignored if NULL.

PARAMETER 3:	Radix to use for the conversion, can be zero (see below) or
					between 2 and 36.  The number to convert must contain letters
					and digits appropriate for expressing an integer of the given
					radix.

					The letters from a (or A) to z (or Z) correspond to the values
					10 to 35.  Only letters whose values are less than that of
					<base> are permitted.

					If <base> is zero:
						A leading 0x or 0X is skipped and <base> is set to 16.
						A leading 0 is skipped and <base> is set to 8.
						Without a leading 0, <base> is set to 10.


RETURN VALUE:	The unsigned long number represented by <sptr>.

					If no conversion could be performed, zero is returned.  In
					addition, the global <errno> is set to EINVAL if <radix> was
					invalid.

					If the correct value is outside the range of representable
					values, ULONG_MAX is returned, and the global <errno> is set
					to ERANGE.

SEE ALSO:	strtod (floating point), strtol (signed long)

KEYWORDS: convert

NOTE:		For Rabbit 4000+ users, this function supports FAR pointers.
			The macro USE_FAR_STRING_LIB will change all calls to functions in
			this library to their far versions by default. The user may also
			explicitly call the far version with _f_strfunc where strfunc is
			the name of the string function.

			Because FAR addresses are larger, the far versions of this function
			will run slightly slower than the near version.  To explicitly call
			the near version when the USE_FAR_STRING_LIB macro is defined and all
			pointers are near pointers, append _n_ to the function name, e.g.
			_n_strtoul. 	For more information about FAR pointers, see the
			Dynamic C Manual or the samples in Samples/Rabbit4000/FAR/

			Warning: The far version of strtoul is _NOT_ backwards compatible
			with near pointers due to the use of a double pointer. The problem
			is that char ** tailptr is a 16-bit pointer pointing to another
			16-bit pointer. The far version, char far * far * tailptr, is a
			32-bit pointer pointing to a 32-bit pointer.  If you pass a double
			near pointer as the argument to the double far pointer function,
			the double dereference (**tailptr) of the double pointer will
			attempt to access a 32-bit address pointed to by the passed near
			pointer.  The compiler does not know the contents of a pointer and
			will assume the inner pointer is a 32-bit pointer. For more infor-
			mation, please see the Dynamic C documentation about FAR pointers.

				[ ] = 1 byte
				[ ][ ][x][x] indicates a NEAR address (16 bit) upcast to FAR

			Passing a 'char far * far * ptr' as tailptr:
			ADDRESS:				DATA:
				[ ][ ][x][x]		[y][y][y][y]			(tailptr)
				[y][y][y][y]		[z][z][z][z]			(*tailptr)
				[z][z][z][z]		[Correct contents]	(**tailptr)

			Passing a 'char ** ptr' as tailptr: Note the first pointer can be
			upcast to FAR but the compiler cannot upcast the internal pointer.
			ADDRESS:				DATA:
				[ ][ ][x][x]		[ ][ ][y][y]			(tailptr)
				[ ][ ][y][y]		[?][?][z][z]			(*tailptr)
				[?][?][z][z]		[Incorrect contents]	(**tailptr)

END DESCRIPTION **********************************************************/
/*** BeginHeader _f_strtoul */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_f_strtoul                                                       <stdlib.h>

SYNTAX:	unsigned long _f_strtoul( const char far *sptr,
												char far * far *tailptr, int base)

See function help for strtoul() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
unsigned long _f_strtoul( const char __far *sptr,
														char __far * __far *tailptr, int base)
{
	auto unsigned long sum, premul_limit;
	auto int char_limit, overflow;
   auto const char __far *original_sptr = sptr;
   auto int saw_digit = FALSE;
	auto char ch;
	auto int negative = 0;

	if (base > 36 || base < 0 || base == 1)
	{
		if (tailptr != (void __far *)NULL)
		{
			*tailptr = (char __far *) sptr;
		}
		errno = EINVAL;
		return 0;
	}

	// eat up whitespace
	while (isspace( *sptr))
	{
		sptr++;
	}

	// ignore sign
	switch (*sptr)
	{
		case '-':
			negative = 1;
		case '+':
			sptr++;
	}

	// determine base
	if (*sptr == '0')
	{
		if ((base == 0 || base == 16) && (toupper( sptr[1]) == 'X'))
		{
			// skip over leading 0x (and 0X) for hex conversions
			sptr += 2;
			base = 16;
		}
		else if (base == 0)
		{
			base = 8;
		}
	}
	else if (base == 0)
	{
		base = 10;
	}

	premul_limit = ULONG_MAX / base;
	char_limit = (int) (ULONG_MAX % base);

	// start conversion
	for (sum = 0, overflow = 0;;)
	{
		ch = *sptr++;
		if (isdigit( ch))
		{
			ch -= '0';
		}
		else if (isalpha( ch))
		{
			ch = toupper(ch) + (10 - 'A');
		}
		else
		{
			// invalid character
			break;
		}

      if (ch >= base)
      {
      	// character is too big for this base
         break;
      }

      saw_digit = TRUE;
      
		// check for overflow -- if either condition is true, the multiply/add
		// to calculate the new sum will overflow
      if (overflow ||
      	sum > premul_limit || (sum == premul_limit && ch > char_limit))
      {
      	overflow = 1;
      }
		else
		{
      	sum = sum * base + ch;
      }
	}

	// only update tailptr if not NULL
   if (tailptr)
   {
      *tailptr = (char __far *)(saw_digit ? sptr - 1 : original_sptr);
   }

	if (overflow)
	{
		errno = ERANGE;
      return ULONG_MAX;
	}
	else
	{
      return negative ? -sum : sum;
	}
}


/*** BeginHeader _n_strtoul */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_n_strtoul                                                       <stdlib.h>

SYNTAX:	unsigned long _n_strtoul(const char *sptr, char **tailptr, int base)

See function help for strtoul() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
unsigned long _n_strtoul(const char *sptr, char **tailptr, int base)
{
	auto char __far *far_tail;
	auto unsigned long retval;

	retval = _f_strtoul( (const char __far *) sptr, &far_tail, base);
	if (tailptr)
	{
		*tailptr = (char *) (word) (unsigned long) far_tail;
	}

	return retval;
}


/*** BeginHeader atof */
#define atof( str)		_f_strtod( str, NULL)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
atof                                                            <stdlib.h>

SYNTAX:	double atof( const char far * sptr)

DESCRIPTION:	Converts the initial portion of the string <sptr> to a floating
					point value.  It is equivalent to:

						strtod( sptr, NULL).

RETURN VALUE: The converted floating value.

(Note that the float and double types have the same 32 bits of precision.)

KEYWORDS: convert

END DESCRIPTION **********************************************************/
_stdlib_debug
double (atof)( const char __far * sptr)
{
	return _f_strtod( sptr, NULL);
}


/*** BeginHeader atoi */
#define atoi( sptr)		((int) _f_strtol( sptr, NULL, 10))
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
atoi                                                            <stdlib.h>

SYNTAX: int atoi( const char far * sptr);

DESCRIPTION:	Converts the initial portion of the string <sptr> to an integer
					value.  It is equivalent to:

						(int) strtol( sptr, NULL, 10)

RETURN VALUE: The converted integer value.

KEYWORDS: convert

END DESCRIPTION **********************************************************/
_stdlib_debug
int (atoi)( const char __far * sptr)
{
	return (int) _f_strtol( sptr, NULL, 10);
}

/*** BeginHeader atol */
#define atol( sptr)		_f_strtol( sptr, NULL, 10)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
atol                                                            <stdlib.h>

SYNTAX:	long atol( const char far * sptr);

DESCRIPTION:	Converts the initial portion of the string <sptr> to a long
					integer value.  It is equivalent to:

						strtol( sptr, NULL, 10)

RETURN VALUE: The converted long integer value.

KEYWORDS: convert

END DESCRIPTION **********************************************************/
_stdlib_debug
long (atol)( const char __far * sptr)
{
	return _f_strtol(sptr, NULL, 10);
}


/*****************************
 *
 *		7.10.2 Pseudo-random sequence generation functions
 *
 */

/*** BeginHeader _rand_next */
extern unsigned long _rand_next;
/*** EndHeader */
unsigned long _rand_next = 1;

/*** BeginHeader rand */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
rand                                                            <stdlib.h>

SYNTAX:	int rand( void)

NOTE:	The rand() function in versions of Dynamic C prior to 10.64 generated a
		pseudo-random sequence of floating point values from 0.0 to 1.0.  That
		function was renamed to randf() in the 10.64 release in favor of the
		ANSI C90 functionality.

DESCRIPTION:	Computes a sequence of pseudo-random integers in the range 0 to
					RAND_MAX (32767).

PARAMETER:	None.

RETURN VALUE:	Psuedo-random integer from 0 to 32767, inclusive.

SEE ALSO:	srand, rand16, seed_init, randf, randg, randb, srandf

END DESCRIPTION **********************************************************/
_stdlib_debug
int rand( void)
{
	_rand_next = _rand_next * 1103515245 + 12345;
	return (unsigned int)(_rand_next >> 16) % (RAND_MAX + 1);
}


/*** BeginHeader srand */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
srand                                                           <stdlib.h>

SYNTAX:	void srand( unsigned int seed)

NOTE:	The srand() function in versions of Dynamic C prior to 10.64 was used to
		seed a floating point pseudo-random generator.  That function was renamed
		to srandf() in the 10.64 release in favor of the ANSI C90 functionality.

DESCRIPTION:	Sets the seed for the pseudo-random number generator used by
					rand().  The generated sequence is always the same for a given
					seed value.  If rand() is called before srand(), the sequence
					is identical to one seeded by calling srand(1).

PARAMETER 1:	New seed value.

RETURN VALUE:	None.

SEE ALSO:	rand, rand16, seed_init, randf, randg, randb, srandf

END DESCRIPTION **********************************************************/
_stdlib_debug
void srand( unsigned int seed)
{
	_rand_next = seed;
}


/*****************************
 *
 *		7.10.4 Communication with the environment
 *
 */

/*** BeginHeader _abort */
/*** EndHeader */
#include <signal.h>				// for raise() and SIGABRT
/* START FUNCTION DESCRIPTION ********************************************
abort                                                           <stdlib.h>

SYNTAX:	void abort( void)

DESCRIPTION:	Sends a SIGABRT to the program before exiting.  Ignores any
					exit functions registered with atexit().  If program has
					registered a SIGABRT handler (via the signal() function),
					that handler can cancel the abort by not returning (perhaps
					by calling longjmp).

PARAMETER:	None

RETURN VALUE:	Function does not return.

SEE ALSO:	signal, raise, exit, atexit

END DESCRIPTION **********************************************************/
_stdlib_debug __root
void _abort( void)
{
	raise( SIGABRT);

	// Jump into _sys_exit() function in sys.lib, leaving stack set up so that
	// the function calling abort() appears to have called _sys_exit().
	#asm
		pop	de							; return address
		ld		hl, EXIT_FAILURE		; exit code
		push	hl							; exit code on stack first
		push	de							; then return address
		jp		_sys_exit				; chain into system's exit handler
	#endasm
}

/*** BeginHeader _atexit_func_table */
/*** EndHeader */
// Dynamic C doesn't zero out static variables, necessary to do so manually.
_exit_func _atexit_func_table[_ATEXIT_FUNC_COUNT] = { NULL };

/*** BeginHeader atexit */
/*** EndHeader */
#if ! (_ATEXIT_FUNC_COUNT > 0)
	#fatal If calling atexit(), you must define _ATEXIT_FUNC_COUNT to at least 1.
#endif
/* START FUNCTION DESCRIPTION ********************************************
atexit                                                          <stdlib.h>

SYNTAX:	int atexit( void (*func)(void))

DESCRIPTION:	Registers the function <func> to be called without arguments at
					normal program termination.  Supports the registration of
					_ATEXIT_FUNC_COUNT functions.

					_ATEXIT_FUNC_COUNT defaults to 4 but can be set in the Global
					Macro Definitions of the Project Options.

PARAMETER 1:	The function to register.

RETURN VALUE:	0 on success,
					-EINVAL for NULL parameter,
					-ENOENT if the table is full.

SEE ALSO:	abort, exit

END DESCRIPTION **********************************************************/
_stdlib_debug
int atexit( _exit_func func)
{
	_exit_func *f;

	if (! func)
	{
		return -EINVAL;
	}

	// find an empty slot in the table and insert the function
	f = _atexit_func_table;
	while (f <= &_atexit_func_table[_ATEXIT_FUNC_COUNT-1])
	{
		if (! *f)
		{
			*f = func;
			return 0;
		}
		++f;
	}

	return -ENOENT;
}

/*** BeginHeader exit */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
exit                                                            <stdlib.h>

SYNTAX:	void exit( int status)

DESCRIPTION:	Stops the program and returns <status> to Dynamic C.  If not
					debugging, exit() will run an infinite loop, causing a watchdog
					timeout if the watchdog is enabled.

					Before termination, exit() first calls all functions registered
					with atexit(), in the reverse order of registration.

					Next, all open streams are flushed, closed and files created
					with tmpfile() are deleted.

PARAMETER:	Exit code to pass to Dynamic C.  Can be either EXIT_SUCCESS or
				EXIT_FAILURE (for general success/failure conditions) or a specific,
				negated error macro (like -ETIME to report a timeout).

RETURN VALUE:	Function does not return.

SEE ALSO:	abort, atexit

END DESCRIPTION **********************************************************/
_stdlib_debug __root
void exit( int status)
{
	static int exit_state = 0;
	_exit_func *f;
	FILE __far *stream;

	// Use exit_state variable to track the status of the exit call.  If one of
	// the atexit() functions calls exit, or flushing/closing a stream calls
	// exit, we need to be able to resume where we left off.
	if (exit_state == 0)
	{
		exit_state = 1;
	}
	if (exit_state > 0)
	{
		#if _ATEXIT_FUNC_COUNT > 0
			if (exit_state <= _ATEXIT_FUNC_COUNT)
			{
				// Call registered atexit() functions in reverse order.
				f = &_atexit_func_table[_ATEXIT_FUNC_COUNT - exit_state];
				while (f >= _atexit_func_table)
				{
					// Increment exit_state so we'll resume at the next handler
					// if (*f)() calls exit.
					++exit_state;
					if (*f)
					{
						(*f)();
					}
					--f;
				}
			}
		#endif
		exit_state = -1;
	}

	if (exit_state >= -FOPEN_MAX)		// same as (-exit_state <= FOPEN_MAX)
	{
		// Close all open streams.
		stream = &_stdio_files[FOPEN_MAX + exit_state];
		while (stream >= _stdio_files)
		{
			--exit_state;	// if fclose() calls exit, we'll resume at the next file
			if ((stream->flags & (_FILE_FLAG_USED | _FILE_FLAG_OPEN))
									==	(_FILE_FLAG_USED | _FILE_FLAG_OPEN))
			{
				fclose( stream);
			}
			--stream;
		}
	}

	// Jump into _sys_exit() function in sys.lib, leaving stack set up so that
	// the function calling exit() appears to have called _sys_exit().
	// DEVNOTE: This passes info from the last call to exit() instead of the
	//				first.  It might be preferable to load the return address and
	//				status into static variables at exit_state 0 and then use them
	//				in the call to _sys_exit.
	#asm
		add	sp, 6			; remove auto vars from stack
		ld		hl, (sp+2)	; load <status> into HL
		jp		_sys_exit	; chain into system's exit handler
	#endasm
}

/*** BeginHeader getenv */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
getenv                                                          <stdlib.h>

SYNTAX:	char *getenv( const char far *name)

DESCRIPTION:	Stub function provided for ANSI C90 compatability.

					Search an environment list, provided by the host environment,
					for a string that matches <name>.  Dynamic C does not currently
					have an environment list, so getenv() always returns NULL.

PARAMETER 1:	Name to search for.

RETURN VALUE:	String associated with list member, which shall not be modified
					by the program.

					This function always returns NULL in Dynamic C.

END DESCRIPTION **********************************************************/
_stdlib_debug
char *getenv( const char __far *name)
{
	return NULL;
}

/*** BeginHeader system */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
system                                                          <stdlib.h>

SYNTAX:	int system( const char far *string)

DESCRIPTION:	Stub function provided for ANSI C90 compatability.

					Passes <string> to the host environment to be executed by a
					command processor.

					Dynamic C does not have a command processor, so it is not
					possible to execute commands via the system() function.

PARAMETER 1:	String to pass to the command processor or NULL to inquire
					whether a command processor exists.

RETURN VALUE:	If <string> is NULL, returns non-zero if a command processor is
					available.

					If <string> is not NULL, returns -EINVAL since there isn't a
					command processor available.

END DESCRIPTION **********************************************************/
_stdlib_debug
int system( const char __far *string)
{
	return string ? -EINVAL : 0;
}


/*****************************
 *
 *		7.10.5 Searching and sorting utilities
 *
 */

/*** BeginHeader _qsort_swap */
void _qsort_swap(__far char *a, __far char *b, int s);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION *********************************************
_qsort_swap                                                       <stdlib.h>

SYNTAX:	void _qsort_swap( far char *a, far char *b, int s);

KEYWORDS: swap

DESCRIPTION:	Swaps two far character arrays.  Support routine for qsort.

PARAMETER 1:	Base address of the first array

PARAMETER 2:	Base address of the second array

PARAMETER 3:	Number of characters to swap

RETURN VALUE:	None

END DESCRIPTION **********************************************************/
_stdlib_debug __root
void _qsort_swap(__far char *a, __far char *b, int s)
{
	// 32 clocks to set up, 39 clocks in the loop
	// near version was 76 clocks to set up and 53 clocks in the loop
	// DEVNOTE: could speed up by using BC as the index and copying 2 bytes at
	// a time (after doing a 1-off if BC is odd).
	#asm
		; px already points to first buffer
		ld		hl, (sp+@SP+s)
		test	hl
		ret	z						; zero chars count, done!

		ret	m						; negative chars count? (done!)

		ld		py, (sp+@SP+b)
		ld		bc, hl
		clr	hl
	.loop:
		ld		a, (px+hl)
		ld		e, a
		ld		a, (py+hl)
		ld		(px+hl), a
		ld		a, e
		ld		(py+hl), a
		inc	hl
		dwjnz	.loop
	#endasm
}

/* START FUNCTION DESCRIPTION ********************************************
qsort                         <stdlib.h>

NEAR SYNTAX:	void _n_qsort( void *base, unsigned nbytes, unsigned bsize,
							int (*cmp)( const void *p, const void *q));
FAR SYNTAX:		void _f_qsort( void far *base, unsigned nbytes, unsigned bsize,
							int (*cmp)( const void far *p, const void far *q));

	Unless USE_FAR_STRING_LIB is defined, qsort is defined to _n_qsort.

DESCRIPTION:
			 Quicksort with center pivot, stack control, and easy-to-change
			 comparison method.

			 This version sorts fixed-length data items. It is ideal for integers,
			 longs, floats and packed string data without delimiters.

Notes:
			 Qsort() can sort raw integers, longs, floats, pointers to structures,
			 or strings. However, the string sort is not efficient.

PARAMETER 1:	Base address of blocks to sort.

PARAMETER 2:	Number of blocks to sort.

PARAMETER 3:	Number of bytes in each block.

PARAMETER 4:	Compare routine for two block pointers, p and q, that returns
					an integer with the same rules used by Unix strcmp(p,q):

						= 0     Blocks p,q are equal
						< 0     p < q
						> 0     p > q

					Beware of using ordinary strcmp() - it requires a NULL at the
					end of each string.

					The relative order of blocks that are considered equal by the
					comparison function is unspecified.

RETURN VALUE:  None

KEYWORDS: sort

END DESCRIPTION **********************************************************/
/*** BeginHeader _n_qsort */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_n_qsort                                                        <stdlib.h>

SYNTAX:	void _n_qsort( void *base, unsigned nbytes, unsigned bsize,
							int (*cmp)( const void *p, const void *q));

See function help for qsort() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
void _n_qsort(void *abase, unsigned n, unsigned s, _compare_func_n cmp)
{
	auto int i, j, piv, hi;
	auto char *pivot;
	auto char *base;
	auto char *loaddr;
	auto char *hiaddr;

	// Need a pointer to type for pointer arithmetic
	base = abase;

	while (n > 1)
	{
		i = 0;
		j = hi = n - 1;
		piv = hi / 2; // center pivot

		pivot = base + (s * piv);
		loaddr = base;
		hiaddr = base + (s * hi);
		while (i < j)
		{
			while (i < j && cmp( loaddr, pivot) <= 0)
			{
				++i;
				loaddr += s;
			}
			while (i < j && cmp( hiaddr, pivot) >= 0)
			{
				--j;
				hiaddr -= s;
			}
			if (i < j)
			{
				_qsort_swap( loaddr, hiaddr, s);
			}
		}
		if (piv < i && cmp( loaddr, pivot) > 0)
		{
			--i;
			loaddr -= s;
		}
		if (loaddr != pivot)
		{
			_qsort_swap( loaddr, pivot, s);
		}
		// Control stack by calling qsort for smaller half and doing
		// tail recursion for larger half.
		if (i <= hi - i)
		{
			if (i > 1)
			{
				_n_qsort( base, i, s, cmp);
			}
			base = loaddr + s;
			n = hi - i;
		}
		else
		{
			if (hi - i > 1)
			{
				_n_qsort( loaddr + s, hi - i, s, cmp);
			}
			n = i;
		}
	}
}

/*** BeginHeader _f_qsort */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_f_qsort                                                        <stdlib.h>

SYNTAX:	void _f_qsort( void far *base, unsigned nbytes, unsigned bsize,
							int (*cmp)( const void far *p, const void far *q));

See function help for qsort() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
void _f_qsort(__far void *abase, unsigned n, unsigned s, int (*cmp) ())
{
	auto int i, j, piv, hi;
	auto char __far *pivot;
	auto char __far *base;
	auto char __far *loaddr;
	auto char __far *hiaddr;

	// Need a pointer to type for pointer arithmetic
	base = abase;

	while (n > 1)
	{
		i = 0;
		j = hi = n - 1;
		piv = hi / 2; // center pivot

		pivot = base + (s * piv);
		loaddr = base;
		hiaddr = base + (s * hi);
		while (i < j)
		{
			while (i < j && cmp( loaddr, pivot) <= 0)
			{
				++i;
				loaddr += s;
			}
			while (i < j && cmp( hiaddr, pivot) >= 0)
			{
				--j;
				hiaddr -= s;
			}
			if (i < j)
			{
				_qsort_swap( loaddr, hiaddr, s);
			}
		}
		if (piv < i && cmp( loaddr, pivot) > 0)
		{
			--i;
			loaddr -= s;
		}
		if (loaddr != pivot)
		{
			_qsort_swap( loaddr, pivot, s);
		}
		// Control stack by calling qsort for smaller half and doing
		// tail recursion for larger half.
		if (i <= hi - i)
		{
			if (i > 1)
			{
				_f_qsort( base, i, s, cmp);
			}
			base = loaddr + s;
			n = hi - i;
		}
		else
		{
			if (hi - i > 1)
			{
				_f_qsort( loaddr + s, hi - i, s, cmp);
			}
			n = i;
		}
	}
}


/* START FUNCTION DESCRIPTION ********************************************
bsearch                                                         <stdlib.h>

NEAR SYNTAX:	void *_n_bsearch( const void *key, const void *base,
									size_t nmemb, size_t membsize,
									int (*cmp)( const void *p, const void *q));

FAR SYNTAX:		void far *_f_bsearch( const void far *key, const void far *base,
									size_t nmemb, size_t membsize,
									int (*cmp)( const void far *p, const void far *q));

	Unless USE_FAR_STRING_LIB is defined, bsearch is defined to _n_bsearch.

DESCRIPTION:	Searches a sorted array of <nmemb> objects, of <membsize> bytes,
					at the address <base>, for the object <key>.

PARAMETER 1:	Pointer to key to search for.

PARAMETER 2:	Pointer to sorted array to search.

PARAMETER 3:	Number of elements in the array.

PARAMETER 4:	Size of each element in the array.

PARAMETER 5:	Compare routine for two block pointers, p and q, that returns
					an integer with the same rules used by Unix strcmp(p,q):

						= 0     Blocks p,q are equal
						< 0     p < q
						> 0     p > q

					Beware of using ordinary strcmp() - it requires a null at the
					end of each string.

RETURN VALUE:  The address of the matching object if <key> is found.
					NULL if <key> was not found.
					If multiple elements match the <key>, any one of them can
					be returned.

END DESCRIPTION **********************************************************/
/*** BeginHeader _n_bsearch */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_n_bsearch                                                      <stdlib.h>

SYNTAX:	void *_n_bsearch( const void *key, const void *base,
									size_t nmemb, size_t membsize,
									int (*cmp)( const void *p, const void *q));

See function help for bsearch() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
void *_n_bsearch( const void *key, const void *base,
								size_t nmemb, size_t membsize, _compare_func_n compar)
{
	auto size_t low, high, guess;
	auto const void *guessptr;
	auto int result;

	low = 0;
	high = nmemb;
	while (1)				// search [ low, high )
	{
		if (low == high)				// not in list
		{
			return NULL;
		}
		guess = low + ((high - low) >> 1);
		guessptr = (const char *) base + guess * membsize;
		result = compar( key, guessptr);
		if (result == 0)				// found
		{
			return (void *)guessptr;
		}
		else if (result < 0)			// in lower half of list
		{
			high = guess;
		}
		else								// in upper half of list
		{
			low = guess + 1;
		}
	}
}

/*** BeginHeader _f_bsearch */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
_f_bsearch                                                      <stdlib.h>

SYNTAX:	void far *_f_bsearch( const void far *key, const void far *base,
									size_t nmemb, size_t membsize,
									int (*cmp)( const void far *p, const void far *q));

See function help for bsearch() for description, parameters and return value.

END DESCRIPTION **********************************************************/
_stdlib_debug
void __far *_f_bsearch( const void __far *key, const void __far *base,
								size_t nmemb, size_t membsize, _compare_func_f compar)
{
	auto size_t low, high, guess;
	auto const void __far *guessptr;
	auto int result;

	low = 0;
	high = nmemb;
	while (1)				// search [ low, high )
	{
		if (low == high)				// not in list
		{
			return NULL;
		}
		guess = low + ((high - low) >> 1);
		guessptr = (const char __far *) base + guess * membsize;
		result = compar( key, guessptr);
		if (result == 0)				// found
		{
			return (void __far *)guessptr;
		}
		else if (result < 0)			// in lower half of list
		{
			high = guess;
		}
		else								// in upper half of list
		{
			low = guess + 1;
		}
	}
}


/*****************************
 *
 *		7.10.6 Integer arithmetic functions
 *
 */

/*** BeginHeader abs */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
abs                                                             <stdlib.h>

SYNTAX:	int abs(int x)

DESCRIPTION:	Computes the absolute value of an integer arg.

PARAMETER 1:	Integer for which the absolute value is to be returned.

KEYWORDS:		math

RETURN VALUE:	Absolute value of the argument.

SEE ALSO:	labs (for long), fabs (for float)

END DESCRIPTION **********************************************************/
#asm __root __nodebug
abs::
	test	hl
	ret	p
	neg	hl
	ret
#endasm

/*** BeginHeader div */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
div                                                             <stdlib.h>

SYNTAX:	div_t div( int numer, int denom)

DESCRIPTION:	Computes the quotient and remainder of (<numer> / <denom>).  If
					the division is inexact, the resulting quotient is truncated
					toward zero.

						int numer, denom;
						div_t answer;

						answer = div( numer, denom);
						// answer.quot * denom + answer.rem == numer

					Sign of result:

						numer  denom  .quot  .rem
						  +      +      +      +
						  -      +      -      -
						  +      -      -      +
						  -      -      +      -

					Since div() performs a single division, it is more effient than:

						answer.quot = numer / denom;
						answer.rem = numer % denom;

PARAMETER 1:	Numerator for division.

PARAMETER 2:	Denominator for division.

RETURN VALUE:	Returns a structure of type div_t, comprising two members of
					type int, <quot> (the quotient) and <rem> (the remainder).

SEE ALSO:	ldiv

END DESCRIPTION **********************************************************/
_stdlib_debug __root
div_t div( int numer, int denom)
{
	#asm
		; numer is in HL, load denom into de
		ex		de, hl					; C.DIV needs dividend in DE, divisor in HL
		ld		hl, (sp+@SP+denom)
		call	c_div			; DE / HL, returns quotient in HL, modulus in DE
		; quotient and modulus returned on stack, above return address
		; and parameters passed to function
		ld		(sp+6), hl				; store quotient
		ex		de, hl
		ld		(sp+8), hl				; store modulus
		ret
	#endasm

/* Portable version (performs two divides)
	div_t answer;

	answer.quot = numer / denom;
	answer.rem = numer % denom;

	return answer;
*/
}


/*** BeginHeader labs */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
labs                                                            <stdlib.h>

SYNTAX:	long labs(long x)

DESCRIPTION:	Computes the absolute value of a long integer arg.

PARAMETER 1:	Long integer for which the absolute value is to be returned.

KEYWORDS:		math

RETURN VALUE:	Absolute value of the argument.

SEE ALSO:	abs (for int), fabs (for float)

END DESCRIPTION **********************************************************/
#asm __root __nodebug
labs::
	test	bcde
	ret	p
	neg	bcde
	ret
#endasm


/*** BeginHeader ldiv */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
ldiv                                                            <stdlib.h>

SYNTAX:	ldiv_t ldiv( long int numer, long int denom)

DESCRIPTION:	Computes the quotient and remainder of (<numer> / <denom>).
					If the division is inexact, the resulting quotient is
					truncated toward zero.

						long numer, denom;
						ldiv_t answer;

						answer = ldiv( numer, denom);
						// answer.quot * denom + answer.rem == numer

					Sign of result:

						numer  denom  .quot  .rem
						  +      +      +      +
						  -      +      -      -
						  +      -      -      +
						  -      -      +      -

					Since ldiv() performs a single division, it is more
               effient than:

						answer.quot = numer / denom;
						answer.rem = numer % denom;

               Note: an unsigned long version is (uldiv()) is also available,
               however uldiv is not presently an ANSI standard function.

PARAMETER 1:	Numerator for division.

PARAMETER 2:	Denominator for division.

RETURN VALUE:	Returns a structure of type ldiv_t, comprising two members of
					type long, <quot> (the quotient) and <rem> (the remainder).

SEE ALSO:	div, uldiv

END DESCRIPTION **********************************************************/
_stdlib_debug __root
ldiv_t ldiv( long int numer, long int denom)
{
	#asm
		; numerator is in bcde, needs to be on stack
		push	bcde
		ld		bcde, (sp+4+denom)
		call	L_div			; returns quotient in BCDE, modulus in HL'HL
								; Note that L_div also removes numerator from stack

		; quotient and modulus returned on stack, above return address
		; and parameters passed to function
		ld		(sp+10), bcde				; store quotient
		ex		jk, hl'
		ld		(sp+14), jkhl				; store modulus
		ret
	#endasm
/* Portable version (performs two divides)

	ldiv_t answer;

	answer.quot = numer / denom;
	answer.rem = numer % denom;

	return answer;
*/

}


/*****************************
 *
 *		7.10.7 Multibyte character functions
 *
 */

/*** BeginHeader mblen, mbtowc, wctomb, mbstowcs, wcstombs */
/*** EndHeader */
// Fatal error if code needs any of the above functions.  Not supported.
#error Dynamic C does not include support for multibyte characters.  Contact
#fatal your Digi/Rabbit sales representative if you require this feature.


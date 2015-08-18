/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*** BeginHeader */
#include <locale.h>

#ifdef LOCALE_DEBUG
	#define _locale_debug	__debug
#else
	#define _locale_debug	__nodebug
#endif
/*** EndHeader */

/*** BeginHeader _locale_C */
extern const struct lconv _locale_C;
/*** EndHeader */
/*
	DO NOT MODIFY _locale_C.  It is defined by the ANSI C90 standard.  To
	make changes, modify the member values of the variable _lc_numeric.

	Note that Dynamic C does not currently make use of this structure.
*/
#include <limits.h>			// for CHAR_MAX
const struct lconv _locale_C =
{
	".",				// decimal_point
   "",				// thousands_sep
   "",				// grouping
   "",				// currency_symbol
   "",				// mon_decimal_point
   "",				// mon_thousands_sep
   "",				// mon_grouping
   CHAR_MAX,		// frac_digits
   "",				// positive_sign
   CHAR_MAX,		// p_cs_precedes
   CHAR_MAX,		// p_sep_by_space
   CHAR_MAX,		// p_sign_posn
   "",				// negative_sign
   CHAR_MAX,		// n_cs_precedes
   CHAR_MAX,		// n_sep_by_space
   CHAR_MAX,		// n_sign_posn
   "",				// int_curr_symbol
   CHAR_MAX,		// int_frac_digits
   CHAR_MAX,		// int_p_cs_precedes
   CHAR_MAX,		// int_p_sep_by_space
   CHAR_MAX,		// int_p_sign_posn
   CHAR_MAX,		// int_n_cs_precedes
   CHAR_MAX,		// int_n_sep_by_space
   CHAR_MAX,		// int_n_sign_posn
};

/*** BeginHeader _locale */
/*** EndHeader */
struct lconv _locale = _locale_C;


/*** BeginHeader setlocale */
/*** EndHeader */
_locale_debug
char *setlocale(int category, const char *locale)
{
	// For NULL locale, "" locale (default) and C locale, respond with "C"
	if (! locale || ! *locale || strcmp( locale, "C") == 0)
	{
		return "C";
	}

	// can't honor request, return NULL as error response
	return NULL;
}


/*** BeginHeader localeconv */
/*** EndHeader */
_locale_debug
struct lconv *(localeconv)(void)
{
	return &_locale;
}
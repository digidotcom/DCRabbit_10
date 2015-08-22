/*
	locale.c

	Copyright (c) 2010 Digi International Inc., All Rights Reserved

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
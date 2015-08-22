/*
	locale.h

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

/*
	C90 - 7.4 Localization
*/

/*
	Note that Dynamic C does not have localization support beyond the "C" locale.
	The locale.h and locale.c files are (at this point) just stubs.

	If you require localization support for a project, please contact your
	Digi/Rabbit sales rep with your requirements.
*/

#ifndef __LOCALE_H
#define __LOCALE_H

	#define LC_ALL				0
	#define LC_COLLATE		1
	#define LC_CTYPE			2
	#define LC_MONETARY		3
	#define LC_NUMERIC		4
	#define LC_TIME			5

	#define NULL				(void *) 0

	// This struct lconv conforms to C99 standard.
	typedef struct lconv {
	// LC_NUMERIC:  These members apply to non-monetary values
	   char *decimal_point;			// decimal point (".")
	   char *thousands_sep;			// separator for digits left of decimal (",")
	   char *grouping;				// size of digit groups left of decimal ({3, 0})

	// LC_MONETARY:  The following members apply to monetary values
	   char *currency_symbol;		// local currency symbol ("$")
	   char *mon_decimal_point;	// decimal point (".")
	   char *mon_thousands_sep;	// separator for digits left of decimal (",")
	   char *mon_grouping;			// size of digit groups ({3, 0})
	   char frac_digits;				// digits to display to right of decimal (2)

											// Displaying positive monetary values
	   char *positive_sign;			// positive sign ("+")
	   char p_cs_precedes;			// currency symbol precedes value (1)
	   char p_sep_by_space;			// see _LC_SPACE_SEP_* macros below (0)
	   char p_sign_posn;				//	see _LC_SIGN_* macros below (4)

											// Displaying negative monetary values
	   char *negative_sign;			// negative sign ("-")
	   char n_cs_precedes;			// currency symbol precedes value (1)
	   char n_sep_by_space;			// see _LC_SPACE_SEP_* macros below (0)
	   char n_sign_posn;				//	see _LC_SIGN_* macros below (4)

											// Displaying international monetary values
	   char *int_curr_symbol;		// ISO 4217 currency symbol ("USD ")
	   char int_frac_digits;		// digits to display to right of decimal (2)
	   char int_p_cs_precedes;		// currency symbol precedes positive value (1)
	   char int_p_sep_by_space;	// see _LC_SPACE_SEP_* macros below (0)
	   char int_p_sign_posn;		//	see _LC_SIGN_* macros below (4)
	   char int_n_cs_precedes;		// currency symbol precedes negative value (1)
	   char int_n_sep_by_space;	// see _LC_SPACE_SEP_* macros below (0)
	   char int_n_sign_posn;		//	see _LC_SIGN_* macros below (4)
	} _lc_numeric_t;

	// Values for *_sign_posn members of struct lconv.
	#define _LC_SIGN_PARENS		0	// parentheses surround value and curr. symbol
	#define _LC_SIGN_PRECEDE	1	// sign precedes value & currency symbol
	#define _LC_SIGN_FOLLOW		2	// sign follows value & currency symbol
	#define _LC_SIGN_P_SYMBOL	3	// sign immediately precedes currency symbol
	#define _LC_SIGN_F_SYMBOL	3	// sign immediately follows currency symbol

	// Values for *_sep_by_space members of struct lconv.
	#define _LC_SPACE_SEP_NO	0	// no space separates symbol from value
	#define _LC_SPACE_SEP_YES	1	// space separates symbol (and sign) from value
	#define _LC_SPACE_SEP_SIGN	2	// space separates sign from symbol or value

	// 7.4.1 Locale control
	char *setlocale(int category, const char *locale);

	// 7.4.2 Numeric formatting convention inquiry
	extern struct lconv _locale;
	struct lconv *localeconv(void);
	#define localeconv()		(&_locale)

	#use "locale.c"

#endif
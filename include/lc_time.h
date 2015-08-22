/*
	lc_time.h

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
	Support for time and date localization.
*/

#ifndef __LC_TIME_H
#define __LC_TIME_H

	// Struct used for locale-related names (not defined by C90)
	typedef struct _lc_time_t {
		char	*ampm;				// "AM|PM"
		char	*months_abr;		// "Jan|Feb|Mar|..."
		char	*months_full;		// "January|February|March|April|..."
		char	*days_abr;			// "Sun|Mon|Tue|Wed|Thu|Fri|Sat"
		char	*days_full;			// "Sunday|Monday|Tuesday|..."
		char	*tz;					// 3-letter zone, standard & daylight ("EST|EDT")
		char	*format_date;		// format string for %x (e.g. "%m/%d/%Y")
		char	*format_time;		// format string for %X (e.g. "%H:%M:%S")
		char	*format_datetime;	// format string for %c (e.g. "%m/%d/%Y %H:%M:%S")
	} _lc_time_t;

	extern const	_lc_time_t		_lc_time_C;			// values for C locale
	extern 			_lc_time_t		_lc_time;			// current values

	// Internal API for use by other libraries.
	size_t _strftime( char __far *s, size_t maxsize, const char __far *format,
					const struct tm __far *timeptr, const _lc_time_t __far *tnames);

	#use "lc_time.lib"

#endif
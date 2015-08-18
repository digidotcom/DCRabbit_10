/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
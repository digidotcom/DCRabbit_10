/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*** BeginHeader */
#include <lc_time.h>			// _strftime() called by strftime()
#include <time.h>

#ifdef TIME_DEBUG
	#define _time_debug	__debug
#else
	#define _time_debug	__nodebug
#endif

// DEVNOTE: We should force RTC_IS_UTC and make use of time zones.  Eventually,
//				it would be great to have enough timezone information to correctly
//				adjust for daylight savings.

/*** EndHeader */

/*** BeginHeader clock */
#define clock()	(0+TICK_TIMER)
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
clock                                                             <time.h>

SYNTAX:	clock_t clock(void)

DESCRIPTION:	Returns the number of clock ticks of elapsed processor time,
					counting from program startup.

RETURN VALUE:	Number of ticks since startup.  The macro CLOCKS_PER_SEC defines
					the number of ticks in a second.

SEE ALSO:	difftime, mktime, time, asctime, ctime, gmtime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
clock_t (clock)( void)
{
	return TICK_TIMER;
}


/*** BeginHeader difftime */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
difftime                                                          <time.h>

SYNTAX:	double difftime( time_t time1, time_t time0)

DESCRIPTION:	Computes the difference (in seconds) between two calendar times.

PARAMETER 1:	A time_t value (seconds since 1/1/1980).

PARAMETER 1:	The time_t value to subtract from <time1>.

RETURN VALUE:	<time1>-<time0> as a floating point value.

SEE ALSO:	clock, mktime, time, asctime, ctime, gmtime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
double difftime( time_t time1, time_t time0)
{
	time_t	delta;

	// try to use integer math for subtraction
	if (time1 == time0)
	{
		return 0.0;
	}
	else if (time1 > time0)
	{
		delta = time1 - time0;
		if (delta <= LONG_MAX)
		{
			return (double) delta;
		}
	}
	else
	{
		delta = time0 - time1;
		if (delta <= LONG_MAX)
		{
			return -(double) delta;
		}
	}

	return (double)time1 - (double)time0;
}

/*** BeginHeader __jom */
extern const unsigned int __jom[2][12];
/*** EndHeader */
// Number of days since January 1st for the 1st of each month.
// __jom[a][b]:
//		a == 0 ? leap year : non-leap year
//		b is number of months since January
const unsigned int __jom[2][12] =
{
	{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},		// leap year
	{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}		// non-leap year
};
/*** BeginHeader __dom */
extern const char __dom[2][12];
// this leap year calculation works for 1901 to 2099
#define __DAYSINMONTH(m,y)		(__dom[(y & 0x03) != 0][m])
/*** EndHeader */
// Number of days in each month.
// __dom[a][b]:
//		a == 0 ? leap year : non-leap year
//		b is the number of months since January
const char __dom[2][12] =
{
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},				// leap year
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}				// non-leap year
};

/*** BeginHeader mktime */
/*** EndHeader */
/* START _FUNCTION DESCRIPTION ********************************************
_mktime_normalize                                                 <time.c>

					If __ANSI_TIME__ is defined, ANSI syntax is used.

ANSI SYNTAX:	void _mktime_normalize( int *major, int *minor, int count)
LEGACY SYNTAX:	void _mktime_normalize( char *major, char *minor, int count)

DESCRIPTION:	Private function used by mktime() to normalize values.  Adjusts
					<minor> until it is in the range [0, <count>).  Adds or
					subtracts 1 from <major> for every <count> values added to or
					subtracted from <minor>.

PARAMETER 1:	Pointer to the "major" value.

PARAMETER 2:	Pointer to the "minor" value.

PARAMETER 3:	Number of "minor" objects per "major" object.

EXAMPLE:		_mktime_normalize( &t.tm_min, &t.tm_sec, 60);

RETURN VALUE:	None

END DESCRIPTION **********************************************************/
#include <stdlib.h>				// for div()
_time_debug
#ifdef __ANSI_TIME__
	void _mktime_normalize( int *major, int *minor, int count)
#else
	// Legacy DC struct uses 8-bits for members of struct tm
	void _mktime_normalize( char *major, char *minor, int count)
#endif
{
	div_t		d;

	// Normalize minor such that it is in the range [0, count) by adding
	// count to minor (and subtracting 1 from major) or subtracting count
	// from minor (and adding 1 to major).
	d = div( *minor, count);
	if (d.rem < 0)
	{
		d.rem += count;
		d.quot -= 1;
	}

	#ifdef __ANSI_TIME__
		*minor = d.rem;
		*major += d.quot;
	#else
		*minor = (char) d.rem;
		*major = (char) (*major + d.quot);
	#endif
}

/* START FUNCTION DESCRIPTION ********************************************
mktime                                                            <time.h>

SYNTAX:	time_t mktime( struct tm far *timeptr)

DESCRIPTION:	Normalizes <timeptr> so all values are within their valid
					ranges (e.g., minutes between 0 and 59, correct days per month,
					etc.).  Sets the tm_wday and (if __ANSI_TIME__ or __ANSI_STRICT__
					is defined) the tm_yday members of <timeptr>.

					This function is useful for performing math on dates.  For
					example, to find the correct date for 90 days from today:

						struct tm	t, *tp;
						time_t		now;

						now = time( NULL);
						tp = localtime( &now);
						if (! tp) printf( "error calling localtime()\n");
						else {
							t = *tp;						// make a copy of struct
							t.tm_mday += 90;			// add 90 days from now
							mktime( &t);				// normalize
							printf( "In 90 days it will be %s\n", asctime( &t));
						}

					Note that mktime() cannot represent times from before the
					Rabbit's epoch of January 1, 1980.  Dynamic C does not
					support Daylight Savings Time, so mktime() does not modify
					tm_isdst.

struct tm:		The "struct tm" object holds a date/time broken down into
					component parts.  Past versions of Dynamic C used a declaration
					that isn't compatible with the ANSI/ISO C standard.

					If the macros __ANSI_TIME__ or __ANSI_STRICT__ are defined,
					struct tm is declared as:

		struct tm
		{
			int tm_sec;		// seconds after minute [0, 60] (60 = leap second)
			int tm_min;		// minutes after the hour [0, 59]
			int tm_hour;	// hours since midnight [0, 23]
			int tm_mday;	// day of the month [1, 31]
			int tm_mon;		// months since January [0, 11]
			int tm_year;	// years since 1900
			int tm_wday;	// days since Sunday [0, 6]
			int tm_yday;	// days since January 1 [0, 365]
			int tm_isdst;	// Daylight Savings Time flag
								// >0 if in effect, 0 if not in effect, <0 if unknown
		};

					If __ANSI_TIME__ and __ANSI_STRICT__ are not defined, the legacy
					declaration is used:

		struct tm
		{
			char tm_sec;	// seconds after minute [0, 60] (60 = leap second)
			char tm_min;	// minutes after the hour [0, 59]
			char tm_hour;	// hours since midnight [0, 23]
			char tm_mday;	// day of the month [1, 31]
			char tm_mon;	// months since January [1, 12]
			char tm_year;	// years since 1900
			char tm_wday;	// days since Sunday [0, 6]
		};

					tm_mon in ANSI Standard struct ranges from 0 to 11.
					tm_mon in the legacy struct ranges from 1 to 12.

					The ANSI Standard struct includes tm_yday and tm_isdst members.

PARAMETER 1:	Pointer to broken-down time to normalize and convert to time_t.

RETURN VALUE:	The specified calendar time encoded as a value of type time_t.
					Returns -1 if the calendar time cannot be represented.

SEE ALSO:	clock, difftime, time, asctime, ctime, gmtime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
time_t mktime( struct tm __far *timeptr)
{
	// Work with local copy of timeptr to reduce amount of pointer dereferencing.
	struct tm t = *timeptr;

	int years;						// years since 1980
	unsigned int days;			// days since 1/1/1980
	unsigned int yday;			// day of the year
	int daysinmonth;
	int pastleapdays;

	#ifndef __ANSI_TIME__
		// switch to 0-indexed tm_mon
		--t.tm_mon;
	#endif

	// normalize min/sec, hour/min, mday/hour and year/mon
	_mktime_normalize( &t.tm_min, &t.tm_sec, 60);	// 60 sec/min
	_mktime_normalize( &t.tm_hour, &t.tm_min, 60);	// 60 min/hour
	_mktime_normalize( &t.tm_mday, &t.tm_hour, 24);	// 24 hour/day
	_mktime_normalize( &t.tm_year, &t.tm_mon, 12);	// 12 mon/yr

	// normalize mon/mday by incrementing through the months

	// if day of month is < 1, roll back to previous month
	while (t.tm_mday < 1)
	{
		if (t.tm_mon == 0)
		{
			// roll back January to December of previous year
			t.tm_mon = 11;
			t.tm_year -= 1;
		}
		else
		{
			t.tm_mon -= 1;
		}
		t.tm_mday += __DAYSINMONTH( t.tm_mon, t.tm_year);
	}

	// if day of month is > number of days in this month, roll to next month
	while (t.tm_mday > (daysinmonth = __DAYSINMONTH(t.tm_mon, t.tm_year)))
	{
		t.tm_mday -= daysinmonth;
		if (t.tm_mon == 11)
		{
			// roll from December to January of next year
			t.tm_mon = 0;
			t.tm_year += 1;
		}
		else
		{
			t.tm_mon += 1;
		}
	}

	// Date is normalized at this point -- year/month/day hour/minute/second

	years = t.tm_year - STARTYR;
	if (years < 0)
	{
		// year is before our epoch, return error
		return -1;
	}

	// determine day of the year using month and day of month
	yday = __jom[(years & 0x03) != 0][t.tm_mon] + t.tm_mday - 1;

	// number of past leap days is:
	// the number of years / 4, plus 1 if this is not a leap year
	pastleapdays = (years >> 2) + (years & 0x03 ? 1 : 0);

	// calculate days since 1/1/1980 (a Tuesday)
	// 365 days per year since 1980, plus leap days, plus the current day of year
	days = (years * 365U) + pastleapdays + yday;
	#ifdef __ANSI_TIME__
		t.tm_yday = yday;
		t.tm_wday = (days + 2) % 7;
	#else
		// switch back to non-standard tm_mon
		++t.tm_mon;
		t.tm_wday = (char) ((days + 2) % 7);
	#endif

	// Copy normalized struct tm back to caller and return seconds since 1/1/80.
	*timeptr = t;
	return days * 86400LU + t.tm_hour * 3600LU + t.tm_min * 60 + t.tm_sec;
}


/*** BeginHeader time */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
time                                                              <time.h>

SYNTAX:	time_t time( time_t far *timer)

DESCRIPTION:	Determines the current calendar date/time.

PARAMETER 1:	Pointer to a time_t object to hold a copy of the return value.

RETURN VALUE:	Returns the best approximation to the current calendar time.
					The value (time_t)-1 is returned if the calendar time is not
					available.  If <timer> is not NULL, the return value is also
					assigned to the object it points to.

SEE ALSO:	clock, difftime, mktime, asctime, ctime, gmtime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
time_t time( time_t __far *timer)
{
	if (timer)
	{
		return *timer = SEC_TIMER;
	}
	return SEC_TIMER;
}


/*** BeginHeader asctime */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
asctime                                                           <time.h>

SYNTAX:	char *asctime( const struct tm far *timeptr)

DESCRIPTION:	Converts the broken-down time in <timeptr> into a string in
					the form

						Sun Sep 16 01:03:52 1973\n\0

					Equivalent to calling strftime() with a format string of
					"%a %b %e %H:%M:%S %Y\n".

					Note that ctime() and asctime() share the same static character
					buffer.  A call to either function will alter the contents of
					the string pointed to by previous ctime() and asctime() calls.

PARAMETER 1:	Time to convert.

RETURN VALUE:	Pointer to a static buffer with the time in string form.

SEE ALSO:	clock, difftime, mktime, time, ctime, gmtime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
char *asctime( const struct tm __far *timeptr)
{
	static char buffer[26];

	_strftime( buffer, sizeof(buffer), "%a %b %e %H:%M:%S %Y\n", timeptr,
		&_lc_time_C);

	return buffer;
}


/*** BeginHeader ctime */
#define ctime( timer)	asctime( localtime( timer))
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
ctime                                                             <time.h>

SYNTAX:	char *ctime( const time_t far *timer)

DESCRIPTION:	Converts the calendar time pointed to by <timer> to local time
					in the form of a string.  It is equivalent to:

							asctime( localtime( timer));

					Note that ctime(), localtime() and gmtime() all share the same
					static struct tm.  A call to any of those functions will alter
					the contents of the struct tm pointed to by previous localtime()
					and gmtime() calls.

					Note that ctime() and asctime() share the same static character
					buffer.  A call to either function will alter the contents of
					the string pointed to by previous ctime() and asctime() calls.

PARAMETER 1:	Pointer to time to convert.

RETURN VALUE:	The string returned by asctime().

SEE ALSO:	clock, difftime, mktime, time, asctime, gmtime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
char *(ctime)( const time_t __far *timer)
{
	return asctime( localtime( timer));
}


/*** BeginHeader gmtime, _gmtime, _gmtime_tm, _f_gmtime */
#define gmtime(t)		_gmtime(&_gmtime_tm, *(t))
extern struct tm _gmtime_tm;
struct tm __far *_f_gmtime( struct tm __far *dest_tm, time_t timer);
/*** EndHeader */
struct tm _gmtime_tm;

/* START FUNCTION DESCRIPTION ********************************************
gmtime                                                            <time.h>

SYNTAX:	struct tm *gmtime( const time_t far *timer)

DESCRIPTION:	Converts the calendar time at <timer> into a broken-down time,
					expressed as Coordinated Universal Time (UTC).

					Note that ctime(), localtime() and gmtime() all share the same
					static struct tm.  A call to any of those functions will alter
					the contents of the struct tm pointed to by previous localtime()
					and gmtime() calls.

PARAMETER 1:	Non-NULL pointer to time to convert.

RETURN VALUE:	Pointer to broken-down time.

SEE ALSO:	clock, difftime, mktime, time, asctime, ctime, localtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
struct tm *(gmtime)( const time_t __far *timer)
{
	return _gmtime( &_gmtime_tm, *timer);
}

// Core function for gmtime() and mktm() -- break down timer and put in dest_tm.
_time_debug
struct tm __far *_f_gmtime( struct tm __far *dest_tm, time_t timer)
{
	struct tm t;
	uldiv_t	usplit;
	ldiv_t	lsplit;
	div_t		isplit;

	unsigned days;
	int diy;
	const char *dom;

	// split seconds into days and seconds
	usplit = uldiv( (unsigned long) timer, 86400uL);
	days = (unsigned) usplit.quot;

	///// Populate Time Fields

	// split seconds into minutes and seconds
	lsplit = ldiv( (long) usplit.rem, 60);
	// split minutes into hours and minutes
	isplit = div( (int) lsplit.quot, 60);

	t.tm_sec = (int) lsplit.rem;
	t.tm_min = isplit.rem;
	t.tm_hour = isplit.quot;

	///// Populate Date Fields

	t.tm_wday = (days + 2) % 7;
	t.tm_year = STARTYR;
	while (days >= (diy = 365 + ((t.tm_year & 0x03) == 0)))
	{
		days -= diy;
		++t.tm_year;
	}

	#ifdef __ANSI_TIME__
		// set members only present in ANSI version of struct tm
		t.tm_isdst = 0;
		t.tm_yday = days;
	#endif

	t.tm_mon = 0;
	dom = __dom[(t.tm_year & 0x03) != 0];
	while (days >= dom[t.tm_mon])
	{
		days -= dom[t.tm_mon];
		++t.tm_mon;
	}
	t.tm_mday = days + 1;

	#ifndef __ANSI_TIME__
		// legacy time starts counting month from 1
		++t.tm_mon;
	#endif

	// copy back to caller and return
	*dest_tm = t;
	return dest_tm;
}

_time_debug
struct tm *_gmtime( struct tm *dest_tm, time_t timer)
{
	_f_gmtime(dest_tm, timer);
	return dest_tm;
}

/*** BeginHeader localtime */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
localtime                                                         <time.h>

SYNTAX:	struct tm *localtime( const time_t far *timer)

DESCRIPTION:	Converts the calendar time at <timer> into a broken-down time,
					adjusted for the current timezone.  Uses the function
					rtc_timezone(), which uses either the timezone provided by
					the DHCP server, or by the macro TIMEZONE.

					Note that ctime(), localtime() and gmtime() all share the same
					static struct tm.  A call to any of those functions will alter
					the contents of the struct tm pointed to by previous localtime()
					and gmtime() calls.

PARAMETER 1:	Non-NULL pointer to time to convert.

RETURN VALUE:	Pointer to broken-down time or NULL if <timer> was NULL.

SEE ALSO:	clock, difftime, mktime, time, asctime, ctime, gmtime, strftime

END DESCRIPTION **********************************************************/
_time_debug
struct tm *localtime( const time_t __far *timer)
{
	struct tm *tm;
	time_t	local;
	long		seconds;
	int		minutes;

	if (! timer)
	{
		return NULL;
	}

	rtc_timezone( &seconds, NULL);
	local = *timer + seconds;
	tm = gmtime( &local);

	return tm;
}

/*** BeginHeader strftime */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
strftime                                                          <time.h>

SYNTAX:	size_t strftime( char far *s, size_t maxsize, const char far *format,
																const struct tm far *timeptr)

DESCRIPTION:	Formats a time as a printable string, using a format string
					(similar, but different than the formats used by printf).

PARAMETER 1:	Buffer to hold formatted string.

PARAMETER 2:	Size of buffer.

PARAMETER 3:	Format to use.  Consists of zero or more conversion specifiers
					and ordinary characters.  A conversion specifier consists of
					a '%' character followed by a single character that determines
					what is written to the buffer.  All other characters, including
					the null terminator, are copied to the buffer unchanged.

					Each conversion specifier is replaced by appropriate characters
					described in the following list.  The appropriate characters
					are determined by the LC_TIME category of the current locale
					and the values in the struct tm pointed to by <timeptr>.

					Note that Dynamic C only includes support for the "C" locale.

			%a - the locale's abbreviated weekday name.
			%A - the locale's full weekday name.
			%b - the locale's abbreviated month name.
			%B - the locale's full month name.
			%c - the locale's appropriate date and time representation.
			%C - the century (year divided by 100 and truncated to an integer).
			%d - the day of the month as a decimal number (01-31).
			%D - equivalent to "%m/%d/%y".
			%e - the day of the month as a decimal number, leading space ( 1-31).
			%F - equivalent to "%Y-%m-%d", the ISO 8601 date format.
			%h	- equivalent to "%b".
			%H - the hour (24-hour clock) as a decimal number (00-23).
			%I - the hour (12-hour clock) as a decimal number (01-12).
			%j - the day of the year as a decimal number (001-366 ).
			%m - the month as a decimal number (01-12).
			%M - the minute as a decimal number (00-59).
			%n	- replaced by a newline character ('\n').
			%p - the locale's equivalent of either AM or PM .
			%R - equivalent to "%H:%M".
			%S - the second as a decimal number (00-60).
			%t	- replaced by a horizontal-tab ('\t').
			%T - equivalent to "%H:%M:%S", the ISO 8601 time format.
			%u - replaced by the ISO 8601 weekday as a decimal number (1-7),
						where Monday is 1.
			%U - the week number of the year (the first Sunday as the
						first day of week 1) as a decimal number (00-53).
			%w - the weekday as a decimal number (0-6), where Sunday is 0.
			%W - the week number of the year (the first Monday as the
						first day of week 1) as a decimal number (00-53).
			%x - the locale's appropriate date representation.
			%X - the locale's appropriate time representation.
			%y - the year without century as a decimal number (00-99).
			%Y - the year with century as a decimal number.
			%Z - the time zone name, or by no characters if no time
						zone is determinable.
			%% - replaced by "%".

  			If a conversion specification is not one of the above, it will be
  			replaced by a single question mark character ('?').

			Formats %j, %U, %W and %Z are only available if the macros
			__ANSI_TIME__ or __ANSI_STRICT__ are defined.  The legacy Dynamic C
			"struct tm" doesn't include the necessary tm_yday and tm_isdst members
			required for these formats.

			This implementation supports all specifiers listed are part of the
			ANSI C89/ISO C90 spec.  Additionally, it supports the following
			specifiers from the C99 spec:  %C, %D, %e, %F, %h, %n, %R, %t, %T.
			It does not support the following C99 specifiers:  %g, %G, %r, %V, %z.


PARAMETER 4:	Time to print.

RETURN VALUE:	The number of characters written to <s>, not including the null
					terminator.  If the destination buffer was not large enough,
					to hold the formatted string, strftime() returns 0 and the
					contents of <s> are indeterminate.

SEE ALSO:	clock, difftime, mktime, time, asctime, ctime, gmtime, localtime

END DESCRIPTION **********************************************************/
_time_debug
size_t strftime( char __far *s, size_t maxsize, const char __far *format,
						const struct tm __far *timeptr)
{
	size_t	retval;

	retval = _strftime( s, maxsize, format, timeptr, &_lc_time);

	return (retval == -1) ? 0 : retval;
}




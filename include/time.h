/*
	time.h

	Copyright (c) 2009-10 Digi International Inc., All Rights Reserved

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
	C90 - 7.12 Date and Time
*/
/* START FUNCTION DESCRIPTION ********************************************
tm_mon2month                                                      <time.h>

MACRO SYNTAX:	int tm_mon2month( int tm_mon)

DESCRIPTION:	Converts from a tm_mon member of struct tm to a calendar
					month (1-12).

					If using ANSI struct tm, tm_mon ranges from 0 to 11.
					If using Dynamic C legacy struct tm, tm_mon ranges from 1 to 12.

					This macro allows for conversions regardless of which
					struct is in place.  If __ANSI_TIME__ or __ANSI_STRICT__ is
					defined, it will add 1 to the tm_mon value, otherwise it will
					not alter it.

END DESCRIPTION **********************************************************/
/* START FUNCTION DESCRIPTION ********************************************
month2tm_mon                                                      <time.h>

MACRO SYNTAX:	int month2tm_mon( int month)

DESCRIPTION:	Converts from a calendar month (1-12) to a tm_mon member of
					struct tm.

					If using ANSI struct tm, tm_mon ranges from 0 to 11.
					If using Dynamic C legacy struct tm, tm_mon ranges from 1 to 12.

					This macro allows for conversions regardless of which
					struct is in place.  If __ANSI_TIME__ or __ANSI_STRICT__ is
					defined, it will subtract 1 to the month value, otherwise it
					will not alter it.

END DESCRIPTION **********************************************************/
#ifndef __TIME_H
#define __TIME_H

	#ifdef __ANSI_STRICT__
		#define __ANSI_TIME__
	#endif

	// Legacy Dynamic C macro
	#define STARTYR 80		// start year=number of years since 1900

	// 7.12.1 Components of time
	#define NULL				(void *) 0
	typedef unsigned short	size_t;

	// We use the existing TICK_TIMER as the return value of clock().
	typedef unsigned long	clock_t;
	#define CLOCKS_PER_SEC	1024					// C90
	#define CLK_TCK			CLOCKS_PER_SEC		// C89, obsolete name

	// time_t is the type used to hold the number of seconds since 1/1/1980
	typedef unsigned long	time_t;

	#ifdef __ANSI_TIME__
		struct tm
		{
			int tm_sec;			// seconds after minute [0, 60] (60 = leap second)
			int tm_min;			// minutes after the hour [0, 59]
			int tm_hour;		// hours since midnight [0, 23]
			int tm_mday;		// day of the month [1, 31]
			int tm_mon;			// months since January [0, 11]
			int tm_year;		// years since 1900
			int tm_wday;		// days since Sunday [0, 6]
			int tm_yday;		// days since January 1 [0, 365]
			int tm_isdst;		// Daylight Savings Time flag
									// >0 if in effect, 0 if not in effect, <0 if unknown
		};
		#define _TM_MON_JANUARY		0
		#define tm_mon2month(x)		((x)+1)
		#define month2tm_mon(x)		((x)-1)
	#else
		struct tm						// Legacy Dynamic C time definition
		{
			char tm_sec;				// seconds 0-59
			char tm_min;				// 0-59
			char tm_hour;				// 0-59
			char tm_mday;				// 1-31
			char tm_mon;				// 1-12
			char tm_year;				// 00-150 (1900-2050)
			char tm_wday;				// 0-6 0==sunday
		};
		#define _TM_MON_JANUARY		1
		#define tm_mon2month(x)		(x)
		#define month2tm_mon(x)		(x)
	#endif
	#define _TM_MON_DECEMBER (_TM_MON_JANUARY + 11)

	// 7.12.2 Time manipulation functions
	clock_t clock( void);
	double difftime( time_t time1, time_t time0);
	time_t mktime( struct tm __far *timeptr);
	time_t time( time_t __far *timer);

	// 7.12.3 Time conversion functions
	char *asctime( const struct tm __far *timeptr);
	char *ctime( const time_t __far *timer);
	struct tm *gmtime( const time_t __far *timer);
	struct tm *_gmtime( struct tm *dest_tm, time_t timer);
	struct tm *localtime( const time_t __far *timer);
	size_t strftime( char __far *s, size_t maxsize, const char __far *format,
							const struct tm __far *timeptr);

   #use "time.c"
#endif
/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/*****************************************************

	 rtc_test.c

    Example of using the real-time clock functions by checking year 2000 rollover.

	 This program demonstrates the use of the RTC functions by:
			1.  reading (and saving) the current date/time settings
			2.  setting the date to 12/31/99 at 23:59:00
			3.  reading the RTC again two seconds later
			4.  setting the RTC back to the original date/time

	 It uses the functions:		read_rtc, write_rtc, mktime, mktm, tm_wr, tm_rd

    Note that the function tm_rd() will not be accurate after the RTC is
    written to until you restart the program!  This is because it uses the
    virtual driver variable SEC_TIMER instead of reading the RTC directly.  In
    general, this is a good thing, but if you change the RTC in a program (as in
    the program below), you can use the read_rtc() function that will read the
    RTC directly.

******************************************************/
#class auto

// function prototype for print_time function
void print_time(unsigned long);


int main()
{
	unsigned int	i;
	struct tm		rtc;					// time struct
	unsigned long	t0, t1, t2;			// used for date->seconds conversion

	//////////////////////////////////////////////////
	// read current date/time via tm_rd

	tm_rd(&rtc);						// get time in struct tm
	t0 = mktime(&rtc);				// convert struct tm into seconds since 1980
	printf("Current date/time is ");
	print_time(t0);

	//////////////////////////////////////////////////
	// change the date/time via tm_wr

	rtc.tm_sec = 59;							// change the time
	rtc.tm_min = 59;
	rtc.tm_hour = 23;
	tm_setMDY( &rtc, 12, 31, 1999);

	tm_wr(&rtc);								// set clock
	t1 = mktime(&rtc);
	printf("Setting date/time to ");
	print_time(t1);


	//////////////////////////////////////////////////
	// wait two seconds...

	printf("Waiting two seconds...\n\n");

	do {
		t2 = read_rtc();							// read time in seconds since 1980
	} while ( t2 < (t1+2) );				// wait until 2 seconds have elapsed


	//////////////////////////////////////////////////
	// read current date/time

	t2 = read_rtc();							// read time in seconds since 1980
	printf("Current date/time is ");		// NOTE that we use read_rtc() and not tm_rd()
	print_time(t2);							//		after resetting the clock


	//////////////////////////////////////////////////
	// reset date/time to original+elapsed via write_rtc

	t2 = read_rtc();					// new time in seconds since 1/1/1980

	t0 += (t2 - t1);						// new time should be original + elapsed

	write_rtc(t0);


	//////////////////////////////////////////////////
	// read current date/time

	t2 = read_rtc();						// read time in seconds since 1980
	printf("Current date/time is ");
	print_time(t2);


	return(0);
}

////////////////////////////////////////////////////////////////////////////////
// prints out date and time handed to it in tm struct

void print_time(unsigned long thetime)
{
	char buffer[80];
	struct tm	thetm;

	mktm(&thetm, thetime);
	strftime( buffer, sizeof buffer, "%c\n\n", &thetm);
	puts( buffer);
}
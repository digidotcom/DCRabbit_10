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

	 setrtckb.c

    Example of setting the real-time clock from keyboard entry.

    This program sets the Rabbit real-time clock from keyboard entry:
    valid dates are from 1/1/1980 (entered as 1/1/80) to
    12/31/2047 (entered as 12/31/47).  Just press Enter
    to leave the real-time clock unchanged.

******************************************************/
#class auto


int main()
{
	char *p, *endptr, s[80], *e;
	unsigned int month, day, year, hour, minute, second, done;
	struct tm		rtc;					// time struct
	unsigned long	t0;					// seconds
	int i, j;

	for (i = 0; i < 80; ++i) s[i] = 0;

	//////////////////////////////////////////////////
	// print current date/time

	printf("The current RTC date/time is:\n\n");
	tm_rd(&rtc);
	printf("%s\n", asctime(&rtc));

	done = 0;
	while (!done) {
		printf("Enter the correct date/time as:  mm/dd/yy hh:mm:ss\n");
      printf(" ( Valid dates are from 01/01/80 thru 12/31/47 )\n");
		printf("or just press Enter to exit with date/time unchanged:\n\n");
		gets(s);
		p = s;
		if (!*p) {
			printf("The date and time are unchanged.\n");
			return 0;
		}
		month = (unsigned int)strtod(p, &endptr);
		if (endptr != p && month > 0 && month < 13) {
			p = endptr + 1;
			day = (unsigned int)strtod(p, &endptr);
			if (endptr != p && day > 0 && day < 32) {
				p = endptr + 1;
				year = (unsigned int)strtod(p, &endptr);
				if (endptr != p && (year < 48 || year > 79) && year < 100) {
					if (year < 48) year += 100;
					p = endptr + 1;
					hour = (unsigned int)strtod(p, &endptr);
					if (endptr != p && hour < 25) {
						p = endptr + 1;
						minute = (unsigned int)strtod(p, &endptr);
						if (endptr != p && minute < 61) {
							p = endptr + 1;
							second = (unsigned int)strtod(p, &endptr);
							if (second < 61) {
								done = 1;
							}
							else {
								e = "SECOND";
							}
						}
						else {
							e = "MINUTE";
						}
					}
					else {
						e = "HOUR";
					}
				}
				else {
					e = "YEAR";
				}
			}
			else {
				e = "DAY";
			}
		}
		else {
			e = "MONTH";
		}

		if (!done) {
			printf("\n\n%s Error\n\n\n", e);
			continue;
		}
	}


	//////////////////////////////////////////////////
	// change the date/time via tm_wr

	rtc.tm_sec  = second;	   // 0-59
	rtc.tm_min  = minute;		// 0-59
	rtc.tm_hour = hour;			// 0-23
	rtc.tm_mday = day;			// 1-31
	rtc.tm_mon  = month2tm_mon(month);			// 1-12 (legacy) or 0-11 (ANSI)
	rtc.tm_year = year;			// 80-147, add 1900 to get year
										//		(i.e. 99 -> 1999)

	tm_wr(&rtc);					// set clock
	printf("RTC changed!\n\n");


	//////////////////////////////////////////////////
	// print new date/time

	// Note that read_rtc() will report the correct time now,
	// but tm_rd() will read incorrectly until a program is restarted!

	printf("The RTC date/time now is:\n\n");
	t0 = read_rtc();
	mktm(&rtc, t0);
	printf("%s\n", asctime(&rtc));

	printf("The SEC_TIMER variable used by tm_rd() will be reset\n");
	printf("properly when you restart this (or any other) program.\n");
	return 0;
}
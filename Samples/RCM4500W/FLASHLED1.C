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
/*******************************************************************

	flashled1.c

   This program is used with RCM4500W series controllers
   with prototyping boards.

   Description
   ===========
   This assembly program uses costatements to flash LED's,
   DS2 and DS3, on the prototyping board at different intervals.
   brdInit() is not called in this demonstration.

	I/O control			On proto-board
	--------------		----------------------
	Port B bit 2		DS2, LED
	Port B bit 3		DS3, LED

   Instructions
   ============
   1.  Compile and run this program.
   2.  DS2 and DS3 LED's flash on/off at different times.

*******************************************************************/
#class auto
#use RCM45xxW.lib

main()
{
	brdInit();
	while (1)
	{
		costate
		{	// DS2 LED
			DS2led(ON);							//on for 50 ms
			waitfor(DelayMs(50));
			DS2led(OFF);						//off for 100 ms
			waitfor(DelayMs(100));
		}

		costate
		{	// DS3 LED
			DS3led(ON);							//on for 200 ms
			waitfor(DelayMs(200));
			DS3led(OFF);						//off for 50 ms
			waitfor(DelayMs(50));
		}
	}
}
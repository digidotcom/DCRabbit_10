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
/*************************************************************************
 Samples\SEEDTEST.C   Demo program to test the random seed generator.

 When this program is run, press keys randomly.  Every time sufficient
 "randomness" has been accumulated, by timing the interval between
 keystrokes, the random data is extracted and printed as a number between
 0 and 7.

 If you hold a key down (i.e. use the typematic feature of your keyboard),
 then in theory the results will not be random since the time intervals
 will be fairly uniform.  In practice, there is enough variability in
 the timings to still allow a fairly good random number stream to be
 generated.

 The number of random bits which can be accumulated depends on
 the time interval, and the resolution with which the time interval can
 be measured.

 If timer B is not used, then 4 bits can be accumulated approximately
 every second (depending on the number of independent timings which
 can be made).  If timer B is running, it can be used to increase the
 resolution of the timer clock.  With timer B running at maximum rate,
 10 bits can be accumulated every second.
*************************************************************************/
#class auto


#memmap xmem
//#define RAND_DEBUG			// Turn on Dynamic C debugging for RAND.LIB
//#define RAND_USE_MD5		// Use MD5 hashing on entropy buffer (not used
									// for this demo, since we never extract more
									// bits than were added to the buffer).

#use "rand.lib"

void main()
{
	auto word r;
	auto word accumbits;
	auto word bins[8];
	auto word i;


	// Set timer B running at maximum rate to increase clock resolution.
	WrPortI(TBCR, &TBCRShadow, 0x00);	// clock timer B with (perclk/2) and
													//     set interrupt level to 0 (off)
	WrPortI(TBCSR, &TBCSRShadow, 0x01);	// enable timer B
	
	
	printf("Keep pressing keys to 'roll the 8-sided die'...\n");
	printf("Press 's' to print statistics\n");

	accumbits = 0;
	memset(bins, 0, sizeof(bins));

	seed_init(NULL);

	printf("Timer B is adding %u bits per observation.\n", _entropy_xbits);
	
	for (;;) {

		// Whenever we have 3 bits of seed, extract and print.
		
		while (seed_bits() >= 3) {
			r = (word)seed_getbits(3);
			printf("%u ", r);
			accumbits += 3;
			bins[r]++;
		}

		// Whenever keystroke available, use it as a source of randomness
		// by calling seed_clock().
		
		if (kbhit()) {
			if (getchar() == 's') {
				printf("\n");
				printf("Accumulated bits: %u\n", accumbits);
				for (i = 0; i < 8; i++)
					printf("  %u's: %u\n", i, bins[i]);
			}
			seed_clock(0);
		}	
	}
}

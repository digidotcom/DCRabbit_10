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
/* lopower.c -- demonstrate use of set_cpu_power_mode().

   This sample runs through various allowable mode settings.  Some of the
   clock speeds will be too low to allow target communications (debug via
   dynamic C), however if you run through the program without single stepping,
   it should survive until the end, where the original clock frequency is
   restored.

   A line is printed (starting with a letter a, b, c...) for each different
   mode.  Some of these lines may not be visible if the clock speed is
   incompatible with target communications.  This can happen for very slow
   clock speeds, as well as speeds where the integer divider for the
   serial port baud rate cannot be made close enough to the required value.

   In all cases, the get_cpu_frequency() function is called, and the result
   is stored.  When the clock is reset to normal rate, the captured values
   are printed.

   Try lowering the debug baud rate in the Dynamic C
   	Options->ProjectOptions->Communications
   dialog box.
*/

//#define LOWPOWER_DEBUG
#use "low_power.lib"


void main()
{
	long cf;
	long sf[14];
	word i;
	int clkdbl;


	printf("Startup CPU freq = %ld\n", get_cpu_frequency());
	clkdbl = (GCDRShadow & 0x1F) != 0;
	printf("  - initial clock doubler is %s\n", clkdbl ? "ON" : "OFF");

	set_cpu_power_mode(2,CLKDOUBLER_OFF,SHORTCS_OFF);
	printf("a. div2, doubler OFF: CPU freq = %ld\n", sf[0] = get_cpu_frequency());

	set_cpu_power_mode(3,CLKDOUBLER_OFF,SHORTCS_OFF);
	printf("b. div4, doubler OFF: CPU freq = %ld\n", sf[1] = get_cpu_frequency());

	set_cpu_power_mode(4,CLKDOUBLER_OFF,SHORTCS_OFF);
	printf("c. div6, doubler OFF: CPU freq = %ld\n", sf[2] = get_cpu_frequency());

	set_cpu_power_mode(5,CLKDOUBLER_OFF,SHORTCS_OFF);
	printf("d. div8, doubler OFF: CPU freq = %ld\n", sf[3] = get_cpu_frequency());

	if (clkdbl) {
	   set_cpu_power_mode(5,CLKDOUBLER_ON,SHORTCS_OFF);
	   printf("e. div8, doubler ON: CPU freq = %ld\n", sf[4] = get_cpu_frequency());

	   set_cpu_power_mode(4,CLKDOUBLER_ON,SHORTCS_OFF);
	   printf("f. div6, doubler ON: CPU freq = %ld\n", sf[5] = get_cpu_frequency());

	   set_cpu_power_mode(3,CLKDOUBLER_ON,SHORTCS_OFF);
	   printf("g. div4, doubler ON: CPU freq = %ld\n", sf[6] = get_cpu_frequency());

	   set_cpu_power_mode(2,CLKDOUBLER_ON,SHORTCS_OFF);
	   printf("h. div2, doubler ON: CPU freq = %ld\n", sf[7] = get_cpu_frequency());
	}

	set_cpu_power_mode(1,CLKDOUBLER_ON,SHORTCS_OFF);
	printf("i. div1, doubler %s: CPU freq = %ld\n", clkdbl ? "ON" : "OFF",
			sf[8] = get_cpu_frequency());

	printf("Saved CPU freq measurements:\n");
	for (i = 0; i < 9; ++i) {
		if (!clkdbl && i >= 4 && i < 8)
			continue;
		printf("%c.  %ld\n", 'a'+i, sf[i]);
	}

	// 32k mode only allowable for Rabbits 4000 and 5000
	printf("Setting 32kHz...\n");
	set_cpu_power_mode(6,CLKDOUBLER_OFF,SHORTCS_OFF);
	for (i = 0; i < 100; ++i) {
		//WrPortI(WDTCR, NULL, 0x5A);	// Hit watchdog
		hitwd();
	}
	sf[9] = get_cpu_frequency();


	set_cpu_power_mode(0,CLKDOUBLER_ON,SHORTCS_OFF);
	printf("Back to start! CPU freq = %ld\n", get_cpu_frequency());


	printf("Saved 32kHz CPU freq measurements:\n");
	for (i = 9; i < 10; ++i) {
		printf("%c.  %ld\n", 'a'+i, sf[i]);
	}


}
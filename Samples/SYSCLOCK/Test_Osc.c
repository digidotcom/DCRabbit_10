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
/*******************************************************************************

   test_osc.c

   This program demonstrate the various CPU oscillator settings available with
   Rabbit 4000 and 5000 CPUs.

   This program is not intended for use with the Rabbit 6000 CPU, which Dynamic
   C defaults to using a PLL-based main clock.  For details, please refer to the
   Rabbit 6000 Microprocessor User's manual, section "2 Clocks" and especially
   to subsections "2.1 Overview" and "2.3.5 32 kHz Clock."

   This program will blink LEDs at a fixed rate for different oscillator
   settings.  The following settings are demonstrated:
      - full processor clock
      - 1/2 processor clock
      - 1/4 processor clock
      - 1/6 processor clock
      - 1/8 processor clock
      - run processor clock off 32kHz oscillator
      - full processor clock

   Dynamic C will lose communication with the target in debug mode when the
   program switches from full processor clock mode.  To avoid this, compile
   without running (F5) and then use run with no polling (alt-F9).  The blink
   rate when the CPU is switched to the 32kHz oscillator will be very slow!

   Running with the 32kHz oscillator also disables the periodic interrupt.  If
   your program depends on that interrupt (i.e. uses TICK_TIMER, MS_TIMER,
   SEC_TIMER, of waitfor statements in costates), then periodically update those
   counters by calling updateTimers() within your code.

   Note also that since the periodic interrupt is disabled while running off the
   32 KHz oscillator, the watchdog timer is disabled by first calling the
   Disable_HW_WDT() function.  An alternative is to call hitwd() periodically to
   ensure that the Rabbit CPU's hardware watchdog timer does not expire.

   As written, this program will blink LEDs on the prototyping board for RCM4xxx
   and RCM54xxW core modules, the single LED on the interface board for the
   RCM56xxW/RCM57xx core modules and the Ethernet activity LED on the BL4S1xx
   SBCs.

   To add blinky-LED support for other Rabbit targets, edit this sample to alter
   the BLINKYLEDS_* macro definitions as appropriate for available LEDs on the
   "other" target board.  It may also be necessary to add "#use <library name>"
   for some "other" boards.

*******************************************************************************/
#if CPU_ID_MASK(_CPU_ID_) == R4000
 // choose the appropriate board-specific library for the brdInit() call
 #if RCM4000_SERIES
	#use "RCM40xx.LIB"
 #elif RCM4100_SERIES
	#use "RCM41xx.LIB"
 #elif RCM4200_SERIES
	#use "RCM42xx.LIB"
 #elif RCM4300_SERIES
	#use "RCM43xx.LIB"
 #elif RCM4400W_SERIES
	#use "RCM44xxW.LIB"
 #elif RCM4500W_SERIES
	#use "RCM45xxW.LIB"
 #elif BL4S100_SERIES
	#use "BL4S1xx.LIB"
   // Use Ethernet Activity LED on BL4Sxx SBCs
	#define BLINKYLEDS_OFF BitWrPortI(PEDR, &PEDRShadow, 1, 7)
	#define BLINKYLEDS_ON  BitWrPortI(PEDR, &PEDRShadow, 0, 7)
 #else
	#fatal "Unknown RCM4xxx type, this sample must be updated to suit."
 #endif

 #if !BL4S100_SERIES
	 // these macros are suitable for the RCM4xxx prototyping board
	 #define BLINKYLEDS_OFF WrPortI(PBDR, &PBDRShadow, 0x08)
	 #define BLINKYLEDS_ON WrPortI(PBDR, &PBDRShadow, 0x04)
 #endif

 #define BLINKYLEDS_INIT brdInit();

#elif CPU_ID_MASK(_CPU_ID_) == R5000
 // choose the appropriate board-specific library for the brdInit() call
 #if RCM5400W_SERIES
	#use "RCM54xxW.LIB"
   // Set LED macros for RCM54xxW prototyping board
   #define BLINKYLEDS_OFF WrPortI(PBDR, &PBDRShadow, 0x08)
   #define BLINKYLEDS_ON WrPortI(PBDR, &PBDRShadow, 0x04)
 #elif RCM5600W_SERIES
	#use "RCM56xxW.LIB"
   // Set LED macros for RCM57xxW prototyping board
   #define BLINKYLEDS_OFF BitWrPortI(PDDR, &PDDRShadow, 1, 0)
   #define BLINKYLEDS_ON BitWrPortI(PDDR, &PDDRShadow, 0, 0)
 #elif RCM5700_SERIES
	#use "RCM57xx.LIB"
   // Set LED macros for RCM57xx prototyping board
   #define BLINKYLEDS_OFF BitWrPortI(PDDR, &PDDRShadow, 1, 0)
   #define BLINKYLEDS_ON BitWrPortI(PDDR, &PDDRShadow, 0, 0)
 #else
	#fatal "Unknown RCM5xxx type, this sample must be updated to suit."
 #endif
 // all 5000 based cores support brdInit for prototype/interface boards
 #define BLINKYLEDS_INIT brdInit();

#elif CPU_ID_MASK(_CPU_ID_) == R6000
   #error "This program is not intended for use with the Rabbit 6000 CPU, which"
   #error "Dynamic C defaults to using a PLL-based main clock.  For details,"
   #error "please refer to the Rabbit 6000 Microprocessor User's manual,"
   #error "section \"2 Clocks\" and especially to subsections \"2.1 Overview\""
   #fatal "and \"2.3.5 32 kHz Clock.\""

#else
	#fatal "Unknown core type, this sample must be updated to suit."
#endif

#class auto 			// Change default storage class for local variables: on the stack

// We need a delay independent of the real-time clock,
// so this function will delay for several hundred
// thousand CPU clock cycles.

nodebug void delayloop()
{
	auto unsigned int	i;

	for (i=0u; i<10000u; ++i)
		;
}

////////////////////////////////////////////////////////////

void blink_leds()
{
	BLINKYLEDS_OFF;
	delayloop();
	BLINKYLEDS_ON;
	delayloop();
}

////////////////////////////////////////////////////////////

#define	STEPDELAY	5		// delay in secs for each mode

void main()
{
	auto int	done;

	BLINKYLEDS_INIT;

	done = FALSE;
	while (!done) {

		costate {
			// full processor clock
			useMainOsc();
			waitfor(DelaySec(STEPDELAY));

			// 1/2 processor clock
			useClockDivider3000(CLKDIV_2);
			waitfor(DelaySec(STEPDELAY));

			// 1/4 processor clock
			useClockDivider3000(CLKDIV_4);
			waitfor(DelaySec(STEPDELAY));

			// 1/6 processor clock
			useClockDivider3000(CLKDIV_6);
			waitfor(DelaySec(STEPDELAY));

			// 1/8 processor clock
			useClockDivider3000(CLKDIV_8);
			waitfor(DelaySec(STEPDELAY));

			// disable watchdog time-outs for running off the 32 KHz oscillator
			Disable_HW_WDT();

         // run processor clock off 32kHz oscillator
         use32kHzOsc();
            // waitfor() will not work when running off the
            // 32kHz oscillator (the periodic interrupt is
            // disabled).  Instead we'll just call the
            // LED-blinking code once, which should be enough
            // (note that it will take on the order of
            // half a minute to finish).
         blink_leds();

			// full processor clock
			useMainOsc();

			// re-enable watchdog time-outs
			Enable_HW_WDT();

			waitfor(DelaySec(STEPDELAY));
			done = TRUE;
		}

		costate {
			blink_leds();
		}
	}
	printf("Done.\n");
}
/*******************************************************************************

   test_32khz.c
   Digi International, Copyright (C) 2007-2009.
   All rights reserved.

   This program demonstrates the Rabbit 4000 and 5000 CPUs' ability to run off
   the 32kHz oscillator divided by various values.

   This program is not intended for use with the Rabbit 6000 CPU, which Dynamic
   C defaults to using a PLL-based main clock.  For details, please refer to the
   Rabbit 6000 Microprocessor User's manual, section "2 Clocks" and especially
   to subsections "2.1 Overview" and "2.3.5 32 kHz Clock."

   This program will blink LEDs at a fixed rate for different oscillator
   settings.  The following processor clock rates are demonstrated:
      - full processor clock
      - 32kHz oscillator
      - 32kHz oscillator divided by two
      - 32kHz oscillator divided by four
      - 32kHz oscillator divided by eight
      - 32kHz oscillator divided by sixteen
      - back to full processor clock

   Dynamic C will lose communication with the target in debug mode when the
   program switches from full processor clock mode.  To avoid this, compile
   without running (F5) and then use run with no polling (alt-F9).

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
// so this function will delay for a few hundred
// CPU clock cycles.

void delayloop()
{
	auto unsigned	i;

	for (i=0; i<10; i++)
		hitwd();		// necessary when running off 32kHz oscillator
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

void main()
{
	auto int	i;

	BLINKYLEDS_INIT;

	// full processor clock
	useMainOsc();
	for (i=0; i<10000; i++)
		blink_leds();

	// disable watchdog time-outs for when running off the 32 KHz oscillator
	Disable_HW_WDT();

	// switch to 32kHz oscillator
	use32kHzOsc();

	// run processor clock off 32kHz oscillator
	set32kHzDivider(OSC32DIV_1);
	for (i=0; i<16; i++)		blink_leds();

	// 32kHz oscillator divided by two
	set32kHzDivider(OSC32DIV_2);
	for (i=0; i<8; i++)		blink_leds();

	// 32kHz oscillator divided by four
	set32kHzDivider(OSC32DIV_4);
	for (i=0; i<4; i++)		blink_leds();

	// 32kHz oscillator divided by eight
	set32kHzDivider(OSC32DIV_8);
	for (i=0; i<2; i++)		blink_leds();

	// 32kHz oscillator divided by sixteen
	set32kHzDivider(OSC32DIV_16);
	for (i=0; i<1; i++)		blink_leds();

	// full processor clock
	useMainOsc();

	// re-enable watchdog time-outs
	Enable_HW_WDT();

	for (i=0; i<10000; i++)
		blink_leds();

	printf("Done.\n");
}
/*******************************************************************************
   timer_c_int.c
   Rabbit Semiconductor, 2007

   This program is used with Rabbit 4000+ series controllers.

   Description
   ===========
   This program demonstrates use of the timer C interrupt. It uses the
   peripheral clock over 16. The TCDLR and TCDHR registers make up a 16 bit
   divider to closely approximate the desired timer C interrupt frequency.
*******************************************************************************/

// Set timer C interrupt to fire every TIMERC_MS milliseconds
// (e.g. 1UL = interrupt every millisecond = 1000 interrupts per second).
#define TIMERC_MS 1UL

#if CPU_ID_MASK(_CPU_ID_) >= R6000 && defined PLL_DEFAULT_PLL_SPEED_MHz
	// Assume the Rabbit board is running in its default main clock mode, as
	//  defined by the PLL_DEFAULT_PLL_SPEED_MHz macro, and that main clock and
	//  peripheral clock frequencies are equal. Note that Rabbit's main clock
	//  frequency is half the PLL frequency.
	#define MAIN_PCLK_FREQUENCY (PLL_DEFAULT_PLL_SPEED_MHz * 1000000UL / 2UL)
#else
	// Assume the Rabbit board is running in its default main clock mode, as
	//  defined by the _CRYSTAL_SPEED_ and CLOCK_DOUBLED macros, and that main
	//  clock and peripheral clock frequencies are equal.
	#if CLOCK_DOUBLED
		#define MAIN_PCLK_FREQUENCY (_CRYSTAL_SPEED_ * 2UL)
	#else
		#define MAIN_PCLK_FREQUENCY (_CRYSTAL_SPEED_ * 1UL)
	#endif
#endif

// The ideal timer C divider value is the number of peripheral clocks in the
//  specified time period divided by 16 peripheral clocks per timer C count,
//  divided by the number of microseconds per millisecond, minus one (to
//  accommodate timer C's divide by N+1 behavior).
#define TIMERC_DIVIDER_IDEAL \
                            (TIMERC_MS * MAIN_PCLK_FREQUENCY / 16. / 1000. - 1.)
// The actual timer C divider value is rounded to an unsigned long integer type.
#define TIMERC_DIVIDER ((unsigned long) (TIMERC_DIVIDER_IDEAL + 0.5))

// Check for an out-of-range timer C divider value; if so, set the macro value
//  to the maximum allowable value and warn the user. The user is responsible
//  for selecting an achievable TIMERC_MS macro value.
#if TIMERC_DIVIDER > 65535UL
	#undef TIMERC_DIVIDER
	#define TIMERC_DIVIDER 65535UL
	#warns "Out of range TIMERC_DIVIDER redefined to the maximum value (65535)."
	#warns "Reduce the TIMERC_MS macro value to an achievable setting."
#endif

#define TCDLR_SETTING (int)(0xff & TIMERC_DIVIDER)
#define TCDHR_SETTING (int)(0xff & (TIMERC_DIVIDER/0x100))

unsigned long timerc_count;

#asm
timerC_isr::
		push	af							; save all used registers
		push	bcde
		push	jkhl
ioi	ld		a, (TCCSR)				; clear the interrupt request and get status
		; handle all interrupts flagged in TCCSR here
		ld		bcde, (timerc_count)
		ld		jkhl, 1
		add	jkhl, bcde
		ld		(timerc_count), jkhl
		pop	jkhl						; restore all used registers
		pop	bcde
		pop	af
		ipres
		ret
#endasm

int main(void)
{
	int waitFlag;

	printf("Rabbit's peripheral clock frequency is taken to be %.4f MHz.\n",
	       MAIN_PCLK_FREQUENCY / 1.e6);
	printf("The ideal (to 2 decimal places) timer C divider value is %.2f.\n",
	       TIMERC_DIVIDER_IDEAL);
	printf("The actual timer C divider value used herein is %lu.\n",
	       TIMERC_DIVIDER);
	printf("If perfect clocks, an approximate %.4f%% count error is expected.\n",
	       (TIMERC_DIVIDER_IDEAL / TIMERC_DIVIDER - 1.) * 100.);

	// ensure timer C is disabled
	WrPortI(TCCSR, &TCCSRShadow, 0x00);
	// set up timer C to use pclk/16
	WrPortI(TCCR, &TCCRShadow, 0x09);
	WrPortI(TCDLR, NULL, TCDLR_SETTING);
	WrPortI(TCDHR, NULL, TCDHR_SETTING);
	// install timer C's ISR
	SetVectIntern(TIMERC_OFS / 0x10, timerC_isr);
	// enable timer C
	WrPortI(TCCSR, &TCCSRShadow, 0x01);

	waitFlag = 1;
	while (waitFlag)
	{
		costate
		{
			// wait until lined up with 1000 millisecond interval
			waitfor (IntervalMs(1000));
			waitFlag = 0;
		}
	}

	timerc_count = 0;
	while (1)
	{
		costate
		{
			// report count at 1000 millisecond intervals
			waitfor (IntervalMs(1000));
			printf("%lu\n", timerc_count);
		}
	}
}
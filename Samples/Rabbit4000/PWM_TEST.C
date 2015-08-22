/********************************************************************
   Samples\Rabbit4000\pwm_test.c
   Z-World, 2006

   Description
   ===========
   This program demonstrates the PWM functions of the Rabbit 4000
   processor.

   It will set the four PWM channels, port E bits 4 - 7, to the
   following values:

	Option 1:
   (PC4) Channel 0:  10% duty cycle
   (PC5) Channel 1:  25% duty cycle

	Option 2:
   (PE4) Channel 0:  10% duty cycle
   (PE5) Channel 1:  25% duty cycle
   (PE6) Channel 2:  50% duty cycle
   (PE7) Channel 3:  99% duty cycle

   Parallel ports C and D are also available.  To use them, change
   PWM_USEPORTE to the appropriate port. Note: C6 and C7 are shared
   between PWM and serial port A used for communication with Dynamic C.

   Pulse spreading can be turned on or off PWM_SPREAD below.
   See documentation for pwm_set() for details.

   Instructions
   ============
   1. Compile and run this program.
   2. Best way to view duty cycles is to observe each channel
      with an oscilloscope.

*********************************************************************/
#class auto

#if (_BOARD_TYPE_ & 0xFF00) == RCM4000
#define OPTION1
#else
#define OPTION2
#endif

void main()
{
   unsigned long  freq;
   int   pwm_options, err;

   // request 10kHz PWM cycle (will select closest possible value)
   freq = pwm_init(10000ul);
   printf("Actual PWM frequency = %lu Hz\n", freq);

   // Select PWM output port
#ifdef OPTION1
   pwm_options = PWM_USEPORTC;
#else
   pwm_options = PWM_USEPORTE;
#endif

   /*
   Uncomment the following line to spread the PWM output
   throughout the cycle. default.h
   */
//	pwm_options |= PWM_SPREAD;

	/*
   Uncomment one of the following to suppress the PWM output
   */
//	pwm_options |= PWM_OUTEIGHTH;		// Suppress 7/8ths
//	pwm_options |= PWM_OUTQUARTER;	// Suppress 3/4ths
//	pwm_options |= PWM_OUTHALF;		// Suppress 1/2

   err = pwm_set(0, (int)(0.10 * 1024.0), pwm_options);
   if(err < 0) {
   	printf("Error, channel 0.\n");
   }
   err = pwm_set(1, (int)(0.25 * 1024.0), pwm_options);
   if(err < 0) {
   	printf("Error, channel 1.\n");
   }
#ifdef OPTION2
   err = pwm_set(2, (int)(0.50 * 1024.0), pwm_options);
   if(err < 0) {
   	printf("Error, channel 2.\n");
   }
   err = pwm_set(3, (int)(0.99 * 1024.0), pwm_options);
   if(err < 0) {
   	printf("Error, channel 3.\n");
   }
#endif
   while(1); //keep running the program
}
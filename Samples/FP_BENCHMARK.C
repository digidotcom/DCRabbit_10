/*************************************************************************

	Samples\fp_benchmark.c
	Rabbit Semiconductor, Inc. 2000

	Demonstration of approximate computation times for various floating
	point operations and functions.  Note that nodebug is specified for
	some functions because it generates faster code for these tests.
	(I.E.:  No debug overhead is introduced into those test functions.)

	This program times various functions or operations executing in loops
	and displays the timing results on STDIO.  Each non-empty loop executes
	a particular function or operation 1000 times in 500 loop iterations.
	An empty loop is also timed over 500 iterations to allow subtraction of
	the looping overhead.  In this way, the millisecond timing measurement
	can be directly related to the reported microsecond timing average.

 Board clock speed is  29.491201 MHz.
 Program code wait states setting is 1 for reads, 2 for writes.
 (Code and BIOS in Flash compile mode.)
empty loop                     1 microseconds
float multiply                23 microseconds
float add                     18 usec
float divide                  44 usec
float square root             48 usec
float sine                   147 usec
float exp                    145 usec
float log                    210 usec
float arccos                 388 usec
float arctan (atan2)         332 usec
empty costatement             11 usec
empty cofunction              38 usec

 Board clock speed is  58.982402 MHz.
 Program code wait states setting is 2 for reads, 3 for writes.
 (Code and BIOS in Flash compile mode.)
empty loop                     1 microseconds
float multiply                10 microseconds
float add                      7 usec
float divide                  18 usec
float square root             20 usec
float sine                    69 usec
float exp                     64 usec
float log                     94 usec
float arccos                 175 usec
float arctan (atan2)         152 usec
empty costatement              5 usec
empty cofunction              20 usec

 Board clock speed is  58.982402 MHz.
 Program code wait states setting is 0 for reads, 1 for writes.
 (Code and BIOS in Flash, Run in RAM compile mode.)
empty loop                     1 microseconds
float multiply                 7 microseconds
float add                      6 usec
float divide                  16 usec
float square root             16 usec
float sine                    51 usec
float exp                     51 usec
float log                     74 usec
float arccos                 139 usec
float arctan (atan2)         119 usec
empty costatement              4 usec
empty cofunction              14 usec

 Board clock speed is  58.982402 MHz.
 Program code wait states setting is 4 for reads, 5 for writes.
 (Code and BIOS in Flash compile mode.)
 (This is a 'Code and BIOS in Flash, Run in RAM' capable board.)
empty loop                     1 microseconds
float multiply                19 microseconds
float add                     16 usec
float divide                  40 usec
float square root             43 usec
float sine                   131 usec
float exp                    131 usec
float log                    190 usec
float arccos                 349 usec
float arctan (atan2)         298 usec
empty costatement              8 usec
empty cofunction              34 usec

*************************************************************************/
#class auto

int rundemo();

nodebug
cofunc void nullcof(void)
{
	// empty cofunction for cofunction timing test
}

void main()
{
	rundemo();
}

nodebug
rundemo()
{
	unsigned long int	timer, emptyloop, emptycostate;
	unsigned int	j,dif;
	float				x, y, z;

	// compute clock speed
	//  freq_divider is the timer Ax divisor value required to set a 19200 bits
	//  per second serial rate.  (Note that to achieve a 19200 bps serial rate,
	//  the actual value put into a TATxR register is freq_divider minus 1
	//  because the TATxR registers divide by n plus 1.)  freq_divider is
	//  calculated by the BIOS based on the timer A prescaler, clock doubler and
	//  peripheral clock divider settings.  Each freq_divider value unit works
	//  out to be 19200 * 32 main oscillator clocks, as our standard is to clock
	//  timer Ax with peripheral clock divided by 2 and to use timer Ax 16 times
	//  baud rate clock input to the serial port.
	x = 19200. * 32. * (float) freq_divider / 1000000.;	// clock speed in mhz
	printf("\n\n Board clock speed is %10.6f MHz.\n", x);

	// determine the wait states setting for the MB0CR quadrant
	//  (which is where we presume to be running this code)
	switch(0xC0 & MB0CRShadow) {
	case 0x00:
		j = 4u;
		break;
	case 0x40:
		j = 2u;
		break;
	case 0x80:
		j = 1u;
		break;
	case 0xC0:
		j = 0u;
//		break;
	default:
//		break;
	}
	printf(" Program code wait states setting is %u for reads, %u for writes.\n",
	       j, 1 + j);

#if FAST_RAM_COMPILE
	#if SUPPRESS_FAST_RAM_COPY
		printf(" (Code and BIOS in Fast RAM compile mode.)\n");
   #else
		printf(" (Code and BIOS in Flash Run in Fast RAM compile mode.)\n");
   #endif
#elif FLASH_COMPILE
	printf(" (Code and BIOS in Flash compile mode.)\n");
#elif RAM_COMPILE
	printf(" (Code and BIOS in RAM compile mode.)\n");
#endif

#if !FAST_RAM_COMPILE
 #ifdef CS_RAM2
	printf(" (This is a 'Code and BIOS in Flash, Run in RAM' capable board.)\n");
 #endif
#endif


	// compute time for an empty loop
	//  MS_TIMER is updated 1000 times per second via the periodic timer ISR
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		// empty loop
	}
	emptyloop = MS_TIMER-timer;				// save this value to adjust later measurements
	printf("empty loop\t\t%8ld microseconds\n", MS_TIMER-timer); // print time in microseconds


	// choose some values to use in the next few calculations
	x=5.68; y=34567.99; z=-2345.67;

	// time float multiplication
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
   	x*y;
		z*x;
	}
	printf("float multiply\t\t%8ld microseconds\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time float addition
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		x+y;
		z+x;
	}
	printf("float add\t\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time float division
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		x/y;
		z/x;
	}
	printf("float divide\t\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time sqrt function
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		sqrt(x);
		sqrt(y);
	}
	printf("float square root\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time sine function
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		sin(0.34);
		sin(-3.0);
	}
	printf("float sine\t\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time exp function
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		exp(5.0);
		exp(0.01);
	}
	printf("float exp\t\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time log function
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		log(5.0);
		log(0.01);
	}
	printf("float log\t\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time arccosine function
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		acos(0.3567);
		acos(-0.89);
	}
	printf("float arccos\t\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time arctangent function
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		atan2(0.5,-0.3444);
		atan2(0.60888,0.2341);
	}
	printf("float arctan (atan2)\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time empty costatement
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		costate{}
		costate{}
	}
	emptycostate = MS_TIMER - timer - emptyloop;			// save this value to adjust next measurement
	printf("empty costatement\t%8ld usec\n", MS_TIMER-timer-emptyloop); // print time in microseconds


	// time empty cofunction
	timer = MS_TIMER;   // get current time in milliseconds
	for(j=0; j<500u; j++) {
		costate {
			waitfordone{ nullcof(); }
		}
		costate {
			waitfordone{ nullcof(); }
		}
	}
	printf("empty cofunction\t%8ld usec\n", MS_TIMER-timer-emptyloop-emptycostate); // print time in microseconds


}  // done with program
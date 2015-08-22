/*******************************************************************************
	Samples\BL4S1xx\low power.c
	Digi International, Copyright © 2005-2008.  All rights reserved.

	Description
	===========
	This program demonstrates conditional usage of TCP/IP networking in Low
	Power modes.  It responds to TCP/IP ping packets and also demonstrates
	pinging a remote host.  It prints a message when the ping response arrives
	here.  If PING_WHO is not defined, then it pings the default gateway.

	In order to run from the Dynamic C debugger, go into the Project Options
	and set Debug Baud Rate to 9600.  At that rate, the debugger works all
	the way down to 2.5 MHz.  When the sample switches to the 32kHz clock,
	it will stop printing to the STDIO window, but will resume if switched
	back to the main oscillator.

	** If testing within Dynamic C, be sure to Compile (F5) and then start the
	program with Run Without Polling (Alt-F9), to prevent errors messages in the
	debugger when the Rabbit switches to the 32kHz clock.

	You can also use a terminal emulator (like HyperTerm or Moni) to monitor
	the sample as it runs.
	 - "Compile to Target" in Dynamic C, then turn off the BL4S1xx SBC.
	 - Choose "Close Connection" from the "Run" menu in Dynamic C.
	 - Switch from the PROG connector to DIAG on the programming cable.
	 - Open the serial port in your terminal emulator (9600 baud).
	 - Turn on the BL4S1xx SBC.

	Connections
	===========
	1. DEMO board jumper settings:
			- Set switches to active low by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller GND, to the DEMO board GND.
	3. Connect a wire from the controller +5V to the DEMO board +V.
   4. Connect the following wires from the controller to the DEMO board:

	   	From OUT0 to LED1	(10 Hz PWM at full speed, slows as Rabbit slows)
	   	From OUT1 to LED2	(flashes when on main oscillator)
	   	From OUT2 to LED3	(flashes when on 32kHz oscillator)
	   	From OUT3 to LED4	(lit when running at slowest speed, ~4kHz)
         From SW1	 to IN0  (Slower clock speed)
         From SW2	 to IN1  (Faster clock speed)

	This program will start in full speed/power mode until the user presses SW1
	on the demo board.

	Each time the SW1 switch is pressed, the CPU changes to a lower power
	consumption mode until networking is disabled. After that, power consumption
	will progressively get reduced with each switch press until a minimum CPU
	speed of ~4kHz is reached.

	Each time the SW2 switch is pressed, the CPU changes to a higher power
	consumption mode until networking is re-enabled.  After that, power
	consumption will progressively increase with each switch press until a
	maximum CPU speed of 40MHz is reached.

	The Ethernet interface cannot run when the Rabbit clock is less than 20MHz,
	so this sample turns it off at those speeds.

	As an example of further lowering power consumption, this sample turns the
	ADC (ADS7870) off when running on the 32kHz clock.

     Note: The LED light is programmed to blink at different rates for
       each lower power mode, to indicate that the board is still running.

*******************************************************************************/

// Serial Cable Communication
//
// The following definitions redirect stdio to serial port A when
// the core module is not attached to the PROG connector of the
// programming cable.  Use the DIAG connector of the programming cable
// to access serial port A.  See Samples\STDIO_SERIAL.C for more details
// on redirecting stdio.

#define	STDIO_DEBUG_SERIAL	SADR
#define	STDIO_DEBUG_BAUD		9600
#define	STDIO_DEBUG_ADDCR

//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// default to storing functions to xmem instead of root
#memmap xmem

// include BL4S1xx series library
#use "BL4S1xx.LIB"

/* Define HIT_WATCHDOG if watchdog is enabled (default) while using
*  power modes 6 through 10 (ie when periodic interrupt is disabled).
*
*/
#define HIT_WATCHDOG

// Debugging option, sample will print extra debug messages when defined
#define LOWPOWER_VERBOSE

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

/** Remote interface to send PING to (passed to resolve()): **/
/*  Undefine to retrieve default gateway and ping that. */
// #define PING_WHO			"10.10.6.1"


/********************************
 * End of configuration section *
 ********************************/

#if (! BL4S100_SERIES)
   #error "This sample is only for the BL4S1xx series boards."
#endif

// include the TCP/IP networking library
#use "dcrtcp.lib"

// include the low power library for set_cpu_power_mode() function
#use "low_power.lib"

enum {
	LP_ETH_DISABLED,		// on-board Ethernet disabled at this clock speed
	LP_ETH_SYSCLK,			// on-board Ethernet uses system clock (20 MHz)
	LP_ETH_HALFSYS			// on-board Ethernet uses system clock / 2 (40 MHz / 2)
};

typedef struct {
	char		desc[13];			// string to describe the clock mode
	char		clock[11];			// string to describe clock speed
	int		doubler;				// whether to turn on the clock doubler
	int		powermode;			// power mode to pass to set_cpu_power_mode
	int		ethernet;			// one of the LP_ETH_* enums
} power_setting_t;

/*
	This sample cannot run slower than 4kHz without the watchdog going off and
	resetting the board.  To run at 2kHz, you would need to uncomment the last
	entry in power_settings, and then disable the watchdog timer completely or
	run a tight assembly routine while waiting to wake up.
*/
const power_setting_t power_settings[] = {
//		Description,	CPU Speed, 		Clock Doubler, powermode,	Ethernet
	 { "Main * 2",		"40.00 MHz",	CLKDOUBLER_ON,		1,		LP_ETH_HALFSYS }
	,{ "Main",			"20.00 MHz",	CLKDOUBLER_OFF,	1,		LP_ETH_SYSCLK }
	,{ "Main / 2",		"10.00 MHz",	CLKDOUBLER_OFF,	2,		LP_ETH_DISABLED }
	,{ "Main * 2 / 6", "6.67 MHz",	CLKDOUBLER_ON,		4,		LP_ETH_DISABLED }
	,{ "Main / 4",		 "5.00 MHz",	CLKDOUBLER_OFF,	3,		LP_ETH_DISABLED }
	,{ "Main / 6",		 "3.33 MHz",	CLKDOUBLER_OFF,	4,		LP_ETH_DISABLED }
	,{ "Main / 8",		 "2.50 MHz",	CLKDOUBLER_OFF,	5,		LP_ETH_DISABLED }
	,{ "32kHz / 1",	"32.768 kHz",	CLKDOUBLER_OFF,	6,		LP_ETH_DISABLED }
	,{ "32kHz / 2",	"16.384 kHz",	CLKDOUBLER_OFF,	7,		LP_ETH_DISABLED }
	,{ "32kHz / 4",	 "8.192 kHz",	CLKDOUBLER_OFF,	8,		LP_ETH_DISABLED }
	,{ "32kHz / 8",	 "4.096 kHz",	CLKDOUBLER_OFF,	9,		LP_ETH_DISABLED }
//	,{ "32kHz / 16",	 "2.048 kHz",	CLKDOUBLER_OFF,	10,	LP_ETH_DISABLED }
};
#define MAX_POWER_SETTING (sizeof(power_settings)/sizeof(power_settings[0])-1)


#define MAX_ETH_MODE_INDEX 1	// maximum mode setting where Ethernet still works
#define MAX_MAIN_OSC_INDEX 6	// maximum mode setting still on Main osc
#define MAX_MAIN_OSC_POWERMODE 5		// maximum powermode setting for Main osc

int set_power (int index)
{
	int nacr;
	static const power_setting_t	*old = &power_settings[0];
	const power_setting_t	*new;

	if (index < 0 || index > MAX_POWER_SETTING) {
	   // don't call printf on 32kHz clock, it causes a watchdog timeout
	   if (old->powermode <= MAX_MAIN_OSC_POWERMODE) {
	      printf ("set_power: invalid setting (%d not between 0 and %d)\n",
	         index, MAX_POWER_SETTING);
		}
		return -1;
	}

	new = &power_settings[index];
	if (old == new) return 0;

   // If we're switching to a mode that won't work with Ethernet,
   // turn off the Ethernet interface.
   if (new->ethernet == LP_ETH_DISABLED) {
      // make sure Ethernet is off
      if (new->ethernet != old->ethernet) {
         printf ("turning ethernet off\n");

         // disable TCP/IP...
         ifdown (IF_ETH0);

         //...and power down Ethernet interface.
         pd_powerdown(IF_ETH0);
      }
   }

   set_cpu_power_mode(new->powermode, new->doubler, SHORTCS_OFF);
	if (new->powermode <= MAX_MAIN_OSC_POWERMODE) {
	   // don't call printf on 32kHz clock, it causes a watchdog timeout
	   printf ("\nsetting %d of %d:  set cpu clock to %s (%s)\n", index + 1,
	   	MAX_POWER_SETTING, new->desc, new->clock);
	}

   if (new->ethernet != LP_ETH_DISABLED) {
   	// CPU speed is adequate for ethernet

      if (old->ethernet == LP_ETH_DISABLED) {
      	// Ethernet was previously turned off
         printf ("turning ethernet on\n");

         // power the Ethernet interface up...
         pd_powerup(IF_ETH0);

         //Re-enable TCP/IP interface.
         ifup (IF_ETH0);
		}
		if (old->ethernet != new->ethernet) {
			// Ethernet was previously using a different clock speed

	      // set upper bits of NACR appropriately (sysclock or sysclock/2)
	      // 40MHz, use sysclock / 2 (set bit 6)
	      // 20MHz, use sysclock (clear bit 6)
	      // Since port NACR does not use a shadow register, read the current
	      // value, change the bit, and then write it back
	      if (new->ethernet == LP_ETH_HALFSYS) {
				WrPortI (NACR, NULL, RdPortI(NACR) | 0x40);
	      } else {
				WrPortI (NACR, NULL, RdPortI(NACR) & ~0x40);
	      }
		}

      // Wait for the interface to come back up
      while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
         tcp_tick(NULL);
      }
   }

   old = new;
   return 0;
}

// LEDs are active low (sink pin to ground to light LED)
#define LED_ON		0
#define LED_OFF	1

#define DIN_SW_FASTER	0		// use DIN0 for input from "faster" swtich (SW0)
#define DIN_SW_SLOWER	1		// use DIN1 for input from "slower" switch (SW1)

#define SW_FASTER_PRESSED() 	(!digIn(DIN_SW_FASTER))	// "faster" switch press
#define SW_SLOWER_PRESSED()	(!digIn(DIN_SW_SLOWER))	// "slower" switch press

#define SET_LED_FAST(state)		digOut(1,state)	// flash LED2 on main osc
#define SET_LED_SLOW(state)		digOut(2,state)	// flash LED3 on 32kHz osc
#define SET_LED_SLOWEST(state)	digOut(3,state)	// light LED4 on slowest clk

int main()
{
	longword seq,ping_who,tmp_seq,time_out;
	int powerIndex;
	int err;
   char ledState;
   int c;

   brdInit();

   ledState = 0; //used to blink LED light on and off

	// at startup, we're in mode 0
   powerIndex = 0;
	set_power (powerIndex);

	sock_init_or_exit (1);

#ifdef PING_WHO
	// Ping a specific IP addr:
	// find the binary IP address for the target of our pinging
	ping_who=resolve(PING_WHO);
	if(! ping_who) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		return 1;
	}
#else
	// Examine our configuration, and ping the default router:
	err = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, &ping_who, IFS_END );
	if( err ) {
		printf( "ERROR: ifconfig() failed --> %d\n", err );
		return 1;
	}
	if(! ping_who) {
		printf("ERROR: unable to resolve IF_ROUTER_DEFAULT\n");
		return 1;
	}
#endif

	printf ("Set up buttons and LEDs...\n");
	setDigIn (0);			// SW1 -- press to lower clock speed (go slower)
	setDigIn (1);			// SW2 -- press to increase clock speed (go faster)
	setDigOut (1, 1);		// LED2 -- flashes when running from main oscillator
	setDigOut (2, 1);		// LED3 -- flashes when running from 32kHz oscillator
	setDigOut (3, 1);		// LED4 -- on when running at slowest speed
	setPWM (0, 10, 50.0, 0, 0);	// LED0 -- RIO-controlled 10Hz PWM @ 50% duty

	seq=0;

	printf ("\nPress SW1 to lower clock speed, SW2 to increase clock speed.\n");
	printf ("LED1 is connected to a 10Hz PWM on the RIO.  Notice how it slows\n");
	printf ("down as you lower the clock speed.  The RIO is clocked at the\n");
	printf ("same speed as the Rabbit so changing clock speeds will change\n");
	printf ("the behavior of your PWM outputs.\n\n");
	printf ("The program flashes LED2 when running from the main oscillator.\n");
	printf ("It flashes LED3 when running from the 32kHz oscillator.  It\n");
	printf ("lights LED4 when you've reached the slowest clock speed (4kHz)\n\n");

	for(;;) {

		costate		// check for button presses
		{
			if (SW_FASTER_PRESSED()) {
				while (SW_FASTER_PRESSED()) yield;
				if (powerIndex < MAX_POWER_SETTING) {
					set_power (++powerIndex);				//set next slower power mode
					if (powerIndex > MAX_MAIN_OSC_INDEX) {
						// turn off the led that flashes when using main oscillator
         			SET_LED_FAST(LED_OFF);

         			// turn off the ADC, we're not going to use it at these speeds
         			ads7870_power(0);
         		}
					if (powerIndex == MAX_POWER_SETTING) {
						// turn on LED that indicates "running at slowest speed"
						SET_LED_SLOWEST(LED_ON);
					}
				}
			}
			if (SW_SLOWER_PRESSED()) {
				while (SW_SLOWER_PRESSED()) yield;
	            if (powerIndex) {
	               set_power (--powerIndex);			//set next faster power mode
	               if (powerIndex <= MAX_MAIN_OSC_INDEX) {
	                  // turn off the LEDs that flashes when using 32kHz osc
	                  SET_LED_SLOW(LED_OFF);

	                  // turn the ADC back on so we can read analog inputs
	                  ads7870_power(1);
	               }
	               // turn off LED that indicates "running at slowest speed"
						SET_LED_SLOWEST(LED_OFF);
	            }
			}
		} //end costate

      /* Only handle tcp events & pings if we are in a
       * power mode that allows for network activity. */
		if(powerIndex <= MAX_ETH_MODE_INDEX) {
         /* It is important to call tcp_tick here because
          * ping packets will not get processed otherwise.
          */
         tcp_tick(NULL);

         // Send one ping per second.
	      costate {
	         _ping(ping_who,seq++);
	         waitfor(DelaySec(1));
	      }

         //	 Has a ping come in?  time_out!=0xfffffff->yes.
         time_out=_chk_ping(ping_who,&tmp_seq);
         if(time_out!=0xffffffff)
            printf("received ping:  %ld (%ld ms)\n", tmp_seq, time_out);
		}

      /* Blink the LED light according to which power mode
       * we are in. When using the main oscillator (in the
       * MHZ range), a delay is introduced. */
      if(powerIndex <= MAX_MAIN_OSC_INDEX)
      {
         costate
         {
            // using main osc., so add a delay
            waitfor(DelayMs (100 * (powerIndex + 1)));
	         ledState ^= 1;    // toggle ledState
            SET_LED_FAST(ledState);
         }
      }
      else //CPU is running off of the 32kHz clock
      {
         ledState ^= 1;		// toggle ledState
         SET_LED_SLOW(ledState);
         /* For cases where the watchdog timer is enabled, and
          * we are operating in a power mode where periodic interrupts
          * are disabled, we also need to hit the watchdog periodically */
         #ifdef HIT_WATCHDOG
            hitwd();
         #endif
         /* If we also use timers, use this opportunity to update
          * timers, such as MS_TIMER, etc.  Uncomment if using timers
          * during low power modes */
         updateTimers();
      }

	} //end for(;;) loop
}
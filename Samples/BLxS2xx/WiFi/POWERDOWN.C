/**********************************************************

   powerdown.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

	This program is used with BL5S220 controllers.

	Description
	===========
   This program demonstrates how to properly power down
   the radio transmitter in order to reduce power
   consumption.

   Note that powering down causes the network interface to
   come down (unlike the case for ethernet), and thus is
   only suitable for applications such as data logging,
   where only intermittent network connectivity is required.

   Because of this complexity, the procedure is demonstrated
   in this sample as a simple sequential state machine.

   Connections:
	============

	****WARNING****: When using the J7 connector, be sure to insulate or cut
      the exposed wire from the wire leads you are not using.  The only
      connection required from J7 for any of the sample programs is +5v.

	1. DEMO board jumper settings:
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller J10 pin 5 GND to the DEMO board
	   J1 GND.

	3. Connect a wire from the controller J7 pin 6 (+5V) to the DEMO board +V.

   4. Connect the following wires from the controller J10 to the DEMO
      board screw terminal:

      From J10 pin 9 DIO0 to LED1
      From J10 pin 4 DIO1 to LED2

	Instructions
	============
	1. Modify the configuration macros, including the DOWNTIME and
	   UPTIME values.  The interface will be powered up and down
	   for these intervals.

	2. Compile and run this program.

	3. LED1 will be on when the network interface is up.
	   LED2 will be on when the radio transceiver is powered up.

	4. On another host, set up a continuous ping and observe
	   the pings successively timeout, then succeed, depending
	   on the LED state.

**********************************************************/
#class auto

/** Define the down- and up-time of the WiFi interface, in milliseconds. **/
#define DOWNTIME  4000
#define UPTIME    5000

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

#memmap xmem
#use "dcrtcp.lib"

/**********************************************************
	Routines to change the LEDs.
***********************************************************/
#use "BLxS2xx.lib"

#define LED1   0
#define LED2   1

#define LEDON	0
#define LEDOFF	1

int ledstatus;


if_led(int onoff)
{
   digOut(LED1, onoff);
}

xcvr_led(int onoff)
{
   digOut(LED2, onoff);
}


main()
{
	int state;
	word tmo;


	brdInit();				//initialize board for this demo

   // Configure IO channels as digital outputs (sinking type outputs)
   setDigOut (LED1, 1);				// Configure LED1 as sinking type output
   setDigOut (LED2, 1);				// Configure LED2 as sinking type output

	// Bring up interface first time (also prints our address)
	sock_init_or_exit(1);

	// First initialization OK, turn on both LEDs
	if_led(LEDON);
	xcvr_led(LEDON);

	state = 0;
	tmo = _SET_SHORT_TIMEOUT(UPTIME);

	for(;;) {
		switch (state) {
		case 0:	// Up, timing out
			tcp_tick(NULL);
			if (_CHK_SHORT_TIMEOUT(tmo)) {
				printf("Bringing interface down...\n");
				state = 1;
				ifdown(IF_WIFI0);
			}
			break;
		case 1:	// bringing down
			tcp_tick(NULL);
			if (ifpending(IF_WIFI0) == IF_DOWN) {
				printf("Powering down...\n");
	         if_led(LEDOFF);
	         xcvr_led(LEDOFF);

            // Set flag for MAC to power down when tcp_tick function is called!
            pd_powerdown(IF_WIFI0);
	         tmo =_SET_SHORT_TIMEOUT(DOWNTIME);
	         state = 2;
	      }
	      break;
	   case 2:	// down, waiting
      	tcp_tick(NULL);
	   	if (_CHK_SHORT_TIMEOUT(tmo)) {
				printf("Powering up...\n");

            // Set flag for MAC to power-up when tcp_tick function is called!
	         pd_powerup(IF_WIFI0);
            xcvr_led(LEDON);
	         tmo =_SET_SHORT_TIMEOUT(500);	// settle for 1/2 sec
	         state = 3;
			}
			break;
		case 3: // let power stabilize
      	tcp_tick(NULL);
			if (_CHK_SHORT_TIMEOUT(tmo)) {
				printf("Bringing interface up...\n");
				ifup(IF_WIFI0);
	         state = 4;
			}
			break;
		case 4:	// waiting for up
			tcp_tick(NULL);
			if (ifpending(IF_WIFI0) != IF_COMING_UP) {
				if (ifpending(IF_WIFI0) == IF_DOWN) {
					printf("!!!!! Failed to come back up!!!!!\n");
					return -1;
				}
				printf("Up again!\n");
				if_led(LEDON);
	         tmo =_SET_SHORT_TIMEOUT(UPTIME);
				state = 0;
			}
			break;
		}
	}
}
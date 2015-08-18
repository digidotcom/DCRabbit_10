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
/**********************************************************

   powerdown.c

	This program is used with RCM4400W series controllers
	with prototyping boards.

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

	Instructions
	============
	1. Modify the configuration macros, including the DOWNTIME and
	   UPTIME values.  The interface will be powered up and down
	   for these intervals.

	2. Compile and run this program.

	3. DS2 will be on when the network interface is up.
	   DS3 will be on when the radio transceiver is powered up.

	4. On another host, set up a continuous ping and observe
	   the pings successively timeout, then succeed, depending
	   on the LED state.

**********************************************************/
#class auto

/** Define the down- and up-time of the WiFi interface, in milliseconds. **/
#define DOWNTIME  5000
#define UPTIME    7000

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
#use "RCM44xxW.lib"

#define DS2 2
#define DS3 3
#define USERLED 0

#define LEDON	0
#define LEDOFF	1

int ledstatus;


if_led(int onoff)
{
	BitWrPortI(PBDR, &PBDRShadow, onoff, DS2);
}

xcvr_led(int onoff)
{
	BitWrPortI(PBDR, &PBDRShadow, onoff, DS3);
}


main()
{
	int state;
	word tmo;


	brdInit();				//initialize board for this demo

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
	         pd_powerdown(IF_WIFI0);
	         tmo =_SET_SHORT_TIMEOUT(DOWNTIME);
	         state = 2;
	      }
	      break;
	   case 2:	// down, waiting
      	tcp_tick(NULL);
	   	if (_CHK_SHORT_TIMEOUT(tmo)) {
				printf("Powering up...\n");
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
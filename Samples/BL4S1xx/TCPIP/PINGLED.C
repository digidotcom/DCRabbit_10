/**********************************************************

   pingled.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

	Description
	===========
   This program demonstrates ICMP by pinging a remote host.
	It will flash LED1 and LED2 on the demo board when a ping
	is sent and received.

	This program was adapted from \Samples\RCM4400W\pingled.c.

	Connections
	===========
	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller GND, to the DEMO board GND.

	3. Connect a wire from the controller +5V to the DEMO board +5V.

   4. Connect the following wires from the controller to the DEMO board:
   		From OUT0 to LED1
   		From OUT1 to LED2

	Instructions
	============
   1. Change PING_WHO to the host you want to ping.
   2. You may modify the PING_DELAY define to change the
   	amount of time in milliseconds between the outgoing
   	pings.
   3. Uncomment the VERBOSE define to see the incoming
      ping replies.
	4. Compile and run this program.
	5. LED1 will flash when a ping is sent.
		LED2 will flash when a ping is received.

	To run the test from two Rabbit controllers, use a different setting
	for MY_IP_ADDRESS on each controller, and set PING_WHO to the other
	controller's MY_IP_ADDRESS.

**********************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// default to storing functions to xmem instead of root
#memmap xmem

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
//#define PING_WHO			"10.10.6.70"

//for two controller test, see description above.
//#define MY_IP_ADDRESS 	"10.10.6.2"

// How often to send PINGs (in milliseconds):
#define PING_DELAY		500

// comment this define to turn off verbose messages to stdout
#define VERBOSE

// LEDs are active low (sink pin to ground to light LED)
#define LEDON	0
#define LEDOFF	1

#define LED_TX	0		// Ping transmit LED on DOUT0
#define LED_RX	1		// Ping receive LED on DOUT1

#define setLED(led,state)	digOut(led, state)

// include the TCP/IP networking library
#use "dcrtcp.lib"

#if PING_DELAY < 100
	// each ping flashes the LED for 50ms, so 100ms is a good minimum for
	// having the LED on 50% of the time
	#fatal "PING_DELAY must be set to at least 100ms"
#endif

main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];

	brdInit();				//initialize board for this demo

	// set up DOUT0 and DOUT1 as general digital outputs
	setDigOut (LED_TX, LEDOFF);
	setDigOut (LED_RX, LEDOFF);

	seq=0;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	/*
	 *		Get the binary ip address for the target of our
	 *		pinging.
	 */

#ifdef PING_WHO
	/* Ping a specific IP addr: */
	ping_who=resolve(PING_WHO);
	if( !ping_who ) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		exit(-EHOSTUNREACH);
	}
#else
	/* Examine our configuration, and ping the default router: */
	tmp_seq = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, & ping_who, IFS_END );
	if( tmp_seq ) {
		printf( "ERROR: ifconfig() failed --> %d\n", (int) tmp_seq );
		exit(-EINVAL);
	}
	if( !ping_who ) {
		printf("ERROR: unable to resolve IFG_ROUTER_DEFAULT\n");
		exit(-EHOSTUNREACH);
	}
#endif

	for(;;) {
		/*
		 *		It is important to call tcp_tick here because
		 *		ping packets will not get processed otherwise.
		 *
		 */

		tcp_tick(NULL);

		/*
		 *		Send one ping every PING_DELAY ms.
		 */

		costate {
			// subtract 50ms from PING_DELAY to account for second DelayMs(50)
			// when flashing the LED
			waitfor(DelayMs(PING_DELAY-50));
			_ping(ping_who,seq++);
			setLED(LED_TX, LEDON);					// flash transmit LED
			waitfor(DelayMs(50));
			setLED(LED_TX, LEDOFF);
		}

		/*
		 *		Has a ping come in?  time_out!=0xfffffff->yes.
		 */

		costate {
			time_out=_chk_ping(ping_who,&tmp_seq);
			if(time_out!=0xffffffff) {

#ifdef VERBOSE
				printf("received ping:  %lu\n", tmp_seq);
#endif

				setLED(LED_RX, LEDON);				// flash receive LED
				waitfor(DelayMs(50));
				setLED(LED_RX, LEDOFF);
			}
		}
	}
}
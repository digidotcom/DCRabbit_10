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

   pingled.c

	This program is used with RCM4200 series controllers
	with prototyping boards.

	Description
	===========
   This program demonstrates ICMP by pinging a remote host.
	It will flash LED's, DS2 and DS3 on the prototyping
	board when a ping is sent and received.

	This program was adapted from \Samples\TCPIP\ping.c.

	Instructions
	============
   1. Change PING_WHO to the host you want to ping.

   2. You may modify the PING_DELAY define to change the
   	amount of time in milliseconds between the outgoing
   	pings.

   3. Uncomment the VERBOSE define to see the incoming
      ping replies.

	4. Compile and run this program.

	5. DS2 will flash when a ping is sent.
		DS3 will flash when a ping is received.


	Quick test with two RCM4200 controllers
	======================================
	1.	Connect a cross-over cable to each controller.
	2. Change #define TCPCONFIG 0.
	3. Uncomment #define MY_IP_ADDRESS and #define PING_WHO.
	4. Compile and run this program on one controller.
	5. Switch the value of MY_IP_ADDRESS and PING_WHO.
	6. Compile and run this program on the second controller.
	7. On each board:
		DS2 will flash when a ping is sent.
		DS3 will flash when a ping is received.

**********************************************************/
#class auto

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
#define PING_WHO			"10.10.6.70"

//for two controller test, see description above.
//#define MY_IP_ADDRESS 	"10.10.6.2"

/** How often to send PING's (in milliseconds): **/
#define PING_DELAY		500


#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"

/**********************************************************
	Routines to change the LEDs.
	The pingleds_setup routine turns off LED's.  The
	pingoutled routine toggles DS2. The pinginled routine
	toggles DS3.
***********************************************************/
#use RCM42xx.lib

#define DS2 2
#define DS3 3
#define USERLED 0

#define LEDON	1
#define LEDOFF	0

int ledstatus;

pingoutled(int onoff)
{
	BitWrPortI(PBDR, &PBDRShadow, !onoff, DS2);
}

pinginled(int onoff)
{
	BitWrPortI(PBDR, &PBDRShadow, !onoff, DS3);
}


main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];

	brdInit();				//initialize board for this demo

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
	if(ping_who==0) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		exit(2);
	}
#else
	/* Examine our configuration, and ping the default router: */
	tmp_seq = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, & ping_who, IFS_END );
	if( tmp_seq != 0 ) {
		printf( "ERROR: ifconfig() failed --> %d\n", (int) tmp_seq );
		exit(2);
	}
	if(ping_who==0) {
		printf("ERROR: unable to resolve IFG_ROUTER_DEFAULT\n");
		exit(2);
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
			waitfor(DelayMs(PING_DELAY));
			_ping(ping_who,seq++);
			pingoutled(LEDON);					// flash transmit LED
			waitfor(DelayMs(50));
			pingoutled(LEDOFF);
		}

		/*
		 *		Has a ping come in?  time_out!=0xfffffff->yes.
		 */

		costate {
			time_out=_chk_ping(ping_who,&tmp_seq);
			if(time_out!=0xffffffff) {

#ifdef VERBOSE
				printf("received ping:  %ld\n", tmp_seq);
#endif

				pinginled(LEDON);					// flash receive LED
				waitfor(DelayMs(50));
				pinginled(LEDOFF);
			}
		}
	}
}
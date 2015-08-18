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

   region_multi_domain.c

	This program is used with 802.11 wifi devices operating on
   their corresponding prototyping or interface boards.

	Description
	===========
	This sample requires an access point that supports and is configured
	for 802.11d.

   This program demonstrates how the multi-domain option can be used
   for configuring your device to meet regional regulations. The demo
   also includes doing pings, which will indicate that the wifi
   device has successfully received country information from your
   access point.

   General info:
   -------------
   The 802.11d region you select will automatically determine the
   power and channel requirements for operating your wifi device.
   Recommend checking the regulations where your wireless devices
   will be deployed for any other requirements.

	Instructions
	============
   1. Inspect code for ifconfig function usage to see how it
      enables multi-domain for your wifi device.
   2. Set network MACRO parameters.
   3. Enable 802.11d option on your access point and select the proper
   country where your device is deployed (see note below).
   4. Compile and run program.
   5. Step through the menu selections to view user options.
   6. With WIFI_REGION_VERBOSE defined, view STDIO to see channel
      and power settings which the access point has sent to your
      wifi device.

   Note: If the AP doesn't support 802.11d or if the option is not
   enabled on the AP, then your wifi device will not be able to
   associate with the AP.
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

// Macro to see channel and power level settings being done by the wifi driver
#define WIFI_REGION_VERBOSE

/** Remote interface to send PING to (passed to resolve()): **/
/*  Undefine to retrieve default gateway and ping that. */
#define PING_WHO			"10.10.6.1"


/** How often to send PING's (in milliseconds): **/
#define PING_DELAY		500

#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"

/**********************************************************
	Routines to change the LEDs.
	The pingleds_setup routine turns off LED's.  The
	pingoutled routine toggles DS2. The pinginled routine
	toggles DS3. On the RCM5600W both in and out use the
   same DS1 LED, it blinks once for ping sent and twice
   when the ping response is received.
***********************************************************/
#if (RCM5600W_SERIES)
  #use "RCM56xxW.lib"
  #define pingoutled(onoff) BitWrPortI(PDDR, &PDDRShadow, onoff, 0);
  #define pinginled(onoff)  BitWrPortI(PDDR, &PDDRShadow, onoff, 0);
#else
  #if (RCM4400W_SERIES)
	 #use "RCM44xxW.lib"
  #elif (RCM5400W_SERIES)
    #use "RCM54xxW.lib"
  #endif
  #define pingoutled(onoff) BitWrPortI(PBDR, &PBDRShadow, onoff, 2);
  #define pinginled(onoff)  BitWrPortI(PBDR, &PBDRShadow, onoff, 3);
#endif

#define LEDON	1
#define LEDOFF	0

int ledstatus;


main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];


	brdInit();				//initialize board for this demo
	seq=0;

	sock_init();			// Initialize wifi interface

   // Make sure wifi IF is down to do ifconfig's functions
	printf("\nBringing interface down (disassociate)...\n");
   ifdown(IF_WIFI0);
   while (ifpending(IF_WIFI0) != IF_DOWN) {
     	printf(".");
     	tcp_tick(NULL);
   }
	printf("...Done.\n");

   // Enable 802.11d country information capability
   // Note: Access Point must have 802.11d enabled with proper country selected
   ifconfig(IF_WIFI0, IFS_WIFI_MULTI_DOMAIN, 1, IFS_END);

   // Startup the  wireless interface here...
	printf("Bringing interface back up (associate)...\n");
   ifup(IF_WIFI0);
   while (ifpending(IF_WIFI0) == IF_COMING_UP) {
      tcp_tick(NULL);
   }
	printf("...Done.\n");
	if (ifpending(IF_WIFI0) != IF_UP) {
		printf("Unfortunately, it failed to associate :-(\n");
		exit(1);
	}
   // End of regional setting section, from this point on do standard tcp/ip
   // protocol.


   /*
   // Here is where we gather the statistics...
	// Note that if you get a compile error here, it is because you are not running
	// this sample on a Wifi-equipped board.

	/* Print who we are... */
	printf( "My IP address is %s\n\n", inet_ntoa(buffer, gethostid()) );

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
			pingoutled(LEDON);					// flash transmit LED
			waitfor(DelayMs(50));
			pingoutled(LEDOFF);
			_ping(ping_who,seq++);
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
#if RCM5600W_SERIES
				waitfor(DelayMs(250));
				pinginled(LEDON);					// flash receive LED again
				waitfor(DelayMs(50));
				pinginled(LEDOFF);
#endif
			}
		}
	}
}
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

   pingled_wpa_psk.c

	This program is used with RCM4400W series controllers
	with prototyping boards.

	Description
	===========
   This program is an extension of pingled.c.  It demonstrates use
   of WPA PSK (Wifi Protected Access with Pre-Shared Key).

   WPA is a more secure replacement for WEP.  This implementation
   supports use of TKIP (Temporal Key Integrity Protocol) cypher
   suite.

   Apart from the configuration of WPA PSK at the top of the sample,
   the rest of the code is identical to the case without WPA.
   Indeed, most of the TCP/IP samples should work with WPA PSK
   simply by using the same configuration settings.

	Instructions
	============
	1. Configure your access point for WPA PSK.  Specify TKIP
	   cypher suite, and enter a suitable pre-shared key.  The key
	   may be entered as 64 hexadecimal digits, or an ascii string
	   of up to 63 characters.  Authentication should be set to
	   "open system", which basically means that knowledge of the
	   key is sufficient to allow access.

	   HINT: since the key is long, there is a good chance of
	   typos.  Hence, it is advisable to enter the key in this
	   sample first, then copy and paste into your access point.
	   This ensures that both the RCM4400W and the access point
	   have the same key!

	   HINT2: for an initial test, it may be easier to use the
	   64 hex digit form of the key rather than the ascii
	   passphrase.  A passphrase requires considerable computation
	   effort which delays the startup of the sample by about
	   40 seconds.

	   See the configuration section below for more details.

	   The following steps are much the same as in the basic
	   pingled.c sample...

   2. Change PING_WHO to the host you want to ping.

   3. You may modify the PING_DELAY define to change the
   	amount of time in milliseconds between the outgoing
   	pings.

   4. Uncomment the VERBOSE define to see the incoming
      ping replies.

	5. Compile and run this program.

	6. DS2 will flash when a ping is sent.
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

/*
 * It is necessary to define these to include WPA support.
 */
#define WIFI_USE_WPA												// Bring in WPA support
#define IFC_WIFI_ENCRYPTION	IFPARAM_WIFI_ENCR_TKIP	// Define cypher suite

/*
 * ASCII passphrase generation requires not only the passphrase itself,
 * but also the correct SSID with which it will be used.  Thus, it is
 * particularly important to give the correct access point SSID here.
 */
#define IFC_WIFI_SSID			"parvati"

/*
 * Define an ASCII passphrase here, from 1 to 63 characters inclusive.
 * This is only used if you did NOT specify a hexadecimal key (see below,
 * for the IFC_WIFI_WPA_PSK_HEXSTR macro).
 */
//#define IFC_WIFI_WPA_PSK_PASSPHRASE "now is the time"

/*
 * If you specify a hex key here, it is used instead of the passphrase
 * above.  Hex keys should be exactly 64 hex digits long (i.e. 32 bytes).
 */
// Following is valid for "now is the time" used with SSID="parvati"
#define IFC_WIFI_WPA_PSK_HEXSTR \
        "75BA7FD02288E764DCE26382768F58BE96EBB59B8CB952C871AED8B0356922E8"

// Here is a simple key for testing
//#define IFC_WIFI_WPA_PSK_HEXSTR \
//        "1010101010101010101010101010101010101010101010101010101010101010"

// Of course, you do not have to hard-code keys as a string.  You can
// also refer to a program variable as in the following...
// (note that passphrases are conventionally limited to 63 characters).
//char my_psk[64];
//#define IFC_WIFI_WPA_PSK_HEXSTR my_psk

/*
 * Define this to print helpful messages when using WPA keys
 */
#define WIFI_VERBOSE_PASSPHRASE


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
	toggles DS3.
***********************************************************/
#use "RCM44XXW.lib"

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
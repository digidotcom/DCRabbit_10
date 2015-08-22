/**********************************************************

   pingled_wpa2_ccmp.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

	This program is used with BL5S220 controllers.

	Description
	===========
   This program is an extension of pingled.c.  It demonstrates use
   of WPA2 PSK (Wifi Protected Access with Pre-Shared Key).

   WPA2 is a more secure replacement for WEP.  This implementation
   uses the Advanced Encryption Standard (AES) based algorithm, also known
   as CCMP (Counter Mode with Cipher Block Chaining Message Authentication
   Code Protocol) cypher suite.

   Apart from the configuration of WPA2_CCMP at the top of the sample,
   the rest of the code is identical to the case without WPA2 PSK.
   Indeed, most of the TCP/IP samples should work with WPA2 CCMP
   simply by using the same configuration settings.

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
	1. Configure your access point for WPA2 PSK.  Specify CCMP
	   cypher suite, and enter a suitable pre-shared key.  The key
	   may be entered as 64 hexadecimal digits, or an ascii string
	   of up to 63 characters.  Authentication should be set to
	   "open system", which basically means that knowledge of the
	   key is sufficient to allow access.

	   HINT: since the key is long, there is a good chance of
	   typos.  Hence, it is advisable to enter the key in this
	   sample first, then copy and paste into your access point.
	   This ensures that both the BL5S220 and the access point
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

	6. LED1 will flash when a ping is sent.
		LED2 will flash when a ping is received.

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
#define WIFI_USE_WPA								// Bring in WPA_PSK support
#define WIFI_AES_ENABLED                 	// Enable AES specific code
#define IFC_WIFI_ENCRYPTION	IFPARAM_WIFI_ENCR_CCMP	// Define encryption cypher suite

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
	The pingoutled routine toggles LED1. The pinginled
	routine toggles LED2.
***********************************************************/
#use "BLxS2xx.lib"

#define LED1   0
#define LED2   1

#define LEDON	0
#define LEDOFF	1

int ledstatus;

pingoutled(int onoff)
{
   digOut(LED1, onoff);
}

pinginled(int onoff)
{
   digOut(LED2, onoff);
}


main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];

	brdInit();				//initialize board for this demo

   // Configure IO channels as digital outputs (sinking type outputs)
   setDigOut (LED1, 1);				// Configure LED1 as sinking type output
   setDigOut (LED2, 1);				// Configure LED2 as sinking type output

   // Set the initial state of the LED's
   digOut(LED1, LEDOFF);
   digOut(LED2, LEDOFF);

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
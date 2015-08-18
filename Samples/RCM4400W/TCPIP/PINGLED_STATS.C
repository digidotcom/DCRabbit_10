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

   pingled_stats.c

	This program is used with RCM4400W series controllers
	with prototyping boards.

	Description
	===========
   This program is similar to pingled.c, except that it
   also prints receiver/transmitter statistics.

	Instructions
	============
   1. Change PING_WHO to the host you want to ping.

   2. You may modify the PING_DELAY define to change the
   	amount of time in milliseconds between the outgoing
   	pings.

   3. Uncomment the VERBOSE define to see the incoming
      ping replies, and print statistics.

	4. Modify the MOVING_AVERAGE macro value to alter the
	   moving average filtering of the statistics.  Also,
	   check out GATHER_INTERVAL and GRAPHICAL.

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

/** Remote interface to send PING to (passed to resolve()): **/
/*  Undefine to retrieve default gateway and ping that. */
#define PING_WHO			"10.10.6.1"

/** How often to send PING's (in milliseconds): **/
#define PING_DELAY		100

/** How many past samples to use to compute moving average smoothing **/
#define MOVING_AVERAGE	5

/** How many samples to gather for each line printed **/
#define GATHER_INTERVAL	25

/** Define to make bar graph display (else numeric) **/
#define GRAPHICAL


#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"

/**********************************************************
	Routines to change the LEDs.
	The pingleds_setup routine turns off LED's.  The
	pingoutled routine toggles DS2. The pinginled routine
	toggles DS3.
***********************************************************/
#use "RCM44xxW.lib"

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


/*

This is the definition of the wstatus variable...

   typedef struct {
      wln_state   state;
      uint8       ssid[WLN_SSID_SIZE];
      int         ssid_len;
      int         channel;
      mac_addr    bss_addr;
      uint16      bss_caps;
      uint8       wpa_info[WLN_WPAIE_SIZE];
      uint32      authen;
      uint32      encrypt;
      int         tx_rate;
      int         rx_rate;
      int         rx_signal;
      int         tx_power;
      uint8       country_info[WLN_COUNTRY_STRLEN];
   } wifi_status;

*/
wifi_status wstatus;

char rxgraph[16];
char txgraph[16];

char * graph(char * buf, float num)
{
	// num is a wifi rate in Kbits/sec.  buf is a 16-char array, which we fill with
	// a 15-* bar graph (and null term).  One * == 1Mbit
	word s;

	s = (word)(num * 0.001);
	if (s > 15) s = 15;
	memset(buf, ' ', 15);
	buf[15] = 0;
	memset(buf, '*', s);
	return buf;
}

main()
{
	char	buffer[100];
	longword seq,ping_who,tmp_seq,time_out;
	float mavg_txrate, mavg_rxrate;
	float avg_txrate, avg_rxrate;
	float txrate, rxrate;
	int gather;

	brdInit();				//initialize board for this demo

	seq=0;

	mavg_txrate = mavg_rxrate = 0.0;
	avg_txrate = avg_rxrate = 0.0;

	// Start network and wait until associated
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

#ifdef VERBOSE
	printf("\n");
	printf("Note: RSSI = Receive Signal Strength Indicator\n");
	printf("The last display line shows a filtered rate estimate.\n");
	printf("Every %u samples (at %u ms intervals) these rates will be\n",
		GATHER_INTERVAL, PING_DELAY);
	printf("  set to the average over the last %u ms, and the next\n",
		GATHER_INTERVAL * PING_DELAY);
	printf("  line is started.\n");
	#ifdef GRAPHICAL
	printf("Bar graphs are * == 1Mbit/sec\n");
	printf("\n\n");
   printf("\tRx rate (Kbit)\t\tTx rate (Kbit)\t\tRSSI (dB)\n");
   printf("Seq\tLast\tFilt.\t\tLast\tFilt.\t\tFilt.\n");
   printf("------- ------- --------------- ------- --------------- -------\n");
   #else
	printf("\n\n");
   printf("\tRx rate (Kbit)\tTx rate (Kbit)\tRSSI (dB)\n");
   printf("Seq\tLast\tFilt.\tLast\tFilt.\tFilt.\n");
   printf("------- ------- ------- ------- ------- -------\n");
   #endif
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

				// Here is where we gather the statistics...
				// Note that if you get a compile error here, it is because you are not running
				// this sample on a Wifi-equipped board.
				ifconfig (IF_WIFI0, IFG_WIFI_STATUS, &wstatus, IFS_END);

				txrate = wstatus.tx_rate * 100.0;	// Convert to Kbit/sec
				rxrate = wstatus.rx_rate * 100.0;

				mavg_txrate = mavg_txrate * ((MOVING_AVERAGE - 1.0)/MOVING_AVERAGE) +
				              txrate * (1.0/MOVING_AVERAGE);
				mavg_rxrate = mavg_rxrate * ((MOVING_AVERAGE - 1.0)/MOVING_AVERAGE) +
				              rxrate * (1.0/MOVING_AVERAGE);

				avg_txrate += txrate;
				avg_rxrate += rxrate;

				gather = tmp_seq % GATHER_INTERVAL == 0;
#ifdef VERBOSE

		 #ifdef GRAPHICAL
				printf("%7lu %7u %15.15s %7u %15.15s %7d       %c",
					tmp_seq,
					(word)rxrate,
					graph(rxgraph, gather ? (word)(avg_rxrate / GATHER_INTERVAL) : (word)mavg_rxrate),
					(word)txrate,
					graph(txgraph, gather ? (word)(avg_txrate / GATHER_INTERVAL) : (word)mavg_txrate),
					wstatus.rx_signal >> _WIFI_RSSI_SCALE_SHIFT,
					gather ? '\n' : '\r'
					);
		 #else
				printf("%7lu %7u %7u %7u %7u %7d       %c",
					tmp_seq,
					(word)rxrate,
					gather ? (word)(avg_rxrate / GATHER_INTERVAL) : (word)mavg_rxrate,
					(word)txrate,
					gather ? (word)(avg_txrate / GATHER_INTERVAL) : (word)mavg_txrate,
					wstatus.rx_signal >> _WIFI_RSSI_SCALE_SHIFT,
					gather ? '\n' : '\r'
					);
		#endif
#endif

				if (gather)
					avg_txrate = avg_rxrate = 0.0;

				pinginled(LEDON);					// flash receive LED
				waitfor(DelayMs(50));
				pinginled(LEDOFF);
			}
		}
	}
}
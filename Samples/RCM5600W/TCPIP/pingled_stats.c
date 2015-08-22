/**********************************************************

   pingled_stats.c
	Digi International, Copyright (C) 2008.  All rights reserved.

	This program is used with RCM56xxW series controllers and interface board,
	and optionaly with a digital I/O accessory board available with the
   deluxe dev kit.

   See "Add Additional Boards" in the User's Manual for instructions on
   how to attach the accessory board.

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

	6. For the interface board of the standard kit:
       DS2 will flash when a ping is sent.
		 DS3 will flash when a ping is received.
      For the digital I/O accessory board of the deluxe dev kit:
       DS1 will go on with a brief toggle off when a ping is sent.
		 DS1 will go off for a longer duration when a ping is received.

**********************************************************/
#class auto

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

// NETWORK CONFIGURATION
// Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
// compile-time network configuration.
#define TCPCONFIG 1

// Remote interface to send PING to (passed to resolve()):
// Undefine to retrieve and ping default gateway.
//#define PING_WHO		 "10.10.6.1"

// SSID of the wireless network to connect to
// Uncomment and define IFC_WIFI_SSID to match your network.
//#define IFC_WIFI_SSID "rabbitTest"

// How often to send PING's (in milliseconds):
#define PING_DELAY		100

// Comment the following define to use only interface board with its one
// button and corresponding LED, uncomment to use the digital I/O accessory
// board with its four buttons and corresponding LEDs.
//#define DIGITAL_IO_ACCESSORY

/** How many past samples to use to compute moving average smoothing **/
#define MOVING_AVERAGE	5

/** How many samples to gather for each line printed **/
#define GATHER_INTERVAL	25

/** Define to make bar graph display (else numeric) **/
#define GRAPHICAL


#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"
#use "rcm56xxw.lib"

#ifdef DIGITAL_IO_ACCESSORY
   #define DS1 4
   #define DS2 5
   #define DS3 6
   #define DS4 7
#else
   #define DS1 0
#endif

#define LEDON	0
#define LEDOFF	1

//	Routines to change the LEDs.
//	The pingleds_setup routine turns off LED's.  The
//	pingoutled routine toggles DS2. The pinginled routine
//	toggles DS3.
pingoutled(int onoff)
{
#ifdef DIGITAL_IO_ACCESSORY
	BitWrPortI(PADR, &PADRShadow, onoff, DS2);
#else
	BitWrPortI(PDDR, &PDDRShadow, onoff, DS1);
#endif
}

pinginled(int onoff)
{
#ifdef DIGITAL_IO_ACCESSORY
	BitWrPortI(PADR, &PADRShadow, onoff, DS3);
#else
	BitWrPortI(PDDR, &PDDRShadow, onoff, DS1);
#endif
}

#ifdef DIGITAL_IO_ACCESSORY
InitIO()
{
   // Set Port A pins for LEDs low
   BitWrPortI(PADR, &PADRShadow, 1, DS1);
   BitWrPortI(PADR, &PADRShadow, 1, DS2);
   BitWrPortI(PADR, &PADRShadow, 1, DS3);
   BitWrPortI(PADR, &PADRShadow, 1, DS4);
}
#endif

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
	auto char ch;

	// num is a wifi rate in Kbits/sec.  buf is a 16-char array, which we fill with
	// a 15 symbol bar graph (* == 1Mbit, # == 4Mbit, and null term).
	word s;

   memset(buf, ' ', 15);
   if (num < 15000)
   {
   	s = (word)(num * 0.001);
      if (s > 15) {
   		s = 15;
      }
		memset(buf, '*', s);
      buf[15] = '\0';
   }
   else
   {
    	s = (word)(num * 0.00025);
      if (s > 15) {
   		s = 15;
      }
		memset(buf, '#', s);
      buf[15] = '\0';
   }
	return buf;
}

main()
{
	char	buffer[100];
	longword seq,ping_who,tmp_seq,time_out;
	float mavg_txrate, mavg_rxrate;
	float avg_txrate, avg_rxrate;
	float txrate, rxrate;
	int gather, ping_sent;

	brdInit();				//initialize board for this demo
#ifdef DIGITAL_IO_ACCESSORY
	InitIO();
#endif

	seq=0;
   ping_sent = 0;

	mavg_txrate = mavg_rxrate = 0.0;
	avg_txrate = avg_rxrate = 0.0;

	// Start network and wait until associated
	sock_init_or_exit(1);

#ifdef PING_WHO
	/* Ping a specific IP addr: */
	ping_who=resolve(PING_WHO);
	if (ping_who==0) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		exit(-1);
	}
#else
	/* Examine our configuration, and ping the default router: */
	tmp_seq = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, & ping_who, IFS_END );
	if ( tmp_seq != 0 ) {
		printf( "ERROR: ifconfig() failed --> %d\n", (int) tmp_seq );
		exit(-2);
	}
	if (ping_who==0) {
		printf("ERROR: unable to resolve IFG_ROUTER_DEFAULT\n");
		exit(-3);
	}
#endif

#ifdef VERBOSE
	printf("\n");
	printf("Note: RSSI = Receive Signal Strength Indicator, a unitless\n");
   printf("   measure of relative signal strength.\n");
	printf("The last display line shows a filtered rate estimate.\n");
	printf("Every %u samples (at %u ms intervals) these rates will be\n",
		GATHER_INTERVAL, PING_DELAY);
	printf("  set to the average over the last %u ms, and the next\n",
		GATHER_INTERVAL * PING_DELAY);
	printf("  line is started.\n");
	#ifdef GRAPHICAL
	printf("Bar graphs are * == 1Mbit/sec and # == 4Mbit/sec\n");
	printf("\n\n");
   printf("\tRx rate (Kbit)\t\tTx rate (Kbit)\t\tRSSI\n");
   printf("Seq\tLast\tFilt.\t\tLast\tFilt.\t\tFilt.\n");
   printf("------- ------- --------------- ------- --------------- -------\n");
   #else
	printf("\n\n");
   printf("\tRx rate (Kbit)\tTx rate (Kbit)\tRSSI\n");
   printf("Seq\tLast\tFilt.\tLast\tFilt.\tFilt.\n");
   printf("------- ------- ------- ------- ------- -------\n");
   #endif
#endif

	for (;;) {
		//	It is important to call tcp_tick here because
		//	ping packets will not get processed otherwise.
		tcp_tick(NULL);
		tcp_tick(NULL);

		/*
		 *		Send one ping every PING_DELAY ms.
		 */

      // When standard kit interface board is used with its single LED it is
      // difficult to show with that LED that each ping occurs as well as
      // show when a ping is received. For this reason the ping send and
      // receive costate actions are predicated on ping_sent clauses so that
      // the LED on, off and delays are not interleaved. Pinging will result
      // in the LED being on except for a brief toggle off. Receiving pings
      // will result in the LED going off for a longer duration.
		costate {
      	if (!ping_sent)
         {
            waitfor(DelayMs(PING_DELAY));  // Send one ping every PING_DELAY ms.
            _ping(ping_who,seq++);
            pingoutled(LEDON);             // transmit LED on
            waitfor(DelayMs(50));
            pingoutled(LEDOFF);            // transmit LED off
#ifndef DIGITAL_IO_ACCESSORY
            waitfor(DelayMs(50));
            pingoutled(LEDON);             // toggle interface board LED
#endif
            ping_sent = 1;
         }
		}

		costate {
      	if (ping_sent)
         {
            time_out=_chk_ping(ping_who,&tmp_seq);  // Has a ping come in?
            if (time_out!=0xffffffff) {

               // Here is where we gather the statistics...
               // Note that if you get a compile error here,
               // it is because you are not running
               // this sample on a Wifi-equipped board.
               ifconfig (IF_WIFI0, IFG_WIFI_STATUS, &wstatus, IFS_END);

               txrate = wstatus.tx_rate * 100.0;   // Convert to Kbit/sec
               rxrate = wstatus.rx_rate * 100.0;

               mavg_txrate =
               	mavg_txrate * ((MOVING_AVERAGE - 1.0)/MOVING_AVERAGE) +
                  txrate * (1.0/MOVING_AVERAGE);
               mavg_rxrate =
               	mavg_rxrate * ((MOVING_AVERAGE - 1.0)/MOVING_AVERAGE) +
                  rxrate * (1.0/MOVING_AVERAGE);

               avg_txrate += txrate;
               avg_rxrate += rxrate;

               gather = (tmp_seq % GATHER_INTERVAL == 0);
#ifdef VERBOSE

          #ifdef GRAPHICAL
               printf("%7lu %7u %15.15s %7u %15.15s %7d       %c",
                  tmp_seq,
                  (word)rxrate,
                  graph(rxgraph, gather ?
                  	(word)(avg_rxrate / GATHER_INTERVAL) : (word)mavg_rxrate),
                  (word)txrate,
                  graph(txgraph, gather ?
                  	(word)(avg_txrate / GATHER_INTERVAL) : (word)mavg_txrate),
                  wstatus.rx_signal >> _WIFI_RSSI_SCALE_SHIFT,
                  gather ? '\n' : '\r'
                  );
          #else
               printf("%7lu %7u %7u %7u %7u %7d       %c",
                  tmp_seq,
                  (word)rxrate,
                  gather ?
                  	(word)(avg_rxrate / GATHER_INTERVAL) : (word)mavg_rxrate,
                  (word)txrate,
                  gather ?
                  	(word)(avg_txrate / GATHER_INTERVAL) : (word)mavg_txrate,
                  wstatus.rx_signal >> _WIFI_RSSI_SCALE_SHIFT,
                  gather ? '\n' : '\r'
                  );
         #endif
#endif

               if (gather) {
                  avg_txrate = avg_rxrate = 0.0;
               }

#ifdef DIGITAL_IO_ACCESSORY
               pinginled(LEDON);           // toggle accessory board LED
               waitfor(DelayMs(50));
#endif
               pinginled(LEDOFF);
               waitfor(DelayMs(50));
            }
            ping_sent = 0;
         }
		}
	}
}
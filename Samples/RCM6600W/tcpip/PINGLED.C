/**********************************************************

   pingled.c
	Digi International, Copyright (C) 2008.  All rights reserved.

	This program is used with RCM66xxW series controllers and interface board,
	and optionally with digital I/O accessory board, available with the
   deluxe dev kit.

   See "Add Additional Boards" in the User's Manual for instructions on
   how to attach the accessory board.

   The RCM6600W has two network interfaces, WiFi and Ethernet.
   By default, both are enabled, however you can disable either by
   adding one of the following macros in the project->defines box:
     DISABLE_ETHERNET
     DISABLE_WIFI
   If you are running this sample with both interfaces enabled, and with
   a static (non-DHCP) network configuration, then you will need to
   define a primary and secondary IP address as follows (in the project
   defines):
	   _PRIMARY_STATIC_IP="10.10.6.100"
	   _PRIMARY_NETMASK="255.255.255.0"
	   _SECONDARY_STATIC_IP="10.66.66.66"
	   _SECONDARY_NETMASK="255.255.0.0"
	   MY_GATEWAY="10.10.6.1"
	   MY_NAMESERVER="200.100.50.25"
	(Changing the numeric addresses as appropriate).  The primary
	address gets assigned to the Ethernet interface, and the
	secondary to the WiFi interface.

	It's much easier to use DHCP.  In this case, just change the
	TCPCONFIG definition (below, in this sample) to 5, and everything
	should "just work".

	With dual interfaces, you should be able to access this board from
	either network.  When run, the IP addresses of both interfaces
	will be printed on the stdio window.

	Description
	===========
   This program demonstrates ICMP by pinging a remote host.
   If the interface board only is used, it will produce a long flash when a
   ping is sent and a short flash when a ping is received.
	If a digital I/O accessory board is used, its DS2 LED will flash when a
   ping is sent and its DS3 LED when a ping is sent and received.

	This program was adapted from \Samples\TCPIP\ping.c.

   Jumper settings (Interface Board)
   ----------------------------
   JP1   1-2 program mode
         5-6 enables DS1 (LED)
         7-8 enables S1  (button), optional

         2    4    6   8
         o    o    o   o
         |         |
         o    o    o   o
         1    3    5   7

   Use the following jumper placements on the digital I/O
   accessory board:

	I/O control       On Digital I/O board
	--------------    ---------------------
	Port A bits 4-7  	LEDs DS1-DS4
	Port B bits 4-7	Button switches S1-S4

   Jumper settings (Digital I/O board)
   -----------------------------------
   JP7   2-4, 3-5

      1 o    o 2
             |
      3 o    o 4
        |
      5 o    o 6

      7 o    o 8


   JP5   1-2, 3-4, 5-6, 7-8
   JP8   1-2, 3-4, 5-6, 7-8

         2    4    6   8
         o    o    o   o
         |    |    |   |
         o    o    o   o
         1    3    5   7

	Instructions
	============
   1. Change PING_WHO to the host you want to ping.

   2. You may modify the PING_DELAY define to change the
   	amount of time in milliseconds between the outgoing
   	pings.

   3. Uncomment the VERBOSE define to see the incoming
      ping replies.

	4. Compile and run this program.

	5. For the interface board of the standard kit:
       DS1 will go on with a brief toggle off when a ping is sent.
		 DS1 will go off for a longer duration when a ping is received.
      For the digital I/O accessory board of the deluxe dev kit:
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

// NETWORK CONFIGURATION
// Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
// compile-time network configuration.
#define TCPCONFIG 1

// Remote interface to send PING to (passed to resolve()):
// Undefine to retrieve and ping default gateway.
//#define PING_WHO		 "10.10.6.1"

// How often to send PING's (in milliseconds):
#define PING_DELAY		500

// Comment the following define to use only interface board with its one
// button and corresponding LED, uncomment to use the digital I/O accessory
// board with its four buttons and corresponding LEDs.
//#define DIGITAL_IO_ACCESSORY

#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"
#use "rcm66xxw.lib"

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

main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];
   int ping_sent;

	brdInit();				// initialize board for this demo
#ifdef DIGITAL_IO_ACCESSORY
	InitIO();
#endif

	seq=0;
   ping_sent = 0;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

#ifdef PING_WHO
	// Ping a specific IP addr:
	ping_who = resolve(PING_WHO);
	if (ping_who == 0) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		exit(2);
	}
#else
	// Examine our configuration, and ping the default router:
	tmp_seq = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, & ping_who, IFS_END );
	if ( tmp_seq != 0 ) {
		printf( "ERROR: ifconfig() failed --> %d\n", (int) tmp_seq );
		exit(2);
	}
	if (ping_who==0) {
		printf("ERROR: unable to resolve IFG_ROUTER_DEFAULT\n");
		exit(2);
	}
#endif

	for (;;) {
		//	It is important to call tcp_tick here because
		//	ping packets will not get processed otherwise.
		tcp_tick(NULL);

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
            time_out = _chk_ping(ping_who,&tmp_seq);  // Has a ping come in?
            if (time_out != 0xffffffff) {

#ifdef VERBOSE
               printf("received ping:  %ld\n", tmp_seq);
#endif

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
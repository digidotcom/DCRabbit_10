/*******************************************************************************

   powerdown.c
   Digi International, Copyright (C) 2007-2012.  All rights reserved.

   This program is used with RCM6600W series controllers
   with prototyping boards.

   The RCM6600W has two network interfaces, WiFi and Ethernet.
   By default, both are enabled, however you can disable either by
   adding one of the following macros in the project->defines box:
      DISABLE_ETHERNET
      DISABLE_WIFI

   Note that this sample demonstrates WiFi power up / down sequencing,
   so custom defining the DISABLE_WIFI macro is not allowed. However,
   the DISABLE_ETHERNET macro may be defined (e.g. add DISABLE_ETHERNET
   into Dynamic C's Project Options' Defines tab).

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
   This program demonstrates how to properly power down
   the radio transmitter in order to reduce power
   consumption.

   Note that powering down causes the network interface to
   come down (unlike the case for ethernet), and thus is
   only suitable for applications such as data logging,
   where only intermittent network connectivity is required.

   Because of this complexity, the procedure is demonstrated
   in this sample as a simple sequential state machine.

   Optionally, this sample also cycles the Ethernet network interface
   up and down via an even simpler sequential state machine that does
   not power-cycle the Ethernet interface.

   Instructions
   ============
   1. Modify the configuration macros, including the DOWNTIME and
      UPTIME values.  The interface will be powered up and down
      for these intervals.

   2. Compile and run this program.

   3. DS2 will be on when the WiFi network interface is up.
      DS3 will be on when the radio transceiver is powered up.

   4. On another host, set up a continuous ping to the WiFi interface
      and observe the pings successively timeout, then succeed,
      matching with the LED state.

   5. (Optional.) On another host, set up a continuous ping to the
      enabled Ethernet interface and observe the pings successively
      timeout, then succeed, matching with the Ethernet state reported
      in the STDIO window.

*******************************************************************************/
#class auto

#if DISABLE_WIFI
	#error "This sample requires that WiFi be enabled; the DISABLE_WIFI macro"
	#error " must not be custom defined."
	#error "Check Dynamic C's Program Options' Defines tab to ensure that it"
	#fatal " does not contain DISABLE_WIFI."
#endif

/** Define the down- and up-time of the WiFi interface, in milliseconds. **/
#define DOWNTIME  3000
#define UPTIME    12000

/** Define the DHCP timeout for the WiFi interface, in seconds. **/
#define DHCP_CUSTOM_TIMEOUT 30

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
#use "RCM66xxW.LIB"

#define DS1 0
#define USERLED 0

#define LEDON	0
#define LEDOFF	1

int ledstatus;


if_led(int onoff)
{
	BitWrPortI(PDDR, &PDDRShadow, onoff, DS1);
}


void main(void)
{
	#if !DISABLE_ETHERNET
	int ifp_ETH0;
	int state_ETH0;
	word tmo_ETH0;
	#endif
	int ifp_WIFI0;
	int state_WIFI0;
	word tmo_WIFI0;

	brdInit();				//initialize board for this demo

	// Bring up interface(s) first time (also prints our address)
	sock_init_or_exit(1);

	// First initialization OK, turn on both LEDs
	if_led(LEDON);

	#if !DISABLE_ETHERNET
	state_ETH0 = 0;
	tmo_ETH0 = _SET_SHORT_TIMEOUT(UPTIME);
	#endif

	state_WIFI0 = 0;
	tmo_WIFI0 = _SET_SHORT_TIMEOUT(UPTIME);

	for(;;) {
		tcp_tick(NULL);

		#if !DISABLE_ETHERNET
		switch (state_ETH0) {
		case 0:	// Up, timing out
			if (_CHK_SHORT_TIMEOUT(tmo_ETH0)) {
				printf("Bringing Ethernet interface down...\n");
				state_ETH0 = 1;
				ifdown(IF_ETH0);
			}
			break;
		case 1:	// bringing down
			if (ifpending(IF_ETH0) == IF_DOWN) {
				tmo_ETH0 =_SET_SHORT_TIMEOUT(DOWNTIME);
				state_ETH0 = 2;
			}
			break;
		case 2:	// down, waiting
			if (_CHK_SHORT_TIMEOUT(tmo_ETH0)) {
				printf("Bringing Ethernet interface up...\n");
				ifup(IF_ETH0);
				state_ETH0 = 3;
			}
			break;
		case 3:	// waiting for up
			ifp_ETH0 = ifpending(IF_ETH0);
			if (ifp_ETH0 == IF_UP)
			{
				printf("\nEthernet interface is up again!\n");
				tmo_ETH0 =_SET_SHORT_TIMEOUT(UPTIME);
				state_ETH0 = 0;
			}
			else if (ifp_ETH0 == IF_DOWN) {
				printf("!!!!! Ethernet failed to come back up!!!!!\n");
				tmo_ETH0 = _SET_SHORT_TIMEOUT(DOWNTIME);
				state_ETH0 = 0;
			}
			break;
		}
		#endif

		switch (state_WIFI0) {
		case 0:	// Up, timing out
			if (_CHK_SHORT_TIMEOUT(tmo_WIFI0)) {
				printf("Bringing WiFi interface down...\n");
				state_WIFI0 = 1;
				ifdown(IF_WIFI0);
			}
			break;
		case 1:	// bringing down
			if (ifpending(IF_WIFI0) == IF_DOWN) {
				printf("Powering WiFi interface down...\n");
				if_led(LEDOFF);
				// Set flag for MAC to power down when tcp_tick function is called!
				pd_powerdown(IF_WIFI0);
				tmo_WIFI0 =_SET_SHORT_TIMEOUT(DOWNTIME);
				state_WIFI0 = 2;
			}
			break;
		case 2:	// down, waiting
			if (_CHK_SHORT_TIMEOUT(tmo_WIFI0)) {
				printf("Powering WiFi interface up...\n");
				// Set flag for MAC to power-up when tcp_tick function is called!
				pd_powerup(IF_WIFI0);
				tmo_WIFI0 =_SET_SHORT_TIMEOUT(750);	// settle for 3/4 sec
				state_WIFI0 = 3;
			}
			break;
		case 3: // let power stabilize
			if (_CHK_SHORT_TIMEOUT(tmo_WIFI0)) {
				printf("Bringing WiFi interface up...\n");
				ifup(IF_WIFI0);
				// Allow the power-cycled WiFi interface more time for DHCP set up.
				ifconfig(IF_WIFI0, IFS_DHCP_TIMEOUT, DHCP_CUSTOM_TIMEOUT, IFS_END);
				state_WIFI0 = 4;
			}
			break;
		case 4:	// waiting for up
			ifp_WIFI0 = ifpending(IF_WIFI0);
			if (ifp_WIFI0 == IF_UP)
			{
				printf("\nWiFi interface is up again!\n");
				if_led(LEDON);
				tmo_WIFI0 =_SET_SHORT_TIMEOUT(UPTIME);
				state_WIFI0 = 0;
			}
			else if (ifp_WIFI0 == IF_DOWN) {
				printf("!!!!! WiFi interface failed to come back up!!!!!\n");
				tmo_WIFI0 = _SET_SHORT_TIMEOUT(DOWNTIME);
				state_WIFI0 = 0;
			}
			break;
		}
	}
}
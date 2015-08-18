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
/****************************************************************************

   WiFiMultipleAPs.c

   This code demostrates changing AP's using WEP keys.  It associates the
   module with the first AP (AP_0 defined below) with WEP key KEY0 (defined
   below).  After associating, it waits for a predefined time period, and then
   pings the Ethernet address of the AP (AP_ADDRESS_0).  Next, it associates
   with the second AP and pings its Ethernet address (AP_1, KEY1, AP_ADDRESS_1).
   Then back to the first AP, etc.

   Key points to notice: when changing AP's, first bring the interface
   down by calling ifdown(IF_WIFI0) (assuming the interface is IF_WIFI0).
   Next, change the SSID and key(s) using ifconfig() calls.  Finally,
   bring the interface back up by calling ifup(IF_WIFI0).  Note that the
   code below checks for status while waiting for the interface to come up
   or down.

   To Use:

		1. Configure your network settings--see the function help (Ctrl-H) for
			TCPCONFIG for more information.  You do not need to configure
			IFC_WIFI_SSID (the SSID of your network) for this sample, since that
			is done from the AP names (see step 2 below).
      2. Edit the AP names below (AP_0, AP_1, AP_0_LEN, AP_1_LEN)
      3. Edit KEY0 and KEY1 to match your APs
      4. Edit the addresses to ping, PING_ADDRESS_0 and PING_ADDRESS_1.
         Note that these can be the ethernet-side IP addresses of the APs,
         with at least one of the APs not connected to the network or
         on a private network only reachable through the AP.
      5. Edit the static Rabbit addresses, MY_ADDRESS_0 and MY_ADDRESS_1, to
   		use when pinging the access points.  If the APs are on different
         subnets, you can set an address that's appropriate for each subnet.

	This sample demonstrates a WiFi specific feature.  Additional networking
	samples for WiFi can be found in the Samples\tcpip directory.

****************************************************************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

#define IFC_WIFI_ENCRYPTION	IFPARAM_WIFI_ENCR_WEP

// First Access Point
#define AP_0 "test1"
#define AP_0_LEN strlen(AP_0)
#define MY_ADDRESS_0 "10.10.6.250"		// use this static IP when connected to AP 0
#define PING_ADDRESS_0 "10.10.6.1"		// address on AP 0 to ping

// WEP keys are 5 or 13 bytes, so the HEX string should be 10 or 26 characters
#define KEY_0 "0123456789abcdef0123456789"


// Second Access Point, should be isolated or on a separate network from AP 1
#define AP_1 "test2"
#define AP_1_LEN strlen(AP_1)
#define MY_ADDRESS_1 "10.10.0.99"		// use this static IP when connected to AP 1
#define PING_ADDRESS_1 "10.10.0.50"		// address on AP 1 to ping

// WEP keys are 5 or 13 bytes, so the HEX string should be 10 or 26 characters
#define KEY_1 "0123456789abcdef0123456789"

#define IFC_WIFI_SSID AP_0
#define _PRIMARY_STATIC_IP MY_ADDRESS_0
#use "dcrtcp.lib"

#memmap xmem

/****************************************************************************
	ping

	Try to resolve the address.  If a resolve is successful, print out the
	MAC associated with it.  Print out ping times.

****************************************************************************/
longword sent;
longword received;
longword tot_delays;
longword last_rcvd;
char *name;

void ping_stats(void)
{
   auto longword temp;

   printf("Ping Statistics:\n");
   printf("Sent        : %lu \n", sent );
   printf("Received    : %lu \n", received );
   if (sent) {
      printf("Success     : %lu%%\n", (100L*received)/sent);
	}
   if (!received) {
      printf("There was no response from %s\n", name );
   } else {
      temp =  tot_delays*100L/received;
      printf("Average RTT : %lu.%02lu msec\n", temp / 100L, temp % 100L);
   }
}

debug
void ping(char *pingwho)
{
   longword host, timer, new_rcvd;
   longword tot_timeout, itts, send_timeout;
   word i;
   word sequence_mode, is_new_line;
   word debug_on;
   static unsigned char tempbuffer[255];
   word arping;
   unsigned long t0;

   sent=received=tot_delays=last_rcvd=0L;
   tot_timeout=itts=send_timeout=0L;
   debug_on=0;
   is_new_line=1;
   arping = 1;

   name=pingwho;
   itts=0;		// 0 == continuous

   if (!(host = resolve( name ))) {
      printf("Unable to resolve '%s'\n", name );
      return;
   }
   if ( isaddr( name ))
      printf("Pinging [%s]",inet_ntoa(tempbuffer, host));
   else
      printf("Pinging '%s' [%s]",name, inet_ntoa(tempbuffer, host));

	if (!itts)
		itts = 3L;

   printf(" %lu times\n", itts);

   tot_timeout = _SET_TIMEOUT((itts + 2)*1000L);

	send_timeout = _SET_TIMEOUT(0);
	t0 = MS_TIMER;
   do {
      /* once per second - do all this */
      if ( chk_timeout( send_timeout )) {
         send_timeout = _SET_TIMEOUT(1000L);
         if (chk_timeout( tot_timeout )) {
            ping_stats();
            break;
         }
         if (arping) {
	         if (!_arp_resolve( host, (eth_address *)tempbuffer, 0 ))
	            printf("Could not resolve hardware address (maybe other "\
                        "host not up yet)\n");
	         else {
	         	arping = 0;
	            printf("Hardware address resolved to "\
                        "%02x:%02x:%02x:%02x:%02x:%02x\n",
	               (int)tempbuffer[0],(int)tempbuffer[1],(int)tempbuffer[2],
                  (int)tempbuffer[3], (int)tempbuffer[4],(int)tempbuffer[5]);
	         }
         }
         if (!arping && sent < itts) {
            sent++;
            if (_ping( host , sent ))
               ping_stats();
            if (!is_new_line) printf("\n");
            printf("sent PING # %lu ", sent );
            is_new_line = 0;
         }
      }

      tcp_tick(NULL);
      if (arping)
      	continue;
      if ((timer = _chk_ping( host , &new_rcvd)) != 0xffffffffL) {
         tot_delays += timer;
         ++received;
         if ( new_rcvd != last_rcvd + 1 ) {
            if (!is_new_line) printf("\n");
            printf("PING receipt received out of order!\n");
            is_new_line = 1;
         }
         last_rcvd = new_rcvd;
         if (!is_new_line) printf(", ");
         printf("PING receipt # %lu : response time %lu ms\n", received, timer);
         is_new_line = 1;
         if ( received == itts )
            ping_stats();
      }
   } while (1);
}

/****************************************************************************
	checkkey

	Verify that a keystring is the proper length and contains only hex
   digits (0-9, A-F, a-f).

****************************************************************************/
void checkkey (char *keyhex)
{
	int i;
	char *p;

	for (i = 0, p = keyhex; *p && (i < 27); p++, i++) {
		if (! isxdigit (*p)) {
	      printf("ERROR: Key [%s] has an illegal character\n", keyhex);
	      printf("\t('%c' is not 0-9, A-F or a-f)\n", *p);
	      printf("exiting....\n");
	      exit(0);
      }
   }
	if (i != 26 && i != 10)
	{
		printf("ERROR: Key [%s] has an illegal length\n", keyhex);
      printf("\t(should be 10 or 26 hex digits)\n", i);
		printf("exiting....\n");
		exit(0);
	}
}

/****************************************************************************
	main

	Print out a menu, wait for keypresses, while calling tcp_tick.

****************************************************************************/
void main(void)
{
	auto int i, retval;
   auto wifi_status status;

   printf ("WiFiMultipleAPs Sample\n");
	// Quick sanity check on key sizes
   checkkey (KEY_0);
   checkkey (KEY_1);

	sock_init();

   for(i = 0; i < 3; ++i) {
      // Associate with AP_0, ping PING_ADDRESS_0 , associate AP_1, ping
      // PING_ADDRESS_1
      printf("\nBringing interface down (disassociate)...\n");
      ifdown(IF_WIFI0);
      while (ifpending(IF_WIFI0) != IF_DOWN) {
      	printf(".");
      	tcp_tick(NULL);
      }
	   printf("...Done.\n");

      printf("Setting parameters for associating with AP 0 ('%s')\n", AP_0);
      retval = ifconfig (IF_WIFI0,
      	IFS_IPADDR, aton (MY_ADDRESS_0),
      	IFS_WIFI_WEP_KEY_HEXSTR, 0, KEY_0,
         IFS_WIFI_SSID, AP_0_LEN, AP_0, IFS_END);
      if (retval) {
      	printf("Failed to set key and SSID for AP_0\n");
      	exit(retval);
      }

      printf("Bringing interface back up (associate AP 0)...\n");
      ifup(IF_WIFI0);
      while (ifpending(IF_WIFI0) == IF_COMING_UP) {
      	printf(".");
         tcp_tick(NULL);
      }
      printf("...Done.\n");
      if (ifpending(IF_WIFI0) != IF_UP) {
         printf("Unfortunately, it failed to associate :-(\n");
         return;
      }

      printf("Waiting for status...");
      status.state = 0;
      while (status.state < WLN_ST_ASSOC_ESS)
      {
         tcp_tick(NULL);
         ifconfig(IF_WIFI0, IFG_WIFI_STATUS, &status, IFS_END);
         printf("%d ", status.state);
      }
      printf("\n");
      ping(PING_ADDRESS_0);

		printf("\nBringing interface down (disassociate)...\n");
      ifdown(IF_WIFI0);
      while (ifpending(IF_WIFI0) != IF_DOWN) {
      	printf(".");
      	tcp_tick(NULL);
      }
		printf("...Done.\n");

      printf("Setting parameters for associating with AP 1 ('%s')\n", AP_1);
      retval = ifconfig (IF_WIFI0,
      	IFS_IPADDR, aton (MY_ADDRESS_1),
      	IFS_WIFI_WEP_KEY_HEXSTR, 0, KEY_1,
         IFS_WIFI_SSID, AP_1_LEN, AP_1, IFS_END);
      if (retval) {
      	printf("Failed to set key and SSID for AP_1\n");
      	exit(retval);
      }

		printf("Bringing interface back up (associate)...\n");
      ifup(IF_WIFI0);
      while (ifpending(IF_WIFI0) == IF_COMING_UP) {
      	printf(".");
         tcp_tick(NULL);
      }
		printf("...Done.\n");
		if (ifpending(IF_WIFI0) != IF_UP) {
			printf("Unfortunately, it failed to associate :-(\n");
			return;
		}

      printf("Waiting for status...");
      status.state = 0;
      while (status.state < WLN_ST_ASSOC_ESS) {
         tcp_tick(NULL);
         ifconfig(IF_WIFI0, IFG_WIFI_STATUS, &status, IFS_END);
         printf("%d ", status.state);
      }
      printf("\n");
      ping(PING_ADDRESS_1);


   }
}
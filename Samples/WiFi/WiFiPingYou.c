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
/*******************************************************************************
	wifipingyou.c

	A sample program that will send out a series of 'pings' to another
	computer on the network.

	This is similar to the samples\tcpip\icmp\pingyou.c sample, except
	it uses an ad-hoc Wifi network.

	1.  Change the NODE #define to 1 for the first (or only) core module.

	2.  Compile and run.  You will probably see "error" messages about
	   not being able to resolve the hardware address of the other node.
	   This is normal, since you may not have started the other node.

	3.  If you have another core module, you can run this progam on it
	   too, except change the NODE #define to 2.  Then, each board will
	   ping the other board.

	4.  Otherwise, you can set up a PC or laptop to join the ad-hoc
	   network.  You should be able to ping the first board from the
	   PC, and the PC may respond to pings from the board.  Note that
      default Windows XP behavior is to ignore ping requests.  Firewall
      settings on other computers may also result in no responses to the
      Rabbit's ping requests.

	This sample demonstrates a WiFi specific feature.  Additional networking
	samples for WiFi can be found in the Samples\tcpip directory.

*******************************************************************************/
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
 *
 * Note that we override some of the settings to create an ad-hoc network.
 */
#define TCPCONFIG 1

/*
 * Define to 1 for first (or only) core module, 2 for second.
 * The second node can be another core module, or could be a PC or laptop set
 * up to use the same ad-hoc network parameters (SSID and channel).
 * Even if using two core modules, you can set up a PC as a third node in the
 * network, and ping both the modules.
 */
#define NODE	1			// <-- this setting is specific to just this sample


/*
 * Think up a good name ("SSID") for your network.
 */
#define IFC_WIFI_SSID  "rab-hoc"

/*
 * Think of a good channel which is legal in your country...
 * If you set this to 0, then this node will keep scanning until it finds an
 * existing ad-hoc network with the matching IFC_WIFI_SSID.  In this case, you
 * must set at least one node to a definite channel, otherwise all nodes will
 * search forever, since none will ever "pitch its tent".
 */
#define IFC_WIFI_CHANNEL	5

/*
 * Specify the IP addresses we are going to use.  You can use anything here which
 * does not conflict with an existing network attached to the node.  If using just
 * core modules, you can use just about anything here.
 */
#define IPADDR_1  "10.10.8.1"
#define IPADDR_2  "10.10.8.2"


/********************************
 * End of configuration section *
 ********************************/

// Since this sample is dedicated to ad-hoc WiFi mode, force this setting...
#define IFC_WIFI_MODE	IFPARAM_WIFI_ADHOC

// Don't change this.  This is how we "swap" the addresses so each core module
// will ping the other.

// Ignore any IP address already defined, we're creating our own network as follows.
#ifdef _PRIMARY_STATIC_IP
	#undef _PRIMARY_STATIC_IP
#endif

#if NODE==1
	#define _PRIMARY_STATIC_IP IPADDR_1
	#define PINGWHO				IPADDR_2
#else
	#define _PRIMARY_STATIC_IP IPADDR_2
	#define PINGWHO				IPADDR_1
#endif




#memmap xmem
#use "dcrtcp.lib"

longword sent;
longword received;
longword tot_delays;
longword last_rcvd;
char *name;

void stats(void)
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
   exit( received ? 0 : 1 );
}

debug
void main()
{
   longword host, timer, new_rcvd;
   longword tot_timeout, itts, send_timeout;
   word i;
   word sequence_mode, is_new_line;
   word debug_on;
   static unsigned char tempbuffer[255];
   word arping;
   auto char ip[16], netmask[16];
   auto int status;

   sent=received=tot_delays=last_rcvd=0L;
   tot_timeout=itts=send_timeout=0L;
   debug_on=0;
   is_new_line=1;
   arping = 1;

	// Start network and wait for interface to come up (or error exit).
	printf("Starting/joining ad-hoc network...\n");
	sock_init_or_exit(1);

   name=PINGWHO;
   itts=0;		// 0 == continuous

   if (!(host = resolve( name ))) {
      printf("Unable to resolve '%s'\n", name );
      exit( 3 );
   }
   if ( isaddr( name ))
      printf("Pinging [%s]",inet_ntoa(tempbuffer, host));
   else
      printf("Pinging '%s' [%s]",name, inet_ntoa(tempbuffer, host));

	if (!itts)
		itts = 1000000L;

   printf(" %lu times\n", itts);




   tot_timeout = _SET_TIMEOUT((itts + 2)*1000L);

	send_timeout = _SET_TIMEOUT(0);
   do {
      /* once per second - do all this */
      if ( chk_timeout( send_timeout )) {
         send_timeout = _SET_TIMEOUT(1000L);
         if (chk_timeout( tot_timeout )) {
            stats();
            break;
         }
         if (arping) {
	         if (!_arp_resolve( host, (eth_address *)tempbuffer, 0 ))
	            printf("Could not resolve hardware address (maybe other host not up yet)\n");
	         else {
	         	arping = 0;
	            printf("Hardware address resolved to %02x:%02x:%02x:%02x:%02x:%02x\n",
	               (int)tempbuffer[0],(int)tempbuffer[1],(int)tempbuffer[2],(int)tempbuffer[3],
	               (int)tempbuffer[4],(int)tempbuffer[5]);
	         }
         }
         if (!arping && sent < itts) {
            sent++;
            if (_ping( host , sent ))
               stats();
            if (!is_new_line) printf("\n");
            printf("sent PING # %lu ", sent );
            is_new_line = 0;
         }
      }

      if ( kbhit() ) {
         getchar();    /* trash the character */
         stats();
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
            stats();
      }
   } while (1);
}
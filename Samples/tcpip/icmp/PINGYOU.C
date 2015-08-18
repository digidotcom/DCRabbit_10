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
        pingyou.c

        A sample program that will send out a series of 'pings' to another
        computer on the network.
*******************************************************************************/
#class auto


/* Portions used with permission of Erick Engelke. */

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
 * PING configuration
 *
 * This is the address of the computer
 * that we should send echo-requests to.
 */
#define PINGWHO	"10.10.6.176"


/********************************
 * End of configuration section *
 ********************************/


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

   sent=received=tot_delays=last_rcvd=0L;
   tot_timeout=itts=send_timeout=0L;
   sequence_mode=debug_on=0;
   is_new_line=1;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

   name=PINGWHO;
   itts=10;

   if (!(host = resolve( name ))) {
      printf("Unable to resolve '%s'\n", name );
      exit( 3 );
   }
   if ( isaddr( name ))
      printf("Pinging [%s]",inet_ntoa(tempbuffer, host));
   else
      printf("Pinging '%s' [%s]",name, inet_ntoa(tempbuffer, host));

   if (itts) printf(" %u times", itts);
   else
      itts = sequence_mode ? 0xffffffffL : 1;

   if (sequence_mode) printf(" once per_second");
   printf("\n");


   if (!_arp_resolve( host, (eth_address *)tempbuffer, 0 )) {  /* resolve it before timer starts */
   	printf("Could not resolve hardware address\n");
   	exit(2);
   }
   tot_timeout = _SET_TIMEOUT((itts + 2)*1000L);
   if ( debug_on ) printf("ETH -> %02x:%02x:%02x:%02x:%02x:%02x\n",
      (int)tempbuffer[0],(int)tempbuffer[1],(int)tempbuffer[2],(int)tempbuffer[3],
      (int)tempbuffer[4],(int)tempbuffer[5]);

	send_timeout = _SET_TIMEOUT(0);
   do {
      /* once per second - do all this */
      if ( chk_timeout( send_timeout )) {
         send_timeout = _SET_TIMEOUT(1000L);
         if (chk_timeout( tot_timeout )) {
            stats();
            break;
         }
         if ( sent < itts ) {
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
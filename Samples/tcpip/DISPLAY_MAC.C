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
		Samples\TCPIP\display_mac.c

		This program prints out the MAC address(es) for your board.
*******************************************************************************/

#class auto
#memmap xmem

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
 * Grabs IP address out of "TCP_CONFIG.LIB" so you can ping the board.
 */
#define TCPCONFIG 1


#use "dcrtcp.lib"

///////////////////////////////////////////////////////////////////////

void main()
{
	char buffer[6];
   word iface;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

   for (iface = 0; iface < IF_MAX; ++iface)
   	if (is_valid_iface(iface)) {
      	printf("Interface %u is valid.\n");
         if (!ifconfig(iface, IFG_HWA, buffer, IFS_END))
         	printf("  Hardware address is %02x:%02x:%02x:%02x:%02x:%02x\n",
	            buffer[0],buffer[1],buffer[2],
	            buffer[3],buffer[4],buffer[5]);
         else
         	printf("  No hardware address for this interface.\n");

      }

	printf("\nCalling ip_print_ifs()...\n\n");
   ip_print_ifs();

	for(;;) {
		tcp_tick(NULL);
	}
}
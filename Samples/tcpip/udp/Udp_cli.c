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
        Samples\tcpip\udp\udp_cli.c

        A UDP example, that sends out 'heartbeat' packets to either a specific
        receiver, or broadcasted over the local subnet. These packets can be
        received by udp_srv.c

*******************************************************************************/
#class auto

/*******************************************************
The samples UDP_SRV.c (server) and UDP_CLI.c (client) were
designed to be used together. You must have two core modules,
each one running one of the before mentioned programs.
When correctly configured and connected the module running the
UDP_CLI sample will send packets to the module running the UDB_SRV sample.
These packets will be sent using the UDP protocol.

To correctly set up the test environment follow the instructions bellow.

-	Connect both modules to a hub or a switch.
   If you don't have either you can interconnect the 2 modules using a
   crossover cable.
-	Open the tcp_config.lib and look for the definition of
   _PRIMARY_STATIC_IP. If you did not modify it before it should be set to
   10.10.6.100. For the purpose of this test, the server will have the
   address 10.10.6.177. So go ahead and change 10.10.6.100 to 10.10.6.177
   and save the file.
-	Connect the programming cable to the server core module. Compile and
   run the UDP_SRV sample.
-	If you are using a single computer, to be able to see the messages
   printed on the server side you must have a separate serial cable
   connected to the server module serial port A. Then you can use
   HyperTerminal to see the messages.
-	At this point, the server is just waiting for the messages from the
   client. For every message received it will print a string in the STDIO.
-	Now let's configure the client.
-	Edit the tcp_config.lib again and change the _PRIMARY_STATIC_IP back
   to 10.10.6.100. Connect the programming cable to the client module and
   compile and run the UDP_CLI sample.
-	When it is running, the client sample will send UDP packets to the
   server with the IP address specified by REMOTE_IP.
-	On the client side STDIO window you will see a message every second
   saying that a packet has been sent ... Transmitted -> ...
-	On the server side you will see a message printed every second
   acknowledging that the message has been received ... Received-> ...
********************************************************/


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
 * Define the number of socket buffers that will be allocated for
 * UDP sockets.  We only need one UDP socket, so one socket buffer
 * will be enough.
 */
#define MAX_UDP_SOCKET_BUFFERS 1

/*
 * UDP demo configuration
 */

/* How often (in seconds) to send out UDP packet. */
#define  HEARTBEAT_RATE 	4

/* what local UDP port to use - packets we send come from here */
#define LOCAL_PORT	1234

/*
 * Where to send the heartbeats. If it is set to all '255's,
 * it will be a broadcast packet.
 */
#define REMOTE_IP			"10.10.6.177"
//#define REMOTE_IP			"255.255.255.255" /*broadcast*/

/* the destination port to send to */
#define REMOTE_PORT	1234

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

udp_Socket sock;

/* send one packet (heartbeat) */
int send_packet(void)
{
	static long sequence;
	auto char 	buf[128];
	auto int 	length, retval;

	#GLOBAL_INIT
	{
		sequence = 0;
	}

	/* fill the packet with interesting data (a sequence number) */
	sequence++;
	sprintf(buf, "SEQ=%ld",sequence);
	length = strlen(buf) + 1;

	/* send the packet */
	retval = udp_send(&sock, buf, length);
	if (retval < 0) {
		printf("Error sending datagram!  Closing and reopening socket...\n");
		if (sequence == 1) {
			printf("   (initial ARP request may not have finished)\n");
		}
		sock_close(&sock);
		if(!udp_open(&sock, LOCAL_PORT, resolve(REMOTE_IP), REMOTE_PORT, NULL)) {
			printf("udp_open failed!\n");
			exit(0);
		}
	}
   else {
   		printf("Transmitted -> %s\n",buf);
   }

	tcp_tick(NULL);
	return 1;
}

void main()
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	/*printf("Opening UDP socket\n");*/
   if(!udp_open(&sock, LOCAL_PORT, resolve(REMOTE_IP), REMOTE_PORT, NULL)) {
		printf("udp_open failed!\n");
		exit(0);
	}

	/* send heartbeats */
	for(;;) {
		//putchar('.');
		tcp_tick(NULL);
		costate {
			waitfor(IntervalSec(HEARTBEAT_RATE));
			waitfor(send_packet());
		}
	}
}


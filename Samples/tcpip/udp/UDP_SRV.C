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
        Samples\tcpip\UDP\udp_srv.c

        A UDP example, that receives the 'heartbeat' packets sent from
        udp_cli.c, and will receive either the direct-send packets or
        broadcast packets, depending on the configuration.

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

/* what local UDP port to use - we receive packets only sent to this port */
#define LOCAL_PORT	1234

/*
 * If REMOTE_IP is set to -1, we will accept packets from anybody.
 *
 * If it is set to 0, we will accept packets from anybody, but
 * the first host to connect to us will complete the socket with
 * their IP address and port number.  At that point, the local socket
 * will be limited to that host only.
 *
 * If it is set to an IP address, the socket will only accept packets
 * from that IP.
 *
 */
#define REMOTE_IP		-1			// accept packets from all hosts
//#define REMOTE_IP		0			// accept packets from first host
//#define REMOTE_IP		IPADDR(10,10,6,100)	// accept from 10.10.6.100 only

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

udp_Socket sock;

/* receive one packet (heartbeat) */
int receive_packet(void)
{
	static char buf[128];

	#GLOBAL_INIT
	{
		memset(buf, 0, sizeof(buf));
	}

	/* receive the packet */
	if (-1 == udp_recv(&sock, buf, sizeof(buf))) {
		/* no packet read. return */
		return 0;
	}

	printf("Received-> %s\n",buf);
	return 1;
}

void main()
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	if(!udp_open(&sock, LOCAL_PORT, REMOTE_IP, 0, NULL)) {
		printf("udp_open failed!\n");
		exit(0);
	}

	/* receive heartbeats */
	for(;;) {
		tcp_tick(NULL);
		receive_packet();
	}
}
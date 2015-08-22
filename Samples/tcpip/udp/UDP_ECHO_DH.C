/*******************************************************************************
        udp_echo_dh.c
        Rabbit Semiconductor, 2002

        A UDP example, that receives packets from any host on UDP port 7, then
        echoes the received packet back to the sender.

        This demonstrates use of a UDP data handler, which is the fastest
        "official" way to implement UDP-based request/response servers.

        This sample will work for Dynamic C 7.30 and later.
*******************************************************************************/
#class auto 			// Change default storage class for local variables: on the stack

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
#define TCPCONFIG		1

//#define DCRTCP_VERBOSE
//#define DCRTCP_DEBUG

/*
 * Define the number of socket buffers that will be allocated for
 * UDP sockets.  We only need one UDP socket, so one socket buffer
 * will be enough.
 */
#define MAX_UDP_SOCKET_BUFFERS 1


/*
 * UDP demo configuration
 */
#define DISABLE_TCP		// Not using TCP

/*
 *  what local UDP port to use - we receive packets only sent to this port.
 *  Port 7 is the standard echo port.
 */
#define LOCAL_PORT	7


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

udp_Socket sock;

char pktbuf[1500];	// Temp root buffer for packet reassembly

/**
 * 	Handler called from the UDP stack.  If message is relevant, then send
 * 	back a reply.  Return 1 tells UDP stack not to copy this into a
 * 	client buffer; it has already been taken care of-thank you.
 *
 * 	RETURN: 	1 = handler has done all relevant processing for datagram.
 */
int echo_handler(int event, udp_Socket * s, ll_Gather * g,
						_udp_datagram_info * udi)
{
	// Datagram has come in.  It is in the Ethernet receive buffer.  No need to
	// copy it anywhere - we just transmit it straight back to the sender.
	// The relevant information comes in the following fields (not all of which
	// we use here) (see LIB\tcpip\net.lib for structure):
	//  g->data1  ->  IP and UDP headers (root)
	//  g->len1   ->  IP and UDP header length
	//  g->data2  ->  UDP datagram data (far) - first buffer
	//  g->len2   ->  UDP datagram data length - first buffer
	//  g->data3  ->  UDP datagram data (far) - second buffer **
	//  g->len3   ->  UDP datagram data length - second buffer
	//  udi->remip  -> sender's IP address
	//  udi->remport  ->  sender's UDP port number
	//  udi->flags  -> flags.

   // ** Note: prior to Dynamic C 9.0, only one buffer would be provided (i.e. g->data3 would
   // always be zero).  From DC9.0, it is now possible for the incoming data to be split into
   // two areas.

	// The 'event' parameter determines the type of event.  As of DC 7.30, this is either
	//  UDP_DH_INDATA : incoming datagram
	//  UDP_DH_ICMPMSG : incoming ICMP message.

	if (event == UDP_DH_ICMPMSG) {
		return 1;	// Just ignore incoming ICMP errors.
	}

	// Otherwise, bounce the packet back!  We ignore errors because this
	// is UDP, and the sender will try again.  Not a good idea in general, though.

   printf("Got UDP  len1=%2u len2=%4u len3=%4u remip=%08lX remport=%5u len=%u\n",
   	g->len1, g->len2, g->len3, udi->remip, udi->remport, udi->len);

	if (!g->len3)
   	// No second buffer.  This is easy - just use udp_sendto directly
		udp_sendto(s, g->data2, g->len2, udi->remip, udi->remport);
   else {
   	// Awkward: got 2 areas, so copy them into a contiguous root buffer and send.
		_f_memcpy(pktbuf, g->data2, g->len2);
      _f_memcpy(pktbuf + g->len2, g->data3, g->len3);
		udp_sendto(s, pktbuf, g->len2+g->len3, udi->remip, udi->remport);
   }

	// Return 1 to indicate that all processing has been done.  No copy to
	// normal udp socket receive buffer.
	return 1;
}



void main()
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	if(!udp_extopen(&sock, IF_ANY, LOCAL_PORT, -1L, 0, echo_handler, 0, 0)) {
		printf("udp_extopen failed!\n");
		exit(0);
	}

	/* Let the stack do everything... */
	for(;;)
		tcp_tick(NULL);
}


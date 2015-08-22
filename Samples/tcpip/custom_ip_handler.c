/**********************************************************************
 *		Samples\TCPIP\custom_ip_handler.c
 *		Copyright (c) 2010, Digi International
 *    This code demonstrates the CUSTOM_IP_HANDLER macro.
 *
 *    We'll be using CUSTOM_IP_HANDLER to preprocess IP packets before
 *    the TCP stack sees them.  We'll parse through the packet to read
 *    the port.  If the port matches one of our custom ports, and the
 *    socket is not currently in use (!sock_alive(&socket)), we'll set
 *    up a tcp_listen() call to accept the connection.
 *
 *    For more information on CUSTOM_IP_HANDLER, read the comment in
 *    IP.lib.
 *
 *    Most of the code in main() was leveraged from echo.c, a basic
 *    server, that when a client connect, echoes back to them any data
 *    that they send.
 *
 **********************************************************************/

#class auto
#memmap xmem
#define TCPCONFIG 1


#define CUSTOM_IP_HANDLER my_ports
#use "dcrtcp.lib"

tcp_Socket socket;

// The following code was leveraged from ip_handler() and tcp_handler()
far ll_prefix *my_ports(far ll_prefix *LL, byte *hdrbuf, word *pflags) {
   int myport;       // The port where we received this message
   word iplen;
   in_Header *ip;
   tcp_Header *tp;

   // We will always be processing the frame with the TCP/IP stack
   *pflags |= CUSTOM_PKT_FLAG_PROCESS;

   // Check whether the socket is in use
   if (! sock_alive(&socket) ) {
      // Socket is free, see if this is a packet bound for one of the ports
      // we're willing to receive on.

      // First, parse the packet (code leveraged from ip_handler(),
      // tcp_handler().)
      _pkt_buf2root(LL, ip = (in_Header *)(hdrbuf+LL->net_offs),
            sizeof(in_Header), LL->net_offs);
      iplen = in_GetHdrlenBytes(ip);
      if (ip->proto == TCP_PROTO) {
         // We have a valid TCP packet, check myport
         _pkt_buf2root(LL, tp = (tcp_Header *)(hdrbuf+LL->tport_offs),
               sizeof(tcp_Header), LL->tport_offs);

         // We received the packet on the packet's destination port
         myport = intel16(tp->dstPort);

         switch(myport) {
            // List of ports which we're willing to receive.  We could
            // also search through a list, use a range (if (myport > 23 &&
            // myport < 30)), etc.
            case 7:  // Echo protocol
            case 23: // Telnet protocol
            case 80: // HTTP protocol
               // Accept the connection by calling listen for the incoming
               // port
               tcp_listen(&socket,myport,0,0,NULL,0);
               break;
            default:
               // Do nothing -- don't accept the connection
         }
      }
   }
   return LL;
}

void main()
{
	int bytes_read;
	char	buffer[100];   // Output buffer for printing

	sock_init_or_exit(1);
   // "Initialize" socket (we want sock_alive() to return 0, so set the
   // current socket state to all 0's.
   memset(&socket, 0, sizeof(socket));

	while(1) {

		printf("Waiting for connection...\n");
		while(!sock_established(&socket) && sock_bytesready(&socket)==-1)
			tcp_tick(NULL);

      // We could invoke a separate function for each of the protocols
      // (Telnet, HTTP, etc.), but instead we'll just print out the port
      // and echo the incoming packets.
		printf("Connection received on port %d...\n", socket.myport);

		do {
			bytes_read=sock_fastread(&socket,buffer,sizeof(buffer)-1);

         // Echo the packet to stdio and back to the sender.
			if(bytes_read>0) {
				buffer[bytes_read]=0;
				printf("%s",buffer);
				sock_write(&socket,buffer,bytes_read);
			}
		} while(tcp_tick(&socket));

		printf("Connection closed...\n");
	}
}
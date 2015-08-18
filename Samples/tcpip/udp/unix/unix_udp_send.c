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
/***************************************************************************
	unix_udp_send.c
   
 	This is the source code for "unix_udp_send" used on the PC console
 	side to communicate with a UDP host that echos messages.
 	
	The executable "unix_tcp_send" is similar to tcp_send.c but is run
	at the command prompt to communicate a Rabbit board running the
	demo "Samples\tcpip\udp\udp_echo_dh.c"
	
   To compile this file in Borland C++ 5.0, load this file and
   select Project/Build All from the menu.  The executable will
   be generated in the same directory as this file.

	Using "unix_udp_send":
	-----------------------

	unixsend runs from the shell.  The command line is as follows:

	% unix_udp_send <ipaddr> <port>

	where <ipaddr> is the IP address to send the message to,
	and <port> is the port number.  The arguments are optional--
	if they are omitted, the IP address defaults to 10.10.6.112,
	and the port defaults to 7 (INANA echo port).  A message will
	display on the remote machine, and then the response will be
	displayed on the console.  "unix_udp_send" exits when it is
	finished sending a message.

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/**
 *   Where to find the Rabbit.
 */
#define DEST_IPADDR "10.10.6.177"
#define DEST_PORT 7


/*  Things to smell like MS-Windows: */
typedef int   SOCKET;
#define  closesocket(s)   close(s)

/////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	int err;
   SOCKET       sock;
   struct sockaddr_in  sin;
   const char message[] = "How are you today?";
   char data[1024];
   char ip_addr[20];
   unsigned short int port;

   if (argc > 1) {
   	strncpy(ip_addr, argv[1], 20);
   } else {
   	strncpy(ip_addr, DEST_IPADDR, 20);
   }

   if (argc > 2) {
   	port = (unsigned short int)atoi(argv[2]);
   } else {
   	port = DEST_PORT;
   }

   sock = socket(PF_INET, SOCK_DGRAM, 0);
   if (sock < 0 ) {
   	printf("socket() failed!\n");
   	return 2;
   }

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = inet_addr(ip_addr);
   sin.sin_port = htons(port);

   err = connect(sock, (struct sockaddr *) &sin, sizeof(sin));
   if (err < 0 ) {
   	perror("connect() failed!");
   	return 2;
   }


   /*
    *  Note, UDP does not give an acknowlegement of delivery.
    *  We inject the message and hope it arrives.  If the destination
    *  is on the same network, it will arrive 99% of the time....
    */
   err = send(sock, message, strlen(message), 0);
   if (err < 0 ) {
   	perror("send() failed!");
      return 2;
   }

   err = recv(sock, data, sizeof(data), 0);
   if (err < 0 ) {
   	perror("recv() failed!");
      return 1;
   }
   data[err] = '\0';
   printf("message -> \"%s\"\n", data);


   err = closesocket(sock);
   if (err < 0 ) {
   	perror("closesocket() failed!");
      return 1;
   }

   return 0;
}   /* end main() */


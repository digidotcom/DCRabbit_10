/***************************************************************************
	pcsend.c
   Z-World, 2000
   
 	This is the source code for pcsend used on the UNIX
 	side to communicate with an Intellicom Series board.
 	
	The executable pcsend is similar to tcp_send.c but is run at the
	command prompt to communicate with an Intellicom Series board running
	tcp_respond.c
	
	To compile this with GCC do normal things.  The executable will
	be generated in the same directory as this file.
		gcc -Wall pcrespond.c -o pcrespond

	Using pcsend:
	-------------

	pcsend runs from the console.  The command line is as
	follows:

	UNIX> pcsend [ <ipaddr> [ <port> ] ]

	where <ipaddr> is the IP address to send the message to,
	and <port> is the port number.  The arguments are optional--
	if they are omitted, the IP address defaults to 10.10.6.112,
	and the port defaults to 4040.

	A message will display on the remote machine, and then the
	response will be displayed on the console.  pcsend exits when
	it is finished receiving a message (the response).

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>			/* For sleep() */
#include <netinet/in.h> 		/* For htons() */
#include <arpa/inet.h>			/* For inet_addr() */	

#define PORT	4040
#define IPADDR "10.10.6.177"


/* --------------------------------------------------------------- */


int main(int argc, char* argv[])
{
   int	err;
   int 	sock;			/* Socket file descriptor */
   struct sockaddr_in sin;
   char data[1024];
   static char message[] = { "How are you today?" };
   unsigned short int port;
   char ip_addr[20];

   if (argc > 1) {
   	strncpy(ip_addr, argv[1], sizeof(ip_addr)-1);
   } else {
   	strncpy(ip_addr, IPADDR, sizeof(ip_addr)-1);
   }

   if (argc > 2) {
   	port = (unsigned short int)atoi(argv[2]);
   } else {
   	port = PORT;
   }

   sock = socket(PF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
   	printf("socket() failed!\n");
   	return 1;
   }

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = inet_addr(ip_addr);
   sin.sin_port = htons(port);
   err = connect(sock, (struct sockaddr *)&sin, sizeof(sin));
   if (err < 0) {
   	printf("connect() failed!\n");
   	return 1;
   }

   err = send(sock, message, strlen(message), 0);
   if (err < 0) {
   	printf("send() failed!\n");
      return 1;
   }

   /*  Wait for repsonse back from unit.. */
   err = recv(sock, data, 1024, 0);
   if (err < 0) {
   	printf("recv() failed!\n");
      return 1;
   }
   data[err] = '\0';
   printf("REPLY: %s\n", data);

   err = close(sock);
   if (err < 0) {
   	printf("close() on socket failed!\n");
      return 1;
   }

   return 0;
}


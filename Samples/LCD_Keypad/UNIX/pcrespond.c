/********************************************************************
	pcrespond.c
	Z-World, 2000

 	This is the source code for pcrespond used on the UNIX
 	side to communicate with an Intellicom Series board.
 	
	The executable pcrespond is similar to tcp_respond.c but is run at the
	command prompt to communicate with an Intellicom Series board running
	tcp_send.c
	
	The executable pcrespond is run at the command prompt
	to communicate with an Intellicom Series board running tcp_send.c
	
	To compile this with GCC do normal things.  The executable will
	be generated in the same directory as this file.
		gcc -Wall pcrespond.c -o pcrespond

	Using pcrespond:
	----------------

	pcrespond runs from the console.  The command line is as
	follows:

	UNIX> pcrespond <port>

	where <port> is the port to listen on for messages.  The
	argument is optional--if omitted, the port defaults to 4040.

	Any message received will be displayed on the console, and a
	response will be send to the remote machine.  pcrespond does
	not exit unless the user types a <ctrl>-c to stop the
	program.

***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>			/* For sleep() */
#include <netinet/in.h> 		/* For htons() */


#define PORT 4040

/* --------------------------------------------------------------- */


int main(int argc, char* argv[])
{
   int	err;
   int 	ssock, csock;		/* Socket file descriptors */
   struct sockaddr_in sin;
   char data[1024];
   static char response[] = { "I hear ya" };
   unsigned short int port;

   if (argc > 1) {
   	port = (unsigned short int)atoi(argv[1]);
   } else {
   	port = PORT;
   }


   ssock = socket(PF_INET, SOCK_STREAM, 0);
   if (ssock < 0) {
   	printf("socket() failed!\n");
   	return 1;
   }

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = 0;
   sin.sin_port = htons(port);
   err = bind(ssock, (struct sockaddr *)&sin, sizeof(sin));
   if (err < 0) {
   	printf("bind() failed!\n");
   	return 1;
   }

   while (1) {
   	err = listen(ssock, 1);
      if (err < 0) {
      	printf("listen() failed!\n");
      	return 1;
      }

	csock = accept(ssock, NULL, NULL);
      if (csock < 0) {
      	printf("accept() failed!\n");
      	return 1;
      }

      err = recv(csock, data, 1024, 0);
      if (err < 0) {
      	printf("recv() failed!\n");
         return 1;
      }

      data[err] = '\0';
      printf("%s\n", data);
      sleep(5);			// Sleep 5 seconds

		err = send(csock, response, strlen(response), 0);
      if (err < 0) {
      	printf("send() failed!\n");
         return 1;
      }

      err = close(csock);
      if (err < 0) {
      	printf("close() on socket failed!\n");
         return 1;
      }
   }

}

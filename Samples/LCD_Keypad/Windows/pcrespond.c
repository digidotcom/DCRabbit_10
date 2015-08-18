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
/********************************************************************
	pcrespond.c

 	This is the source code for pcrespond.exe used on the PC console
 	side to communicate with an Intellicom Series board.
 	
	The executable pcrespond.exe is similar to tcp_respond.c but is run at the
	command prompt to communicate	with an Intellicom Series board running
	tcp_send.c
	
	The excutable pcrespond.exe is run at the command prompt
	to communicate with an Intellicom Series board running tcp_send.c
	
	To compile this file in Borland C++ 5.0, load this file and
   select Project/Build All from the menu.  The executable will
   be generated in the same directory as this file.

	Using pcrespond:
	----------------

	pcrespond runs from the console.  The command line is as
	follows:

	>pcrespond <port>

	where <port> is the port to listen on for messages.  The
	argument is optional--if omitted, the port defaults to 4040.
	Any message received will be displayed on the console, and a
	response will be send to the remote machine.  pcrespond does
	not exit unless the user types a <ctrl>-c to stop the
	program.

***************************************************************************/

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include <winbase.h>
#include <stdlib.h>

#define PORT 4040

int main(int argc, char* argv[]) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
   SOCKET ssock, csock;
   SOCKADDR_IN sin;
   char data[1024];
   char *response = "I hear ya";
   unsigned short int port;

   if (argc > 1) {
   	port = (unsigned short int)atoi(argv[1]);
   } else {
   	port = PORT;
   }

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
   	printf("WSAStartup() failed!\n");
		return 1;
   }

   ssock = socket(PF_INET, SOCK_STREAM, 0);
   if (ssock == INVALID_SOCKET) {
   	printf("socket() failed!\n");
   	return 1;
   }

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = 0;
   sin.sin_port = htons(port);
   err = bind(ssock, (LPSOCKADDR)&sin, sizeof(sin));
   if (err == SOCKET_ERROR) {
   	printf("bind() failed!\n");
   	return 1;
   }

   while (1) {
   	err = listen(ssock, 1);
      if (err == SOCKET_ERROR) {
      	printf("listen() failed!\n");
      	return 1;
      }

		csock = accept(ssock, NULL, NULL);
      if (csock == INVALID_SOCKET) {
      	printf("accept() failed!\n");
      	return 1;
      }

      err = recv(csock, data, 1024, 0);
      if (err == SOCKET_ERROR) {
      	printf("recv() failed!\n");
         return 1;
      }

      data[err] = '\0';
      printf("%s\n", data);
      Sleep(5000);			// Sleep 5 seconds

		err = send(csock, response, strlen(response), 0);
      if (err == SOCKET_ERROR) {
      	printf("send() failed!\n");
         return 1;
      }

      err = closesocket(csock);
      if (err == SOCKET_ERROR) {
      	printf("closesocket() failed!\n");
         return 1;
      }
   }
}

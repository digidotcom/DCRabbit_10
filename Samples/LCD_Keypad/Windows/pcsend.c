/***************************************************************************
	pcsend.c
   Z-World, 2000
   
 	This is the source code for pcsend.exe used on the PC console
 	side to communicate with an Intellicom Series board.
 	
	The executable pcsend.exe is similar to tcp_send.c but is run at the
	command prompt to communicate	with an Intellicom Series board running
	tcp_respond.c
	
   To compile this file in Borland C++ 5.0, load this file and
   select Project/Build All from the menu.  The executable will
   be generated in the same directory as this file.

	Using pcsend:
	-------------

	pcsend runs from the console.  The command line is as
	follows:

	>pcsend <ipaddr> <port>

	where <ipaddr> is the IP address to send the message to,
	and <port> is the port number.  The arguments are optional--
	if they are omitted, the IP address defaults to 10.10.6.112,
	and the port defaults to 4040.  A message will display on
	the remote machine, and then the response will be displayed
	on the console.  pcsend exits when it is finished sending a
	message.

***************************************************************************/

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IPADDR "10.10.6.112"
#define PORT 4040

int main(int argc, char* argv[]) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
   SOCKET sock;
   SOCKADDR_IN sin;
   char data[1024];
   char* message = "How are you today?";
   char ip_addr[20];
   unsigned short int port;

   if (argc > 1) {
   	strncpy(ip_addr, argv[1], 20);
   } else {
   	strncpy(ip_addr, IPADDR, 20);
   }

   if (argc > 2) {
   	port = (unsigned short int)atoi(argv[2]);
   } else {
   	port = PORT;
   }

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
   	printf("WSAStartup() failed!\n");
		return 1;
   }

   sock = socket(PF_INET, SOCK_STREAM, 0);
   if (sock == INVALID_SOCKET) {
   	printf("socket() failed!\n");
   	return 1;
   }

   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = inet_addr(ip_addr);
   sin.sin_port = htons(port);
   err = connect(sock, (LPSOCKADDR)&sin, sizeof(sin));
   if (err == SOCKET_ERROR) {
   	printf("connect() failed!\n");
   	return 1;
   }

   err = send(sock, message, strlen(message), 0);
   if (err == SOCKET_ERROR) {
   	printf("send() failed!\n");
      return 1;
   }

   err = recv(sock, data, 1024, 0);
   if (err == SOCKET_ERROR) {
   	printf("recv() failed!\n");
      return 1;
   }
   data[err] = '\0';
   printf("%s\n", data);

   err = closesocket(sock);
   if (err == SOCKET_ERROR) {
   	printf("closesocket() failed!\n");
      return 1;
   }

   WSACleanup();
   return 0;
}


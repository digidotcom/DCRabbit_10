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
/**********************************************************************
 *		Samples/TCPIP/echo.c
 *		This program demonstrates the tcp_listen call.
 *
 * 	A basic server, that when a client connect, echoes back to them
 * 	any data that they send.
 *
 **********************************************************************/
#class auto


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

#use "dcrtcp.lib"

/**
 *  Port Number    Purpose
 *  -----------    -------
 *      13         Daytime
 *      23         Telnet
 * 					Note: this program doesn't handle telnet option negotiation.
 *      80         HTTP
 */
#define PORT 23


/********************************
 * End of configuration section *
 ********************************/

///////////////////////////////////////////////////////////////////////

void main()
{
	int bytes_read;
	/*
		Unless STDIO_ENABLE_LONG_STRINGS is defined, printf() has max 127 bytes
		it can output.  For this sample, we'll read in a maximum of 100 bytes
		at a time.
	*/
	char	buffer[100];
	tcp_Socket socket;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	while(1) {
		tcp_listen(&socket,PORT,0,0,NULL,0);

		printf("Waiting for connection...\n");
		while(!sock_established(&socket) && sock_bytesready(&socket)==-1)
			tcp_tick(NULL);

		printf("Connection received...\n");

		do {
			bytes_read=sock_fastread(&socket,buffer,sizeof(buffer)-1);

			if(bytes_read>0) {
				buffer[bytes_read]=0;
				printf("%s",buffer);
				sock_write(&socket,buffer,bytes_read);
			}
		} while(tcp_tick(&socket));

		printf("Connection closed...\n");
	}
}
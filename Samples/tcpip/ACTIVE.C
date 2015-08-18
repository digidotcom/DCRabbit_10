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
 *		Samples\TcpIp\active.c
 *		This program demonstrates the tcp_open call.
 *
 * 	A simple demonstration of a TCP/IP session, by retrieving a
 * 	web page from a remote site.  Change DEST and PORT to the
 * 	remote machine.
 *
 *		If you're interested in a more complete HTTP client, take a
 *		look at Samples/tcpip/http/http client.c (which uses http_client.lib).
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

/*
 *  Remote computer to contact:
 */
#define  DEST 		"www.digi.com"
#define  PORT 		80

////////////////////////////////////////////////////////////////////////

void main()
{
	/*
		Unless STDIO_ENABLE_LONG_STRINGS is defined, printf() has max 127 bytes
		it can output.  For this sample, we'll read in a maximum of 100 bytes
		at a time.
	*/
	char	buffer[100];
	int 	bytes_read;
	longword  destIP;
	tcp_Socket socket;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	if( 0L == (destIP = resolve(DEST)) ) {
		printf( "ERROR: Cannot resolve \"%s\" into an IP address\n", DEST );
		exit(2);
	}
	tcp_open(&socket,0,destIP,PORT,NULL);

	printf("Waiting for connection...\n");
	while(!sock_established(&socket) && sock_bytesready(&socket)==-1) {
		tcp_tick(NULL);
	}

	printf("Connection established, sending get request...\n");

	/*
	 *  If don't send the HTTP version number, then server believes we are
	 *  a pre-1.0 version client.
	 */
	sock_write(&socket,"GET /\r\n\r\n",9);

	/*
	 *  When tcp_tick()-ing on a specific socket, we get non-zero return while
	 *  it is active, and zero when it is closed (as used here).
	 */
	do {
		bytes_read=sock_fastread(&socket,buffer,sizeof(buffer)-1);

		if(bytes_read>0) {
			buffer[bytes_read] = '\0';
			/*
			 * By using the "%s" format, if there are "%" in the buffer, printf()
			 *  won't try to interpret them!
			 */
			printf("%s",buffer);
		}
	} while(tcp_tick(&socket));

	sock_abort(&socket);
	printf("\nConnection closed...\n");
}
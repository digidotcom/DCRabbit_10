/*******************************************************************************
        bsd.c
        Digi International, Copyright (c) 2000.  All rights reserved.

        A demonstration of several advanced TCP/IP functions, that are used
        for namespace (DNS) issues, and finding out information about the
        remote host.
*******************************************************************************/

/*********************************************************
 * Example of the use of various naming functions
 *
 * To use:
 *     Set the IP address / netmask above.
 *     Set the domainname / hostname above.
 *     Run, and when it prints your domain name,
 *       telnet to it from another host
 *     You should see information about the connection
 *       be printed out.
 **********************************************************/
#class auto


/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1


/*
 * 	Place to listen for incoming request.  23 is TELNET port.
 */
#define  MY_TEST_PORT	23

/*
 * Domain name configuration
 *
 * This sets your hostname and domainname. If your computer has an
 * internet address like: "test.digi.com", your hostname
 * is "test", and your domainname is "digi.com".
 */
#define MY_HOST 	"test"
#define MY_DOMAIN	"digi.com"

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

tcp_Socket tcpsock;
tcp_Socket *const s = &tcpsock;

void main()
{
	char ipbuf[16];
	int status;
	struct sockaddr sock_addr;

	sock_init_or_exit(1);

	sethostname(MY_HOST);
	setdomainname(MY_DOMAIN);

	printf("We are %s.%s\n", gethostname(NULL, 0), getdomainname(NULL, 0));

	printf("Telnet to port %u.\n", MY_TEST_PORT);

	tcp_listen((tcp_Socket*)s,MY_TEST_PORT,0,0,NULL,0);
	while( ! tcp_established( (tcp_Socket *)s ) ) {
		if( ! tcp_tick( (sock_type *) s ) ) {
			printf( "ERROR: listening\n" );
			exit( 99 );
		}
	}

	printf("Receiving incoming connection\npsocket(s) -> ");
	psocket((sock_type *)s);

	printf("\n\n");
	if(0 == getpeername((sock_type *)s, &sock_addr, NULL)) {
		printf("getpeername: %s:%u",
			inet_ntoa(ipbuf, sock_addr.s_ip),
			sock_addr.s_port
			);
	} else {
		printf("getpeername() retuned no data!\n");
	}

	printf("\n");

	if(0 == getsockname((sock_type *)s, &sock_addr, NULL)) {
		printf("getmyname: %s:%u",
			inet_ntoa(ipbuf, sock_addr.s_ip),
			sock_addr.s_port
			);
	} else {
		printf("getsockname() retuned no data!\n");
	}


	printf("\n\nClosing socket...\n");

	sock_close(s);
	// Discard any data until fully closed.
	while(tcp_tick(s)) sock_fastread(s, NULL, 1024);

	printf("Socket closed. All done!\n");
}
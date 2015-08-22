/*******************************************************************************
        dns.c
        Digi International, Copyright (c) 2000.  All rights reserved.

        Demonstration of how to look up an IP address through a DNS
        server.
*******************************************************************************/
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


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use dcrtcp.lib

void main()
{
	longword ip;
	char buffer[20];

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);


	ip=resolve("www.digi.com");
	if(ip==0)
		printf("couldn't find www.digi.com\n");
	else
		printf("%s is www.digi.com's address.\n",inet_ntoa(buffer,ip));
}
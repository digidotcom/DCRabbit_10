/*******************************************************************************
   WiFiDHCPorStatic.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

   This sample program demonstrates the runtime selection of a static
   IP configuration or DHCP. Please see the DHCP.C for a larger example
   of using DHCP with your application.

   At the prompt in the STDIO window, type 's' for static configuration
   and 'd' for DHCP.

   You will need to modify the definitions below for _PRIMARY_STATIC_IP
   and _WIFI_SSID to match your network.  You may also need to modify
   the values for MY_GATEWAY if you are not pinging from the local
   subnet.  Perform a function lookup on TCPCONFIG for more information.

	This sample demonstrates a WiFi specific feature.  Additional networking
	samples for WiFi can be found in the Samples\tcpip directory.

*******************************************************************************/

#memmap xmem

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 *
 * Note that we are using TCPCONFIG 1, which is a static network configuration.
 * However, we want to be able to select DHCP (dynamic) configuration at
 * runtime.  DHCP code is not compiled in by default.  Therefore, we define
 * the macro USE_DHCP to compile in the DHCP code.
 */
#define USE_DHCP

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG					1

#use "dcrtcp.lib"

main()
{
	char ch;
	long ipaddr;
   char ipaddr_string[16];

   printf ("Wi-Fi DHCP or Static sample\n");

   do {
		// Prompt for configuration method
		printf("Select method (s=static, d=dhcp): ");
   	ch=getchar();

	   // Give feedback on the choice
   	printf("%c\n",ch);

      if(ch=='s' || ch=='S') {
		   // Bring the interface up statically configured
         printf ("configuring interface for static IP\n");
			ifconfig(IF_WIFI0,
   				IFS_IPADDR,aton(_PRIMARY_STATIC_IP),
		         IFS_NETMASK, aton(_PRIMARY_NETMASK),
		         IFS_ROUTER_SET, aton(MY_GATEWAY),
               IFS_END);
         break;
      } else if(ch=='d' || ch=='D') {
			// Bring the interface up with DCHP
         printf ("configuring interface for DHCP\n");
			ifconfig(IF_WIFI0,
		         IFS_DHCP, 1,
      		   IFS_END);
         break;
      } else {
      	printf("Invalid selection\n\n");
      }
   } while(1);

   // Initialize the TCP/IP stack
   sock_init_or_exit(1);

   // Print the IP Address to stdio
   ifconfig(IF_WIFI0,IFG_IPADDR,&ipaddr,IFS_END);
   printf("IP Address: %s -- responding to pings\n",inet_ntoa(ipaddr_string,ipaddr));

   // Give the stack time... Ready to be PINGed
   while(1) {
   	tcp_tick(NULL);
   }
}
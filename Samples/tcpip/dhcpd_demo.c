/*
   DHCP Server Demo

   (C)2002-2007 Tom Logic LLC, All Rights Reserved
   Digi International, Copyright © 2010.  All rights reserved.

   IMPORTANT: Before connecting Rabbit to the network and running this sample
              program, ensure that no DHCP server already exists on the network.

   This sample demonstrates the simple DHCP server functionality provided in
   dhcpd.lib. An Ethernet-equipped or WiFi-equipped Rabbit board can provide an
   IP address, a netmask and (optionally) router and DNS addresses for client
   hosts in a LAN subnet.

   A selection of DHCP client pool sizes are possible, one of 1, 5, 13, 29, 61,
   125 or 253 client hosts (i.e. 2**n - 3 clients, where n is in 2,3,4,...,8).

   Optionally, defining DHCPD_GATEWAY and nonzero DHCPD_DNS_PRIMARY or
   DHCPD_DNS_SECONDARY allows Rabbit and its DHCP clients to communicate with
   devices whose IP address is outside of Rabbit's LAN subnet. See the commented
   out example DHCP_GATEWAY definition in this sample for more information.

   Please refer to comments near the top of dhcpd.lib for more complete details
   of Rabbit's simple DHCP server operation.

   Minimal example instructions, using an Ethernet-equipped Rabbit board and a
   Windows PC that has Dynamic C installed:
      1. Ensure that your PC is not set up to be a DHCP server. Refer to your
         PC's manual for the required procedure. Note that PCs are almost never
         configured to be a DHCP server.
      2. Connect the Rabbit's Ethernet port directly to your PC's Ethernet port,
         using either a crossover or straight through Ethernet cable as
         appropriate for your Rabbit board and PC. If unsure which type of
         Ethernet cable to use, an auto-MDIX hub or switch can be connected
         between the Rabbit and PC, using two Ethernet cables in any combination
         of crossover or straight through types.
      3. Power up the Rabbit and the PC.
      4. Configure your PC to be a DHCP client. Refer to your PC's manual for
         the required procedure. Ensure that your PC is not configured with a
         static IP, not even as an alternate configuration.
      5. Using Dynamic C in debug mode, compile and run this (unaltered) sample
         program on your Rabbit board.
      6. Observe Dynamic C's STDIO window to see Rabbit's DHCP server IP, subnet
         mask and DHCP pool address range information.
      7. With Dynamic C's STDIO window in focus, periodically press the PC's D
         key to dump DHCP lease information. After a few moments you should see
         a DHCP lease entry listed.
      8. Compare your PC's network settings with the information listed in
         Rabbit's DHCP lease table; the information should match.

   More elaborate example instructions, using a router, an Ethernet-equipped or
   WiFi-equipped Rabbit board and a Windows PC that has Dynamic C installed:
      1. If your PC is Ethernet-equipped, connect your PC's Ethernet port to a
         LAN-side Ethernet port on your router using a straight through Ethernet
         cable.
      2. If your Rabbit board is Ethernet-equipped, connect the Rabbit's
         Ethernet port to a LAN-side Ethernet port on your router using a
         straight through Ethernet cable.
      3. Power up your Rabbit board, PC and router.
      4. Configure your router to have, on its LAN side, static IP address
         192.168.20.1 and net mask 255.255.255.0. Refer to your router's manual
         for the required procedure.
      5. Temporarily configure your PC to have static IP address 192.168.20.200
         and net mask 255.255.255.0. Refer to your PC's manual for the required
         procedure. Note that this static IP address is chosen because it is
         outside of the range of DHCP client addresses to be served by Rabbit.
      6. Ensure that the LAN side of your router has no DHCP server. In many
         cases, this requires disabling of the router's built-in DHCP server.
         Refer to your router's manual for the required procedure.
      7. If your router has WiFi B/G capability and your Rabbit board is WiFi-
         equipped, edit the WiFi custom configuration section's IFC_WIFI_.*
         macro definitions, below, to suit your wireless network. Refer to your
         router's manual for the procedure to obtain the required information.
      8. Uncomment the commented-out DHCPD_GATEWAY macro definition line, below.
      9. Edit the PERMANENT_LEASE_MAC_ADDRESS macro value string, in the "#ifdef
         DHCPD_GATEWAY" conditional code section below, to suit your router's
         LAN-side MAC address. Refer to your router's manual for the procedure
         to obtain the required MAC address information.
      10 Using Dynamic C in debug mode, compile and run this (unaltered, except
         as described above) sample program on your Rabbit board.
      11 Configure your PC to be a DHCP client. Refer to your PC's manual for
         the required procedure. Ensure that your PC is not configured with a
         static IP, not even as an alternate configuration.
      12 Observe Dynamic C's STDIO window to see Rabbit's DHCP server IP, subnet
         mask and DHCP pool address range information.
      13 With Dynamic C's STDIO window in focus, periodically press the PC's D
         key to dump DHCP lease information. After a few moments you should see
         the PC's DHCP lease entry listed in addition to the router's permanent
         entry.
      14 Compare your PC's network settings with the information listed in
         Rabbit's DHCP lease table; the information should match.
*/

/*
   Default WiFi / Ethernet custom configuration information follows. Modify as
   required to suit your network environment.
*/

#if RCM4400W_SERIES || RCM5400W_SERIES || RCM5600W_SERIES
	// WiFi-equipped boards
	#define USE_WIFI                  1
	#define IFCONFIG_WIFI0            IFS_DOWN
 #if CPU_ID_MASK(_CPU_ID_) == R5000
	// ETH_MTU must be 1489 or less for Rabbit 5000 WiFi boards.
	#define ETH_MTU                   1489
 #else
	#define ETH_MTU                   1500
 #endif
	#define ETH_MAXBUFS               12

	// Following is an example of a very simple WiFi configuration. See the
	// TCPCONFIG function help WiFi section for more set up information.
	#define IFC_WIFI_SSID             "RabbitTest"
	#define IFC_WIFI_ROAM_ENABLE      1
	#define IFC_WIFI_ROAM_BEACON_MISS 20
	#define IFC_WIFI_MODE             IFPARAM_WIFI_INFRASTRUCTURE
	#define IFC_WIFI_REGION           IFPARAM_WIFI_REGION_AMERICAS
	#define IFC_WIFI_ENCRYPTION       IFPARAM_WIFI_ENCR_NONE
#else
	// Ethernet-equipped boards
	#define USE_ETHERNET              1
	#define IFCONFIG_ETH0             IFS_DOWN
	#define ETH_MTU                   1500
	#define ETH_MAXBUFS               4
#endif

#define USE_PPP_SERIAL               0
#define USE_PPPOE                    0
#define TCPCONFIG                    6

#define USE_DHCP
#define DHCP_NUM_DNS                 5
#define MAX_NAMESERVERS              5

#define MAX_TCP_SOCKET_BUFFERS       3
#define MAX_UDP_SOCKET_BUFFERS       2
#define UDP_BUF_SIZE                 1024
#define TCP_BUF_SIZE                 ((ETH_MTU-40)*4)

#use "dcrtcp.lib"
#use "bootp.lib"

/*
   Default simple DHCP server library custom configuration information follows.
   Modify as required to suit your network environment.
*/

#define DHCPD_HOST_BITS              3                      // 5 DHCP clients
#define DHCPD_NETWORK                IPADDR(192,168,20,0)   // 192.168.20.0
#define DHCPD_LEASE_SECONDS          (60UL*60*1)            // 1 hour lease

#define DHCPD_HOSTNAME               "test"
#define DHCPD_DOMAINNAME             "digi.com"

// Uncomment the following macro definition to enable a gateway device at IP
// address .1. It is then also necessary to:
//    1) Ensure that the gateway device's LAN IP address is either provided by
//       DHCP or, if static, equals DHCP_NETWORK | 1 (see above). If using the
//       default DHCPD_NETWORK macro value, the static address is 192.168.20.1.
//    2) Edit the PERMANENT_LEASE_MAC_ADDRESS macro in the #ifdef DHCPD_GATEWAY
//       code section to match the LAN MAC address of your gateway device.
//    3) Ensure that the Rabbit board that runs this simple DHCP server code is
//       the only DHCP server on the LAN. (Remember to disable DHCP server
//       functionality, if any, on the gateway device itself).
//#define DHCPD_GATEWAY (DHCPD_NETWORK | 1)

#ifdef DHCPD_GATEWAY
	#define DHCPD_DNS_PRIMARY         IPADDR(8,8,8,8)  // 8.8.8.8 public DNS
	#define DHCPD_DNS_SECONDARY       IPADDR(8,8,4,4)  // 8.8.4.4 public DNS
	// Set up a permanent DHCP lease at IP address .1 for the gateway device's
	// LAN MAC address (e.g. 00:21:29:70:DD:5F).
	#define PERMANENT_LEASE_MAC_ADDRESS "\x00\x21\x29\x70\xdd\x5f"
#else
	// no gateway is specified, so there's no (known) way out of this LAN
	#define DHCPD_DNS_PRIMARY         IPADDR(0,0,0,0)  // no primary DNS
	#define DHCPD_DNS_SECONDARY       IPADDR(0,0,0,0)  // no secondary DNS
	// Uncomment the following macro definition to set up a permanent DHCP lease
	// at IP address .1 for the specified MAC address (e.g. 00:90:C2:CE:DC:11).
	//#define PERMANENT_LEASE_MAC_ADDRESS "\x00\x90\xc2\xce\xdc\x11"
#endif

// Uncomment the following macro definition to only respond to Rabbit devices
// (MAC address starts with 00:90:c2).
//#define DHCPD_RABBIT_ONLY

// Uncomment the following macro definition to see DHCP server library activity,
// progress and status information (output to STDIO).
//#define DHCPD_VERBOSE

#use "dhcpd.lib"

// note that dhcpd.lib will set DHCPD_ADDRESS and DHCPD_NETMASK for you based
// on DHCPD_HOST_BITS and DHCPD_NETWORK settings above

void main(void)
{
	int iferr;
	char ipbuf[16];

	printf("Starting the network interface...");

	sock_init();
	printf("done.\nConfiguring the network interface...");

	iferr = ifconfig(IF_DEFAULT,
	                 IFS_DHCP, 0,
	                 IFS_IPADDR, DHCPD_ADDRESS,
	                 IFS_NETMASK, DHCPD_NETMASK,
	                 IFS_UP,
	                 IFS_END);

	printf("done (result %d).\n\n", iferr);

	printf("Starting the dhcp server (v%s).\n", DHCPD_VER_STR);
	if (dhcpd_init(IF_DEFAULT))
	{
		printf("dhcpd_init() failed, program terminated.\n");
	}
	else
	{
		printf("  server ip: %s\n", inet_ntoa (ipbuf, DHCPD_ADDRESS));
		printf("subnet mask: %s\n", inet_ntoa (ipbuf, DHCPD_NETMASK));
		printf("dhcpd serving addresses from .1 to .%u\n", DHCPD_MAX_HOST);

#ifdef PERMANENT_LEASE_MAC_ADDRESS
		// IP address .1 goes to PERMANENT_LEASE_MAC_ADDRESS
		dhcpd_add_permanent(1, PERMANENT_LEASE_MAC_ADDRESS);
#endif
		// dump the complete DHCP leases table, including empty entries
		dhcpd_dump_leases(1);
		printf("Press 'D' to dump the table of DHCP leases.\n");

		while (1)
		{
			tcp_tick(NULL);
			dhcpd_tick();
			if (kbhit())
			{
				switch (getchar())
				{
				case 'd':
				case 'D':
					// dump only the used entries in the DHCP leases table
					dhcpd_dump_leases(0);
					break;
				}
			}
		}
	}
}
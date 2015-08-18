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
/*******************************************************************************
        Samples\TCPIP\dhcp.c

        DHCP (Dynamic Host Configuration Protocol) or BOOTP (Bootstrap
        Protocol) will be used to obtain IP addresses and other
        network configuration items.

        You can press a key ('0' thru '9') to toggle the specified
        interface number up or down at any time.  (Normally, only
        interface 0, Ethernet, will be available but if you have
        PPP then you can try it, too, by changing the TCPCONFIG
        definition).

        If the interface is down, it is brought up (possibly
        reacquiring the DHCP lease).  If up, it is brought down,
        releasing the lease if applicable.  When brought up, and
        the interface is qualified for DHCP, you are prompted to
        tell whether to actually use DHCP, or use the fallbacks only.

        You can also press 'p' - you will be prompted to enter a host
        name or IP address, which will be pinged.

        You can also press 'w' - you will be prompted to enter a host
        name or IP address, which will be sent an HTTP HEAD request to
        TCP port 80.  Since some hosts these days do not respond to ping,
        this instead tries to get a small amount of data from a web server.

        Press 't' to print the current interface, router and ARP cache.
        (This is always done whenever an interface comes up or down).

        'r' and 'R' respectively add and delete a router.
        'n' and 'N' respectively add and delete a nameserver.

        'f' flushes an ARP table entry (by entry number).

        'i' allows you to change the ip address and netmask of the
        specified interface.

*******************************************************************************/
#class auto
#memmap xmem

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 *
 * Config #5 = simple blank-slate DHCP, with fallbacks.  Note that
 * this sample overrides most of this, to show how everything can
 * be done at run-time.
 */
#define TCPCONFIG		5

/*
 * Define this symbol to force BOOTP protocol (not DHCP).  This is useful
 * when code space is tight.  You may need to specially configure your
 * DHCP server to respond to BOOTP-only clients.
 */
//#define BOOTP_ONLY


/*
 * General debugging options...
 */
//#define DCRTCP_DEBUG		// Allow Dynamic C debugging
//#define BOOTP_VERBOSE		// Print lots of detail
//#define ICMP_VERBOSE
//#define IP_VERBOSE
//#define NET_VERBOSE
//#define UDP_VERBOSE
//#define TCP_VERBOSE
//#define DNS_VERBOSE
//#define ARP_VERBOSE
//#define PPP_VERBOSE

// ARP timing overrides
#define ARP_LONG_EXPIRY		20
#define ARP_SHORT_EXPIRY	10
#define ARP_PURGE_TIME		10


// Use interface status callback
#define USE_IF_CALLBACK



// Min DHCP retry interval (secs).  If not defined, defaults to 60.  This applies
// when renewing the lease.  We make it artificially short here so that the test
// gives the retransmit logic a work-out!
#define DHCP_MINRETRY 1


#use "tcp_config.lib"	// This is not normally required, but we bring it in here
								// so that following macro settings may override those in the config.

// This optional string will be sent to the DHCP server as the
// 'class identifier' of this host.  This allows the server to
// "know what it's dealing with".  There is no particular standard
// for this string, but for ease of management Z-World suggests
// colon-separated fields in the form "hardware platform:vendor:
// firmware identifier:version number".  Your DHCP server can be
// configured to recognise these strings in order to select
// appropriate parameters. [NB: it is expected that this string
// be the same for all clones, i.e. not customised for each unit
// of the same product -- the ethernet hardware address is unique
// for each unit and thus identifies particular 'individuals' to
// the DHCP server.  Think of class ID as identifying the "what" but
// not the "who"].
#ifndef DHCP_CLASS_ID
	#define DHCP_CLASS_ID "Rabbit-TCPIP:Z-World:DHCP-Test:1.2.0"
#endif

// This macro causes the MAC address to be used as a unique client
// identifier.
#ifndef DHCP_CLIENT_ID_MAC
	#define DHCP_CLIENT_ID_MAC
#endif


#memmap xmem
#use dcrtcp.lib


tcp_Socket wsock;	// Socket for querying web server.

static longword fallback_ip[IF_MAX];
static longword fallback_netmask[IF_MAX];

/*
 * Print some of the DHCP or BOOTP parameters received.  This is an interface up/down callback.
 */
static void print_results(int iface, int up)
{
	auto long tz;
	auto word dhcp_ok, dhcp_fb;
   auto DHCPInfo * di;
   auto long myip;
   auto long mynetmask;
   auto int i;


   printf("\n\nInterface # %d is now %s\n\n", iface, up ? "UP" : "DOWN");

   if (!up) {
	   printf("...Remaining interfaces:\n");
	   goto _exit;
   }

   // Get stuff...
   if (ifconfig(iface,
         IFG_DHCP_INFO, &di,
         IFG_DHCP_OK, &dhcp_ok,
         IFG_DHCP_FELLBACK, &dhcp_fb,
         IFG_IPADDR, &myip,
         IFG_NETMASK, &mynetmask,
         IFS_END)
      || !di) {
      printf("No DHCP info obtained!\n");
      goto _exit;
   }


   printf("Network Parameters:\n");
   printf("  My IP Address = %08lX\n", myip);
   printf("  Netmask = %08lX\n", mynetmask);

   if (dhcp_fb) {
      printf("DHCP fell back to defaults\n");
      goto _exit;
   }

   if (dhcp_ok) {
      printf("DHCP OK.\n");

	   if (di->dhcp_server) {
	      if (di->lease == DHCP_PERMANENT) {
	         printf("  Permanent lease\n");
	      } else {
	         printf("  Remaining lease = %ld (sec)\n", di->lease - SEC_TIMER);
	         printf("  Renew lease in %ld (sec)\n", di->t1 - SEC_TIMER);
	      }
	      printf("  DHCP server = %08lX\n", di->dhcp_server);
	      printf("  Boot server = %08lX\n", di->bootp_host);
	   }
      #if DHCP_NUM_ROUTERS
	   printf("  Routers:        ");
      for (i=0; i < DHCP_NUM_ROUTERS; ++i) printf("%08lX ", di->router[i]);
      printf("\n");
      #endif
      #if DHCP_NUM_DNS
	   printf("  DNS servers:    ");
      for (i=0; i < DHCP_NUM_DNS; ++i) printf("%08lX ", di->dns[i]);
      printf("\n");
      #endif
      #if DHCP_NUM_SMTP
	   printf("  Mail servers:   ");
      for (i=0; i < DHCP_NUM_SMTP; ++i) printf("%08lX ", di->smtp[i]);
      printf("\n");
      #endif
      #if DHCP_NUM_QOTD
	   printf("  Cookie servers: ");
      for (i=0; i < DHCP_NUM_QOTD; ++i) printf("%08lX ", di->cookie[i]);
      printf("\n");
      #endif
   }

   printf("\n\n");
   if (dhcp_get_timezone(&tz))
      printf("Timezone (fallback only) = %ldh\n", tz / 3600);
   else
      printf("Timezone (DHCP server) = %ldh\n", tz / 3600);
   printf("\n\n");

_exit:
	ip_print_ifs();
	router_printall();
   arpcache_printall();
   printf("\nNameserver list:\n");
   servlist_print(&_dns_server_table);
   printf("\n= = = = = = = = = = = = = = =\n\n");

}

int opt_callback(int iface, DHCPInfo * di, int opt, int len, char * data)
{
	// Callback for handling customized DHCP options.  This function is passed to
   // ifconfig(...IFS_DHCP_OPTIONS...).

   printf("opt_callback: got option %d, length %d on i/f %d\n", opt, len, iface);
   return 0;
}

int get_host(longword * host_ip, word * iface, char * type)
{
	auto char buffer[16];
	auto char hostname[128];
   auto char * h, * k;

   *iface = IF_ANY;
	printf("Enter %s host name or IP, then optional interface number\n", type);
   gets(hostname);
   h = hostname;
   while (isspace(*h)) ++h;
   k = h;
   while (*k && !isspace(*k)) ++k;
   if (isspace(*k)) {
   	*k = 0;
      ++k;
		*iface = atoi(k);
   }
   printf("Looking up host %s...\n", h);
   *host_ip = resolve(h);
	if (!*host_ip) {
   	printf("Could not resolve it.\n");
      return 0;
   }
   printf("Resolved to %s\n", inet_ntoa(buffer, *host_ip));
   if (*iface != IF_ANY)
   	printf("...using specific interface %u\n", *iface);
   return 1;
}

int get_ath(ATHandle * athp)
{
	auto char buffer[16];
	printf("Enter ARP table entry number\n");
   gets(buffer);
   if (!isdigit(buffer[0]))
   	return 0;
   *athp = atoi(buffer);
   if (*athp < 0 || *athp >= ARP_TABLE_SIZE || !_arp_data[*athp].ath) {
   	printf("Not a valid ARP table entry.\n");
   	return 0;
   }
   *athp = _arp_data[*athp].ath;
   return 1;
}

int get_iface(word * iface)
{
	auto char buffer[16];
	printf("Enter interface number\n");
   gets(buffer);
   if (!isdigit(buffer[0]))
   	return 0;
   *iface = atoi(buffer);
   if (is_valid_iface(*iface))
   	return 1;
   printf("Interface %u does not exist.\n", *iface);
   return 0;
}

int get_ip_and_netmask(longword * ip, longword * netmask)
{
	auto char buffer[81];
	auto char b2[16];
   auto char * p;
   auto longword tip, tnm;
   auto int bad;

	printf("Enter IP address and optional netmask\n  (currently %s  %s)\n",
   	inet_ntoa(buffer, *ip), inet_ntoa(b2, *netmask));
   gets(buffer);
   p = buffer;
   tip = aton2(&p, &bad, NULL);
   if (bad) {
   	printf("Could not parse IP address.\n");
      return 0;
   }
   if (*p) {
   	tnm = aton2(&p, &bad, NULL);
      if (bad) {
      	printf("Could not parse netmask.\n");
      	return 0;
      }
	   *netmask = tnm;
   }
   *ip = tip;
   return 1;
}

void main()
{
	auto char sock_data[81];
   auto char buf[81];
   auto word iface;
   auto int opt;
   auto DHCPInfo * di;
   auto char c;
   auto int do_ping, do_webhead, sock_buf_len, sock_data_len;
	auto longword ping_host, ping_id, ping_seq, ping_rseq, web_host, sock_buf;
   auto longword router_ip, nameserver_ip;
   auto word ping_timeout;
   auto word ping_iface, web_iface, router_iface, nameserver_iface;
   auto ATHandle ath;

   static char my_opts[127];	// options 1-127 assigned by IANA.


	for (opt = 0; opt < sizeof(my_opts); opt++)
   	my_opts[opt] = opt+1;	// Try to get every option from 1 to 127.  Server will ignore unknown ones.

   // Set fallbacks...
   for (iface = 0; iface < IF_MAX; ++iface) {
   	// 10.10.6.2, 10.10.8.2, 10.10.10.2 etc.
   	fallback_ip[iface] = inet_addr("10.10.6.2") + (iface<<9L);
   	fallback_netmask[iface] = inet_addr("255.255.255.0");
   }

   printf("========= DHCP.C =========\n");
   printf("Enter the following keystrokes:\n");
   printf(" 0-9   toggle specified interface up/down\n");
   printf(" p     ping a host\n");
   printf(" P     ping the same host again\n");
   printf(" w     do a HTTP HEAD request to specified host\n");
   printf(" W     HTTP HEAD from the same host again\n");
   printf(" t     print the interface, router, ARP and DNS server tables\n");
   printf(" r     add a router entry\n");
   printf(" R     delete a router entry\n");
   printf(" n     add a nameserver (DNS) entry\n");
   printf(" N     delete a nameserver entry\n");
   printf(" f     flush an ARP table entry\n");
   printf(" i     set new IP address and netmask\n");
   printf("\n");
   //debug_on = 5;

#define DTIMEOUT 8				// Define short overall timeout for this demo.

	// Set runtime control for sock_init()...
   for (iface = 0; iface < IF_MAX; iface++) {
		if (!is_valid_iface(iface))
      	continue;	// Skip invalid interface numbers

   	ifconfig(iface, IFG_DHCP_INFO, &di, IFS_END);	// Get whether interface is qualified for DHCP

      printf("Interface %d is %squalified for DHCP.\n", iface, di ? "" : "NOT ");

		ifconfig(iface,
	      IFS_ICMP_CONFIG, 1,     // Also allow use of directed ping to configure (only if DHCP times out).
         IFS_IF_CALLBACK, print_results,	// Print results when i/f comes up/down
         IFS_DHCP, di != NULL,	// Use DHCP if interface is qualified for it
	      IFS_DHCP_TIMEOUT, DTIMEOUT,      // Specify timeout in seconds
	      IFS_DHCP_FALLBACK, 1,   // Allow use of fallbacks to static configuration
         IFS_DHCP_OPTIONS, sizeof(my_opts), my_opts, opt_callback,
         IFS_DHCP_QUERY, IF_P2P(iface),		// Use following address, with DHCP only for other parms - note that
         								// many DHCP servers do not support this.  It is more useful for
                                 // PPP interfaces.
	      IFS_END);
      if (di)
      	// Specify fallbacks for DHCP...
	      ifconfig(iface,
	         IFS_IPADDR, fallback_ip[iface], // Create different addresses for each i/f
	         IFS_NETMASK, fallback_netmask[iface],
	         IFS_END);
   }

	printf("Starting network (DHCP timeout %d seconds)...\n", DTIMEOUT);

   /*
    * Actually initialize the network interfaces.  This blocks for a short time (about 1s)
    * to allow for Ethernet negotiation, but does not block to wait for DHCP to complete.
    * The non-blocking behavior is new since DC8.04.
    */
   sock_init();

	printf("Done sock_init()\n");

   do_ping = 0;
   ping_seq = 0;
   do_webhead = 0;

   // Allocate web socket buffer
   sock_buf_len = 5000;
   sock_buf = xalloc(sock_buf_len);

   for (;;) {
   	tcp_tick(NULL);		// Drive DHCP and everything else.

      if (kbhit()) {
      	c = getchar();
         if (c == 'p') {
         	do_ping = get_host(&ping_host, &ping_iface, "ping");
            continue;
         }
         if (c == 'P') {
         	do_ping = 1;	// same as before
            continue;
         }
         if (c == 'w') {
         	if (!do_webhead)
         		do_webhead = get_host(&web_host, &web_iface, "www");
            continue;
         }
         if (c == 'W') {
         	if (!do_webhead)
         		do_webhead = 1;	// same as before
            continue;
         }
         if (c == 't') {
	         ip_print_ifs();
	         router_printall();
	         arpcache_printall();
   			printf("\nNameserver list:\n");
   			servlist_print(&_dns_server_table);
            continue;
         }
         if (c == 'r') {
         	if (get_host(&router_ip, &router_iface, "router (add)")) {
            	// We use the low-level function rather than ifconfig().
            	ath = router_add(router_ip, router_iface, 0, 0, 0);
               if (ath < 0)
               	printf("Could not add router\n");
               else
               	router_printall();
            }
         	continue;
         }
         if (c == 'R') {
         	if (get_host(&router_ip, &router_iface, "router (delete)")) {
            	ath = router_delete(router_ip);
               if (ath < 0)
               	printf("Could not delete router\n");
               else
               	router_printall();
            }
         	continue;
         }
         if (c == 'n') {
         	if (get_host(&nameserver_ip, &nameserver_iface, "nameserver (add)")) {
            	ifconfig(IF_ANY, IFS_NAMESERVER_ADD, nameserver_ip, IFS_END);
   				servlist_print(&_dns_server_table);
            }
         	continue;
         }
         if (c == 'N') {
         	if (get_host(&nameserver_ip, &nameserver_iface, "nameserver (delete)")) {
            	ifconfig(IF_ANY, IFS_NAMESERVER_DEL, nameserver_ip, IFS_END);
   				servlist_print(&_dns_server_table);
            }
         	continue;
         }
         if (c == 'f') {
         	if (get_ath(&ath)) {
            	printf("Flushing ARP table entry %u\n", ATH2INDEX(ath));
            	arpcache_flush(ath);
            }
         }
         if (c == 'i') {
         	if (get_iface(&iface) && get_ip_and_netmask(fallback_ip + iface, fallback_netmask + iface)) {
            	printf("Setting new fallbacks.\n");
	            ifconfig(iface,
	               IFS_IPADDR, fallback_ip[iface],
	               IFS_NETMASK, fallback_netmask[iface],
	               IFS_END);
            }
         }
         if (!isdigit(c))
         	continue;
			iface = c - '0';
         if (!is_valid_iface(iface)) {
         	printf("%d is not a valid interface number\n", iface);
         	continue;
         }
         if (ifstatus(iface)) {
         	printf("Bringing interface %d DOWN...\n", iface);
         	ifdown(iface);
         }
         else {
         	// Bring up.  Need to explicitly restate fallbacks, since they are nullified
            // if the interface successfully used DHCP.
   			ifconfig(iface, IFG_DHCP_INFO, &di, IFS_END);	// Get whether interface is qualified for DHCP
         	printf("Bringing interface %d UP%s...\n", iface, di ? " (using DHCP)" : "");
            if (di) {
            	// Qualified for DHCP.  Ask whether to use DHCP.
               printf("Interface %u is qualified for DHCP.  Use DHCP? (y/n)\n", iface);
               gets(buf);
	            ifconfig(iface,
	               IFS_IPADDR, fallback_ip[iface],
	               IFS_NETMASK, fallback_netmask[iface],
                  IFS_DHCP, toupper(buf[0]) == 'Y',
	               IFS_UP,
	               IFS_END);
            }
            else
            	// Not DHCP.  Bring up as before.
            	ifconfig(iface,
               	IFS_UP,
                  IFS_END);
         }
      }

      /*
       * Drive "ping" state machine
       */
      if (do_ping == 1) {
      	++ping_seq;
      	if (_send_ping_iface(ping_host, ping_seq, 20, 0, &ping_id, ping_iface)) {
         	printf("Could not send ping.\n");
            do_ping = 0;
         }
         else {
         	printf("Sent ping (seq %lu, id %lu)\n", ping_seq, ping_id);
         	do_ping = 2;
            ping_timeout = _SET_SHORT_TIMEOUT(3000);		// 3 seconds to answer
         }
      }
      else if (do_ping == 2) {
      	if (_chk_ping(ping_host, &ping_rseq) != -1L) {
         	printf("Received ping response (seq %lu)\n", ping_rseq);
            do_ping = 0;
         }
         else if (_CHK_SHORT_TIMEOUT(ping_timeout)) {
         	printf("Ping timed out\n");
            do_ping = 0;
         }
      }

      /*
       * Drive "web" state machine
       */
      if (do_webhead == 1) {
      	if (!tcp_extopen(&wsock, web_iface, 0, web_host, 80, NULL, sock_buf, sock_buf_len)) {
         	printf("Could not ARP this address.\n");
            sock_perror(&wsock, NULL);
            do_webhead = 0;
         }
         else
         	do_webhead = 2;
      }
      else if (do_webhead == 2) {
      	if (sock_established(&wsock)) {
         	sock_fastwrite(&wsock, "HEAD / HTTP/1.0\r\n\r\n", 19);
            do_webhead = 3;
         }
         else if (!sock_alive(&wsock)) {
         	printf("Web socket failed.\n");
            sock_perror(&wsock, NULL);
            do_webhead = 0;
         }
      }
      else if (do_webhead == 3) {
      	sock_data_len = sock_fastread(&wsock, sock_data, sizeof(sock_data)-1);
      	if (sock_data_len > 0) {
         	sock_data[sock_data_len] = 0;
         	printf("%s", sock_data);
         }
         else if (sock_data_len < 0) {
         	sock_close(&wsock);
            do_webhead = 4;
         }
      }
      else if (do_webhead == 4) {
      	if (!sock_alive(&wsock)) {
         	printf("Completed.\n");
            sock_perror(&wsock, NULL);
         	do_webhead = 0;
         }
      }
   }
}
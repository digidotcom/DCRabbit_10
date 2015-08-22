/*******************************************************************************
        Samples\TCPIP\dhcp_bootfile.c
        Rabbit Semiconductor, 2003

        DHCP (Dynamic Host Configuration Protocol) or BOOTP (Bootstrap
        Protocol) will be used to obtain IP addresses and other
        network configuration items.

        This sample shows how to retrieve the "boot file" which may
        be specified by the DHCP server.

        To get the name of the boot file, you request option '0' in the
        DHCP options request list.  The boot file server IP address
        is retrieved in the option callback using di->bootp_host
        and the file is downloaded using TFTP.


*******************************************************************************/
#class auto

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
 * This demo doesn't need TCP, only UDP.
 */
#define DISABLE_TCP

/*
 * General debugging options...
 */
//#define DCRTCP_DEBUG		// Allow Dynamic C debugging
//#define TFTP_DEBUG
//#define BOOTP_VERBOSE		// Print lots of detail
//#define TFTP_VERBOSE
//#define IP_VERBOSE
//#define NET_VERBOSE
//#define UDP_VERBOSE
//#define ARP_VERBOSE

// ARP timing overrides
//#define ARP_LONG_EXPIRY		20
//#define ARP_SHORT_EXPIRY	10
//#define ARP_PURGE_TIME		10


// Use interface status callback (required)
#define USE_IF_CALLBACK



// Min DHCP retry interval (secs).  If not defined, defaults to 60.  This applies
// when renewing the lease.  We make it artificially short here so that the test
// gives the retransmit logic a work-out!
#define DHCP_MINRETRY 1


// Specify buffers for UDP socket.  Need one for TFTP.
#define MAX_UDP_SOCKET_BUFFERS	1


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
#use dcrtcp.lib			// Bring in standard networking
#use tftp.lib				// Bring in TFTP

int got_bootfile;			// We set this when a bootfile name is obtained from DHCP server.
int load_bootfile;		// Set when 'got_bootfile' and network interface has come up fully.
char my_bootfile[129];	// Enough space for bootfile name storage.  Requires 129.
longword bootfile_host;	// This is the boot file host's IP address.
udp_Socket mysock;		// A UDP socket for TFTP to use

/*
 * Print some of the DHCP or BOOTP parameters received.  This is an interface up/down callback.
 */
void print_results(int iface, int up)
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
   //arpcache_printall();
   printf("\nNameserver list:\n");
   servlist_print(&_dns_server_table);
   printf("\n= = = = = = = = = = = = = = =\n\n");

   if (got_bootfile)
   	load_bootfile = 1;	// Signal main loop to load it.
}

int opt_callback(int iface, DHCPInfo * di, int opt, int len, char * data)
{
	char strbuf[30];	// String buffer for converting IP address to ascii

	// Callback for handling customized DHCP options.  This function is passed to
   // ifconfig(...IFS_DHCP_OPTIONS...).

   // We use this function to grab the bootfile name.  This is recognized when
   // the 'opt' parameter is 0.

   printf("opt_callback: got option %d, length %d on i/f %d\n", opt, len, iface);
   if (opt == 0 && !got_bootfile) {
   	// Got first bootfile name
      memcpy(my_bootfile, data, len);
      // Null-terminate it
      my_bootfile[len] = 0;
      printf("Bootfile is '%s'", my_bootfile);
      if (di->bootp_host) {
	      got_bootfile = 1;
         bootfile_host = di->bootp_host;
         printf(" @ %s\n", inet_ntoa(strbuf, bootfile_host));
      }
      else
      	printf("\n...however there is no specified host to get it from!\n");
   }
   return 0;
}


int download_using_TFTP(char * name)
{
	// Use TFTP to get the bootfile.  Function returns a non-negative value on success
   // (which is the length of the downloaded file) or negative for error.  See
   // tftp_init() and tftp_tick() for error code documentation.
   // Rather than use this function, you could also use the tftp_exec() function
   // but you have to provide a string host name to tftp_exec() whereas we have a
   // binary IP address.
   int status;
   struct tftp_state ts;
#define MAX_FILESIZE		3000		// Max bytes to download

	load_bootfile = 0;				// Don't do it more than once

   ts.state = 0;		// Tell it to read (download)
   ts.buf_addr = xalloc(MAX_FILESIZE);	// Where to put the data (in a real program,
   												// you would want to save this address somewhere).
   ts.buf_len = MAX_FILESIZE;				// Max file size to download.
   ts.my_tid = 0;	// Tell it to use default TFTP port number
   ts.sock = &mysock;			// Give it a socket to work with
   ts.rem_ip = bootfile_host;	// IP address of server
   ts.mode = TFTP_MODE_OCTET;	// Binary transfer mode
   strcpy(ts.file, my_bootfile);	// Name of file (from DHCP server)

   if (status = tftp_init(&ts)) {
   	printf("TFTP initialization failed.\n");
      return status;
   }
   else {
		while ((status = tftp_tick(&ts)) == 1);	// Loop until complete or error
      if (!status)
      	return ts.buf_used;		// Return length of downloaded file
   }
	return status;
}


void main()
{
   word iface;
   DHCPInfo * di;
   int status;
   static char my_opts[1];	// This is set to a single entry of '0' (get bootfile name)
   								// since we are not interested in other options.


  	my_opts[0] = 0;

   got_bootfile = 0;			// This flag will be set if a bootfile name is available.
   load_bootfile = 0;		// This is set when interface also comes up.


   printf("========= DHCP_BOOTFILE.C =========\n");
   //debug_on = 5;

#define DTIMEOUT 20				// Define short overall timeout for this demo.

	// Set runtime control for sock_init()...
   for (iface = 0; iface < IF_MAX; iface++) {
		if (!is_valid_iface(iface))
      	continue;	// Skip invalid interface numbers

   	ifconfig(iface, IFG_DHCP_INFO, &di, IFS_END);	// Get whether interface is qualified for DHCP

      printf("Interface %d is %squalified for DHCP.\n", iface, di ? "" : "NOT ");

		ifconfig(iface,
         IFS_IF_CALLBACK, print_results,	// Print results when i/f comes up/down
         IFS_DHCP, di != NULL,	// Use DHCP if interface is qualified for it
	      IFS_DHCP_TIMEOUT, DTIMEOUT,      // Specify timeout in seconds
         // Tell it to call the callback when the bootfile option is received...
         IFS_DHCP_OPTIONS, sizeof(my_opts), my_opts, opt_callback,
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


   for (;;) {
   	tcp_tick(NULL);		// Drive DHCP and everything else.
      							// When the interface comes up, and there is a bootfile name
									// provided by the DHCP server, the interface callback will
                           // set a flag which we can test in order to start downloading
                           // the file.
      if (load_bootfile) {
      	status = download_using_TFTP(my_bootfile);
         if (status < 0)
         	printf("Failed to get bootfile, return code %d\n", status);
         else
         	printf("Got bootfile: total bytes %d\n", status);
      }
   }
}
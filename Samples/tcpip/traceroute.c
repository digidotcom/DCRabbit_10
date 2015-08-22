/*
	Samples/tcpip/traceroute.c
	Digi International, Copyright ©2009.  All rights reserved.

	Demonstrates reverse DNS lookups (PTR queries) and use of a custom ICMP
	handler to process the probe responses.

*/

// Define to enable PTR lookups (IP address to name) in DNS.LIB
#define DNS_ENABLE_REVERSE_LOOKUP

// Define to have DNS library print status messages to STDOUT.  This sample
// uses the DNS library to perform
//#define DNS_VERBOSE

// Define to have ICMP library print status messages to STDOUT.  The ICMP
// library receives packets from each router between the Rabbit and the
// probed host.
//#define ICMP_VERBOSE

// Define to have UDP library print status messages to STDOUT.  This sample
// and the DNS library both use UDP sockets.
//#define UDP_VERBOSE

// Maximum number of hops to track between source and destination
#define TRACEROUTE_RECORDS 30

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

/********************************
 * End of configuration section *
 ********************************/

// This sample uses UDP sockets, so it's necessary to allocate UDP buffers
#define MAX_UDP_SOCKET_BUFFERS 4

#memmap xmem
#use "dcrtcp.lib"

// Wikipedia <http://en.wikipedia.org/wiki/Traceroute>
// "On modern Unix and Linux-based operating systems, the traceroute utility
//  by default uses UDP datagrams with destination ports number from
//  33434 to 33534."
#define TRACEROUTE_BASEPORT 33434U

typedef struct {			// 19 bytes without the hostname field
	longword ip;			// IP address that responded
	word		sent;			// value of MS_TIMER when packet sent
	word		id;			// ID of UDP packet (used to match ICMP responses)
	word		minrt;		// minimum round-trip-time of ICMP response
	byte		responses;	// number of responses (0, 1, 2 or 3)
	int		dnshandle;	// handle of PTR lookup
	char		hostname[DNS_MAX_NAME];
} TRACEROUTE_ENTRY;

typedef struct {
	longword				start;
	int					maxhop;
	int					maxresponse;
	longword				destip;
	TRACEROUTE_ENTRY	hop[TRACEROUTE_RECORDS];
} TRACEROUTE_TABLE;

far TRACEROUTE_TABLE	tr;
udp_Socket udpSock;
unsigned long traceip;

#define DIVLINE \
	"-------------------------------------------------------------------------\n"
void traceroute_print()
{
	int i;
	char ipbuf[16];
	far char *hostname;

	printf ("-- Traceroute Results --\n");
	printf ("hop   ip address    round-trip-time  hostname\n");
	printf (DIVLINE);
	for (i = 0; i <= tr.maxhop; ++i)
	{
		if (tr.hop[i].responses == 0)
		{
			printf ("%2u  no responses\n", i+1);
		}
		else
		{
			inet_ntoa (ipbuf, tr.hop[i].ip);
			hostname = tr.hop[i].hostname;
			printf ("%2u  %15s %4u ms RTT  %ls\n", i+1, ipbuf,
				tr.hop[i].minrt, hostname[0] ? hostname : (far char *) ipbuf);
		}
	}
	printf (DIVLINE);

}

// Callback function for DNS library to pass PTR responses back to our program.
void traceroute_dns_ptr_handler (longword ip, char *hostname)
{
	int i;

	for (i = 0; i < TRACEROUTE_RECORDS; ++i)
	{
		if (tr.hop[i].ip == ip)
		{
			// Copy hostname for <ip> into our traceroute table, truncating
			// to the size of the hostname if necessary.
			snprintf (tr.hop[i].hostname, sizeof(tr.hop[i].hostname),
																				"%s", hostname);
		}
	}
}

// Callback function to process ICMP messages, typically TTL exceeded, from
// routers between the Rabbit and the destination.
int traceroute_icmp_handler (in_Header *ip)
{
	icmp_pkt *icmp;
	in_Header *encip;
	int len;
	int i;
	int h;
	longword hisip;
	longword resolveip;
	char ipbuf[16];
	char inaddr[30];		// 15 for dotted quad + 14 for ".in-addr.arpa." + 1 null
	longword t;
	word rt;

	far TRACEROUTE_ENTRY *trhop;

   len = in_GetHdrlenBytes( ip );
   icmp = (icmp_pkt*)((byte *)ip + len);
   encip = & (icmp->ip.ip);

   hisip = intel(ip->source);

	t = MS_TIMER;
	if (icmp->unused.type == ICMPTYPE_TIMEEXCEEDED ||
		(icmp->unused.type == ICMPTYPE_UNREACHABLE && hisip == tr.destip))
	{
		// need to decode packet further to find which UDP port I was sending to
		for (i = 0; i <= tr.maxhop; ++i)
		{
			trhop = &tr.hop[i];
			if (encip->identification == trhop->id)
			{
				//printf ("got timeout/unreachable ICMP packet from %s for TTL %d.\n", inet_ntoa (ipbuf, hisip), (i + 1));
				if (i > tr.maxresponse)
				{
					tr.maxresponse = i;
				}
				trhop->responses++;
				trhop->ip = hisip;
				if (hisip == tr.destip)
				{
					tr.maxhop = i;
				}
				rt = (word) (t - tr.start - trhop->sent);
				if (trhop->responses == 1 || rt < trhop->minrt)
				{
					trhop->minrt = rt;
				}
				trhop->id = 0;
				#ifdef DNS_ENABLE_REVERSE_LOOKUP
					if (trhop->dnshandle > 0)
					{
						if (resolve_name_check (trhop->dnshandle, &resolveip)
							== RESOLVE_FAILED)
						{
							trhop->dnshandle = 0;
						}
					}
	            if (trhop->dnshandle == 0)
	            {
	               strcat (inet_ntoa (inaddr, intel(hisip)), ".in-addr.arpa.");
	               h = resolve_name_start (inaddr);
	               if (h > 0)
	               {
	               	trhop->dnshandle = h;
	               }
	            }
				#endif
				break;
			}
		}
		// We've handled this ICMP packet, so tell TCP/IP stack to ignore it.
		return 0;
	}
	else
	{
		// Allow the Rabbit TCP/IP stack to process this ICMP packet
		return 1;
	}
}

void traceroute (unsigned long ip)
{
	udp_Socket	*u;
	int ttl;
	int err;
	int i;
	longword timeout;
	longword dummyip;
	unsigned int baseport;

	far TRACEROUTE_ENTRY *trhop;

	u = &udpSock;

	_f_memset (&tr, 0, sizeof(tr));
	tr.maxhop = TRACEROUTE_RECORDS - 1;
	tr.destip = ip;

	set_icmp_handler (&traceroute_icmp_handler);
	#ifdef DNS_ENABLE_REVERSE_LOOKUP
		set_dns_ptr_callback (&traceroute_dns_ptr_handler);
	#endif

	err = udp_waitopen (u, IF_ANY, 0, tr.destip,
											TRACEROUTE_BASEPORT, NULL, 0, 0, 1024);

	if (err <= 0)
	{
		printf ("Couldn't open socket (%d).\n", err);
	}
	else
	{
		tr.start = MS_TIMER;

		// Probe each router 3 times
		for (i = 0; i < 3; i++)
		{
			// Set the TTL (time to live) of each successive packet in order to
			// detect the route our packets take.  Each router decrements the TTL
			// and when a router has a 0 TTL packet, it sends an ICMP packet back.
	      for (ttl = 0; ttl <= tr.maxhop; ttl++)
	      {
	      	trhop = &tr.hop[ttl];
	         u->ttl = ttl + 1;
	         trhop->sent = (word) (MS_TIMER - tr.start);

	         // Send our UDP probe packet with a unique TTL to help identify
	         // the ICMP packet that comes back from the probe.
	         udp_sendto (u, "TRACEROUTE", 10, tr.destip,
	         													TRACEROUTE_BASEPORT + ttl);
	         printf (" sending TTL %d (id = %u) \r", u->ttl, ip_id);
	         trhop->id = intel16(ip_id);
            // Wait for ICMP response to our probe
            timeout = _SET_TIMEOUT(100);
            while (!chk_timeout(timeout))
            {
               tcp_tick(NULL);
            }
	         #ifdef DNS_ENABLE_REVERSE_LOOKUP
	            if (trhop->dnshandle > 0)
	            {
	               resolve_name_check (trhop->dnshandle, &dummyip);
	            }
				#endif
	      }
	      #ifdef DNS_ENABLE_REVERSE_LOOKUP
	      	// check for additional DNS PTR responses
	         for (ttl = 0; ttl < TRACEROUTE_RECORDS; ttl++)
	         {
	            if (tr.hop[ttl].dnshandle > 0)
	            {
	               switch (resolve_name_check (tr.hop[ttl].dnshandle, &dummyip))
	               {
	                  case RESOLVE_SUCCESS:
	                     tr.hop[ttl].dnshandle = -1;
	                     break;

	                  case RESOLVE_FAILED:
	                  case RESOLVE_TIMEDOUT:
	                  case RESOLVE_HANDLENOTVALID:
	                     tr.hop[ttl].dnshandle = 0;
	                     break;
	               }
	            }
	         }
	      #endif
	   }
	   timeout = _SET_TIMEOUT (1024);
	   printf ("1 second delay for extra packets to come in...\n");
      while (!chk_timeout(timeout))
      {
         tcp_tick(NULL);
      }

      // Disable the callback handlers for ICMP packets and DNS PTR responses.
		set_icmp_handler (NULL);
	   #ifdef DNS_ENABLE_REVERSE_LOOKUP
	      set_dns_ptr_callback (NULL);
	   #endif
	   traceroute_print();
	}

}


int main ()
{
	unsigned long ip;
	char buffer[80];
	char ipbuf[16];

	printf ("Calling sock_init()...\n");
	sock_init_or_exit(1);

	for (;;)
	{
	   printf ("\n\nEnter IP address or hostname for traceroute: ");
	   gets (buffer);

		// resolve hostname or dotted quad
		printf ("resolving hostname...\n");
		ip = resolve (buffer);

	   if (ip)
	   {
         printf ("Starting traceroute to %s (%s):\n", buffer,
         	inet_ntoa (ipbuf, ip));
		   traceroute (ip);
		}
		else
		{
			printf ("Unable to resolve '%s', try another hostname or address.\n");
		}
	}
}


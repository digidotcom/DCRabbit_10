/*
	dyndns.c
	Digi International, Copyright © 2008.  All rights reserved.

	Description
	===========
	The website dyndns.org operates a Dynamic DNS service where customers can
	register an IP address for a particular hostname (like test.dyndns.org).
	Take a look at http://www.dyndns.com/services/dns/dyndns/ for additional
	information on this free service.

	This sample program shows how you can use the HTTP Client library on a
	Rabbit to register its IP address to a particular hostname.  The sample
	uses test.dyndns.org as the hostname.

	A Dynamic DNS service is useful for keeping a hostname up-to-date with a
	device's current IP address.  Most PPP connections (either dialup or cellular
	modem) have a different IP address on each connection.  Many DSL connections
	will also have IP addresses that change periodically.

	Instructions
	============
	Update the defines below with the account information you've already set
	up with dyndns.org.  The sample will work as-is, using the default
	configuration which sets the ip address for the hostname test.dyndns.org.

	Since the default settings use the official DynDNS test hostname
	(test.dyndns.org), it's likely that another person on the Internet will
	change the IP address  soon after this sample sets it.

	If you register with dyndns.org for your own hostname, AND your Rabbit
	connects to the Internet directly (without going through a firewall), it
	should be possible to ping it from any computer on the Internet by using
	the hostname.

*/

///// Configuration Options /////

#define DYNDNS_SERVER "members.dyndns.org"
#define DYNDNS_HOSTNAME "test.dyndns.org"
#define DYNDNS_ACCOUNT "test"
#define DYNDNS_PASSWORD "test"

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// define HTTPC_VERBOSE to turn on verbose output from the HTTP Client library
//#define HTTPC_VERBOSE

///// End of Configuration Options /////

//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// by default, compile functions to xmem
#memmap xmem

// Load the TCP/IP networking library
#use "dcrtcp.lib"

#use "http_client.lib"

// Check if an IP address is in one of the private address ranges defined
// by RFC 1918.  192.168.0.0 to 192.168.255.255, 172.16.0.0 to 172.31.255.255
// and 10.0.0.0 to 10.255.255.255.
int isPrivateAddress( unsigned long ip)
{
	return ( (ip & 0xFFFF0000) == IPADDR(192,168,0,0)
			|| (ip & 0xFFF00000) == IPADDR(172,16,0,0)
			|| (ip & 0xFF000000) == IPADDR(10,0,0,0) );
}

// Use checkip.dyndns.org to look up this device's public IP address (i.e.,
// the IP address that servers on the Internet see as the source IP of packets
// we send).
// return codes:
//		-EINVAL: NULL passed in for either parameter.
//		-NETERR_DNSERROR: Can't resolve hostname.
//		-BADMSG: Couldn't parse server response.
int checkip( tcp_Socket *sock, longword *publicip)
{
	int err;
	httpc_Socket hsock;
	char response[200];
	char *ipstr, *end;

	if (! publicip || ! sock)
	{
		return -EINVAL;
	}
	*publicip = 0;			// default value

	err = httpc_init( &hsock, sock);
   if (err)
   {
      printf( "%s: error %d calling %s\n", "checkip", err, "httpc_init");
   }
   else
   {
      err = httpc_get( &hsock, "checkip.dyndns.org", 80, "/", NULL);
	   if (err)
	   {
	      printf( "%s: error %d calling %s\n", "checkip", err, "httpc_get");
		}
   }

   if (!err)
   {
      err = httpc_skip_headers(&hsock);
      if (err > 0)
      {
      	// err is number of bytes in headers, set to 0 (no error)
      	err = 0;
      }
	}

	if (!err)
	{
		/* Typical response (line break added for readability):
			<html><head><title>Current IP Check</title></head>
			<body>Current IP Address: 123.45.67.89</body></html>
		*/
		while (err == 0 && hsock.state == HTTPC_STATE_BODY)
		{
			err = httpc_read_body( &hsock, response, sizeof(response) - 1);
		}

		if (err <= 0)
		{
			printf( "%s: error %d reading body\n", "checkip", err);
		}
		else
		{
			response[err] = '\0';		// add null terminator

			#ifdef STDIO_ENABLE_LONG_STRINGS
				// with long strings enabled, printf will print all of response
	         printf( "%s: response is \n---\n%s\n---\n", "checkip", response);
			#else
	         // printf only prints up to 127 characters, so break it up
	         // into three calls.
	         printf( "%s: response is\n---\n", "checkip");
	         printf( "%s", response);
	         printf( "\n---\n");
			#endif

			// default error to indicate parsing problem
			err = -EBADMSG;

	      if ( (ipstr = _n_strstr( response, "Address: ")) )
	      {
	      	ipstr += 9;		// point to IP address
	      	if ( (end = _n_strchr( ipstr, '<')) )
	      	{
					*end = '\0';	// convert start of next tag to null terminator
	            if ( (*publicip = inet_addr( ipstr)) )
	            {
	            	printf( "%s: successfully parsed %s\n", "checkip", ipstr);
						err = 0;
	            }
	      	}
	      }

	      if (err)
	      {
				printf( "%s: couldn't parse response\n", "checkip");
	      }
		}
   }

   httpc_close( &hsock);
   tcp_tick( NULL);

   return err;
}

// Map an ip address to a hostname using the dyndns.org service.
// return codes:
//		-EINVAL: NULL passed in for either parameter.
//		-NETERR_DNSERROR: Can't resolve hostname.
//		-BADMSG: Couldn't parse server response.
int dyndns( tcp_Socket *sock, const char *server, const char *account,
	const char *password, const char *hostname, unsigned long ip)
{
	httpc_Socket hsock;
	unsigned long current_ip;
	int err;

	char ip_str[16];
	char url[256];
	char auth[64];

	inet_ntoa(ip_str, ip);

	if (isPrivateAddress( ip))
	{
		// if we have a private IP address, we're behind a NAT router and should
		// register the public IP address of the router.  We can use another
		// service of dyndns.org to look up the public IP.
		printf( "%s: warning, %s is a private IP address\n", "dyndns", ip_str);
		printf( "%s: looking up public IP to use instead\n", "dyndns");
		err = checkip( sock, &ip);
		if (err)
		{
			printf( "%s: failed to get public IP, update cancelled\n", "dyndns");
			return err;
		}

		// update ip_str to be my public IP address
		inet_ntoa(ip_str, ip);
	}

	current_ip = resolve( hostname);

	if (current_ip == ip)
	{
		printf( "%s: IP is already set to %s, no update necessary\n",
			"dyndns", ip_str);
		return 0;
	}

	printf( "%s: updating %s to %s (was %s)\n", "dyndns", hostname, ip_str,
		inet_ntoa( url, current_ip));

   err = httpc_init( &hsock, sock);
   if (err)
   {
      printf( "%s: error %d calling %s\n", "dyndns", err, "httpc_init");
   }
   else
   {
   	snprintf( auth, sizeof(auth), "%s:%s", account, password);
		sprintf( url, "/nic/update?hostname=%s&myip=%s&" \
			"wildcard=NOCHG&mx=NOCHG&backmx=NOCHG", hostname, ip_str);
      err = httpc_get( &hsock, server, 80, url, auth);
	   if (err)
	   {
	      printf( "%s: error %d calling %s\n", "dyndns", err, "httpc_get");
		}
   }

   if (!err)
   {
      err = httpc_skip_headers(&hsock);
      if (err > 0)
      {
      	err = 0;
      }
	}

	if (!err)
	{
		// dyndns response will be short (should be "good <ipaddr>")
		while (err == 0 && hsock.state == HTTPC_STATE_BODY)
		{
			err = httpc_read_body( &hsock, url, sizeof(url) - 1);
		}

		if (err <= 0)
		{
			printf( "%s: error %d reading body\n", "dyndns", err);
		}
		else
		{
			url[err] = '\0';		// add null terminator
	      printf( "%s: response is [%s]\n", "dyndns", url);

			if (! strncmp( url, "good", 4))
			{
				// changed IP
				err = 0;
			}
			else if (! strncmp( url, "nochg", 5))
			{
				// no change, IP already set
				err = 0;
			}
			else
			{
				err = -EBADMSG;
			}

	      if (!err)
	      {
	         printf( "%s: successfully set %s to %s\n", "dyndns",
	         	hostname, ip_str);
	      }
		}
   }

   httpc_close( &hsock);
   tcp_tick( NULL);

   return err;

}

// It's safer to keep sockets as globals, especially when using uC/OS-II.  If
// your socket is on the stack, and another task (with its own stack, instead
// of your task's stack) calls tcp_tick, tcp_tick won't find your socket
// structure in the other task's stack.
// Even though this sample doesn't use uC/OS-II, using globals for sockets is
// a good habit to be in.
tcp_Socket demosock;

void main()
{
	unsigned long myip;
	int result;

	printf( "Initializing TCP/IP stack...\n");
	sock_init_or_exit(1);

	// initialize tcp_Socket structure before use
	memset( &demosock, 0, sizeof(demosock));

	// get my IP address...
	ifconfig( IF_DEFAULT, IFG_IPADDR, &myip, IFS_END);

   // ...and set my hostname to it
   result = dyndns( &demosock, DYNDNS_SERVER, DYNDNS_ACCOUNT,
      DYNDNS_PASSWORD, DYNDNS_HOSTNAME, myip);
   if (result)
   {
      printf( "error %d (%ls) setting dns\n", result, strerror( result));
   }
	else
	{
		printf( "Hostname set, try pinging %s.\n", DYNDNS_HOSTNAME);
		for (;;)
		{
			tcp_tick( NULL);
		}
	}
}
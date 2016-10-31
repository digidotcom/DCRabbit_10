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
/*
	https_client_nb.c

	Description
	===========
	This sample program demonstrates the use of the HTTP client library to
	request files from a remote web server and display them to stdout.

	This builds on the HTTPS_CLIENT.C sample, for the purpose of showing
	how to perform "non-blocking" web page retrieval.  This is particularly
	important when using secure HTTP, since the initial session negotiation
	can be fairly time consuming.

	Also, rather than printing the entire received web page, it simply prints a
	dot for each 4096 bytes of data received (however you can
	#define SHOW_CONTENT to show the page content).  And we don't print the
	certificate details, but do some timing instead.

	This sample prints out some additional information regarding any
	redirection to an alternative URL.

	Instructions
	============
	Run sample on a Rabbit with an Internet connection.  Enter URLs in the
	STDIO window, and the sample will retrieve them from the HTTP server.

*/


// Import the certificate files.  These are the CAs used at the time of writing
// this sample.  It is subject to change (beyond Digi's control).  You can
// #define SSL_CERT_VERBOSE and X509_VERBOSE in order to find out the
// certificates in use.

#ximport "../sample_certs/EquifaxSecureCA.crt"  ca_pem1
#ximport "../sample_certs/ThawtePremiumServerCA.crt"  ca_pem2
#ximport "../sample_certs/GTECyberTrustGlobalRoot.crt"  ca_pem3
#ximport "../sample_certs/VerisignClass3PublicPrimaryCA.crt"  ca_pem4

#define MP_SIZE 258			// Recommended to support up to 2048-bit RSA keys.
//#define MP_SIZE 514			// Support up to 4096-bit RSA keys.

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

// Comment this out if you don't need to support sha384/sha512-signed certs.
#define X509_ENABLE_SHA512

// Uncomment this if you need to connect to HTTPS servers that don't support
// TLS 1.2 yet.  See httpc_set_tls() for flags related to TLS 1.0 fallback.
//#define SSL_ALLOW_TLS10_CLIENT_FALLBACK

///// Configuration Options /////

// define SHOW_CONTENT to display the body content (else just shows a dot for
// every 4k).
//#define SHOW_CONTENT

// define SHOW_HEADERS to display the HTTP headers
//#define SHOW_HEADERS

// Override the default number of redirections allowed (1).  Don't set this
// too high, since it potentially causes that amount of recursion.  Anything
// over 5 would probably indicate some sort of configuration error in the
// servers (e.g. two servers bouncing a request back and forth).
#define HTTPC_MAX_REDIRECT 5

// define UPDATE_RTC to sync the Rabbit's real-time clock to the web server's
// if more than 10 seconds out of sync.
//#define UPDATE_RTC

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 5

/*
 * Uncomment these macros to enable single-stepping and debug messages.
 */
/*
#define HTTPC_VERBOSE
//#define URL_VERBOSE
#define SSL_SOCK_VERBOSE
//#define _SSL_PRINTF_DEBUG 1
#define SSL_CERT_VERBOSE
//#define X509_VERBOSE

#define DCRTCP_DEBUG
#define HTTPC_DEBUG
//#define URL_DEBUG
//#define X509_DEBUG
#define RSA_DEBUG
#define SSL_CERT_DEBUG
#define SSL_TPORT_DEBUG
#define SSL_SOCK_DEBUG

#define _MALLOC_AUDIT


*/

///// End of Configuration Options /////

#define SSPEC_NO_STATIC		// Required because we're not using any static
									// Zserver resources.

#use "dcrtcp.lib"
#use "ssl_sock.lib"			// It is the inclusion of this library before
									// http_client.lib which basically enables use
									// of HTTPS.  Use of any other SSL client or server
									// will also enable HTTPS.
#use "http_client.lib"


/*
 *  Server certificate policy callback
 */
int my_server_policy(ssl_Socket far * state, int trusted,
	                       struct x509_certificate far * cert,
	                       httpc_Socket far * s)
{
	// This code determines whether the hostname should be the
	// proxy, or the origin ('real') server.
	const char far * host;
	if (httpc_globals.ip)
		host = httpc_globals.proxy_hostname;
	else
		host = s->hostname;

	printf("Certificate issuer:\n");
	if (cert->issuer.c)
		printf("       Country: %ls\n", cert->issuer.c);
	if (cert->issuer.l)
		printf("      Location: %ls\n", cert->issuer.l);
	if (cert->issuer.st)
		printf("         State: %ls\n", cert->issuer.st);
	if (cert->issuer.o)
		printf("  Organization: %ls\n", cert->issuer.o);
	if (cert->issuer.ou)
		printf("          Unit: %ls\n", cert->issuer.ou);
	if (cert->issuer.email)
		printf("       Contact: %ls\n", cert->issuer.email);
	if (cert->issuer.cn)
		printf("            CN: %ls\n", cert->issuer.cn);
	printf("Certificate subject:\n");
	if (cert->subject.c)
		printf("       Country: %ls\n", cert->subject.c);
	if (cert->subject.l)
		printf("      Location: %ls\n", cert->subject.l);
	if (cert->subject.st)
		printf("         State: %ls\n", cert->subject.st);
	if (cert->subject.o)
		printf("  Organization: %ls\n", cert->subject.o);
	if (cert->subject.ou)
		printf("          Unit: %ls\n", cert->subject.ou);
	if (cert->subject.email)
		printf("       Contact: %ls\n", cert->subject.email);
	printf("Server claims to be CN='%ls'\n", cert->subject.cn);
	printf("We are looking for  CN='%ls'\n", s->hostname);

	if (x509_validate_hostname(cert, host)) {
		printf("Certificate hostname mismatch%s!\n",
			httpc_globals.ip ? " (using proxy)" : "");
		printf("Was expecting %ls, got %ls\n\n",
			host, cert->subject.cn);
		return 1;
	}
	
	if (trusted) {
		printf("This server's certificate is trusted\n");
	} else {
		printf("Invalid certificate supplied, or missing root CA necessary to verify.\n");
		// uncomment this line to reject untrusted certificates
		//return 1;
	}
	
	return 0;
}


void print_time()
{
	struct tm		rtc;					// time struct

   mktm( &rtc, read_rtc());
   printf( "The current RTC date/time is: %s", asctime( &rtc));
}

void httpc_demo(tcp_Socket *sock)
{
	char	url[256];
	static char	hdr[256];
	char	body[80];		// buffer for reading body
	long	curr_skew;
	long  totlen;
	long  prevlen;
	unsigned long	t0, t1;			// Millisecond timestamps
	unsigned long	niter;			// Number of loops when waiting

	httpc_Socket hsock;
	int retval;
	int is_text;
	char far *value;

	// last clock skew setting
	curr_skew = 0;

   retval = httpc_init (&hsock, sock);
   if (retval)
   {
      printf ("error %d calling httpc_init()\n", retval);
   }
   else
   {
   	is_text = 0;
      printf ("\nEnter a URL to retrieve using the following format:\n");
      printf ("[http://][user:pass@]hostname[:port][/file.html]\n");
      printf ("Items in brackets are optional.  Examples:\n");
      printf ("      http://www.google.com/\n");
      printf ("      www.google.com\n");
      printf ("      https://www.google.com/accounts/CreateAccount\n");
      while (1)
      {
         printf ("\n\nEnter URL (blank to exit): ");
         gets (url);
         if (*url == '\0')
         {
         	// To save typing and create a default, comment out the 'break'
         	// and uncomment the following line...
         	break;
         	//strcpy(url, "https://www.google.com/accounts/CreateAccount");
         }

         printf ("\nRetrieving [%s]...\n", url);

         // For this sample, turn on basic non-blocking mode and auto redirect
         httpc_set_mode(HTTPC_NONBLOCKING | HTTPC_AUTO_REDIRECT);

         // The following idiom is used to open a connection to a given
         // URL.  Non-blocking mode allows the application to do some other
         // task inside the 'while' loop, while waiting for open to complete.
         // (The iteration count and timer are optional, of course.)
         niter = 0;
         t0 = MS_TIMER;
         retval = httpc_get_url (&hsock, url);
         while (retval == -EAGAIN) {
         	++niter;
         	// The NULL,0 parameters indicate a continuation of the
         	// open process.
         	retval = httpc_get(&hsock, NULL, 0, NULL, NULL);
         }
         t1 = MS_TIMER;
         printf("\n### Non-blocking open took %lums, and %lu iterations ###\n\n",
         	t1-t0, niter);

			// At this point, the connection either completed or failed.
         if (retval)
         {
            printf ("error %d calling httpc_get_url()\n", retval);
            continue;
         }
         else
         {
         	// Reading the headers and body is the same process for
         	// blocking or non-blocking.
         	t0 = MS_TIMER;
         	niter = 0;
            while (hsock.state == HTTPC_STATE_HEADER)
            {
               retval = httpc_read_header (&hsock, hdr, sizeof(hdr));
               if (retval > 0)
               {
	               if ( (value = httpc_headermatch( hdr, "Content-Type")) )
	               {
	                  is_text = (0 == strncmpi( value, "text/", 5));
	               }
                  #ifdef SHOW_HEADERS
                     #ifndef HTTPC_VERBOSE
                        // echo headers if HTTP client didn't already do so
                        printf (">%s\n", hdr);
                     #endif
                  #endif
               }
               else if (retval < 0)
               {
                  printf ("error %d calling httpc_read_header()\n", retval);
            		continue;
               }
               ++niter;
            }
	         t1 = MS_TIMER;
	         printf("\n### Reading headers took %lums, and %lu iterations ###\n\n",
	            t1-t0, niter);
            printf ("Headers were parsed as follows:\n");
            printf ("  HTTP/%s response = %d, filesize = %lu\n",
               (hsock.flags & HTTPC_FLAG_HTTP10) ? "1.0" :
               (hsock.flags & HTTPC_FLAG_HTTP11) ? "1.1" : "???",
               hsock.response, hsock.filesize);
            if (hsock.flags & HTTPC_FLAG_CHUNKED)
            {
               printf ("  body will be sent chunked\n");
            }
            printf ("  Rabbit's RTC is %ld second(s) off of server's time\n",
               hsock.skew - curr_skew);

            #ifdef UPDATE_RTC
            	if (labs (hsock.skew - curr_skew) > 10)
            	{
            		// only update if off by more than 10 seconds
            		print_time();
	               printf ("  Updating Rabbit's RTC to match web server.\n");
	               write_rtc (SEC_TIMER + hsock.skew);
	               curr_skew = hsock.skew;
	               print_time();
	            }
            #endif

            printf ("\nBody:\n");
         	t0 = MS_TIMER;
         	niter = 0;
            totlen = prevlen = 0;
            while (hsock.state == HTTPC_STATE_BODY)
            {
               retval = httpc_read_body (&hsock, body, 79);
               if (retval < 0)
               {
                  printf ("error %d calling httpc_read_body()\n", retval);
               }
               else if (retval > 0)
               {
               	totlen += retval;
               	while (totlen > prevlen + 4096) {
               	#ifndef SHOW_CONTENT
               		printf(".");
               	#endif
               		prevlen += 4096;
               	}
               	#ifdef SHOW_CONTENT
	               if (is_text)
	               {
	                  body[retval] = '\0';
	                  printf ("%s", body);
	               }
	               else
	               {
							mem_dump( body, retval);
	               }
               	#endif
               }
               ++niter;
            }
	         t1 = MS_TIMER;
	         printf("\n### Reading body took %lums, and %lu iterations ###\n\n",
	            t1-t0, niter);
            printf("\n\nTotal received bytes %ld\n", totlen);


            // Analyze result
            printf("==============================\n");
            printf("Response code = %u\n", hsock.response);
            printf("Final URL = %ls\n",
            	hsock.redirect ? hsock.redirect : (char far *)url);
            printf("Total redirections: %u\n",
            	HTTPC_MAX_REDIRECT - hsock.redirs_remaining);
            printf("==============================\n");

         	niter = 0;
         	t0 = MS_TIMER;
            // Use the following idiom to cleanly close the connection
            // in non-blocking mode.  Normally, this works without any
            // iterations.  It's harmless to do this in error conditions.
            while (httpc_close(&hsock) == -EAGAIN) {
            	++niter;
            	// App can do something useful while waiting...
            }
         	t1 = MS_TIMER;
         	printf("\n### Non-blocking close took %lums, and %lu iterations ###\n\n",
           		t1-t0, niter);

         }
      }
   }

}

// It's safer to keep sockets as globals, especially when using uC/OS-II.  If
// your socket is on the stack, and another task (with its own stack, instead
// of your task's stack) calls tcp_tick, tcp_tick won't find your socket
// structure in the other task's stack.
// Even though this sample doesn't use uC/OS-II, using globals for sockets is
// a good habit to be in.
tcp_Socket demosock;

int load_certificates(void)
{
	int rc;
	// Can't store this on the stack (auto) since the HTTP client library stores
	// a reference to it for use later.
	static far SSL_Cert_t trusted;

	// First, parse the trusted CA certificates.
	_f_memset(&trusted, 0, sizeof(trusted));
	rc = SSL_new_cert(&trusted, ca_pem1, SSL_DCERT_XIM, 0);
	if (rc) {
		printf("Failed to parse CA certificate 1, rc=%d\n", rc);
		return rc;
	}
	rc = SSL_new_cert(&trusted, ca_pem2, SSL_DCERT_XIM, 1 /*append*/);
	if (rc) {
		printf("Failed to parse CA certificate 2, rc=%d\n", rc);
		return rc;
	}
	rc = SSL_new_cert(&trusted, ca_pem3, SSL_DCERT_XIM, 1 /*append*/);
	if (rc) {
		printf("Failed to parse CA certificate 3, rc=%d\n", rc);
		return rc;
	}
	rc = SSL_new_cert(&trusted, ca_pem4, SSL_DCERT_XIM, 1 /*append*/);
	if (rc) {
		printf("Failed to parse CA certificate 4, rc=%d\n", rc);
		return rc;
	}
	
	// Set TLS/SSL options.  These act globally, for all HTTPS connections
	// until chenged to some other setting.  Normally, this only needs to
	// be done once at start of program.
	httpc_set_tls(SSL_F_REQUIRE_CERT,	// Check HTTPS server certificate
						NULL,						// We don't have a cert to offer.  Not
													// normally needed for HTTPS.
						&trusted,				// Have a trusted CA!
						my_server_policy);	// Test policy callback

	return 0;
}

int main()
{
	int rc;
	
	rc = load_certificates();
	if (rc) {
		return rc;
	}
	
	// initialize tcp_Socket structure before use
	memset( &demosock, 0, sizeof(demosock));

	printf ("http client v" HTTPC_VERSTR "\n");

	printf ("Initializing TCP/IP stack...\n");
	sock_init_or_exit(1);

   httpc_demo(&demosock);
}
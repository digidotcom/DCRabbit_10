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
        Samples\tcpip\POP3\pop_tls.c

        A program that will connect to a POP3 server and download
        e-mail from it, optionally deleting the messages after they have
        been read. This version extends the "parse_extend.c" sample to
        add TLS security.



        WARNING: This sample suggests use of some freely available POP3
        services (GMail and Hotmail).  This is for purpose of demonstration
        only and does not imply that use of those services for anything other
        than test/development is permitted by those service providers.
        Please read and abide by the provider's relevant service agreements.


        Before running:

        *  Make sure your basic network configuration is OK, including a
           DNS (name server) and a route to the outside Internet.  The
           default in this sample expects DHCP.  You can use a different
           TCPCONFIG macro and set alternative parameters if necessary.
           (If necessary, use a simpler sample to get this working).
        *  Set up a Google Gmail account for test purposes.  Follow Google's
           instructions for "enabling POP" on this account.
           See http://gmail.com.
        *  You'll also need to turn on access for "Less secure apps".  As of
           March 2017, you can do so at:
             https://www.google.com/settings/u/1/security/lesssecureapps
        *  Modify the POP_USER and POP_PASS macros to be the account name and
           password that you used in step (2).  It is most convenient to
           put these in the Options->ProjectOptions->Defines panel e.g.
              POP_USER="my_account@gmail.com"
              POP_PASS="myPassw0rd"
        *  The POP_SERVER and POP_PORT macros are set appropriately for
           Gmail as of the time this sample was constructed, but you may wish
           to check this if the sample seems to fail in spite of everything.
           You can override these macros in the Defines panel.
        *  For Hotmail/Outlook.com, #define USE_OUTLOOK_SETTINGS, and use your
           Hotmail/Outlook.com credentials in POP_USER and POP_PASS.  You may
           also need to enable POP access for your account in Options:
              https://outlook.live.com/owa/#path=/options/popandimap
        
        To date, Yahoo does not allow POP3/SMTP access with their free email
        accounts.
*******************************************************************************/

#class auto

// Import the certificate file(s).  This is the CA used at the time of writing
// this sample.  It is subject to change (beyond Digi's control).  You can
// #define SSL_CERT_VERBOSE and X509_VERBOSE in order to find out the
// certificates in use.
#ximport "../sample_certs/EquifaxSecureCA.crt"  ca_pem1
#ximport "../sample_certs/ThawtePremiumServerCA.crt"  ca_pem2

// These are for Hotmail/Outlook.com (POP3 and SMTP)
#ximport "../sample_certs/GlobalSign Organization Validation CA - SHA256 - G2.cer" ca_pem3
#ximport "../sample_certs/DigiCert-Global-Root-CA.cer" ca_pem4

const long certs[] = { ca_pem1, ca_pem2, ca_pem3, ca_pem4 };

// Uncomment the following line if you're connecting to pop-mail.outlook.com
//#define USE_OUTLOOK_SETTINGS

#define MP_SIZE 258			// necessary for GMail's RSA keys

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

// Comment this out if you don't need to support sha384/sha512-signed certs.
#define X509_ENABLE_SHA512

// Uncomment this if you need to connect to POP servers that don't support
// TLS 1.2 yet.  See pop3_set_tls() for flags related to TLS 1.0 fallback.
//#define SSL_ALLOW_TLS10_CLIENT_FALLBACK


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
#define TCPCONFIG 5		// 5 for DHCP

/*
 * POP3 settings
 */

/*
 *	Enter the name and TCP port of your POP3 server here.
 */
#ifndef POP_SERVER
	#ifndef USE_OUTLOOK_SETTINGS
		#define POP_SERVER	"pop.gmail.com"	// GMail
	#else
		#define POP_SERVER	"pop-mail.outlook.com"		// Hotmail
	#endif
#endif
#ifndef POP_PORT
	#define POP_PORT	995	// Port 995 used for POP3 tunneled through TLS
#endif

/*
 * This is the username and password for the account on the
 * pop3 server.
 */
#ifndef POP_USER
	#warnt "Using bogus account.  Set POP_USER=... and POP_PASS=..."
	#warnt "in the Options->Project->Defines panel."
	#define POP_USER  "me@gmail.com"
	#define POP_PASS  "my_password"
#endif


/* comment this out to delete the messages off the server after they are read */
#define POP_NODELETE

/* comment this out if you want to display the message bodies */
#define HEADERS_ONLY

/*
 *   The SMTP_VERBOSE macro logs the communications between the mail
 *   server and your controller.  Uncomment this define to begin
 *   logging.  The other macros add more info from other components.
 */
//#define POP_VERBOSE
//#define SSL_SOCK_VERBOSE
//#define _SSL_PRINTF_DEBUG 1
//#define SSL_CERT_VERBOSE
//#define X509_VERBOSE
//#define TCP_VERBOSE

//#define POP_DEBUG
//#define X509_DEBUG
//#define RSA_DEBUG
//#define SSL_CERT_DEBUG
//#define SSL_TPORT_DEBUG
//#define SSL_SOCK_DEBUG

/********************************
 * End of configuration section *
 ********************************/



/*
 * When this is defined, the POP3 library will do extra parsing of the
 * incoming e-mails, separating the 'to:', 'from:', 'subject:' and body
 * fields from the rest of the header, and provide this data in a nicer
 * manner.
 * NOTE: Changes the parameters passed to storemsg() .
 */
#define POP_PARSE_EXTRA

#define SSPEC_NO_STATIC		// Required because we're not using any static
									// Zserver resources.
#define POP_AUTH_FAIL_IF_NO_AUTH
#define POP_AUTH_TLS 1		// Required to include TLS
#memmap xmem
#use "dcrtcp.lib"
#use "ssl_sock.lib"
#use "pop3.lib"

/*
 *  Server certificate policy callback
 */
int pop_server_policy(ssl_Socket far * state, int trusted,
	                       struct x509_certificate far * cert,
                          void __far * data)
{
	printf("\nChecking server certificate...\n");
	if (trusted)
		printf("This server's certificate is trusted\n");
	else
		printf("There was no list of CAs, so cannot verify this server's certificate\n");

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
	printf("We are looking for  CN='%s'\n", pop3_getserver());

	if (x509_validate_hostname(cert, pop3_getserver())
#ifdef USE_OUTLOOK_SETTINGS
		// temporary hack to accept *.hotmail.com for pop-mail.outlook.com
		&& x509_validate_hostname(cert, "mail.hotmail.com")
#endif
	) {
		printf("Mismatch!\n\n");
		return 1;
	}
	printf("We'll let that pass...\n\n");
	return 0;
}



/*
 * 	This is the POP_PARSE_EXTRA calling style.
 */
int n;
int storemsg(int num, char *to, char *from, char *subject, char *body, int len)
{
	#GLOBAL_INIT { n = -1; }

	if(n != num) {
		n = num;
		printf("RECEIVING MESSAGE <%d>\n", n);
		printf("\tFrom: %s\n", from);
		printf("\tTo: %s\n", to);
		printf("\tSubject: %s\n", subject);
	}

#ifndef HEADERS_ONLY
	printf("MSG_DATA> '%s'\n", body);
#endif

	return 0;
}

void main()
{
	// Can't store this on the stack (auto) since the POP client library stores
	// a reference to it for use later.
	static far SSL_Cert_t trusted;
	auto int rc, i;

	_f_memset(&trusted, 0, sizeof(trusted));
	for (i = 0; i < (sizeof certs / sizeof certs[0]); ++i) {
	   rc = SSL_new_cert(&trusted, certs[i], SSL_DCERT_XIM, i > 0);
	   if (rc) {
	      printf("Failed to parse CA certificate %u, rc=%d\n", i + 1, rc);
	      return;
	   }
	}

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	pop3_init(storemsg);

	pop3_set_tls(SSL_F_REQUIRE_CERT,		// Check POP3 server certificate
						NULL,			// We don't have a cert to offer
						&trusted,	// Have a trusted CA!
						pop_server_policy,	// Test policy callback
						0);

	pop3_setserver(POP_SERVER);

	pop3_getmail(POP_USER, POP_PASS, 0);

	while((rc = pop3_tick()) == POP_PENDING)
		continue;

	printf("============= Completed: ===============\n");
	if(rc == POP_SUCCESS)
		printf("POP was successful\n");
	else if(rc == POP_TIME)
		printf("POP timed out\n");
	else if(rc == POP_ERROR)
		printf("POP could not open TCP socket\n");
	else
		printf("DNS failed to resolve server domain name\n");

}
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
        smtp_tls.c

        A small program that uses the SMTP library to send an e-mail.
        If the server supports it, TLS will be used to authenticate and
        secure the session.

          *** Don't be fooled: securing mail with TLS does not guarantee
              end-to-end secrecy.  It only allows your mail server
              account name and password to be kept secure from
              eavesdropping. ***

          The use of a trusted (CA) certificate in this sample, and requiring
          verification of the SMTP server certificate, assures that the
          server is not an attacker masquerading as the real server.


        WARNING: This sample suggests use of some freely available SMTP
        services (GMail and Hotmail).  This is for purpose of demonstration
        only and does not imply that use of those services for anything other
        than test/development is permitted by those service providers.
        Please read and abide by the provider's relevant service agreements.


        Before running:

        1) Make sure your basic network configuration is OK, including a
           DNS (name server) and a route to the outside Internet.  The
           default in this sample expects DHCP.  You can use a different
           TCPCONFIG macro and set alternative parameters if necessary.
           (If necessary, use a simpler sample to get this working).
        2) Set up a Google Gmail account for test purposes.  Follow Google's
           instructions for "enabling POP" on this account.  Enabling POP
           also enables mail origination using SMTP.  See http://gmail.com.
        3) Modify the parameters to the smtp_setauth() function call (in this
           sample) to the account name and password that you used in step (2).
           It is most convenient to put these in the
           Options->ProjectOptions->Defines panel e.g.
              SMTP_USER="my_account@gmail.com"
              SMTP_PASS="myPassw0rd"
        4) Modify the FROM, SMTP_TO, SUBJECT and BODY macros to be a valid mail
           recipient and desired message.  Preferably, override SMTP_TO
           in the Defines box to be a valid recipient (that you can check!)
        5) The SMTP_SERVER and SMTP_PORT macros are set appropriately for
           Gmail as of the time this sample was constructed, but you may wish
           to check this if the sample seems to fail in spite of everything.
           You can override these macros in the Defines panel.

        NOTE: you can also use a Hotmail/Outlook.com account by forcing the
        Rabbit to use TLS 1.0 instead of TLS 1.2 (since Microsoft does not
        support TLS 1.2 on their mail servers as of March 2017).
        
        In this case, #define SMTP_SERVER "smtp-mail.outlook.com", and use your
        Hotmail/Outlook.com credentials in SMTP_USER and SMTP_PASS.  There is no
        need to change any of your Hotmail account settings (still true as
        of March 2017).
        
        Unfortunately, Microsoft in their wisdom use 4096 bit RSA keys in
        some of their certificates, thus you need to #define MP_SIZE 514.
        GMail uses 2048-bit keys, and requires a MP_SIZE of at least 258.
        
        To date, Yahoo does not allow POP3/SMTP access with their free email
        accounts.

*******************************************************************************/
#class auto



// Import the certificate files.  These are the CAs used at the time of writing
// this sample.  It is subject to change (beyond Digi's control).  You can
// #define SSL_CERT_VERBOSE and X509_VERBOSE in order to find out the
// certificates in use.

// These two are for Google Gmail (POP3 and SMTP)
#ximport "../sample_certs/EquifaxSecureCA.crt"  ca_pem1
#ximport "../sample_certs/ThawtePremiumServerCA.crt"  ca_pem2

// This one for Hotmail/Outlook.com (POP3 and SMTP)
#ximport "../sample_certs/GlobalSign Organization Validation CA - SHA256 - G2.cer" ca_pem3

// Uncomment the following line if you're connecting to smtp-mail.outlook.com
#define USE_OUTLOOK_SETTINGS

#define MP_SIZE 258			// necessary for GMail's RSA keys

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

// Comment this out if you don't need to support sha384/sha512-signed certs.
#define X509_ENABLE_SHA512

// Uncomment this if you need to connect to SMTP servers that don't support
// TLS 1.2 yet.  See smtp_set_tls() for flags related to TLS 1.0 fallback.
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
 *   These macros need to be changed to the appropriate values or
 *   the smtp_sendmail(...) call in main() needs to be changed to
 *   reference your values.
 */

#ifndef FROM
	#ifdef SMTP_USER
		#define FROM SMTP_USER
	#else
		#define FROM     "001@example.com"
	#endif
#endif
#ifndef SMTP_TO
	#warnt "Using bogus recipient.  Set SMTP_TO=..."
	#warnt "in the Options->Project->Defines panel."
	#define SMTP_TO       "007@example.com"
#endif
#define SUBJECT  "Enemy agents in country"
#define BODY     "Go see Q"

/*
 * This is the username and password for the account on the
 * SMTP server.
 */
#ifndef SMTP_USER
	#warnt "Using bogus account.  Set SMTP_USER=... and SMTP_PASS=..."
	#warnt "in the Options->Project->Defines panel."
	#define SMTP_USER  "me@gmail.com"
	#define SMTP_PASS  "my_password"
#endif


/*
 *   The SMTP_SERVER macro tells DCRTCP where your mail server is.  This
 *   mail server MUST be configured to relay mail for your controller.
 *
 *   This value can be the name or the IP address, however a domain name
 *   should be used since it helps to validate the server certificate,
 *   since the certificate's Common Name (CN) field must correspond to the
 *   domain name that we are expecting.
 *
 *   Rather than using these macros, you can also call smtp_setserver()
 *   and/or smtp_setport() at run-time.
 */
#ifndef SMTP_SERVER
	#ifndef USE_OUTLOOK_SETTINGS
		#define SMTP_SERVER "smtp.gmail.com"
	#else
		#define SMTP_SERVER "smtp-mail.outlook.com"
	#endif
#endif
#ifndef SMTP_PORT
	// Port 587 used by secure SMTP service (both Gmail and Hotmail)
	#define SMTP_PORT   587
#endif

#if defined(USE_OUTLOOK_SETTINGS) && !defined(SSL_ALLOW_TLS10_CLIENT_FALLBACK)
	#define SSL_ALLOW_TLS10_CLIENT_FALLBACK
#endif

/*
 *   The SMTP_DOMAIN should be the name of your controller.  i.e.
 *   "somecontroller.somewhere.com"  Many SMTP servers ignore this
 *   value, but some SMTP servers use this field.  If you have
 *   problems, turn on the SMTP_VERBOSE macro and see were it is
 *   bombing out.  If it is in the HELO or EHLO command consult the
 *   person in charge of the mail server for the appropriate value
 *   for SMTP_DOMAIN. If you do not define this macro it defaults
 *   to the value in MY_IP_ADDRESS.
 *
 */

//#define SMTP_DOMAIN "mycontroller.mydomain.com"

/*
 *   The SMTP_VERBOSE macro logs the communications between the mail
 *   server and your controller.  Uncomment this define to begin
 *   logging.  The other macros add more info from other components.
 */
//#define SMTP_VERBOSE
//#define SSL_SOCK_VERBOSE
//#define _SSL_PRINTF_DEBUG 1
//#define SSL_CERT_VERBOSE
//#define X509_VERBOSE
//#define TCP_VERBOSE

//#define SMTP_DEBUG
//#define X509_DEBUG
//#define RSA_DEBUG
//#define SSL_CERT_DEBUG
//#define SSL_TPORT_DEBUG
//#define SSL_SOCK_DEBUG

/********************************
 * End of configuration section *
 ********************************/
#define SSPEC_NO_STATIC		// Required because we're not using any static
									// Zserver resources.
#define USE_SMTP_AUTH		// Required for use of SMTP over TLS
#define SMTP_AUTH_FAIL_IF_NO_AUTH	// Highly recommended (else no point in TLS)
#memmap xmem
#use "dcrtcp.lib"
#use "ssl_sock.lib"
#use "smtp.lib"

/*
 *  Server certificate policy callback
 */
int smtp_server_policy(ssl_Socket far * state, int trusted,
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
	printf("We are looking for  CN='%s'\n", smtp_getserver());

	if (x509_validate_hostname(cert, smtp_getserver())
#ifdef USE_OUTLOOK_SETTINGS
		// temporary hack to accept *.hotmail.com for smtp-mail.outlook.com
		&& x509_validate_hostname(cert, "mail.hotmail.com")
#endif
	) {
		printf("Mismatch!\n\n");
		return 1;
	}
	printf("We'll let that pass...\n\n");
	return 0;
}


void main()
{
	// Can't store this on the stack (auto) since the SMTP client library stores
	// a reference to it for use later.
	static far SSL_Cert_t trusted;
	auto int rc;

	// First, parse the trusted CA certificates.
	_f_memset(&trusted, 0, sizeof(trusted));
	rc = SSL_new_cert(&trusted, ca_pem1, SSL_DCERT_XIM, 0);
	if (rc) {
		printf("Failed to parse CA certificate 1, rc=%d\n", rc);
		return;
	}
	rc = SSL_new_cert(&trusted, ca_pem2, SSL_DCERT_XIM, 1 /*append*/);
	if (rc) {
		printf("Failed to parse CA certificate 2, rc=%d\n", rc);
		return;
	}
	rc = SSL_new_cert(&trusted, ca_pem3, SSL_DCERT_XIM, 1 /*append*/);
	if (rc) {
		printf("Failed to parse CA certificate 3, rc=%d\n", rc);
		return;
	}


	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	printf("Setting authentication parameters...\n");

	smtp_setauth (SMTP_USER, SMTP_PASS);

	smtp_set_tls(
#ifdef USE_OUTLOOK_SETTINGS
						SSL_F_FORCE_TLS10 |     // Microsoft's servers require TLS 1.0
#endif
						SSL_F_REQUIRE_CERT,		// Check SMTP server certificate
						NULL,						// We don't have a cert to offer.  Not
													// normally needed for SMTP.
						&trusted,				// Have a trusted CA!
						smtp_server_policy);	// Test policy callback

	printf("Sending mail to %s via %s\n", SMTP_TO, smtp_getserver());
	smtp_sendmail(SMTP_TO, FROM, SUBJECT, BODY);

	while(smtp_mailtick()==SMTP_PENDING)
		continue;

	if(smtp_status()==SMTP_SUCCESS)
		printf("Message sent\n");
	else
		printf("Error sending message\n");
}
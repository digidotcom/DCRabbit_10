/*******************************************************************************
        smtpxmem.c
        Rabbit Semiconductor, 2000

        A small program that uses the SMTP library
        to send an e-mail.
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

/*
 *   These macros need to be changed to the appropriate values or
 *   the smtp_sendmail(...) call in main() needs to be changed to
 *   reference your values.
 */

#define SUBJECT  "Mail from smtpxmem.c!"

/*		For this sample, set the FROM and TO addresses for the email we'll
 *		be sending.  Uncomment and set both of these macros to your email address.
 */
//#define SMTP_FROM		"tester@example.com"
//#define SMTP_TO			"tester@example.com"

/*
 *   The SMTP_SERVER macro tells DCRTCP where your mail server is.  This
 *   mail server MUST be configured to relay mail for your controller.
 *
 *   Uncomment and set to the name or the IP address of your SMTP server.
 */

//#define SMTP_SERVER "10.10.6.1"
//#define SMTP_SERVER "mymailserver.mydomain.com"

/*
 *		The SMTP protocol runs on port 25 by default.
 *
 *		Some ISPs block outbound connections on port 25 (requiring clients to use
 *    to the ISP's SMTP server).  Some mail servers accept connections on
 *    port 587 (submission) if the client uses SMTP AUTH (see below).
 *
 *		If you need to use a port other than 25, uncomment and set it here.
 */
//#define SMTP_PORT 25

/*
 *   The SMTP_DOMAIN should be the name of your controller.  i.e.
 *   "somecontroller.somewhere.com"  Many SMTP servers ignore this
 *   value, but some SMTP servers use this field.  If you have
 *   problems, turn on the SMTP_DEBUG macro and see were it is
 *   bombing out.  If it is in the HELO command consult the
 *   person in charge of the mail server for the appropriate value
 *   for SMTP_DOMAIN. If you do not define this macro it defaults
 *   to the value in MY_IP_ADDRESS.
 *
 */

//#define SMTP_DOMAIN "mycontroller.mydomain.com"

/*
 *   The SMTP_VERBOSE macro logs the communications between the mail
 *   server and your controller.  Uncomment this define to begin
 *   logging
 */

//#define SMTP_VERBOSE

/*
 *   The USE_SMTP_AUTH macro enables SMTP Authentication, a method
 *   where the client authenticates with the server before sending
 *   a message.  Call smtp_setauth() before smtp_sendmail() to set
 *   the username and password to use for authentication.
 */
//#define USE_SMTP_AUTH
//#define SMTP_AUTH_USER "test@foo.bar"
//#define SMTP_AUTH_PASS "secret"

/*
 *   If the following macro is defined, then if SMTP authentication
 *   fails, the library will NOT attempt non-authenticated SMTP.
 */
//#define SMTP_AUTH_FAIL_IF_NO_AUTH

/********************************
 * End of configuration section *
 ********************************/

#ifndef SMTP_SERVER
	#error "You must define SMTP_SERVER to your server's IP or hostname."
#endif
#ifndef SMTP_FROM
	#error "You must define SMTP_FROM to a valid email address."
#endif
#ifndef SMTP_TO
	#error "You must define SMTP_TO to your email address."
#endif

#memmap xmem
#use dcrtcp.lib
#use smtp.lib

/*
 *		ximport takes a file from disk and associates it with a
 *		c symbol.  The c symbol evaluates to the physical address
 *    of the data. The first four bytes of the data are the
 *    length of the file followed directly by the length of the
 *    file.
 *
 *    To use smtp_sendmailxmem with RAM, xalloc a region, place
 *    the length of the data in the first four bytes and the
 *    data directly following it.  NOTE:  the data must not
 *    contain the string "\r\n.\r\n"
 *
 */

#ximport "samples/tcpip/smtp/smtp.txt" test_txt

void main()
{
	long length;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

#ifdef USE_SMTP_AUTH
	smtp_setauth (SMTP_AUTH_USER, SMTP_AUTH_PASS);
#endif

	xmem2root(&length,test_txt,sizeof(length));
	smtp_sendmailxmem(SMTP_TO, SMTP_FROM, SUBJECT, test_txt+4, length);

	while(smtp_mailtick()==SMTP_PENDING)
		continue;

	if((int)(length=smtp_status())==SMTP_SUCCESS)
		printf("Message sent\n");
	else
		printf("Error sending message : %x\n",(int)length);
}


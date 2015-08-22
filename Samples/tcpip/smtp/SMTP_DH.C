/*******************************************************************************
        smtp_dh.c
        Rabbit Semiconductor, 2000

        A small program that uses the SMTP library
        to send an e-mail.  This makes use of the smtp_data_handler() function
        introduced in DC 8.0 to generate message data on-the-fly.
*******************************************************************************/

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

#define SUBJECT  "Mail with data handler!"

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


int mail_generator(char * buf, int len, longword offset,
                                int flags, void * dhnd_data)
{
	auto int * my_data;

	if (flags == SMTPDH_OUT) {
		if (offset > 500)
			return 0;	// EOF after 500 bytes

		my_data = (int *)dhnd_data;	// Access our opaque data

		// Note that a '.' at the beginning of a line is deleted by the mail server.
		// Also, email normally requires lines to be terminated with \r\n not just \n.

		snprintf(buf, len,
      	".. Line %d: The quick brown fox jumps over the lazy dog.  %lu\r\n",
         *my_data, offset);

		(*my_data)++;	// Increment line number (data is not opaque to us!).

		return(strlen(buf));
	}
	return -1;	// Indicate don't understand or care about this flag
}


int main()
{
	int line_num;	// This is our data handler opaque parameter.

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

#ifdef USE_SMTP_AUTH
	smtp_setauth (SMTP_AUTH_USER, SMTP_AUTH_PASS);
#endif

	smtp_sendmail(SMTP_TO, SMTP_FROM, SUBJECT, NULL);	// No fixed message
	smtp_data_handler(mail_generator, &line_num, 0);	// Set message generator function

	line_num = 1;	// Initialize for data handler benefit.

	while(smtp_mailtick()==SMTP_PENDING)
		continue;

	if(smtp_status()==SMTP_SUCCESS)
		printf("Message sent\n");
	else
		printf("Error sending message\n");
	return 0;
}
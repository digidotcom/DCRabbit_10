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
/**********************************************************

   smtp.c

	This program is used with RCM56xxW series controllers with interface board.

	Description
	===========
   This program uses the SMTP library to send an e-mail
   when a switch on the interface board is pressed.

	DS1 LED on the interface board will light-up
	when sending mail.

   Use the following jumper placements on the interface board:

	I/O control       On Interface Board
	--------------    ------------------
	Port D bit 0		DS1, LED

   Jumper settings (Interface Board)
   ---------------------------------
   JP1   1-2 program mode
         5-6 enables DS1 (LED)
         7-8 enables S1  (button)

         2    4    6   8
         o    o    o   o
         |         |   |
         o    o    o   o
         1    3    5   7

	Test Instructions:
	------------------
	1. Compile and run this program.
   2. Press switch S1 on the prototyping board to send an email.

***********************************************************/
#use "rcm56xxw.lib"

// LED
#define DS1 0

// Switch
#define S1  1

#define LEDON	0
#define LEDOFF 1

// The structure that holds an email message
typedef struct {
	char *from;
	char *to;
	char *subject;
	char *body;
} Email;
Email emailArray[2];	// holds the email messages

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

/*		For this sample, set the FROM and TO addresses for the email we'll
 *		be sending.  Uncomment and set both of these macros to your email address.
 */
//#define SMTP_FROM		"tester@example.com"
//#define SMTP_TO			"tester@example.com"

/*
 *   The SMTP_DOMAIN should be the hostname of your controller.  i.e.
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
#use "dcrtcp.lib"
#use "smtp.lib"

// Send the email that corresponding to the alternating status of email.
void SendMail(int email)
{
	int rc;

	// Start sending the email
	smtp_sendmail(emailArray[email].to, 	  emailArray[email].from,
   	           emailArray[email].subject, emailArray[email].body);

	// Wait until the message has been sent
	while(smtp_mailtick() == SMTP_PENDING);

	// Check to see if the message was sent successfully
	if ((rc = smtp_status()) == SMTP_SUCCESS)
	{
		printf("\n\rMessage sent\n\r");
	}
	else
	{
		printf("\n\rError sending the email message :%x\n\r",rc);
	}
}

// Check the status of switch
cofunc void CheckSwitch()
{
	static message_number;

	if (BitRdPortI(PDDR, S1))						   // wait for switch press
   {
		abort;											   // if button not down skip out
   }
	waitfor(DelayMs(50));							   // wait 50 ms
	if (BitRdPortI(PDDR, S1))						   // wait for switch press
   {
		abort;											   // if button not still down exit
   }

	BitWrPortI(PDDR, &PDDRShadow, LEDON, DS1);	// led on
   message_number = !message_number;
	SendMail(message_number); 						   // send if button was down 50 ms
	BitWrPortI(PDDR, &PDDRShadow, LEDOFF, DS1);	// led off

	while (1)
	{
		waitfor(BitRdPortI(PDDR, S1));			   // wait for button to go up
		waitfor(DelayMs(200));						   // wait additional 200 ms
		if (BitRdPortI(PDDR, S1))					   // wait for switch press
      {
			break;										   // if button still up exit loop
      }
	}
}


// Check if the switch has been pressed
//   if so then a send email message.
void check_switch_status(void)
{
	costate
	{
		waitfordone { CheckSwitch(); }
	}
}

void init_emails(void)
{
	// Define emails here
	emailArray[0].from = SMTP_FROM;
	emailArray[0].to = SMTP_TO;
	emailArray[0].subject = "Self-Test status OK";
	emailArray[0].body = "Self-test completed, all tests passed.";

	emailArray[1].from = SMTP_FROM;
	emailArray[1].to = SMTP_TO;
	emailArray[1].subject = "System malfunction!";
	emailArray[1].body = "Temperature Sensor #3 failed.";
}

main()
{
	static int counter;

	brdInit();			//initialize board for this demo

	// Wait for interface to come up...
	sock_init_or_exit(1);

#ifdef USE_SMTP_AUTH
	smtp_setauth (SMTP_AUTH_USER, SMTP_AUTH_PASS);
#endif

	// initialize the email structure array with user defined emails
	init_emails();

	counter = 0;
	while(1)
	{
      // check if user has activated a switch to send an email
      check_switch_status();
	}
}


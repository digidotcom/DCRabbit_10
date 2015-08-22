/*******************************************************************************
   smtp.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
   This program uses the SMTP library to send an e-mail when a switch on
   the demo board is pressed.

	Connections:
	============
	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller GND, to the DEMO board GND.

	3. Connect a wire from the controller +5V to the DEMO board +5V.

   4. Connect the following wires from the controller to the DEMO board:
   		From IN0 to SW1
   		From IN1 to SW2
   		From IN2 to SW3
   		From IN3 to SW4

	Instructions:
	============:
	1. Compile and run this program.
	2. Press any one of the DEMO board switches SW1 - SW4 to send an email.

*******************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// default to storing functions to xmem instead of root
#memmap xmem

// The structure that holds an email message
typedef struct {
	char *from;
	char *to;
	char *subject;
	char *body;
} Email;
Email emailArray[4];	// holds the email messages

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

// Basic TCP/IP networking
#use "dcrtcp.lib"

// this sample uses functions from the SMTP library
#use "smtp.lib"

// Send the email that corresponds to the switch that was pressed.
void SendMail(int email)
{
	printf ("\nsending [%s] to %s...\n", emailArray[email].subject,
		emailArray[email].to);

	// Start sending the email
	smtp_sendmail(emailArray[email].to, 	  emailArray[email].from,
   	           emailArray[email].subject, emailArray[email].body);

	// Wait until the message has been sent
	while(smtp_mailtick()==SMTP_PENDING)
		continue;

	// Check to see if the message was sent successfully
	if(smtp_status()==SMTP_SUCCESS)
	{
		printf("\n\rMessage sent\n\r");
	}
	else
	{
		printf("\n\rError sending the email message\n\r");
	}
}

/*
 * Check the status of switch.
 * Use a cofunction array, with one function per switch.
 */
cofunc void CheckSwitch[4](int sw)
{
	if (digIn(sw)) abort;			// if button not down skip out
	waitfor(DelayMs(50));			// wait 50 ms
	if (digIn(sw)) abort;			// if button not still down exit

	SendMail(sw);						// send email since button was down 50 ms

	while (1) {
		waitfor(digIn(sw));			// wait for button to go up
		waitfor(DelayMs(200));		// wait additional 200 ms
		if (digIn(sw))
			break;		// if button still up break out of while loop
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
	emailArray[1].subject = "Self-Test failed.";
	emailArray[1].body = "Rocket booster failed";

	emailArray[2].from = SMTP_FROM;
	emailArray[2].to = SMTP_TO;
	emailArray[2].subject = "System shut down";
	emailArray[2].body = "The system has been shut down.";

	emailArray[3].from = SMTP_FROM;
	emailArray[3].to = SMTP_TO;
	emailArray[3].subject = "System malfunction";
	emailArray[3].body = "Temperature Sensor #3 failed.";
}


///////////////////////////////////////////////////////////////////////////

void main()
{
	static int counter;
   int channel;
   int i;

	// Initialize the controller
	brdInit();

   // set up digital inputs 0-3
	for (channel = 0; channel < 4; channel++) {
		setDigIn (channel);
   }

	sock_init_or_exit(1);

#ifdef USE_SMTP_AUTH
	// Set the username and password to use for SMTP Authentication, an access
   // control mechanism used by some SMTP servers to allow legitimate users to
   // relay mail.  (See <http://en.wikipedia.org/wiki/SMTP-AUTH> for a full
   // description).
	smtp_setauth (SMTP_AUTH_USER, SMTP_AUTH_PASS);
#endif

	// initialize the email structure array with user defined emails
	init_emails();

	printf ("Press buttons SW1 through SW4 on the demo board to send email from\n");
	printf ("%s to %s.\n\n", SMTP_FROM, SMTP_TO);

	counter = 0;
	while(1)
	{
		costate
		{  // check if user has activated a switch to send an email
			for (i = 0; i < 4; i++)
			{
	      	wfd { CheckSwitch[i](i); }
	      }
		}

      printf("\rCode execution counter = %d", counter++);
	}
}
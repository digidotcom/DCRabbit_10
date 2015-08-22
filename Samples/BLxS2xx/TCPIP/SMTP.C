/**********************************************************

   smtp.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

	Description
	===========
   This program uses the SMTP library to send an e-mail
   when a switch on an attached demo board is pressed.

	LED1 and LED2 on the demo board will light up when
	sending mail.

   Connections:
	============

	****WARNING****: When using the J7 connector, be sure to insulate or cut
      the exposed wire from the wire leads you are not using.  The only
      connection required from J7 for any of the sample programs is +5v.

	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller J10 pin 5 GND to the DEMO board
	   J1 GND.

	3. Connect a wire from the controller J7 pin 6 (+5V) to the DEMO board +V.

   4. Connect the following wires from the controller J10 to the DEMO
      board screw terminal:

      From J10 pin 9 DIO0 to LED1
      From J10 pin 4 DIO1 to LED2
      From J10 pin 7 DIO4 to SW1
      From J10 pin 2 DIO5 to SW2

	Test Instructions:
	------------------
	1. Compile and run this program.
	2. Press SW1 and SW2 on the demo board to	send an
	   email.

***********************************************************/
#use "BLxS2xx.lib"

// default to storing functions to xmem instead of root
#memmap xmem

#define LED1   0
#define LED2   1

#define SW1  4
#define SW2  5

#define LEDON	0
#define LEDOFF	1

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

// Basic TCP/IP networking
#use "dcrtcp.lib"

// this sample uses functions from the SMTP library
#use "smtp.lib"


/*
 * Send the email that corresponding to the switch that was
 * pressed.
 *
 */

void SendMail(int email)
{
	int rc;

	// Start sending the email
	smtp_sendmail(emailArray[email].to, 	  emailArray[email].from,
   	           emailArray[email].subject, emailArray[email].body);

	// Wait until the message has been sent
	while(smtp_mailtick()==SMTP_PENDING)
		continue;

	// Check to see if the message was sent successfully
	if((rc=smtp_status())==SMTP_SUCCESS)
	{
		printf("\n\rMessage sent\n\r");
	}
	else
	{
		printf("\n\rError sending the email message :%x\n\r",rc);
	}
}

/*
 * Check the status of switch 1
 */
cofunc void CheckSwitch1()
{
	if (digIn(SW1))									// wait for switch press
		abort;											// if button not down skip out
	waitfor(DelayMs(50));							// wait 50 ms
	if (digIn(SW1))									// wait for switch press
		abort;											// if button not still down exit

	digOut(LED1, LEDON);								// led on
	SendMail(0);										// send email since button was down 50 ms
	digOut(LED1, LEDOFF);							// led off

	while (1)
	{
		waitfor(digIn(SW1));							// wait for button to go up
		waitfor(DelayMs(200));						// wait additional 200 ms
		if (digIn(SW1))								// wait for switch press
			break;										// if button still up break out of while loop
	}
}


/*
 * Check the status of switch 2
 */
cofunc void CheckSwitch2()
{
	if (digIn(SW2))									// wait for switch press
		abort;											// if button not down skip out
	waitfor(DelayMs(50));							// wait 50 ms
	if (digIn(SW2))									// wait for switch press
		abort;											// if button not still down exit

	digOut(LED2, LEDON);								// led on
	SendMail(1);										// send email since button was down 50 ms
	digOut(LED2, LEDOFF);							// led off

	while (1)
	{
		waitfor(digIn(SW2));							// wait for button to go up
		waitfor(DelayMs(200));						// wait additional 200 ms
		if (digIn(SW2))								// wait for switch press
			break;										// if button still up break out of while loop
	}
}


/*
 *
 *		Check if any of the switches have been pressed
 *    if so then a send email message.
 *
 */
void check_switch_status(void)
{
	costate
	{
		waitfordone { CheckSwitch1(); }
	}
	costate
	{
		waitfordone { CheckSwitch2(); }
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


///////////////////////////////////////////////////////////////////////////
main()
{
	static int counter;


	brdInit();			//initialize board for this demo

   // Configure IO channels as digital outputs (sinking type outputs)
   setDigOut (LED1, 1);				// Configure LED1 as sinking type output
   setDigOut (LED2, 1);				// Configure LED2 as sinking type output

   // Set the initial state of the LED's
   digOut(LED1, LEDOFF);
   digOut(LED2, LEDOFF);

	// Configure IO channels as digital inputs
	setDigIn(SW1);
	setDigIn(SW2);

	// Wait for interface to come up...
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

	printf ("Press buttons SW1 or SW2 on the demo board to send email from\n");
	printf ("%s to %s.\n\n", SMTP_FROM, SMTP_TO);

	counter = 0;
	while(1)
	{
		costate
		{  // check if user has activated a switch to send an email
			check_switch_status();
		}
		costate
		{
			printf("\rCode execution counter = %d", counter++);
		}
	}
}


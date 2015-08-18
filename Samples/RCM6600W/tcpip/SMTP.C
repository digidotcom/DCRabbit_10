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

	This program is used with RCM66xxW series controllers with interface board.

   The RCM6600W has two network interfaces, WiFi and Ethernet.
   By default, both are enabled, however you can disable either by
   adding one of the following macros in the project->defines box:
     DISABLE_ETHERNET
     DISABLE_WIFI
   If you are running this sample with both interfaces enabled, and with
   a static (non-DHCP) network configuration, then you will need to
   define a primary and secondary IP address as follows (in the project
   defines):
	   _PRIMARY_STATIC_IP="10.10.6.100"
	   _PRIMARY_NETMASK="255.255.255.0"
	   _SECONDARY_STATIC_IP="10.66.66.66"
	   _SECONDARY_NETMASK="255.255.0.0"
	   MY_GATEWAY="10.10.6.1"
	   MY_NAMESERVER="200.100.50.25"
	(Changing the numeric addresses as appropriate).  The primary
	address gets assigned to the Ethernet interface, and the
	secondary to the WiFi interface.

	It's much easier to use DHCP.  In this case, just change the
	TCPCONFIG definition (below, in this sample) to 5, and everything
	should "just work".

	With dual interfaces, you should be able to access this board from
	either network.  When run, the IP addresses of both interfaces
	will be printed on the stdio window.

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
#use "rcm66xxw.lib"

// LED
#define DS1 0

// Switch
#define S1  1

#define LEDON	0
#define LEDOFF 1

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

// The structure that holds an email message
typedef struct {
	char *from;
	char *to;
	char *subject;
	char *body;
} Email;
Email emailArray[2];	// holds the email messages

// NETWORK CONFIGURATION
// Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
// compile-time network configuration.
#define TCPCONFIG 1


//   These macros need to be changed to the appropriate values.
 #define EMAILTO "me@somewhere.com"
#define EMAILFROM "you@anywhere.com"


//   The SMTP_SERVER macro tells DCRTCP where your mail server is.  This
//   mail server MUST be configured to relay mail for your controller.

//   This value can be the name or the IP address.
// Uncomment and define the following to match your network.
//#define SMTP_SERVER "10.40.6.1"

/*
 *   The SMTP_DOMAIN should be the name of your controller.  i.e.
 *   "somecontroller.somewhere.com"  Many SMTP servers ignore this
 *   value, but some SMTP servers use this field.  If you have
 *   problems, turn on the SMTP_DEBUG macro and see where it is
 *   failing.  If it is in the HELO command consult the
 *   person in charge of the mail server for the appropriate value
 *   for SMTP_DOMAIN. If you do not define this macro it defaults
 *   to the value in MY_IP_ADDRESS.
 *
 */

//#define SMTP_DOMAIN "mycontroller.mydomain.com"

//   The SMTP_VERBOSE macro logs the communications between the mail
//   server and your controller.
#define SMTP_VERBOSE

/********************************
 * End of configuration section *
 ********************************/


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
	emailArray[0].from = EMAILFROM;
	emailArray[0].to = EMAILTO;
	emailArray[0].subject = "Self-Test status OK";
	emailArray[0].body = "Self-test completed, all tests passed.";

	emailArray[1].from = EMAILFROM;
	emailArray[1].to = EMAILTO;
	emailArray[1].subject = "System malfunction!";
	emailArray[1].body = "Temperature Sensor #3 failed.";
}

main()
{
	static int counter;

	brdInit();			//initialize board for this demo

	// Wait for interface to come up...
	sock_init_or_exit(1);

	// initialize the email structure array with user defined emails
	init_emails();

	counter = 0;
	while(1)
	{
      // check if user has activated a switch to send an email
      check_switch_status();
	}
}


/*******************************************************************************
        LP_SMTP.c
        Digi International, Copyright (C) 2005-2009.  All rights reserved.

        This program demonstrates Low Power mode while using TCP/IP
        networking to send e-mails upon request.

        This sample does not run on WiFi core modules. See powerdown.c
        sample in samples/RCMxxxx for a sample that toggles power to the
        WiFi interface.

        When it is run, it will start off in minimum power (Mode 10) until
        Switch S2 (or "S1" on the RCM57xx) is pressed by the user.

        When the switch is pressed, the board switches to full power
        (Mode 1), with CPU working at full speed, and an e-mail message
        is sent. After the message has been sent, the board switches back
        to minimal power usage (Mode 10) with the CPU running at only 2kHz.

      For a detailed description of the above power modes, see function
      description for set_cpu_power_mode() in LOW_POWER.LIB.

        Note: When running in Low power mode, LED is programmed to blink
         to indicate that board is still operating.  This program will not
         run in debugging mode with program cable is connected.
*******************************************************************************/
#class auto

#define LOWPOWER_VERBOSE // Debugging options

/* Define HIT_WATCHDOG if watchdog is enabled (default) while using
*  power modes 6 through 10 (ie when periodic interrupt is disabled).
*/
#define HIT_WATCHDOG

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

#define SUBJECT  "You've got mail!"
#define BODY     "Visit the Rabbit Semiconductor web site.\r\n" \
	"There you'll find the latest news about Dynamic C."

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

/* The following defines the macros SWITCH_PRESSED() and SET_LED(), based on
 * which Rabbit prototyping board is being used. SWITCH_PRESSED() returns 1
 * if the switch "S2" (or "S1" on the RCM57xx) is pressed
 * on the prototyping board and 0 otherwise.
 */
#if (RCM4000_SERIES)
   #use "rcm40xx.lib"
   // We use Switch 2 (S2), if using the RCM4xxx Prototyping Board.
   #define  SWITCH_PRESSED()   !BitRdPortI(PBDR, 4)
   #define  SET_LED(on)        BitWrPortI(PBDR, &PBDRShadow, !on, 2)
#elif (RCM4100_SERIES)
   #use "rcm41xx.lib"
   // We use Switch 2 (S2), if using the RCM4xxx Prototyping Board.
   #define  SWITCH_PRESSED()   !BitRdPortI(PBDR, 4)
   #define  SET_LED(on)        BitWrPortI(PBDR, &PBDRShadow, !on, 2)
#elif (RCM4200_SERIES)
   #use "rcm42xx.lib"
   // We use Switch 2 (S2), if using the RCM4xxx Prototyping Board.
   #define  SWITCH_PRESSED()   !BitRdPortI(PBDR, 4)
   #define  SET_LED(on)        BitWrPortI(PBDR, &PBDRShadow, !on, 2)
#elif (RCM4300_SERIES)
   #use "rcm43xx.lib"
   // We use Switch 2 (S2), if using the RCM4xxx Prototyping Board.
   #define  SWITCH_PRESSED()   !BitRdPortI(PBDR, 4)
   #define  SET_LED(on)        BitWrPortI(PBDR, &PBDRShadow, !on, 2)
#elif (RCM4400W_SERIES || RCM5400W_SERIES || RCM5600W_SERIES)
   #fatal "This sample does not work on Wi-Fi based core modules."
#elif (RCM5700_SERIES)
   #use "rcm57xx.lib"
   // We use Switch 1 (S1), if using the RCM57xx Interface Board.
   #define  SWITCH_PRESSED()   !BitRdPortI(PDDR, 1)
   #define  SET_LED(on)        BitWrPortI(PDDR, &PDDRShadow, !on, 0)
#else
   #error "This board type does not have a switch attached to general purpose"
   #fatal "I/O.  It is not supported by this sample."
#endif

#memmap xmem
#use dcrtcp.lib
#use smtp.lib
#use "low_power.lib"

void main()
{
	auto int powerMode;
   auto unsigned long delayStart;
   auto char ledState;

   brdInit();

	ledState = 0; //used to blink LED light on and off

	sock_init();
	// Wait for the interface to come up
	while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
		tcp_tick(NULL);
	}

   //Bring down TCP/IP interface in preparation for Low Power mode.
   ifconfig(IF_DEFAULT, IFS_DOWN, IFS_END);
   //...and power down Ethernet controller.
   pd_powerdown(IF_DEFAULT);
   /* Set Rabbit to work under minimum power consumption mode (Mode 10)
    * until user presses Switch 2 (Switch 1 on RCM57xx boards) */
   powerMode = 10;
   set_cpu_power_mode(powerMode, CLKDOUBLER_OFF, SHORTCS_OFF);

#ifdef USE_SMTP_AUTH
	// Set the username and password to use for SMTP Authentication, an access
   // control mechanism used by some SMTP servers to allow legitimate users to
   // relay mail.  (See <http://en.wikipedia.org/wiki/SMTP-AUTH> for a full
   // description).
	smtp_setauth (SMTP_AUTH_USER, SMTP_AUTH_PASS);
#endif

   while(1)
   {
 		costate
		{
			if (!SWITCH_PRESSED())	 //wait for switch S2 press
				abort;                // (S1 on RCM57xx boards)

         while(SWITCH_PRESSED())  //wait for release
         	yield;

			powerMode = 1;	 //set power mode to work at full cpu speed
         set_cpu_power_mode(powerMode, CLKDOUBLER_ON, SHORTCS_OFF);

         //power up Ethernet controller
			pd_powerup(IF_DEFAULT);

         //wait 1 second for ethernet to go back up
         delayStart = MS_TIMER;
         while(MS_TIMER - delayStart < 1000L);

         //Re-enable TCP/IP interface.
         ifconfig(IF_DEFAULT, IFS_UP, IFS_END);
			// Wait for the interface to come back up
  			while (ifpending(IF_DEFAULT) == IF_COMING_UP) {
				tcp_tick(NULL);
  			}
		} //end costate

 	   if(powerMode == 1)
      {
	      //switch press detected if got here, send an email
	      smtp_sendmail(SMTP_TO, SMTP_FROM, SUBJECT, BODY);

	      while(smtp_mailtick()==SMTP_PENDING)
	         continue;

	      if(smtp_status()==SMTP_SUCCESS)
	         printf("Message sent\n");
	         else
	      printf("Error sending message\n");

         //Switch back to lowest power consumption mode after sending e-mail
         //Bring down TCP/IP interface.
         ifconfig(IF_DEFAULT, IFS_DOWN, IFS_END);
         //...and power down Ethernet controller.
         pd_powerdown(IF_DEFAULT);
         //Set power back to 10 (Minimum power consumption)
         powerMode = 10;
         set_cpu_power_mode(powerMode, CLKDOUBLER_OFF, SHORTCS_OFF);
      }

      /* CPU is running in 2-32kHZ range here, so
   	 * blink the LED light to indicate that we are
       * still running. */
      if(powerMode > 5)
      {
	   	ledState ^= 1;
	   	SET_LED(ledState);
      }

   /* For cases where the watchdog timer is enabled, and
    * we are operating in a power mode where periodic interrupts
    * are disabled, we also need to hit the watchdog promptly */
#ifdef HIT_WATCHDOG
  		if(powerMode > 5)
   	{
         //hit watchdog
  			hitwd();
         /* If we also use timers, use this opportunity to update
          * timers, such as MS_TIMER, etc.  Uncomment if using timers
          * during low power mode (modes 6-10)  */
        	//updateTimers();

		}
#endif
   } //end loop
}
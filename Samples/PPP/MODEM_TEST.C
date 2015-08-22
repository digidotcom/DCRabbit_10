/*****

   modem_test.c

   Tests the PPP interface, with modem attached to serial port C (default).
   Brings the PPP interface up and down three times, each time connecting to the
   ISP and sending a test email.

   You will need to change the default macro definitions for DIALUP_NAME,
   DIALUP_NUMBER, DIALUP_PASSWORD, EMAIL_FROM, EMAIL_TO and SMTP_SERVER.

   You also may need to alter the choice of serial port, whether or not CTS/RTS
   flow control is enabled via the DIALUP_FLOWCONTROL macro and the choice of
   flow control (CTS, RTS) I/O bits and ports, and the send/expect sequence
   defined in the DIALUP_SENDEXPECT macro.

   This sample assumes you are using hardware modem flow control i.e. CTS and
   RTS.  The CTS port and bit number are defined in the IFS_PPP_CTSPIN
   parameter, defaulting to parallel port C, bit 1.  Likewise, RTS is set via
   IFS_PPP_RTSPIN and defaults to parallel port C, bit 0.

   Changing either or both I/O bits and ports used for CTS / RTS is simply a
   matter of updating the MY_xTS_BIT and MY_xTS_PORT macro definitions
   appropriately.

   Note that no real checking of these CTS / RTS macro definitions is performed,
   so it is entirely possible to make a non-working choice.  In fact, the
   default selections are not appropriate for some Rabbit boards.  Please check
   your hardware manual in order to make choices that are appropriate for your
   board.

   When using a Rabbit SBC or an RCM on a protyping board, it can be convenient
   to choose the CTS / RTS flow control lines based on a second RS-232 port, if
   available.  In many cases, the second RS-232 serial port will already have an
   RS-223 tranceiver chip installed, saving some time and effort.  The second
   serial port's RX line is repurposed as the CTS, and the TX line as the RTS.
   The standard I/O pins used for serial ports are:
      RXA (CTS) on PC7
      TXA (RTS) on PC6
      RXB (CTS) on PC5
      TXB (RTS) on PC4
      RXC (CTS) on PC3
      TXC (RTS) on PC2
      RXD (CTS) on PC1
      TXD (RTS) on PC0
      RXE (CTS) on PG7 (Rabbit 3000) or PC7 (Rabbit 4000)
      TXE (RTS) on PG6 (Rabbit 3000) or PC6 (Rabbit 4000)
      RXF (CTS) on PG3 (Rabbit 3000) or PC3 (Rabbit 4000)
      TXF (RTS) on PG2 (Rabbit 3000) or PC2 (Rabbit 4000)

   It is important to ensure that your modem is set up correctly for your choice
   of CTS/RTS hardware flow control enabled vs. disabled on the Rabbit board.
   Please consult your modem's manual to determine what its setup requirements
   are for your choice of flow control enabled vs. disabled.

********/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 0				// No predefined config for this sample

#define DIALUP_FLOWCONTROL 1	// 0 to disable, 1 to enable CTS/RTS flow control
#define DIALUP_NAME "username"
#define DIALUP_NUMBER "5551212"
#define DIALUP_PASSWORD "password"

// select the PPP serial port, as appropriate for the target board
//  defining MY_PPP_SERIAL_PORT to 1 selects A, 2 selects B, ..., 6 selects F
#define MY_PPP_SERIAL_PORT 3	// our default is serial port C

// select the CTS and RTS bits and ports
// our defaults are PC1 (standard RXD) and PC0 (standard TXD), respectively
#define MY_CTS_BIT 1
#define MY_CTS_PORT PCDR
#define MY_RTS_BIT 0
#define MY_RTS_PORT PCDR


//Uncomment to get PPP detail
//#define PPP_VERBOSE
//#define PPP_DEBUG
//#define PPPLINK_VERBOSE
//#define PPPLINK_DEBUG
//#define SMTP_VERBOSE
//#define SMTP_DEBUG 1
#define CHAT_VERBOSE				// See what's going on.
//#define CHAT_DEBUG

// The following macro sets up the initial login screen navigation.  This is
// necessary for access to many ISPs, since when you dial in with the modem, the
// ISP does user authentication via an old-fashioned login screen.
//
// %0 gets userid substituted, %1 is password -- obtained from values passed for
// IFS_PPP_REMOTEAUTH.  The string below gets compiled to
// "ATZ #ok @ATDT5551212 #CONNECT '' #ogin: @%0 #word: @%1 ~"
//
// This is a send/expect sequence used to establish a modem connection to the
// ISP (ready for PPP to take over).  Expanded out:
//  ATZ         - send the modem reset command (usually default config 0).
//                This helps get the modem into a known initial state.
//  #ok         - wait for 'ok' string (the leading '#' means case-insensitive
//                i.e. will accept 'OK', 'Ok' etc.
//  @ATDT5551212 - After 1.5 seconds pause, send the dialout command. The pause
//                allows time for the modem's reset to complete for those modems
//                which respond with an early 'ok' to the prior reset command.
//  #CONNECT    - wait for 'connect' message
//  ''          - Send nothing (quotes are a place-holder).  Note that the CRLF
//                is appended to any send-string _except_ an empty string like
//                this (but you can suppress the CRLF by prefixing the
//                send-string with '#').
//  #ogin:      - Wait for a 'login:' message.  We don't actually look for the
//                initial 'L' since the first character can sometimes be lost.
//  @%0         - send the userid (i.e. the value supplied by the
//                IFS_PPP_REMOTEAUTH parameter.  If the character-based login
//                prompt expects a different userid than the one passed to the
//                following PPP authentication phase, then you can't use %0.  In
//                this case, put the character-based login ID directly in the
//                send/expect string.  The initial '@' character causes us to
//                pause for 1.5 seconds before sending the string.  This is not
//                theoretically required, however some ISPs have software which
//                tries to discourage machine-based hacking attempts.  If we
//                respond too quickly, the ISP thinks we are typing at inhumanly
//                fast rates, and deems us to be a hacker.
//  #word:      - Wait for the 'password:' prompt.
//  @%1         - Send the password string.  See considerations above for the
//                logon ID string.
//  ~           - Wait for ASCII tilde char (0x7E).  This is handy, since this
//                is the first character sent by PPP when it starts up on the
//                peer.  Some peers send an ascii 'PPP' string, but this is not
//                usually so reliable as a means of detecting PPP startup.  Both
//                methods are provided in the choice of send / expect macros,
//                below.  If one doesn't work (i.e. CHAT: times out waiting for
//                the '~' or the 'PPP') try the other macro definition.
// Comment out exactly one of the following two send / expect macro definitions.
// It may be necessary to edit these send / expect macro definitions to suit.
#define DIALUP_SENDEXPECT "ATZ #ok @ATDT" DIALUP_NUMBER " #CONNECT '' #ogin: @%0 #word: @%1 ~"
//#define DIALUP_SENDEXPECT "ATZ #ok @ATDT" DIALUP_NUMBER " #CONNECT '' #ogin: @%0 #word: @%1 PPP"

#if DIALUP_FLOWCONTROL
	#define DIALUP_SPEED 57600L	// higher serial rate if flow control enabled
#else
	#define DIALUP_SPEED 19200L	// lower serial rate if flow control disabled
#endif

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


// set up the selected PPP serial port
#if 1 == MY_PPP_SERIAL_PORT
	// for PPP on serial port A
	#warnt "Choosing serial port A disallows debugging via the programming port."
	#define USE_PPP_SERIAL 0x01
#elif 2 == MY_PPP_SERIAL_PORT
	// for PPP on serial port B
	#define USE_PPP_SERIAL 0x02
#elif 3 == MY_PPP_SERIAL_PORT
	// for PPP on serial port C
	#define USE_PPP_SERIAL 0x04
#elif 4 == MY_PPP_SERIAL_PORT
	// for PPP on serial port D
	#define USE_PPP_SERIAL 0x08
#elif 5 == MY_PPP_SERIAL_PORT && _CPU_ID_ >= R3000
	// for PPP on serial port E
	#define USE_PPP_SERIAL 0x10
#elif 6 == MY_PPP_SERIAL_PORT && _CPU_ID_ >= R3000
	// for PPP on serial port F
	#define USE_PPP_SERIAL 0x20
#else
	#error "Invalid PPP serial port selection!"
#endif

#ifndef PFDR
	#define PFDR 0
#endif
#ifndef PGDR
	#define PGDR 0
#endif

#if PADR == MY_CTS_PORT
	// parallel port A
 #if MY_CTS_PORT == MY_RTS_PORT
	#error "Parallel port A can't be shared between inputs and outputs."
 #endif
	#define MY_CTS_BIT_SETUP WrPortI(SPCR, &SPCRShadow, 0x80);
#elif PBDR == MY_CTS_PORT
	// parallel port B
	#define MY_CTS_BIT_SETUP BitWrPortI(PBDDR, &PBDDRShadow, 0, MY_CTS_BIT);
#elif PCDR == MY_CTS_PORT
	// parallel port C
	#define MY_CTS_BIT_SETUP BitWrPortI(PCFR, &PCFRShadow, 0, MY_CTS_BIT);
	                         // no DDR setup is possible
#elif PDDR == MY_CTS_PORT
	// parallel port D
	#define MY_CTS_BIT_SETUP BitWrPortI(PDFR, &PDFRShadow, 0, MY_CTS_BIT); \
	                         BitWrPortI(PDDDR, &PDDDRShadow, 0, MY_CTS_BIT);
#elif PEDR == MY_CTS_PORT
	// parallel port E
	#define MY_CTS_BIT_SETUP BitWrPortI(PEFR, &PEFRShadow, 0, MY_CTS_BIT); \
	                         BitWrPortI(PEDDR, &PEDDRShadow, 0, MY_CTS_BIT);
#elif PFDR && PFDR == MY_CTS_PORT
	// parallel port F
	#define MY_CTS_BIT_SETUP BitWrPortI(PFFR, &PFFRShadow, 0, MY_CTS_BIT); \
	                         BitWrPortI(PFDDR, &PFDDRShadow, 0, MY_CTS_BIT);
#elif PGDR && PGDR == MY_CTS_PORT
	// parallel port G
	#define MY_CTS_BIT_SETUP BitWrPortI(PGFR, &PGFRShadow, 0, MY_CTS_BIT); \
	                         BitWrPortI(PGDDR, &PGDDRShadow, 0, MY_CTS_BIT);
#else
	#error "Invalid MY_CTS_PORT selection!"
#endif

#if PADR == MY_RTS_PORT
	// parallel port A
	#define MY_RTS_BIT_SETUP BitWrPortI(PADR, &PADRShadow, 0, MY_RTS_BIT); \
	                         WrPortI(SPCR, &SPCRShadow, 0x84);
#elif PBDR == MY_RTS_PORT
	// parallel port B
	#define MY_RTS_BIT_SETUP BitWrPortI(PBDR, &PBDRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PBDDR, &PBDDRShadow, 1, MY_RTS_BIT);
#elif PCDR == MY_RTS_PORT
	// parallel port C
	#define MY_RTS_BIT_SETUP BitWrPortI(PCFR, &PCFRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PCDR, &PCDRShadow, 0, MY_RTS_BIT);
	                         // no DDR setup is possible
#elif PDDR == MY_RTS_PORT
	// parallel port D
	#define MY_RTS_BIT_SETUP BitWrPortI(PDFR, &PDFRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PDDR, &PDDRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PDDDR, &PDDDRShadow, 1, MY_RTS_BIT);
#elif PEDR == MY_RTS_PORT
	// parallel port E
	#define MY_RTS_BIT_SETUP BitWrPortI(PEFR, &PEFRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PEDR, &PEDRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PEDDR, &PEDDRShadow, 1, MY_RTS_BIT);
#elif PFDR && PFDR == MY_RTS_PORT
	// parallel port F
	#define MY_RTS_BIT_SETUP BitWrPortI(PFFR, &PFFRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PFDR, &PFDRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PFDDR, &PFDDRShadow, 1, MY_RTS_BIT);
#elif PGDR && PGDR == MY_RTS_PORT
	// parallel port G
	#define MY_RTS_BIT_SETUP BitWrPortI(PGFR, &PGFRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PGDR, &PGDRShadow, 0, MY_RTS_BIT); \
	                         BitWrPortI(PGDDR, &PGDDRShadow, 1, MY_RTS_BIT);
#else
	#error "Invalid MY_RTS_PORT selection!"
#endif

#memmap xmem
#use "dcrtcp.lib"
#use "smtp.lib"


#define TIMEZONE        -8

#define LCP_TIMEOUT 5000

const char mail_to[] = SMTP_TO;
const char mail_from[] = SMTP_FROM;
const char mail_subject[] = "Mail from Rabbit";
const char mail_body[] = "This is a test message sent by a Rabbit through an ISP.";


int main()
{
	auto unsigned long t;
	auto char buffer[100];
	auto int mail_status;
	auto int count;
	auto int if_status;

	MY_CTS_BIT_SETUP	// set up the CTS handshake input
	MY_RTS_BIT_SETUP	// set up the RTS handshake output

	sock_init(); //initialize TCP/IP

#ifdef USE_SMTP_AUTH
	// Set the username and password to use for SMTP Authentication, an access
   // control mechanism used by some SMTP servers to allow legitimate users to
   // relay mail.  (See <http://en.wikipedia.org/wiki/SMTP-AUTH> for a full
   // description).
	smtp_setauth (SMTP_AUTH_USER, SMTP_AUTH_PASS);
#endif

	//test repeated open/close of modem link
	for(count = 0; count < 3; count++)
	{

	   //configure PPP for dialing in to ISP and bring it up
	   ifconfig(IF_DEFAULT,
	            IFS_PPP_SPEED, DIALUP_SPEED,
	            // Note: the NULL is for the shadow register.  From DC 9.0 on,
	            //       this is computed automatically - the value passed here
	            //       for the shadow register is ignored.
	            IFS_PPP_RTSPIN, MY_RTS_PORT, NULL, MY_RTS_BIT,
	            IFS_PPP_CTSPIN, MY_CTS_PORT, MY_CTS_BIT,
	            IFS_PPP_FLOWCONTROL, DIALUP_FLOWCONTROL,
	            IFS_PPP_SENDEXPECT, DIALUP_SENDEXPECT,
	            IFS_PPP_HANGUP, "ATH #ok",
	            IFS_PPP_MODEMESCAPE, 1,
	            IFS_PPP_ACCEPTIP, 1,
	            IFS_PPP_ACCEPTDNS, 1,
	            IFS_PPP_REMOTEAUTH, DIALUP_NAME, DIALUP_PASSWORD,
	            IFS_UP,
	            IFS_END);

		while (IF_COMING_UP == (if_status = ifpending(IF_DEFAULT)) ||
		       IF_COMING_DOWN == if_status)
	      tcp_tick(NULL);

	   if(ifstatus(IF_DEFAULT))
	      printf("PPP established\n");
	   else
	      printf("PPP failed\n");

	   ifconfig(IF_DEFAULT, IFG_IPADDR, &t, IFS_END);
	   printf("IP address is %s\n", inet_ntoa( buffer, t));

	   smtp_sendmail(mail_to, mail_from, mail_subject, mail_body);

	   while (SMTP_PENDING == (mail_status = smtp_mailtick()));

	   if(mail_status == SMTP_SUCCESS)
	      printf("Message sent\n");
	   else
	      printf("Failed to send message\n");

	   ifconfig(IF_DEFAULT, IFS_DOWN, IFS_END);

	   //wait while PPP terminates
		while (IF_COMING_DOWN == (if_status = ifpending(IF_DEFAULT)) ||
		       IF_COMING_UP == if_status)
	      tcp_tick(NULL);

	} //end of for loop

   return 0;
}
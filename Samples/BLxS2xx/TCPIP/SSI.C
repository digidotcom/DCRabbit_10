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

	ssi.c

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
   This program demonstrates using a WEB page to control LED's from the
   BLxS2xx series controller board. Users can browse to the controller,
   and change the status of the LEDs to match the indicators displayed
   on the WEB page.

   Connections:
	============

	****WARNING****: When using the J7 connector, be sure to insulate or cut
      the exposed wire from the wire leads you are not using.  The only
      connection required from J7 for any of the sample programs is +5v.

	1. DEMO board jumper settings:
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
      From J10 pin 8 DIO2 to LED3
      From J10 pin 3 DIO3 to LED4

	Instructions:
	=============
   1. Get help (ctrl-H) on TCPCONFIG for instructions on setting up
      networking for this server sample.
   2. Compile and run this program.
   3. With your WEB browser access the WEB page, using the URL shown.
   4. View LEDS on Web page and the controller to see that they match
      when changing them via the WEB page control button.

*******************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// MACROS for the LED's
#define ON     0
#define OFF    1

#define LED1   0
#define LED2   1
#define LED3   2
#define LED4   3


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
 * TCP/IP modification - reduce TCP socket buffer
 * size, to allow more connections. This can be increased,
 * with increased performance, if the number of sockets
 * are reduced.  Note that this buffer size is split in
 * two for TCP sockets--1024 bytes for send and 1024 bytes
 * for receive.
 */
#define TCP_BUF_SIZE 2048

/*
 * Web server configuration
 */

/*
 * only one socket and server are needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1


/*
 * Our web server as seen from the clients.
 * This should be the address that the clients (netscape/IE)
 * use to access your server. Usually, this is your IP address.
 * If you are behind a firewall, though, it might be a port on
 * the proxy, that will be forwarded to the Rabbit board. The
 * commented out line is an example of such a situation.
 */

#define REDIRECTHOST  _PRIMARY_STATIC_IP
//#define REDIRECTHOST	"proxy.domain.com:1212"

/********************************
 * End of configuration section *
 ********************************/

/*
 *  REDIRECTTO is used by each ledxtoggle cgi's to tell the
 *  browser which page to hit next.  The default REDIRECTTO
 *  assumes that you are serving a page that does not have
 *  any address translation applied to it.
 *
 */

#define REDIRECTTO  "http://" REDIRECTHOST "/"

#memmap xmem
#use "BLxS2xx.lib"
#use "dcrtcp.lib"
#use "http.lib"

/*
 *  Notice that we have ximported in the source code for
 *  this program.  This allows us to <!--#include file="ssi.c"-->
 *  in the pages/showsrc.shtml.
 *
 */

#ximport "samples/BLxS2xx/tcpip/pages/ssi.shtml"       index_html
#ximport "samples/BLxS2xx/tcpip/pages/rabbit1.gif"     rabbit1_gif
#ximport "samples/BLxS2xx/tcpip/pages/ledon.gif"       ledon_gif
#ximport "samples/BLxS2xx/tcpip/pages/ledoff.gif"      ledoff_gif
#ximport "samples/BLxS2xx/tcpip/pages/button.gif"      button_gif
#ximport "samples/BLxS2xx/tcpip/pages/showsrc.shtml"   showsrc_shtml
#ximport "samples/BLxS2xx/tcpip/ssi.c"                 ssi_c

/*
 *  In this case the .html is not the first type in the
 *  type table.  This causes the default (no extension)
 *  to assume the shtml_handler.
 *
 */

/* the default for / must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".shtml", "text/html", shtml_handler),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END


/*
 *  Each led_LEDx contains a text string that is either
 *  "ledon.gif" or "ledoff.gif"  This string is toggled
 *  each time the led_LEDx.cgi is requested from the
 *  browser.
 *
 */

char led_LED1[15];
char led_LED2[15];
char led_LED3[15];
char led_LED4[15];

/*
 *  Instead of sending other text back from the cgi's
 *  we have decided to redirect them to the original page.
 *  the cgi_redirectto forms a header which will redirect
 *  the browser back to the main page.
 *
 */

int led_toggle1(HttpState* state)
{
   if (strcmp(led_LED1,"ledon.gif")==0)
   {
      strcpy(led_LED1,"ledoff.gif");
      digOut(LED1, OFF);
   }
   else
   {
      strcpy(led_LED1,"ledon.gif");
      digOut(LED1, ON);
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led_toggle2(HttpState* state)
{
   if (strcmp(led_LED2,"ledon.gif")==0)
   {
      strcpy(led_LED2,"ledoff.gif");
      digOut(LED2, OFF);
   }
   else
   {
      strcpy(led_LED2,"ledon.gif");
      digOut(LED2, ON);
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led_toggle3(HttpState* state)
{
   if (strcmp(led_LED3,"ledon.gif")==0)
   {
      strcpy(led_LED3,"ledoff.gif");
      digOut(LED3, OFF);
   }
   else
   {
      strcpy(led_LED3,"ledon.gif");
      digOut(LED3, ON);
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led_toggle4(HttpState* state)
{
   if (strcmp(led_LED4,"ledon.gif")==0)
   {
      strcpy(led_LED4,"ledoff.gif");
      digOut(LED4, OFF);
   }
   else
   {
      strcpy(led_LED4,"ledon.gif");
      digOut(LED4, ON);
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}


SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/showsrc.shtml", showsrc_shtml),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledon.gif", ledon_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledoff.gif", ledoff_gif),
	SSPEC_RESOURCE_XMEMFILE("/button.gif", button_gif),
	SSPEC_RESOURCE_XMEMFILE("/ssi.c", ssi_c),
	SSPEC_RESOURCE_ROOTVAR("led_LED1", led_LED1, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led_LED2", led_LED2, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led_LED3", led_LED3, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led_LED4", led_LED4, PTR16, "%s"),
	SSPEC_RESOURCE_FUNCTION("/led_LED1.cgi", led_toggle1),
	SSPEC_RESOURCE_FUNCTION("/led_LED2.cgi", led_toggle2),
	SSPEC_RESOURCE_FUNCTION("/led_LED3.cgi", led_toggle3),
	SSPEC_RESOURCE_FUNCTION("/led_LED4.cgi", led_toggle4)
SSPEC_RESOURCETABLE_END



void main()
{
	unsigned long ip;
	char ipbuf[16];

   // initialize the controller
   brdInit();

   // Configure IO channels as digital outputs (sinking type outputs)
   setDigOut (LED1, 1);				// Configure LED1 as sinking type output
   setDigOut (LED2, 1);				// Configure LED2 as sinking type output
   setDigOut (LED3, 1);				// Configure LED3 as sinking type output
   setDigOut (LED4, 1);				// Configure LED4 as sinking type output

   sock_init();
   http_init();
   tcp_reserveport(80);

   // set the initial state of the LED's
   digOut(LED1, ON);
   digOut(LED2, ON);
   digOut(LED3, ON);
   digOut(LED4, ON);
   strcpy(led_LED1,"ledon.gif");
   strcpy(led_LED2,"ledon.gif");
   strcpy(led_LED3,"ledon.gif");
   strcpy(led_LED4,"ledon.gif");

   // show that the server is up and the IP address to connect to it
   ifconfig (IF_DEFAULT, IFG_IPADDR, &ip, IFS_END);
   printf ("Connect your browser to http://%s/\n", inet_ntoa (ipbuf, ip));

   // process WEB page requests and update the LED's
   while (1)
   {
      http_handler();
   }
}


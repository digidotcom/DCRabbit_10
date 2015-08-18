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
   browseled.c

	This program is used with RCM57xx or RCM67xx series controllers
	with Digital I/O accessory boards.

	Description
	===========
	This program demonstrates a basic controller running a
	Web page. Four "device LED's" are created with four
	buttons to toggle	them.  Users can browse to the device
	and change the	status of the lights. The LED's on the
	base board will match the ones on the web page.

   This program is adapted from \Samples\TCPIP\ssi.c.

   Jumper settings (Digital I/O board)
   -----------------------------------
   JP7   2-4, 3-5


      1 o   o 2
            |
      3 o   o 4
        |
      5 o   o 6

      7 o   o 8


   JP5   1-2, 3-4, 5-6, 7-8
   JP8   1-2, 3-4, 5-6, 7-8

         2    4    6   8
         o    o    o   o
         |    |    |   |
         o    o    o   o
         1    3    5   7

	Instructions
	============
	1. Make changes below in the configuration section to
	   match your application.
	2. Compile and run this program.
   3. With your Web browser access the Web page running on
   	the controller.
   4. View LEDS on Web page and the base board, DS1-DS4,
      to see that	they match-up when changing them via the
      Web page control button.

**********************************************************/
#class auto

#define DS1 4
#define DS2 5
#define DS3 6
#define DS4 7

#define USERLED 0
#define ON  0
#define OFF 1

/***********************************
 * Configuration Section           *
 * ---------------------           *
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
 * Define the number of HTTP servers and socket buffers.
 * With tcp_reserveport(), fewer HTTP servers are needed.
 */
#define HTTP_MAXSERVERS 2
#define MAX_TCP_SOCKET_BUFFERS 2

/*
 * Our web server as seen from the clients.
 * This should be the address that the clients (netscape/IE)
 * use to access your server. Usually, this is your IP address.
 * If you are behind a firewall, though, it might be a port on
 * the proxy, that will be forwarded to the Rabbit board. The
 * commented out line is an example of such a situation.
 */

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

#define REDIRECTTO 		myurl()

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

/*
 *  Notice that we have ximported in the source code for
 *  this program.  This allows us to <!--#include file="ssi.c"-->
 *  in the pages/showsrc.shtml.
 *
 */

#ximport "pages/browseled.shtml" index_html
#ximport "pages/rabbit1.gif"    	rabbit1_gif
#ximport "pages/ledon.gif"      	ledon_gif
#ximport "pages/ledoff.gif"     	ledoff_gif
#ximport "pages/button.gif"     	button_gif
#ximport "pages/showsrc.shtml"  	showsrc_shtml
#ximport "browseled.c"          	browseled_c


/*
 *  In this case the .html is not the first type in the
 *  type table.  This causes the default (no extension)
 *  to assume the shtml_handler.
 *
 */

/* the default for / must be first */
SSPEC_MIMETABLE_START
   SSPEC_MIME_FUNC( ".shtml", "text/html", shtml_handler), // ssi
   SSPEC_MIME( ".html", "text/html"),           // html
   SSPEC_MIME( ".cgi", ""),                     // cgi
   SSPEC_MIME( ".gif", "image/gif")
SSPEC_MIMETABLE_END

/*
 *  Each ledx contains a text string that is either
 *  "ledon.gif" or "ledoff.gif"  This string is toggled
 *  each time the ledxtoggle.cgi is requested from the
 *  browser.
 *
 */
char led1[15];
char led2[15];
char led3[15];
char led4[15];

/*
 *  Instead of sending other text back from the cgi's
 *  we have decided to redirect them to the original page.
 *  the cgi_redirectto forms a header which will redirect
 *  the browser back to the main page.
 *
 */
char *myurl() {
	static char URL[64];
   char tmpstr[32];
   long ipval;

   ifconfig(IF_DEFAULT, IFG_IPADDR, &ipval, IFS_END);
   sprintf(URL, "http://%s/index.shtml", inet_ntoa(tmpstr, ipval));
   return URL;
}

/*
 *  These ledXtoggle functions are called when buttons images
 *  in on the SHTML page are clicked. They toggle the LED images
 *  between lit LED and unlit LED images.
 *
 */
int led1toggle(HttpState* state)
{
   if (strcmp(led1,"ledon.gif")==0) {
      strcpy(led1,"ledoff.gif");
   }
   else {
      strcpy(led1,"ledon.gif");
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led2toggle(HttpState* state)
{
   if (strcmp(led2,"ledon.gif")==0) {
      strcpy(led2,"ledoff.gif");
   }
   else  {
      strcpy(led2,"ledon.gif");
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led3toggle(HttpState* state)
{
   if (strcmp(led3,"ledon.gif")==0) {
      strcpy(led3,"ledoff.gif");
   }
   else {
      strcpy(led3,"ledon.gif");
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led4toggle(HttpState* state)
{
   if (strcmp(led4,"ledon.gif")==0) {
      strcpy(led4,"ledoff.gif");
   }
   else {
      strcpy(led4,"ledon.gif");
   }
   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

SSPEC_RESOURCETABLE_START
   SSPEC_RESOURCE_XMEMFILE("/", index_html),
   SSPEC_RESOURCE_XMEMFILE("/index.shtml", index_html),
   SSPEC_RESOURCE_XMEMFILE("/showsrc.shtml", showsrc_shtml),
   SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
   SSPEC_RESOURCE_XMEMFILE("/ledon.gif", ledon_gif),
   SSPEC_RESOURCE_XMEMFILE("/ledoff.gif", ledoff_gif),
   SSPEC_RESOURCE_XMEMFILE("/button.gif", button_gif),

   SSPEC_RESOURCE_XMEMFILE("browseled.c", browseled_c),

   SSPEC_RESOURCE_ROOTVAR("led1", led1, PTR16, "%s"),
   SSPEC_RESOURCE_ROOTVAR("led2", led2, PTR16, "%s"),
   SSPEC_RESOURCE_ROOTVAR("led3", led3, PTR16, "%s"),
   SSPEC_RESOURCE_ROOTVAR("led4", led4, PTR16, "%s"),

   SSPEC_RESOURCE_FUNCTION("/led1tog.cgi", led1toggle),
   SSPEC_RESOURCE_FUNCTION("/led2tog.cgi", led2toggle),
   SSPEC_RESOURCE_FUNCTION("/led3tog.cgi", led3toggle),
   SSPEC_RESOURCE_FUNCTION("/led4tog.cgi", led4toggle)
SSPEC_RESOURCETABLE_END

/*
 *  Update the LEDs on the Digital I/O board to match
 *  the images chosen in the browser.
 *
 */
void update_outputs()
{
	if (strcmp(led1,"ledon.gif")) {
		BitWrPortI(PADR, &PADRShadow, OFF, DS1);
   }
   else {
		BitWrPortI(PADR, &PADRShadow, ON, DS1);
   }
	if (strcmp(led2,"ledon.gif")) {
		BitWrPortI(PADR, &PADRShadow, OFF, DS2);
   }
   else {
		BitWrPortI(PADR, &PADRShadow, ON, DS2);
   }
	if (strcmp(led3,"ledon.gif")) {
		BitWrPortI(PADR, &PADRShadow, OFF, DS3);
   }
   else {
		BitWrPortI(PADR, &PADRShadow, ON, DS3);
   }
	if (strcmp(led4,"ledon.gif")) {
		BitWrPortI(PADR, &PADRShadow, OFF, DS4);
   }
   else {
		BitWrPortI(PADR, &PADRShadow, ON, DS4);
   }
}

main()
{
   // Set Port A pins for LEDs low
   BitWrPortI(PADR, &PADRShadow, 1, DS1);
   BitWrPortI(PADR, &PADRShadow, 1, DS2);
   BitWrPortI(PADR, &PADRShadow, 1, DS3);
   BitWrPortI(PADR, &PADRShadow, 1, DS4);

   // Make Port A bit-wide output
   BitWrPortI(SPCR, &SPCRShadow, 1, 2);
   BitWrPortI(SPCR, &SPCRShadow, 0, 3);

   strcpy(led1,"ledon.gif");
   strcpy(led2,"ledoff.gif");
   strcpy(led3,"ledon.gif");
   strcpy(led4,"ledoff.gif");

   sock_init_or_exit(1);
   http_init();
   tcp_reserveport(80);

   while (1)
   {
   	update_outputs();
      http_handler();

	}
}


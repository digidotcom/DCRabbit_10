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
        Samples\TcpIp\RabbitWeb\updating.c

        Demonstrates the use of the updating() feature in the RabbitWeb HTTP
        enhancements.  The actual use of the updating() feature is in the

        samples\tcpip\rabbitweb\pages\updating.zhtml

        page.

        The variables in the web page must be between the values of 0 and 100.

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

/********************************
 * End of configuration section *
 ********************************/

/*
 * This is needed to be able to use the RabbitWeb HTTP enhancements and the
 * ZHTML scripting language.
 */
#define USE_RABBITWEB 1

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"

/*
 * This page contains the ZHTML portion of the updating() demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/updating.zhtml" updating_zhtml
#ximport "samples/tcpip/rabbitweb/pages/updating_success.html" success_html

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server. */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", updating_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", updating_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/success.html", success_html)
SSPEC_RESOURCETABLE_END

// These are simply variables that the user can update to demonstrate the
// updating() capability.
int foo;
int bar;

#web foo (($foo >= 0) && ($foo <= 100))
#web bar (($bar >= 0) && ($bar <= 100))

void main(void)
{
	// Initialize the variables
	foo = 25;
	bar = 75;

	// Initialize the TCP/IP stack and HTTP server
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();

	// This yields a performance improvement for an HTTP server
	tcp_reserveport(80);

   while (1) {
		// Drive the HTTP server
      http_handler();
   }
}


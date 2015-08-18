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
        Samples\TcpIp\RabbitWeb\web_error.c

        Demonstrates the use of the web_error() feature, along with displaying
        these errors in web pages, in the RabbitWeb HTTP enhancements.
        See also the

        samples\tcpip\rabbitweb\pages\web_error.zhtml

        page that demonstrates the corresponding ZHTML scripting features for
        web_error() error messages.

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
 * This page contains the ZHTML portion of the web_error() demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/web_error.zhtml"	web_error_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server. */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", web_error_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", web_error_zhtml)
SSPEC_RESOURCETABLE_END

// This variable will hold a temperature value that must be between the values
// 50 and 90.
int temperature;

// The following #web registration ensures that temperature is 50 or greater,
// and sets an error message of "too low" if it is not.
// Note the use of the C short-cut evaluation of the '||' operator.  If the
// first expression (LHS) is 'true', then the second expression (RHS) is not
// evaluated.  Otherwise, the RHS is evaluated, throwing the error, and the
// overall result is 'false' because web_error() always returns 0.
#web temperature ($temperature >= 50 || web_error("too low"))
// The following #web registration ensures that temperature is 90 or less,
// and sets an error message of "too high" if it is not.  Note that if a
// variable is registered more than once, then all guards for that variable are
// checked.  This guard expression uses the conditional operator (?:) rather
// than relying on short-cut evaluation.  Which style to use is a matter of
// personal taste.
#web temperature (($temperature <= 90)?1:web_error("too high"))

void main(void)
{
	// Initialize the temperature
	temperature = 72;

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


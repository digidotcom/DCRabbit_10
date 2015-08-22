/*******************************************************************************
        Samples\TcpIp\RabbitWeb\\arrays.c
        Rabbit Semiconductor, 2004

        Demonstrates the use of arrays in the RabbitWeb HTTP enhancements.
        See also the

        samples\tcpip\rabbitweb\pages\arrays.zhtml

        page that demonstrates the corresponding ZHTML scripting features for
        array variables.

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
 * This page contains the ZHTML portion of the selection variable demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/arrays.zhtml" arrays_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", arrays_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", arrays_zhtml)
SSPEC_RESOURCETABLE_END

/*
 * Array variables to be registered.  Note that you can use watch
 * expressions or the evaluate expression feature during runtime to ensure that
 * the variables have properly changed values.
 */

int array1[5];
int array2[2][2];
int array3[3];
int a3_lowbounds[3];

/*
 * #web statements
 */

// The following statement registers the entire array1[] array.  The '@'
// symbol is required as a placeholder for the array index.
#web array1[@] ($array1[@] > 0)
// This registers the multidimensional array array2[].  In this case there
// must be multiple '@' placeholders.
#web array2[@][@] (($array2[@][@] > 0) && ($array2[@][@] <= 10))
// It is not possible to register individual array elements differently,
// since all members of an array must have identical metadata.  It is
// possible, however, for an equivalent effect can be obtained by using
// an auxiliary array.  Note that the '@' placeholder is only valid in
// a $-prefixed variable name.
#web array3[@]	($array3[@] >= $a3_lowbounds[@])
// This is the auxiliary array mentioned above.  In this case, we use it to
// hold lower bounds for each of the array3 elements.
#web a3_lowbounds[@] groups=all(ro)	// Preclude external write access

void main(void)
{
	auto int i;

	// Initialized the #web-registered variables.
	for (i = 0; i < 5; i++) {
		array1[i] = i + 1;
	}
	array2[0][0] = 2;
	array2[0][1] = 3;
	array2[1][0] = 4;
	array2[1][1] = 5;
	for (i = 0; i < 3; i++) {
		array3[i] = i + 5;
		a3_lowbounds[i] = i+2;
	}

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


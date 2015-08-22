/*******************************************************************************
        Samples\TcpIp\RabbitWeb\structures.c
        Rabbit Semiconductor, 2004

        Demonstrates the use of structures in the RabbitWeb HTTP enhancements.
        See also the

        samples\tcpip\rabbitweb\pages\structures.zhtml

        page that demonstrates the corresponding ZHTML scripting features for
        structures.

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
#ximport "samples/tcpip/rabbitweb/pages/structures.zhtml"	structures_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", structures_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", structures_zhtml)
SSPEC_RESOURCETABLE_END

/*
 * Structure variable to be registered.  Note that you can use watch
 * expressions or the evaluate expression feature during runtime to ensure that
 * the variables have properly changed values.
 */

struct foo_struct {
	int a;
	struct {
		int b;
		long c;
	} bar;
	char d[2];	// Single byte string (plus a null terminator is always required)
	char e[40];	// Up to 39 char string, plus null term.
};

struct foo_struct foo;
struct foo_struct foo2;

/*
 * #web statements
 */

// In the following statement, the entire foo structure is registered at once.
// The guard expression checks all members.  The advantage of this is that it
// is fairly compact.  The disadvantages are:
//  - less efficient because the entire expression is evaluated for each
//    variable (field) which is changed
//  - an error in any one variable makes all changed variables appear to be
//    in error, which can confuse the end-user because it may not be clear which
//    value to "fix up".  For example, in this sample try changing foo.a to 11
//    and foo.d to 'C'.  The first change is legitimate, but the second is
//    not, however both variables are marked as being in error!
// In general, it is best to use a separate guard expression for each
// variable unless there is a true interdependency in the allowable values
// of several variables.
#web foo (($foo.a > 5) && ((*$foo.d == 'A') || (*$foo.d == 'B')) && \
          ($foo.bar.b > 0) && ($foo.bar.c > 30000))
// Below, each member of foo2 has its own guard expression.  This is much more
// user-friendly than the above all-in-one expression, since it allows the
// user to determine exactly which field is in error.
// Also, this shows how to make error messages which are more informative
// than the default "value out of range" messages.
#web foo2		// Note that this is required in order to register the "root"
					// variable.  Otherwise, foo2 is invisible regardless of the
					// following statements.
#web foo2.a ($foo2.a > 5 || \
							web_error("foo2.a must be greater than 5"))
#web foo2.d ((*$foo2.d == 'A') || (*$foo2.d == 'B') || \
							web_error("foo2.d must be 'A' or 'B'"))
#web foo2.bar.b ($foo2.bar.b > 0 || \
							web_error("foo2.bar.b must be positive"))
#web foo2.bar.c ($foo2.bar.c > 30000 || \
							web_error("foo2.bar.c must greater than 30000"))

void main(void)
{
	// Initialize the #web-registered variables
	foo.a = 10;
	foo.bar.b = 2;
	foo.bar.c = 50000;
	strcpy(foo.d, "B");
	strcpy(foo.e, "Hello world!");

	foo2.a = 20;
	foo2.bar.b = 4;
	foo2.bar.c = 100000;
	strcpy(foo2.d, "A");

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


/*******************************************************************************
        Samples\TcpIp\RabbitWeb\web_update.c
        Rabbit Semiconductor, 2004

        Demonstrates the use of the #web_update feature in the RabbitWeb HTTP
        enhancements, which allows the program to be notified when certain
        variables have acquired new values.  See also the

        samples\tcpip\rabbitweb\pages\web_update.zhtml

        page for the rest of the program.

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
 * This page contains the ZHTML portion of the #web_update demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/web_update.zhtml"	web_update_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", web_update_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", web_update_zhtml)
SSPEC_RESOURCETABLE_END

// This is a simple variable that will be updated
int foo;
// This structure defines some serial port parameters
typedef struct {
	long baud;
	int databits;
	int stopbits;
} SerPort;
// This array defines three serial ports.  Since arrays must be homogeneous,
// if we intend to assign different actions and checks for each element, it
// is better to define a structure with named (but identical) members in
// sequence.  This can be treated as an array in C (by setting a pointer to the
// address of the first field), and yet each element is distinct as far as
// RabbitWeb is concerned.  Note that it is wise to use decimal number
// suffixes on the field names, since this is convenient when looping over
// the fields (as if they were a true array) in ZHTML.  For example, the
// ZHTML for this sample uses constructs like varname($ports_s.port_$A.baud)
struct {
	SerPort port_0;
	SerPort port_1;
	SerPort port_2;
} ports_s;

// Here is where we set a pointer.  This is only for use by C, since RabbitWeb
// does not understand pointers.  This is convenient for program access
// since the code can access each element using e.g. ports[1].databits.
SerPort * ports = &ports_s.port_0;

// Register the root level variables
#web foo ($foo > 10)
#web ports_s

// This shows one possible style for
// coding the guard expression.  Assigning a web_error() to the value
// which is first found to be in error helps the user fix up any mistakes
// (otherwise, it can be difficult for the user to determine which field was
// incorrect if more than one field was edited at the same time).
// This is repeated for each port.  It is tedious, but allows different
// ports to have different validation requirements.
#web ports_s.port_0 \
	  			  (($ports_s.port_0.baud > 0 || \
				  			web_error("must be positive")) && \
               ($ports_s.port_0.databits == 7 || $ports_s.port_0.databits == 8 || \
               		web_error("must be 7 or 8")) && \
               ($ports_s.port_0.stopbits == 1 || $ports_s.port_0.stopbits == 2 || \
               		web_error("must be 1 or 2")))
#web ports_s.port_1 \
	  			  (($ports_s.port_1.baud == 4800 || $ports_s.port_1.baud == 9600 || \
				  			web_error("must be 4800 or 9600")) && \
               ($ports_s.port_1.databits == 7 || $ports_s.port_1.databits == 8 || \
               		web_error("must be 7 or 8")) && \
               ($ports_s.port_1.stopbits == 1 || $ports_s.port_1.stopbits == 2 || \
               		web_error("must be 1 or 2")))
#web ports_s.port_2 \
	  			  (($ports_s.port_2.baud == 19200 || $ports_s.port_2.baud == 9600 || \
				  			web_error("must be 9600 or 19200")) && \
               ($ports_s.port_2.databits == 7 || $ports_s.port_2.databits == 8 || \
               		web_error("must be 7 or 8")) && \
               ($ports_s.port_2.stopbits == 1 || $ports_s.port_2.stopbits == 2 || \
               		web_error("must be 1 or 2")))

// These are function prototypes for the #web_update statements below.
void foo_update(void);
void port0_update(void);
void port1_update(void);
void port2_update(void);

// Associate the foo_update() function with an update of the variable "foo".
// Note that foo_update() will only be called when the value of foo has been
// successfully updated.  That is, if any other variables being updated on the
// same web form is in error, then this function will *not* be called.  The
// function is only called when the committed value of "foo" is changed.
#web_update foo foo_update

// Each of the ports_s fields gets its own #web_update statement so that
// we can call different update functions for each port.
#web_update ports_s.port_0 port0_update
#web_update ports_s.port_1 port1_update
#web_update ports_s.port_2 port2_update

// This function is called when the value foo is updated.
void foo_update(void)
{
	// Print a
	printf("foo has been updated to %d!\n", foo);
}

// The following functions are called when the corresponding ports_s
// member is updated.  In a real program, the serial port may need to be
// closed and reopened using the new serial port information.
void port0_update(void)
{
	printf("ports[0] has been updated!\n");
}
void port1_update(void)
{
	printf("ports[1] has been updated!\n");
}
void port2_update(void)
{
	printf("ports[2] has been updated!\n");
}


void main(void)
{
	auto int i;

	// Initialize the #web-registered variables
	foo = 15;
	for (i = 0; i < 3; i++) {
		ports[i].baud = 9600;
		ports[i].databits = 8;
		ports[i].stopbits = 1;
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


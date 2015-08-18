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
        Samples\tcpip\RabbitWeb\tcp_to_serial.c

        Uses the RabbitWeb HTTP enhancements to configure a simple
        TCP-to-serial converter.  This sample only supports listening
        TCP sockets, meaning that TCP-to-serial devices can only be
        started by another device initiating the network connection to the
        Rabbit.

        This sample will work on any Rabbit board with either Ethernet or
        Wifi networking (or both, but only with one interface at a time!) plus a
        free asynchronous serial port.

        Each serial port can be associated with a specific TCP port.  The Rabbit
        will listen on each of these TCP ports for a connection, which will
        then be associated with a specific serial port.  Data will then be
        shuttled between the serial and network (TCP) connections.

   Instructions:

   0. If the network enabled Rabbit board is equipped with both Ethernet and
      WiFi network interfaces, ensure that exactly one of the DISABLE_ETHERNET
      or DISABLE_WIFI macros is custom defined. A convenient way to do this
      custom macro definition is to add e.g. DISABLE_ETHERNET into Dynamic C's
      Project Options' Defines tab.

   1. See the function help (Ctrl-H) on TCPCONFIG for instructions on
      compile-time network configuration.

	2. Compile and run this program.

   3. With your WEB browser access the WEB page running on the controller,
      such as http://10.10.6.100.

   4. Select the serial port settings on the browser.

   5. An example of associating a serial port with a tcp port is with telnet:
      Open a cmd window and type >telnet 10.10.6.100 1235,
      using your ip, to associate serial port D.

   6. Open a serial utility such as HyperTerminal, configured to match the
      settings selected on the browser.  Configure the serial utility to send
      CR+LF for new lines (in HyperTerminal, choose the File/Properties menu
      item, then click on the Settings tab and then the [ASCII Setup...] button
      and check the "Send line ends with line feeds" checkbox.
      Echo typed characters locally

   7. Connect the PC serial port selected in your serial utility to the Rabbit
      (for example, serial port D on connector J4 of the prototyping board).

   9. Type characters in the cmd window connection utility and observe them
      appearing in the serial utility window, and vice versa.

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

/*
 * The following array defines which serial ports are used in this sample.
 * Please check the documentation for your board to find which serial ports
 * are convenient to use.  Note that you can have any number of serial ports
 * defined below, not just two.
 */
const char ports_config[] = { 'C', 'D' };

#define T2S_BUFFER_SIZE 1024

/*
 * Only one server is really needed for the HTTP server as long as
 * tcp_reserveport() is called on port 80.
 */
#define HTTP_MAXSERVERS 1

/*
 * Define the number of TCP socket buffers to the number of sockets needed for
 * the HTTP server (HTTP_MAXSERVERS) + the number of serial ports configured
 * (sizeof(ports_config)).
 */
#define MAX_TCP_SOCKET_BUFFERS (HTTP_MAXSERVERS + sizeof(ports_config))

/*
 * Define the sizes of the input and output buffers for each of the serial
 * ports
 */
#define SERINBUFSIZE		127
#define SEROUTBUFSIZE	127

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem

/*
 * This is needed to be able to use the RabbitWeb HTTP enhancements and the
 * ZHTML scripting language.
 */
#define USE_RABBITWEB 1

#use "dcrtcp.lib"
#use "http.lib"

/*
 * Ensure the Rabbit board has exactly one enabled TCP/IP network interface.
 */
#if !(USING_ETHERNET || USING_WIFI)
	#fatal "This sample requires either an Ethernet or a WiFi network interface."
#elif USING_ETHERNET && USING_WIFI
	#error "This sample supports exactly one Ethernet or WiFi network interface"
	#error " at a time."
	#error "For Rabbit boards with both Ethernet and WiFi network interfaces,"
	#error " one interface must be disabled by adding one or the other of the"
	#error " DISABLE_ETHERNET or DISABLE_WIFI macros into Dynamic C's Project"
	#fatal " Options' Defines tab."
#endif

/*
 * This page contains the configuration interface for the serial ports.
 */
#ximport "samples/tcpip/rabbitweb/pages/config.zhtml"    config_zhtml

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", config_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", config_zhtml)
SSPEC_RESOURCETABLE_END

/*
 * Function declarations
 */
void restart_socket(int i);
void update_tcp(void);
void restart_serial(int i);
void update_serial(void);
void serial_open(int i);
void t2s_init(void);
void t2s_tick(void);

/*
 * This structure contains the configuration information for each serial port /
 * TCP port pair.
 */
struct SerialPort {
	word tcp_port;
	struct {
		char port[2];
		long baud;
		int databits;
      int parity;
		int stopbits;
	} ser;
};

/*
 * This stores the configuration information on the serial ports.  The members
 * will be registered with the #web statement so that the HTTP enhancements can
 * be used.
 */
struct SerialPort serial_ports[sizeof(ports_config)];

/*
 * This will function as a copy of the above.  It is used to determine which
 * port information changed when the update function is called (this will be
 * explained later in the program).
 */
struct SerialPort serial_ports_copy[sizeof(ports_config)];

/*
 * #web statements
 */
// It is necessary to always register the "root" level of each variable.
// In the case of arrays, like this sample, the placeholder must be specified.
#web serial_ports[@]

// A #web registration for the TCP port.  Note that the only rule in the guard
// is that the new value must be greater than 0.
#web serial_ports[@].tcp_port ($serial_ports[@].tcp_port > 0)
// The character ('B', 'C', etc.) representing the serial port.  This is a
// read-only variable.
#web serial_ports[@].ser.port
// The following two #web statements correspond to the baud rate.  The guards
// are split into two so that the WEB_ERROR() feature can be used.  WEB_ERROR()
// will indicated why the guard statement failed; the string message can later
// be used in the ZHTML scripting.
#web serial_ports[@].ser.baud ($serial_ports[@].ser.baud >= 300 || \
                              	web_error("too low"))
#web serial_ports[@].ser.baud ($serial_ports[@].ser.baud <= 115200 || \
                               	web_error("too high"))
// Each of the following are selection variables, since each of the variables
// can only take on a few values.
#web serial_ports[@].ser.databits select("7" = 7, "8" = 8)
#web serial_ports[@].ser.parity select("None" = 0, "Even", "Odd")
#web serial_ports[@].ser.stopbits select("1" = 1, "2" = 2)

// The #web_update feature will initiate a function call when the corresponding
// variables are updated.  Note that update_tcp() will be called when the TCP
// port changes, and update_serial() will be called when any of the other
// members are updated.
#web_update serial_ports[@].tcp_port update_tcp
#web_update serial_ports[@].ser.baud,serial_ports[@].ser.databits,\
            serial_ports[@].ser.stopbits update_serial

// The following simply sets the buffer sizes for the serial ports based on the
// user configuration above.
#define AINBUFSIZE	SERINBUFSIZE
#define AOUTBUFSIZE	SEROUTBUFSIZE
#define BINBUFSIZE	SERINBUFSIZE
#define BOUTBUFSIZE	SEROUTBUFSIZE
#define CINBUFSIZE	SERINBUFSIZE
#define COUTBUFSIZE	SEROUTBUFSIZE
#define DINBUFSIZE	SERINBUFSIZE
#define DOUTBUFSIZE	SEROUTBUFSIZE
#define EINBUFSIZE	SERINBUFSIZE
#define EOUTBUFSIZE	SEROUTBUFSIZE
#define FINBUFSIZE	SERINBUFSIZE
#define FOUTBUFSIZE	SEROUTBUFSIZE

// The following symbols represent different states in the TCP-to-serial
// state machine
enum {
	T2S_INIT,
	T2S_LISTEN,
	T2S_PROCESS
};

// This is the core of the TCP-to-serial state machine.
struct {
	int state;			// Current state of the the state machine
	tcp_Socket sock;	// Socket associated with this serial port
	// The following members are function pointers for accessing this serial
	// port
	int  (*open)(long baud);
	void (*close)(void);
	int  (*read)(void far *data, int length, unsigned long tmout);
	int  (*write)(const void far *source, int length);
	void (*setdatabits)(int state);
	void (*setparity)(int state);
} t2s_state[sizeof(ports_config)];

// A temporary buffer for copying data between the serial ports and TCP ports.
char t2s_buffer[T2S_BUFFER_SIZE];

// Aborts and restarts the given socket (index i).
void restart_socket(int i)
{
	printf("Restarting socket %d\n", i);
   // Abort the socket
   sock_abort(&(t2s_state[i].sock));
   // Set up the state machine to reopen the socket
   t2s_state[i].state = T2S_INIT;
}

// This function is called when a TCP port is updated via the HTML interface.
// It determines which TCP port(s) changed, and then restarts them with the new
// parameters.
void update_tcp(void)
{
	auto int i;

	// Check which TCP port(s) changed
	for (i = 0; i < sizeof(ports_config); i++) {
		if (serial_ports[i].tcp_port != serial_ports_copy[i].tcp_port) {
			// This port has changed, restart the socket on the new port
			restart_socket(i);
			// Save the new port, so we can check which one changed on the next
			// update
			serial_ports_copy[i].tcp_port = serial_ports[i].tcp_port;
		}
	}
}

// Closes and reopens the given serial port (index i).
void restart_serial(int i)
{
	printf("Restarting serial port %d\n", i);
   // Close the serial port
	t2s_state[i].close();
   // Open the serial port
   serial_open(i);
}

// This function is called when a serial port is updated via the HTML interface.
// It determines which serial port(s) changed, and then restarts them with the
// new parameters.
void update_serial(void)
{
	auto int i;

	// Check which serial port(s) changed
	for (i = 0; i < sizeof(ports_config); i++) {
		if (memcmp(&(serial_ports[i].ser), &(serial_ports_copy[i].ser),
		    sizeof(serial_ports[i].ser))) {
			// This serial port has changed, so re-open the serial port with the
			// new parameters
			restart_serial(i);
			// Save the new parameters, so we can check which one changed on the
			// next update
			memcpy(&(serial_ports_copy[i].ser), &(serial_ports[i].ser),
			       sizeof(serial_ports[i].ser));
		}
	}
}

// This function does all of the work necessary to open a serial port, including
// setting the number of data bits, stop bits, and parity.
void serial_open(int i)
{
	// Open the serial port
	t2s_state[i].open(serial_ports[i].ser.baud);

	// Set the data bits
	if (serial_ports[i].ser.databits == 7) {
		t2s_state[i].setdatabits(PARAM_7BIT);
	}
	else {
		t2s_state[i].setdatabits(PARAM_8BIT);
	}

	// Set the stop bits
	if (serial_ports[i].ser.stopbits == 1) {
   	if (serial_ports[i].ser.parity == 0) {
      	// No parity
			t2s_state[i].setparity(PARAM_NOPARITY);
      }
      else if (serial_ports[i].ser.parity == 1) {
      	// Even parity
      	t2s_state[i].setparity(PARAM_EPARITY);
      }
      else {
      	// Odd parity (== 2)
      	t2s_state[i].setparity(PARAM_OPARITY);
      }
	}
	else {
   	// 2 stop bits
		t2s_state[i].setparity(PARAM_2STOP);
	}
}

// Initialize the TCP-to-serial state machine.
void t2s_init(void)
{
	auto int i;

	for (i = 0; i < sizeof(ports_config); i++) {
	   // Initialize the state
	   t2s_state[i].state = T2S_INIT;

	   // Initialize the serial function pointers
	   switch (ports_config[i]) {
	   case 'A':
	   	t2s_state[i].open = serAopen;
	   	t2s_state[i].close = serAclose;
	   	t2s_state[i].read = serAread;
	   	t2s_state[i].write = serAwrite;
	   	t2s_state[i].setdatabits = serAdatabits;
	   	t2s_state[i].setparity = serAparity;
	   	break;
	   case 'B':
	   	t2s_state[i].open = serBopen;
	   	t2s_state[i].close = serBclose;
	   	t2s_state[i].read = serBread;
	   	t2s_state[i].write = serBwrite;
	   	t2s_state[i].setdatabits = serBdatabits;
	   	t2s_state[i].setparity = serBparity;
	   	break;
	   case 'C':
	   	t2s_state[i].open = serCopen;
	   	t2s_state[i].close = serCclose;
	   	t2s_state[i].read = serCread;
	   	t2s_state[i].write = serCwrite;
	   	t2s_state[i].setdatabits = serCdatabits;
	   	t2s_state[i].setparity = serCparity;
	   	break;
	   case 'D':
	   	t2s_state[i].open = serDopen;
	   	t2s_state[i].close = serDclose;
	   	t2s_state[i].read = serDread;
	   	t2s_state[i].write = serDwrite;
	   	t2s_state[i].setdatabits = serDdatabits;
	   	t2s_state[i].setparity = serDparity;
	   	break;
	   case 'E':
	   	t2s_state[i].open = serEopen;
	   	t2s_state[i].close = serEclose;
	   	t2s_state[i].read = serEread;
	   	t2s_state[i].write = serEwrite;
	   	t2s_state[i].setdatabits = serEdatabits;
	   	t2s_state[i].setparity = serEparity;
	   	break;
	   case 'F':
	   	t2s_state[i].open = serFopen;
	   	t2s_state[i].close = serFclose;
	   	t2s_state[i].read = serFread;
	   	t2s_state[i].write = serFwrite;
	   	t2s_state[i].setdatabits = serFdatabits;
	   	t2s_state[i].setparity = serFparity;
	   	break;
	   default:
	   	// Error--not a valid serial port
	   	exit(-1);
	   }

	   // Open each serial port
	   serial_open(i);
	}
}

// Drive the TCP-to-serial state machine.  This largely concerns itself
// with handling each of the TCP sockets (the different states correspond to
// the state of the TCP socket).  In particular, in the T2S_PROCESS state, it
// does the copying of data from TCP socket to serial port and vice versa.
void t2s_tick(void)
{
	auto char ipbuf[16];
	auto int i;
   auto int len;
   auto tcp_Socket *sock;

	for (i = 0; i < sizeof(ports_config); i++) {
   	sock = &(t2s_state[i].sock);
		switch (t2s_state[i].state) {
		case T2S_INIT:
			tcp_listen(sock, serial_ports[i].tcp_port, 0, 0, NULL, 0);
			t2s_state[i].state = T2S_LISTEN;
			break;
		case T2S_LISTEN:
      	if (!sock_waiting(sock)) {
         	// The socket is no longer waiting
            if (sock_established(sock)) {
            	// The socket is established
            	t2s_state[i].state = T2S_PROCESS;
            }
            else if (!sock_alive(sock)) {
            	// The socket was established but then aborted by the peer
               t2s_state[i].state = T2S_INIT;
            }
            else {
            	// The socket was opened, but is now closing.  Just go to the
               // PROCESS state to read off any data.
               t2s_state[i].state = T2S_PROCESS;
            }
            if (t2s_state[i].state == T2S_PROCESS) {
            	// Send a banner to the telnet connection as confirmation
            	// that connection was established, and to trigger character
            	// echo in the telnet client.
					len = sprintf( t2s_buffer,
						"Connected to serial port %s at %lubps:\r\n",
						serial_ports[i].ser.port, serial_ports[i].ser.baud);
					sock_fastwrite(sock, t2s_buffer, len);

					// Send a banner to the serial connection with the IP address
					// of the client.
					len = sprintf( t2s_buffer, "\r\nConnection from %s:\r\n",
						inet_ntoa( ipbuf, sock->hisaddr));
	            t2s_state[i].write(t2s_buffer, len);

	            // Finally, log the connection to STDOUT
					printf( "%s connected to serial port %s at %lubps:\n",
						ipbuf, serial_ports[i].ser.port, serial_ports[i].ser.baud);
            }
         }
			break;
		case T2S_PROCESS:
      	// Check if the socket is dead
         if (!sock_alive(sock)) {
         	t2s_state[i].state = T2S_INIT;
         }
			// Read from TCP socket and write to serial port
			len = sock_fastread(sock, t2s_buffer, T2S_BUFFER_SIZE);
         if (len < 0) {
         	// Error
            t2s_state[i].state = T2S_INIT;
         }
         if (len > 0) {
         	// Write the read data to the serial port--Note that for simplicity,
            // this code will drop bytes if more data has been read from the TCP
            // socket than can be written to the serial port.
            t2s_state[i].write(t2s_buffer, len);
         }
         else {
         	// No data read--do nothing
         }
         // Read from the serial port and write to the TCP socket
         len = t2s_state[i].read(t2s_buffer, T2S_BUFFER_SIZE, (unsigned long)0);
			if (len > 0) {
         	// Write the read data to the TCP port--Note that for simplicity,
            // this code will drop bytes if more data has been read from the
            // serial port than can be written to the TCP socket.
            len = sock_fastwrite(sock, t2s_buffer, len);
            if (len < 0) {
            	// Error
               t2s_state[i].state = T2S_INIT;
            }
         }
			if (t2s_state[i].state == T2S_INIT) {
            sock_abort(sock);

            // Log end of session
            printf( "%s closed connection to serial port %s.\n",
               inet_ntoa( ipbuf, sock->hisaddr), serial_ports[i].ser.port);

            // Send a message to the serial connection to let them know the
            // connection is closed.
            len = sprintf( t2s_buffer, "\r\nConnection closed.\r\n");
            t2s_state[i].write(t2s_buffer, len);
			}
			break;
		}
	}
}

void main(void)
{
	auto int i;

	// Initialize the serial_ports data structure
	for (i = 0; i < sizeof(ports_config); i++) {
		serial_ports[i].tcp_port = 1234 + i;
		serial_ports[i].ser.port[0] = ports_config[i];
		serial_ports[i].ser.port[1] = 0; // null term
		serial_ports[i].ser.baud = 9600;
		serial_ports[i].ser.databits = 8;
      serial_ports[i].ser.parity = 0;
		serial_ports[i].ser.stopbits = 1;
	}
	// Make a copy of the configuration options to be compared against when
	// the update functions are called
	memcpy(serial_ports_copy, serial_ports, sizeof(serial_ports));

	// Initialize the TCP/IP stack, HTTP server, and TCP-to-serial state
	// machine.
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
   t2s_init();

	// This is a performance improvement for the HTTP server (port 80),
	// especially when few HTTP server instances are used.
   tcp_reserveport(80);

   while (1) {
		// Drive the HTTP server
      http_handler();
      // Drive the TCP-to-serial state machine
      t2s_tick();
   }
}


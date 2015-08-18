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
        vserial.c

        This demonstrates the use of the new VSERIAL.LIB, which provides
        a gateway between serial ports or serial-port-like devices, and
        a telnet-style TCP socket.

			- Serial Port C is telnet port 23
			- Serial Port B is telnet port 3023

*******************************************************************************/
#class auto

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

/******************
 * vserial config *
 ******************/
/*
 * Unique gateway identifiers
 *
 * Each gateway mapping must be uniquely identified
 * with a number. A macro for each will make the
 * code much easier to read. Here, they are named
 * after the serial port they use.
 */
#define GATEWAY_PORTC	1
#define GATEWAY_PORTB	2
#define GATEWAY_PORTA	3

/*
 * serial buffer sizes
 * These are necessary any time we use the given
 * serial ports, because of how RS232.LIB works.
 */
#define AINBUFSIZE	31
#define AOUTBUFSIZE	31
#define BINBUFSIZE	31
#define BOUTBUFSIZE	31
#define CINBUFSIZE	31
#define COUTBUFSIZE	31

/* uncomment this to see debug messages */
#define VSERIAL_DEBUG

/*
 * The number of gateways that will be specified. This
 * must match the number of rows in the VSerialSpecTable[]
 * that is defined below.
 */
#define VSERIAL_NUM_GATEWAYS	3

/*
 * Pull in the library
 */
#use "dcrtcp.lib"
#use "vserial.lib"

/*
 * This table defines the low-level serial routines used
 * to talk to the serial port hardware. Each row is one
 * possible hardware gateway. Because the builtin Rabbit
 * serial ports will be used often, shortcut-macros have
 * been defined for each of the ports, A-D. These macros
 * take as a parameter an identifier such that they can
 * be referenced by the vserial_* functions below.
 *
 * The commented-out row at the end of the table is a
 * pseudocode example of defining your own low-level
 * gateway interface, if another hardware interface is
 * required. Most of the parameters are function pointers
 * to RS232.LIB style I/O routines.
 */
const VSerialSpec VSerialSpecTable[] = {
	VSERIAL_PORTC(GATEWAY_PORTC),
	VSERIAL_PORTB(GATEWAY_PORTB),
	VSERIAL_PORTA(GATEWAY_PORTA)
	// { ID, myOpen, myClose, myTick, myReadBufUsed, myWriteBufFree, myRead, myWrite }
};

///////////////////////////////////////////////////////////////////////

void main()
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	/* Initilize the vserial library (parse the above structures) */
	if(vserial_init()) {
		printf("Error starting vserial library!\n");
		exit(-1);
	}

	/*
	 * enable our first serial->tcp mapping
	 *
	 * The VSERIAL_COOKED turnes on parsing of the telnet control
	 * codes. This is useful if the other side of the network is
	 * actuall telnet software. This does change the datastream, though,
	 * so if a raw, unaltered, datastream is required, this option should
	 * not be used.
	 *
	 *  Serial Port C is telnet port 23
	 *  Serial Port B is telnet port 3023
	 */
	if(vserial_listen(GATEWAY_PORTC,57600,23,0L,VSERIAL_COOKED)) {
		printf("Error listening!\n");
		exit(-1);
	}

	if(vserial_listen(GATEWAY_PORTB,57600,3023,0L,VSERIAL_COOKED)) {
		printf("Error listening!\n");
		exit(-1);
	}

	/*
	 * Force the tcp connection to be persistent. This causes
	 * TCP Keepalives to be sent on the socket periodicly. It is
	 * important to note that this can cause an amount of network
	 * traffic over time.
	 */
	if(vserial_keepalive(GATEWAY_PORTC,30)) {
		printf("Error setting keepalive!\n");
		exit(-1);
	}

	/*
	 * GATEWAY_PORTA is not used here, as that would cause the
	 * programming/debugging port (it uses serial port A as well)
	 * to lose connectivity. It could be used in a similar manner,
	 * though, with another vserial_listen() or vserial_open().
	 */

	/* run it */
	for(;;) {
		vserial_tick();
	}
}
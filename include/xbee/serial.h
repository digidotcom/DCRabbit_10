/*
	Copyright (c)2008-2010 Digi International Inc., All Rights Reserved

	This software contains proprietary and confidential information of Digi
	International Inc.  By accepting transfer of this copy, Recipient agrees
	to retain this software in confidence, to prevent disclosure to others,
	and to make no use of this software other than that for which it was
	delivered.  This is a published copyrighted work of Digi International
	Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
	prohibited.

	Restricted Rights Legend

	Use, duplication, or disclosure by the Government is subject to
	restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
	Technical Data and Computer Software clause at DFARS 252.227-7031 or
	subparagraphs (c)(1) and (2) of the Commercial Computer Software -
	Restricted Rights at 48 CFR 52.227-19, as applicable.

	Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
*/

/**
	@addtogroup xbee_serial
	@{
	@file xbee/serial.h
	Platform-specific layer provieds a consistent serial API to upper layers
	of the driver.

	@section xbee_serial_overview XBee Serial API Overview

	This platform-specific layer maps a consistent
	serial API for the upper levels of the driver to the device's native
	serial API (e.g., serXread/serXwrite on the Rabbit, opening a COM port on a
	Windows PC).

	- sending and receiving serial data
		- xbee_ser_write()
		- xbee_ser_read()
		- xbee_ser_putchar()
		- xbee_ser_getchar()

	- checking the status of transmit and receive buffers
		- xbee_ser_tx_free()
		- xbee_ser_tx_used()
		- xbee_ser_tx_flush()
		- xbee_ser_rx_free()
		- xbee_ser_rx_used()
		- xbee_ser_rx_flush()

	- managing the serial port and control lines
		- xbee_ser_open()
		- xbee_ser_baudrate()
		- xbee_ser_close()
		- xbee_ser_break()
		- xbee_ser_flowcontrol()
		- xbee_ser_set_rts()
		- xbee_ser_get_cts()

	User code will not typically call these functions, unless they are
	not making use of the higher layers of the driver.

	Note that we may need some additional functions to support firmware updates.
	The firmware update code typically needs to be able to open the serial
	port at different baud rates, send a break on the Tx pin and control the
	reset pin.
*/

#ifndef __XBEE_SERIAL
#define __XBEE_SERIAL

#include "xbee/platform.h"

/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_invalid                        <serial.h>

SYNTAX:
   bool_t xbee_ser_invalid( xbee_serial_t *serial)

DESCRIPTION:
     Helper function used by other xbee_serial functions to
     validate the <serial> parameter.

     Confirms that it is non-NULL and is set to a valid port.


PARAMETER1:  serial - XBee serial port


RETURNS:  1        - <serial> is not a valid XBee serial port
          0        - <serial> is a valid XBee serial port

**************************************************************************/
bool_t xbee_ser_invalid( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_portname                       <serial.h>

SYNTAX:
   const char *xbee_ser_portname( xbee_serial_t *serial)

DESCRIPTION:
     Returns a human-readable string describing the serial port.

     For example, on a Windows machine this will be "COM1" or "COM999".
     On a Rabbit, it will be a single letter, "A" through "F".
     On Freescale HCS08, it will be something like "SCI1" or "SCI2".
     On POSIX, the name of the device (e.g., "/dev/ttyS0")

     Returns "(invalid)" if <serial> is invalid or not configured.


PARAMETER1:  serial - port


RETURNS:  null-terminated string describing the serial port

**************************************************************************/
const char *xbee_ser_portname( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_write                          <serial.h>

SYNTAX:
   int xbee_ser_write( xbee_serial_t *serial,  const void FAR *buffer, 
                       int length)

DESCRIPTION:
     Transmits <length> bytes from <buffer> to the XBee serial
     port <serial>.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  buffer - source of bytes to send

PARAMETER3:  length - number of bytes to write


RETURNS:  >=0      - The number of bytes successfully written to XBee
                     serial port.
          -EINVAL  - <serial> is not a valid XBee serial port.
          -EIO     - I/O error attempting to write to serial port.

SEE ALSO:  xbee_ser_read(), xbee_ser_putchar(), xbee_ser_getchar()

**************************************************************************/
int xbee_ser_write( xbee_serial_t *serial, const void FAR *buffer,
	int length);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_read                           <serial.h>

SYNTAX:
   int xbee_ser_read( xbee_serial_t *serial,  void FAR *buffer,  int bufsize)

DESCRIPTION:
     Reads up to <bufsize> bytes from XBee serial port <serial>
     and into <buffer>.

     If there is no data available when the function is
     called, it will return immediately.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  buffer - buffer to hold bytes read from XBee serial port

PARAMETER3:  bufsize - maximum number of bytes to read


RETURNS:  >=0      - The number of bytes read from XBee serial port.
          -EINVAL  - <serial> is not a valid XBee serial port.
          -EIO     - I/O error attempting to write to serial port.

SEE ALSO:  xbee_ser_write(), xbee_ser_putchar(), xbee_ser_getchar()

**************************************************************************/
int xbee_ser_read( xbee_serial_t *serial, void FAR *buffer, int bufsize);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_putchar                        <serial.h>

SYNTAX:
   int xbee_ser_putchar( xbee_serial_t *serial,  uint8_t ch)

DESCRIPTION:
     Transmits a single character, <ch>, to the XBee serial
     port <serial>.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  ch - character to send


RETURNS:  0        - Successfully sent (queued) character.
          -ENOSPC  - The write buffer is full and the character wasn't sent.
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_read(), xbee_ser_write(), xbee_ser_getchar()

**************************************************************************/
int xbee_ser_putchar( xbee_serial_t *serial, uint8_t ch);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_getchar                        <serial.h>

SYNTAX:
   int xbee_ser_getchar( xbee_serial_t *serial)

DESCRIPTION:
     Reads a single character from the XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  0-255    - character read from XBee serial port
          -ENODATA - There aren't any characters in the read buffer.
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_read(), xbee_ser_write(), xbee_ser_getchar()

**************************************************************************/
int xbee_ser_getchar( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_tx_free                        <serial.h>

SYNTAX:
   int xbee_ser_tx_free( xbee_serial_t *serial)

DESCRIPTION:
     Returns the number of bytes of unused space in the serial
     transmit buffer for XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  INT_MAX	The buffer size is unlimited (or unknown).
          >=0      - The number of bytes it would take to fill the XBee
                     serial port's serial transmit buffer.
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_rx_free(), xbee_ser_rx_used(), xbee_ser_rx_flush(),
          xbee_ser_tx_used(), xbee_ser_tx_flush()

**************************************************************************/
int xbee_ser_tx_free( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_tx_used                        <serial.h>

SYNTAX:
   int xbee_ser_tx_used( xbee_serial_t *serial)

DESCRIPTION:
     Returns the number of queued bytes in the serial transmit buffer
     for XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  0        - The buffer size is unlimited (or space used is unknown).
          >0       - The number of bytes queued in the XBee
                     serial port's serial transmit buffer.
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_rx_free(), xbee_ser_rx_used(), xbee_ser_rx_flush(),
          xbee_ser_tx_free(), xbee_ser_tx_flush()

**************************************************************************/
int xbee_ser_tx_used( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_tx_flush                       <serial.h>

SYNTAX:
   int xbee_ser_tx_flush( xbee_serial_t *serial)

DESCRIPTION:
     Flushes (i.e., deletes and does not transmit) characters in the
     serial transmit buffer for XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  0        - success
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_rx_free(), xbee_ser_rx_used(), xbee_ser_rx_flush(),
          xbee_ser_tx_free(), xbee_ser_tx_used()

**************************************************************************/
int xbee_ser_tx_flush( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_rx_free                        <serial.h>

SYNTAX:
   int xbee_ser_rx_free( xbee_serial_t *serial)

DESCRIPTION:
     Returns the number of bytes of unused space in the serial
     receive buffer for XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  INT_MAX	The buffer size is unlimited (or unknown).
          >=0      - The number of bytes it would take to fill the XBee
                     serial port's serial receive buffer.
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_tx_free(), xbee_ser_tx_used(), xbee_ser_tx_flush(),
          xbee_ser_rx_used(), xbee_ser_rx_flush()

**************************************************************************/
int xbee_ser_rx_free( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_rx_used                        <serial.h>

SYNTAX:
   int xbee_ser_rx_used( xbee_serial_t *serial)

DESCRIPTION:
     Returns the number of queued bytes in the serial receive buffer
     for XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  >=0      - The number of bytes queued in the XBee
                     serial port's serial transmit buffer.
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_tx_free(), xbee_ser_tx_used(), xbee_ser_tx_flush(),
          xbee_ser_rx_free(), xbee_ser_rx_flush()

   NOTE: Unlike xbee_ser_tx_used(), this function MUST return the number
          of bytes available.  Some layers of the library wait until enough
          bytes are ready before continuing.

          We may expand on or replace this API.  On some platforms (like
          Win32) we have to do some of our own buffering in order to
          peek at data in the serial receive buffer.  Most of the driver
          only requires a check to see if some number of bytes are available
          or not.  Consider changing the API to something like:



          returns TRUE if \c count bytes are available, FALSE otherwise.

**************************************************************************/
int xbee_ser_rx_used( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_rx_flush                       <serial.h>

SYNTAX:
   int xbee_ser_rx_flush( xbee_serial_t *serial)

DESCRIPTION:
     Deletes all characters in the serial receive buffer for XBee serial
     port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  0        - success
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_tx_free(), xbee_ser_tx_used(), xbee_ser_tx_flush(),
          xbee_ser_rx_free(), xbee_ser_rx_used()

**************************************************************************/
int xbee_ser_rx_flush( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_open                           <serial.h>

SYNTAX:
   int xbee_ser_open( xbee_serial_t *serial,  uint32_t baudrate)

DESCRIPTION:
     Opens the serial port connected to XBee serial port <serial> at
     <baudrate> bits/second.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  baudrate - Bits per second of serial data transfer speed.


RETURNS:  0        - Opened serial port within 5% of requested baudrate.
          -EINVAL  - <serial> is not a valid XBee serial port.
          -EIO     - Can't open serial port within 5% of requested baudrate.

SEE ALSO:  xbee_ser_baudrate(), xbee_ser_close(), xbee_ser_break()

**************************************************************************/
int xbee_ser_open( xbee_serial_t *serial, uint32_t baudrate);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_baudrate                       <serial.h>

SYNTAX:
   int xbee_ser_baudrate( xbee_serial_t *serial,  uint32_t baudrate)

DESCRIPTION:
     Change the baud rate of XBee serial port <serial> to
     <baudrate> bits/second.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  baudrate - Bits per second of serial data transfer speed.


RETURNS:  0        - Opened serial port within 5% of requested baudrate.
          -EINVAL  - <serial> is not a valid XBee serial port.
          -EIO     - Can't open serial port within 5% of requested baudrate.

SEE ALSO:  xbee_ser_open(), xbee_ser_close(), xbee_ser_break()

**************************************************************************/
int xbee_ser_baudrate( xbee_serial_t *serial, uint32_t baudrate);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_close                          <serial.h>

SYNTAX:
   int xbee_ser_close( xbee_serial_t *serial)

DESCRIPTION:
     Close the serial port attached to XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port


RETURNS:  0        - closed serial port
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_open(), xbee_ser_baudrate(), xbee_ser_break()

**************************************************************************/
int xbee_ser_close( xbee_serial_t *serial);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_break                          <serial.h>

SYNTAX:
   int xbee_ser_break( xbee_serial_t *serial,  bool_t enabled)

DESCRIPTION:
     Disable the serial transmit pin and pull it low to send a break
     to the XBee serial port.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  enabled - Set to 1 to start the break or 0 to end the break (and
              resume transmitting).


RETURNS:  0        - Success
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_open(), xbee_ser_close()

**************************************************************************/
int xbee_ser_break( xbee_serial_t *serial, bool_t enabled);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_flowcontrol                    <serial.h>

SYNTAX:
   int xbee_ser_flowcontrol( xbee_serial_t *serial,  bool_t enabled)

DESCRIPTION:
     Enable or disable hardware flow control (CTS/RTS) on the serial
     port for XBee serial port <serial>.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  enabled - Set to 0 to disable flow control or non-zero to enable
              flow control.


RETURNS:  0        - Success
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_set_rts(), xbee_ser_get_cts()


**************************************************************************/
int xbee_ser_flowcontrol( xbee_serial_t *serial, bool_t enabled);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_set_rts                        <serial.h>

SYNTAX:
   int xbee_ser_set_rts( xbee_serial_t *serial,  bool_t asserted)

DESCRIPTION:
     Disable hardware flow control and manually set the RTS (ready to
     send) pin on the XBee device's serial port.

     Typically used to enter the XBee device's boot loader and initiate
     a firmware update.


PARAMETER1:  serial - XBee serial port

PARAMETER2:  asserted - Set to 1 to assert RTS (ok for XBee to send to us)
              or 0 to deassert RTS (tell XBee not to send to us).


RETURNS:  0        - Success
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_flowcontrol(), xbee_ser_get_cts()

**************************************************************************/
int xbee_ser_set_rts( xbee_serial_t *serial, bool_t asserted);


/* START FUNCTION DESCRIPTION ********************************************
xbee_ser_get_cts                        <serial.h>

SYNTAX:
   int xbee_ser_get_cts( xbee_serial_t *serial)

DESCRIPTION:
     Read the status of the /CTS (clear to send) pin on the serial
     port connected to XBee serial port <serial>.

     Note that this
     function doesn't return the value of the pin -- it returns
     whether it's asserted (i.e., clear to send to the XBee serial
     port) or not.


PARAMETER1:  serial - XBee serial port


RETURNS:  1        - it's clear to send
          0        - it's not clear to send
          -EINVAL  - <serial> is not a valid XBee serial port.

SEE ALSO:  xbee_ser_flowcontrol(), xbee_ser_set_rts()

**************************************************************************/
int xbee_ser_get_cts( xbee_serial_t *serial);

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
   #use "xbee_serial_rabbit.c"
#endif

#endif

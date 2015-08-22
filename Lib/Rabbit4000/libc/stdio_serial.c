/*
	stdio_serial.c

	Copyright (c)2010 Digi International Inc., All Rights Reserved

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

/*
	Support functions for accessing a serial port as a stream.
*/

/*** BeginHeader */
#include <stdio.h>

#ifdef STDIO_SERIAL_DEBUG
	#define _stdio_serial_debug __debug
#else
	#define _stdio_serial_debug __nodebug
#endif
/*** EndHeader */

/*** BeginHeader _serXstream */
#define serXstream(n, mode)		_serXstream( sxd[n], mode)
FILE __far *_serXstream( serXdata *port, char __far *mode);
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
serXstream                                                <stdio_serial.c>

SYNTAX:	FILE far *serXstream( int port, char far *mode)

DESCRIPTION:	Open a stream and attach it to a serial port already opened
					with serAopen, serBopen, etc.

PARAMETER 1:	The port number.  Valid inputs are SER_PORT_A through SER_PORT_F.
					This function is defined (through a macro) to use this value to
					select the appropriate serial port data structure.

PARAMETER 2:	Either "r", "w" or "rw".  Due to how stream buffering works,
					"rw" mode is not recommended.  It is possible to open two streams
					for a serial port -- one for read and the other for write.

					If opening the port in "rw" mode, it will be necessary to seek
					between reading and writing

RETURN VALUE:	Pointer to the FILE object for accessing the serial port as a
					stream.  Returns NULL if all streams are in use or mode is
					invalid.

END DESCRIPTION **********************************************************/
_stdio_serial_debug
FILE __far *_serXstream( serXdata *port, char __far *mode)
{
	FILE __far *stream;
	int c0, c1;

	stream = _stdio_FILE_alloc();
	if (! stream)
	{
		return NULL;
	}

	_f_memset( stream, 0, sizeof *stream);
	stream->flags = _FILE_FLAG_USED
		| _FILE_FLAG_OPEN_RW
		| _FILE_FLAG_CAN_READ
		| _FILE_FLAG_CAN_WRITE
		| _FILE_FLAG_BUF_NONE;
	stream->cookie = port;
	c0 = mode[0];
	c1 = mode[1];
	if (c0 == 'r')
	{
		stream->read = _stream_serial_read;
	}
	if (c0 == 'w' || c1 == 'w')
	{
		stream->write = _stream_serial_write;
	}

	return stream;
}

/*** BeginHeader _stream_serial_read, _stream_serial_write */
size_t _stream_serial_read( void __far *cookie, void __far *buffer, size_t bytes);
size_t _stream_serial_write( void __far *cookie, const void __far *buffer, size_t bytes);
/*** EndHeader */
/*
	DEVNOTE: cookie should be a pointer to the serXdata structure for a given
	serial port.  Will need an fopen_serial(?) function to attach a stream to
	an already-opened serial port.

	These handlers should actually be quite easy to write, since we're going
	to rely on the underlying serial driver's functions.

	The serial stream will only run in unbuffered (stream) mode (e.g., clear the
	buffer flags?)

	No need for seek, since serial ports aren't seekable.  Assign NULL to the
	.seek function pointer.
*/

/* START _FUNCTION DESCRIPTION ********************************************
_stream_serial_read                                       <stdio_serial.c>

	Standard .read function for FILE object.  See _stream_read() for API.

END DESCRIPTION **********************************************************/
_stdio_serial_debug
size_t _stream_serial_read( void __far *cookie, void __far *buffer, size_t bytes)
{
	serXdata *ser;

	// cookie is a near pointer to a serXdata object, promoted to a far.
	ser = (serXdata *)(unsigned)(unsigned long)cookie;
	return _serXread( ser, buffer, bytes, 0);
}

/* START _FUNCTION DESCRIPTION ********************************************
_stream_serial_write                                      <stdio_serial.c>

	Standard .write function for FILE object.  See _stream_write() for API.

END DESCRIPTION **********************************************************/
_stdio_serial_debug
size_t _stream_serial_write( void __far *cookie, const void __far *buffer,
																						size_t bytes)
{
	serXdata *ser;

	// cookie is a near pointer to a serXdata object, promoted to a far.
	ser = (serXdata *)(unsigned)(unsigned long)cookie;
	return _serXwrite( ser, buffer, bytes);
}
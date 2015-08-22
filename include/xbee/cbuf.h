/*
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

/**
	@addtogroup util_cbuf
	@{
	@file xbee/cbuf.h
	Circular buffer data type used by the OTA (Over-The-Air) firmware update
	client and transparent serial cluster.
*/

#ifndef XBEE_CBUF_H
#define XBEE_CBUF_H

#include "xbee/platform.h"

/// Circular buffer used by transparent serial cluster handler.  Buffer is
/// empty when \c head == \c tail and full when \c head == \c (tail - 1).
typedef struct xbee_cbuf_t {
	uint8_t			lock;
	uint8_t			head;		// front, or head index of data
	uint8_t			tail;		// back, or tail index of data
	uint8_t			mask;		// 2^n - 1
	uint8_t			data[1];	// variable length (<mask> bytes + 1 separator byte)
} xbee_cbuf_t;

/**
	XBEE_CBUF_OVERHEAD is used when allocating memory for a circular buffer.
	For a cbuf that can hold X bytes, you need (X + CBUF_OVERHEAD) bytes of
	memory.

	XBEE_CBUF_OVERHEAD includes a 4-byte header, plus a separator byte in the
	buffered data.

	For example, to set up a 31-byte circular buffer:
@code
	#define MY_BUF_SIZE 31			// must be 2^n -1
	char my_buf_space[MY_BUF_SIZE + XBEE_CBUF_OVERHEAD];
	xbee_cbuf_t *my_buf;

	my_buf = (xbee_cbuf_t *)my_buf_space;
	xbee_cbuf_init( my_buf, MY_BUF_SIZE);

	// or

	struct {
		xbee_cbuf_t		cbuf;
		char				buf_space[MY_BUF_SIZE];
	} my_buf;

	xbee_cbuf_init( &my_buf.cbuf, MY_BUF_SIZE);
@endcode
*/
#define XBEE_CBUF_OVERHEAD sizeof(xbee_cbuf_t)

/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_init                          <cbuf.h>

SYNTAX:
   int xbee_cbuf_init( xbee_cbuf_t FAR *cbuf,  uint_fast8_t datasize)

DESCRIPTION:
     Initialize the fields of the circular buffer.

   NOTE: 
     You must initialize the xbee_cbuf_t structure before using it with any
     other xbee_cbuf_xxx() functions.  If you have ISRs pushing data
     into the buffer, don't enable them until AFTER you've called
     xbee_cbuf_init.


PARAMETER1:  cbuf - 
              Address of buffer to use for the circular buffer.  This buffer
              must be (datasize + CBUF_OVEREAD) bytes long to hold the locks,
              head, tail, size and buffered bytes.

PARAMETER2:  datasize - 
              Maximum number of bytes to store in the circular buffer.  This
              size must be at least 3, no more than 255, and
              a power of 2 minus 1 (2^n - 1).


RETURNS:  0        - success
          -EINVAL  - invalid parameter

**************************************************************************/
int xbee_cbuf_init( xbee_cbuf_t FAR *cbuf, uint_fast8_t datasize);

/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_putch                         <cbuf.h>

SYNTAX:
   int xbee_cbuf_putch( xbee_cbuf_t FAR *cbuf,  uint_fast8_t ch)

DESCRIPTION:
     Append a single byte to the circular buffer (if not full).


PARAMETER1:  cbuf - Pointer to circular buffer.

PARAMETER2:  ch - Byte to append.


RETURNS:  0        - buffer is full
          1        - the byte was appended



**************************************************************************/
int xbee_cbuf_putch( xbee_cbuf_t FAR *cbuf, uint_fast8_t ch);


/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_getch                         <cbuf.h>

SYNTAX:
   int xbee_cbuf_getch( xbee_cbuf_t FAR *cbuf)

DESCRIPTION:
     Remove and return the first byte of the circular buffer.


PARAMETER1:  cbuf - Pointer to circular buffer.


RETURNS:  -1       - buffer is empty
          0-255    - byte removed from the head of the buffer




**************************************************************************/
int xbee_cbuf_getch( xbee_cbuf_t FAR *cbuf);


/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_length                        <cbuf.h>

MACRO SYNTAX:
     xbee_cbuf_length( cbuf)

DESCRIPTION:
     Returns the capacity of the circular buffer.


PARAMETER1:  cbuf - Pointer to circular buffer.


RETURNS:  0-255    - Maximum number of bytes that can
                     be stored in the circularbuffer.



**************************************************************************/
#define xbee_cbuf_length(cbuf)	((cbuf)->mask)


/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_used                          <cbuf.h>

SYNTAX:
   uint_fast8_t xbee_cbuf_used( xbee_cbuf_t FAR *cbuf)

DESCRIPTION:
     Returns the number of bytes stored in the circular buffer.


PARAMETER1:  cbuf - Pointer to circular buffer.


RETURNS:  0-255    - Number of bytes stored in the circular buffer.




**************************************************************************/
uint_fast8_t xbee_cbuf_used( xbee_cbuf_t FAR *cbuf);


/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_free                          <cbuf.h>

SYNTAX:
   uint_fast8_t xbee_cbuf_free( xbee_cbuf_t FAR *cbuf)

DESCRIPTION:
     Returns the number of additional bytes that can be stored in
     the circular buffer.


PARAMETER1:  cbuf - Pointer to circular buffer.


RETURNS:  0-255    - Number of unused bytes in the circular buffer.




**************************************************************************/
uint_fast8_t xbee_cbuf_free( xbee_cbuf_t FAR *cbuf);


/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_flush                         <cbuf.h>

SYNTAX:
   void xbee_cbuf_flush( xbee_cbuf_t FAR *cbuf)

DESCRIPTION:
     Flush the contents of the circular buffer.


PARAMETER1:  cbuf - Pointer to circular buffer.


**************************************************************************/
void xbee_cbuf_flush( xbee_cbuf_t FAR *cbuf);


/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_put                           <cbuf.h>

SYNTAX:
   uint_fast8_t xbee_cbuf_put( xbee_cbuf_t FAR *cbuf,  const void FAR *buffer, 
                               uint_fast8_t length)

DESCRIPTION:
     Append multiple bytes to the end of a circular buffer.


PARAMETER1:  cbuf - circular buffer
PARAMETER2:  buffer - data to copy into circular buffer
PARAMETER3:  length - number of bytes to copy


RETURNS:  0-255    - number of bytes copied (may be less than length if buffer
                     is full)

**************************************************************************/
uint_fast8_t xbee_cbuf_put( xbee_cbuf_t FAR *cbuf, const void FAR *buffer,
																		uint_fast8_t length);
/* START FUNCTION DESCRIPTION ********************************************
xbee_cbuf_get                           <cbuf.h>

SYNTAX:
   uint_fast8_t xbee_cbuf_get( xbee_cbuf_t *cbuf,  void FAR *buffer, 
                               uint_fast8_t length)

DESCRIPTION:
     Read (and remove) multiple bytes from circular buffer.


PARAMETER1:  cbuf - circular buffer
PARAMETER2:  buffer - destination to copy data from circular buffer
PARAMETER3:  length - number of bytes to copy


RETURNS:  0-255    - number of bytes copied (may be less than length if buffer
                     is empty)

**************************************************************************/
uint_fast8_t xbee_cbuf_get( xbee_cbuf_t *cbuf, void FAR *buffer,
																		uint_fast8_t length);

#endif		// XBEE_CBUF_H defined

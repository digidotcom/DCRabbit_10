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
	@file xbee_cbuf.c

	Circular buffer data type used by the OTA (Over-The-Air) firmware update
	client and transparent serial cluster.

	Write to tail, read from head.
*/
/*** BeginHeader */
#include "xbee/cbuf.h"
/*** EndHeader */

/*		Functions are documented in xbee/cbuf.h		*/

/*** BeginHeader xbee_cbuf_init */
/*** EndHeader */
int xbee_cbuf_init( xbee_cbuf_t FAR *cbuf, uint_fast8_t datasize)
{
	if (! cbuf || (datasize < 3) || (datasize & (datasize + 1)))
	{
		return -EINVAL;
	}
	cbuf->mask = datasize;
	cbuf->lock = 0;
	cbuf->head = cbuf->tail = 0;

	return 0;
}

/*** BeginHeader xbee_cbuf_putch */
/*** EndHeader */
int xbee_cbuf_putch( xbee_cbuf_t FAR *cbuf, uint_fast8_t ch)
{
	uint8_t t;

	t = cbuf->tail;
	cbuf->data[t] = ch;
	t = (t + 1) & cbuf->mask;
	if (t == cbuf->head)
	{
		return 0;		// buffer is full
	}
	cbuf->tail = t;
	return 1;
}

/*** BeginHeader xbee_cbuf_getch */
/*** EndHeader */
int xbee_cbuf_getch( xbee_cbuf_t FAR *cbuf)
{
	uint8_t	h;
	uint8_t	retval;

	h = cbuf->head;
	if (h == cbuf->tail)
	{
		return -1;		// buffer is empty
	}
	retval = cbuf->data[h];
	cbuf->head = (h + 1) & cbuf->mask;

	return retval;
}

/*** BeginHeader xbee_cbuf_used */
/*** EndHeader */
uint_fast8_t xbee_cbuf_used( xbee_cbuf_t FAR *cbuf)
{
	return (cbuf->tail - cbuf->head) & cbuf->mask;
}

/*** BeginHeader xbee_cbuf_free */
/*** EndHeader */
uint_fast8_t xbee_cbuf_free( xbee_cbuf_t FAR *cbuf)
{
	return (cbuf->head - cbuf->tail - 1) & cbuf->mask;
}

/*** BeginHeader xbee_cbuf_flush */
/*** EndHeader */
void xbee_cbuf_flush( xbee_cbuf_t FAR *cbuf)
{
	cbuf->head = cbuf->tail;
}

/*** BeginHeader xbee_cbuf_put */
/*** EndHeader */
uint_fast8_t xbee_cbuf_put( xbee_cbuf_t FAR *cbuf, const void FAR *buffer,
																			uint_fast8_t length)
{
	// TODO optimize by using two memcpy calls
	uint_fast8_t copy;
	uint_fast8_t stop;
	uint_fast8_t mask = cbuf->mask;

	stop = (cbuf->head - 1) & mask;
	// when tail == stop, buffer is full
	for (copy = length; copy && cbuf->tail != stop; --copy)
	{
		cbuf->data[cbuf->tail] = *(const char FAR *)buffer;
		buffer = (const char FAR *)buffer + 1;
		cbuf->tail = (cbuf->tail + 1) & mask;
	}

	#ifdef XBEE_CBUF_VERBOSE
		if (length != copy)
		{
			printf( "%s: %u bytes in\n", __FUNCTION__, length - copy);
		}
	#endif

	return length - copy;
}

/*** BeginHeader xbee_cbuf_get */
/*** EndHeader */
uint_fast8_t xbee_cbuf_get( xbee_cbuf_t *cbuf, void FAR *buffer,
																			uint_fast8_t length)
{
	// TODO optimize by using two memcpy calls
	uint_fast8_t copy;
	uint_fast8_t mask = cbuf->mask;

	// when head == tail, buffer is empty
	for (copy = length; copy && cbuf->head != cbuf->tail; --copy)
	{
		*(char FAR *)buffer = cbuf->data[cbuf->head];
		buffer = (char FAR *)buffer + 1;
		cbuf->head = (cbuf->head + 1) & mask;
	}

	#ifdef XBEE_CBUF_VERBOSE
		if (length != copy)
		{
			printf( "%s: %u bytes out\n", __FUNCTION__, length - copy);
		}
	#endif

	return length - copy;
}


/*
	Copyright (c)2009-2010 Digi International Inc., All Rights Reserved

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
	@addtogroup xbee_wpan
	@{
	@file xbee_wpan.c
	Glue layer between XBee-specific code and general WPAN layer.
*/

/*** BeginHeader */
#include <stddef.h>
#include <stdio.h>

#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/wpan.h"
#include "wpan/types.h"
#include "wpan/aps.h"
#include "zigbee/zdo.h"
#include "zigbee/zcl.h"

#ifndef __DC__
	#define xbee_wpan_debug
#elif defined XBEE_WPAN_DEBUG
	#define xbee_wpan_debug		__debug
#else
	#define xbee_wpan_debug		__nodebug
#endif
/*** EndHeader */

/*** BeginHeader _xbee_handle_receive_explicit */
/*** EndHeader */

#include "xbee/byteorder.h"

/**
	Process XBee "Receive Explicit" frames (type 0x91) and hand
	off to wpan_envelope_dispatch() for further processing.

	Please view the function help for xbee_frame_handler_fn() for details
	on this function's parameters and possible return values.
*/
xbee_wpan_debug
int _xbee_handle_receive_explicit( xbee_dev_t *xbee, const void FAR *raw,
	uint16_t length, void FAR *context)
{
	const xbee_frame_receive_explicit_t FAR *frame = raw;
	wpan_envelope_t	env;

	// this XBee frame handler (standard API) doesn't use the context parameter
	XBEE_UNUSED_PARAMETER( context);

	if (length < offsetof( xbee_frame_receive_explicit_t, payload))
	{
		// invalid frame -- should always be at least as long as payload field
		return -EBADMSG;
	}

	env.dev = &xbee->wpan_dev;
	env.ieee_address = frame->ieee_address;
	env.network_address = be16toh( frame->network_address_be);
	env.source_endpoint = frame->source_endpoint;
	env.dest_endpoint = frame->dest_endpoint;
	env.cluster_id = be16toh( frame->cluster_id_be);
	env.profile_id = be16toh( frame->profile_id_be);
	env.options = 0;
	if (frame->options & XBEE_RX_OPT_BROADCAST)
	{
		env.options |= WPAN_ENVELOPE_BROADCAST_ADDR;
	}
	if (frame->options & XBEE_RX_OPT_APS_ENCRYPT)
	{
		env.options |= WPAN_ENVELOPE_RX_APS_ENCRYPT;
	}
	env.payload = frame->payload;
	env.length = length - offsetof( xbee_frame_receive_explicit_t, payload);

	return wpan_envelope_dispatch( &env);
}

/*** BeginHeader _xbee_endpoint_send */
int _xbee_endpoint_send( const wpan_envelope_t FAR *envelope, uint16_t flags);
/*** EndHeader */
/* START _FUNCTION DESCRIPTION *******************************************
_xbee_endpoint_send                     <xbee_wpan.c>

SYNTAX:
   int _xbee_endpoint_send( const wpan_envelope_t FAR *envelope, 
                            uint16_t flags)

DESCRIPTION:

     Sends data to an endpoint/profile/cluster on a remote WPAN
     node.  See wpan_endpoint_send_fn() for parameters and return values.

     User code should use wpan_envelope_send() instead of calling this
     function directly.

**************************************************************************/
xbee_wpan_debug
int _xbee_endpoint_send( const wpan_envelope_t FAR *envelope, uint16_t flags)
{
	xbee_dev_t *xbee;
	xbee_header_transmit_explicit_t	header;
	int error;

	// note that wpan_envelope_send() verifies that envelope is not NULL

	xbee = (xbee_dev_t *) envelope->dev;

	// Convert envelope to the necessary frame type and call xbee_frame_send
	header.frame_type = (uint8_t) XBEE_FRAME_TRANSMIT_EXPLICIT;
	header.frame_id = xbee_next_frame_id( xbee);
	header.ieee_address = envelope->ieee_address;
	header.network_address_be = htobe16( envelope->network_address);
	header.source_endpoint = envelope->source_endpoint;
	header.dest_endpoint = envelope->dest_endpoint;
	header.cluster_id_be = htobe16( envelope->cluster_id);
	header.profile_id_be = htobe16( envelope->profile_id);
	header.broadcast_radius = 0;
	header.options = (flags & WPAN_SEND_FLAG_ENCRYPTED)
														? XBEE_TX_OPT_APS_ENCRYPT : 0;

	#ifdef XBEE_WPAN_VERBOSE
		printf( "%s: %u bytes to 0x%04x "	\
			"(ep=%02x->%02x clust=%04x prof=%04x opt=%02x)\n",
			__FUNCTION__, envelope->length, envelope->network_address,
			envelope->source_endpoint, envelope->dest_endpoint,
			envelope->cluster_id, envelope->profile_id, header.options);
	#endif

	error = xbee_frame_write( xbee,
		&header, sizeof(header), envelope->payload, envelope->length, 0);

	#ifdef XBEE_WPAN_VERBOSE
		printf( "%s: %s returned %d\n", __FUNCTION__, "xbee_frame_write", error);
	#endif

	return error;
}

/*** BeginHeader xbee_wpan_init */
/*** EndHeader */
/* START _FUNCTION DESCRIPTION *******************************************
_xbee_wpan_tick                         <xbee_wpan.c>

SYNTAX:
   int _xbee_wpan_tick( wpan_dev_t *dev)

DESCRIPTION:
     Simple stub function for the wpan_dev_t object to map the generic
     "tick wpan_dev_t" call into a "tick xbee_dev_t" call.

     Note that this works because the xbee_dev_t structure starts with
     a wpan_dev_t structure, so the wpan_dev_t address is the same as the
     xbee_dev_t address.

     See wpan_tick_fn() for parameters and return value.

**************************************************************************/
xbee_wpan_debug
int _xbee_wpan_tick( wpan_dev_t *dev)
{
	return xbee_dev_tick( (xbee_dev_t *) dev);
}

/* START FUNCTION DESCRIPTION ********************************************
xbee_wpan_init                          <xbee_wpan.c>

SYNTAX:
   int xbee_wpan_init( xbee_dev_t *xbee, 
                       const wpan_endpoint_table_entry_t *ep_table)

DESCRIPTION:
     Configure xbee_dev_t for APS-layer (endpoint/cluster) networking.

     If using
     this layer, be sure to call wpan_tick (instead of xbee_dev_tick) so it can
     manage the APS layers of the network stack.


PARAMETER1:  xbee - device to configure
PARAMETER2:  ep_table - pointer to an endpoint table to use with device


RETURNS:  0        - success
          -EINVAL  - invalid parameter passed to function

**************************************************************************/
xbee_wpan_debug
int xbee_wpan_init( xbee_dev_t *xbee,
	const wpan_endpoint_table_entry_t *ep_table)
{
	if (xbee == NULL || ep_table == NULL)
	{
		return -EINVAL;
	}

//	xbee->wpan_dev.config = NULL;			// future use
	xbee->wpan_dev.tick = _xbee_wpan_tick;
	xbee->wpan_dev.endpoint_send = _xbee_endpoint_send;
	xbee->wpan_dev.endpoint_table = ep_table;

	return 0;
}


/*** BeginHeader _xbee_handle_transmit_status */
/*** EndHeader */
// see xbee/device.h for documentation
xbee_wpan_debug
int _xbee_handle_transmit_status( xbee_dev_t *xbee,
	const void FAR *payload, uint16_t length, void FAR *context)
{
	#ifdef XBEE_DEVICE_VERBOSE
		const xbee_frame_transmit_status_t FAR *frame = payload;
	#else
		XBEE_UNUSED_PARAMETER( payload);
	#endif

	// standard XBee frame handler; stub isn't using any parameters yet
	XBEE_UNUSED_PARAMETER( xbee);
	XBEE_UNUSED_PARAMETER( length);
	XBEE_UNUSED_PARAMETER( context);

	// it may be necessary to push information up to user code so they know when
	// a packet has been received or if it didn't make it out
	#ifdef XBEE_DEVICE_VERBOSE
		printf( "%s: id 0x%02x to 0x%04x retries=%d del=0x%02x disc=0x%02x\n",
			__FUNCTION__, frame->frame_id,
			be16toh( frame->network_address_be), frame->retries, frame->delivery,
			frame->discovery);
	#endif

	return 0;
}


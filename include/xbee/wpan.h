/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup xbee_wpan
	@{
	@file xbee/wpan.h
*/

#ifndef __XBEE_WPAN_H
#define __XBEE_WPAN_H

#include "xbee/device.h"
#include "wpan/types.h"

/// Format of XBee API frame type 0x90 (#XBEE_FRAME_RECEIVE);
/// received from XBee by host.
typedef struct xbee_frame_receive_t {
	uint8_t			frame_type;				///< XBEE_FRAME_RECEIVE (0x90)
	addr64			ieee_address;
	uint16_t			network_address_be;
	uint8_t			options;					///< bitfield, see XBEE_RX_OPT_xxx macros
	uint8_t			payload[1];				///< multi-byte payload
} xbee_frame_receive_t;


/// Format of XBee API frame type 0x91 (#XBEE_FRAME_RECEIVE_EXPLICIT);
/// received from XBee by host.
typedef struct xbee_frame_receive_explicit_t {
	uint8_t			frame_type;				///< XBEE_FRAME_RECEIVE_EXPLICIT (0x91)
	addr64			ieee_address;
	uint16_t			network_address_be;
	uint8_t			source_endpoint;
	uint8_t			dest_endpoint;
	uint16_t			cluster_id_be;
	uint16_t			profile_id_be;
	uint8_t			options;					///< bitfield, see XBEE_RX_OPT_xxx macros
	uint8_t			payload[1];				///< multi-byte payload
} xbee_frame_receive_explicit_t;

int xbee_wpan_init( xbee_dev_t *xbee,
	const wpan_endpoint_table_entry_t *ep_table);

#define XBEE_FRAME_HANDLE_RX_EXPLICIT	\
	{ XBEE_FRAME_RECEIVE_EXPLICIT, 0, _xbee_handle_receive_explicit, NULL }

int _xbee_handle_receive_explicit( xbee_dev_t *xbee, const void FAR *raw,
	uint16_t length, void FAR *context);

/// Format of XBee API frame type 0x10 (#XBEE_FRAME_TRANSMIT); sent
/// from host to XBee.  Note that the network stack does not include a
/// function for sending this frame type -- use an explicit transmit frame
/// instead (type 0x11) with WPAN_ENDPOINT_DIGI_DATA as the source and
/// destination endpoint, DIGI_CLUST_SERIAL as the cluster ID and
/// WPAN_PROFILE_DIGI as the profile ID.  Or, use the xbee_transparent_serial()
/// function from xbee_transparent_serial.c to fill in those fields and
/// send the data.
typedef struct xbee_header_transmit_t {
	uint8_t			frame_type;				///< XBEE_FRAME_TRANSMIT (0x10)
	uint8_t			frame_id;
	addr64			ieee_address;
	uint16_t			network_address_be;
	uint8_t			broadcast_radius;		///< set to 0 for maximum hop value
	uint8_t			options;					///< combination of XBEE_TX_OPT_* macros
} xbee_header_transmit_t;

/// Format of XBee API frame type 0x11 (#XBEE_FRAME_TRANSMIT_EXPLICIT); sent
/// from host to XBee.
typedef struct xbee_header_transmit_explicit_t {
	uint8_t			frame_type;				///< XBEE_FRAME_TRANSMIT_EXPLICIT (0x11)
	uint8_t			frame_id;
	addr64			ieee_address;
	uint16_t			network_address_be;
	uint8_t			source_endpoint;
	uint8_t			dest_endpoint;
	uint16_t			cluster_id_be;
	uint16_t			profile_id_be;
	uint8_t			broadcast_radius;		///< set to 0 for maximum hop value
	uint8_t			options;					///< combination of XBEE_TX_OPT_* macros
} xbee_header_transmit_explicit_t;

/*	@name
	Options for \c options field of xbee_header_transmit_t and
	xbee_header_transmit_explicit_t.
	@{
*/
#define XBEE_TX_OPT_DISABLE_ACK				0x01
#define XBEE_TX_OPT_APS_ENCRYPT				0x20
#define XBEE_TX_OPT_EXTENDED_TIMEOUT		0x40
//@}

typedef struct xbee_frame_transmit_status_t {
	uint8_t			frame_type;			//< XBEE_FRAME_TRANSMIT_STATUS (0x8B)
	uint8_t			frame_id;
	uint16_t			network_address_be;
	uint8_t			retries;			// # of application Tx retries
	uint8_t			delivery;
/** @name
	Values for \c delivery member of xbee_frame_transmit_status_t.
	@{
*/
		#define XBEE_TX_DELIVERY_SUCCESS						0x00
		#define XBEE_TX_DELIVERY_MAC_ACK_FAIL				0x01
		#define XBEE_TX_DELIVERY_CCA_FAIL					0x02
		#define XBEE_TX_DELIVERY_BAD_DEST_EP				0x15
		#define XBEE_TX_DELIVERY_NO_BUFFERS					0x18
		#define XBEE_TX_DELIVERY_NET_ACK_FAIL				0x21
		#define XBEE_TX_DELIVERY_NOT_JOINED					0x22
		#define XBEE_TX_DELIVERY_SELF_ADDRESSED			0x23
		#define XBEE_TX_DELIVERY_ADDR_NOT_FOUND			0x24
		#define XBEE_TX_DELIVERY_ROUTE_NOT_FOUND			0x25
		#define XBEE_TX_DELIVERY_BROADCAST_NOT_HEARD		0x26
		#define XBEE_TX_DELIVERY_INVALID_BINDING_INDX	0x2B
		#define XBEE_TX_DELIVERY_INVALID_EP					0x2C
		#define XBEE_TX_DELIVERY_RESOURCE_ERROR			0x32
		#define XBEE_TX_DELIVERY_PAYLOAD_TOO_BIG			0x74
		#define XBEE_TX_DELIVERY_INDIRECT_NOT_REQ			0x75
		#define XBEE_TX_DELIVERY_KEY_NOT_AUTHORIZED		0xBB
//@}

	uint8_t			discovery;		// bitfield
/** @name
	Values for \c discovery member of xbee_frame_transmit_status_t.
	@{
*/
		#define XBEE_TX_DISCOVERY_NONE				 0x00
		#define XBEE_TX_DISCOVERY_ADDRESS			 0x01
		#define XBEE_TX_DISCOVERY_ROUTE				 0x02
		#define XBEE_TX_DISCOVERY_EXTENDED_TIMEOUT 0x40
//@}

} xbee_frame_transmit_status_t;

/* START _FUNCTION DESCRIPTION *******************************************
_xbee_handle_transmit_status            <wpan.h>

SYNTAX:
   int _xbee_handle_transmit_status( xbee_dev_t *xbee,  const void FAR *frame, 
                                     uint16_t length,  void FAR *context)

DESCRIPTION:
     Dummy frame handler for 0x8B (XBEE_FRAME_TRANSMIT_STATUS) frames.
     Placeholder until we integrate processing of those frames into the stack.
SEE ALSO:  xbee_frame_handler_fn()


**************************************************************************/
int _xbee_handle_transmit_status( xbee_dev_t *xbee,
	const void FAR *frame, uint16_t length, void FAR *context);

#define XBEE_FRAME_HANDLE_TX_STATUS	\
	{ XBEE_FRAME_TRANSMIT_STATUS, 0, _xbee_handle_transmit_status, NULL }

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "xbee_wpan.c"
#endif

#endif

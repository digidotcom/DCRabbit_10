/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/**
	@addtogroup xbee_discovery
	@{
	@file xbee/discovery.h
	Header for code related to "Node Discovery" (the ATND command, 0x95 frames)
*/

#ifndef XBEE_DISCOVERY_H
#define XBEE_DISCOVERY_H

#include "xbee/platform.h"
#include "xbee/device.h"
#include "wpan/types.h"

#define XBEE_DISC_MAX_NODEID_LEN 20

typedef struct xbee_node_id1_t {
	uint16_t			network_addr_be;		///< ATMY value
	addr64			ieee_addr_be;			///< ATSH and ATSL
	/// ATNI value (variable length)
	char				node_info[XBEE_DISC_MAX_NODEID_LEN + 1];
} xbee_node_id1_t;

// data following variable-length node info
typedef struct xbee_node_id2_t {
	uint16_t			parent_addr_be;		// ATMP
	uint8_t			device_type;
		#define XBEE_ND_DEVICE_TYPE_COORD	0
		#define XBEE_ND_DEVICE_TYPE_ROUTER	1
		#define XBEE_ND_DEVICE_TYPE_ENDDEV	2
	uint8_t			status;					// reserved
	uint16_t			profile_be;				// always 0xC105
	uint16_t			manufacturer_be;		// always 0x101E
} xbee_node_id2_t;

// Node ID in host-byte-order and fixed length fields
typedef struct xbee_node_id_t {
	addr64			ieee_addr_be;		///< ATSH and ATSL
	uint16_t			network_addr;		///< ATMY value
	uint16_t			parent_addr;		///< ATMP
	/// one of XBEE_ND_DEVICE_TYPE_COORD, _ROUTER or _ENDDEV
	uint8_t			device_type;
	/// ATNI value (variable length)
	char				node_info[XBEE_DISC_MAX_NODEID_LEN + 1];
} xbee_node_id_t;

int xbee_disc_nd_parse( xbee_node_id_t FAR *parsed, const void FAR *source);
void xbee_disc_node_id_dump( const xbee_node_id_t FAR *ni);
const char *xbee_disc_device_type_str( uint8_t device_type);

typedef struct xbee_frame_node_id_t {
	uint8_t				frame_type;			///< XBEE_FRAME_NODE_ID (0x95)
	addr64				ieee_address_be;
	uint16_t				network_address_be;
	uint8_t				options;
	/// variable-length data, parse it with xbee_disc_nd_parse()
	xbee_node_id1_t	node_data;
} xbee_frame_node_id_t;

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "xbee_discovery.c"
#endif

#endif

/*
	Copyright (c)2011 Digi International Inc., All Rights Reserved

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

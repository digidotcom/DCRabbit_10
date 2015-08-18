/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/**
/**
	@addtogroup zdo
	@{
	@file zigbee/zdo.h

	ZigBee Device Objects (ZDO) and ZigBee Device Profile (ZDP).
*/
/*
	ZigBee Protocol Overview, Section 1.2.1.3:

	Fields that are longer than a single octet are sent to the PHY in order from
	the octet containing the lowest numbered bits to the octet containing the
	highest numbered bits.
*/

#ifndef __XBEE_ZDO_H
#define __XBEE_ZDO_H

#include "wpan/types.h"
#include "wpan/aps.h"

/// number of seconds to wait for ZDO responses before timing out
#define ZDO_CONVERSATION_TIMEOUT	15

/* START FUNCTION DESCRIPTION ********************************************
ZDO_ENDPOINT                            <zdo.h>

MACRO SYNTAX:
     ZDO_ENDPOINT( state)

DESCRIPTION:
     Macro for ZDO endpoint table entry.

     state is the name of a
     wpan_ep_state_t global used to track the ZDO endpoint's state.
     Can't just have the state declared in zigbee_zdo.c since a device with
     multiple XBee modules, will need multiple ZDO wpan_ep_state_t globals.

**************************************************************************/
// endpoint, profile, handler, context, device, version, cluster list
#define ZDO_ENDPOINT(state)												\
	{ WPAN_ENDPOINT_ZDO, WPAN_PROFILE_ZDO, zdo_handler,			\
		&state, 0x0000, 0x00, NULL }

// These functions are documented in zigbee_zdo.c.

wpan_ep_state_t FAR *zdo_endpoint_state( wpan_dev_t *dev);

int zdo_handler(
		const wpan_envelope_t	FAR *envelope,	// endpoint/profile/cluster/etc.
		wpan_ep_state_t			FAR *ep_state	// track transactions
	);

int zdo_send_response( const wpan_envelope_t FAR *request,
	const void FAR *response, uint16_t length);
int zdo_match_desc_request(void *buffer, int16_t buflen,
	uint16_t addr_of_interest, uint16_t profile_id,
	const uint16_t *inClust, const uint16_t *outClust);

/**	@name
	values for \c .request_type in NWK and IEEE address requests
*/
//@{
#define ZDO_REQUEST_TYPE_SINGLE		0x00
#define ZDO_REQUEST_TYPE_EXTENDED	0x01
//@}

/**	@name ZDO Mac Capability Flags
	@{
*/
#define ZDO_CAPABILITY_ALTERNATE_PAN	0x01
#define ZDO_CAPABILITY_FULL_FUNCTION	0x02
#define ZDO_CAPABILITY_MAINS_POWER		0x04
#define ZDO_CAPABILITY_RX_ON_IDLE		0x08
#define ZDO_CAPABILITY_RESERVED_1		0x10
#define ZDO_CAPABILITY_RESERVED_2		0x20
#define ZDO_CAPABILITY_SECURITY			0x40
#define ZDO_CAPABILITY_ALLOCATE_ADDR	0x80
//@}

/**	@name ZDO Status Values
	@{
*/
// Status values
#define ZDO_STATUS_SUCCESS					0x00
// 0x01 to 0x7F are reserved
#define ZDO_STATUS_INV_REQUESTTYPE		0x80
#define ZDO_STATUS_DEVICE_NOT_FOUND		0x81
#define ZDO_STATUS_INVALID_EP				0x82
#define ZDO_STATUS_NOT_ACTIVE				0x83
#define ZDO_STATUS_NOT_SUPPORTED			0x84
#define ZDO_STATUS_TIMEOUT					0x85
#define ZDO_STATUS_NO_MATCH				0x86
// 0x87 is reserved
#define ZDO_STATUS_NO_ENTRY				0x88
#define ZDO_STATUS_NO_DESCRIPTOR			0x89
#define ZDO_STATUS_INSUFFICIENT_SPACE	0x8A
#define ZDO_STATUS_NOT_PERMITTED			0x8B
#define ZDO_STATUS_TABLE_FULL				0x8C
#define ZDO_STATUS_NOT_AUTHORIZED		0x8D
//@}

/// Cluster IDs with the high bit set are responses.
#define ZDO_CLUST_RESPONSE_MASK			0x8000

#define ZDO_CLUST_IS_RESPONSE(c)		(c & ZDO_CLUST_RESPONSE_MASK)

/*********************************************************
					NWK_addr Descriptor
**********************************************************/
/// cluster ID for ZDO NWK_addr request
#define ZDO_NWK_ADDR_REQ		0x0000
/// cluster ID for ZDO NWK_addr response
#define ZDO_NWK_ADDR_RSP		0x8000
/// frame format for ZDO NKW_addr request
typedef struct zdo_nwk_addr_req_t {
	uint8_t		transaction;
	addr64		ieee_address_le;
	uint8_t		request_type;		///< See ZDO_REQUEST_TYPE_* macros
	uint8_t		start_index;
} zdo_nwk_addr_req_t;

typedef struct zdo_nwk_addr_rsp_header_t {
	uint8_t		status;
	addr64		ieee_remote_le;
	uint16_t		net_remote_le;
	uint8_t		num_assoc_dev;
	uint8_t		start_index;
	// followed by variable-length list of 16-bit associated device addresses
} zdo_nwk_addr_rsp_header_t;

#define ZDO_NET_ADDR_PENDING			0xFFFE
#define ZDO_NET_ADDR_TIMEOUT			0xFFFF
#define ZDO_NET_ADDR_ERROR				0xFFFD

/* START FUNCTION DESCRIPTION ********************************************
zdo_send_nwk_addr_req                   <zdo.h>

SYNTAX:
   int zdo_send_nwk_addr_req( wpan_dev_t *dev,  const addr64 FAR *ieee_be, 
                              uint16_t FAR *net)

DESCRIPTION:
     Given a device's IEEE (64-bit) address, get its 16-bit network
     address by unicasting a ZDO NWK_addr request to it.

     After calling this function, <>*net is set to ZDO_NET_ADDR_PENDING.

     When a SUCCESS response comes back, <>*net is set to the 16-bit network
     address in the response.

     When an ERROR response comes back, <>*net is set to ZDO_NET_ADDR_ERROR.

     If a timeout occurs waiting for a response, <>*net is set to
     ZDO_NET_ADDR_TIMEOUT.

     (So the caller needs to wait until (*net != ZDO_NET_ADDR_PENDING)).


PARAMETER1:  dev - wpan_dev_t to send request
PARAMETER2:  ieee_be - IEEE address (in big-endian byte order) of device
              we're seeking a network address for
PARAMETER3:  net - location to store the 16-bit network address when the
              NWK_addr response comes back.


RETURNS:  -EINVAL  - invalid parameter passed to function
          -ENOSPC  - conversation table is full, wait and try sending later
          0        - request sent
          !0			error trying to send request

**************************************************************************/
int zdo_send_nwk_addr_req( wpan_dev_t *dev, const addr64 FAR *ieee_be,
	uint16_t FAR *net);

/*********************************************************
					IEEE_addr Descriptor
**********************************************************/
/// cluster ID for ZDO IEEE_addr request
#define ZDO_IEEE_ADDR_REQ		0x0001
/// cluster ID for ZDO IEEE_addr response
#define ZDO_IEEE_ADDR_RSP		0x8001
/// frame format for ZDO IEEE_addr request
typedef struct zdo_ieee_addr_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
	uint8_t		request_type;		///< See ZDO_REQUEST_TYPE_* macros
	uint8_t		start_index;
} zdo_ieee_addr_req_t;

typedef zdo_nwk_addr_rsp_header_t zdo_ieee_addr_rsp_header_t;

#define ZDO_IEEE_ADDR_PENDING			WPAN_IEEE_ADDR_UNDEFINED
#define ZDO_IEEE_ADDR_TIMEOUT			WPAN_IEEE_ADDR_BROADCAST
#define ZDO_IEEE_ADDR_ERROR			WPAN_IEEE_ADDR_ALL_ZEROS

/* START _FUNCTION DESCRIPTION *******************************************
_zdo_send_ieee_addr_req                 <zdo.h>

SYNTAX:
   int _zdo_send_ieee_addr_req( wpan_dev_t *dev,  uint16_t net_addr, 
                                addr64 FAR *ieee_be)

DESCRIPTION:


     address by unicasting a ZDO IEEE_addr request to it.

   NOTE: This API has not been implemented, due to a limitation of the XBee
     firmware.  It is not possible to send unicast messages without an IEEE
     address (which is necessary in this case).

     After calling this function, <>*ieee_be is set to *ZDO_IEEE_ADDR_PENDING.

     When a SUCCESS response comes back, <>*ieee_be is set to the 64-bit IEEE
     address in the response.

     If an ERROR response comes back, <>*ieee_be is set to all zeros
     (*ZDO_IEEE_ADDR_ERROR).

     If a timeout occurs waiting for a response, <>*ieee_be is set to
     *ZDO_IEEE_ADDR_TIMEOUT.


     addr64 ieee_be;		// IEEE address in big-endian byte order
     uint16_t net_addr;

     printf( "Sending IEEE_addr request for 0x%04X\n", net_addr);
     zdo_send_ieee_addr_req( dev, net_addr, &ieee_be);
     do {
     wpan_tick( dev);
     } while (addr64_equal( &ieee_be, ZDO_IEEE_ADDR_PENDING));

     if (addr64_equal( &ieee_be, ZDO_IEEE_ADDR_TIMEOUT))
     {
     printf( "IEEE_addr request for 0x%04X timed out\n", net_addr);
     }
     else if (addr64_is_zero( &ieee_be))
     {
     printf( "Error retrieving IEEE_addr for 0x%04X\n", net_addr);
     }
     else
     {
     printf( "IEEE address of 0x%04X is %" PRIsFAR "\n", net_addr,
     addr64_format( buffer, &ieee_be));
     }



PARAMETER1:  dev - wpan_dev_t to send request
PARAMETER2:  net_addr - network address of device we're seeking an IEEE
              address for
PARAMETER3:  ieee_be - location to store the 64-bit IEEE address when the
              IEEE_addr response comes back.


RETURNS:  -EINVAL  - invalid parameter passed to function
          -ENOSPC  - conversation table is full, wait and try sending later
          0        - request sent
          !0			error trying to send request

**************************************************************************/
int _zdo_send_ieee_addr_req( wpan_dev_t *dev, uint16_t net_addr,
	addr64 FAR *ieee_be);

/*********************************************************
					Node Descriptor
**********************************************************/
/// cluster ID for ZDO Node_Desc request
#define ZDO_NODE_DESC_REQ		0x0002
/// cluster ID for ZDO Node_Desc response
#define ZDO_NODE_DESC_RSP		0x8002
/// frame format for ZDO Node_Desc request
/// @see zdo_send_descriptor_req()
typedef struct zdo_node_desc_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
} zdo_node_desc_req_t;

/// frame format for ZDO Node_Desc response
typedef struct zdo_node_desc_t {
	uint8_t		flags0;
	uint8_t		flags1;
	uint8_t		mac_capability;
	uint16_t		manufacturer_le;
	uint8_t		max_buffer;
	uint16_t		max_incoming_le;
	uint16_t		server_mask_le;
	uint16_t		max_outgoing_le;
	uint8_t		desc_capability;
} zdo_node_desc_t;

#define ZDO_NODE_DESC_FLAGS0_TYPE_MASK						0x07
#define ZDO_NODE_DESC_FLAGS0_TYPE_COORD					0x00
#define ZDO_NODE_DESC_FLAGS0_TYPE_ROUTER					0x01
#define ZDO_NODE_DESC_FLAGS0_TYPE_ENDDEV					0x02

#define ZDO_NODE_DESC_FLAGS0_COMPLEX_DESC_AVAIL			0x08
#define ZDO_NODE_DESC_FLAGS0_USER_DESC_AVAIL				0x10

#define ZDO_NODE_DESC_FLAGS0_RESERVED						0xE0

#define ZDO_NODE_DESC_FLAGS1_APS_FIELD						0x07
#define ZDO_NODE_DESC_FLAGS1_FREQUENCY_MASK				0xF8

#define ZDO_NODE_DESC_FLAGS1_FREQ_868						(1<<0)
#define ZDO_NODE_DESC_FLAGS1_FREQ_RESERVED_1				(1<<1)
#define ZDO_NODE_DESC_FLAGS1_FREQ_900						(1<<2)
#define ZDO_NODE_DESC_FLAGS1_FREQ_2400						(1<<3)
#define ZDO_NODE_DESC_FLAGS1_FREQ_RESERVED_4				(1<<4)

/**	@name ZDO Node Descriptor - MAC Capability Flags
	@{
*/
#define ZDO_NODE_DESC_MAC_CAPABILITY_ALT_PAN_COORD		(1<<0)
#define ZDO_NODE_DESC_MAC_CAPABILITY_DEVICE_TYPE		(1<<1)
#define ZDO_NODE_DESC_MAC_CAPABILITY_POWER_SOURCE		(1<<2)
#define ZDO_NODE_DESC_MAC_CAPABILITY_RX_ON_IDLE			(1<<3)
#define ZDO_NODE_DESC_MAC_CAPABILITY_RESERVED_4			(1<<4)
#define ZDO_NODE_DESC_MAC_CAPABILITY_RESERVED_5			(1<<5)
#define ZDO_NODE_DESC_MAC_CAPABILITY_SECURITY			(1<<6)
#define ZDO_NODE_DESC_MAC_CAPABILITY_ALLOCATE_ADDR		(1<<7)
//@}

/**	@name ZDO Node Descriptor - Server Mask Bit Assignments
	@{
*/
#define ZDO_NODE_DESC_SERVER_MASK_PRIMARY_TRUST			(1<<0)
#define ZDO_NODE_DESC_SERVER_MASK_BACKUP_TRUST			(1<<1)
#define ZDO_NODE_DESC_SERVER_MASK_PRIMARY_BINDING		(1<<2)
#define ZDO_NODE_DESC_SERVER_MASK_BACKUP_BINDING		(1<<3)
#define ZDO_NODE_DESC_SERVER_MASK_PRIMARY_DISCOVERY	(1<<4)
#define ZDO_NODE_DESC_SERVER_MASK_BACKUP_DISCOVERY		(1<<5)
#define ZDO_NODE_DESC_SERVER_MASK_NETWORK_MANAGER		(1<<6)
// bits 7 through 15 are reserved
//@}

typedef struct zdo_node_desc_resp_t {
	uint8_t					status;					///< see ZDO_STATUS_* macros
	uint16_t					network_addr_le;
	zdo_node_desc_t		node_desc;
} zdo_node_desc_resp_t;

/*********************************************************
					Power Descriptor
**********************************************************/
/// cluster ID for ZDO Power_Desc request
#define ZDO_POWER_DESC_REQ		0x0003
/// cluster ID for ZDO Power_Desc response
#define ZDO_POWER_DESC_RSP		0x8003
/// frame format for ZDO Power_Desc request
/// @see zdo_send_descriptor_req()
typedef struct zdo_power_desc_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
} zdo_power_desc_req_t;

/** @name PowerDescriptor macros
	macros used in PowerDescriptor (zdo_power_desc_t)
*/
//@{
#define ZDO_POWER_SOURCE_MAINS			0x01
#define ZDO_POWER_SOURCE_RECHARGABLE	0x02
#define ZDO_POWER_SOURCE_DISPOSABLE		0x04
#define ZDO_POWER_SOURCE_RESERVED		0x08

// for .power0, choose one of the following...
#define ZDO_POWER0_SYNC						0x00
#define ZDO_POWER0_PERIODIC				0x01
#define ZDO_POWER0_STIMULATED				0x02

// ... and choose one or more of the following
#define ZDO_POWER0_AVAIL_MAINS			(ZDO_POWER_SOURCE_MAINS << 4)
#define ZDO_POWER0_AVAIL_RECHARGABLE	(ZDO_POWER_SOURCE_RECHARGABLE << 4)
#define ZDO_POWER0_AVAIL_DISPOSABLE		(ZDO_POWER_SOURCE_DISPOSABLE << 4)
#define ZDO_POWER0_AVAIL_RESERVED		(ZDO_POWER_SOURCE_RESERVED << 4)

// for .power 1, choose one of the following...
#define ZDO_POWER1_LEVEL_CRITICAL		0x00
#define ZDO_POWER1_LEVEL_33				0x40
#define ZDO_POWER1_LEVEL_66				0x80
#define ZDO_POWER1_LEVEL_FULL				0xC0

// ... and choose one or more of the following
#define ZDO_POWER1_CURRENT_MAINS			ZDO_POWER_SOURCE_MAINS
#define ZDO_POWER1_CURRENT_RECHARGABLE	ZDO_POWER_SOURCE_RECHARGABLE
#define ZDO_POWER1_CURRENT_DISPOSABLE	ZDO_POWER_SOURCE_DISPOSABLE
#define ZDO_POWER1_CURRENT_RESERVED		ZDO_POWER_SOURCE_RESERVED
//@}

/// format for ZDO PowerDescriptor
typedef struct zdo_power_desc_t {
	uint8_t		power0;		///< combination of ZDO_POWER0_* macros
	uint8_t		power1;		///< combination of ZDO_POWER1_* macros
} zdo_power_desc_t;

/// frame format for ZDO Power_Desc response
typedef struct zdo_power_desc_rsp_t {
	uint8_t					status;					///< see ZDO_STATUS_* macros
	uint16_t					network_addr_le;
	zdo_power_desc_t		power_desc;
} zdo_power_desc_rsp_t;


/*********************************************************
					Simple Descriptor
**********************************************************/
/// cluster ID for ZDO Simple_Desc request
#define ZDO_SIMPLE_DESC_REQ	0x0004
/// cluster ID for ZDO Simple_Desc response
#define ZDO_SIMPLE_DESC_RSP	0x8004
/// frame format for ZDO Simple_Desc request
typedef struct zdo_simple_desc_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
	uint8_t		endpoint;			///< 0x01 to 0xFE
} zdo_simple_desc_req_t;

/// header for ZDO Simple_Desc response, followed by a SimpleDescriptor
typedef struct zdo_simple_desc_resp_header_t {
	uint8_t		status;				///< see ZDO_STATUS_* macros
	uint16_t		network_addr_le;	///< device's network address (little-endian)
	uint8_t		length;				///< length of simple descriptor
	// variable-length simple descriptor follows
} zdo_simple_desc_resp_header_t;

/// header for ZDO SimpleDescriptor (part of a Simple_Desc response), followed
/// by uint8_t input cluster count, multiple uint16_t input cluster IDs,
/// uint8_t output cluster count, multiple uint16_t output cluster IDs
typedef struct zdo_simple_desc_header_t {
	uint8_t		endpoint;			///< 0x01 to 0xFE
	uint16_t		profile_id_le;		///< endpoint's profile ID (little-endian)
	uint16_t		device_id_le;		///< endpoint's device ID (little-endian)
	uint8_t		device_version;	///< upper 4 bits are reserved
	// variable-length cluster counts and ids follow
} zdo_simple_desc_header_t;

/* START FUNCTION DESCRIPTION ********************************************
zdo_simple_desc_request                 <zdo.h>

SYNTAX:
   int zdo_simple_desc_request( wpan_envelope_t *envelope, 
                                uint16_t addr_of_interest, 
                                uint_fast8_t endpoint, 
                                wpan_response_fn callback, 
                                const void FAR *context)

DESCRIPTION:
     Send a ZDO Simple Descriptor Request.

     The simple descriptor contains information specific to each of a node's
     endpoints.  Use the ZDO Simple Descriptor Request to get a descriptor
     for an endpoint on a remote node.


PARAMETER1:  envelope - Envelope created with wpan_envelope_create().
              Only dev, ieee_address and network_address
              should be set, all other structure elements should be zero.  Address
              may match <addr_of_interest> or an alternative device that contains
              the discovery information of that device (like an end device's
              parent).

PARAMETER2:  addr_of_interest - Network address of the device for which the
              simple descriptor is required.

PARAMETER3:  endpoint - Endpoint of interest, a value from 1 to 254.

PARAMETER4:  callback - Function to receive the Simple Descriptor
              Response.  See documentation for wpan_response_fn
              for this callback's API.

PARAMETER5:  context - Context pointer passed to the callback along
              with the response.


RETURNS:  0        - request sent
          !0	error sending request

**************************************************************************/
int zdo_simple_desc_request( wpan_envelope_t *envelope,
	uint16_t addr_of_interest, uint_fast8_t endpoint,
	wpan_response_fn callback, const void FAR *context);

/*********************************************************
					Active EP Descriptor
**********************************************************/
/// cluster ID for ZDO Active_EP request
#define ZDO_ACTIVE_EP_REQ		0x0005
/// cluster ID for ZDO Active_EP response
#define ZDO_ACTIVE_EP_RSP		0x8005
/// frame format for ZDO Active_EP request
/// @see zdo_send_descriptor_req()
typedef struct zdo_active_ep_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
} zdo_active_ep_req_t;

/// header for ZDO Active_EP response, followed by \c .ep_count
/// uint8_t endpoints
typedef struct zdo_active_ep_rsp_header_t {
	uint8_t		status;					///< see ZDO_STATUS_* macros
	uint16_t		network_addr_le;
	uint8_t		ep_count;
} zdo_active_ep_rsp_header_t;


/*********************************************************
					Match Descriptor
**********************************************************/
/// cluster ID for ZDO Match_Desc request
#define ZDO_MATCH_DESC_REQ		0x0006
/// cluster ID for ZDO Match_Desc response
#define ZDO_MATCH_DESC_RSP		0x8006
/// header for ZDO Match_Desc request
typedef struct zdo_match_desc_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
	uint16_t		profile_id_le;
	uint8_t		num_in_clusters;
	uint16_t		in_cluster_list[1];		///< variable length
} zdo_match_desc_req_t;

/// Second half of Match_Desc request.
/// cast to address of zdo_match_desc_req_t
///			+ offsetof \c .in_cluster_list + 2 * \c .num_in_clusters
typedef struct zdo_match_desc_out_clust_t {
	uint8_t		num_out_clusters;
	uint16_t		out_cluster_list[1];		///< variable length
} zdo_match_desc_out_clust_t;

/// header for ZDO Match_Desc response, followed by \c .match_length
/// uint8_t endpoints
typedef struct zdo_match_desc_rsp_header_t {
	uint8_t		status;					///< see ZDO_STATUS_* macros
	uint16_t		network_addr_le;
	uint8_t		match_length;
} zdo_match_desc_rsp_header_t;		// followed by <match_length> bytes


/*********************************************************
					Complex Descriptor
**********************************************************/
/// cluster ID for ZDO Complex_Desc request
#define ZDO_COMPLEX_DESC_REQ		0x0007
/// cluster ID for ZDO Complex_Desc response
#define ZDO_COMPLEX_DESC_RSP		0x8007
/// frame format for ZDO Complex_Desc request
/// @see zdo_send_descriptor_req()
typedef struct zdo_complex_desc_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
} zdo_complex_desc_req_t;


/*********************************************************
					User Descriptor
**********************************************************/
/// cluster ID for ZDO User_Desc request
#define ZDO_USER_DESC_REQ			0x0011
/// cluster ID for ZDO User_Desc response
#define ZDO_USER_DESC_RSP			0x8011
/// frame format for ZDO Complex_Desc request
/// @see zdo_send_descriptor_req()
typedef struct zdo_user_desc_req_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
} zdo_user_desc_req_t;


/*********************************************************
					Device Announce
**********************************************************/
/// cluster ID for ZDO Device_annce announcement
#define ZDO_DEVICE_ANNCE			0x0013
// Device_annce does not have a response (0x8013 is not used)
/// frame format for a ZDO Device_annce announcement
typedef struct zdo_device_annce_t {
	uint8_t		transaction;
	uint16_t		network_addr_le;
	addr64		ieee_address_le;
	uint8_t		capability;		///< see ZDO_CAPABILITY_* macros
} zdo_device_annce_t;

/* START FUNCTION DESCRIPTION ********************************************
*zdo_device_annce_fn                    <zdo.h>

SYNTAX:
   typedef void (*zdo_device_annce_fn)( const wpan_envelope_t FAR *envelope, 
                                        const zdo_device_annce_t FAR *annce)

DESCRIPTION:
     Signature for a function to receive ZDO Device Announce frames.

     Define ZDO_DEVICE_ANNCE_HANDLER to a function to receive these frames.

     If the network stack is a pre-compiled library, the program will need
     to provide a function to process the device announcements, or set the
     handler to NULL.




PARAMETER1:  envelope - envelope from request
PARAMETER2:  annce - ZDO Device Announcement

**************************************************************************/
typedef void (*zdo_device_annce_fn)( const wpan_envelope_t FAR *envelope,
	const zdo_device_annce_t FAR *annce);

#ifdef ZDO_DEVICE_ANNCE_HANDLER
	const zdo_device_annce_fn ZDO_DEVICE_ANNCE_HANDLER;
#endif

/*********************************************************
					Bind/Unbind Request
**********************************************************/
/// cluster ID for ZDO Bind request
#define ZDO_BIND_REQ					0x0021
/// cluster ID for ZDO Bind response
#define ZDO_BIND_RSP					0x8021
/// cluster ID for ZDO Unbind request
#define ZDO_UNBIND_REQ				0x0022
/// cluster ID for ZDO Unbind response
#define ZDO_UNBIND_RSP				0x8022

typedef struct zdo_bind_req_header_t {
	addr64		src_address_le;
	uint8_t		src_endpoint;
	uint16_t		cluster_id_le;
	uint8_t		dst_addr_mode;
} zdo_bind_req_header_t;

/// frame format for a ZDO Bind Request with a group address as the
/// destination (header.dst_addr_mode == ZDO_BIND_DST_MODE_GROUP)
typedef struct zdo_bind_group_req_t {
	zdo_bind_req_header_t	header;
	uint16_t						group_addr_le;
} zdo_bind_group_req_t;

/// frame format for a ZDO Bind Request with an IEEE address and endpoint
/// as the destination (header.dst_addr_mode == ZDO_BIND_DST_MODE_ADDR)
typedef struct zdo_bind_address_req_t {
	zdo_bind_req_header_t	header;
	addr64						dst_address_le;
	uint8_t						dst_endpoint;
} zdo_bind_address_req_t;

typedef union zdo_bind_req_t {
	zdo_bind_req_header_t	header;
	zdo_bind_group_req_t		group;
	zdo_bind_address_req_t	address;
} zdo_bind_req_t;

/// zdo_bind_req_t contains 16-bit group address
#define ZDO_BIND_DST_MODE_GROUP			0x01
/// zdo_bind_req_t contains 64-bit destination address and endpoint
#define ZDO_BIND_DST_MODE_ADDR			0x03

/* START FUNCTION DESCRIPTION ********************************************
zdo_send_bind_req                       <zdo.h>

SYNTAX:
   int zdo_send_bind_req( wpan_envelope_t *envelope,  uint16_t type, 
                          wpan_response_fn callback,  void FAR *context)

DESCRIPTION:
     Send a ZDO Bind (or Unbind) Request to the destination address in the
     envelope.

     Binds .dest_endpoint on .ieee_address to
     .source_endpoint on .dev using .cluster_id.

     Ignores the .options, .payload, and .length members of the
     envelope.


PARAMETER1:  envelope - addressing information used for the Bind Request
PARAMETER2:  type - ZDO_BIND_REQ for a Bind Request or ZDO_UNBIND_REQ
              for an Unbind Request; all other values are invalid
PARAMETER3:  callback - callback to receive Bind/Unbind (or Default)
              Response; NULL if you don't care about the response
PARAMETER4:  context - context passed to callback


RETURNS:  0        - successfully sent request
          -EINVAL  - bad parameter passed to function
          !0			error sending request

**************************************************************************/
int zdo_send_bind_req( wpan_envelope_t *envelope, uint16_t type,
	wpan_response_fn callback, void FAR *context);


/*********************************************************
					Management Leave Request
**********************************************************/
/// cluster ID for ZDO Management Leave Request
#define ZDO_MGMT_LEAVE_REQ			0x0034
/// cluster ID for ZDO Management Leave Response
#define ZDO_MGMT_LEAVE_RSP			0x8034

/// frame format for a ZDO Management Leave Request
typedef struct zdo_mgmt_leave_req_t {
	addr64		device_address;
	uint8_t		flags;
		#define ZDO_MGMT_LEAVE_REQ_FLAG_NONE					0x00
		#define ZDO_MGMT_LEAVE_REQ_FLAG_REJOIN					0x01
		#define ZDO_MGMT_LEAVE_REQ_FLAG_REMOVE_CHILDREN		0x02
} zdo_mgmt_leave_req_t;

typedef struct zdo_mgmt_leave_rsp_t {
	uint8_t		status;
} zdo_mgmt_leave_rsp_t;

/// extra sending option for zdo_mgmt_leave_req
#define ZDO_MGMT_LEAVE_REQ_ENCRYPTED		0x0100
/* START FUNCTION DESCRIPTION ********************************************
zdo_mgmt_leave_req                      <zdo.h>

SYNTAX:
   int zdo_mgmt_leave_req( wpan_dev_t *dev,  const addr64 *address, 
                           uint16_t flags)

DESCRIPTION:
     Send a ZDO Management Leave Request.


PARAMETER1:  dev - device to send request on
PARAMETER2:  address - address to send request to, or NULL for self-addressed
PARAMETER3:  flags - one or more of the following flags:
              - ZDO_MGMT_LEAVE_REQ_FLAG_NONE
              - ZDO_MGMT_LEAVE_REQ_FLAG_REMOVE_CHILDREN	- set the Remove Children
              flag in the ZDO request
              - ZDO_MGMT_LEAVE_REQ_FLAG_REJOIN - set the Rejoin flag in the ZDO
              request
              - ZDO_MGMT_LEAVE_REQ_ENCRYPTED - send the request with APS encryption


RETURNS:  0        - successfully sent request
          -EINVAL  - bad parameter passed to function
          !0			error sending request

**************************************************************************/
int zdo_mgmt_leave_req( wpan_dev_t *dev, const addr64 *address, uint16_t flags);


/* START FUNCTION DESCRIPTION ********************************************
zdo_send_descriptor_req                 <zdo.h>

SYNTAX:
   int zdo_send_descriptor_req( wpan_envelope_t *envelope,  uint16_t cluster, 
                                uint16_t addr_of_interest, 
                                wpan_response_fn callback, 
                                const void FAR *context)

DESCRIPTION:
     Send a ZDO Node, Power, Complex or User Descriptor request,
     or an Active Endpoint request.


PARAMETER1:  envelope - Envelope created by wpan_envelope_create; this
              function will fill in the <cluster> and reset the
              <payload> and <length>.
PARAMETER2:  cluster - Any ZDO request with a transaction and 16-bit network
              address as its only fields, including:
              - ZDO_NODE_DESC_REQ
              - ZDO_POWER_DESC_REQ
              - ZDO_ACTIVE_EP_REQ
              - ZDO_COMPLEX_DESC_REQ
              - ZDO_USER_DESC_REQ
PARAMETER3:  addr_of_interest - address to use in ZDO request
PARAMETER4:  callback - function to receive response
PARAMETER5:  context - context to pass to <callback> with response


RETURNS:  !0		error sending request
          0        - request sent

**************************************************************************/
int zdo_send_descriptor_req( wpan_envelope_t *envelope, uint16_t cluster,
	uint16_t addr_of_interest, wpan_response_fn callback,
	const void FAR *context);


// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "zigbee_zdo.c"
#endif

#endif		// __XBEE_ZDO_H

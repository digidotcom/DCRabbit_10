/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup zcl_client
	@{
	@file zcl_client.c

	Code to support ZCL client clusters.
*/

/*** BeginHeader */
#include <stddef.h>
#include <stdio.h>

#include "xbee/platform.h"
#include "zigbee/zdo.h"
#include "zigbee/zcl.h"
#include "zigbee/zcl_client.h"

#ifndef __DC__
	#define zcl_client_debug
#elif defined ZCL_CLIENT_DEBUG
	#define zcl_client_debug		__debug
#else
	#define zcl_client_debug		__nodebug
#endif
/*** EndHeader */

/*** BeginHeader zcl_process_read_attr_response */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
zcl_process_read_attr_response          <zcl_client.c>

SYNTAX:
   int zcl_process_read_attr_response( zcl_command_t *zcl, 
                                   const zcl_attribute_base_t FAR *attr_table)

DESCRIPTION:
     Process a Read Attributes Response and populate the attibute values into
     an attribute table as if it was a Write Attributes Request.

     Used in ZCL clients that want to read a lot of ZCL attributes.  The
     client has a mirrored copy of the attributes on the target, and this
     function is used to populate that copy using the Read Attributes Responses.


PARAMETER1:  zcl - ZCL command to process
PARAMETER2:  attr_list - start of the attribute list to use for storing
              attribute responses


RETURNS:  ZCL status value to send in a default response

**************************************************************************/
zcl_client_debug
int zcl_process_read_attr_response( zcl_command_t *zcl,
	const zcl_attribute_base_t FAR *attr_table)
{
	const uint8_t					FAR *payload_end;
	zcl_attribute_write_rec_t write_rec;

	if (zcl == NULL || attr_table == NULL)
	{
		return ZCL_STATUS_FAILURE;
	}

	write_rec.buffer = zcl->zcl_payload;
	payload_end = write_rec.buffer + zcl->length;
	write_rec.status = ZCL_STATUS_SUCCESS;
	while (write_rec.status == ZCL_STATUS_SUCCESS
			&& write_rec.buffer < payload_end)
	{
		write_rec.flags = ZCL_ATTR_WRITE_FLAG_ASSIGN
								| ZCL_ATTR_WRITE_FLAG_READ_RESP;
		write_rec.buflen = (int16_t)(payload_end - write_rec.buffer);
		zcl_parse_attribute_record( attr_table, &write_rec);
	}

	return write_rec.status;
}


/*** BeginHeader zcl_client_read_attributes */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
zcl_client_read_attributes              <zcl_client.c>

SYNTAX:
   int zcl_client_read_attributes( wpan_envelope_t FAR *envelope, 
                                   const zcl_client_read_t *client_read)

DESCRIPTION:
     Send a Read Attributes request for attributes listed in <client_read>.


PARAMETER1:  envelope - addressing information for sending a ZCL Read
              Attributes request; this function fills in the
              payload and length
PARAMETER2:  client_read - data structure used by ZCL clients to do
              ZDO discovery followed by ZCL attribute reads


RETURNS:  0        - successfully sent request
          !0		error sending request

**************************************************************************/
zcl_client_debug
int zcl_client_read_attributes( wpan_envelope_t FAR *envelope,
	const zcl_client_read_t *client_read)
{
   struct {
		zcl_header_response_t	header;
		uint16_t						attrib_le[20];
	} zcl_req;
	uint8_t							*request_start;
	const uint16_t			FAR *attrib_src;
	uint16_t							*attrib_dst_le;
	uint_fast8_t					i;

   // Generate a Read Attributes request for attributes in list
	if (client_read->mfg_id == ZCL_MFG_NONE)
	{
		zcl_req.header.u.std.frame_control = ZCL_FRAME_TYPE_PROFILE |
			ZCL_FRAME_GENERAL | ZCL_FRAME_CLIENT_TO_SERVER;
		request_start = &zcl_req.header.u.std.frame_control;
	}
	else
	{
		zcl_req.header.u.mfg.mfg_code_le = htole16( client_read->mfg_id);
		zcl_req.header.u.mfg.frame_control = ZCL_FRAME_TYPE_PROFILE |
			ZCL_FRAME_MFG_SPECIFIC | ZCL_FRAME_CLIENT_TO_SERVER;
		request_start = &zcl_req.header.u.mfg.frame_control;
	}
	zcl_req.header.sequence = wpan_endpoint_next_trans( client_read->ep);
	zcl_req.header.command = ZCL_CMD_READ_ATTRIB;

  // copy attribute list into request
	attrib_src = client_read->attribute_list;
	attrib_dst_le = zcl_req.attrib_le;
	for (i = 20; i && *attrib_src != ZCL_ATTRIBUTE_END_OF_LIST; --i)
	{
		*attrib_dst_le = htole16( *attrib_src);
		++attrib_dst_le;
		++attrib_src;
	}

	envelope->payload = request_start;
	envelope->length = (uint8_t *)attrib_dst_le - request_start;

	#ifdef ZCL_CLIENT_VERBOSE
		printf( "%s: sending %d-byte ZCL request\n", __FUNCTION__,
			envelope->length);
		hex_dump( envelope->payload, envelope->length,
			HEX_DUMP_FLAG_TAB);
	#endif

	return wpan_envelope_send( envelope);
}

/*** BeginHeader zcl_find_and_read_attributes */
/*** EndHeader */
/* START _FUNCTION DESCRIPTION *******************************************
_zcl_send_read_from_zdo_match           <zcl_client.c>

SYNTAX:
   int _zcl_send_read_from_zdo_match( wpan_conversation_t FAR *conversation, 
                                      const wpan_envelope_t FAR *envelope)

DESCRIPTION:

     Process responses to ZDO Match_Desc broadcast sent by zdo_send_match_desc()
     and generate ZCL Read Attributes request for the attributes in the
     related conversation's context (a pointer to a zcl_client_read_t object).

     The Read Attributes Responses are handled by the function registered to the
     cluster in the endpoint table.


PARAMETER1:  conversation - matching entry in converstation table (which
              contains a context pointer)
PARAMETER2:  envelope - envelope of response or NULL if conversation
              timed out


RETURNS:  0        - send ZCL request for current time
          !0			error sending ZCL request for current time



**************************************************************************/
zcl_client_debug
int _zcl_send_read_from_zdo_match( wpan_conversation_t FAR *conversation,
	const wpan_envelope_t FAR *envelope)
{
	const struct {
		uint8_t								transaction_id;
		zdo_match_desc_rsp_header_t	header;
		uint8_t								endpoint;
	}									FAR *zdo_rsp;
	zcl_client_read_t						client_read;
	wpan_envelope_t						reply_envelope;		// for sending ZCL request
	const wpan_cluster_table_entry_t *cluster;

	// note that conversation is never NULL in wpan_response_fn callbacks

	if (envelope == NULL)
	{
		// conversation timed out, that's OK.
		return WPAN_CONVERSATION_END;
	}

	zdo_rsp = envelope->payload;
	client_read = *(zcl_client_read_t FAR *)conversation->context;

	#ifdef ZCL_CLIENT_VERBOSE
		printf( "%s: generating ZCL request from ZDO response\n",
			__FUNCTION__);
		hex_dump( envelope->payload, envelope->length, HEX_DUMP_FLAG_TAB);
	#endif

	// Make sure this is a Match Descriptor Response.
	if (envelope->cluster_id != ZDO_MATCH_DESC_RSP)
	{
		#ifdef ZCL_CLIENT_VERBOSE
			printf( "%s: ignoring response (cluster=0x%04x, not MatchDescRsp)\n",
				__FUNCTION__, envelope->cluster_id);
		#endif
		return WPAN_CONVERSATION_CONTINUE;
	}

	// Only respond to successful responses with a length > 0
	if (zdo_rsp->header.status != ZDO_STATUS_SUCCESS
		|| zdo_rsp->header.match_length == 0)
	{
		#ifdef ZCL_CLIENT_VERBOSE
			printf( "%s: ignoring response (status=0x%02x, length=%u)\n",
				__FUNCTION__, zdo_rsp->header.status,
				zdo_rsp->header.match_length);
		#endif
		return WPAN_CONVERSATION_CONTINUE;
	}

	// Reply on the same device that received the response.
	wpan_envelope_reply( &reply_envelope, envelope);

	// assert that network_address == zdo_rsp->header.network_addr_le?
	if (reply_envelope.network_address !=
										le16toh( zdo_rsp->header.network_addr_le))
	{
		#ifdef ZCL_CLIENT_VERBOSE
			printf( "%s: ERROR net addr mismatch (env=0x%04x, zdo=0x%04x)\n",
				__FUNCTION__, reply_envelope.network_address,
				le16toh( zdo_rsp->header.network_addr_le));
		#endif
		return WPAN_CONVERSATION_CONTINUE;
		// DEVNOTE: exit here, or send request anyway?
	}

	// change source endpoint from ZDO (0) to selected ZCL client
	reply_envelope.source_endpoint = client_read.ep->endpoint;
	reply_envelope.profile_id = client_read.ep->profile_id;
	reply_envelope.cluster_id = client_read.cluster_id;

	// change dest endpoint to the one specified in the ZDO response
	reply_envelope.dest_endpoint = zdo_rsp->endpoint;

	// look up the cluster on the endpoint to set envelope options (like
	// the encryption flag)
	cluster = wpan_cluster_match( client_read.cluster_id,
		WPAN_CLUST_FLAG_CLIENT, client_read.ep->cluster_table);
	if (cluster)
	{
		reply_envelope.options = cluster->flags;
	}

   zcl_client_read_attributes( &reply_envelope, &client_read);

	return WPAN_CONVERSATION_CONTINUE;
}

/* START FUNCTION DESCRIPTION ********************************************
zcl_find_and_read_attributes            <zcl_client.c>

SYNTAX:
   int zcl_find_and_read_attributes( wpan_dev_t *dev, 
                                     const uint16_t *clusters, 
                                     const zcl_client_read_t FAR *cr)

DESCRIPTION:
     Use ZDO Match Descriptor Requests to find devices with a given
     profile/cluster and then automatically send a ZCL Read Attributes request
     for some of that cluster's attributes.


PARAMETER1:  dev - device to use for time request

PARAMETER2:  clusters - pointer to list of server clusters to search for,
              must end with #WPAN_CLUSTER_END_OF_LIST

PARAMETER3:  cr - zcl_client_read record containing information on
              the request (endpoint, attributes, etc.); must
              be a static object (not an auto variable) since
              the ZDO responder will need to access it


RETURNS:  0        - request sent
          !0	error sending request

**************************************************************************/
zcl_client_debug
int zcl_find_and_read_attributes( wpan_dev_t *dev, const uint16_t *clusters,
	const zcl_client_read_t FAR *cr)
{
	return zdo_send_match_desc( dev, clusters, cr->ep->profile_id,
		_zcl_send_read_from_zdo_match, cr);
}


/*** BeginHeader zdo_send_match_desc */
/*** EndHeader */

/* START FUNCTION DESCRIPTION ********************************************
zdo_send_match_desc                     <zcl_client.c>

SYNTAX:
   int zdo_send_match_desc( wpan_dev_t *dev,  const uint16_t *clusters, 
                            uint16_t profile_id,  wpan_response_fn callback, 
                            const void FAR *context)

DESCRIPTION:
     Send a ZDO Match Descriptor request for a list of cluster IDs.


PARAMETER1:  dev - device to use for time request
PARAMETER2:  clusters - pointer to list of server clusters to search for,
              must end with #WPAN_CLUSTER_END_OF_LIST
PARAMETER3:  profile_id - profile ID associated with the cluster IDs (cannot
              be WPAN_APS_PROFILE_ANY)
PARAMETER4:  callback - function that will process the ZDO Match Descriptor
              responses; see wpan_response_fn for the callback's
              API
PARAMETER5:  context - context to pass to <callback> in the
              wpan_conversation_t structure


RETURNS:  0        - request sent
          !0	error sending request




**************************************************************************/
zcl_client_debug
int zdo_send_match_desc( wpan_dev_t *dev, const uint16_t *clusters,
	uint16_t profile_id, wpan_response_fn callback, const void FAR *context)
{
	wpan_envelope_t	envelope;
	uint8_t				buffer[50];
	int					frame_length;
	int					trans;
	int					retval;

	if (dev == NULL || clusters == NULL || callback == NULL)
	{
		return -EINVAL;
	}

	// build the envelope -- use broadcast to find all matching devices
	memset( &envelope, 0, sizeof envelope);
	envelope.dev = dev;
	envelope.ieee_address = _WPAN_IEEE_ADDR_BROADCAST;
	envelope.network_address = WPAN_NET_ADDR_UNDEFINED;

	// Sending a ZDO Match Descriptor request
	envelope.cluster_id = ZDO_MATCH_DESC_REQ;

	// build the request (return error if not enough space)
	frame_length = zdo_match_desc_request( buffer, sizeof(buffer),
		WPAN_NET_ADDR_BCAST_NOT_ASLEEP, profile_id, clusters, NULL);
	if (frame_length < 0)
	{
		#ifdef ZCL_CLIENT_VERBOSE
			printf( "%s: error %d calling %s\n",
				__FUNCTION__, frame_length, "zdo_match_desc_request");
		#endif
		return frame_length;
	}

	// register the conversation to get a transaction id
	trans = wpan_conversation_register( zdo_endpoint_state( dev), callback,
		context, 60);
	if (trans < 0)
	{
		#ifdef ZCL_CLIENT_VERBOSE
			printf( "%s: error %d calling %s\n",
				__FUNCTION__, trans, "wpan_conversation_register");
		#endif
		return trans;
	}
	// Copy the transaction ID into the request and send it.  Should I add these
	// three parameters to the conversation_register function?  Lots of
	// parameters to pass around!
	*buffer = (uint8_t)trans;
	envelope.payload = buffer;
	envelope.length = frame_length;

	#ifdef ZCL_CLIENT_VERBOSE
		printf( "%s: sending %u-byte ZDO Match Desc request\n", __FUNCTION__,
			frame_length);
		hex_dump( buffer, frame_length, HEX_DUMP_FLAG_TAB);
	#endif

	retval = wpan_envelope_send( &envelope);
	#ifdef ZCL_CLIENT_VERBOSE
		if (retval)
		{
			printf( "%s: error %d calling %s\n",
				__FUNCTION__, retval, "wpan_envelope_send");
		}
	#endif

	return retval;
}

/*** BeginHeader zcl_create_attribute_records */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
zcl_create_attribute_records            <zcl_client.c>

SYNTAX:
   int zcl_create_attribute_records( void FAR *buffer,  uint8_t bufsize, 
                                 const zcl_attribute_base_t FAR **p_attr_list)

DESCRIPTION:
     From a list of attributes, write ID (in little-endian byte order), type
     and value to a buffer as would be done in a Write Attributes Request.

     The attribute list should be an array of attribute records, ending with
     an attribute ID of ZCL_ATTRIBUTE_END_OF_LIST.


PARAMETER1:  buffer - buffer to write values to
PARAMETER2:  bufsize - size of <buffer>
PARAMETER3:  p_attr_list - pointer to the attribute list to encode in
              <buffer>; points to next attribute to
              encode if buffer is filled


RETURNS:  number of bytes written to buffer

**************************************************************************/
#ifdef __XBEE_PLATFORM_HCS08
	#pragma MESSAGE DISABLE C5909		// Assignment in condition is OK
#endif
zcl_client_debug
int zcl_create_attribute_records( void FAR *buffer,
	uint8_t bufsize, const zcl_attribute_base_t FAR **p_attr_list)
{
	zcl_attrib_t FAR *encoded;
	uint8_t FAR *buffer_insert;
	const zcl_attribute_base_t FAR *attr;
	int result;

	if (p_attr_list == NULL || buffer == NULL ||
		(attr = *p_attr_list) == NULL)
	{
		// can't encode any attributes; 0 bytes written to buffer
		return 0;
	}

	buffer_insert = buffer;

	while (attr->id != ZCL_ATTRIBUTE_END_OF_LIST && bufsize >= 3)
	{
		encoded = (zcl_attrib_t FAR *) buffer_insert;
		encoded->id_le = htole16( attr->id);
		encoded->type = attr->type;
		result = zcl_encode_attribute_value( encoded->value, bufsize - 3, attr);
		if (result < 0)		// not enough room to encode value
		{
			break;
		}
		result += 3;			// account for 3 bytes used by ID and type
		bufsize -= result;
		buffer_insert += result;

		attr = zcl_attribute_get_next( attr);	// get next attribute to encode
	}

	// update *p_attr_list to point to the next attribute to encode, or
	// the end of the attribute list
	*p_attr_list = attr;

	return (int) (buffer_insert - (uint8_t FAR *)buffer);
}
#ifdef __XBEE_PLATFORM_HCS08
	#pragma MESSAGE DEFAULT C5909		// restore C5909 (Assignment in condition)
#endif

/*** BeginHeader zcl_send_write_attributes */
/*** EndHeader */
/* START _FUNCTION DESCRIPTION *******************************************
zcl_send_write_attributes               <zcl_client.c>

SYNTAX:
   int zcl_send_write_attributes( wpan_envelope_t *envelope, 
                                  const zcl_attribute_base_t FAR *attr_list)

DESCRIPTION:

     Send one or more Write Attributes Requests to a node on the network.

   NOTE: This function API and is likely to change in a future release
     to include a Manufacturer ID and flags for sending server-to-client
     and "undivided" write requests.


PARAMETER1:  envelope - envelope to use for sending request; \c payload
              and length cleared on function exit
PARAMETER2:  attr_list - attributes with values to send a Write
              Attributes Request for


RETURNS:  -EINVAL  - NULL parameter passed, or couldn't find source endpoint
                     based on envelope
          <0       - error trying to send request (for large writes, one or
                     more requests may have been successful)
          0        - request(s) sent

**************************************************************************/
// Consider adding mfg_id and flags field for direction and undivided?
// Should this function send a single request, and somehow return the next
// attribute to write (as zcl_create_attribute_records does)?
zcl_client_debug
int zcl_send_write_attributes( wpan_envelope_t *envelope,
	const zcl_attribute_base_t FAR *attr_list)
{
	const wpan_endpoint_table_entry_t *source_endpoint;
	struct request {
		zcl_header_nomfg_t	header;
		uint8_t					payload[80];
	} request;
	int bytecount;
	int retval = 0;

	// wpan_endpoint_of_envelope returns NULL for a NULL envelope
	source_endpoint = wpan_endpoint_of_envelope( envelope);
	if (source_endpoint == NULL || attr_list == NULL)
	{
		return -EINVAL;
	}

	envelope->payload = &request;
	request.header.frame_control = ZCL_FRAME_CLIENT_TO_SERVER
											| ZCL_FRAME_TYPE_PROFILE
											| ZCL_FRAME_GENERAL
											| ZCL_FRAME_DISABLE_DEF_RESP;
	request.header.command = ZCL_CMD_WRITE_ATTRIB;

	// it may take multiple packets to encode the write attributes records
	while (attr_list->id != ZCL_ATTRIBUTE_END_OF_LIST && retval == 0)
	{
		bytecount = zcl_create_attribute_records( &request.payload,
			sizeof(request.payload), &attr_list);

		if (bytecount <= 0)
		{
			// some sort of error, or attribute is too big to fit in request
			retval = bytecount;
			break;
		}

		request.header.sequence = wpan_endpoint_next_trans( source_endpoint);
		envelope->length = offsetof( struct request, payload) + bytecount;
		#ifdef ZCL_CLIENT_VERBOSE
			printf( "%s: sending Write Attributes Request\n", __FUNCTION__);
			wpan_envelope_dump( envelope);
		#endif
		retval = wpan_envelope_send( envelope);
	}

	envelope->payload = NULL;
	envelope->length = 0;

	return retval;
}




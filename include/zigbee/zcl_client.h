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
	@addtogroup zcl_client
	@{
	@file zigbee/zcl_client.h

	Code to support ZCL client clusters.
*/
#ifndef __ZCL_CLIENT_H
#define __ZCL_CLIENT_H

#include "wpan/aps.h"
#include "zigbee/zcl.h"

typedef struct zcl_client_read
{
	// include ieee, network and dest endpoint?
	// discovery has those set to broadcast, response handler fills them in?
	const wpan_endpoint_table_entry_t		*ep;
	uint16_t											mfg_id;
		// ZCL_MFG_NONE for standard
	uint16_t											cluster_id;
	const uint16_t							FAR	*attribute_list;
} zcl_client_read_t;

int zcl_find_and_read_attributes( wpan_dev_t *dev, const uint16_t *clusters,
	const zcl_client_read_t FAR *cr);
int zcl_process_read_attr_response( zcl_command_t *zcl,
	const zcl_attribute_base_t FAR *attr_table);
int _zcl_send_read_from_zdo_match( wpan_conversation_t FAR *conversation,
	const wpan_envelope_t FAR *envelope);
int zdo_send_match_desc( wpan_dev_t *dev, const uint16_t *clusters,
	uint16_t profile_id, wpan_response_fn callback, const void FAR *context);
int zcl_client_read_attributes( wpan_envelope_t FAR *envelope,
	const zcl_client_read_t *client_read);

int zcl_create_attribute_records( void FAR *buffer,
	uint8_t bufsize, const zcl_attribute_base_t FAR **p_attr_list);
int zcl_send_write_attributes( wpan_envelope_t *envelope,
	const zcl_attribute_base_t FAR *attr_list);

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "zcl_client.c"
#endif

#endif	// __ZCL_CLIENT_H

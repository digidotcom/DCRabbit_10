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
	@addtogroup zcl_identify
	@{
	@file zigbee/zcl_identify.h
	Header for implementation of the ZigBee Identify Cluster
	(ZCL Spec section 3.5).

	Numbers in comments refer to sections of ZigBee Cluster Library
	specification (Document 075123r02ZB).
*/

#ifndef __XBEE_ZCL_IDENTIFY_H
#define __XBEE_ZCL_IDENTIFY_H

#include "zigbee/zcl.h"

extern const zcl_attribute_tree_t zcl_identify_attribute_tree[];

#define ZCL_IDENTIFY_ATTR_IDENTIFY_TIME	0x0000
										///< 3.5.2.2, IdentifyTime Attribute

// client->server commands
#define ZCL_IDENTIFY_CMD_IDENTIFY			0x00
										///< 3.5.2.3.1, Identify Command
#define ZCL_IDENTIFY_CMD_QUERY				0x01
										///< 3.5.2.3.2, Identify Query Command

// server->client responses
#define ZCL_IDENTIFY_CMD_QUERY_RESPONSE	0x00
										///< 3.5.2.4.1, Identify Query Response Command

int zcl_identify_command( const wpan_envelope_t FAR *envelope,
	void FAR *context);

uint16_t zcl_identify_isactive( void);

/// Macro to add an Identify Server cluster to an endpoint's cluster table.
#define ZCL_CLUST_ENTRY_IDENTIFY_SERVER	\
	{ ZCL_CLUST_IDENTIFY,						\
		&zcl_identify_command,					\
		zcl_identify_attribute_tree,			\
		WPAN_CLUST_FLAG_SERVER }

#ifdef __DC__
	#use "zcl_identify.c"
#endif

#endif		// __XBEE_ZCL_IDENTIFY_H

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
	@file xbee_discovery.c

	Code related to "Node Discovery" (the ATND command, 0x95 frames)
*/

/*** BeginHeader */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "xbee/platform.h"
#include "xbee/atcmd.h"
#include "xbee/byteorder.h"
#include "xbee/discovery.h"
/*** EndHeader */

/*** BeginHeader xbee_disc_nd_parse */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
xbee_disc_nd_parse                      <xbee_discovery.c>

SYNTAX:
   int xbee_disc_nd_parse( xbee_node_id_t FAR *parsed,  const void FAR *source)

DESCRIPTION:
     Parse a Node Discovery response and store it in an xbee_node_id_t structure.

**************************************************************************/
int xbee_disc_nd_parse( xbee_node_id_t FAR *parsed, const void FAR *source)
{
	int ni_len;
	const xbee_node_id1_t FAR *id1;
	const xbee_node_id2_t FAR *id2;

	if (parsed == NULL || source == NULL)
	{
		return -EINVAL;
	}

	id1 = (const xbee_node_id1_t FAR *) source;
	ni_len = strlen( id1->node_info);
	if (ni_len > XBEE_DISC_MAX_NODEID_LEN)
	{
		return -EINVAL;
	}

	id2 = (const xbee_node_id2_t FAR *) &id1->node_info[ni_len + 1];
	parsed->ieee_addr_be = id1->ieee_addr_be;
	parsed->network_addr = be16toh( id1->network_addr_be);
	parsed->parent_addr = be16toh( id2->parent_addr_be);
	parsed->device_type = id2->device_type;
	_f_memset( parsed->node_info, 0, sizeof(parsed->node_info));
	_f_memcpy( parsed->node_info, id1->node_info, ni_len);

	return 0;
}

/*** BeginHeader xbee_disc_device_type_str */
/*** EndHeader */
const char *_xbee_disc_device_type[] =
	{ "Coord", "Router", "EndDev", "???" };
/* START FUNCTION DESCRIPTION ********************************************
xbee_disc_device_type_str               <xbee_discovery.c>

SYNTAX:
   const char *xbee_disc_device_type_str( uint8_t device_type)

DESCRIPTION:
     Return a string ("Coord", "Router", "EndDev", or "???") description for
     the "Device Type" field of 0x95 frames and ATND responses.


PARAMETER1:  device_type - the device_type field from an 0x95 frame or
              ATND response


RETURNS:  s pointer to a string describing the type, or "???" if
                     <device_type> is invalid

**************************************************************************/
const char *xbee_disc_device_type_str( uint8_t device_type)
{
	if (device_type > XBEE_ND_DEVICE_TYPE_ENDDEV)
	{
		device_type = XBEE_ND_DEVICE_TYPE_ENDDEV + 1;
	}

	return _xbee_disc_device_type[device_type];
}

/*** BeginHeader xbee_disc_node_id_dump */
/*** EndHeader */
/* START FUNCTION DESCRIPTION ********************************************
xbee_disc_node_id_dump                  <xbee_discovery.c>

SYNTAX:
   void xbee_disc_node_id_dump( const xbee_node_id_t FAR *ni)

DESCRIPTION:
     Debugging function used to dump an xbee_node_id_t structure to stdout.


PARAMETER1:  ni - pointer to an xbee_node_id_t structure


**************************************************************************/
void xbee_disc_node_id_dump( const xbee_node_id_t FAR *ni)
{
	printf( "Address:%08" PRIx32 "-%08" PRIx32 " 0x%04x  "
		"PARENT:0x%04x  %6s  NI:[%" PRIsFAR "]\n",
		be32toh( ni->ieee_addr_be.l[0]), be32toh( ni->ieee_addr_be.l[1]),
		ni->network_addr, ni->parent_addr,
		xbee_disc_device_type_str( ni->device_type), ni->node_info);
}

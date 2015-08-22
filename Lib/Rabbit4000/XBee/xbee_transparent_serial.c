/*
	Copyright (c)2010-11 Digi International Inc., All Rights Reserved

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
	@addtogroup xbee_transparent
	@{
	@file xbee_transparent_serial.c

	Support code for the "Digi Transparent Serial" cluster (cluster 0x0011 of
	endpoint 0xE8).
*/

/*** BeginHeader */
#include <string.h>

#include "xbee/platform.h"
#include "xbee/transparent_serial.h"
/*** EndHeader */

/*** BeginHeader xbee_transparent_serial */
/*** EndHeader */
// documented in xbee/transparent_serial.h
int xbee_transparent_serial( wpan_envelope_t *envelope)
{
	// fill in rest of envelope
	envelope->profile_id = WPAN_PROFILE_DIGI;
	envelope->source_endpoint = envelope->dest_endpoint
																= WPAN_ENDPOINT_DIGI_DATA;
	envelope->cluster_id = DIGI_CLUST_SERIAL;

	return wpan_envelope_send( envelope);
}

/*** BeginHeader xbee_transparent_serial_str */
/*** EndHeader */
// documented in xbee/transparent_serial.h
int xbee_transparent_serial_str( wpan_envelope_t *envelope,
																	const char FAR *data)
{
	envelope->payload = data;
	envelope->length = strlen( data);

	return xbee_transparent_serial( envelope);
}


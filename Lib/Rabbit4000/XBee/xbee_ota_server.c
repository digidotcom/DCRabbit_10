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
	@addtogroup xbee_ota_server
	@{
	@file xbee_ota_server.c

	Code to add an OTA Server Cluster to a device.  It receives notification
	to start an update, and then calls back to the bootloader to receive
	that update.
*/

/*** BeginHeader */
#include <stdio.h>
#include "xbee/ota_server.h"
/*** EndHeader */

/*** BeginHeader xbee_ota_server_cmd */
/*** EndHeader */
int xbee_ota_server_cmd( const wpan_envelope_t FAR *envelope,
	void FAR *context)
{
	uint16_t options;
	const char *err = NULL;

	// If cluster configuration requires encryption, make sure this
	// frame was received encrypted.

	options = envelope->options &
		(WPAN_ENVELOPE_RX_APS_ENCRYPT | WPAN_CLUST_FLAG_ENCRYPT);
	if (options == WPAN_CLUST_FLAG_ENCRYPT)
	{
		// cluster requires encryption, but frame wasn't encrypted
		// This isn't ZCL, so we can't send a Default Response and we
		// just have to ignore the message.
		err = "encryption required";
	}
	else
	{
		// call function provided by the application for starting the update
		err = xbee_update_firmware_ota( envelope, context);

		// xbee_update_firmware_ota won't return if it's able to install the
		// update and reboot
	}

	if (err)
	{
		wpan_envelope_t reply;

		wpan_envelope_reply( &reply, envelope);
		reply.payload = err;
		reply.length = strlen( err);

		wpan_envelope_send( &reply);
	}

	return 0;
}

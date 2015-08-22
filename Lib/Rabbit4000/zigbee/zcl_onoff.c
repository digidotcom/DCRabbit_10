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
	@addtogroup zcl_onoff
	@{
	@file zigbee/zcl_onoff.c
*/
/*** BeginHeader _zcl_onoff_handler */
#include "wpan/aps.h"
#include "zigbee/zcl.h"
#include "zigbee/zcl_onoff.h"
/*** EndHeader */

/* START _FUNCTION DESCRIPTION *******************************************
_zcl_onoff_handler                      <zcl_onoff.c>

SYNTAX:
   int _zcl_onoff_handler( const wpan_envelope_t FAR *envelope, 
                           void FAR *context)

DESCRIPTION:

     This is an incomplete implementation of the OnOff Cluster Command handler.

     It needs to look at the OnOff attribute (found by walking zcl.attributes),
     and call the attribute's .write_fn() to set the new value.

     On TOGGLE, take the current value (calling the attributes' .read_fn() if
     necessary) and write the opposite value.

     See the documentation of wpan_aps_handler_fn for parameters and return
     values.

**************************************************************************/
int _zcl_onoff_handler( const wpan_envelope_t FAR *envelope,
	void FAR *context)
{
	zcl_command_t						zcl;

	if (zcl_command_build( &zcl, envelope, context) == 0 &&
		ZCL_CMD_MATCH( &zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER))
	{
		uint8_t status = ZCL_STATUS_SUCCESS;

		switch (zcl.command)
		{
			case ZCL_ONOFF_CMD_OFF:

			case ZCL_ONOFF_CMD_ON:

			case ZCL_ONOFF_CMD_TOGGLE:

			default:
				status = ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
		}

		return zcl_default_response( &zcl, status);
	}

	// Allow General Command handler to process general
	// commands and send errors out for unsupported commands.
	return zcl_general_command( envelope, context);
}

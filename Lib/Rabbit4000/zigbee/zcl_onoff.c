/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

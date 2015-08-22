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
	@addtogroup zcl_basic
	@{
	@file zcl_basic.c

	Support for the "Reset to Factory Defaults" command on the Basic Server
	Cluster.
*/

/*** BeginHeader */
#include <stdio.h>

#include "xbee/platform.h"
#include "zigbee/zcl.h"
#include "zigbee/zcl_basic.h"
/*** EndHeader */

/*** BeginHeader _zcl_basic_server */
/*** EndHeader */
#ifndef ZCL_FACTORY_RESET_FN
// PPH, revisit and decide how to allow customers customize this function.
// Now, we just disable the error message to build
//	#error "Must define ZCL_FACTORY_RESET_FN to use this module."
#endif

void ZCL_FACTORY_RESET_FN( void);

/* START _FUNCTION DESCRIPTION *******************************************
_zcl_basic_server                       <zcl_basic.c>

SYNTAX:
   int _zcl_basic_server( const wpan_envelope_t FAR *envelope, 
                          void FAR *context)

DESCRIPTION:

     Handles commands for the Basic Server Cluster.

     Currently supports the
     only command ID in the spec, 0x00 - Reset to Factory Defaults.

   NOTE: You must define the macro ZCL_FACTORY_RESET_FN in your program, and have
     it point to a function to be called when a factory reset command is sent.



**************************************************************************/
int _zcl_basic_server( const wpan_envelope_t FAR *envelope,
	void FAR *context)
{
	zcl_command_t	zcl;

	if (zcl_command_build( &zcl, envelope, context) == 0 &&
		ZCL_CMD_MATCH( &zcl.frame_control, GENERAL, CLIENT_TO_SERVER, CLUSTER))
	{
		// This function only handles command 0x00, reset to factory defaults.
		if (zcl.command == ZCL_BASIC_CMD_FACTORY_DEFAULTS)
		{
			#ifdef ZCL_BASIC_VERBOSE
				printf( "%s: resetting to factory defaults\n", __FUNCTION__);
			#endif
			ZCL_FACTORY_RESET_FN();

			return zcl_default_response( &zcl, ZCL_STATUS_SUCCESS);
		}
	}

	return zcl_general_command( envelope, context);
}

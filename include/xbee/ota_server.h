/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup xbee_ota_server
	@{
	@file xbee/ota_server.h

	Code to add an OTA Server Cluster to a device.  It receives notification
	to start an update, and then calls back to the bootloader to receive
	that update.
*/


#include "wpan/aps.h"
#include "zigbee/zcl.h"

/* START FUNCTION DESCRIPTION ********************************************
xbee_ota_server_cmd                     <ota_server.h>

SYNTAX:
   int xbee_ota_server_cmd( const wpan_envelope_t FAR *envelope, 
                            void FAR *context)

DESCRIPTION:
     Cluster command to initiate firmware updates.

     Verifies that APS encryption
     was used (if cluster is configured as such) before handing off to
     implementation-provided function xbee_update_firmware_ota().

SEE ALSO:  wpan_aps_handler_fn

**************************************************************************/
int xbee_ota_server_cmd( const wpan_envelope_t FAR *envelope,
	void FAR *context);

/* START FUNCTION DESCRIPTION ********************************************
xbee_update_firmware_ota                <ota_server.h>

SYNTAX:
   const char *xbee_update_firmware_ota( const wpan_envelope_t FAR *envelope, 
                                         void FAR *context)

DESCRIPTION:
     Application needs to provide this function as a method of
     receiving firmware updates over-the-air with Xmodem protocol.

     See xbee/ota_client.h for details on sending updates.

     Your application can support password-protected updates by checking the
     payload of the request.  If the payload is a valid request to initiate
     an update, this function should enter an "XMODEM receive" mode and
     start sending 'C' to the sender of the request, indicating that it
     should start sending 64-byte XMODEM packets with the new firmware.

     On Digi's Programmable XBee platform, this function would exit to the
     bootloader so it can receive the new application firmware.


PARAMETER1:  envelope - command sent to start update -- function may want
              to use the payload for some sort of password
              verification
PARAMETER2:  context - user context (from cluster table)


RETURNS:  NULL	do not respond to request
          !NULL	respond to request with error message


**************************************************************************/
const char *xbee_update_firmware_ota( const wpan_envelope_t FAR *envelope,
	void FAR *context);

/* START FUNCTION DESCRIPTION ********************************************
XBEE_OTA_CMD_SERVER_CLUST_ENTRY         <ota_server.h>

MACRO SYNTAX:
     XBEE_OTA_CMD_SERVER_CLUST_ENTRY( flag)

DESCRIPTION:
     Macro to add the OTA cluster to the Digi Data Endpoint.


PARAMETER1:  flag - set to WPAN_CLUST_FLAG_NONE or WPAN_CLUST_FLAG_ENCRYPT

**************************************************************************/
#define XBEE_OTA_CMD_SERVER_CLUST_ENTRY(flag)								\
	{	DIGI_CLUST_PROG_XBEE_OTA_UPD,	xbee_ota_server_cmd, NULL,			\
		(flag) | WPAN_CLUST_FLAG_SERVER | WPAN_CLUST_FLAG_NOT_ZCL }


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


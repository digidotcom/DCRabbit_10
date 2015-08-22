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
	@addtogroup xbee_transparent
	@{
	@file xbee/transparent_serial.h

	Support code for the "Digi Transparent Serial" cluster (cluster 0x0011 of
	endpoint 0xE8).
*/

#ifndef XBEE_TRANSPARENT_SERIAL_H
#define XBEE_TRANSPARENT_SERIAL_H

#include "xbee/platform.h"
#include "wpan/aps.h"

/* START FUNCTION DESCRIPTION ********************************************
xbee_transparent_serial                 <transparent_serial.h>

SYNTAX:
   int xbee_transparent_serial( wpan_envelope_t *envelope)

DESCRIPTION:
     Send data to the "Digi Transparent Serial" cluster (cluster 0x0011 of
     endpoint 0xE8).


PARAMETER1:  envelope - Envelope with device, addresses, payload, length
              and options filled in.  This function sets the profile,
              endpoints, and cluster fields of the envelope.


RETURNS:  0        - data sent
          !0	error trying to send request



   NOTE: This is a preliminary API and may change in a future release.

**************************************************************************/
int xbee_transparent_serial( wpan_envelope_t *envelope);

/* START FUNCTION DESCRIPTION ********************************************
xbee_transparent_serial_str             <transparent_serial.h>

SYNTAX:
   int xbee_transparent_serial_str( wpan_envelope_t *envelope, 
                                    const char FAR *data)

DESCRIPTION:
     Send string to the "Digi Transparent Serial" cluster (cluster 0x0011 of
     endpoint 0xE8).


PARAMETER1:  envelope - Envelope with device, addresses and options
              filled in.  This function sets the payload,
              length, profile, endpoints, and cluster fields
              of the envelope.
PARAMETER2:  data - string to send


RETURNS:  0        - data sent
          !0	error trying to send request



   NOTE: This is a preliminary API and may change in a future release.

**************************************************************************/
int xbee_transparent_serial_str( wpan_envelope_t *envelope,
																	const char FAR *data);

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "xbee_transparent_serial.c"
#endif

#endif

/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

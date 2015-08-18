/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup xbee_atmode
	@{
	@file xbee/atmode.h
	Header for working with XBee modules in AT command mode instead of API mode.
*/

#ifndef __XBEE_ATMODE
#define __XBEE_ATMODE

#include "xbee/platform.h"
#include "xbee/device.h"

// Datatypes should be defined in this file as well, possibly with a
// "function help" documentation block to document complex structures.

// Function declarations that would normally appear in BeginHeader/EndHeader
// blocks of a .lib file.
int xbee_atmode_enter( xbee_dev_t *xbee);
int xbee_atmode_exit( xbee_dev_t *xbee);
int xbee_atmode_tick( xbee_dev_t *xbee);

int xbee_atmode_send_request( xbee_dev_t *xbee, const char FAR *command);
int xbee_atmode_read_response( xbee_dev_t *xbee, char FAR *response,
	int resp_size, int FAR *bytesread);

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "xbee_atmode.c"
#endif

#endif



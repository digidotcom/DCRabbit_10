/*
	Copyright (c)2009-2010 Digi International Inc., All Rights Reserved

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



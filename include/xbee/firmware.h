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
	@addtogroup xbee_firmware
	@{
	@file xbee/firmware.h
	Driver layer for performing XBee firmware updates.

	@todo Create typedefs for xbee_fw_source_t read() and seek() handlers,
			write documentation including what the return values are.  We will
			have to check existing functions to confirm the API, but I'm guessing
			<0 for error, >=0 for number of bytes read.  Make sure calls to the
			read() function check for errors!  _xbee_oem_verify() does not!
*/

#ifndef __XBEE_FIRMWARE
#define __XBEE_FIRMWARE

#include "xbee/xmodem.h"

// Datatypes should be defined in this file as well, possibly with a
// "function help" documentation block to document complex structures.

typedef struct xbee_fw_oem_state_t {
	uint32_t				firmware_length;
	uint32_t				block_offset;
	uint32_t				cur_offset;
	uint16_t				block_length;
} xbee_fw_oem_state_t;

typedef struct xbee_fw_source_t
{
	xbee_dev_t				*xbee;
	int						state;
	int						next_state;
	int						tries;
	uint32_t					timer;
	union {
		xbee_xmodem_state_t	xbxm;			// sub-status of xmodem transfer
		xbee_fw_oem_state_t	oem;			// sub-status of oem transfer
	} u;
	char						buffer[128+5];	// buffer for xmodem packet, must persist
													// for duration of update
	int		(*seek)( void FAR *context, uint32_t offset);
	int		(*read)( void FAR *context, void FAR *buffer, int16_t bytes);
	void				FAR *context;		// spot to hold user data
} xbee_fw_source_t;

#define XBEE_FW_LOAD_TIMEOUT		5000

int xbee_fw_install_init( xbee_dev_t *xbee, const wpan_address_t FAR *target,
	xbee_fw_source_t *source);

int xbee_fw_install_ebl_tick( xbee_fw_source_t *source);
unsigned int xbee_fw_install_ebl_state( xbee_fw_source_t *source);
char FAR *xbee_fw_status_ebl( xbee_fw_source_t *source, char FAR *buffer);

int xbee_fw_install_oem_tick( xbee_fw_source_t *source);
char FAR *xbee_fw_status_oem( xbee_fw_source_t *source, char FAR *buffer);

// xbee_fw_install_tick calls either _s2 or _s1, depending on hardware type
//int _xbee_fw_install_tick_s2( xbee_fw_source_t *source);


// Helper function for installing firmware from a .OEM or .EBL file in memory.
typedef struct {
	xbee_fw_source_t source;
	uint32_t			length;
	const char FAR	*address;
	uint32_t			offset;
} xbee_fw_buffer_t;

int xbee_fw_buffer_init( xbee_fw_buffer_t *fw, uint32_t length,
	const char FAR *address);

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "xbee_firmware.c"
#endif

#endif



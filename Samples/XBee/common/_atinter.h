/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#ifndef _ATINTER_H_INCL
#define _ATINTER_H_INCL

#include "xbee/atcmd.h"
#include "xbee/device.h"

// first two functions don't need to be public
//int parseParameter (const char *paramstr, int16_t request);
//int xbee_cmd_callback( const xbee_cmd_response_t FAR *response);

void process_command_remote( xbee_dev_t *xbee, const char *cmdstr,
			const addr64 FAR *ieee);
void process_command( xbee_dev_t *xbee, const char *cmdstr);
void printATCmds( xbee_dev_t *xbee);

enum {
	OUTPUT_HEX,
	OUTPUT_DECIMAL,
	OUTPUT_STRING
};

typedef struct
{
	char		*command;
	uint16_t	flags;
	char		*name;
	char		*desc;
} command_t;

// store output format in lower nibble
#define FLAG_OUTPUT_MASK		0x000F

// store node type in next nibble
#define FLAG_ENDDEV				0x0010
#define FLAG_ROUTER				0x0020
#define FLAG_COORDINATOR		0x0040
#define FLAG_ANYNODETYPE		(FLAG_ENDDEV | FLAG_ROUTER | FLAG_COORDINATOR)

// store firmware type in upper byte
#define FLAG_ZNET					0x0100
#define FLAG_ZB					0x0200
#define FLAG_SE					0x0400
// DigiMesh 900 is always router
#define FLAG_DIGIMESH900		(0x0800 | FLAG_ROUTER)

// all node types for all firmwares
#define FLAG_ANYDEVICE			0xFFF0

extern const command_t command_list[];
extern const int command_count;
#endif	// _ATINTER_H_INCL


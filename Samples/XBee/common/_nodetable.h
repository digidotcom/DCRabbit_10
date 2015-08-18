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

/*
	Simple code for managing a table of xbee_node_id_t objects -- the
	XBee-specific node record used for ATND responses and Join Notifications.
*/

#ifndef _NODETABLE_H
#define _NODETABLE_H

#include "xbee/discovery.h"

#define NODE_TABLE_SIZE  10
extern xbee_node_id_t node_table[NODE_TABLE_SIZE];

xbee_node_id_t *node_by_addr( const addr64 FAR *ieee_be);
xbee_node_id_t *node_by_name( const char *name);
xbee_node_id_t *node_by_index( int idx);
xbee_node_id_t *node_add( const xbee_node_id_t *node_id);
void node_table_dump( void);

#endif

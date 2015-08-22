/* Digi International, Copyright © 2005-2011.  All rights reserved. */

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

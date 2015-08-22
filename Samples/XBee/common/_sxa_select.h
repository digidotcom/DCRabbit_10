/* Digi International, Copyright © 2005-2011.  All rights reserved. */
#ifndef _SXA_SELECT_H_INCL
#define _SXA_SELECT_H_INCL

extern addr64 target_ieee;
extern int have_target;
extern int ever_had_target;
extern sxa_node_t FAR *sxa;

void set_sxa(void);
int set_target(const char *str, addr64 FAR *address);
void sxa_select_help(void);
int sxa_select_command( char *cmdstr);


#endif	// _SXA_SELECT_H_INCL


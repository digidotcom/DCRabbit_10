/*
   Copyright (c) 2016 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef __BIOS_ALREADY_COMPILED
   #define __BIOS_ALREADY_COMPILED
#else
   #fatal "Cannot compile the BIOS as a user program!"
#endif

#use "SYSIODEFS.LIB"
#use "BOARDTYPES.LIB"	// board-specific initialization header
#use "SYSCONFIG.LIB"		// All user-defined macros for BIOS configuration

#pragma CompileBIOS

#undef MECR_VALUE
#define MECR_VALUE 0
#orgstart

#orgdef rcodorg rootcode above phy 0 log 0 size 0x6000

// This is a dummy definition; The cold loader should not refer to variables.
#orgdef rvarorg rootdata above rootcode size 1

// This is a dummy definition; the cold loader should not be using xmem code.
#orgdef xcodorg xmemcode above rootdata size 1

// This is a dummy definition; the cold loader should not be using xmem data.
#orgdef xvarorg xmemdata above xmemcode size 1


#orgend

#orgact rootcode apply

enum OriginType { UNKNOWN_ORG = 0, RCODORG, XCODORG, WCODORG, RVARORG, XVARORG,
						WVARORG, RCONORG, RESVORG };

typedef struct {
	char type;
   char flags;                //
   unsigned long	paddr;      // Physical address
   unsigned		laddr;      	// Logical address
   unsigned long 	usedbytes;  // Actual number of bytes used or reserved
   									// starting from paddr/laddr
   unsigned long  totalbytes; // Total space allocated
} _sys_mem_origin_t;

// These are defined by the compiler.
extern int _orgtablesize;
extern _sys_mem_origin_t _orgtable[];

#define _cexpr(X) 0+(X)

#flushlib
#flushlib

#flushcompile

#flushlib
#flushlib
#flushlib

//#pragma CompileProgram

#memmap root

#asm
StartUserCode::
#endasm



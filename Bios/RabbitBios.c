/***********************************************************************
* ******************************************************************** *
* *                                                                  * *
* *   BIOS Selector Shell for Rabbit 4000 CPU based boards.          * *
* *                                                                  * *
* *                                                                  * *
* *    Copyright (c)  1999-2006 Rabbit Semiconductor, Inc.           * *
* *                                                                  * *
* *   Assumes ATMEL compatible flash algorithms.                     * *
* *                                                                  * *
* ******************************************************************** *
***********************************************************************/

#ifndef __BIOS_ALREADY_COMPILED
   #define __BIOS_ALREADY_COMPILED
#else
   #fatal "Cannot compile the BIOS as a user program!"
#endif

#use "SYSIODEFS.LIB"
#use "SYSCONFIG.LIB"    // All user-defined macros for BIOS configuration
#use "BOARDTYPES.LIB"   // board-specific initialization header

// ***** _CLK_DBL_ macro check *****************************************
#if (CPU_ID_MASK(_CPU_ID_) < R4000)
   #fatal "This version of Dynamic C is only supported for RCM4XXX or newer core modules."
#endif

enum OriginType { UNKNOWN_ORG = 0, RCODORG, XCODORG, WCODORG, RVARORG, XVARORG,
                  WVARORG, RCONORG, RESVORG };

typedef struct {
   char type;
   char flags;                //
   unsigned long  paddr;      // Physical address
   unsigned    laddr;         // Logical address
   unsigned long  usedbytes;  // Actual number of bytes used or reserved
                              // starting from paddr/laddr
   unsigned long  totalbytes; // Total space allocated
} _sys_mem_origin_t;

// These are defined by the compiler.
extern int _orgtablesize;
extern _sys_mem_origin_t _orgtable[];

#define _cexpr(X) 0+(X)

#use "stdBIOS.c"

#flushlib
#flushlib

#pragma CompileProgram

#flushcompile

#flushlib
#flushlib
#flushlib

#asm
StartUserCode::
#if _DK_ENABLE_DEBUGKERNEL_ || defined RFU_BIN_RUN_IMMEDIATELY
	/*
	   When RFU_BIN_RUN_IMMEDIATELY is defined in Dynamic C's Project Options'
	   Defines tab, a BIN program image successfully loaded by RFU 4.72 or later
	   will run both BIOS code and user code immediately after loading, with the
	   programming cable still connected. With the programming cable
	   disconnected, BIOS code and user code will execute as usual.

	   This mode of operation is enforced when Dynamic C's debug kernel is
	   enabled. It can also be useful in test fixture code, for example, where a
	   test fixture can not or does not control the SMODEx levels and manual
	   disconnection of the programming cable is inconvenient or undesirable.
	*/
#elif defined RFU_BIN_WAIT_FOR_RUN_MODE
	/*
	   When RFU_BIN_WAIT_FOR_RUN_MODE is defined in Dynamic C's Project Options'
	   Defines tab, a BIN program image successfully loaded by RFU 4.72 or later
	   will run BIOS code and then pause in an idle loop, waiting for the
	   programming cable to be disconnected. As soon as the programming cable is
	   disconnected, user code will begin to execute. With the programming cable
	   disconnected, BIOS code and user code will execute as usual.

	   This mode of operation can be useful in test fixture code, for example,
	   where a test fixture can control at least one of the SMODEx levels and
	   manual disconnection of the programming cable is inconvenient or
	   undesirable.
	*/
		push	af
.rfu_wait_for_run_mode:
		call	bioshitwd
ioi	ld		a, (SPCR)
		and	0x60
		cp		0x60										; are both SMODE0==1 and SMODE1==1?
		jr		z, .rfu_wait_for_run_mode			; if yes, loop back and check again

		pop	af
#else	// effectively, RFU_BIN_WAIT_FOR_RESET
	/*
	   Default behavior. When neither RFU_BIN_RUN_IMMEDIATELY nor
	   RFU_BIN_WAIT_FOR_RUN_MODE are defined, a BIN program image successfully
	   loaded by RFU 4.72 or later will run BIOS code only and then will wait in
	   an idle loop until a hardware reset occurs. Following a hardware reset and
	   with the programming cable disconnected, BIOS code and user code will
	   execute as usual.

	   This mode of operation most closely emulates the behavior of RFU versions
	   prior to 4.72. It can also be useful in test fixture code, for example,
	   where a test fixture can control at least one of the SMODEx levels as well
	   as the RESET level, and manual disconnection of the programming cable is
	   inconvenient or undesirable.
	*/
		push	af
ioi	ld		a, (SPCR)
		and	0x60
		cp		0x60										; are both SMODE0==1 and SMODE1==1?
		jr		nz, .rfu_continue_after_reset		; if no, go execute user code

.rfu_wait_for_reset:
		call	bioshitwd
		jr		.rfu_wait_for_reset					; idle loop (until hardware reset)

.rfu_continue_after_reset:
		pop	af
#endif
#endasm
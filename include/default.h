/* START LIBRARY DESCRIPTION *********************************************
default.h
	Copyright (c) 2007, Rabbit Semiconductor, Inc.

DESCRIPTION:    Default libraries to be used for the boards. The use
					statements do not necessarily compile anything, it is just
					an indication that functions in the used libraries may be
					called and linked.

END DESCRIPTION **********************************************************/

#ifndef PILOT_BIOS
#ifndef BIOSCODE

	#define SETUSERMODE
	#define SETSYSMODE
	#define _system
	#define _SYS_CALL_VARS
	#define _NET_SYSCALL(x)
	#define _rk_printf printf
	#define _rk_sprintf sprintf

	//***** Debug Kernel Information ******************************************
                                 // _DK_ENABLE_BREAKPOINTS_ is defined
                                 // internally by Dynamic C and is set to 1 to
                                 // enable breakpoint support in debug kernel, 0
                                 // to disable support and reclaim code space

	#define DK_ENABLE_BREAKPOINTS _DK_ENABLE_BREAKPOINTS_

                                 // _DK_ENABLE_ASMSINGLESTEP_ is defined
                                 // internally by Dynamic C and is set to 1
                                 // to enable assembly level single step
                                 // support in the debug kernel, 0 to
                                 // disable support and reclaim code space

	#define DK_ENABLE_ASMSINGLESTEP _DK_ENABLE_ASMSINGLESTEP_

                                 // _DK_ENABLE_WATCHEXPRESSIONS_ is defined
                                 // internally by Dynamic C and is set to 1
                                 // to enable watch expressions in the debug
                                 // kernel, 0 to disable support and reclaim
                                 // code space

	#define DK_ENABLE_WATCHEXPRESSIONS _DK_ENABLE_WATCHEXPRESSIONS_

                                 // _DK_ENABLE_TRACING_ is defined
                                 // internally by Dynamic C and is set to 1
                                 // to enable execution tracing in the debug
                                 // kernel, 0 to disable support and reclaim
                                 // code space

	#define DK_ENABLE_TRACING _DK_ENABLE_TRACING_

                                 // _DK_ENABLE_STACK_TRACING_ is defined
                                 // internally by Dynamic C and is set to 1
                                 // to enable stack tracing in the debug
                                 // kernel, 0 to disable support and reclaim
                                 // code space

	#define DK_ENABLE_STACK_TRACING _DK_ENABLE_STACK_TRACING_

                                 // _DK_SEND_STACK_LENGTH_ is defined
                                 // internally by Dynamic C and is set to 32
                                 // if Stack Tracing is disabled up to a max
                                 // value maintained in the registry, 4096
                                 // by default

	#define DK_SEND_STACK_LENGTH _DK_SEND_STACK_LENGTH_

	#define DK_ENABLE_DEBUGKERNEL _DK_ENABLE_DEBUGKERNEL_

	// If the debug kernel core is not enabled from the GUI, make sure all pieces
	// are disabled.
 #if DK_ENABLE_DEBUGKERNEL == 0

	#undef  DK_ENABLE_TRACING
	#define DK_ENABLE_TRACING				0

	#undef  DK_ENABLE_STACK_TRACING
	#define DK_ENABLE_STACK_TRACING		0

	#undef  DK_ENABLE_ASMSINGLESTEP
	#define DK_ENABLE_ASMSINGLESTEP		0

	#undef  DK_ENABLE_BREAKPOINTS
	#define DK_ENABLE_BREAKPOINTS			0

	#undef  DK_ENABLE_WATCHEXPRESSIONS
	#define DK_ENABLE_WATCHEXPRESSIONS  0
 #endif

	// libraries for user programs
	#include <rabbit.h>

#else	// BIOSCODE

	// libraries to parse during the BIOS compile
	#include <dc.h>
	#use "util.lib"			// FLASHWR.LIB needs helper functions from UTIL.LIB
	#use "math.lib"			// IDBLOCK.LIB uses getcrc() from MATH.LIB

	#undef BIOSCODE
#endif	// BIOSCODE
#endif


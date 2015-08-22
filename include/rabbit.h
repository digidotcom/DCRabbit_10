/*
	rabbit.h

	Copyright (c)2000-2010 Digi International Inc., All Rights Reserved

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

/*
	Support headers and libraries, previously brought in by default.h.

	User programs should #include <rabbit.h> after defining configuration
	macros.
*/

#if ! (defined PILOT_BIOS || defined BIOSCODE)

#ifndef __RABBIT_H
	#define __RABBIT_H

	#makechain _GLOBAL_INIT
	extern void _GLOBAL_INIT();

	////////////////////////////////////////////////////////////////////////
	//RCM4xxx
	//initial block for RCM40xx, RCM41xx, RCM42xx, RCM45xx, RCM54xx, and BL4S1xx
	////////////////////////////////////////////////////////////////////////
  #if RCM4000_SERIES  || RCM4100_SERIES  || RCM4200_SERIES  || \
      RCM4500W_SERIES || RCM5400W_SERIES || BL4S100_SERIES

   // If this is a BLxS2xx series SBC.
	#if ((_DC_MB_TYPE_ & 0x0100) && !(_DC_MB_TYPE_ & 0xFE00))
      // Reserve 2k of userblock for ADC calibration factors
	  	#define ZWORLD_RESERVED_SIZE 0x0800
   #else
    	#ifdef ADC_ONBOARD
	      // Reserve 2k of userblock for ADC calibration factors
		  	#define ZWORLD_RESERVED_SIZE 0x0800
	   #endif
	#endif

  #endif

  #if RCM4300_SERIES
    // If this is a BLxS2xx series SBC.
	 #if ((_DC_MB_TYPE_ & 0x0100) && !(_DC_MB_TYPE_ & 0xFE00))
	  	#define ZWORLD_RESERVED_SIZE 0x0800	// Reserved 2k userblock for Z-World
	 #endif
    #ifdef ADC_ONBOARD
		#define ZWORLD_RESERVED_SIZE 0x0800	// Reserved 2k userblock for Z-World
    #endif
  #endif

  #if RCM4400W_SERIES
    // If this is a BLxS2xx series SBC.
	 #if ((_DC_MB_TYPE_ & 0x0100) && !(_DC_MB_TYPE_ & 0xFE00))
	  	#define ZWORLD_RESERVED_SIZE 0x0800	// Reserved 2k userblock for Z-World
	 #endif
  #endif

	// Set macro to indicate SBCs or core modules not
	// reserving any User block area for Z-World's use.
  #ifndef ZWORLD_RESERVED_SIZE
	#define ZWORLD_RESERVED_SIZE 0
  #endif

#endif // ifdef __RABBIT_H

	#use "errno.c"
	#use "dma.lib"
	#use "mutilfp.lib"
	#use "sys.lib"

	#use "ExternIO.LIB"
	#use "PWM.LIB"
	#use "QD.LIB"
	#use "VBAT.LIB"

	#include <limits.h>
	#include <stddef.h>
	#include <setjmp.h>
	#include <time.h>				// for struct tm

	// libraries to compile for user programs
	// program.lib contains premain, and so must appear here
	#use "PLL.lib"
	#use "dkentry.lib"
	#use "dkcommon.lib"
	#use "dkcore.lib"
	#use "dkapp.lib"
	#use "cofunc.lib"
	#use "costate.lib"
	#use "slice.lib"
	#use "cbuf.lib"
	#use "rs232.lib"
	#include <stdio.h>			// stdio_serial.c refers to serXdata in RS232.LIB
	#include <stdlib.h>			// exit() needs fclose() and FILE * in stdio.h
	#include <string.h>
	#include <strings.h>

	#use "rtclock.lib"
	#use "vdriver.lib"
	#use "stdvdriver.lib"
	#include <errno.h>

	#use "program.lib"			// premain() calls exit() in stdlib

	#use "mem.lib"			// BIOS's STACK.LIB needs _xalloc() from mem.lib
	#use "stack.lib"
	#use "xmem.lib"

	#include <assert.h>


#endif 		// ! (defined PILOT_BIOS || defined BIOSCODE)
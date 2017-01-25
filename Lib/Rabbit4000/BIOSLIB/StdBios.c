/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	StdBios.c
	Universal BIOS for Rabbit 4000/5000/6000 CPU based boards.
*/

/*
	Assumes ATMEL-compatible flash algorithms for serial flash and
	JEDEC-compatible flash algorithms for parallel flash.
*/

/*** BeginHeader */
#ifndef __STDBIOS_C
#define __STDBIOS_C

// The compiler understands __wfd, so define __waitfordone to that.
#define __waitfordone   __wfd

#ifndef abandon
   #define abandon      __abandon
#endif
#ifndef abort
   #define abort        __abort
#endif
#ifndef always_on
   #define always_on    __always_on
#endif
#ifndef anymem
   #define anymem       __anymem
#endif
#ifndef bbram
   #define bbram        __bbram
#endif
#ifndef cofunc
   #define cofunc       __cofunc
#endif
#ifndef costate
   #define costate      __costate
#endif
#ifndef debug
   #define debug        __debug
#endif
#ifndef everytime
   #define everytime    __everytime
#endif
#ifndef far
   #define far          __far
#endif
#ifndef firsttime
   #define firsttime    __firsttime
#endif
#ifndef init_on
   #define init_on      __init_on
#endif
#ifndef interrupt
   #define interrupt    __interrupt
#endif
#ifndef near
   #define near         __near
#endif
#ifndef nodebug
   #define nodebug      __nodebug
#endif
#ifndef norst
   #define norst        __norst
#endif
#ifndef nouseix
   #define nouseix      __nouseix
#endif
#ifndef protected
   #define protected    __protected
#endif
#ifndef root
   #define root         __root
#endif
#ifndef scofunc
   #define scofunc      __scofunc
#endif
#ifndef segchain
   #define segchain     __segchain
#endif
#ifndef shared
   #define shared       __shared
#endif
#ifndef slice
   #define slice        __slice
#endif
#ifndef useix
   #define useix        __useix
#endif
#ifndef waitfor
   #define waitfor      __waitfor
#endif
#ifndef waitfordone
   #define waitfordone  __wfd
#endif
#ifndef wfd
   #define wfd          __wfd
#endif
#ifndef xdata
   #define xdata        __xdata
#endif
#ifndef xmem
   #define xmem         __xmem
#endif
#ifndef xstring
   #define xstring      __xstring
#endif
#ifndef yield
   #define yield        __yield
#endif

#use "CPUPARAM.LIB"     // CPU-specific settings

/////// BEGIN USER MODIFIABLE MACROS /////////////////////////////////

//***** Miscellaneous Information *******************************************
#define PRODUCT_NAME   "Universal Rabbit BIOS Version 10.70"

//***** MMIDR definition **********************************************

// Address line values for setting the origin directive MMIDR inversions
// Bitwise OR with __SEPARATE_INST_DATA__ to automatically enable/disable
// respective settings in the MMIDR (from MMIDR_VALUE) and origin directive.
#define _SID_ADDRESS_16 (0x10 & (__SEPARATE_INST_DATA__ << 4))
#define _SID_ADDRESS_MSB (0x80 & (__SEPARATE_INST_DATA__ << 7))


// Specify inversions for the MMIDR required for separate I&D space
#if FLASH_COMPILE || FAST_RAM_COMPILE
   #define DATASEG_INV _SID_ADDRESS_MSB
   #define CODESEG_INV _SID_ADDRESS_16
#elif RAM_COMPILE
   #define DATASEG_INV _SID_ADDRESS_16
   #define CODESEG_INV _SID_ADDRESS_16
#endif

#ifdef DATAORG
   #define ROOT_SIZE_4K (DATAORG >> 12)
   #warnt "DATAORG is deprecated; Define ROOT_SIZE_4K instead."
#else
   #define DATAORG ROOT_SIZE
#endif

#if __SEPARATE_INST_DATA__
   #ifndef ROOT_SIZE_4K
      #define ROOT_SIZE_4K 0x3U
   #endif
   #ifndef STACK_SIZE_4K
      #define STACK_SIZE_4K 0x1U
   #endif

   #if FLASH_COMPILE || RAM_COMPILE
      // Error checking for macro definitions.
      #if STACK_SIZE_4K + ROOT_SIZE_4K > 0xEU
         #fatal "Root segment and stack segment cannot overlap."
      #else
         #define ROOTCODE_SIZE_4K (0xEU - STACK_SIZE_4K)
         #define ROOTCONSTANTS_SIZE_4K ROOT_SIZE_4K
         #define ROOTDATA_SIZE_4K (0xEU - STACK_SIZE_4K - ROOT_SIZE_4K)
      #endif

   #elif FAST_RAM_COMPILE
      #ifndef BBROOTDATA_SIZE_4K
         #define BBROOTDATA_SIZE_4K 0x1U
      #endif

      #if STACK_SIZE_4K + ROOT_SIZE_4K + BBROOTDATA_SIZE_4K > 0xEU
         #if BBROOTDATA_SIZE_4K != 0
            #fatal "Root segment and bbrootdata segment cannot overlap."
         #else
            #fatal "Root segment and stack segment cannot overlap."
         #endif
      #else
         #define ROOTCODE_SIZE_4K (0xEU - STACK_SIZE_4K - BBROOTDATA_SIZE_4K)
         #define ROOTCONSTANTS_SIZE_4K ROOT_SIZE_4K
         #define ROOTDATA_SIZE_4K (0xEU - STACK_SIZE_4K - BBROOTDATA_SIZE_4K \
                                                                 - ROOT_SIZE_4K)
      #endif
   #endif

#else //if not separate I&D
   #ifndef ROOT_SIZE_4K
      #define ROOT_SIZE_4K 0x6U
   #endif
   #ifndef STACK_SIZE_4K
      #define STACK_SIZE_4K 0x1U
   #endif

   #if FLASH_COMPILE || RAM_COMPILE
      #if STACK_SIZE_4K + ROOT_SIZE_4K > 0xEU
         #fatal "Root segment and stack segment cannot overlap."
      #else
         #define ROOTCODE_SIZE_4K ROOT_SIZE_4K
         #define ROOTDATA_SIZE_4K (0xEU - STACK_SIZE_4K - ROOT_SIZE_4K)
      #endif
   #elif FAST_RAM_COMPILE
      #ifndef BBROOTDATA_SIZE_4K
         #define BBROOTDATA_SIZE_4K 0x1U
      #endif

      #if STACK_SIZE_4K + ROOT_SIZE_4K + BBROOTDATA_SIZE_4K > 0xEU
         #if BBROOTDATA_SIZE_4K != 0
            #fatal "Root segment and bbrootdata segment cannot overlap."
         #else
            #fatal "Root segment and stack segment cannot overlap."
         #endif
      #else
         #define ROOTCODE_SIZE_4K ROOT_SIZE_4K
         #define ROOTDATA_SIZE_4K \
            (0xEU - STACK_SIZE_4K - BBROOTDATA_SIZE_4K - ROOT_SIZE_4K)
      #endif
   #endif
#endif

#if ROOT_SIZE_4K <= 0 || (7 - ROOT_SIZE_4K) < 0
   #error "The root segment must have size greater than zero."
   #fatal "Ensure that the ROOT_SIZE_4K macro definition is an unsigned value."
#endif

#define ROOTCODE_SIZE (ROOTCODE_SIZE_4K << 12)

#ifdef ROOTCONSTANTS_SIZE_4K
   #define ROOTCONSTANTS_SIZE (ROOTCONSTANTS_SIZE_4K << 12)
#endif

#define ROOTDATA_SIZE (ROOTDATA_SIZE_4K << 12)
#define STACK_SIZE (STACK_SIZE_4K << 12)

// BBROOTDATASIZE is deprecated and will be removed in a future release.
#if defined BBROOTDATASIZE
   #warns "BBROOTDATASIZE is deprecated, define BBROOTDATA_SIZE_4K instead."
   #warns "BBROOTDATA_SIZE_4K specifies the size of bbram root data in 4K " \
                                                                       "blocks."
 #if defined BBROOTDATA_SIZE_4K
   #undef BBROOTDATASIZE
   #warns "BBROOTDATASIZE will be redefined to suit the predefined " \
                                                           "BBROOTDATA_SIZE_4K."
 #elif defined BBROOTDATA_SIZE
   #undef BBROOTDATASIZE
   #warns "BBROOTDATASIZE will be redefined to suit the predefined " \
                                                              "BBROOTDATA_SIZE."
 #else
   #define BBROOTDATA_SIZE BBROOTDATASIZE
   #warns "BBROOTDATA_SIZE has been defined to suit the deprecated " \
                                                               "BBROOTDATASIZE."
 #endif
#endif

#if defined BBROOTDATA_SIZE_4K && defined BBROOTDATA_SIZE
 #if BBROOTDATA_SIZE != BBROOTDATA_SIZE_4K << 12
   #undef BBROOTDATA_SIZE
   #define BBROOTDATA_SIZE    (BBROOTDATA_SIZE_4K << 12)
   #warns "Redefined BBROOTDATA_SIZE to be compatible with BBROOTDATA_SIZE_4K."
 #endif
#elif defined BBROOTDATA_SIZE_4K
   #define BBROOTDATA_SIZE    (BBROOTDATA_SIZE_4K << 12)
#elif defined BBROOTDATA_SIZE
   #define BBROOTDATA_SIZE_4K (BBROOTDATA_SIZE >> 12)
 #if BBROOTDATA_SIZE != BBROOTDATA_SIZE_4K << 12
   #error "Can't define BBROOTDATA_SIZE_4K to suit the predefined " \
                                                              "BBROOTDATA_SIZE."
   #fatal "Define BBROOTDATA_SIZE_4K (as 4K blocks) instead of defining " \
                                                              "BBROOTDATA_SIZE."
 #else
   #warns "Defined BBROOTDATA_SIZE_4K to suit the predefined BBROOTDATA_SIZE."
   #warns "Define BBROOTDATA_SIZE_4K (as 4K blocks) instead of defining " \
                                                              "BBROOTDATA_SIZE."
 #endif
#else
   #define BBROOTDATA_SIZE_4K 0u
   #define BBROOTDATA_SIZE    (BBROOTDATA_SIZE_4K << 12)
#endif

// BBROOTDATASIZE is deprecated and will be removed in a future release.
#if !defined BBROOTDATASIZE
   #define BBROOTDATASIZE BBROOTDATA_SIZE
#endif


#use "CLONECONFIG.LIB"
#use "FATCONFIG.LIB"
#use "MEMCONFIG.LIB"
#use "DKCONFIG.LIB"

//********************************************************************
//  Advanced configuration items follow
//  (Primarily of interest to board makers)
//********************************************************************

/*
	The macro MAX_USERBLOCK_SIZE defines the amount of flash reserved for the
	User and system ID blocks and excluded from other use. The User block may
	have a size of 0, but the system ID block will always reserve at least
	0x1000 bytes.

	The value of this macro must always be a positive multiple of 0x1000 (4KB)
	and should always reflect the maximum size of the system ID plus User blocks
	that are installed on any Rabbit board used in a custom application. To
	facilitate development of multiple applications, a custom value for this
	macro may be added into Dynamic C's Project Options' Defines box.

	Note that simply changing the value of this macro does not change the actual
	size of the system ID or User blocks. It is also necessary to use a write
	system ID block program to change the actual size of the ID blocks area on
	each board that will use an application with a modified MAX_USERBLOCK_SIZE.
*/
#ifndef MAX_USERBLOCK_SIZE
   #define MAX_USERBLOCK_SIZE 0x8000
#endif

////////// END OF USER-MODIFIABLE MACROS /////////////////////////////////
#if (MAX_USERBLOCK_SIZE&0x0fff)
   #fatal "MAX_USERBLOCK_SIZE must be a multiple of 0x1000"
#endif
#if (MAX_USERBLOCK_SIZE<0x1000)
   //Not enough space was reserved for the system ID block.
   #fatal "MAX_USERBLOCK_SIZE must be at least 0x1000"
#endif

#define WATCHCODESIZE    0x800   // Number of root RAM bytes for RAM segment.
                                 // Must be 0, 0x400, 0x800 or 0x1000
#if (WATCHCODESIZE != 0 && WATCHCODESIZE != 0x400 && \
     WATCHCODESIZE != 0x800 && WATCHCODESIZE != 0x1000)
   #fatal "WATCHCODESIZE must be set to 0, 0x400, 0x800 or 0x1000"
#endif

#ifdef ZERO_OUT_STATIC_ // Define macro to zero out static data on startup/reset
   #define ZERO_OUT_STATIC_DATA 1//  Not compatible with protected variables.
#else                            //  Does not conflict with GLOBAL_INIT.
   #define ZERO_OUT_STATIC_DATA 0
#endif

#if (RAMONLYBIOS == 1)
   #if !RAM_COMPILE
      #fatal "Must compile to RAM when using RAM-only BIOS!"
   #endif
#endif

#ifdef CC_VER
   #define __DC__    CC_VER
#endif

typedef unsigned char      uint8;
typedef unsigned short int uint16;
typedef short int          int16;
typedef unsigned long int  uint32;
typedef long int				int32;
typedef unsigned long int  faraddr_t;
typedef unsigned long int  longword;     // Unsigned 32 bit
typedef unsigned           word;
typedef unsigned char      byte;


///////////////////////////////////////////////////////////////////////
// ***** BIOS data structures *****************************************

typedef struct {
   unsigned short addr;  // address
   uint16 base;          // base (BBR or CBR)
} ADDR24_S;

typedef union {
   unsigned long l;    // long for increment/decrement
   struct {
      ADDR24_S a; // the address itself
   } aaa;
} ADDR24;

typedef struct
{
   unsigned    short  Size;
   unsigned    short  Type;
   unsigned    long   Mods;
} TypedInfo;

typedef union
{
   unsigned short  Integer;
   unsigned long   Long;
   float               Float;
   ADDR24          Addr;
} TypedValue;

typedef struct
{
   TypedInfo       TypedData;
   TypedValue      Value;  // value pushed without padding
} TypedArg;

struct progStruct {
   ADDR24      RCB,RCE,    // root code (Begin and End)
               XCB,XCE,    // extended code (Begin and End)
               RDB,RDE,    // root data (Begin and End)
               XDB,XDE,    // extended data (RAM) (Begin and End)
               RCDB,RCDE,  // root constant data (Begin and End)
               HPA;        // Highest address of program in flash
                           //    (max of RCDE, RCE, XCE)
   unsigned short  auxStkB,// aux stack Begin
               auxStkE,    // end
               stkB,       // stack begin
               stkE,       // end
               freeB,      // free begin
               freeE,      // end
               heapB,      // heap begin
               heapE;      // end
};

struct _dcParam
{
	word errorExit;		//	errorExit routine pointer
};

#define ERROR_EXIT DCParam+errorExit

//***** Settings for bank control registers ***************************
#define MB0CR_INVRT ((MB0CR_INVRT_A18<<4) | (MB0CR_INVRT_A19<<5))
#define MB1CR_INVRT ((MB1CR_INVRT_A18<<4) | (MB1CR_INVRT_A19<<5))
#define MB2CR_INVRT ((MB2CR_INVRT_A18<<4) | (MB2CR_INVRT_A19<<5))
#define MB3CR_INVRT ((MB3CR_INVRT_A18<<4) | (MB3CR_INVRT_A19<<5))

#if FLASH_COMPILE  // compiled to and running in flash
   #define MB0CR_SETTING    (FLASH_WSTATES | CS_FLASH | MB0CR_INVRT)
   #define MB1CR_SETTING    (FLASH_WSTATES | CS_FLASH | MB1CR_INVRT)
   #define MB2CR_SETTING    (RAM_WSTATES | CS_RAM | MB2CR_INVRT)

   #if RCM5700_SERIES
      #if _BOARD_TYPE_ == RCM5700
         // RCM5700 or equivalent
         #define MB3CR_SETTING (FLASH_WSTATES | CS_FLASH | MB0CR_INVRT)
      #else
         // RCM5750 or equivalent
         #define MB3CR_SETTING (RAM2_WSTATES | CS_RAM2 | MB3CR_INVRT)
      #endif
   #else
      #define MB3CR_SETTING (RAM_WSTATES | CS_RAM | MB3CR_INVRT)
   #endif
#endif

#if FAST_RAM_COMPILE // compiled to flash, running in RAM
   #define MB0CR_SETTING    (RAM_WSTATES | CS_RAM  | MB0CR_INVRT)

   #ifdef CS_RAM3 // if board has a third RAM defined
      #define MB1CR_SETTING (RAM3_WSTATES | CS_RAM3)
   #else // if board does not have a third RAM defined, mirror first RAM
      #define MB1CR_SETTING (RAM_WSTATES | CS_RAM | MB1CR_INVRT)
   #endif

   #if defined RAM2_WSTATES && defined CS_RAM2
      // the fast RAM board has a RAM2 (typical, and it's expected to be BB)
      #define MB2CR_SETTING (RAM2_WSTATES | CS_RAM2 | MB2CR_INVRT)
   #else
      // the fast RAM board has no RAM2 (atypical)
      #if _SERIAL_BOOT_FLASH_
         #define MB2CR_SETTING (RAM_WSTATES | CS_RAM | MB2CR_INVRT)
      #else
         #define MB2CR_SETTING (FLASH_WSTATES | CS_FLASH | MB2CR_INVRT)
      #endif
   #endif

   #if  _SERIAL_BOOT_FLASH_
      #define MB3CR_SETTING (MB2CR_SETTING)
   #else
      #define MB3CR_SETTING (FLASH_WSTATES | CS_FLASH | MB3CR_INVRT)
   #endif
#endif

#if RAM_COMPILE  // compiled to and running in RAM
   #define MB0CR_SETTING (RAM_WSTATES | CS_RAM | MB0CR_INVRT)

   #if RCM5700_SERIES
      #if _BOARD_TYPE_ == RCM5700
         // RCM5700 or equivalent
         #define MB1CR_SETTING (RAM_WSTATES | CS_RAM | MB1CR_INVRT)
         #define MB2CR_SETTING (FLASH_WSTATES | CS_FLASH | MB2CR_INVRT)
      #else
         // RCM5750 or equivalent
         #define MB1CR_SETTING (RAM2_WSTATES | CS_RAM2 | MB1CR_INVRT)
         #define MB2CR_SETTING (FLASH_WSTATES | CS_FLASH | MB2CR_INVRT)
      #endif
   #else
      #define MB1CR_SETTING (RAM_WSTATES | CS_RAM | MB1CR_INVRT)
      #define MB2CR_SETTING (RAM_WSTATES | CS_RAM | MB2CR_INVRT)
   #endif

   #define MB3CR_SETTING (FLASH_WSTATES | CS_FLASH | MB3CR_INVRT)
#endif

//***** Compute the SEGSIZE value *************************************
#if FAST_RAM_COMPILE && (RUN_IN_RAM_CS == 0x2)
   // SEGSIZE reg value
   #define CLONE_MEMBREAK _cexpr((STACKORG/256U) + (DATAORG/4096U))
   #if __SEPARATE_INST_DATA__
      // Invert A16 for the data segment to allow cloning to run out of flash
      #define CLONE_MMIDR_VALUE 0x4 | (MMIDR_VALUE)
   #else
      // Adjust data seg. to allow cloning to run in flash (ram moved up 512K)
      #define CLONE_DATASEGVAL _cexpr((0x40 << MSB_OFFSET ) + (DATASEGVAL))
   #endif
#endif

#if RAM_COMPILE
   // Calculate reserved Xmem size for RAM compile mode
   #if __SEPARATE_INST_DATA__
      #if RAM_SIZE <= 0x20
         #if FAT_TOTAL
            #warnt "May be too little xmem RAM to support the FAT Data store."
         #endif
         #if _SOS_USERDATA
            #warnt "May be too little xmem RAM to support the User Data store."
         #endif
      #endif
   #endif
#endif

//***** Compute the MMU segment registers *****************************

// Specify inversions for the MMIDR required for separate I&D space
#if __SEPARATE_INST_DATA__
   #if FLASH_COMPILE || FAST_RAM_COMPILE
      #if RCM5600W_SERIES     //the RCM5600W uses only the base A16 inversion
         #define _ROOTSEG_INVERT_A16
      #else
         #define _ROOTSEG_INVERT_A16
         #define _DATASEG_INVERT_MSB
      #endif
   #elif RAM_COMPILE
      #define _ROOTSEG_INVERT_A16
      #define _DATASEG_INVERT_A16
   #endif
#endif

// Setup the enable and inversion bits for the MMIDR.  The _SEPARATE_INST_DATA_
// macro value is used to set the I&D enable (5th) bit of the MMIDR.
#define MMIDR_ENABLE (__SEPARATE_INST_DATA__ << 5)

#ifdef _ROOTSEG_INVERT_A16
   #ifdef _ROOTSEG_INVERT_MSB
      #define MMIDR_ROOT_INV 0x03
   #else
      #define MMIDR_ROOT_INV 0x01
   #endif
#else
   #ifdef _ROOTSEG_INVERT_MSB
      #define MMIDR_ROOT_INV 0x02
   #else
      #define MMIDR_ROOT_INV 0x00
   #endif
#endif

#ifdef _DATASEG_INVERT_A16
   #ifdef _DATASEG_INVERT_MSB
      #define MMIDR_DATA_INV 0x0C
   #else
      #define MMIDR_DATA_INV 0x04
   #endif
#else
   #ifdef _DATASEG_INVERT_MSB
      #define MMIDR_DATA_INV 0x08
   #else
      #define MMIDR_DATA_INV 0x00
   #endif
#endif


// The actual value to load into the MMIDR register.
#define MMIDR_VALUE 0x80 | MMIDR_ENABLE | MMIDR_DATA_INV | MMIDR_ROOT_INV \
                                                         |(CS1_ALWAYS_ON << 4)

#if __SEPARATE_INST_DATA__ || (FAST_RAM_COMPILE && (RUN_IN_RAM_CS == 0x2))
   #define DATASEGVAL 0
#else
   #if FAST_RAM_COMPILE
      #if BBROOTDATA_SIZE > 0
         #define DATASEGVAL BBROOTDATA_SEGMENT_VAL
      #else
         #define DATASEGVAL 0
      #endif
   #else
      #define DATASEGVAL ROOTDATA_SEGMENT_VAL
   #endif
#endif

#if FAST_RAM_COMPILE && BBROOTDATA_SIZE > 0
   // top of battery-backed root data org (data usage grows downward from top)
   #define BBROOTDATAORG STACKORG-0x1
   #if __SEPARATE_INST_DATA__ && !RCM5600W_SERIES
      // when separate I&D is enabled, most boards' DATASEG is set to 0
      #define BBDATASEGVAL 0
   #else
      // when separate I&D is disabled, or always for the RCM5600W family,
      //  DATASEG points into battery-backable SRAM
      #define BBDATASEGVAL BBROOTDATA_SEGMENT_VAL
   #endif
#else
   /*
      No distinct battery-backable root data, so just set a harmless default.
      I.E. either DATASEGVAL is used instead of BBDATASEGVAL or SEGSIZE is set
      such that the stack segment and data segment boundaries are equal and
      (because the stack segment "wins" in this case) there really is no
      distinct root data segment at all.
   */
   #define BBDATASEGVAL 0
#endif

#if __SEPARATE_INST_DATA__
   #define ROOTCONSTORG 0x0                  // Beginning of constants
   #define ROOTCONSTSEGVAL 0x0               // Constant segment offset
   #if RAM_COMPILE && (RAM_SIZE <= 0x20)
      // Beginning of root data
      #define ROOTDATAORG STACKORG - FLASHDRIVER_SIZE - BBROOTDATA_SIZE - 0x1
   #else
      // Beginning of root data
      #define ROOTDATAORG STACKORG - FLASHDRIVER_SIZE - VECTOR_TABLE_SIZE \
                                                         - BBROOTDATA_SIZE - 0x1
   #endif
#else // not Separate I&D
   // Beginning of root data
   #define ROOTDATAORG STACKORG - FLASHDRIVER_SIZE - VECTOR_TABLE_SIZE \
                                                         - BBROOTDATA_SIZE - 0x1
#endif

#define BBRAM_RESERVE_SIZE (USERDATA_NBLOCKS*4096L) + FAT_TOTAL+ \
                           (FS2_RAM_RESERVE*4096L) + (ERRLOG_NBLOCKS*4096L)

#if FAST_RAM_COMPILE
   #define RAM_RESERVE_SIZE  TC_SYSBUF_BLOCK+FLASH_BUF_SIZE
#else
   #define RAM_RESERVE_SIZE  BBRAM_RESERVE_SIZE+TC_SYSBUF_BLOCK+FLASH_BUF_SIZE
#endif


//***** This is to let Dynamic C programs know we are running in Flash or RAM.
//***** _RAM_ and _FLASH_ are defined by the compiler.  RUN_IN_RAM
//***** predates these compiler generated macros.
//***** FAST_RAM_COMPILE mode compiles to flash and runs in RAM
#if FLASH_COMPILE
   #define RUN_IN_RAM 0
#else
   #define RUN_IN_RAM 1      // RAM_COMPILE || FAST_RAM_COMPILE
#endif

//***** Tell the compiler this is a BIOS ******************************
#pragma CompileBIOS

#use "memory_layout.lib"

//MEMBREAK will be loaded into the SEGSIZE register
#if FAST_RAM_COMPILE
   #if BBROOTDATA_SIZE > 0
      #define MEMBREAK _cexpr((BBROOTDATA_LOGICAL_END >> 8) + \
                                               (BBROOTDATA_LOGICAL_START >> 12))
   #else
      #define MEMBREAK _cexpr(((0xEU - STACK_SIZE_4K) << 4) | \
                                                         (0xEU - STACK_SIZE_4K))
   #endif
#else
   #define MEMBREAK _cexpr((ROOTDATA_LOGICAL_END >> 8) + \
                                                 (ROOTDATA_LOGICAL_START >> 12))
#endif

//RAMSR_VALUE specifies the RAM segment, which is used in separate I&D mode
//for watch expressions.
#if __SEPARATE_INST_DATA__
   #if WATCHCODESIZE == 0
      #define RAMSR_VALUE 0
   #elif WATCHCODESIZE == 0x400
      #define RAMSR_VALUE _cexpr(((WATCHCODE_LOGICAL_START >> 8) & 0xFC) | 1)
   #elif WATCHCODESIZE == 0x800
      #define RAMSR_VALUE _cexpr(((WATCHCODE_LOGICAL_START >> 8) & 0xFC) | 2)
   #elif WATCHCODESIZE == 0x1000
      #define RAMSR_VALUE _cexpr(((WATCHCODE_LOGICAL_START >> 8) & 0xFC) | 3)
   #endif
#endif

//Values used to load into GOCR and GCSR initially
#define GOCR_VALUE 0xA0 //set STATUS pin low, CLK pin off
#define GCSR_VALUE 0x08 //use undivided main oscillator for processor and
                        //peripheral clocks, periodic interrupt off

//***** Libraries needed by the BIOS *********************************
#use "SYSIO.LIB"        // IO register assignments and functions
#use "DBUGKERN.LIB"     // The debug kernel
#use "CSUPPORT.LIB"     // Some C support functions

#if _SERIAL_BOOT_FLASH_==0
   #use "FLASHWR.LIB"      // The flash writing functions
#else
   #use "BOOTDEV_SFLASH.LIB" // Serial boot flash functions
#endif

#use "IDBLOCK.LIB"      // Flash ID block access functions (and CRC).
#use "ERRORS.LIB"       // Runtime error handling
#use "TC.LIB"           // New-style target communications
#use "TC_SYSTEMAPP.LIB" // The System-type handler
#use "DKTCBASE.LIB"     // debug communications interface
#use "MUTIL.LIB"        // Math support for error logging and manipulations

#if __SEPARATE_INST_DATA__
   #use "SEPARATEID.LIB"
#endif

#include <xmem.h>			// macros for converting to/from linear/segmented addr
#use "RWEB_SUPPORT.LIB"

#if (ENABLE_CLONING == 1)
   #if _SERIAL_BOOT_FLASH_==0
      #use "CLONE.LIB"        // Contains cloning support functions
   #else
      #use "CLONESERIALF.LIB"
   #endif
#endif

#use "SERIAL_FLASH_BOOT_LOADER.LIB" //definition of SERIAL_FLASH_BOOT_LOADER

#use "firmware_info.lib"				// firmware_info struct embedded in BIOS

#if _BOARD_TYPE_ == RCM5750
 #ifdef BU_ENABLE_MINILOADER
	unsigned _bu_ml_StartPage;
	unsigned long _bu_ml_FW_length;
	#use "miniLoader.lib"
 #endif
#endif

//*** end of macro definitions ***

//***** variable definitions ******************************************
#define BIOSSTACKSIZE 256     //the stack in the BIOS does not need to be large
char dbXPC;
char dkB1CR;
char dkB2CR;
char dkB3CR;
char dkB4CR;
char dkB5CR;
char dkB6CR;

void *dkcState;                    // the address (state) of debug kernel FSM
char BiosStack[BIOSSTACKSIZE];     // initial stack for bios in the data segment
char commBuffer[256];              // communication buffer

struct _dcParam DCParam;
char OPMODE;

char _dkenable28,_dkdisable28;       // byte to store at RST28 enable, disable
int dkInitStkTop;
int bios_divider19200;  // timer reg setting for 19200 baud calculated by BIOS
int freq_divider;       // adjusted timer register for 19200 baud
int bios_timer_count;    // raw counter value from BIOS timing calculation
char periodic_counter;   // temporary variable used during timing calculation

// store value of GCSR register to determine reset reason
char reset_status;

char dkcstartuserprog;

char _BIOSdownloading2serial;

#if CPU_ID_MASK(_CPU_ID_) >= R6000
  int cpuGf_spd_chg;   // used to ensure get_cpu_frequency correct after CPU
                       //  speed change
  // Variables used by the R6000 PLL
  char TAT4RShadowOrig;
  char	_pll_GCDR_initial;
  int	freq_dividerOrig;
  char _pll_CPU_usingPLL;
#endif

// the internal I/O shadow registers
// *** do not change the order of definition ***
char GCSRShadow, GOCRShadow, GCDRShadow;
char PADRShadow;
char PBDRShadow;
char PCDRShadow, PCFRShadow;
char PDDRShadow, PDCRShadow, PDFRShadow, PDDCRShadow, PDDDRShadow;
char PEDRShadow, PECRShadow, PEFRShadow, PEDDRShadow;

#if CPU_ID_MASK(_CPU_ID_) >= R6000
	char	PE0CRShadow, PE1CRShadow, PE2CRShadow, PE3CRShadow,
			PE4CRShadow, PE5CRShadow, PE6CRShadow, PE7CRShadow;
	char PFDRShadow, PFCRShadow, PFFRShadow, PFDDRShadow;
	char PGDRShadow, PGCRShadow, PGFRShadow, PGDDRShadow;
	char PFDCRShadow, PFALRShadow, PFAHRShadow;
	char PGDCRShadow, PGALRShadow, PGAHRShadow;
	char POCRShadow;
	char SGMCRShadow;
#endif

char MMIDRShadow, MECRShadow;
char MB3CRShadow, MB2CRShadow, MB1CRShadow, MB0CRShadow;
char IB0CRShadow, IB1CRShadow, IB2CRShadow, IB3CRShadow, IB4CRShadow,
     IB5CRShadow, IB6CRShadow, IB7CRShadow;
char I0CRShadow, I1CRShadow;
char TACSRShadow, TACRShadow, TAT1RShadow, TAT2RShadow, TAT3RShadow,
     TAT4RShadow, TAT5RShadow, TAT6RShadow, TAT7RShadow;
char TBCSRShadow, TBCRShadow;
char SPCRShadow, SACRShadow, SBCRShadow, SCCRShadow, SDCRShadow,
     SECRShadow, SFCRShadow;
char SAERShadow, SBERShadow, SCERShadow, SDERShadow;

char GCM0RShadow, GCM1RShadow;
char MTCRShadow;

char GPSCRShadow;
char PBDDRShadow;
char BDCRShadow;
char ICCSRShadow, ICCRShadow;
char ICT1RShadow, ICS1RShadow, ICT2RShadow, ICS2RShadow;
char PWL0RShadow, PWM0RShadow, PWL1RShadow, PWM1RShadow,
     PWL2RShadow, PWM2RShadow, PWL3RShadow, PWM3RShadow;
char QDCSRShadow, QDCRShadow;
char SEERShadow, SFERShadow;
char TAPRShadow;
char TAT8RShadow, TAT9RShadow, TAT10RShadow;
char STKCRShadow;

char PCALRShadow, PCAHRShadow, PCDCRShadow, PCDDRShadow;
char PDALRShadow, PDAHRShadow;
char PEALRShadow, PEAHRShadow, PEDCRShadow;
char TCCSRShadow, TCCRShadow;
char NACRShadow, NAPCRShadow, NARCRShadow, NATCRShadow, NACSRShadow;
char MACRShadow, IHCRShadow, IHSRShadow, ACS0CRShadow, ACS1CRShadow;

//Rabbit 5000 and newer processors have parallel port H available
#if (CPU_ID_MASK(_CPU_ID_) >= R5000)
   char PHDRShadow, PHFRShadow, PHDCRShadow, PHDDRShadow;
   // note, no shadow for ENPR -- just read the port directly
   // e.g.: WrPortI( ENPR, NULL, RdPortI(ENPR) | 0x80);
#endif

// Required in case board has Wifi which shares a SPI port
int _sf_spi_busy;
#define SPI_BUSY_FLAG_DECLARED

// prog_param is defined by the compiler.
extern struct progStruct prog_param;

// Required for board_update.lib, copy of running firmware's CRC-32 (from end
// of firmware image) before that space is used by xalloc and friends.
long _bu_boot_crc32;

// Assembly label _firmware_info is the the firmware_info embedded in BIOS.
extern const __far firmware_info_t _firmware_info;

// Assembly label bu_loadapp_64k is the serial flash offset of the 64K offset
// of the boot image.  Not declared const since BIOS modifies it.
extern __far long bu_loadapp_64k;

//*** end of variable definitions ***
//***** Begin prototypes **********************************************
__root void biosmain();
__root void dkInit();
__xmem void WaitSettle();
__root void dkClearBxCR();
__xmem void dkSaveBxCR();
__xmem void dkUpdateBxCR();
__xmem void dkInitTC();
__root void dkcDoSerial();
__root void DevMateSerialISR();
__root void DevMateReadPort();
__root void DevMateWritePort();
__root void divider19200();
__root void __bp_init();
__root void _xexit();
__root void __xexit__();
__xmem void dkSetIntVecTabP();
__xmem void dkSetIntVecTabR();
__root void dkStartup();
__root void dkEnd();
char      dkInBiosStack;
unsigned int _xexithook;
#if _SERIAL_BOOT_FLASH_ == 1
   __root int _sbf_BIOS_LoadApp();
#endif
//*** End Prototypes ***

#ifndef _TARGETLESS_COMPILE_
   #define _TARGETLESS_COMPILE_ 0
#endif

// Defines for firminfo structure near start of BIOS
#if __SEPARATE_INST_DATA__
	#define CC_FLAGS_SID _FIRMINFO_FLAG_SEP_INST_DATA
#else
	#define CC_FLAGS_SID 0
#endif
#if DEBUG_RST == 1
	#define CC_FLAGS_RST28 _FIRMINFO_FLAG_RST28
#else
	#define CC_FLAGS_RST28 0
#endif
#if RAM_COMPILE || SUPPRESS_FAST_RAM_COPY
	#define CC_FLAGS_RAM_COMPILE _FIRMINFO_FLAG_RAM_COMPILE
#else
	#define CC_FLAGS_RAM_COMPILE 0
#endif
#ifdef BU_ENABLE_SECONDARY
	#if (_SERIAL_BOOT_FLASH_ != 1)
		#fatal "BU_ENABLE_SECONDARY is not a valid option for this hardware."
	#endif
	#define CC_FLAGS_SECONDARY _FIRMINFO_FLAG_CAN_BE_SECONDARY
#else
	#define CC_FLAGS_SECONDARY 0
#endif

// temporary definition for CC_FLAGS
#define CC_FLAGS (CC_FLAGS_SID + CC_FLAGS_RST28 + CC_FLAGS_RAM_COMPILE \
	+ CC_FLAGS_SECONDARY)

// default setting for user-defined firmware version number
#ifndef _FIRMWARE_VERSION_
	#define _FIRMWARE_VERSION_ 0
#endif

#ifndef _FIRMWARE_NAME_
	#define _FIRMWARE_NAME_ ""
#endif

// allow user to override build-time timestamp with a fixed value
#ifndef _FIRMWARE_TIMESTAMP_
	// compiler-generated _DC_GMT_TIMESTAMP_ is the # of seconds since 1/1/1980.
   #define _FIRMWARE_TIMESTAMP_ _DC_GMT_TIMESTAMP_
#endif

// end of firminfo defines

// begin defines for powerfail-safe RPU
#ifdef BU_ENABLE_SECONDARY
	#if (_SERIAL_BOOT_FLASH_ != 1)
		#warns "BU_ENABLE_SECONDARY only works for serial boot flash."
		#undef BU_ENABLE_SECONDARY
	#endif
#endif
#ifdef BU_ENABLE_SECONDARY
	// These macros define the size of the firmware_marker_t structure defined in
	// board_update.lib, and the offsets to the 16-bit sequence number and 32-bit
	// flash address of the firmware image.
	#define BU_MARKER_SIZE        128   // bytes in marker (checksum is last 2)
	#define BU_MARKER_SEQ_OFS     1     // offset to 16-bit sequence number
	#define BU_MARKER_ADDR_OFS    3     // offset to 32-bit address
	#define BU_MARKER_CMD_OFS     7     // offset to 32-bit flash command

	/*
	   Addresses of the A-valid and B-valid markers.  These markers must be on
	   separate pages of the serial flash, and located past the pages used by
	   the coldloader and warmloader.  Because of the space required by the boot
	   loader, BU_MARKER_ADDR_A will always be 1056.  On products with a page
	   size of 1056 bytes (either 264 or 528), BU_MARKER_B can be at the next
	   page boundary (1056+264 or 1056+528).

	   512KB and 1MB use 264-byte pages, 2MB and 4MB use 528-byte pages, 8MB uses
	   a 1056-byte page.

	   For consistency, and to allow external utilities (like RFU) to understand
	   firmware markers, we use fixed offsets for all devices.
	*/

	#define BU_MARKER_ADDR_A		1056     // start of A-valid marker
	#define BU_MARKER_ADDR_B		2112     // start of B-valid marker

	// Default flash offset for image A (follows loader and firmware markers).
	#define BU_IMAGE_ADDR_A			(3 * 1056ul)


	#if _FLASH_SIZE_ == 1024/4 || \
									((_RAM_SIZE_ == 512/4) && (_FLASH_SIZE_ > 1024/4))
		#if MAX_USERBLOCK_SIZE > 32 * 1024
			#error "Address calculation below only protects 32KB for userblock."
		#endif
	   /*
	      Products with 1MB flash (264 * 4096): reserve 3*1056 for boot loader
	      and two markers, 32*1024 for UserBlock, 1*1024 for SystemIDBlock and
	      then split the rest.  Support two 510KB firmware images.

	      Use this sizing for boards with 512KB of SRAM as well, since you'll
	      never need more than 510KB to store its firmware.

	      1978 = (264 * 4096 - 3 * 1056 - 33 * 1024) / 264 / 2 and rounded down
	   */
	   #define _MAX_PFSRPU_BINSIZE	(1978 * 264ul)

	#elif _FLASH_SIZE_ >= 2048/4
		#if MAX_USERBLOCK_SIZE > 56 * 1024
			#error "Address calculation below only protects 56KB for userblock."
		#endif
	   /*
	      Products with 2MB+ flash (528 * 4096): reserve 3*1056 for boot loader
	      and two markers, 32*1024 for UserBlock and then split the reset.  Would
	      support two 1037KB images, but we use 1986 * 528 instead, which allows
	      1024KB per image (better matching RAM size) and leaves space for a
	      larger UserBlock.  Allowing a full 1024KB may be generous -- based on
	      the orgtable of the BL4S150, we only need to support 951KB per image.

	      1986 = (1024 * 1024 / 528) and rounded up
	   */
	   #define _MAX_PFSRPU_BINSIZE	(1986 * 528ul)

	#endif

	// Check if MAX_FIRMWARE_BINSIZE exceeds limits and reduce if needed
	#ifdef MAX_FIRMWARE_BINSIZE
		// preferred method of limiting firmware size
		#undef BU_IMAGE_ADDR_B			// ignore old method of setting size
		#if defined _MAX_PFSRPU_BINSIZE 				\
										&& (MAX_FIRMWARE_BINSIZE > _MAX_PFSRPU_BINSIZE)
			#warns "MAX_FIRMWARE_BINSIZE is too big, setting it to maximum."
			#undef MAX_FIRMWARE_BINSIZE
		#endif
	#elif defined BU_IMAGE_ADDR_B
		// legacy (pre-10.64) method of limiting firmware size
		#if defined _MAX_PFSRPU_BINSIZE 			\
					&& (BU_IMAGE_ADDR_B - BU_IMAGE_ADDR_A > _MAX_PFSRPU_BINSIZE)
			#warns "BU_IMAGE_ADDR_B is too big, setting it to maximum."
			#undef BU_IMAGE_ADDR_B
		#else
			#define MAX_FIRMWARE_BINSIZE	(BU_IMAGE_ADDR_B - BU_IMAGE_ADDR_A)
		#endif
	#endif

	// If MAX_FIRMWARE_BINSIZE not set by user (via that macro or some other),
	// set it to the maximum possible size (if we were able to calculate that).
	#ifndef MAX_FIRMWARE_BINSIZE
		#ifndef _MAX_PFSRPU_BINSIZE
			// need to define _MAX_PFSRPU_BINSIZE for this board type
		   #fatal "Need to update StdBIOS.C for BU_ENABLE_SECONDARY on this board."
		#endif
		#define MAX_FIRMWARE_BINSIZE		_MAX_PFSRPU_BINSIZE
      #define __AUTO_FIRMWARE_BINSIZE
	#endif

	// If BU_IMAGE_ADDR_B wasn't set to a valid value by the user, set it
	// based on the value of MAX_FIRMWARE_BINSIZE.
	#ifndef BU_IMAGE_ADDR_B
		#define BU_IMAGE_ADDR_B				(BU_IMAGE_ADDR_A + MAX_FIRMWARE_BINSIZE)
	#endif

	// Default command bytes to send to flash to read from ADDR_A and ADDR_B
	#define BU_FLASH_CMD_A			(BU_IMAGE_ADDR_A / 33 * 64ul)
	#define BU_FLASH_CMD_B			(BU_IMAGE_ADDR_B / 33 * 64ul)

	// Other code needs to avoid writing to boot and secondary firmware on flash
	#define BU_PROTECT_BYTES      (BU_IMAGE_ADDR_B + MAX_FIRMWARE_BINSIZE)

	#if CPU_ID_MASK(_CPU_ID_) >= R5000
		// The tripleted-in serial boot loader has already configured PDDDR to set
		//  PDDR bit 6 as an output; chip select configuration is not needed here.
	   #define BU_SBF_CS_CONFIG
	   #define BU_SBF_CS_REG         PDDR
	   #define BU_SBF_CS_ENABLE      ioi res 6, (hl)
	   #define BU_SBF_CS_DISABLE     ioi set 6, (hl)
	#elif RCM4300_SERIES
	   // disable SD chip select on NAPCR4
	   #define BU_SBF_CS_CONFIG      ld iy, NAPCR $ ioi set 4, (iy)
	   #define BU_SBF_CS_REG         NAPCR
	   #define BU_SBF_CS_ENABLE      ioi set 5, (hl)
	   #define BU_SBF_CS_DISABLE     ioi res 5, (hl)
	#elif BL4S100_SERIES
	   #define BU_SBF_CS_CONFIG      ld iy, PEDDR $ ioi set 4, (iy)
	   #define BU_SBF_CS_REG         PEDR
	   #define BU_SBF_CS_ENABLE      ioi res 4, (hl)
	   #define BU_SBF_CS_DISABLE     ioi set 4, (hl)
	#else
	   #fatal "Need to update StdBIOS.C for BU_ENABLE_SECONDARY on this board."
	#endif
// end defines for powerfail-safe RPU
#else
	#if _RUN_FROM_RAM
	  // Maximum size for a firmware image, typically the number of bytes of
	  // fast RAM, but could be limited if available boot flash is smaller than
	  // fast RAM.
	  #define _MAX_PHYSICAL_BINSIZE \
									(ORG_RAM_SIZE - ORG_BBRAM_SIZE - ORG_USER_BLOCK_SIZE)
	#else
	  // The whole parallel flash except the user block is available.
	  #define _MAX_PHYSICAL_BINSIZE \
									(ORG_FLASH_SIZE - MAX_USERBLOCK_SIZE)
	#endif

	#ifdef MAX_FIRMWARE_BINSIZE
		#if MAX_FIRMWARE_BINSIZE > _MAX_PHYSICAL_BINSIZE
			#warns MAX_FIRMWARE_BINSIZE is too big, setting it to physical maximum.
			#undef MAX_FIRMWARE_BINSIZE
			#define MAX_FIRMWARE_BINSIZE	_MAX_PHYSICAL_BINSIZE
         #define __AUTO_FIRMWARE_BINSIZE
		#else
			#define _MAX_EXPECTED_BINSIZE	MAX_FIRMWARE_BINSIZE
		#endif
	#else
		#define MAX_FIRMWARE_BINSIZE		_MAX_PHYSICAL_BINSIZE
      #define __AUTO_FIRMWARE_BINSIZE
	#endif

	// _MAX_EXPECTED_BINSIZE is used by SFLASH.LIB to see if there is enough
	// room for a FAT filesystem on the serial boot flash.  Either set to
	// MAX_FIRMWARE_BINSIZE (if set by the user and valid) or _MAX_ORG_BINSIZE
	// (set in memory_layout.lib to the max based on the org table).  Since
	// _MAX_ORG_BINSIZE may vary from release to release (if the org table
	// changes), do not use this macro for setting the location of temp firmware,
	// a FAT partition, or the protected area of serial flash.
	#ifndef _MAX_EXPECTED_BINSIZE
		#define _MAX_EXPECTED_BINSIZE		_MAX_ORG_BINSIZE
	#endif
#endif
//***** BIOS main program *********************************************

/*
	Contents of first 1KB of serial flash when BU_ENABLE_SECONDARY is defined.

	bu_loadapp_64k: Stores address used by bootdev_sflash.lib to load remaining
			firmware, after first 64KB. Note that this variable must ALWAYS be at
			this address, in order for an old boot loader to work correctly with
			new firmware.

	_fletcher8: Helper function to checksum markers.

	bu_warmloader: Helper function relocated above 64KB boundary to load first
			64KB of image A or image B.

	bu_check_markers: Loader that looks at markers A and B. Possibly copies
			bu_warmloader to high address and configures serial flash so
			warmloader can reload first 64KB of RAM from image A or B.

	.boot_normal: Normal BIOS entry point

	Offsets on Rabbit 4000 (approximate offsets indicated with *)
		0x0000	jp bu_check_markers
		0x0003	bu_loadapp_64k
		0x0007	_fletcher8 (helper function)
		0x0027*	bu_warmloader
		0x0047*	bu_check_markers
		0x00C7*	.boot_normal
					BIOS
					BIOS Vars
					firmware_info
					More BIOS
		0x0400	Maximum address for boot loader

	Offsets on Rabbit 5000/6000 (approximate offsets indicated with *)
		0x0000	nop $ nop $ jp 0x0200 $ nop (triplet magic)
		0x0006	tripletted boot loader (about 300 bytes)
		0x0160*	_fletcher8
		0x0180*	bu_warmloader
		0x01FC	bu_loadapp_64k
		0x0200	bu_check_markers
		0x0280*	.boot_normal
					BIOS
					BIOS Vars
					firmware_info
					More BIOS
		0x0400	Maximum address for boot loader

*/
#asm __nodebug
biosmain::
#if (CPU_ID_MASK(_CPU_ID_) >= R5000) && (_SERIAL_BOOT_FLASH_ == 1)
   ; The serial flash boot loader is a tripleted program embedded into the
   ; start of the BIOS.  On power-up or reset, this program will boot the BIOS
   ; off of the serial flash.  It has no effect during debug mode.

   ; As triplets, these six bytes will write 0xC3 to address 0x0000 and 0x00 to
   ; address 0x0002.  That's OK because the tripleted bootloader will overwrite
   ; them when it re-loads these bytes with the first 64KB of firmware.
   ; As assembly code, they are nop $ nop $ jp 512 $ nop, and will jump
   ; around the tripleted bootloader and helper functions that follows.
   db 0x00, 0x00, 0xC3
   db 0x00, 0x02, 0x00

   ; The tripleted bootloader sets up registers and contains a small program
   ; to load the first 64KB off of the serial flash, starting at address 0x0000.
   ; It is designed to be copied to and run from 0x014000.
   ; This macro is defined in serial_flash_boot_loader.lib, auto-generated by
   ; BL_triplets.exe using COLDLOAD_serflash_boot.c.
   SERIAL_FLASH_BOOT_LOADER

   #ifndef BU_ENABLE_SECONDARY
   	; If we don't have code to handle a secondary image, we'll just have
   	; a big blank spot in the firmware between the end of the boot loader
   	; and address 512 (destination of jumping over the boot loader).  To aid
   	; in debugging, let's just throw some text in that will show up in a
   	; hex editor.  Make sure it's aligned at a 16-byte address following the
   	; SERIAL_FLASH_BOOT_LOADER code.
      align 16
      db "secondary loader disabled"
      align 512
      ; standard BIOS code follows
   #endif
#endif

#ifdef BU_ENABLE_SECONDARY
	#if (CPU_ID_MASK(_CPU_ID_) < R5000)
		; Don't have a tripleted loader, so we're still at address 0
		; Jump over the fletcher checksum and warmloader
		jp bu_check_markers
		; Make sure bu_loadapp_64k is always at a fixed address and set to 0.
		; Once Dynamic C is released, this address can't change.  Since the
		; old warmloader (from the Z-image) sets bu_loadapp_64k, any new
		; image needs to put it in the exact same place.
		bu_loadapp_64k::	; address for _sbf_BIOS_LoadApp to load firmware > 64K
			dw 0, 0			; if zero, we're booting the Z image for the first time
	#endif

; _fletcher8:
; Returns 16-bit checksum of 1 to 65,535 bytes of data (as used in
; firmware_marker_t).  Must generate the same checksum as _bu_fletcher8.
; params: far pointer to data in px and non-zero length in hl.  Return address
;				in IX (since there isn't necessarily a stack set up)
; return: 16-bit checksum in hl (checkA in H and checkB in L)
; trashes: bc, a, px, hl (return value)
_fletcher8::
	ld		bc, hl				; number of bytes to sum

	clr	hl						; checksum starts at all 1's
	dec	hl

.loop:
	ld		a, (px + 0)			; load byte and increment pointer
	ld		px, px + 1

	add	a, h					; update checksum
	adc	a, 0
	ld		h, a
	add	a, l
	adc	a, 0
	ld		l, a
	dwjnz	.loop

	jp		(ix)					; returns checksum in hl
; end of _fletcher8

   ; this warmloader is copied to 0x014000 by code at .warmload
bu_warmloader::
	; On entry, flash is ready to send bytes
	; de = PDDR
	; hl = SBDR + _SR_OFFS
	; pz = flash offset of program

		; receive 64K of program bytes, writing to physical address 0
		ld		px, 0
		ld    bc, 0
ioi	ld		a, (SBF_DR+_AR_OFFS)	; start a byte receive operation (discard byte)
.warm_readloop:
		;; Optimization Note:
		;;   Because the SPI divisor value may be greater than zero, we can't be
		;;   sure that new serial data will always be waiting when we get back to
		;;   the top of this loop. Therefore, we always test the receiver status.
.warm_readwait:						; wait for the receive to complete
ioi	ld		a, (hl)					; get status
		bit	7, a						; test receiver status bit
		jr		z, .warm_readwait		; spin if no receive data yet

ioi	ld		a, (SBF_DR+_AR_OFFS)	; get the byte, start SPI transfer of next one
		ld		(px), a					; store the byte
		ld		px, px+1					; point to next byte
		dwjnz	.warm_readloop

      ld		hl, BU_SBF_CS_REG
		BU_SBF_CS_DISABLE

		; modify firmware to have flash address stored at fixed offset
		ldf	(bu_loadapp_64k), pz

      ; read is complete, jump to newly loaded code
      jp    0x0000
bu_warmloader_end::
; end of bu_warmloader

	#if (CPU_ID_MASK(_CPU_ID_) >= R5000)
		; Make sure bu_loadapp_64k is always at a fixed address and set to 0.
		; Once Dynamic C is released, this address can't change.  Since the
		; old warmloader (from the Z-image) sets bu_loadapp_64k, any new
		; image needs to put it in the exact same place.
			align 508		; must be 508 so bu_check_markers ends up at 512
		bu_loadapp_64k::	; address for _sbf_BIOS_LoadApp to load firmware > 64K
			dw 0, 0			; if zero, we're booting the Z image for the first time

	; Entry point for BIOS follows.  On the Rabbit 5000 and later, a jp 0 hits
	; two nops, then a jp 512 to end up here.
	#endif

bu_check_markers::
	#if (FAST_RAM_COMPILE && DEBUG_RST)
      ; Check SMODE pins for run mode.  If we're in run mode, we haven't booted
      ; from the serial flash and therefore don't need to look at the markers
      ; or load any code from the flash.
      ; This check is only necessary if code was compiled with RST28's enabled
      ; (which is what Dynamic C does when debugging).
	      ioi   ld    a, (SPCR)
	            and   0x60
	            cp    0x60
	            jp    z, .boot_normal
	#endif

	; cold loader jumps to this entry point, need to decide on which firmware
	; image to load -- Z, A or B?  The A-valid and B-valid markers have already
	; been copied to RAM.

	; If bu_loadapp_64k is non-zero, this BIOS was warmloaded from an
	; A or B image (or we're being rebooted) and should not check the markers.
	ldf	bcde, (bu_loadapp_64k)
	test	bcde
	jp		nz, .boot_normal

	; verify checksum for second marker
	; Note that we use ld instead of ldl -- BU_MARKER_ADDR_A is an address in
	; code space, not data space.

	; We should possibly check the first byte and make sure that it's set to
	; 0x01 before continuing.  Otherwise, the structure is potentially a new
	; version that we don't understand.
	ld		px, BU_MARKER_ADDR_B
	ld		a, (px)
	cp		0x0F
	jr		gt, .set_b_invalid
	ld		hl, BU_MARKER_SIZE - 2
	ld		ix, .b_checked
	jp		_fletcher8		; address in px, bytecount in hl, return addr in ix
.b_checked:
	ld		de, hl
	ld		hl, (BU_MARKER_ADDR_B + BU_MARKER_SIZE - 2)
	cp		hl, de
.set_b_invalid:
	flag	nz, hl			; hl == 1 for invalid
	ld		iy, hl			; set iy = 0 if marker B is valid

	ld		px, BU_MARKER_ADDR_A
	ld		a, (px)
	cp		0x0F
	jr		gt, .a_invalid
	ld		hl, BU_MARKER_SIZE - 2
	ld		ix, .a_checked
	jp		_fletcher8		; address in px, bytecount in hl, return addr in ix
.a_checked:
	ld		de, hl
	ld		hl, (BU_MARKER_ADDR_A + BU_MARKER_SIZE - 2)
	cp		hl, de
	jr		z, .a_valid

.a_invalid:
	; a is invalid, check b
	test	iy
	jr		z, .boot_b		; b is valid so boot it
	; fall through to .none_valid

.none_valid:
	; Neither A or B markers are valid, boot the Z image
	jp		.boot_z

.a_valid:
	; a is valid, check b
	test	iy
	jr		z, .both_valid

	; a is valid, b is invalid
	; fall through to .boot_a

.boot_a:
	; we have decided to boot the A image
	ld		bcde, (BU_MARKER_ADDR_A + BU_MARKER_ADDR_OFS)
	ld		jkhl, (BU_MARKER_ADDR_A + BU_MARKER_CMD_OFS)
	jr		.warmload

.both_valid:
	; both a and b are valid, compare sequence numbers using rollover
	; if sign bit in hl is set after doing unsigned subtraction, hl < de
	ld		hl, (BU_MARKER_ADDR_B + BU_MARKER_SEQ_OFS)
	ld		de, (BU_MARKER_ADDR_A + BU_MARKER_SEQ_OFS)
	sub	hl, de					; hl = b.sequence - a.sequence
	rl		hl							; shift sign bit into carry flag
	jr		c, .boot_a				; b.sequence < a.sequence
	; fall through to .boot_b

.boot_b:
	; we have decided to boot the b image
	ld		bcde, (BU_MARKER_ADDR_B + BU_MARKER_ADDR_OFS)
	ld		jkhl, (BU_MARKER_ADDR_B + BU_MARKER_CMD_OFS)
	; fall through to .warmload

.warmload:
	; Reload firmware from serial flash:
	;	BCDE = flash offset to start of firmware
	;	JKHL = address bytes in flash command form (lower 3 bytes)
		inc	bc					; add 64K to BCDE (now address for _sbf_BIOS_LoadApp)
		ld		pz, bcde			;		and pass to warmloader in pz

		; use BCDE' to temporarily store command bytes to send
		ex		jkhl, bcde		; BCDE = flash command
      exx						; BCDE' = flash command

	#if (CPU_ID_MASK(_CPU_ID_) >= R5000)
		; I/O registers already set up (by cold loader)
	#else
		; need to initialize I/O registers, code copied from _sbf_bios_initSPIB
		; no stack yet, so we can't call that function (or the functions it calls)
		; PB0, PC4, PC5, TAT5R
	         ld    a, SFDIVISOR
	   ioi   ld    (TAT5R),a   ; For SCLCK divisor
	         xor   a
	   ioi   ld    (PCDCR),a   ; Set TXB to push-pull
	   ioi   ld    (PDFR), a
	   #if BL4S100_SERIES
	   ioi   ld    (NAPCR), a  ; reset Ethernet output
	   #endif
	         inc   a           ; A = 0x01
	   ioi   ld    (PBDDR),a   ; Set PB0 to output for SCLCK
	   ioi   ld    (TACSR),a   ; Start clock

	   ioi   ld    a, (PCFR)
	         or    0x10
	         and   ~0x20
	   ioi   ld    (PCFR), a       ; & set Target comm bits
	   ioi   ld    a, (PCDDR)
	         or    0x10
	         and   ~0x20
	   ioi   ld    (PCDDR),a   ; Set TXB to output

      ; Initialize clocked serial port B (connected to SBF)
	         ld    a, SF_SPI_ECONTROL_VALUE
	   ioi   ld    (SBER),  a
	         ld    a, SBF_SPI_CONTROL
	   ioi   ld    (SBCR),  a

      ; Set up SEGSIZE and DATASEG to match how Rabbit 5000 coldloader does.
      ; Now we can copy the warmloader to 0x014000 and run it with jp 0x4000.
	         ld    a, 0xD4
	   ioi   ld    (SEGSIZE), a      ; stack at 0xD000, data at 0x4000
	         ld    a, 0x10
	   ioi   ld    (DATASEG), a      ; map data segment to 64K boundary
	#endif

		; copy the warmloader to 0x4000 (logical 0x4000 is mapped above 64k)
		ld		bc, bu_warmloader_end - bu_warmloader
		ld		hl, bu_warmloader
		ld		de, 0x4000
		ldir

		; Set up the serial flash for the warmloader
		; Using BU_SBF_CS_DISABLE before BU_SBF_CS_CONFIG prevents any possible
		;  output glitch!
		ld		hl, BU_SBF_CS_REG
		BU_SBF_CS_DISABLE
		BU_SBF_CS_CONFIG
.wl_dropDataLoop:
ioi	ld		a, (SBF_DR)				; clear the serial port data register / FIFO
ioi	ld		a, (SBF_DR+_SR_OFFS)	; check status register, is more data waiting?
		and	0xAC						; test Rx full/overrun, Tx full/sending bits
		jr		nz, .wl_dropDataLoop	; if TX busy or RX not empty go re-check . . .

		; from above, HL still contains BU_SBF_CS_REG
		BU_SBF_CS_ENABLE
		ld		hl, SBDR + _SR_OFFS
;; The industry standard 0x03 continuous read command followed by three address
;; bytes will not work for boards equipped with Atmel serial flash devices with
;; revision levels earlier than "D" (i.e. revisions prior to AT45DB041D,
;; AT45DB081D, AT45DB161D, AT45DB321D, AT45DB642D). The 0x03 command is also
;; limited to 33 MHz operation, maximum. In contrast, the Atmel-specific
;; "legacy" 0xE8 continuous read command is supported for all AT45DBxxxx serial
;; flash device revisions to date, at up to 66 MHz operation, maximum.
		ld		b', 0xE8					; Atmel-specific 0xE8 continuous read command
		ld		b, 8						; command + 3 address + 4 don't care bytes
.sbfStreamOutL:
		exx
		rlb	a, bcde					; swap in BCDE' and get next byte of command
		exx
ioi	ld		(SBF_DR), a				; send a command, address or don't care byte
;; The Rabbit's SPI transmitter idle status bit is actually set 1/2 bit-time
;; early. To compensate for this, we start a simultaneous receive plus transmit
;; operation and wait for the receive operation to complete. We discard the
;; received byte before proceeding.
		ld		a, SBF_SPI_CONTROL | SPI_RXMASK | SPI_TXMASK
ioi	ld		(SBF_DR+_CR_OFFS), a	; start simultaneous receive+transmit operation
.sbfSoTXwait:
ioi	ld		a, (hl)					; get status
		bit	7, a						; wait for receive not empty status bit set
		jr		z, .sbfSoTXwait

ioi	ld		a, (SBF_DR)				; clear Rx buffer not empty flag (discard byte)
		xor	a							; don't care byte will be 0 when "recirculated"
		djnz	.sbfStreamOutL

		; now jump to the warmloader that we've copied to a safe area in memory
		jp		0x4000

.boot_z:
		; For Z image, _sbf_BIOS_LoadApp needs to load from flash offset 64K
		ld		bcde, 0
		inc	bc						;load 64k into BCDE
		ldf	(bu_loadapp_64k), bcde

.boot_normal:
		; normal execution
#endif //BU_ENABLE_SECONDARY

#ifdef _ENABLE_16BIT_FLASH_
   ;Set up 16-bit devices.
   ;This is not needed for RAM compile mode; 16-bit mode will have already been
   ;set up in RAM by the cold loader.
   #if FLASH_COMPILE
      ; !!!! Warning: Use Rabbit 3000 opcodes only until the EDMR is set up.
      xor   a        ; a <= 00000000
      xor   a
      ld    h, a     ; h <= 00000000
      ld    h, a
      scf
      scf
      rla         ; a <= 00000001
      rla         ; a <= 00000010
      ld    b, a     ; b <= 00000010
      ld    b, a
      scf
      scf
      ; Since Rabbit processors start in Rabbit 3000 mode, 7F opcodes are
      ; actually on the mainpage, so we must db those opcodes directly. Check
      ; the Rabbit 3000 assembly manual for more information.
      ;adc a, b   ; a <= 00000101
      db    0x88
      ;adc a, b   ; a <= 00000111
      db    0x88
      ;add a, a   ; a <= 00001110
      db    0x87
      ;add a, a   ; a <= 00011100
      db    0x87
      scf
      scf
      ;adc a, h   ; a <= 00011101
      db    0x8C
      ;adc a, h
      db    0x8C
      ld    l, a     ; l <= 00011101
      ld    l, a
      ;ioi        ; two IOIs same as one
      ;ioi
      db    0xD3
      db    0xD3
      ld    (hl), b  ; MACR <= 00000010 (advanced 16-bit mode on CS0)
      ld    (hl), b  ; dummy memory write (no /WE)
      nop         ; required delay to start
      nop         ; up the 16-bit bus
      nop         ; required delay to start
      nop         ; up the 16-bit bus
      ld    a,0x01
ioi   ld    (PEAHR),a
      ld    a,0x10
ioi   ld    (PEFR),a
      ld    a,0x00       ; in case we got here from the 16-bit Pilot
ioi   ld    (MB0CR),a
      jr    _biosentry_
   #endif   //FLASH_COMPILE
#endif   //_ENABLE_16BIT_FLASH_

_biosentry_::

#ifdef MACR_SETTING
		ld		a,MACR_SETTING
ioi	ld		(MACR),a
		nop $ nop		; *always* follow any MACR update with two NOPs
#endif

      ; enable 15-bit internal I/O addresses, turn on separate I&D if enabled,
      ;  enable Rabbit 4000+ instruction set
      ld    a, MMIDR_VALUE
ioi   ld    (MMIDR), a
      ld    a, 0xC0
ioi   ld    (EDMR), a

#ifdef ACS1CR_SETTING
      ld    a, ACS1CR_SETTING
ioi   ld    (ACS1CR), a ;set CS1's wait states, enable byte reads/writes
#endif
#ifdef ACS2CR_SETTING
      ld    a, ACS2CR_SETTING
ioi   ld    (ACS2CR), a ;set CS2's wait states, enable byte reads/writes
								; This is only available on the Rabbit 6000
#endif

#if FLASH_COMPILE || FAST_RAM_COMPILE
      jp    InFlashNow
#else
      jp    InRAMNow
#endif
#endasm //root

// *** BIOS constants and special variables ***
// When I&D space is enabled, the constants and variables in the
//  following block must be accessed via physical reads/writes.
#asm
#if FAST_RAM_COMPILE
// Flag for the FAST_RAM_COMPILE copy
FastRAM_InRAM::
      dw    0
#endif
divider19200::
      db    _FREQ_DIV_19200_
#if ENABLE_CLONING == 1
I_am_a_clone::
      dw    0
I_am_a_sterile_clone::
      dw    0
#endif

; Insert firmware_info_t structure (defined in firmware_info.lib) for use
; in firmware updates.  Serial Flash Boot and cloning also make use of
; _program_HPA.
; Must be on a 64-byte boundary and fully-contained in first 1024 bytes of BIOS.
		align 64			; align to a 64-byte boundary by inserting NOP (0x00) bytes
_firmware_info::
_fi_running_start::
		#pragma crc32 begin
		db 0x42, 0x55, 0x44, 0x21		; magic marker (BUD!)
		db	0x01								; struct_ver
		dw	_BOARD_TYPE_					; board_type
_program_HPA::
_fi_running_length::
		#pragma filesize					; length (unsigned long)
		dw _FIRMWARE_VERSION_			; version
		dw CC_VER							; compiler_ver
		dw	CC_FLAGS							; flags

		dw (_FIRMWARE_TIMESTAMP_ & 0x0000FFFFul)	; lower word of build_timestamp
		dw (_FIRMWARE_TIMESTAMP_ >> 16)				; upper word of build_timestamp

		dw	(_DC_MB_TYPE_ & 0x0000FFFFul)				; lower word of MB_TYPE macro
		dw (_DC_MB_TYPE_ >> 16)							; upper word of MB_TYPE macro

		db 'C'								; compiler revision, first added in 10.72B

		; reserved, 10 bytes reserved for Digi's use
		db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		
		; user_defined, 8 bytes allocated for customer use
		db 0, 0, 0, 0, 0, 0, 0, 0

		db _FIRMWARE_NAME_				; program_name

		align 64			; align to a 64-byte boundary by inserting NOP (0x00) bytes

		#pragma crc32 end
_fi_running_end::
		#pragma crc32 insert				; struct_crc32
; end of firmware_info_t structure
#endasm //root

/*
// This sizeof() test results in the name being inserted into the .BIN twice.
#if sizeof _FIRMWARE_NAME_  > 20
	#fatal "_FIRMWARE_NAME_ must be defined as a zero to 19-byte " \
		"null-terminated string."
#endif
*/

//***** Begin BIOS code ***********************************************
#asm
InFlashNow::
//for FLASH_COMPILE or FAST_RAM_COMPILE, much of the initialization
//code resides in xmem so we need to jump there.
#if FLASH_COMPILE || FAST_RAM_COMPILE
dkInit::
      ljp   __dkinit__

  #if _BOARD_TYPE_ == RCM5750    // Must pull mini-loader into BIOS so
    #ifdef BU_ENABLE_MINILOADER  // that it resides low in program flash.
      ld hl, _bu_sfMiniLoader    // If used, BU_ENABLE_MINILOADER
    #endif                       // should be #define'd in the compiler
  #endif                         // options dialog box or this file.

#endasm //root

#asm __xmem
__dkinit__::
#endif

//for RAM_COMPILE the following code is still in root
InRAMNow::
      ipset 3

      ;; set 2 second WDT time out
      ld    a, 0x5A
ioi   ld    (WDTCR), a

      ;; turn off primary WDT
      ld    a, 0x51
ioi   ld    (WDTTR),a
      ld    a, 0x54
ioi   ld    (WDTTR),a

      ;; turn off secondary WDT
      ld    a, 0x5A
ioi   ld    (SWDTR),a
      ld    a, 0x52
ioi   ld    (SWDTR),a
      ld    a, 0x44
ioi   ld    (SWDTR),a

      ; set STATUS pin low and CLK pin off
      ld    a, GOCR_VALUE
      ; normal oscillator for processor and peripheral, periodic int. off
ioi   ld    (GOCR), a
      ld    a, GCSR_VALUE
ioi   ld    (GCSR), a

// *** Set the physical bank sizes, segment size and data segment ***
dkSetMMU:
      ld    a, MECR_VALUE
ioi   ld    (MECR), a
      ld    a, MEMBREAK
ioi   ld    (SEGSIZE), a

#if FAST_RAM_COMPILE
      ld    hl, _cexpr(BBDATASEGVAL)
#else
      ld    hl, _cexpr(DATASEGVAL)
#endif
ioi   ld    (DATASEGL), hl

#if __SEPARATE_INST_DATA__
      ld    a, RAMSR_VALUE
ioi   ld    (RAMSR), a
#endif

// ***** Do fast RAM copy if necessary *****
//Fast RAM copy for parallel flash boards
#if FAST_RAM_COMPILE && (_SERIAL_BOOT_FLASH_ == 0)
   ; SUPPRESS_FAST_RAM_COPY will be set to 1 if you compiled your program
   ; to RAM instead of flash.  The pilot BIOS will have already loaded
   ; the program into RAM if this is the case.
   #if !SUPPRESS_FAST_RAM_COPY
      ;Currently all parallel flash fast RAM boards map fast RAM to MB0.
      ;If the board has a second fast RAM, that is mapped to MB1, otherwise
      ;the first RAM is mirrored.
      ;Battery-backed RAM is mapped to MB2.
      ;Flash is mapped to MB3.  If the flash is larger than the size of
      ;a memory bank, only half of the flash ends up being mapped.
      ;
      ;MB3 - Flash
      ;MB2 - BB RAM
      ;MB1 - Fast RAM
      ;MB0 - Fast RAM
      ;
      ;All the fast RAM copy does is mirror the contents of flash into
      ;the fast RAM.  Afterwards, code execution takes place in the fast RAM.

      ; First we check if we are running in flash.
      ; If not, we are running in RAM and have already finished the copy.
      ; FastRAM_InRAM: 1 = running in RAM, 0 = running in flash
      ; A physical load is used because separate I&D might be active.
      ldf   hl, (FastRAM_InRAM)
      test  hl
      jr    nz, .fastRAMCopyDone ; skip copy if already done

.swapProgToRAM::
      ; Swap the compiled program over to RAM from flash. Maps flash to
      ; the first 2 physical memory banks (MB0CR, MB1CR), and RAM to
      ; the top banks (MB2CR, MB3CR)
      ld    hl, _cexpr((MB3CR_SETTING)<<8 | (MB3CR_SETTING))
ioi   ld    (MB0CR), hl  ; this maps the flash (quad 3 setting) to quads 0 and 1
      ld    hl, _cexpr((MB1CR_SETTING)<<8 | (MB0CR_SETTING))
ioi   ld    (MB2CR), hl

      ld    jkhl, 0
      ld    px, jkhl
      ld    jk, 1 << (MSB_BIT - 16)
      ld    py, jkhl
      ld    hl, _RAM_SIZE_>>4          ; Number of 64K blocks to copy
      ld    bcde, 0                    ; bc = 0
.copy64kLoop:
      copy              ; because bc = 0, 64k will be copied at a time
                        ; BC, PX and PY will be correctly set for the next copy
      dec   hl
      test  hl
      jr    nz, .copy64kLoop

      ;Start executing from RAM.  This is needed so that the FastRAM_InRAM
      ;flag that gets written is the one in RAM, not flash.  Other memory
      ;setup is handled by fastRAMCopyDone.
      ld    a, _cexpr(MB0CR_SETTING)
ioi   ld    (MB0CR), a

   #endif   //!SUPPRESS_FAST_RAM_COPY
//end parallel flash fast RAM copy

//Fast RAM copy for serial flash boards
#elif (FAST_RAM_COMPILE) && (_SERIAL_BOOT_FLASH_ == 1)
      ; Currently serial flash boards will have the following memory mapping:
      ;
      ; MB3 - (mirror of MB2)
      ; MB2 - Secondary RAM (either BB RAM or internal RAM, depending on board)
      ; MB1 - Fast RAM
      ; MB0 - Fast RAM
      ;
      ; Serial flash boards in run mode will need to copy their program from
      ; the serial flash into fast RAM, as code cannot be executed from
      ; serial flash.  All such boards guarantee that the first 64k of serial
      ; flash will be loaded into whatever device is currently mapped to MB0
      ; Note that on some boards this is not the device that will be mapped to
      ; MB0 later!
      ;
      ; _sbf_BIOS_LoadApp will bring the remainder of the program into RAM.
      ; MB2 and MB3 are loaded with the MB0 and MB1 settings and then all
      ; program bytes above 64k are copied from serial flash into MB2/MB3.

      ; FastRAM_InRAM: 1 = copy is done, 0 = copy not done
      ; The physical address is used because separate I&D might be active.
      ldf   hl, (FastRAM_InRAM)
      test  hl
      jp    nz, .fastRAMCopyDone ; skip copy if already done

   #if ENABLE_CLONING
      ;If you are a clone, force the fast RAM copy instead of checking SMODE
      ;pins.  A just-cloned serial flash board will have its smode pins tied as
      ;though it is in debug mode, but it will not have its full program in RAM.
      ldf   hl, (I_am_a_clone)
      test  hl
      jr    nz, .skipSMODEcheck
   #endif

      ; Check SMODE pins for run mode, don't do copy if in debug mode.
      ; This is because the serial flash pilot BIOS will load the program into
      ; RAM as well as the serial flash; repeating the copy is unnecessary.
ioi   ld    a, (SPCR)
      and   0x60
      cp    0x60
      jp    z, .fastRAMCopyDone
.skipSMODEcheck:
   #if (CPU_ID_MASK(_CPU_ID_) >= R5000)
      ; Rabbit 5000-based serial flash boards will boot up in internal RAM.
      ; Because the main fast RAM is external, the first 64k in the
      ; internal RAM must be mirrored into the external RAM as well.
      #ifdef _ENABLE_16BIT_RAM_
      ; If the external RAM is 16-bit, it needs to be set up before it can
      ; be used properly.
      ld    a, MACR_SETTING		;enable advanced 16-bit mode on CS1
ioi   ld    (MACR), a
		nop $ nop		; *always* follow any MACR update with two NOPs
      ld    a, ACS1CR_SETTING
ioi   ld    (ACS1CR), a ;set CS1's wait states, enable byte reads/writes
      ld    a, 0x10
ioi   ld    (PEDDR), a  ;set pin E4 to /A0, which is UB/LB toggle on 16-bit RAM
      ld    a, 0x01
ioi   ld    (PEAHR), a
      ld    a, 0x10
ioi   ld    (PEFR), a
      #endif

      ld    a, _cexpr(MB0CR_SETTING)
ioi   ld    (MB2CR), a

      ;copy 64k from internal RAM to external RAM
      ld    bc, 0
      ld    px, 0
      ld    py, 1 << MSB_BIT   ;the start of MB2
      copy

   #endif
      ; The physical D000-DFFF range should be unused at this point and is an
      ; easy place to stick a temporary stack.  All fast RAM boards currently
      ; use a physical memory space of more than one megabyte, and for boards
      ; with more than 1M memory space, physical addresses below address 0xE000
      ; cannot safely be used for xmem code.  This means that with or without
      ; separate I&D mode, you are guaranteed that this range will not contain
      ; any code.  Root data is already guaranteed to be mapped elsewhere.
      ;
      ; Additionally, because D000-DFFF lies below 64k, the fast RAM copy will
      ; not overwrite a stack placed there.
      xor   a
ioi   ld    (STACKSEG), a
      ld    sp, 0xDFFF

      call  _sbf_BIOS_LoadApp   ;load all code/constants from the flash into RAM
#endif
//end serial flash fast RAM copy

.fastRAMCopyDone::
; Following a fast RAM copy, the current device mapped to MB0 and the device
; that is actually supposed to be mapped to MB0 contain the same code at this
; point.  Switching from one device to the other will be seamless.
; For boards that do not have fast RAM, this simply maps devices appropriately.
      ld    a, _cexpr(MB0CR_SETTING)
ioi   ld    (MB0CR), a
      ld    a, _cexpr(MB1CR_SETTING)
ioi   ld    (MB1CR), a
      ld    a, _cexpr(MB2CR_SETTING)
ioi   ld    (MB2CR), a
      ld    a,  _cexpr(MB3CR_SETTING)
ioi   ld    (MB3CR), a

; Certain boards with 16-bit memory devices will start up with a default number
; of wait states.  The correct device setup is performed here.
#if FLASH_COMPILE || RAM_COMPILE
   #ifdef _ENABLE_16BIT_FLASH_
      ld    a, ACS0CR_SETTING
ioi   ld    (ACS0CR), a
      #ifdef __ALLOW_16BIT_AUXIO_DEFECT
ioi   ld    a,(MACR)
      and   0xF9                 ; remove CS0 setting
      or    0x02                 ; set advanced 16-bit mode for CS0
ioi   ld    (MACR),a
		nop $ nop		; *always* follow any MACR update with two NOPs
      #endif
   #endif

   #ifdef _ENABLE_16BIT_RAM_
      ld    a, ACS1CR_SETTING
ioi   ld    (ACS1CR), a
   #endif
#endif

// Not implemented with block moves because of potential assembler replacements
// that assume a stack exists. This relies on ROOTDATA_SIZE being a multiple of
// four, which is true since it should also be a multiple of 4K.
#if ZERO_OUT_STATIC_DATA && ROOTDATA_SIZE >= 4096u
      //****************************************
      // Initialize dataseg data area to zeros
      //  ~250K clocks for 0x7000 bytes
      //***************************************
      ld    a, 0x5A              ; hit watchdog
ioi   ld    (WDTCR), a
      ldl   px, _cexpr(ROOTDATA_LOGICAL_START)
      ld    bc, _cexpr(ROOTDATA_SIZE/4u)
      ld    jkhl, 0
.zISDloop:
      ld    (px+0), jkhl         ; 20
      ld    px, px+4             ; 6
      dwjnz .zISDloop            ; 8

      ld    a, 0x5A              ; hit watchdog
ioi   ld    (WDTCR), a
#endif

#if FAST_RAM_COMPILE
      ; Have we copied the firmware to RAM already?  If not, make a copy of
      ; the firmware's CRC32.

      ; FastRAM_InRAM: 1 = running in RAM, 0 = running in flash
      ldf   hl, (FastRAM_InRAM)
      test  hl
      jr    nz, .biosSkipCRC32Copy ; skip copy if already done

		; set the flag so we know that the copy is complete
      inc	hl
      ldf   (FastRAM_InRAM), hl
#endif

; Make a copy of the crc32 from this firmware image in RAM, before it's
; wiped out by xalloc re-using that space.  Use <ldf> to read
; _fi_running_length, since it's an assembler label (and is a code
; address, unaffected by Separate I&D).  Use <ld> to save _bu_boot_crc32
; since it's a root variable.
; Equivalent C code:
; _bu_boot_crc32 = *(long far *)
;									((*(long far *) _fi_running_length) - 4)
		ldf	px, (_fi_running_length)	; px = <# of bytes in firmware>
		ld		bcde, (px-4)		; bcde = <last 4 bytes of firmware image>
		ld		(_bu_boot_crc32), bcde

.biosSkipCRC32Copy:

// Now that we have mapped our memory devices and possibly zeroed out our
// static data area, we can safely save things to memory.
      ld    a, MMIDR_VALUE
      ld    (MMIDRShadow), a
ioi   ld    a, (MB0CR)
      ld    (MB0CRShadow), a
ioi   ld    a, (MB1CR)
      ld    (MB1CRShadow), a
ioi   ld    a, (MB2CR)
      ld    (MB2CRShadow), a
ioi   ld    a, (MB3CR)
      ld    (MB3CRShadow), a

dkSetSP:
      ;; set up temporary BIOS stack in the array BiosStack
      ld    hl, BiosStack
      ld    de, BIOSSTACKSIZE-1
      add   hl, de               ; hl points to the bottom of the stack
      ld    sp, hl               ; temporarily set stack pointer for next call

      xor   a
      call  dkcLogicalToPhysical ; convert stack address to physical address
      ld    a,c                  ; load phys addr bits 19-16 into a
      ex    de,hl                ; move phys addr LSB into hl
      ld    b,0xd                ; normalize to 0xd000 range
      _GEN_LIN2SEG(b)            ; generate normalized segmented address

      ld    sp,hl                ; set stack pointer to 0xd000 address
      ex    jk,hl
ioi   ld    (STACKSEGL),hl      ; set STACKSEG accordingly

      ld    a, 1                 ; Now that all the memory devices are mapped,
      ld    (dkInBiosStack), a   ;  set our "in BIOS stack" flag.

; ***** Detect clock speed *****
      ;because the timing procedure will trash the status bits of GCSR,
      ;reset_status must be saved first
ioi   ld    a, (GCSR)
      ld    (reset_status), a

      ;In RAM_COMPILE mode, you may reach this point with the clock doubler on.
      ;The timing calculation assumes that the doubler is off, so it must be
      ;disabled here.  The doubler setting will be restored by _more_inits02().
      xor   a
ioi   ld    (GCDR), a

      ;On some boards the real time clock may take some time to settle.
      ;Since this measurement depends on the accuracy of the real time clock,
      ;we should wait until it is stable before proceeding.
      lcall WaitSettle

      ;We need to know the board's clock speed.  This is accomplished by
      ;running periodic interrupts, triggered by the real time clock, in
      ;parallel with timer A interrupts, triggered by the main clock.
      ;By default, timer A is driven by main clock/2.  If you trigger
      ;timer A interrupts every 225 cycles, and run the periodic interrupt
      ;128 times (1/16th second), your final timer count will equal the
      ;oscillator frequency / 7200.  As an example, the formula for computing
      ;the rounded timer A divider value needed for 57600 baud becomes:
      ;
      ;divider = (timer_count + 128) / 256
      ;
      ;This same procedure is used by the cold loader during bootstrapping.

      ;set up interrupt vector table
      ld    a, 0xff & (INTVEC_BASE >> 8)
      ld    iir, a
      ;set up interrupt vectors
      ld    a, _OP_JP
      ld    (INTVEC_BASE+PERIODIC_OFS), a
      ld    (INTVEC_BASE+TIMERA_OFS), a
      ld    hl, _BIOStiming_periodic_timer_isr
      ld    (INTVEC_BASE+PERIODIC_OFS+1), hl
      ld    hl, _BIOStiming_timerA_timer_isr
      ld    (INTVEC_BASE+TIMERA_OFS+1), hl

      ;set up interrupt-related registers
      ld    a, 224     ;prepare timer A interrupt but do not activate yet
ioi   ld    (TAT1R), a
      ld    a, 0x03    ;enables timer A and timer A interrupts
ioi   ld    (TACSR), a

      ld    a, 129   ;we need one extra since the first interrupt doesn't count
      ld    (periodic_counter), a
      clr   hl
      ld    (bios_timer_count), hl

ioi   ld    a, (GCSR)              ;reading GCSR clears periodic interrupt
      ld    a, GCSR_VALUE | 0x02   ;enable periodic interrupt at priority 2
ioi   ld    (GCSR), a
      ld    a, 0x03                ;enable timer A interrupt at priority 3
ioi   ld    (TACR), a

      ipset 1           ;turn on interrupts

      ;wait for timing measurement to complete (periodic_counter == 0)
      ;If startup time is crucial, other initialization tasks could be
      ;performed while the ISRs run in the background.
._BIOStiming_timer_wait_loop:
      ld    a, (periodic_counter)
      or    a
      jr    nz, ._BIOStiming_timer_wait_loop

      ipres
      ld    a, GCSR_VALUE     ;disable periodic interrupt
ioi   ld    (GCSR), a
      xor   a                 ;disable timer A interrupt
ioi   ld    (TACR), a

      ;Calculate 19200 baud divider, which may be used after this point.
      ;The calculation is: (count * 3 + 128) / 256
      ld    hl, (bios_timer_count)
      ld    de, hl
      add   hl, de
      add   hl, de

		; A = (HL + 128) / 256
		xor	a
		rl		l				; shift top bit of L into carry
      adc	a, h			; load A with H plus possibly carry from L (rounding)

      clr	hl				; promote A to 16-bit register HL
      ld    l, a

      ld    (bios_divider19200), hl  ; save for later use
      jr    _BIOStiming_end
#if !RAM_COMPILE
#endasm //xmem if FLASH_COMPILE or FAST_RAM_COMPILE

//interrupt vectors used by the timing measurement must go in root
#asm __root
#endif
_BIOStiming_periodic_timer_isr::
      push  af          ;protect A and flags
ioi   ld    a, (GCSR)   ;reading GCSR clears interrupt
      ld    a, (periodic_counter)   ;how many more times this ISR will run
      dec   a
      ld    (periodic_counter), a
      pop   af
      ipres
      ret

_BIOStiming_timerA_timer_isr::
      push  af          ;protect A and flags, HL
      push  hl
ioi   ld    a, (TACSR)  ;clear interrupt
      ;only increment bios_timer_count if periodic counter is 128 or less
      ld    a, (periodic_counter)
      cp    a, 128
      jr    gtu, .timerA_isr_exit
      ;increment bios_timer_count
      ld    hl, (bios_timer_count)
      inc   hl          ;hl holds the number of times this ISR has run
      ld    (bios_timer_count), hl
.timerA_isr_exit:
      pop   hl
      pop   af
      ipres
      ret
#if !RAM_COMPILE
#endasm //root if FLASH_COMPILE or FAST_RAM_COMPILE

#asm __xmem
#endif

//end of clock speed detection
_BIOStiming_end::

      ;do flash initialization
      lcall _more_inits0
      ;initialize status pin, clocks, clock doubler, debug baud rate
      lcall _more_inits02

;******************************************************************************
#if RAM_COMPILE
; Enter here for software reset requested by Dynamic C
; to turn off periodic interrupt
dkInit::
      ipset 3
      ld    a, GCSR_VALUE  ; normal oscillator, processor and peri.
                           ;  from main clock, no periodic interrupt
ioi   ld    (GCSR), a
#endif
;******************************************************************************

      ;  set serial port a to async, 8-bit, interrupt priority 1
      ld    a, 0x01
ioi   ld    (SACR), a
      ld    (SACRShadow), a
      xor   a
ioi   ld    (SAER), a
      ld    (SAERShadow), a
      ; make sure port C is set up for serial port A TX/RX
ioi   ld    a, (PCFR)
      or    0x40
ioi   ld    (PCFR), a
      ld    (PCFRShadow), a

      lcall dkInitTC          ;initialize target communications
      lcall dkSetIntVecTabP

      ;set up serial A ISR
      ld    a, _OP_JP         ; jump instruction
      ld    (INTVEC_BASE+SERA_OFS), a
      ld    hl, DevMateSerialISR
      ld    (INTVEC_BASE+SERA_OFS+1), hl

      lcall _init_IO_regs

      // Check for run mode. If both SMODE pins high,
      //  programming cable is attached and we're in
      //  program/debug mode
ioi   ld    a, (SPCR)
      and   0x60
      cp    0x60
      jp    nz, RunMode

#if (ENABLE_CLONING==1)
      // Clones are already programmed and can't be debugged.
      // If they are still attached to the cloning board, they
      // will have their SMODE pins tied high, but they should
      // still act like they are in run mode.
      ldf   hl, (I_am_a_clone)
      test  hl
      jp    nz, RunMode
#endif

#if RAM_COMPILE
      //in a RAM compile, we must wait for the rest of the program to be
      //uploaded from Dynamic C before we can continue.
      //dkcstartuserprog will be set once the program is loaded.
      lcall  _init_dkLoop
.waitforuserprog:
      call  bioshitwd
      ld    a,(dkcstartuserprog)
      or    a
      jr    z,.waitforuserprog
#endif
      ld    hl, OPMODE
      ld    (hl), 0x88

      //StartUserCode is in RabbitBios.c
      //dkStartup and premain will then be called

      jp    StartUserCode

#endasm  //xmem if FAST_RAM_COMPILE or FLASH_COMPILE, root if RAM_COMPILE

#asm
RunMode::

#if (ENABLE_CLONING==1)
      ;Do not clone if:
      ;- you are a sterile clone
      ;- you are a master and no cloning cable is attached
      ldf   hl, (I_am_a_sterile_clone)
      test  hl
      jp    nz, NotCloning

   #if CL_FORCE_MASTER_MODE == 0
      // Enter clone mode if only cloning board detected
      // see if cloning cable is attached (is PB1 low?)
      // CloneMode will not return.
      ld    a, 0x02
      ld    hl, PBDR
ioi   and   a, (hl)
      jr    nz,  NotCloning
   #endif
      //if CL_FORCE_MASTER_MODE, always jump to CloneMode
      ljp   CloneMode
#endif   //ENABLE_CLONING

NotCloning:

      ipset 3
      lcall dkSetIntVecTabR      ; disable debug RSTs
      ipset 0

//***** Jump to user code *********************************************
      ld    hl, OPMODE
      ld    (hl), 0x80

      xor   a
ioi   ld    (SACR),a          ; disable comm interrupts.
      ld    (SACRShadow),a
      //StartUserCode is in RabbitBios.c
      //dkStartup and premain will then be called

      jp    StartUserCode
#endasm
// *** End of main BIOS code ***



//***** BIOS functions ************************************************
#asm __root
dkClearBxCR::
   push	af
   xor	a
   ioi	ld	(B1CR),a
   ioi	ld	(B2CR),a
   ioi	ld	(B3CR),a
   ioi	ld	(B4CR),a
   ioi	ld	(B5CR),a
   ioi	ld	(B6CR),a
   pop	af
   ret
#endasm
#asm __xmem
dkSaveBxCR::
	push	af
	ioi ld	a,(B1CR)
   ld		(dkB1CR),a
	ioi ld	a,(B2CR)
   ld		(dkB2CR),a
	ioi ld	a,(B3CR)
   ld		(dkB3CR),a
	ioi ld	a,(B4CR)
   ld		(dkB4CR),a
	ioi ld	a,(B5CR)
   ld		(dkB5CR),a
	ioi ld	a,(B6CR)
   ld		(dkB6CR),a
   pop	af
   lret

dkUpdateBxCR::
	push	af
   ld		a,(dkB1CR)
	ioi ld	(B1CR),a
   ld		a,(dkB2CR)
	ioi ld	(B2CR),a
   ld		a,(dkB3CR)
	ioi ld	(B3CR),a
   ld		a,(dkB4CR)
	ioi ld	(B4CR),a
   ld		a,(dkB5CR)
	ioi ld	(B5CR),a
   ld		a,(dkB6CR)
	ioi ld	(B6CR),a
   pop	af
	lret

// This code repeatedly measures the value of the 32kHz Real Time Clock until
// the differences between two consecutive measurements match.  Some RTCs may
// take time to settle, and we want to ensure that the the output is  stable
// before performing any operations dependent on the RTC.
WaitSettle::
     ;writing any value into RTC0R will load the RTC registers for reading
ioi   ld    (RTC0R), a     ;move RTC value into registers
ioi   ld    jkhl, (RTC0R)  ;JKHL = RTC
      ld    px, jkhl
      ld    jkhl, 0
      ld    py, jkhl

      ;run until RTC differences between two consecutive passes match
.settle_loop:
      ;the length of the delay loop is somewhat arbitrary
      ;it only needs to be long enough to produce sufficiently large RTC values
      ld    bc, 0x8000
.delay_loop:
      ld    a, 0x5A     ;hit watchdog
ioi   ld    (WDTCR), a
      dwjnz .delay_loop

ioi   ld    (RTC0R), a     ;move RTC value into registers
ioi   ld    jkhl, (RTC0R)  ;JKHL = new_RTC
      ld    pz, jkhl       ;PZ = new_RTC
      ld    bcde, px       ;BCDE = prev_RTC
      sub   jkhl, bcde     ;JKHL = new_RTC - prev_RTC = new_diff
      srl   4, jkhl        ;divide by 16 to rule out minor differences
      ld    bcde, py       ;BCDE = prev_diff
      ld    py, jkhl       ;prev_diff = new_diff
      ld    px, pz         ;old_RTC = new_RTC
      cp    jkhl, bcde     ;compare JKHL, BCDE
      jr    nz, .settle_loop  ;if not equal, keep waiting

      lret  //end WaitSettle


//Initialize target communications (between debug kernel and dynamic C)
dkInitTC::
      ; the low-level read driver
      ld    hl,DevMateReadPort
      ld    (TCState+[TCState]+ReadPort),hl
      ; the low-level write driver
      ld    hl,DevMateWritePort
      ld    (TCState+[TCState]+WritePort),hl
      ; low-level driver to clear spurious ints
      ld    hl,DevMateClearReadInt
      ld    (TCState+[TCState]+ClearReadInt),hl
      ; the list of receive buffers, sorted by 'type'
      ld    hl,dkcRXBufferListStore
      ld    (TCState+[TCState]+RXBufferList),hl
      ; the list of callbacks, sorted by 'type'
      ld    hl,dkcCallbackListStore
      ld    (TCState+[TCState]+CallbackList),hl
      ; special-case hander for sys-writes
      ld    hl,dkcSystemWriteHeader
      ld    (TCState+[TCState]+SysWriteHandler),hl

      xor   a
      ld    (dkcstartuserprog),a
      ; flag that sys-writes should be handled as a special case
      ld    a,1
      ld    (TCState+[TCState]+TrapSysWrites),a

      ld    iy,TCState
      lcall dkcSystemBufINIT      ;  initialize the system buffers
      lcall dkcInit               ;   initialize comm module
      call  dkcSystemINIT         ;  initialize the system-type handler

      ; initialize error exit for exception handler
      ld    hl, _xexit
      ld    (DCParam+errorExit), hl
      ; initialize debug kernel - target communication interface
      call  dkInitDebugKernelComs
      lret
#endasm //xmem

//*** End Xmem BIOS code section ***
///////////////////////////////////////////////////////////////////////

#asm __root
//Debug kernel's function for handling serial A TX/RX events
dkcDoSerial::
      ld    iy,TCState         ; iy == pointer to our state structure
ioi   ld    a,(SASR)         ;   check the status
      bit   SS_RRDY_BIT,a         ;  was a character received?
      jr    z,_DevMatenoRxIntReq

      call  dkcEntryRX         ;  handle the RX interrupt
      jr    _DevMatereadyToExit      ;  all done for now
_DevMatenoRxIntReq:
      bit   3,a
      jr    z,_DevMateSecondTXInt
      call  dkcEntryTX         ;   handle the TX interrupt
      jr    _DevMatereadyToExit
_DevMateSecondTXInt:
      ld    a,(TCState+[TCState]+TXBusy)
      or    a
      jr    z,_DevMateSkipInt

      call  dkcEntryTX
      jr    _DevMatereadyToExit
_DevMateSkipInt:
      ; just clear the int
ioi   ld    (SASR), a
_DevMatereadyToExit:
      ret

//Serial A ISR for the debug kernel
//To be safe, every register is protected
DevMateSerialISR::
      push  ip
      push  af
      ex    af,af'
      push  af
      push  pw
      push  px
      push  py
      push  pz
      push  jkhl
      ld    hl,lxpc
      push  hl
      push  bcde
      push  ix
      push  iy
      exx
      exp
      push  bcde
      push  pw
      push  px
      push  py
      push  pz
      push  jkhl

      call  dkcDoSerial

      pop   jkhl
      pop   pz
      pop   py
      pop   px
      pop   pw
      pop   bcde
      exx
      exp
      pop   iy
      pop   ix
      pop   bcde
      pop   hl
      ld    lxpc,hl
      pop   jkhl
      pop   pz
      pop   py
      pop   px
      pop   pw
      pop   af
      ex    af,af'
      pop   af
      pop   ip
      ipres

      ret

DevMateReadPort::
      ; trashes A
      ; returns byte read (if any) in A
      ; returns with Z set if nothing is read

      ; check if there is anything available
ioi   ld    a, (SASR)
      bit   SS_RRDY_BIT,a      ;   if a received byte ready?
      ret   z                  ;   nope, return with z set
      ; otherwise, a byte *is* ready, read from data port
ioi   ld    a, (SADR)
      ret                     ;   return with z *not* set

DevMateClearReadInt::
      ld    a,SS_RRDY_BIT
ioi   ld    (SASR),a
      ret

DevMateWritePort::   ;   assumes byte to transmit is in C
      ; trashes A
      ; returns with Z reset if not transmitted

      ; check if the port is ready
ioi   ld    a, (SASR)
      bit   SS_TFULL_BIT,a      ;   can I transmit now?
      ret   nz                  ;   nope, return with nz set
      ; otherwise, the transmit buffer is ready, write to it!
      ld    a,c               ;   move byte to transmit to a
ioi   ld    (SADR), a
      ret                     ;   return with z *not* set
#endasm //root

#asm const
dkDCID::
      db "DynamicC "
dkProdName::
      db PRODUCT_NAME
#endasm

#define ERROR_EXIT DCParam+errorExit

#asm
//Function to hit watchdog timer
bioshitwd::
      push  af
      ld    a, 0x5a
ioi   ld    (WDTCR),a
      pop   af
      ret

_xexit::
      ld    ix,(_xexithook)
      jp    (ix)
#endasm

#asm __xmem
//Set up the interrupt vector table
dkSetIntVecTabP::
      ld    a,0xff & (INTVEC_BASE >> 8)
      ld    iir,a
      ld    a,0xff & (XINTVEC_BASE >> 8)
      ld    eir,a
      lret

//Disable the RST instructions
dkSetIntVecTabR::
      ld    a, _OP_RET
      ld    (INTVEC_BASE+RST18_OFS),a      ;   all are relays
      ld    (INTVEC_BASE+RST20_OFS),a
      ld    (INTVEC_BASE+RST28_OFS),a

      ld    a,0x80
ioi   ld    (BDCR),a      ; make RST 28Hs NOPs
      ld    (BDCRShadow),a
      lret



//**************************************************************
//Perform flash initialization/read ID block.
_more_inits0::
      ;; mark program flash initialization not yet done (this is a safety
      ;;  feature in case future BIOS changes reintroduce a path where
      ;;  _readIDBlock gets called before _InitFlashDriver is called)
      clr   hl
      ld    (_InitFlashDriverOK), hl

#if !RAMONLYBIOS
   #if _SERIAL_BOOT_FLASH_
      call  _sbf_bios_initSF
      ld    jkhl, 0x08
      call  _InitFlashDriver
      ld    jkhl, 0x03
      call   _readIDBlock         //  HL <- _readIDBlock(HL:quad_bitmask)
   #else
      ; Default value: Primary flash is mapped to MB0 in FLASH_COMPILE mode.
      ld    bcde, 0
      ld    jkhl, 0
      #if FAST_RAM_COMPILE || RAM_COMPILE
      ; Flash is mapped to MB3 in both FAST_RAM_COMPILE and RAM_COMPILE modes.
      ld    bc, _cexpr((1 << (MSB_BIT-15)) - (1 << (MSB_BIT-17)))
      ld    jk, _cexpr((1 << (MSB_BIT-15)) - (FLASH_SIZE >> 4))
      #endif
      ;BCDE = physical start of MB3
      ;JKHL = physical start of flash when it is mapped to MB3
      push  jkhl
      push  bcde
      call  _InitFlashDriver
      add   sp, 8
      #if FAST_RAM_COMPILE || RAM_COMPILE
      ; Flash is mapped to MB3 in both FAST_RAM_COMPILE and RAM_COMPILE modes.
      ld    jkhl, 0x08
      #else	// FLASH_COMPILE
      ; Flash is mapped into MB0 and (possibly ghosted in) MB1 in FLASH_COMPILE
      ;  mode. When the primary flash size is less than or equal to the quadrant
      ;  size, the system ID block is in flash in the MB0 quadrant. When the
      ;  flash size is greater than the quadrant size the system ID block is in
      ;  the MB1 quadrant.
         #if ((0ul + _FLASH_SIZE_) << 12) > _QUADRANT_SIZE
      ld    jkhl, 0x03     ; the system ID block is in the MB1 quadrant
         #else
      ld    jkhl, 0x01     ; the system ID block is in the MB0 quadrant
         #endif
      #endif
      call  _readIDBlock   ; L contains a bit mask of memory banks to check
                           ; returns 0 in HL if ID block is ok
   #endif
      test  hl
      jr    z, .idBlockOk

#endif	// !RAMONLYBIOS

      ;; erase SysIDBlock if error returned or if RAM-only BIOS
      ld    hl, SysIDBlock
      ld    b, _cexpr(sizeof SysIDBlock)
      xor   a
.blockEraseLoop:
      ld    (hl), a
      inc   hl
      djnz  .blockEraseLoop

.idBlockOk:
      lret  //end _more_inits0


//**************************************************************
// Initialize the status pin, clocks, clock doubler, debug baud rate
_more_inits02::
      lcall _getDoublerSetting   ; get doubler setting value into L (h always 0)
      ld    a, L                 ; zero value also disables /OEx early output
      or    a                    ; update the Zero flag
      jr    z, .notEarlyOutputEnable

      ld    a, 0x0C              ; value to set both /OE0 and /OE1 early output
.notEarlyOutputEnable:
ioi   ld    (MTCR), a            ; first, update /OE0, /OE1 early output enable
      ld    (MTCRShadow), a
      ld    a, L                 ; recover the doubler setting value
ioi   ld    (GCDR), a            ; next, update clock doubler setting
      ld    (GCDRShadow), a

#if CPU_ID_MASK(_CPU_ID_) >= R6000
      ld    (_pll_GCDR_initial), a
#endif
      lcall _enableClockModulation  ; last, update spreader setting

      ; make timers tick
      ld    a, 0x01
ioi   ld    (TACSR),a
      ld    (TACSRShadow),a
      ; make timer A4 clocked by main clock, disable timer A interrupt
      xor   a
ioi   ld    (TACR),a
      ld    (TACRShadow),a

      ; Read the timer count produced by the timing measurement earlier
      ld    hl, (bios_timer_count)
#ifdef USE_TIMERA_PRESCALE
      xor   a              ;Timer A uses peripheral clock
      add   hl, hl         ;double timer count
#else
      ld    a, 0x01        ;Timer A uses peripheral clock/2
#endif
ioi   ld    (TAPR), a      ; Timer A Prescale Register
      ld    (TAPRShadow), a
      ; is clock doubled?
      ld    a, (GCDRShadow)  ; get clock doubler register
      or    a
      jr    z, .saveFreq     ; if zero, clock not doubled
      add   hl, hl           ; if clock doubled, double timer count
.saveFreq:
      ; calculate the adjusted divider value for 19200 baud
      ; the rounded value is calculated by (count * 3 + 128) / 256
      ; while there may be some error with certain oscillator speeds,
      ; it is unlikely to be significant at low baud rates
      ld    bc, hl   ;save HL's value (adjusted count) for later
      ld    de, hl
      add   hl, de
      add   hl, de

		; A = (HL + 128) / 256
		xor	a
		rl		l				; shift top bit of L into carry
      adc	a, h			; load A with H plus possibly carry from L (rounding)

      clr	hl				; promote A to 16-bit register HL
      ld    l, a

      ld    (freq_divider), hl   ; save for later use
#if CPU_ID_MASK(_CPU_ID_) >= R6000
      ld    (freq_dividerOrig), hl   ; save for later use by PLL
#endif
      ld    hl, bc   ;restore HL's value

;for lower baud, get to the divider we need by shifting A
#if (_BIOSBAUD_ == 2400)
      sla   a              ; multiply by eight
      sla   a
      sla   a
#elif (_BIOSBAUD_ == 4800)
      sla   a              ; multiply by four
      sla   a
#elif (_BIOSBAUD_ == 9600)
      sla   a              ; multiply by two
#elif (_BIOSBAUD_ == 19200)
      ;do nothing
#elif (_BIOSBAUD_ == 38400)
      srl   a              ; divide by two

;for 57600 or 115200 baud, calculate the timer divider directly
;the timer count is still in HL
#elif (_BIOSBAUD_ == 57600)
;for 57600 baud, the calculation is (count + 128)/256
      ld    bc, 128
      add   hl, bc
      ld    a, h
#elif (_BIOSBAUD_ == 115200)
;for 115200 baud, the calculation is (count + 256)/512
      ld    bc, 256
      add   hl, bc
      ld    a, h
      srl   a
#elif
	#fatal "Invalid _BIOSBAUD_ value, can't calculate TAT4R."
#endif
      dec   a                 ; put divider-1 into timer scaling register
ioi   ld    (TAT4R), a
      ld    (TAT4RShadow), a

#if CPU_ID_MASK(_CPU_ID_) >= R6000
      ld    (TAT4RShadowOrig), a
#endif

      lret  //end _more_inits02


//**************************************************************
//Set as many registers as possible to known values.  Some registers are
//already in use, such as serial A registers, and these should not be touched.
//Most registers have a default state after a hardware reset, but a BIOS
//restart could also occur via the debug kernel or some other soft reset.
//Certain boards require certain ports to be intialized differently; if
//necessary these exceptions are dealt with here.
_init_IO_regs::
      xor   a
ioi   ld    (PADR), a         ; Parallel Port A Data Register
      ld    (PADRShadow), a
ioi   ld    (PBDR), a         ; Parallel Port B Data Register
      ld    (PBDRShadow), a

#ifdef _ZW_RESET_PCDR_ALL_ZEROS
ioi   ld    (PCDR), a         ; set TxA,TxB,TxC,TxD bits all low
      ld    (PCDRShadow), a    ;  (restore previous BIOS behavior)
#endif

#if _SERIAL_BOOT_FLASH_ && CPU_ID_MASK(_CPU_ID_) >= R5000
      ; Serial-boot Rabbit 5000 and 6000 boards do not function correctly
      ; without special port D setup.
      ; PD6 and PD7 control the chip selects of the flash and ADC
      ; PD4 and PD5 are TxB and RxB
ioi   ld    a, (PDDDR)        ; Parallel port D
      and   a, 0xD0
ioi   ld    (PDDDR), a
      ld    (PDDDRShadow), a
      ld    a, 0xC0        ;set CS lines high
ioi   ld    (PDDR), a
      ld    (PDDRShadow), a
ioi   ld    a, (PDFR)
      ld    (PDFRShadow), a
      xor   a
#else //other boards
ioi   ld    (PDDDR),a         ; Parallel port D
      ld    (PDDDRShadow),a
ioi   ld    (PDDR), a
      ld    (PDDRShadow), a
ioi   ld    (PDFR), a
      ld    (PDFRShadow), a
#endif

ioi   ld    (PDCR), a         ; Parallel port D
      ld    (PDCRShadow), a
ioi   ld    (PDDCR), a
      ld    (PDDCRShadow), a

ioi   ld    (PEDDR),a         ; Parallel port E
      ld    (PEDDRShadow),a
ioi   ld    (PEDR), a
      ld    (PEDRShadow), a
ioi   ld    (PECR), a
      ld    (PECRShadow), a

; copy some registers to their shadows
ioi   ld    a, (PEFR)
      ld    (PEFRShadow), a
ioi   ld    a, (PCDCR)
      ld    (PCDCRShadow), a
ioi   ld    a, (PCDDR)
      ld    (PCDDRShadow), a
ioi   ld    a, (PEDCR)
      ld    (PEDCRShadow), a
ioi   ld    a, (PCALR)
      ld    (PCALRShadow), a
ioi   ld    a, (PCAHR)
      ld    (PCAHRShadow), a
ioi   ld    a, (PDALR)
      ld    (PDALRShadow), a
ioi   ld    a, (PDAHR)
      ld    (PDAHRShadow), a
ioi   ld    a, (PEALR)
      ld    (PEALRShadow), a
ioi   ld    a, (PEAHR)
      ld    (PEAHRShadow), a

;Rabbit 4000 processors do not have port H available
#if (CPU_ID_MASK(_CPU_ID_) >= R5000)
ioi   ld    a,(PHDR)
   #if RCM5400W_SERIES
      or    a, 0x80			;make sure link LED is off
ioi   ld    (PHDR), a
   #endif
      ld    (PHDRShadow),a
ioi   ld    a,(PHFR)
      ld    (PHFRShadow),a
ioi   ld    a,(PHDCR)
      ld    (PHDCRShadow),a
ioi   ld    a,(PHDDR)
      ld    (PHDDRShadow),a

      xor   a                 ; make sure network interface is off
ioi   ld    (ENPR), a

#else    //CPU < Rabbit 5000
   #if RCM4300_SERIES
      ld    a, 0x30
   #elif RCM4500W_SERIES
      ; if this is an XBee board, raise RTS to stop the Radio from
      ; trying to send us messages.
      ld    a, 0x04     ; turn on TX- (/RTS) to stop incoming transmissions.
   #else
      xor   a
   #endif
ioi   ld    (NAPCR), a
      ld    (NAPCRShadow), a

      xor   a
ioi   ld    (NACR), a
      ld    (NACRShadow), a
ioi   ld    (NARCR), a
      ld    (NARCRShadow), a
ioi   ld    (NATCR), a
      ld    (NATCRShadow), a
ioi   ld    (NACSR), a
      ld    (NACSRShadow), a
#endif

      xor   a
ioi   ld    (TCCSR), a
      ld    (TCCSRShadow), a
ioi   ld    (TCCR), a
      ld    (TCCRShadow), a
ioi   ld    (IHCR), a
      ld    (IHCRShadow), a
ioi   ld    (IHSR), a
      ld    (IHSRShadow), a

ioi   ld    (TBCSR), a        ; Timer B
      ld    (TBCSRShadow), a
ioi   ld    (TBCR), a
      ld    (TBCRShadow), a

ioi   ld    (IB0CR), a        ; External I/O Control Registers
      ld    (IB0CRShadow), a
ioi   ld    (IB1CR), a
      ld    (IB1CRShadow), a
ioi   ld    (IB2CR), a
      ld    (IB2CRShadow), a
ioi   ld    (IB3CR), a
      ld    (IB3CRShadow), a
ioi   ld    (IB4CR), a
      ld    (IB4CRShadow), a
ioi   ld    (IB5CR), a
      ld    (IB5CRShadow), a
ioi   ld    (IB6CR), a
      ld    (IB6CRShadow), a
ioi   ld    (IB7CR), a
      ld    (IB7CRShadow), a

ioi   ld    (I0CR), a         ; External Interrupt Control Registers
      ld    (I0CRShadow), a
ioi   ld    (I1CR), a
      ld    (I1CRShadow), a

ioi   ld    (TAT1R), a        ; Timer A Time Constant Register 1
      ld    (TAT1RShadow), a
ioi   ld    (SBCR), a
      ld    (SBCRShadow), a
ioi   ld    (SBER), a
      ld    (SBERShadow), a
ioi   ld    (SCCR), a
      ld    (SCCRShadow), a
ioi   ld    (SCER), a
      ld    (SCERShadow), a
ioi   ld    (SDCR), a
      ld    (SDCRShadow), a
ioi   ld    (SDER), a
      ld    (SDERShadow), a
ioi   ld    (SECR), a
      ld    (SECRShadow), a
ioi   ld    (SEER), a
      ld    (SEERShadow), a
ioi   ld    (SFCR), a
      ld    (SFCRShadow), a
ioi   ld    (SFER), a
      ld    (SFERShadow), a

      ; Ignore SMODE pin settings for determining bootstrap mode.
      ; Dynamic C will set this also, but in run mode it may be
      ; important to treat the SMODE pins as general input.
      ld    a, 0x80
ioi   ld    (SPCR), a
      ld    (SPCRShadow), a

      ; enable watchdog if not active already
      xor   a
ioi   ld    (WDTTR), a

      ld    a, 0x55
ioi   ld    (PCDDR), a        ; (Rabbit 4000 default after reset)
      ld    (PCDDRShadow), a

#ifndef _ZW_RESET_PCDR_ALL_ZEROS
      ld    a, 0x15             ; set TxA bit low but TxB,TxC,TxD bits high
ioi   ld    (PCDR), a
      ld    (PCDRShadow), a
#endif

      xor   a
ioi   ld    (GPSCR), a
      ld    (GPSCRShadow), a
ioi   ld    (BDCR), a
      ld    (BDCRShadow), a

ioi   ld    (PWL0R), a        ; Pulse Width Modulation Registers
      ld    (PWL0RShadow), a
ioi   ld    (PWM0R), a
      ld    (PWM0RShadow), a
ioi   ld    (PWL1R), a
      ld    (PWL1RShadow), a
ioi   ld    (PWM1R), a
      ld    (PWM1RShadow), a
ioi   ld    (PWL2R), a
      ld    (PWL2RShadow), a
ioi   ld    (PWM2R), a
      ld    (PWM2RShadow), a
ioi   ld    (PWL3R), a
      ld    (PWL3RShadow), a
ioi   ld    (PWM3R), a
      ld    (PWM3RShadow), a

ioi   ld    (ICCSR), a        ; Input Capture Registers
      ld    (ICCSRShadow), a
ioi   ld    (ICCR), a
      ld    (ICCRShadow), a
ioi   ld    (ICT1R), a
      ld    (ICT1RShadow), a
ioi   ld    (ICT2R), a
      ld    (ICT2RShadow), a
ioi   ld    (ICS1R), a
      ld    (ICS1RShadow), a
ioi   ld    (ICS2R), a
      ld    (ICS2RShadow), a

ioi   ld    (QDCSR), a        ; Quadrature Decoder Registers
      ld    (QDCSRShadow), a
ioi   ld    (QDCR), a
      ld    (QDCRShadow), a

      ld    a, 0xC0           ; Parallel Port B Data Direction Register
#if RCM4300_SERIES
      or    0x01
#endif
ioi   ld    (PBDDR), a
      ld    (PBDDRShadow), a

ioi   ld    a, (GOCR)     ; General output control register
      ld    (GOCRShadow), a

      ;Not all bits of the GCSR can be read directly.
      ;We must set the shadow register to the correct value by hand.
      ld    a, GCSR_VALUE     ; General status register
      ld    (GCSRShadow), a

      lret     //end _init_IO_regs


//**************************************************************
// prepare to jump into the debugger
_init_dkLoop::
      call  bioshitwd
      ipset 0
      lret

#endasm //xmem

#asm
dkEnd::
__xexit::
      jp _xexit
#endasm
#flushlib

#include <default.h>        // pull in BIOS C support libraries

//Do not insert code after this line.
#flushlib

#endif //ndef __STDBIOS_C
/*** EndHeader */
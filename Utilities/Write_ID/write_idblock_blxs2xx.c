/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	write_idblock_blxs2xx.c

	Example of using WriteIdBlock.lib and one of WriteIdBlock_RCM43xx.LIB,
	WriteIdBlock_RCM40xx.LIB, WriteIdBlock_RCM54xxW.LIB, or
	WriteIdBlock_RCM45xxW.LIB to write a system ID Block to an appropriate Rabbit
	4000+ based core module, for use as a BLxS2xx SBC topping.

	The core module must be one of the following types:
	   An RCM4310  paired with BLxS2xx SBC board == BL4S200.
	   An RCM4010  paired with BLxS2xx SBC board == BL4S210.
	   An RCM5400W paired with BLxS2xx SBC board == BL5S220.
	   An RCM4510W paired with BLxS2xx SBC board == BL4S230.

	Special note for the BL5S220 (RCM5400W core module):
	   Each RCM5400W core module's WiFi is individually calibrated. The WiFi
	   calibration information is stored in the system constants table, which
	   should never be erased. The size of the system constants table is
	   determined by the value of the _DC_CONST_SZ_ system macro, which should
	   always equal 128. This system ID block writing utility makes every effort
	   to preserve the content of the system constants area, even on boards which
	   no longer have a valid system ID block.

	To program a BLxS2xx core module which lacks an existing correct and valid
	system ID block, add the appropriate set of the following sets of macro=value
	lines to Dynamic C's Project Options' Defines tab. If Dynamic C stops the
	compile because of too many invalid macro redefinition warnings, increase the
	"Max Shown > Warnings:" count to 100 on the Project Options' Compiler tab.

	For a BL4S200 (RCM4310 core module) add:
	   _BOARD_TYPE_=RCM4310
	   _DC_CLK_DBL_=1
	   _DC_BRD_OPT0_=0x20
	   _DC_DFLASH0_=0x14011F24
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0x1F24
	   _DC_MD0_TYPE_=0x2
	   _DC_MD0_SIZE_=0x100
	   _DC_MD0_SECSIZE_=0x108
	   _DC_MD0_SECNUM_=0x1000
	   _DC_MD0_SPEED_=0x0
	   _DC_MD0_MBC_=0x0
	   _DC_MD2_=0x1
	   _DC_MD2_ID_=0x0
	   _DC_MD2_TYPE_=0x0
	   _DC_MD2_SIZE_=0x80
	   _DC_MD2_SECSIZE_=0x0
	   _DC_MD2_SECNUM_=0x0
	   _DC_MD2_SPEED_=0x37
	   _DC_MD2_MBC_=0x45

	For a BL4S210 (RCM4010 core module) add:
	   _BOARD_TYPE_=RCM4010
	   _DC_CLK_DBL_=1
	   _DC_BRD_OPT0_=0x3
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0x2780
	   _DC_MD0_TYPE_=0x5
	   _DC_MD0_SIZE_=0x80
	   _DC_MD0_SECSIZE_=0x1000
	   _DC_MD0_SECNUM_=0x80
	   _DC_MD0_SPEED_=0x37
	   _DC_MD0_MBC_=0x40
	   _DC_MD2_=0x1
	   _DC_MD2_ID_=0x0
	   _DC_MD2_TYPE_=0x0
	   _DC_MD2_SIZE_=0x80
	   _DC_MD2_SECSIZE_=0x0
	   _DC_MD2_SECNUM_=0x0
	   _DC_MD2_SPEED_=0x37
	   _DC_MD2_MBC_=0x45

	For a BL5S220 (RCM5400W core module) add:
	   _DC_CLK_DBL_=0x1
	   _DC_CONST_SZ_=0x80
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0xBFD7
	   _DC_MD0_TYPE_=0x1
	   _DC_MD0_SIZE_=0x80
	   _DC_MD0_SECSIZE_=0x1000
	   _DC_MD0_SECNUM_=0x80
	   _DC_MD0_SPEED_=0x46
	   _DC_MD0_MBC_=0x0
	   _DC_MD2_=0x1
	   _DC_MD2_ID_=0x0
	   _DC_MD2_TYPE_=0x0
	   _DC_MD2_SIZE_=0x80
	   _DC_MD2_SECSIZE_=0x0
	   _DC_MD2_SECNUM_=0x0
	   _DC_MD2_SPEED_=0xF
	   _DC_MD2_MBC_=0xC6
	   _DC_MD3_=0x1
	   _DC_MD3_ID_=0x0
	   _DC_MD3_TYPE_=0x0
	   _DC_MD3_SIZE_=0x80
	   _DC_MD3_SECSIZE_=0x0
	   _DC_MD3_SECNUM_=0x0
	   _DC_MD3_SPEED_=0x37
	   _DC_MD3_MBC_=0x5

	For a BL4S230 (RCM4510W core module) add:
	   _BOARD_TYPE_=RCM4510W
	   _DC_CLK_DBL_=0
	   _DC_BRD_OPT0_=0x20
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0xBFD7
	   _DC_MD0_TYPE_=0x1
	   _DC_MD0_SIZE_=0x80
	   _DC_MD0_SECSIZE_=0x1000
	   _DC_MD0_SECNUM_=0x80
	   _DC_MD0_SPEED_=0x46
	   _DC_MD0_MBC_=0x80
	   _DC_MD2_=0x1
	   _DC_MD2_ID_=0x0
	   _DC_MD2_TYPE_=0x0
	   _DC_MD2_SIZE_=0x80
	   _DC_MD2_SECSIZE_=0x0
	   _DC_MD2_SECNUM_=0x0
	   _DC_MD2_SPEED_=0x37
	   _DC_MD2_MBC_=0xC5

	Instruction for the BL4S200 (RCM4310 core module), BL4S210 (RCM4010 core
	module), or BL5S220 (RCM5400W core module) only:
	   If the board does not already have a system ID block with a unique MAC
	   address then edit the const char newMAC[6] array below and press the N key
	   when asked, "Retain the XX:XX:XX:XX:XX:XX MAC address (Y/N)?"

	Instruction for the BL4S230 (RCM4510W core module) only:
	   To add a _DC_XBEE_ID_ system macro uncomment the XBEE_ID_VALUE macro
	   definition line, below. To have the program set the _DC_XBEE_ID_ system
	   macro based on the installed XBee module, define the XBEE_ID_VALUE macro's
	   value to 0xFFFFFFFFul. To skip XBee version detection, either define the
	   XBEE_ID_VALUE macro's value to 0 (no XBee installed) or define the
	   XBEE_ID_VALUE macro to a known valid value constructed according to the
	   following information:
	      Upper 16 bits -- hardware version
	         0x1941 - XBee Series 2 (ember processor, ZNet 2.5 or ZB firmware)
	      Lower 16 bits -- firmware version
	         0x1141 - ZNet coordinator firmware
	         0x1341 - ZNet router/end device firmware
	         0x2121 - ZB coordinator firmware
	         0x2321 - ZB router firmware
	         0x2921 - ZB end device firmware
	   For example, if using a known XBee series 2 module with firmware version
	   0x2921, set xbee_id = 0x19412921ul. Note that this setting is only
	   effective when using Dynamic C version 10.44 or later.

	If rewriting the system ID block in order to change the size of the User
	block or the size of the system's persistent data area, the following
	information should be of particular interest.

	The MAX_USERBLOCK_SIZE macro value tells Dynamic C how much flash storage
	space is reserved for the Rabbit board's system ID and User blocks area, as
	described in the following paragraphs. On all parallel flash equipped and on
	some serial boot flash equipped Rabbit boards, this reserved area size is
	excluded from the xmem code memory org in order to prevent overwrite of the
	system ID and User blocks reserved area. On Rabbit boards equipped with small
	sector flash devices, the value of the MAX_USERBLOCK_SIZE macro should be at
	least ((WIB_SYS4KBLKS + WIB_USER4KBLKS * 2ul) * 4096ul). Large or nonuniform
	sector flash devices' sector layout may cause "gaps" between the system ID
	block and / or the User block images, requiring a further increase in the
	MAX_USERBLOCK_SIZE macro's value. In this latter case, a run time exception
	assertion failure may occur in the WriteIdBlock.lib library. To see more
	information about such an exception, define WIB_VERBOSE (see code below),
	recompile and rerun this utility program. Also see the flash device's data
	sheet to find its sector layout, which in turn determines the optimal
	MAX_USERBLOCK_SIZE macro value.

	All stock Rabbit 4000+ based boards have an 8 KByte (less 6 bytes for the
	validity marker) User block installed. A nonstandard User block size is
	possible within the restrictions imposed by the program (primary, parallel
	or serial) flash type. The User assumes all responsibility for any problems
	that may result from making such a change. In all cases the User block size,
	combined with the 6 byte validity marker, must be in the range [0,64] KBytes
	(i.e. in the range [0,16] 4K blocks). Uncomment and edit the WIB_USER4KBLKS
	macro definition below to set a nonstandard User block size.

	On all stock Rabbit 4000+ based boards the system ID block is combined with a
	persistent data area (16 KBytes in total) which contains the system macros
	table and, optionally, extra memory devices specification table, persistent
	constants, etc. The information in the persistent data area may be extremely
	difficult for a User to replace correctly, and is not intended for alteration
	by User code. A nonstandard persistent data area size is possible on some
	boards depending on the program (primary, parallel or serial) flash type. The
	User assumes all responsibility for any problems that may result from making
	such a change. In all cases the minimum persistent data area size, combined
	with the system ID block, is 4 KBytes (i.e. one 4K block). Uncomment and edit
	the WIB_SYS4KBLKS macro definition below to set a nonstandard persistent data
	area size.
*/

#if _BOARD_TYPE_ != RCM4510W
// edit the last three bytes of the newMAC array to set the unique MAC address
const char newMAC[6] = { 0x00, 0x90, 0xC2,
                         0x00, 0x00, 0x00 };
#else	// _BOARD_TYPE_ != RCM4510W
// if using Dynamic C version 10.44 or later, to add a _DC_XBEE_ID_ system macro
//  uncomment and possibly edit the following XBEE_ID_VALUE macro definition
//#define XBEE_ID_VALUE 0xFFFFFFFFul
#endif	// _BOARD_TYPE_ != RCM4510W

// uncomment and edit the WIB_USER4KBLKS macro definition value to set a
//  nonstandard User block size (4 KByte block units, default is 2)
//#define WIB_USER4KBLKS 2

// uncomment and edit the WIB_SYS4KBLKS macro definition value to set a
//  nonstandard persistent data area size (4 KByte block units, default is 4)
//#define WIB_SYS4KBLKS 4

// uncomment the following line to make WriteIdBlock.lib functions debuggable
//#define WIB_DEBUG
// uncomment the following line to enable WriteIdBlock.lib functions' verbosity
//#define WIB_VERBOSE

// no user-settable options below this point

// BLxS2xx series controllers identified by _DC_MB_TYPE_ == 0x0100
#if _BOARD_TYPE_ != RCM4310 && _BOARD_TYPE_ != RCM4010 && \
    _BOARD_TYPE_ != RCM5400W && _BOARD_TYPE_ != RCM4510W
	#fatal "The BLxS2xx series SBC boards will not work with this core module."
#endif

/*
	For the RCM5400W core only, this utility program absolutely enforces the
	value 128 for each of the _DC_CONST_SZ_ and WIB_CONST_SZ_MAX macros before
	#using WriteIDBlock.lib.
	However, a test fixture program could have need to write different size
	system constants to different boards and so may not always need to enforce
	any particular _DC_CONST_SZ_ or WIB_CONST_SZ_MAX macros values.
*/
#if _BOARD_TYPE_ == RCM5400W
 #ifndef _DC_CONST_SZ_
	#define _DC_CONST_SZ_ 128
 #else	// _DC_CONST_SZ_
  #if 128 != _DC_CONST_SZ_
	#undef _DC_CONST_SZ_
	#define _DC_CONST_SZ_ 128
	#warns "The _DC_CONST_SZ_ macro value has been redefined to be 128."
  #endif	// 128 != _DC_CONST_SZ_
 #endif	// _DC_CONST_SZ_
 #ifndef WIB_CONST_SZ_MAX
	#define WIB_CONST_SZ_MAX 128
 #else	// WIB_CONST_SZ_MAX
  #if 128 != WIB_CONST_SZ_MAX
	#undef WIB_CONST_SZ_MAX
	#define WIB_CONST_SZ_MAX 128
	#warns "The WIB_CONST_SZ_MAX macro value has been redefined to be 128."
  #endif	// 128 != WIB_CONST_SZ_MAX
 #endif	// WIB_CONST_SZ_MAX
#endif	// _BOARD_TYPE_ == RCM5400W

// the following macro definition is needed before WriteIdBlock.lib is used
#define ENABLE_SYSTEM_ID_BLOCK_OVERWRITE

#use "WriteIdBlock.lib"
#use "WriteIdBlock_RCM43xx.LIB"
#use "WriteIdBlock_RCM40xx.LIB"
#use "WriteIdBlock_RCM54xxW.LIB"
#use "WriteIdBlock_RCM45xxW.LIB"

#if 0x0A44 <= CC_VER
/*
	This XBee functionality only exists in Dynamic C versions 10.44 and later.
*/
// bring in idBlock-related function for reading XBee module HW and FW versions
#use "WriteIdBlock_XBee.lib"
#endif	// 0x0A44 <= CC_VER

debug
void main(void)
{
	int rc;
	long build_xalloc_size;
	wib_sysIdBlockData far *build;

	// allocate xmem space for the ID block data structure
	build_xalloc_size = sizeof (wib_sysIdBlockData);
	build = (wib_sysIdBlockData far *)_xalloc(&build_xalloc_size, 0, XALLOC_ANY);

	// initialize the ID block build data structure and fill in typical items
	wib_initWriteIdBlockLib(build, _FLASH_SIZE_);
	wib_addAutomaticValues(build);

#if RCM4310 == _BOARD_TYPE_
	// add RCM4310 specific, fixed information
	wib_addRCM4310Info(build);
#elif RCM4010 == _BOARD_TYPE_
	// add RCM4010 specific, fixed information
	wib_addRCM4010Info(build);
#elif RCM5400W == _BOARD_TYPE_
	// add RCM5400W specific, fixed information
	wib_addRCM5400WInfo(build);
	/*
		Information specific to coding a test fixture follows.

		For each RCM54xxW family board, the test fixture should provide individual
		WiFi calibration information. In test fixture code, this would be the
		place to copy the individual board's calibration information into the
		system ID block build data structure's system constants buffer.

		To enable this functionality in test fixture code, one would:
		   1) Uncomment the following two lines of code containing the
		      wib_setSysConsts() and wib_addMacro() calls.
		   2) Edit the second parameter to the wib_setSysConsts() function, here
		      named WiFiCalibInfo, to suit the name of the actual byte buffer
		      containing the board's individual WiFi calibration information.
		   3) Edit the third parameter to the wib_setSysConsts() function, here
		      named WiFiCalibInfo, to suit the actual size of the board's
		      individual WiFi calibration information. Note that the maximum
		      buffer size is equal to the value of the WIB_CONST_SZ_MAX macro,
		      which may be defined in Dynamic C's Project Options' Defines box.
	*/
//	wib_setSysConsts(build, WiFiCalibInfo, sizeof WiFiCalibInfo);
//	wib_addMacro(build, "CONST_SZ", wib_getSysConstsSz(build));
#elif RCM4510W == _BOARD_TYPE_
	// add RCM4510W specific, fixed information
	wib_addRCM4510WInfo(build);
#else	// RCM4310 == _BOARD_TYPE_
	#fatal "This sample does not support the current _BOARD_TYPE_ definition."
#endif	// RCM4310 == _BOARD_TYPE_

	/*
		Add the MB_TYPE base macro name to enable BLxS2xx usage. We do this here
		in the write system ID block sample program so as to neither:
		   1) clutter up the core-specific wib_add.*Info() functions with
		      conditional code; nor
		   2) create nearly identical redundant board-specific copies of the
		      wib_add.*Info() functions.
	*/
	wib_addMacro(build, "MB_TYPE", 0x0100);

#if RCM4510W != _BOARD_TYPE_
	/*
		The RCM4510W does not have a MAC address stored in its system ID block.
		(Instead, its unique network identifier is stored in the XBee module.)

		For other BLx2Sxx boards, copy either the old or (default) new MAC
		address.
	*/
	wib_askRetainMAC(build, newMAC);
#endif	// RCM4510W != _BOARD_TYPE_

	// build the ID block and associated items, then write them to the primary
	//  (program, parallel or serial) flash
	rc = wib_buildAndWrite(build);
	wib_termWriteIdBlockLib(build, 1);
	if (rc) {
		printf("\nError, wib_buildAndWrite() result is %d!\n", rc);
	} else {
		printf("\nFinished writing system ID block.\n");
	}

	// we can release our xmem space so long as it was the last one allocated
	xrelease((long) build, build_xalloc_size);
}
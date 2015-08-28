/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	write_idblock_rcm54xxw.c

	Example of using WriteIdBlock.lib and WriteIdBlock_RCM54xxW.LIB to write a
	system ID Block to an RCM54xxW board.

	Each RCM54xxW family board's WiFi is individually calibrated. The WiFi
	calibration information is stored in the system constants table, which should
	never be erased. The size of the system constants table is determined by the
	value of the _DC_CONST_SZ_ system macro, which should always equal 128. This
	system ID block writing utility makes every effort to preserve the content of
	the system constants area, even on boards which no longer have a valid system
	ID block.

	To program an RCM54xxW board which lacks an existing correct and valid system
	ID block, add the appropriate set of the following sets of macro=value lines
	to Dynamic C's Project Options' Defines tab. If Dynamic C stops the compile
	because of too many invalid macro redefinition warnings, increase the
	"Max Shown > Warnings:" count to 100 on the Project Options' Compiler tab.

	For an RCM5400W add:
	   _BOARD_TYPE_=RCM5400W
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

	For an RCM5450W add:
	   _BOARD_TYPE_=RCM5450W
	   _DC_CLK_DBL_=0x1
	   _DC_CONST_SZ_=0x80
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0x1DA
	   _DC_MD0_TYPE_=0x10
	   _DC_MD0_SIZE_=0x100
	   _DC_MD0_SECSIZE_=0x80
	   _DC_MD0_SECNUM_=0x13
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
	   _DC_MD4_=0x1
	   _DC_MD4_ID_=0x0
	   _DC_MD4_TYPE_=0x0
	   _DC_MD4_SIZE_=0x80
	   _DC_MD4_SECSIZE_=0x0
	   _DC_MD4_SECNUM_=0x0
	   _DC_MD4_SPEED_=0xF
	   _DC_MD4_MBC_=0xC2

	If an RCM5400W core is to be programmed as a BL5S220 SBC topping, uncomment
	the wib_addMacro() function call line in the main() function's RCM5400W
	conditional code section.

	If the board does not already have a system ID block with a unique MAC
	address then edit the const char newMAC[6] array below and press the N key
	when asked, "Retain the XX:XX:XX:XX:XX:XX MAC address (Y/N)?"

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

// edit the last three bytes of the newMAC array to set the unique MAC address
const char newMAC[6] = { 0x00, 0x90, 0xC2,
                         0x00, 0x00, 0x00 };

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

/*
	This utility program absolutely enforces the value 128 for each of the
	_DC_CONST_SZ_ and WIB_CONST_SZ_MAX macros before #using WriteIDBlock.lib.
	However, a test fixture program could have need to write different size
	system constants to different boards and so may not always need to enforce
	any particular _DC_CONST_SZ_ or WIB_CONST_SZ_MAX macros values.
*/
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

// the following macro definition is needed before WriteIdBlock.lib is used
#define ENABLE_SYSTEM_ID_BLOCK_OVERWRITE

#use "WriteIdBlock.lib"
#use "WriteIdBlock_RCM54xxW.LIB"

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

#if RCM5400W == _BOARD_TYPE_
	// add RCM5400W specific, fixed information
	wib_addRCM5400WInfo(build);
	// if programming an RCM5400W core as a BL5S220 SBC topping,
	//  uncomment the following line to add the necessary system macro
//	wib_addMacro(build, "MB_TYPE", 0x0100);
#elif RCM5450W == _BOARD_TYPE_
	// add RCM5450W specific, fixed information
	wib_addRCM5450WInfo(build);
#else	// RCM5400W == _BOARD_TYPE_
	#fatal "This sample does not support the current _BOARD_TYPE_ definition."
#endif	// RCM5400W == _BOARD_TYPE_

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

	// copy either the old or (default) new MAC address
	wib_askRetainMAC(build, newMAC);

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
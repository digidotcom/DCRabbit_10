/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	write_idblock_rcm40xx.c

	Example of using WriteIdBlock.lib and WriteIdBlock_RCM40xx.LIB to write a
	system ID Block to an RCM40xx board.

	To program an RCM40xx board which lacks an existing correct and valid system
	ID block, add the appropriate set of the following sets of macro=value lines
	to Dynamic C's Project Options' Defines tab. If Dynamic C stops the compile
	because of too many invalid macro redefinition warnings, increase the
	"Max Shown > Warnings:" count to 100 on the Project Options' Compiler tab.

	For a current RCM4000 add:
	   _BOARD_TYPE_=RCM4000A
	   _DC_CLK_DBL_=1
	   _DC_BRD_OPT0_=0x17
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

	For an RCM4010 add:
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

	For an RCM4050 add:
	   _BOARD_TYPE_=RCM4050
	   _DC_CLK_DBL_=1
	   _DC_BRD_OPT0_=0x13
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0x2781
	   _DC_MD0_TYPE_=0x5
	   _DC_MD0_SIZE_=0x100
	   _DC_MD0_SECSIZE_=0x1000
	   _DC_MD0_SECNUM_=0x100
	   _DC_MD0_SPEED_=0x37
	   _DC_MD0_MBC_=0x40
	   _DC_MD2_=0x1
	   _DC_MD2_ID_=0x0
	   _DC_MD2_TYPE_=0x0
	   _DC_MD2_SIZE_=0x100
	   _DC_MD2_SECSIZE_=0x0
	   _DC_MD2_SECNUM_=0x0
	   _DC_MD2_SPEED_=0x37
	   _DC_MD2_MBC_=0x45

	For an obsolete RCM4000 add:
	   _BOARD_TYPE_=RCM4000
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

	If an RCM4010 core is to be programmed as a BL4S210 SBC topping, uncomment
	the wib_addMacro() function call line in the main() function's RCM4010
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

// the following macro definition is needed before WriteIdBlock.lib is used
#define ENABLE_SYSTEM_ID_BLOCK_OVERWRITE

#use "WriteIdBlock.lib"
#use "WriteIdBlock_RCM40xx.LIB"

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

#if RCM4000A == _BOARD_TYPE_
	// add current RCM4000 specific, fixed information
	wib_addRCM4000Info(build);
#elif RCM4010 == _BOARD_TYPE_
	// add RCM4010 specific, fixed information
	wib_addRCM4010Info(build);
	// if programming an RCM4010 core as a BL4S210 SBC topping,
	//  uncomment the following line to add the necessary system macro
//	wib_addMacro(build, "MB_TYPE", 0x0100);
#elif RCM4050 == _BOARD_TYPE_
	// add RCM4050 specific, fixed information
	wib_addRCM4050Info(build);
#elif RCM4000 == _BOARD_TYPE_
	// add obsolete original RCM4000 specific, fixed information
	wib_obsoleteRCM4000Info(build);
#else	// RCM4000 == _BOARD_TYPE_
	#fatal "This sample does not support the current _BOARD_TYPE_ definition."
#endif	// RCM4000 == _BOARD_TYPE_

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
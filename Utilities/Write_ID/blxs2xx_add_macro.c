/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	blxs2xx_add_macro.c

	Adds the MB_TYPE base macro name to the system macro table of an appropriate
	Rabbit 4000+ based core module, for use as a BLxS2xx SBC topping.

	The core module must be one of the following types:
	   An RCM4310  paired with BLxS2xx SBC board == BL4S200.
	   An RCM4010  paired with BLxS2xx SBC board == BL4S210.
	   An RCM5400W paired with BLxS2xx SBC board == BL5S220.
	   An RCM4510W paired with BLxS2xx SBC board == BL4S230.

	This program preserves persistent data areas associated with the ID block,
	including any such WiFi calibration constants. Similarly, valid User block
	content is also preserved but content of an invalid User block may be lost.
*/

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

#ifdef _DC_MB_TYPE_
 #if ((_DC_MB_TYPE_ & 0xFF00) == 0x0100)
	#fatal "This core already has the_DC_MB_TYPE_ BLxS2xx system macro set."
 #elif (_DC_MB_TYPE_ != 0)
	#fatal "This core has the _DC_MB_TYPE_ macro set for some other board type."
 #endif
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

debug
void main(void)
{
	int error;
	long build_xalloc_size;
	wib_sysIdBlockData far *build;

	// allocate xmem space for the ID block data structure
	build_xalloc_size = sizeof (wib_sysIdBlockData);
	build = (wib_sysIdBlockData far *)_xalloc(&build_xalloc_size, 0, XALLOC_ANY);

	// copy existing system ID and persistent data into the build structure
	error = wib_copyExistingIdBlock(build);
	if (error) {
		printf ("Error trying to copy existing ID block --- exiting.\n");
		exit(1);
	}

	// inform the User of what system macros currently exist
	printf ("System macros table before change:\n");
	wib_dumpMacroTable(build);

	// add the MB_TYPE base macro name to enable BLxS2xx usage
	wib_addMacro(build, "MB_TYPE", 0x0100);

	// inform the User of what system macros will come to exist
	printf ("\nSystem macros table after change:\n");
	wib_dumpMacroTable(build);

	// build the ID block and associated items, then write them to the primary
	//  (program, parallel or serial) flash
	error = wib_buildAndWrite(build);
	wib_termWriteIdBlockLib(build, 1);
	if (error) {
		printf("\nError, wib_buildAndWrite() result is %d!\n", error);
	} else {
		printf("\nFinished writing system ID block.\n");
	}

	// we can release our xmem space so long as it was the last one allocated
	xrelease((long) build, build_xalloc_size);
}
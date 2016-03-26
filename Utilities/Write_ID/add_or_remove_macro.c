/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	add_or_remove_macro.c

	Example of using WriteIdBlock.lib functions to update (add, change and / or
	remove) macros from a Rabbit 4000+ CPU based board's system macros table.

	CAUTION!
	Never remove or alter the value of the CONST_SZ (i.e. _DC_CONST_SZ_) system
	macro on a Rabbit 5000+ based WiFi equipped board (e.g. RCM54xxW, RCM56xxW).
	Removal or alteration of the CONST_SZ (i.e. _DC_CONST_SZ_) macro on these
	boards will damage the WiFi calibration information, resulting in seriously
	impaired WiFi performance! Once damaged, the WiFi calibration information is
	*NOT* easily recoverable by an end-User. In this situation, please contact
	Rabbit Technical Support for assistance in restoring WiFi performance.

	This program requires that a correct and valid version 5 system ID block
	exists on the Rabbit board.

	Comment out or uncomment any of the provided wib_AddMacro() or
	wib_removeMacro() calls in the main() function, or edit / add custom
	wib_addMacro() and / or wib_removeMacro() function calls to suit.

	Note that the maximum base macro name length allowed is MACRO_NAME_SIZE - 1
	(currently 8) characters, excluding the zero-byte terminator.

	Also note that Dynamic C automatically adds a "_DC_" prefix and a "_" suffix
	to the base macro name that is stored in the system macros table. For
	example, "CLK_DBL" becomes "_DC_CLK_DBL_" for use in library or application
	code.
*/

// uncomment the following line to make WriteIdBlock.lib functions debuggable
//#define WIB_DEBUG
// uncomment the following line to enable WriteIdBlock.lib functions' verbosity
//#define WIB_VERBOSE

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
	printf ("System macros table before change(s):\n");
	wib_dumpMacroTable(build);

	// optional: uncomment zero or more base system macro names to add or update
//	wib_addMacro(build, "CLK_DBL", 0);
//	wib_addMacro(build, "BRD_OPT0", 0x20);
//	wib_addMacro(build, "MB_TYPE", 0x0100);
//	wib_addMacro(build, "XBEE_ID", 0);

	// optional: uncomment zero or more base system macro names to remove
//	wib_removeMacro(build, "BRD_OPT0");
//	wib_removeMacro(build, "MB_TYPE");
//	wib_removeMacro(build, "XBEE_ID");

	// inform the User of what system macros will come to exist
	printf ("\nSystem macros table after change(s):\n");
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
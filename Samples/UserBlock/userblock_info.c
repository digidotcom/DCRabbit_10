/***********************************************************************
	userblock_info.c
	Copyright 2006 by Rabbit Semiconductor, Inc.

	This program reports on the size and capabilities of the ID/User
	Blocks.  In particular, it will report the version of the ID Block,
	the size of the ID and User Blocks, the size of the User Block's
	reserved for calibration constants etc. section, whether or not
	the ID/User blocks are mirrored, and the total amount of flash used
	by the ID/User Blocks.

	See userblock_clear.c for clearing the contents of the User Block
	(excepting the calibration constants reserved area and the ID Block).
***********************************************************************/

#class auto

#use "idblock_api.lib"

// If it exists, use the Z-World reserved size that is explicitly set
//  for this controller in its hardware-specific library.
#ifndef ZWORLD_RESERVED_SIZE
// Otherwise, reserve 2K bytes of space below top of ID/User Blocks.
// This may be overprotective for boards with no calibration constants
//  or other Z-World installed information, but, better safe than sorry!
#define ZWORLD_RESERVED_SIZE 0x0800
// Warn the User that ZWORLD_RESERVED_SIZE has been arbitrarily defined.
#warnt "ZWORLD_RESERVED_SIZE not previously defined (arbitrarily defined here)."
#endif

void main(void)
{
	char *userblock_type;
	int is_combined;
	unsigned long available_size;
	unsigned long calib_size;
	unsigned long flash_required;
	unsigned long sysid_size;
	unsigned long total_size;

	// this is just here to display the appropriate warning in RAM compile mode
	readUserBlock;

	// this is true for all boards, even if it is only an assumed size
	calib_size = (unsigned long) ZWORLD_RESERVED_SIZE;

	// this is also true for all boards and ID/User block versions
	total_size = GetIDBlockSize() * 4096ul;

	printf("System ID Block version:  %d\n\n", SysIDBlock.tableVersion);

	if (SysIDBlock.tableVersion >= 3) {
		printf("Contains mirrored ID/User blocks, which ensures that there is\n");
		printf("always a correct version of User block information written.\n\n");
	}
	else {
		printf("Does not contain mirrored ID/User blocks.  A power cycle\n");
		printf("while the User block is being written can leave the User\n");
		printf("block in an inconsistent state.\n\n");
	}

	if (1 > SysIDBlock.tableVersion) {
		printf("No valid ID or User blocks actually exist on this board.\n");
		printf("However, the top-most 8 KB of the first flash is reserved for\n");
		printf("an ID block, and the next 8 KB of flash below that may be\n");
		printf("used as a simulated User block, provided the application's\n");
		printf("xmem code does not extend up into the top 16 KB of the first\n");
		printf("flash.\n\n");
		userblock_type = "Separate, simulated ID and User blocks may";
		is_combined = 0;
		sysid_size = 0ul;
		flash_required = 0x4000ul;
		available_size = total_size - calib_size;
	}
	else if (1 == SysIDBlock.tableVersion) {
		printf("A valid ID block exists, but no User block actually exists on\n");
		printf("this board.  The top-most 8 KB of the first flash is reserved\n");
		printf("for the ID block, and the next 8 KB of flash below that may\n");
		printf("be used as a simulated User block, provided the application's\n");
		printf("xmem code does not extend up into the top 16 KB of the first\n");
		printf("flash.\n\n");
		userblock_type = "Separate ID block and simulated User block may";
		is_combined = 0;
   	sysid_size = SysIDBlock.idBlockSize;
		flash_required = 0x4000ul;
		available_size = total_size - calib_size;
	}
	else if (((2 <= SysIDBlock.tableVersion) && (4 >= SysIDBlock.tableVersion))
	         || ((5 <= SysIDBlock.tableVersion) && SysIDBlock.userBlockSize))
	{
		userblock_type = "Combined ID and User blocks";
		is_combined = 1;
		sysid_size = SysIDBlock.idBlockSize;
		if (5 == SysIDBlock.tableVersion) {
			sysid_size += SysIDBlock.idBlock2.persBlockLoc;
		}
		flash_required = SysIDBlock.idBlockSize + IDBlockAddr - UserBlockAddr;
		if (total_size < sysid_size) {
			available_size = calib_size = total_size = 0;
		}
		else {
			if (calib_size > sysid_size) {
				available_size = total_size - calib_size;
				calib_size -= sysid_size;
			}
			else {
				available_size = total_size - sysid_size;
				calib_size = 0;
			}
			total_size -= sysid_size;
		}
	}
	else if ((5 <= SysIDBlock.tableVersion) && !SysIDBlock.userBlockSize) {
		userblock_type = "Separate ID and User blocks";
		is_combined = 0;
		sysid_size = SysIDBlock.idBlockSize + SysIDBlock.idBlock2.persBlockLoc;
		flash_required = SysIDBlock.idBlockSize + IDBlockAddr - UserBlockAddr;
		if (total_size < sizeof(SysIDBlock.marker)) {
			available_size = calib_size = total_size = 0;
		}
		else {
			if (calib_size > total_size) {
				available_size = 0;
				calib_size = total_size - sizeof(SysIDBlock.marker);
			}
			else if (calib_size > sizeof(SysIDBlock.marker)) {
				available_size = total_size - calib_size;
				calib_size -= sizeof(SysIDBlock.marker);
			}
			else {
				available_size = total_size - sizeof(SysIDBlock.marker);
				calib_size = 0;
			}
			total_size -= sizeof(SysIDBlock.marker);
		}
	}


	printf("%s consume 0x%08LX (%ld) bytes\n",
	       userblock_type, flash_required, flash_required);
	printf("at the top of the first flash.\n\n");
	if (is_combined) {
		printf("Combined ID plus User blocks size:  0x%08LX (%ld) bytes.\n\n",
		       sysid_size + total_size, sysid_size + total_size);
	}
	printf("System ID block / area size:        0x%08LX (%ld) bytes.\n",
	       sysid_size, sysid_size);
	printf("User block size:                    0x%08LX (%ld) bytes.\n",
	       total_size, total_size);
	printf("Available User block size:          0x%08LX (%ld) bytes.\n",
	       available_size, available_size);
	if ((unsigned long) (ZWORLD_RESERVED_SIZE) != calib_size) {
		printf("Reserved (EG calibration) size:     0x%08LX (%ld) bytes.\n",
		       calib_size, calib_size);
	}
	printf("Reserved (default, total) size:     0x%08LX (%ld) bytes.\n\n",
	       (unsigned long) (ZWORLD_RESERVED_SIZE),
	       (unsigned long) (ZWORLD_RESERVED_SIZE));
	if (available_size) {
		printf("Available User block offsets:       "\
		       "0x%08lX to 0x%08lX (%lu to %lu).\n",
		       0ul, available_size - 1, 0ul, available_size - 1);
	}
	if ((unsigned long) (ZWORLD_RESERVED_SIZE) != calib_size) {
	printf("Reserved (EG calibration) offsets:  "\
	       "0x%08lX to 0x%08lX (%lu to %lu).\n",
	       available_size, available_size + calib_size - 1,
	       available_size, available_size + calib_size - 1);
	}
#if ZWORLD_RESERVED_SIZE > 0
	printf("Reserved (default, total) offsets:  "\
	       "0x%08lX to 0x%08lX (%lu to %lu).\n",
	       available_size, available_size + ZWORLD_RESERVED_SIZE - 1,
	       available_size, available_size + ZWORLD_RESERVED_SIZE - 1);
#endif
}
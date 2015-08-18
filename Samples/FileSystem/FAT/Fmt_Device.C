/*
   Copyright (c) 2015, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/***********************************************************************
	FMT_DEVICE.C

   Formats the configured device(s) for use with the FAT file system.

   Requires that you run this on a board with a compatible
   storage medium (serial flash, NAND flash or SD card).

   IMPORTANT!!!!  Please use with caution.

   Allows formatting of existing FAT partitions (previously formatted
   or unformatted).  This will only affect the selected partition.

   Formatting a device will unconditionally ERASE ALL DATA on any
   FAT 12/16 partitions during formatting.  It will not affect non-FAT
   partitions or FAT 32 partitions except as noted below.

   If format device is selected and NO FAT 12/16 partitions are found, a
   prompt will appear allowing erasure of all existing partitions and
   then the creation of one large FAT partition using the entire device.
   This option WILL DESTROY FAT 32 PARTITIONS AND ALL DATA STORED THERE!

   This sample does not allow the creation of multiple partitions that
   do not already exist on the device.  To create multiple partitions,
   see the fat_write_mbr.c sample program.

   This sample deals with low level formatting of FAT devices and uses
   several 'internal' data structures which are not documented in the
   FAT user's manual.  This is especially true of the erase_MBR function,
   which destroys existing device formating.  This is NOT a function
   that should typically be included in an application, and is only
   included here to make initial formatting of preformatted commercial
   media easier for the Rabbit user.

************************************************************************/

// Map program to xmem if not compiling to separate I&D space.
#if !__SEPARATE_INST_DATA__
#memmap xmem
#endif

// Set FAT library to blocking mode
#define FAT_BLOCK

//#define FAT_DEBUG
//#define FATFTC_DEBUG
//#define FTL_DEBUG

// Error numbers library
#include <errno.h>

// Makes the device drivers and FAT filesystem libraries available
#use "fat16.lib"

// format one partition or all partitions on specified device
int format(int dev, int part)
{
	int i, j, k, end, rc;
	word flags_to_use;
   char s[9], flash_buf[512];

   if (dev >= num_fat_devices)
   {
      printf("\nDevice does not exist.\n");
      return -ENODEV;
   }

   if (part < 0) {
      printf("This will destroy ALL DATA ON ALL PARTITIONS on this device!");
   }
   else {
      part += dev * FAT_MAX_PARTITIONS;
      printf("This will destroy ALL DATA on Partition %c!", 'A' + part);
   }
   printf("\nAre you sure you want to continue? (y/n) ");
   gets(s);
   printf("\n\n");
   if (toupper(s[0]) != 'Y') return 0;

   // check for option, as partition letter or device number
	if (part >= 0) {
      i = end = part;
   }
   else {
      i = 0;
      end = 3;
   }
   j = 0;

   while (i <= end) {
      if (fat_part_mounted[i]) {
		   // Unmount the selected mounted partition
		   printf("Unmounting Partition %c . . . \n",
                                   'A' + i + (dev * FAT_MAX_PARTITIONS));
		   rc = fat_UnmountPartition(fat_part_mounted[i]);
		   if (rc) {
			   printf("fat_UnmountPartition Error: (%d) %ls\n\n", rc,
                             error_message(rc));
            break;
		   }
      }
      else {
        // Find unmounted FAT partitions (if any)
        rc = fat_EnumPartition(&(_fat_device_table[dev]), i,
        						    &(_fat_part_table[i + (dev * FAT_MAX_PARTITIONS)]));
        if (!rc || rc == -EUNFORMAT) {
          fat_part_mounted[i] = &(_fat_part_table[i+(dev*FAT_MAX_PARTITIONS)]);
        }
        else {
          if (rc == -ENOPART || rc == -EBADPART || rc == -ENOSYS) {
            printf("Partition %c doesn't exist or is not a supported FAT partition.\n",
                                   'A' + i + (dev * FAT_MAX_PARTITIONS));
            rc = 0;
            i++;       // Not a FAT partition, move to next
            j++;       // Count invalid partitions
            if (j < FAT_MAX_PARTITIONS)
              continue;

            printf("\n\nNO supported FAT partitions found.\n\n");
            printf("Erase device and create one large partition? (y/n)");
            gets(s);
            printf("\n\n");
            if (toupper(s[0]) == 'Y') {
               i = j = end = 0;
               _fat_clear_mbr(&(_fat_device_table[dev]));
               // Set format flags for device to be formatted
               if (dev == 0) {
                  flags_to_use = FDDF_COND_DEV_FORMAT | FDDF_COND_PART_FORMAT |
                                 FDDF_MOUNT_DEV_0 | FDDF_MOUNT_PART_0;
               }
               else {
                  flags_to_use = FDDF_COND_DEV_FORMAT | FDDF_COND_PART_FORMAT |
                                 FDDF_MOUNT_DEV_1 | FDDF_MOUNT_PART_0;
               }
             	rc = fat_AutoMount(flags_to_use);
               if (rc == -EEXIST) {
                  rc = 0;    // Ignore -EEXIST errors
               }
            	if (rc) {
            		printf("\nfat_AutoMount() error (%d) %ls\n", rc,
                                  error_message(rc));
            	}
               else {
                  i++;
               }
            }
            continue;
          }
          else break;
        }
      }
		printf("Formatting Partition %c . . . \n",
                                   'A' + i + (dev * FAT_MAX_PARTITIONS));
		rc = fat_FormatPartition( &(_fat_device_table[dev]),
											   fat_part_mounted[i], i, 6, NULL, NULL);

		if (rc) {
  			printf("fat_FormatPartition Error: (%d) %ls\n\n", rc,
                          error_message(rc));
         break;
		}
      else {
  			printf("Mounting Partition %c . . . \n",
                                   'A' + i + (dev * FAT_MAX_PARTITIONS));
   		rc = fat_MountPartition(fat_part_mounted[i++]);
	   	if (rc) {
		   	printf("fat_MountPartition Error: (%d) %ls\n\n", rc,
                            error_message(rc));
            break;
  			}
      }
	}

	printf("\n");
   return rc;
}


int main()
{
   int i, rc;
   char s[80];
   int dev;
   char cmd;
   word fmt_flags;

	rc = fat_AutoMount(FDDF_MOUNT_DEV_ALL | FDDF_MOUNT_PART_ALL);
	if (rc) {
		printf("\nfat_AutoMount() error (%d) %ls\n", rc, error_message(rc));
      // If device initialization fails or medium not present, exit
      if (rc == -EIO || rc == -ENOMEDIUM) exit(rc);
	}

   for (i = 0; i < num_fat_devices; i++) {
      if (rc = fat_UnsupportedPartition(i)) {
         printf("\nFound FAT%2d partition on device %d, THIS MAY HOLD DATA!\n",
                (rc == FAT_TYPE_12 ? 12 : (rc == FAT_TYPE_16 ? 16 : 32)), i);
      }
   }

   printf("\nNOTE:  Formatting destroys all data.\n");
   printf("       Quit now or forever hold your peace!\n\n");

   for (;;) {
   	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS; ++i) {
   		if (fat_part_mounted[i]) {
   			printf("Partition %c is mounted.\n", 'A' + i);
         }
   	}
   	printf("\nEnter a command (single char only, then press Enter):\n");
      printf("  p      Purge cache - try this if getting -310 error codes\n");
      printf("  a-d    Format partition 0,1,2 or 3 on device 0\n");
      if (num_fat_devices > 1)
	      printf("  e-h    Format partition 0,1,2 or 3 on device 1\n");
      printf("  0      Format entire device 0 (all FAT 12/16 partitions)\n");
      if (num_fat_devices > 1)
         printf("  1      Format entire device 1 (all FAT 12/16 partitions)\n");
      printf("  q      Quit (after unmounting all partitions)\n");
      printf("\nYour choice?  ");
      gets(s);
      printf("\n\n");
      cmd = toupper(s[0]);
      rc = 0;
      switch (cmd) {
      case 'P':
      	rc = fatftc_flushall(FTC_PURGE | FTC_WAIT);
     		printf("Cache purge was %s\n", rc ? "unsuccessful!" : "successful.\n");
      	if (!rc) {
      		printf("Restart fmt_device.c to format devices (exiting now).\n");
      		exit(0);
      	}
         break;
      case 'A': case 'B': case 'C': case 'D':
         rc = format(0, (cmd - 'A'));
         break;
      case 'E': case 'F': case 'G': case 'H':
         rc = format(1, (cmd - 'E'));
         break;
      case '0':
         rc = format(0, -1);  // -1 parameter triggers format ALL partitions
         break;
      case '1':
         rc = format(1, -1);  // -1 parameter triggers format ALL partitions
         break;
      case 'Q':
      	// Unmount all of the mounted FAT partitions & devices before exiting
      	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS;
         											 i += FAT_MAX_PARTITIONS) {
	      	if (fat_part_mounted[i]) {
	      		fat_UnmountDevice(fat_part_mounted[i]->dev);
	      	}
	      }
      	exit(0);
      default:
      	printf("Unrecognized command.\n");
         continue;
      }

      if (rc)
      	printf("\nOperation failed (%d) %ls\n\n", rc, error_message(rc));
      else
      	printf("\nOperation successful.\n\n");
   }
}


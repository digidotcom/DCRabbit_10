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
/*****************************************************************************
        Samples\FILESYSTEM\FAT_WRITE_MBR.C

        Requires that you run this on a core module with a FAT compatible
        flash memory device.  This sample works with the primary device
        defined by MBR_DRIVER_INIT as the default.  If you need to work
        with a different device, uncomment the appropriate MBR_DRIVER_INIT
        definition to override the core module's primary setting.

        This sample creates multiple partitions on an ERASED flash device.
        It only writes the partition table to the MBR, it does not
        format the individual partitions.  This can be done through the
        sample programs fmt_device or fat_shell.

        If the device is not erased, it will exit with an error message.

        You can erase a SD flash card by using sdflash_inspect and
        clearing the pages 0, 1 & 2.  Be aware that multiple
        partitions on a SD card are not supported by most PC's, so if
        you need your SD card to be PC compatible, use one partition.

        You can erase a serial flash device by using sflash_inspect and
        clearing the pages 0, 1, 2 & 3.

******************************************************************************/
#class auto
#define FAT_BLOCK

// To override the primary device and force use of the SD card
// uncomment the following two lines.
//#define MBR_DRIVER_INIT sd_InitDriver(root_driver, NULL)
//#define MBR_SIG "SDFLASH-1"

// To override the primary device and force use of the serial flash
// uncomment the following two lines.
//#define MBR_DRIVER_INIT sf_InitDriver(root_driver, NULL)
//#define MBR_SIG "SFLASH-1"


// Call in the FAT filesystem support code.
#use "fat16.lib"

mbr_drvr  my_driver;    // Driver structure, only used in fat_Init call
mbr_dev	 my_device;    // Device structure, this holds the partition table
fat_part  my_part;      // Only needed for call to fat_Init

int main()
{
	int rc;		// Return code store.  Always need to check return codes from
   				// FAT library functions.
   unsigned long sectors, partsize, start;
   float megs;
   char i, buf[15], *ptr;

	// Initialize the FAT filesystem.
   rc = fat_Init(
                 0,				// Partition number.  0 is first partition.
                 &my_driver,	// This will be initialized...
                 &my_device,	// ...as will this...
                 &my_part,		// ...and this.
                 0				// 0 specifies normal crash recovery
                );

   if (rc == -ENOMEDIUM) {
   	// No medium present, print error and exit
     	printf("No medium detected, insert card and run again.\n");
      exit(rc);
   }
   if (rc == -EIO) {
   	// Device failed initialization, print error and exit
     	printf("Flash device failed initialization.\n");
      exit(rc);
   }
	if (rc != -EUNFORMAT) {
   	// Not unformatted, print error and exit
     	printf("Device Formatted, Please erase flash device and run again.\n");
      exit(rc);
   }

   sectors = my_device.seccount - 1;
   for (i = 0, start = 1; i < 4 && sectors > 32; i++)
   {
     megs = (float)sectors / 2048.0;
     printf("\nAvailable space for Partition %d:%6.1fMB\n", i+1, megs);
     printf("Size in MB for Partition %d: ", i+1);
     gets(buf);
     megs = strtod(buf, &ptr);
     if (megs > 0.01) {
       partsize = (unsigned long)(megs * 2048.0);
       if (partsize > sectors) partsize = sectors;
       if (partsize > FAT16_MAX_PARTSECSIZE) {
         partsize = FAT16_MAX_PARTSECSIZE;
         printf("Partition %d limited to FAT16 maximum of %6.1fMB\n",
           i + 1, (float)partsize / 2048.0);
       }
       my_device.part[i].starthead = 254;
       my_device.part[i].parttype = 6;
       my_device.part[i].endhead = 254;
       my_device.part[i].startsector = start;
       my_device.part[i].partsecsize = partsize;
       start += partsize;
       sectors -= partsize;
     }
     else break;
   }

   printf("\nReady to Create new partitions (y/n)? ");
   gets(buf);
   if (buf[0] == 'y' || buf[0] == 'Y') {
      // Format the erased device with the new partition table
      rc = fat_FormatDevice(&my_device,0);

      if (rc < 0) {
        	printf("\nPartitioning failed with error code %d.\n", rc);
      }
      else {
         printf("\nPartitions created.\n");
      }
   }
   else {
      printf("\nPartitioning aborted.\n");
   }

   return fatftc_flushall(FTC_WAIT);
}



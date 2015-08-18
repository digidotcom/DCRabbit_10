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
        Samples\FILESYSTEM\FAT_CREATE.C

        Requires that you run this on a board with a compatible
        storage medium (serial flash, NAND flash or SD card).

        Demonstrate the FAT filesystem.  FAT is a filesystem familiar to
        most PC users who use the DOS operating system (i.e. Microsoft (R)
        MSDOS, IBM (R) PCDOS and similar).

        This sample creates a file, writes "Hello world!" to it,
        and closes the file.

        The file is then re-opened, read, and the results printed
        to the debugging console.

        We use the default structures provided in FAT_CONFIG.LIB, so we
        don't need to define our own structures in order for FAT to work.
        (FAT_CONFIG.LIB is automatically #used by FAT16.LIB.)

******************************************************************************/
#class auto


// This macro causes the FAT library to wait for everything to complete
// before returning to the caller.  This makes the application MUCH simpler.
#define FAT_BLOCK

// Uncomment to turn on Debug options
//#define FAT_DEBUG
//#define NFLASH_DEBUG	// only useful for boards with nand flash
//#define SFLASH_DEBUG	// only useful for boards with serial flash
//#define FATFTC_DEBUG
//#define PART_DEBUG
//#define ECC_DEBUG

// Call in the FAT filesystem support code.
#use "fat16.lib"


// When files are accessed, we need a FATfile structure.
FATfile my_file;


// This is a buffer for reading/writing the file.
char buf[128];


int main()
{
	int i;
	int rc;		// Return code store.  Always need to check return codes from
   				// FAT library functions.
   long prealloc;
   				// Used if the file needs to be created.
	fat_part *first_part;
					// Use the first mounted FAT partition.

	// Auto-mount the FAT file system, which populates the default mounted
	// partition list array that is provided in FAT_CONFIG.LIB.  This is the most
	// important information since, when you open a file, you need only to
	// specify the partition.  Also, tell auto-mount to use the default device
	// configuration flags at run time.
   rc = fat_AutoMount(FDDF_USE_DEFAULT);

	// Scan the populated mounted partitions list to find the first mounted
	// partition.  The number of configured fat devices, as well as the mounted
	// partition list, are provided for us in FAT_CONFIG.LIB.
	first_part = NULL;
	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS; ++i) {
		if ((first_part = fat_part_mounted[i]) != NULL) {
			// found a mounted partition, so use it
			break;
		}
	}

	// Check if a mounted partition was found
	if (first_part == NULL) {
		// No mounted partition found, ensure rc is set to a FAT error code.
		rc = (rc < 0) ? rc : -ENOPART;
	} else {
		// It is possible that a non-fatal error was encountered and reported,
		// even though fat_AutoMount() succeeded in mounting at least one
		// FAT partition.
		printf("fat_AutoMount() succeeded with return code %d.\n", rc);
		// We found a partition to work with, so ignore other error (if any).
		rc = 0;
	}

   // FAT return codes always follow the convention that a negative value
   // indicates an error.
	if (rc < 0) {
   	// An error occurred.  Here, we print out the numeric code.  You can
      // look in lib\filesystem\errno.lib to see what type of error it
      // really is.  Note that the values in errno.lib are the absolute
      // value of the return code.
   	if (rc == -EUNFORMAT)
      	printf("Device not Formatted, Please run Fmt_Device.c\n");
      else
	   	printf("fat_AutoMount() failed with return code %d.\n", rc);
      exit(1);
   }

   // OK, filesystem exists and is ready to access.  Let's create a file.

   // Do not pre-allocate any more than the minimum necessary amount of
   // storage.
	prealloc = 0;

   // Open (and maybe create) it...
   rc = fat_Open(
                 first_part,	// First partition pointer from fat_AutoMount()
                 "HELLO.TXT",	// Name of file.  Always an absolute path name.
                 FAT_FILE,		// Type of object, i.e. a file.
                 FAT_CREATE,	// Create the file if it does not exist.
                 &my_file,		// Fill in this structure with file details
                 &prealloc		// Number of bytes to allocate.
                );

	if (rc < 0) {
   	printf("fat_Open() failed with return code %d\n", rc);
      exit(1);
   }

   // Write to it...
   rc = fat_Write(
                  &my_file,				// File, as set by fat_Open()
                  "Hello, world!\r\n",	// Some data to write.
                  15							// Number of characters to write.
                 );
   // Note: in the above, we write CR LF (\r \n) at the end of the string.
   // This is a FAT (or really, DOS) convention for text files.  With a serial
   // flash we do not have to worry about other operating systems reading our
   // files, but it is good to get into the habit of using the standard
   // line-end convention. [If you just use LF (\n) then the file will read
   // just fine on Unix systems, but some DOS-based programs may have
   // difficulties.  In any case, CR LF is required for HTML and many other
   // networking protocols.]

   if (rc < 0) {
   	printf("fat_Write() failed with return code %d\n", rc);

      // In real applications which don't exit(), we would probably want to
      // close the file and continue with something else.
      exit(1);
   }

   // Done writing; close it.
   rc = fat_Close(&my_file);

   // Many programmers do not check the return code from "close".  This is a
   // bad idea, since an error return code can indicate that data was lost!
   // Your application should be concerned about this...
   if (rc < 0) {
   	printf("fat_Close() failed with return code %d\n", rc);
      // In this case, we soldier on to see if the file can be read.
   }

   // At this point, my_file cannot be used until it is opened again.

   // Open the same file for reading.
   rc = fat_Open(
                 first_part,	// First partition pointer from fat_AutoMount()
                 "HELLO.TXT",	// Name of file.  Always an absolute path name.
                 FAT_FILE,		// Type of object, i.e. a file.
                 0,				// 0 means the file must exist.
                 &my_file,		// Fill in this structure with file details
                 NULL			// This will not be used, you can pass NULL.
                );

	if (rc < 0) {
   	printf("fat_Open() (for read) failed with return code %d\n", rc);
      exit(1);
   }

   // Read the first 128 bytes (sizeof buf) from the file.  Of course, we
   // only wrote 15 characters so this will be all that can be read.
   // No matter, the return code indicates this.
   rc = fat_Read(&my_file, buf, sizeof(buf));

   if (rc < 0) {
   	printf("fat_Read() failed with return code %d\n", rc);
   }
   else {
   	// Read OK.  Print out the buffer contents.
      printf("Read %d bytes:\n", rc);
      printf("%*.*s", rc, rc, buf);		// Print a string which is not NULL
      printf("\n");							//   terminated.
   }

   // Since we are using blocking mode, it will not return until it has
   // closed all files and unmounted the partition & device.
   fat_UnmountDevice(first_part->dev);

   // Many operating systems do not like "hard reset/reboot" when a filesystem
   // is involved.  The Rabbit FAT implementation is robust enough to
   // gracefully recover from reset/power loss without losing data. It will
   // automatically recover when fat_Init() is called at startup. However, it
   // is still a good idea to shut down properly if you know you are exiting
   // the program.
   printf("All OK.\n");
}


/***************************************************************
	FAT_NB_Costate.C
   Digi International, Copyright (c) 2007.  All rights reserved.

   Requires that you run this on a board with a compatible
   storage medium (serial flash, NAND flash or SD card).

   Non-Blocking demo of FAT library using Costatements to
   implement a simple multi-tasking system.

   If running on an RCM4xxx or RCM54xxW target with development board,
   one costate is conditionally coded to blink an LED.

***************************************************************/

// Map program to xmem if not compiling to separate I&D space.
#if !__SEPARATE_INST_DATA__
#memmap xmem
#endif

#define MAX_FILES		5

#if (CPU_ID_MASK(_CPU_ID_) == R4000) || (RCM5400W_SERIES)
   // Assume R4xxx Dev. Board
   #define LED_DR        PBDR          // Port B Data Register
   #define LED_DRSHADOW  PBDRShadow    // Port B Data Register Shadow
   #define LED_DDR       PBDDR         // Port B Data Direction Register
   #define LED_DDRSHADOW PBDDRShadow   // Port B Data Direction Register Shadow
   #define LED_PIN       2             // data bit 2
   #define LED_MASK      (1 << LED_PIN)
#endif

// Uncomment to turn on Debug options
//#define FAT_DEBUG
//#define NFLASH_DEBUG	// only useful for boards with nand flash
//#define SFLASH_DEBUG	// only useful for boards with serial flash
//#define SDFLASH_DEBUG	// only useful for boards with serial flash
//#define FATFTC_DEBUG
//#define PART_DEBUG

// Set file system to use forward slash as directory separator
#define FAT_USE_FORWARDSLASH

// This is the FAT filesystem library
#use "fat16.lib"

// Required control structure to operate the FAT filesystem
FATfile  file;

// Main program
int main()
{
#define REC_LEN	21				// Record length (uses fixed length records)
   int i, rc, savedRC;
   int led;
	int h, m, s;
   int filestate;
   long alloc;
   long readto;
	float val;
   char *iptr, *optr;
   char ibuf[REC_LEN], obuf[REC_LEN];
   int icount, ocount;
   static char * const name = "/nbdata.txt";
	fat_part *first_part;		// Use the first mounted FAT partition.

   h = m = s = 0;					// Clear system variables
   filestate = 0;
   led = 0;
   readto = 0;
   alloc = 1;
   savedRC = 0;

#if defined LED_DR
   WrPortI(LED_DR, &LED_DRSHADOW, LED_DRSHADOW | LED_MASK);
 #if PADR == LED_DR
   WrPortI(SPCR, &SPCRShadow, 0x84);
 #else
   WrPortI(LED_DDR, &LED_DDRSHADOW, LED_DDRSHADOW | LED_MASK);
 #endif
 #if defined LED_FR
   WrPortI(LED_FR, &LED_FRSHADOW, LED_FRSHADOW & ~LED_MASK);
 #endif
#endif

	// Auto-mount the FAT file system, which populates the default mounted
	// partition list array that is provided in FAT_CONFIG.LIB.  This is the most
	// important information since, when you open a file, you need only to
	// specify the partition.  Also, tell auto-mount to use the default device
	// configuration flags at run time.
   while ((rc = fat_AutoMount(FDDF_USE_DEFAULT)) == -EBUSY);

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
      // look in include/errno.h to see what type of error it
      // really is.  Note that the values in errno.h are the absolute
      // value of the return code.
   	if (rc == -EUNFORMAT)
      	printf("Device not Formatted, Please run Fmt_Device.c\n");
      else
	   	printf("fat_AutoMount() failed with return code %d.\n", rc);
      exit(1);
   }

   // OK, filesystem exists and is ready to access.  Let's create a file.

	// Delete the data file (in case re-running the program - Blocking)
   while ((rc = fat_Delete( first_part, FAT_FILE, name)) == -EBUSY);
   if (rc < 0 && rc != -ENOENT)
   {
   	printf("fat_Delete: rc = %d\n",rc);
      while ((rc = fat_UnmountDevice( first_part->dev )) == -EBUSY);
   	return rc;
   }

	// Open the data file  (Block until complete)
   file.state = 0;			// Initialize filestate to Idle
   while ((rc = fat_Open( first_part, name, FAT_FILE, FAT_MUST_CREATE,
   									&file, &alloc)) == -EBUSY);
   if (rc < 0)
   {
   	printf("fat_Open: rc = %d\n",rc);
      while ((rc = fat_UnmountDevice( first_part->dev )) == -EBUSY);
   	return rc;
   }

  	printf("Fat_NB_Costate Log -- Press any key to exit.\n\n");

   // FAT Library is used in Non-Blocking mode from here on
   while (1)
   {
	   // This process puts random time-stamped data into data file.
		costate putdata always_on
	   {
   		waitfor (DelaySec(1));     // Wait for one second to elapse
      	if (++s == 60)					// Increment seconds
	      {
   	   	s = 0;
      		if (++m == 60)          // Increment minutes
         	{
	         	m = 0;
   	         if (++h == 24)       // Increment hours
      	      	h = 0;
         	}
	      }
		   // Create the fill string by inserting timestamp and reading
   		sprintf(obuf, "%02d:%02d:%02d -- %6.3f \n", h, m, s, (25.0 * randf()));
	      ocount = 0;             // Initialize output count and buffer pointer
   	  	optr = obuf;
      	waitfor (filestate == 0);  // Wait until file is available
	      filestate = 1;					// Show file is being updated
   	   while (ocount < REC_LEN)   // Loop until entire record is written
      	{
	      	waitfor((rc = fat_Write(&file, optr, REC_LEN - ocount)) != -EBUSY);
	         if (rc < 0)
   	      {
			   	printf("fat_Write: rc = %d\n",rc);
			      while ((rc = fat_UnmountDevice( first_part->dev )) == -EBUSY);
	   			return rc;
   	      }
      	   optr += rc;          // Move output pointer
         	ocount += rc;        // Add number of characters written
   	   }
      	filestate = 0;				// Show file is idle
	   }

   	// This waits for keypress
   	//  (and conditionally turns an LED on or off every 1/2 second).
		costate blink always_on
   	{
   		waitfor (DelayMs(500));
   	   led ^= 1;
	      // Toggle LED
#if defined LED_DR
			BitWrPortI(LED_DR, &LED_DRSHADOW, led, LED_PIN);
#endif
         if (kbhit()) {
         	waitfor (filestate == 0);  // Wait until file is available
				savedRC = 0;
	      	// Unmount all of the mounted FAT partitions & devices before exit
   	   	for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS; ++i) {
		      	if (fat_part_mounted[i]) {
				      while ((rc = fat_UnmountDevice(fat_part_mounted[i]->dev)) ==
						       -EBUSY);
						if (!savedRC && rc) {
							savedRC = rc;
						}
	   	   	}
	      	}
         	exit(savedRC);
         }
  		}

	   // This process reads the data file written by the first process
   	// every 10 seconds showing the new records that were written.
		costate showdata always_on
   	{
   		waitfor (DelaySec(10));    // Wait for 10 seconds to elapse
	      waitfor (filestate == 0);  // Wait until file is available
   	   filestate = 2;					// Show file is being read
      	// Seek read position
	      waitfor (fat_Seek(&file, readto, SEEK_SET) != -EBUSY);

	      while (filestate)       // Loop until all records have been read
   	   {
      		icount = 0;          // Initialize receive count and buffer pointer
      		iptr = ibuf;
		      while (icount < REC_LEN)	// Loop until a full record is received
   		   {
	   		   waitfor((rc = fat_Read(&file, iptr, REC_LEN-icount)) != -EBUSY);
         		if (rc < 0)
	         	{
	            	if (rc == -EEOF)
   	            {
      	         	filestate = 0;		// Show file in idle state
         	         break;
            	   }
			   		printf("fat_Read: rc = %d\n",rc);
				      while ((rc = fat_UnmountDevice( first_part->dev ))== -EBUSY);
   					return rc;
         		}
	         	iptr += rc;          // Move input pointer
		         icount += rc;        // Add number of characters received
      		}
            if (filestate)
            {
	         	printf("%s", ibuf);
		         readto += REC_LEN;
            }
   	   }
	   }
   }
}
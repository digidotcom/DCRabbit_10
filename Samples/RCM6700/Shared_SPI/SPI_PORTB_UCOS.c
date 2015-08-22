/*****************************************************************

   SPI_PORTB_UCOS.C

	Rabbit Semiconductor, 2007

	This sample program is for R6000 series controllers.


	Description
	===========
	This program demonstrates setting up uC/OS-II to use SPIgetSemaphore()
   and SPIfreeSemaphore() to protect against multiple devices accessing
   the SPI from different tasks. These functions supercede MCOS standard
   semaphore functions for SPI port usage.


   This program requires the Dynamic C uC/OS-II Module.

   Instructions
   ============
   1. Compile and run program.
   2. Monitor STDIO output messages to see SPI Port B accesses
      by FAT, Serial Flash, and pseudo customer device.

****************************************************************/

#memmap xmem
#class auto

#use "r6000_bios.lib"
#use "idblock_api.lib"
#use "bootdev_sf_api.lib"

// Customer MARCO(s) for semaphore SPI port B control
#define SPI_CUST &_spi_dev[3]  // Customer starts with index 3, index's
							 			 // 0, 1 and 2 are reserverd for LIB usage.

#define SPI_SF_CUST_DIVISOR 3 // SPI CLK
#define SPI_CUST_SBER  0x20 	// Set SPI mode for given device


//*** Structure for tracking of shared devices on the SPI port ***
// Note: This is already defined in R6000_bios.lib
// struct SPIDev{
//   char ID;         // 0   Device ID
//   char SPInesting; // 1   Semaphore nesting
//   char SCLCKdiv;   // 2   SPI CLK divider
//   char SBERvalue;  // 3   SPI mode setting
// };

// First 3 struct elements are fixed and must not change, customer app will
// start with dev 4 and add additonal entries for devices as needed.
const struct SPIDev _spi_dev[] = {
/*Dev 1 */{1, 1, SPI_SF_DIVISOR, SPI_SF_SBER},    	// SF
/*Dev 2*/ {2, 1, SPI_DF_DIVISOR, SPI_DF_SBER}, 		// FAT
/*Dev 3*/ {3, 1, SPI_SD_DIVISOR, SPI_SD_SBER}, 		// SD (not supported)
/*Dev 4*/ {4, 1, SPI_SF_CUST_DIVISOR, SPI_CUST_SBER}  //CUSTOMER Device #1

};

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


//#define FAT_USE_UCOS_MUTEX

// Call in the FAT filesystem support code.
#use "fat16.lib"

// When files are accessed, we need a FATfile structure.
FATfile my_file;

#define MINFLASHADDR 0x00100000UL  // The lowest addr. unvailable for programs
#define MAXFLASHADDR 0x001F0000UL  // Top of flash - 64K

// This is a buffer for reading/writing the file.
char buf[128];
fat_part *first_part;
int i;
// FAT library functions.
long prealloc;
// Use the first mounted FAT partition.
unsigned int cmd;

int logData();
//int displayLog();
int FAT_Write(void);

int  sampleComplete;        // global flag
unsigned long logAddr;    // current log position


#define OS_TIME_DLY_HMSM_EN	 1   // Enable hour, min, sec, ms delays
#define OS_TASK_CREATE_EN	    1	  // Enable normal task creation

// Bring in library
#use "ucos2.lib"

int  sampleComplete;        // global flag

void SF_Access(void* pdata);
void FAT_Access(void* pdata);
void Cust_Access(void* pdata);


void main()
{
   auto int rc;

   // Auto-mount the FAT file system, which populates the default mounted
	// partition list array that is provided in FAT_CONFIG.LIB.  This is the most
	// important information since, when you open a file, you need only to
	// specify the partition.  Also, tell auto-mount to use the default device
	// configuration flags at run time.
   // Note: MUST automount FAT device before starting up MCOS.

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

   // Initialize MCOS
   OSInit();

   rc = (int)OSTaskCreate(Cust_Access,  NULL, 512, 6);
   rc = (int)OSTaskCreate(SF_Access,  NULL, 512, 5);
   rc = (int)OSTaskCreate(FAT_Access,  NULL, 512, 4);

   OSStart();    // Start multi-tasking
}


void SF_Access(void* pdata)
{
   static int rc;
   static char buffer[32];

	while(1)
   {

   	// Wait for SPI interface to become available, once done using the
      // the Semaphore MUST call SPIfreeSemaphore to keep the semaphore in sync
      // for future SPI use.
      if((rc=SPIgetSemaphore(SPI_SF)) == 0)
      {
      	// Yield!!!
      	printf("\n\nSF is Owner\n");

   		logAddr = 0x00100000UL;

      	strcpy(buffer, "Testing SFLASH 1-2-3\0");

      	// -EBUSY means the serial flash is busy writing or erasing a sector.
      	//  sbfWriteFlash will return this value many times before any write
      	//  is completed because these operations take several milliseconds.
      	//  We don't want to busy wait while this happens because an application
      	//  will normally have lots of other work to do. So if  sbfWriteFlash
      	//  returns -EBUSY, allows execution for other tasks to run.
  	 		while(-EBUSY == (rc = sbfWriteFlash((unsigned long)logAddr,
              (void*)buffer, strlen(buffer))));
              OSTimeDlyHMSM(0,0,0,500);
      	if(rc == -1){
         	printf("\nIllegal serial flash write attempt");
          	exit(rc);
      	}

      	// Clear buffer to assure data read from Sflash is valid
      	memset(buffer, 0, sizeof(buffer));

      	while(1)
         {
         	if((rc =sbfRead(buffer, logAddr, sizeof(buffer) ) ) == 0)
            	break;
            OSTimeDlyHMSM(0,0,0,500);
         }
      	if(strstr(buffer, "Testing SFLASH 1-2-3"))
      	{
      		printf("\nSuccessful Write/Read of SFlash\n\n");
      		printf("SF- All OK\n\n");
      	}
      	else
      	{
         	printf("\nFailed Write/Read of SFlash\n\n");
      		printf("SF- Fails Test\n\n");
            while((rc =sbfRead(buffer, logAddr, sizeof(buffer) ) ) != 0);
      	}

      	// This delay is for example only so SPI_FAT costate can execute to see
      	// semaphore is locked by the SPI_SF device.
   		SPIfreeSemaphore(SPI_SF);
      	printf("\nSF - Release Ownership\n");
      }
      if(rc > 0)
   		printf("SFlash detected other device has ownership     \n");
      OSTimeDlyHMSM(0,0,0,500);
   }
}


// read the A/D with using the low level driver
void FAT_Access(void* pdata)
{
	static int rc;

   while(1)
   {

   	if((rc=SPIgetSemaphore(SPI_DF)) == 0)
      {
      	printf("\n\nFAT is Owner\n");
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

   		// Done reading; close it.
   		rc = fat_Close(&my_file);

   		// Many programmers do not check the return code from "close".  This is a
   		// bad idea, since an error return code can indicate that data was lost!
   		// Your application should be concerned about this...
   		if (rc < 0) {
   			printf("fat_Close() failed with return code %d\n", rc);
      		// In this case, we soldier on to see if the file can be read.
   		}

   		// Since we are using blocking mode, it will not return until it has
   		// closed all files and unmounted the partition & device.
   		//fat_UnmountDevice(first_part->dev);

   		// Many operating systems do not like "hard reset/reboot" when a filesystem
   		// is involved.  The Rabbit FAT implementation is robust enough to
   		// gracefully recover from reset/power loss without losing data. It will
   		// automatically recover when fat_Init() is called at startup. However, it
   		// is still a good idea to shut down properly if you know you are exiting
   		// the program.
   		printf("FAT - All OK.\n\n");

      	// This delay is for example only, so SPI_SF costate can execute to
      	// see semaphore is locked by the SPI_FAT device.
         OSTimeDlyHMSM(0,0,0,500);
   		SPIfreeSemaphore(SPI_DF);
      	printf("\nFAT - Release Ownership\n");
      }
      if(rc > 0)
   		printf("FAT detected other device has ownership     \n");
      OSTimeDlyHMSM(0,0,0,500);
   }

}

void Cust_Access(void* pdata)
{
	static int rc;

	while(1)
   {
		if((rc=SPIgetSemaphore(SPI_CUST)) == 0)
      {
      	printf("\n\nCUST is Owner\n");
         // Do SPI stuff
         OSTimeDlyHMSM(0,0,0,500);
        	SPIfreeSemaphore(SPI_CUST);
         printf("\nCUST - Release Ownership\n");
      }
   	if(rc > 0)
   		printf("CUST detected other device has ownership     \n");
      OSTimeDlyHMSM(0,0,0,500);
   }
}





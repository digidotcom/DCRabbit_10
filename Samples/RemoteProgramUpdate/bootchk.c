/*******************************************************************************
	Samples/RemoteProgramUpdate/BootChk.c
	Digi International, Copyright ©2009.  All rights reserved.

	Application Note 421 contains full documentation on using the Remote Program
	Update library and samples.

	*** Although this sample was designed for the RCM43xx, it can be used with
	other core modules.  Instructions for non-RCM43xx modules are shown after the
	instructions for the RCM43xx. ***

	Sample to demonstrate Board_Update.lib functions on an RCM43xx.  At boot,
	will check the SD card for a file called bootchk.bin (override filename
	using the FIRMWARE_BIN macro below).

	If that file is a valid firmware image and a newer version than what's
	already running on the RCM43xx, it will install it to the boot flash and
	reboot.

	Requires an SD card writer for your PC, to copy new firmware file to SD card.

	Instructions
	------------
	Set the _FIRMWARE_NAME_ and _FIRMWARE_VERSION_ macros in the Project Defines.

	_FIRMWARE_NAME_="Boot Check Demo"
	_FIRMWARE_VERSION_=0x0100

	Compile and this sample to the flash on the RCM4300.  Connect a serial cable
	from the PC to the 10-pin header J4 on the prototyping board.  Open the
	PC's serial port to 115200 bps using a terminal program.

	Reset the RCM4300 and confirm that you can view its output in the terminal
	program.  After testing, turn off the RCM4300 and remove its mini-SD card.

	Now increment _FIRMWARE_VERSION_ (set to 0x0101 to represent version 1.01)
	in the Dynamic C project defines and compile it to a binary file.  Use the
	mini-SD adapter from your RCM4300 dev kit and an SD reader on the PC to copy
	the binary file onto the mini-SD card (should be called bootchk.bin).

	Place the mini-SD card back inside the RCM4300.  When you turn on the core
	module and view its output with the terminal program, you should see it find
	the new firmware, install it and reboot.

	Integrating with another program
	--------------------------------
	To add this functionality to an existing program, copy the bu_bootcheck()
	and unmount_all() functions to your program, and add a call to bu_bootcheck()
	after you've mounted the FAT filesystems.

*/

#if ! RCM4300_SERIES
	#warnt "This sample was designed for RCM43xx core modules.  Be sure to read"
	#warnt "the instructions below on how to use it with this target."
#endif

/*
	Using this sample with non-RCM43xx core modules
	-----------------------------------------------
	It is possible to run this sample on other modules that support the FAT
	file system using the following instructions.

	1) Choose a program to represent the "new" firmware you're going to install
		with this sample (e.g., PONG.C, DEMO1.C or even this sample).

	2) Set _FIRMWARE_NAME_ and _FIRMWARE_VERSION_ (e.g., 0x0101) in the "Project
		Defines" tab of the "Project Options".  Choose the correct target device
		on the "Targetless" tab.

	3) Compile to a .bin file.

	4) Run a sample that will allow you to upload a file to the FAT filesystem
		on the core module.
			Samples/tcpip/ftp/ftp_fat.c		Uplaod file with an FTP client.

		You may need to use Samples/filesystem/fat/fat_shell.c to format the
		filesystem first if you haven't done so already.

	5) Upload the .bin file created in step #3.

	6) Set _FIRMWARE_VERSION_ to a smaller value (e.g., 0x0100) than what was
		used in step #2.

	7) Set FIRMWARE_BIN macro in this sample to the filename used in step 4.

	8) Compile this sample (bootchk.c) to the core module.

	9) Select "Close Connection" from the Run menu in Dynamic C, and then
		disconnect power to the core module.

	10) Connect a serial cable from the PC to the 10-pin header J4 on the
		prototyping board.  Open the PC's serial port to 115200 bps using a
		terminal program.

	11) Power on the core module, and you should see it find the new firmware,
		install it and then reboot into the new program.

*******************************************************************************/

/***********************************
 * Configuration                   *
 ***********************************/

// File to check for updates on the FAT filesystem (e.g., SD card on RCM4300)
// Remember that the Rabbit's FAT filesystem uses short, 8.3 filenames.
#define FIRMWARE_BIN "a:bootchk.bin"

// define this macro for stdio status messages from board_update.lib
//	#define BOARD_UPDATE_VERBOSE

// Modify these macros to change the serial port used to monitor this sample.
// Since it will install new firmware and reboot, it cannot be tested from
// within Dynamic C.
#define  STDIO_DEBUG_SERIAL   SDDR
#define	STDIO_DEBUG_BAUD		115200
#define	STDIO_DEBUG_ADDCR

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem

// Load the FAT filesystem library
#use "fat16.lib"

// load the Remote Program Update library
#use "board_update.lib"

void unmount_all()
{
	int i, rc;

   for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS;
   	i += FAT_MAX_PARTITIONS)
   {
      if (fat_part_mounted[i])
      {
      	do {
	         rc = fat_UnmountDevice(fat_part_mounted[i]->dev);
	      } while (rc == -EBUSY);
         if (rc < 0)
         {
            printf("Unmount Error on %c: %ls\n", 'A' + i, strerror(rc));
         }
      }
   }
}

int bu_bootcheck()
{
	int error;
	int progress;
	int i;
	firmware_info_t fi;

	printf( "Currently running %s v%u.%02x...\n", _FIRMWARE_NAME_,
		_FIRMWARE_VERSION_ >> 8, _FIRMWARE_VERSION_ & 0xFF);

	printf( "Checking %s for updated firmware...\n", FIRMWARE_BIN);
	error = buOpenFirmwareFAT( FIRMWARE_BIN, BU_FLAG_NONE);

	if (!error)
	{
	   // buGetInfo is a non-blocking call, and may take multiple attempts
	   // before the file is completely open.
	   i = 0;
	   do {
	      error = buGetInfo( &fi);
	   } while ( (error == -EBUSY) && (++i < 20) );
	}

	if (error)
	{
		printf( "Error %d checking for new firmware on FAT filesystem\n", error);
	}
   else
   {
      printf( "Found %s v%u.%02x on FAT...\n", fi.program_name,
         fi.version >> 8, fi.version & 0xFF);

      if (fi.version < _FIRMWARE_VERSION_)
      {
			printf( "Ignoring older version.\n");
      }
      else if (fi.version == _FIRMWARE_VERSION_)
      {
			printf( "Ignoring same version on FAT filesystem.\n");
      }
      else
      {
			printf( "Attempting to install new version...\n");
	      progress = 0;
	      do
	      {
	         printf( "\r verify %u.%02u%%\r", progress / 100, progress % 100);
	         error = buVerifyFirmware( &progress);
	      } while (error == -EAGAIN);
         if (error)
         {
            printf( "\nError %d verifying firmware\n", error);
				printf( "firmware image bad, installation aborted\n");
	      }
	      else
	      {
		      printf( "verify complete, installing new firmware\n");
		      error = buInstallFirmware();
		      if (error)
		      {
					printf( "!!! Error %d installing firmware !!!\n", error);
				}
				else
				{
					printf( "Install successful: rebooting.\n");
					exit( 0);
				}
		   }
      }
   }

   // make sure firmware file is closed if there were any errors
	while (buCloseFirmware() == -EBUSY);

	return error;
}

int main()
{
	int result;

	// Auto-mount the FAT filesystem
	printf( "mounting FAT partitions\n");
   do {
	   result = fat_AutoMount(FDDF_USE_DEFAULT);
   } while (result == -EBUSY);
   if (result == -EIO || result == -ENOMEDIUM)
   {
		printf( "Fatal device initialization error!  Exiting now.\n");
		exit( result);
   }
   if (! fat_part_mounted[0])
   {
		printf( "Couldn't mount A:\n");
		exit( -EIO);
   }

	// register a function to unmount all FAT volumes on exit
   atexit( unmount_all);

	bu_bootcheck();

	printf( "\n\nIf this sample did anything, it would start here...\n");

	// Unmount filesystem and spin.  If main() ends, the program restarts.
	unmount_all();

   printf( "\n\nFAT partitions have been unmounted.\n");

	for (;;)
	{
		// do nothing...
	}

	return 0;
}
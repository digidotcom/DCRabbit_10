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
/*
	Samples/RemoteProgramUpdate/tftp_get_firmware.c

	Application Note 421 contains full documentation on using the Remote Program
	Update library and samples.

	Sample of downloading new firmware from a TFTP server and then installing
	it on the boot flash.

	Before running this program, be sure to test your TFTP Server with a TFTP
	client on the same network as the Rabbit to ensure that you can connect.
	If you're running Windows, the firewall can prevent TFTP clients from
	connecting to your TFTP Server.

	If you don't already have a TFTP Server to test with, you can download a
	free TFTP Server for Windows from SolarWinds:

	http://www.solarwinds.com/products/freetools/free_tftp_server.aspx

	You'll need to compile another one of the samples (like PONG.C) to a .bin
	file, and put it on the TFTP server with the name "firmware.bin".

   NOTE: For the RCM5750, you must include this macro definition in the
   Global Macro Definitions of the Project Options:  BU_ENABLE_MINILOADER

   To use Power-Fail Safe updates on boards with a serial boot flash, define
   BU_ENABLE_SECONDARY in the Global Macro Definitions of the Project Options.
   PFS RPU works by storing two complete firmware images on the boot flash.
   Updates are done by copying new firmware to the non-boot image and then
   using an atomic flash write to enable that firmware for booting.
*/

// Configuration Options

// Source of firmware, if not already defined in project defines
#ifndef TFTP_SERVER
	#define TFTP_SERVER  "10.10.6.1"
	#define TFTP_FILE "firmware.bin"
#endif

/*
 * Unless BU_ENABLE_SECONDARY was defined in the Global Macro Definitions,
 * define one of the following macros to select the temp storage location.
 * On the RCM5600W, the only option is to write new firmware directly to the
 * boot flash.  This is dangerous, as a power failure during the download will
 * result in a board that needs to be reloaded with the RFU or some other
 * direct serial connection on the programming port.
 */
//	#define BU_TEMP_USE_FAT				// use file on FAT filesystem
//	#define BU_TEMP_USE_SBF				// use unused portion of serial boot flash
//	#define BU_TEMP_USE_SFLASH			// use serial data flash (without FAT)
//	#define BU_TEMP_USE_DIRECT_WRITE	// write directly to boot firmware image

/*
 * If using the serial data flash as a target (BU_TEMP_USE_SFLASH), you can
 * specify a page offset (other than the default of 0) for storing the
 * temporary firmware image.
 */
//	#define BU_TEMP_PAGE_OFFSET 0

// define this macro for stdio status messages from board_update.lib
// #define BOARD_UPDATE_VERBOSE

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// End of Configuration Options

// Make sure an option has been enabled.
#if ! defined BU_TEMP_USE_FAT && \
	 ! defined BU_TEMP_USE_SBF && \
	 ! defined BU_TEMP_USE_SFLASH && \
	 ! defined BU_TEMP_USE_DIRECT_WRITE && \
	 ! defined BU_ENABLE_SECONDARY
#fatal "You must uncomment a BU_TEMP_USE_xxx macro at the top of this sample."
#endif

// Redirect STDOUT to serial port A to monitor progress and reboot after install
#define  STDIO_DEBUG_SERIAL   SADR
#define	STDIO_DEBUG_BAUD		115200
#define	STDIO_DEBUG_ADDCR

// default functions to xmem
#memmap xmem

// need a UDP buffer for the TFTP connection
#define MAX_UDP_SOCKET_BUFFERS 2

#use "dcrtcp.lib"
#use "tftp.lib"

#ifdef BU_TEMP_USE_FAT
	// Set FAT library to blocking mode (optional, required if using uC/OS-II)
	// #define FAT_BLOCK

	// Set file system to use forward slash as directory separator (optional)
	#define FAT_USE_FORWARDSLASH

	// Load the FAT filesystem library
	#use "fat16.lib"
#endif

#use "board_update.lib"

#ifdef BU_TEMP_USE_FAT
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
#endif

/*
	Return Codes:
		Note that this function will either succeed and reboot to new firmware,
		or it will fail with one of the following error codes:

	      -EILSEQ: Not a valid firmware_info_t struct (bad marker bytes
	               or unsupported version of structure).
	      -EBADMSG: Bad CRC (structure has been corrupted).
	      -ENODATA: Source not open, or firmware info not found in source.
	      -EPERM: Firmware was compiled for a different target.
	      -EBADDATA: CRC-32 mismatch, firmware image corrupted.
	      -EBADMSG: CRC-32 mismatch after installing.
	      -ENOMEM: Couldn't allocate buffer to copy firmware.

	      Error codes when using a FAT file for temporary storage:
	      -EINVAL: Couldn't parse BU_TEMP_FILE.
	      -ENOENT: File BU_TEMP_FILE does not exist.
	      -EMFILE: Too many open files.

	      Error codes when using the serial flash for temporary storage:
	      -ENODEV: Can't find/read the serial flash.
*/

int install_firmware()
{
	firmware_info_t fi;
	int			i;
	int 			result;
	int			progress;

   printf( "verifying and installing new firmware\n");

   result = buOpenFirmwareTemp( BU_FLAG_NONE);

   if (!result)
   {
	   // buGetInfo is a non-blocking call, and may take multiple attempts
	   // before the file is completely open.
	   i = 0;
	   do {
	      result = buGetInfo( &fi);
	   } while ( (result == -EBUSY) && (++i < 20) );
   }

   if (result)
   {
      printf( "Error %d reading new firmware\n", result);
   }
   else
   {
      printf( "Found %s v%u.%02x...\n", fi.program_name,
         fi.version >> 8, fi.version & 0xFF);

      printf( "Attempting to install new version...\n");
      progress = 0;
      do
      {
         printf( "\r verify %u.%02u%%\r", progress / 100, progress % 100);
         result = buVerifyFirmware( &progress);
      } while (result == -EAGAIN);
      if (result)
      {
         printf( "\nError %d verifying firmware\n", result);
         printf( "firmware image bad, installation aborted\n");
      }
      else
      {
         printf( "verify complete, installing new firmware\n");
         result = buInstallFirmware();
         if (result)
         {
            printf( "!!! Error %d installing firmware !!!\n", result);
         }
         else
         {
            printf( "Install successful: rebooting.\n");
            exit( 0);
         }
      }
   }

   // make sure firmware file is closed if there were any errors
	while (buCloseFirmware() == -EBUSY);

   return result;
}

// It's safer to keep sockets as globals, especially when using uC/OS-II.  If
// your socket is on the stack, and another task (with its own stack, instead
// of your task's stack) calls tcp_tick, tcp_tick won't find your socket
// structure in the other task's stack.
// Even though this sample doesn't use uC/OS-II, using globals for sockets is
// a good habit to be in.
udp_Socket demosock;

/*	Return codes:
		Most return codes come from install_firmware (see above).

		Additional return codes for tftp_and_install:
         -EPERM: buTempCreate not supported on this hardware.
         -ENODEV: Couldn't read from serial flash.
         -EBUSY: Timeout waiting for FAT filesystem.
         -ENODATA: Download didn't contain a valid firmware image for this
         		device.
         <0: Error opening FAT file, see fat_Open for full list of
             error codes and their meanings.

		TFTP Errors:
         -1: Error from remote side, transfer terminated.  In this
             case, the ts_addr->file field will be overwritten with a
             null-terminated error message from the server.
         -2: Error, could not contact remote host or lost contact.
         -3: Timed out, transfer terminated.
         -4: (not used)
         -5: Transfer complete, but truncated because buffer too
             small to receive the complete file.
			-7: Server address zero. (Failed to resolve TFTP_SERVER)
*/

int tftp_and_install( char *server, char *file)
{
	int 			result;
	int			offset;
	char			ipbuf[16];
	unsigned long	copied;
	struct tftp_state ts;
	char			buffer[512];

  	ts.state = 0;								// 0 = read
  	ts.buf_len = 512;							// download a block at a time
  	ts.buf_addr = paddr(buffer);			// address of buffer
  	ts.my_tid = 0;								// zero to use default TFTP UDP port number
  	ts.sock = &demosock;						// point to socket to use
  	ts.rem_ip = resolve(TFTP_SERVER);	// resolve server IP address
  	ts.mode = TFTP_MODE_OCTET;				// send/receive binary data
  	strcpy(ts.file, TFTP_FILE);			// set file name on server

  	printf("Downloading %s from %s via TFTP...\n",
  		ts.file, inet_ntoa (ipbuf, ts.rem_ip));

  	if ( (result = tftp_init(&ts)) )
  	{
		printf("Couldn't resolve TFTP server address.\n");
		return result;
  	}

	while ( (result = buTempCreate()) == -EBUSY);
	if (result)
	{
		printf( "Error %d calling buTempCreate\n", result);
		return result;
	}

	copied = 0;
  	// This uses the non-blocking TFTP functions, but in a blocking
  	// manner.  It would be easier to use tftp_exec(), but this
  	// doesn't return the server error message.
	while ((result = tftp_tick(&ts)) >= 0)
	{
		if (ts.buf_used)
		{
			// buTempWrite is non-blocking, so it may take multiple calls to
			// complete the write.
			offset = 0;
			while (offset < ts.buf_used)
			{
				result = buTempWrite( &buffer[offset], ts.buf_used - offset);
				if (result == -EBUSY)
				{
					// resources busy, try again without any changes
				}
				else if (result < 0)
				{
					printf( "Error %d writing firmware to temp location.\n", result);
	            sock_close( &demosock);
	            break;
				}
				else
				{
					offset += result;
				}
			}

			copied += ts.buf_used;
			printf( " %lu bytes received\r", copied);
			ts.buf_used = 0;
		}
		if (!result)
		{
			// this was the last block of data, exit
			break;
		}
	}
	printf( "\n");

	// close firmware, if it's still open
   while (buTempClose() == -EBUSY);

	switch (result)
	{
	   case 0:
	      printf( "Download completed, attempting to install.\n");
	      result = install_firmware();
	      break;

	   case -5:
	      printf( "ERROR: Download completed, but truncated.\n");
	      break;

		case -2:
			printf( "ERROR: Could not contact remote host or lost contact.\n");
			break;

	   case -3:
	      printf( "ERROR: Download timed out.\n");
	      break;

	   case -1:
         // tftp_tick() puts error messages in ts.file
         printf( "  Message from server: %s\n", ts.file);
         // fall-through to default case, that download failed

	   default:
	      printf( "ERROR: Download failed (code %d)\n", result);
	}

   #ifdef BU_TEMP_USE_DIRECT_WRITE
      if (result && result != -ENODATA)
      {
         printf( "attempting to restore boot firmware from RAM\n");
         // There was an error downloading or installing the firmware,
         // so we need to restore the boot firmware image from the copy
         // of the program running in RAM.
         result = buRestoreFirmware( 0);
         if (result)
         {
            printf( "error %d attempting to restore firmware\n", result);
            // At this point, the firmware stored on the boot flash is
            // corrupted and cannot be restored.
         }
         else
         {
            printf( "restore complete\n");
         }
      }
   #endif

   return result;
}

int main()
{
	int result;
	char url[128];
	char localfile[128];

	printf( "Firmware Download Sample (TFTP)\n\n");

	printf( "Initializing TCP/IP stack...\n");

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

#ifdef BU_TEMP_USE_FAT
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
#endif

	printf( "Press any key to download\n\t%s from %s\n", TFTP_FILE, TFTP_SERVER);
	getchar();
	printf( "Connecting...\n");

	result = tftp_and_install( TFTP_SERVER, TFTP_FILE);

	if (result)
	{
		exit( result);
	}

	return 0;
}
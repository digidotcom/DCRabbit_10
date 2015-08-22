/*
	ftp2fat.c
	Digi International, Copyright © 2008.  All rights reserved.

	Description
	===========
	This sample program demonstrates the use of the FTP client library and
	ftp2fat helper library to copy files from a remote ftp server and save
	them to the FAT filesystem on the Rabbit.

	It requires a Rabbit with mass storage compatible with the FAT library,
	such as the RCM4000 with NAND flash; RCM4200, RCM4400W, RCM5400W and
	BL4S100 series boards with serial flash; or the RCM4300 series with an
	SD card.

	Instructions
	============
	Run sample on a Rabbit with an Internet connection and mass storage.  After
	downloading a pre-set file (defined by the macros DL_URL and DL_FILE), the
	sample will prompt you to enter a URL and local filename to save to.

	Note that the Rabbit's FAT filesystem doesn't support long filenames, only
	DOS-style "8.3" filenames (a 1 to 8 character filename, optionally followed
	by a dot and 1 to 3 character extension).

	When you're done running the sample, enter a blank line for the URL and the
	sample will unmount the fat filesystem and exit.

*/

///// Configuration Options /////

// Define DL_URL and DL_FILE In your project defines, or modify the macro
// declarations shown here.
#ifndef DL_URL
	#define DL_URL "ftp://anonymous:guest@ftp1.digi.com/support/documentation/" \
	                                                           "DC1060_notes.txt"
#endif
#ifndef DL_FILE
	#define DL_FILE "a:/dc1060.txt"
#endif

// define FTP2FAT_VERBOSE to turn on verbose output from the ftp2fat library
//#define FTP2FAT_VERBOSE

// define FTP_VERBOSE to turn on verbose output from the FTP client library
//#define FTP_VERBOSE

// define USE_PASSIVE to use passive-ftp as opposed to active FTP.  Usually a
// good idea, especially when connecting through a firewall.
#define USE_PASSIVE

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// increase maximum filename length
#define FTP_MAX_FNLEN 48

// Set FAT library to blocking mode (not required for this sample)
//#define FAT_BLOCK

// Set FAT library to use forward slash as directory separator
#define FAT_USE_FORWARDSLASH

///// End of Configuration Options /////

// Set a default of declaring all local variables "auto" (on stack)
#class auto

// by default, compile functions to xmem
#memmap xmem

// Load the TCP/IP networking library
#use "dcrtcp.lib"

////////////  end of configuration options ////////////////


// Load the FAT filesystem library
#use "fat16.lib"

#ifdef USE_PASSIVE
	#define PASSIVE_FLAG  FTP_MODE_PASSIVE
#else
	#define PASSIVE_FLAG  0
#endif

#use "ftp_client.lib"

// library to download file from FTP server and save on FAT filesystem
#use "ftp2fat.lib"

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

// calculate percentage of numerator/denominator
int percent( unsigned long numerator, unsigned long denominator)
{
	if (!denominator)
	{
		// error, division by zero
		return -EINVAL;
	}

	if (numerator > 0xFFFFFFFFul / 100)
	{
		// fraction will overflow if we multiply numerator by 100, so divide
		// denominator by 100 instead.
		return (int) (numerator / (denominator / 100));
	}
	else
	{
		return (int) (numerator * 100 / denominator);
	}
}

// The FTP client has it's own sockets for the control and data connections.
int download( char *url, char *file)
{
	ftp2fat_t	dl;
	int			result;

   result = ftp2fat_init( &dl, url, file);
	if (result)
	{
      printf( "couldn't initiate download (error %d)\n", result);
      printf( "ftp_last_code() result is %d.\n", ftp_last_code());
	}
	else
	{
	   while ((result = ftp2fat_tick (&dl)) == -EBUSY)
	   {
	      if (dl.filesize)
	      {
	         printf (" %lu/%lu (%u%%)\r", dl.bytesread, dl.filesize,
	            percent( dl.bytesread, dl.filesize));
	      }
	      else
	      {
	         printf (" %lu bytes read\r", dl.bytesread);
	      }
	   }
	   if (result)
	   {
	      printf( "download canceled (error %d)\n", result);
	      printf( "ftp_last_code() result is %d.\n", ftp_last_code());
	   }
		else
		{
		   printf( "download complete, %lu bytes\n", dl.bytesread);
		}
	}

   return result;
}

void main()
{
	int result;
	char url[128];
	char localfile[128];

	printf ("ftp2fat sample\n\n");

	printf ("Initializing TCP/IP stack...\n");
	sock_init_or_exit(1);

	// Auto-mount the FAT filesystem
	printf ("mounting FAT partitions\n");
   do {
	   result = fat_AutoMount(FDDF_USE_DEFAULT);
   } while (result == -EBUSY);
   if (result == -EIO || result == -ENOMEDIUM)
   {
		printf("Fatal device initialization error!  Exiting now.\n");
		exit(result);
   }

	printf ("Press any key to download\n");
	printf ("   %s\n", DL_URL);
	printf (" to\n");
	printf ("   %s\n", DL_FILE);
	while (!kbhit())
	{
		tcp_tick( NULL);
	}
	getchar();

	download( DL_URL, DL_FILE);

	printf ("\n\nNow you try...  Enter a URL and filename to download to.\n");
	printf ("Try downloading\n");
	printf ("   ftp://anonymous:guest@ftp1.digi.com/support/documentation/DC10");
	printf ("64_notes.txt\n to\n   a:/dc1064.txt\n");
	while (1)
	{
	   printf ("\nEnter URL to download (blank to exit and unmount fs)\n]>");
      while (! getswf( url))
      {
      	tcp_tick( NULL);
      }
	   if (! *url)
	   {
			break;
	   }
	   printf ("Enter local files in the format 'a:/filename.ext'.\n");
	   printf ("Must be 1 to 8 character name, "\
	   						"followed by dot and 0 to 3 character extension.\n");
	   printf ("Enter local file to save as: ");
	   gets (localfile);

		download( url, localfile);
	}

	printf ("sample done, unmounting FAT partitions\n");
	unmount_all();

}
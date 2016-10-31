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
	http2fat.c

	Description
	===========
	This sample program demonstrates the use of the HTTP client library and
	http2fat helper library to copy files from a remote web server and save
	them to the FAT filesystem on the Rabbit.

	It requires a Rabbit board with already-formatted mass storage compatible
	with the the FAT library, such as RCM4000 boards series with NAND flash;
	RCM4200, RCM4400W, RCM5400W or BL4S100 series boards with serial flash; or
	RCM4300 series boards with an SD card.

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
	#define DL_URL		"http://ftp1.digi.com/support/documentation/DC1062_notes.txt"
#endif
#ifndef DL_FILE
	#define DL_FILE	"a:/dc1062.txt"
#endif

// define HTTPC_VERBOSE to turn on verbose output from the HTTP client library
//#define HTTPC_VERBOSE

// define HTTP2FAT_VERBOSE to turn on verbose output from the http2fat library
//#define HTTP2FAT_VERBOSE

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

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

// Load the FAT filesystem library
#use "fat16.lib"

// load the HTTP client library
#use "http_client.lib"

// library to download file from HTTP server and save on FAT filesystem
#use "http2fat.lib"

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


// It's safer to keep sockets as globals, especially when using uC/OS-II.  If
// your socket is on the stack, and another task (with its own stack, instead
// of your task's stack) calls tcp_tick, tcp_tick won't find your socket
// structure in the other task's stack.
// Even though this sample doesn't use uC/OS-II, using globals for sockets is
// a good habit to be in.
tcp_Socket demosock;

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

int download( char *url, char *localfile)
{
	http2fat_t	dl;
	int 			result;

	// initialize tcp_Socket structure before use
	memset( &demosock, 0, sizeof(demosock));

   result = http2fat_init (&dl, &demosock, url, localfile);

   if (result)
   {
      printf ("couldn't initiate download (error %d)\n", result);
      printf("   %ls\n", strerror(result));
   }
   else
   {
      while ((result = http2fat_tick (&dl)) == -EBUSY)
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
         printf ("download canceled (error %d)\n", result);
      }
      else
      {
         printf ("download complete, %lu bytes\n", dl.bytesread);
      }
   }

   return result;
}

void main()
{
	int result;
	char url[128];
	char localfile[128];

	printf ("http2fat sample\n\n");

	printf ("Initializing TCP/IP stack...\n");
	sock_init_or_exit(1);

	// Auto-mount the FAT filesystem
	printf ("mounting FAT partitions\n");
	do
	{
		result = fat_AutoMount(FDDF_USE_DEFAULT);
	} while (result == -EBUSY);
	// Verify that PARTITION_A (i.e partition 0 on device 0) is mounted. Note
	//  that even if PARTITION_A is successfully mounted, fat_AutoMount() may
	//  still report other errors, e.g. failure to mount a different partition
	//  because it is not FAT-formatted.
	if (NULL == PARTITION_A)
	{
		if (result)
		{
			// Report the fatal error result.
			printf("Error: fat_AutoMount() reports %ls.\n", strerror(result));
		}
		// This sample program cannot succeed without access to PARTITION_A.
		printf("The required PARTITION_A was not mounted; exiting now.\n");
		exit(result);
	}

	printf ("Press any key to download\n\t%s\n", DL_URL);
	printf ("\tto %s\n", DL_FILE);
	while (!kbhit())
	{
		tcp_tick( NULL);
	}
	getchar();

	download( DL_URL, DL_FILE);

	printf ("\n\nNow you try...  Enter a URL and filename to download to.\n");
	printf ("Try downloading http://www.digi.com/images/nav-logo-digi.png to\n");
	printf ("a:/logo.png\n");
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
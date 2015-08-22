/*
  FAT_SETUP.C
  Digi International, Copyright (c) 2007.  All rights reserved.

  Sample program to demonstrate use of ZSERVER.LIB and FAT
  filesystem functionality.

  This is the first of a 2-part sample.  The other part (run next)
  is FAT_SERVE.C, or you can run FAT_SERVE2.C (which adds access
  control), or you can run samples\tcpip\http\ssi2_fat.c which adds
  FAT access to the ssi2.c sample.

  This sample copies some #ximported files into the FAT filesystem.
  You should make sure that the FAT filesystem is already formatted
  (see the FAT samples for how to do this).

*/


// Necessary for zserver.
#define FAT_USE_FORWARDSLASH

/*
 *  This value is required to enable FAT blocking mode for ZServer.lib
 */
#define FAT_BLOCK

#use "fat16.lib"

#define INPUT_COMPRESSION_BUFFERS 4
#use "zimport.lib"

#define SSPEC_ALLOW_ANONYMOUS_WRITE		// Required for this sample, since using anonymous user
#use "zserver.lib"

// Call these files into program flash (xmem).  They will be copied to
// the FAT filesystem.
#ximport "samples/tcpip/http/pages/fatindex.html"        static_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"          rabbit1_gif
#zimport "samples/tcpip/http/pages/alice.html"           alice_html
#ximport "samples/tcpip/http/pages/alice-rabbit.jpg"     alice_jpg
#ximport "samples/tcpip/http/pages/ssi2_fat.shtml"       ssi2_shtml
#ximport "samples/tcpip/http/pages/ledon.gif"      ledon_gif
#ximport "samples/tcpip/http/pages/ledoff.gif"     ledoff_gif
#ximport "samples/tcpip/http/pages/button.gif"     button_gif
#ximport "samples/tcpip/http/pages/showsrc.shtml"  showsrc_shtml
#ximport "samples/tcpip/http/ssi2.c"               ssi2_c

// This shows the new way of initializing the flashspec.  No need to
// worry about read/write permissions, since we are not serving these
// in this sample.
SSPEC_RESOURCETABLE_START
   SSPEC_RESOURCE_XMEMFILE("/static.html",   static_html),
   SSPEC_RESOURCE_ZMEMFILE("/alice.html",		alice_html),		// Compressed
   SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif",   rabbit1_gif),
   SSPEC_RESOURCE_XMEMFILE("/alice.jpg",     alice_jpg),
   SSPEC_RESOURCE_XMEMFILE("/ssi2.ssi",      ssi2_shtml),
   SSPEC_RESOURCE_XMEMFILE("/ledon.gif",     ledon_gif),
   SSPEC_RESOURCE_XMEMFILE("/ledoff.gif",    ledoff_gif),
   SSPEC_RESOURCE_XMEMFILE("/button.gif",    button_gif),
   SSPEC_RESOURCE_XMEMFILE("/source.ssi",    showsrc_shtml)
SSPEC_RESOURCETABLE_END


ServerContext dummy_context;

void copy(char * from, char * to)
{
   char buf[512];		// 512-byte chunks gives best efficiency
   int len, len2, wr;
	int handle, handle2;
   long total;
   long filelen;
   long alloc;

   printf("\nCopying from %s to %s...\n", from, to);

   handle = sspec_open(from, &dummy_context, O_READ, 0);
   if (handle < 0) {
   	printf("Could not open %s for reading, rc=%d\n", from, handle);
      return;
   }
   filelen = sspec_getlength(handle);
   if (filelen < 0)
   	filelen = 0;	// Don't know size of compressed files
   // Create with pre-allocation in terms of 1k blocks
   alloc = (word)(filelen >> 10);
   if (alloc > 20)
   	alloc = 20;	// max 20k
   handle2 = sspec_open(to, &dummy_context, O_WRITE|O_CREAT, (word)(filelen >> 10) );
   if (handle2 < 0) {
   	printf("Could not open %s for writing, rc=%d\n", to, handle2);
      sspec_close(handle);
      return;
   }
   total = 0;
	while (sspec_read(handle, NULL, 0)) {		// While not EOF
		len = sspec_read(handle, buf, sizeof(buf));
      if (len > 0) {
      	wr = 0;
      	while (wr < len) {
	         len2 = sspec_write(handle2, buf+wr, len-wr);
	         if (len2 < 0) {
	            printf("Could not write to %s, only wrote %ld bytes, rc=%d\n",
               		to, total, len2);
	            goto _close;	// double break
	         }
	         else {
	            total += len2;
               wr += len2;
            }
         }
      }
   };
_close:
   sspec_close(handle2);
   sspec_close(handle);
   printf("...done (copied %ld bytes into preallocation of %ld)\n", total, filelen);
}


int main()
{
	char buf[80];
   int rc;

   // Set up a dummy server context, so we can use zserver functions to do the copying.
   dummy_context.userid = -1;			// No userid
   dummy_context.server = SERVER_ANY;// All servers
   dummy_context.rootdir = "/";		// Access from root
   strcpy(dummy_context.cwd, "/");
   dummy_context.dfltname = NULL;	// No default filename


	printf("Initializing filesystem...\n");
	// Note: sspec_automount automatically initializes all known filesystems.  We assume
   // that the first partition on the device will be a valid FAT12 or FAT16 partition
   // which will be mounted on '/A'.
   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
   if (rc)
   	printf("Failed to initialize, rc=%d\nProceeding anyway...\n", rc);

	//    "flashspec" name   FAT file name
   //   -----------------   ---------------
   copy("/static.html",     "/A/static.htm");
   printf("\n[This one will take a while...]\n");
   copy("/alice.html",      "/A/alice.htm");
   copy("/rabbit1.gif",     "/A/rabbit1.gif");
   copy("/alice.jpg",       "/A/alice.jpg");
   copy("/ssi2.ssi",        "/A/ssi2.ssi");
   copy("/ledon.gif",       "/A/ledon.gif");
   copy("/ledoff.gif",      "/A/ledoff.gif");
   copy("/button.gif",      "/A/button.gif");
   copy("/source.ssi",      "/A/source.ssi");
   printf("All done.\n");

   // You should always unmount the device on exit to flush cache entries.
	fat_UnmountDevice(sspec_fatregistered(0)->dev);
	return 0;
}
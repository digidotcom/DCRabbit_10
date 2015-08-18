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
/**************************************************************
  FAT_HOT_SWAP_SD.c

  Requires a core module with SD interface (RCM4300 or RCM4310).

  Demonstrate hot swapping an SD card.

  Run program and hit any keyboard key while focus is on stdio
  window to signal a hot swap. Pull card and insert new or same
  one on prompt.

  Don't pull card if not prompted!

  If hot swapping is to be performed, it must be done while the
  SD card is unmounted. This sample unmounts the device and waits
  for a new one when it detects a keyboard hit.

**************************************************************/

#memmap xmem

#define  GREEN    "\x1b[32m"    // Foreground colors for printf
#define  RED      "\x1b[31m"
#define  BLUE     "\x1b[34m"
#define  BLACK    "\x1b[30m"

#define  FILESIZE 256
#define  NFILES   30
#define  NTESTS   20
#define  NRETRY   50

//**** Define these macros for library debugging info to display
//***  in the STDIO window
//#define FAT_VERBOSE
//#define FAT_DEBUG
//#define FAT_HOTSWAP_VERBOSE

#define FAT_BLOCK          // Set FAT library to blocking mode

#use "fat16.lib"

#ifndef FAT_ALLOW_HOTSWAP
   	#error "Board type does not support hot swapping."
#endif

int main()
{
   FATfile my_file;
   static char fbuff[FILESIZE];
   auto int retries, rc, i, swapPending, j, nErrs;
   auto char filename[13], buf[6];
   auto int ntests;

   sdspi_initDevice(0,&SD_dev0);

   if(!sdspi_debounce(&SD[0])){
      printf("\n\n\n INSERT SD CARD");
   }

   swapPending = 2;
   ntests = nErrs = 0;

   while(ntests < NTESTS && !nErrs)
   {
      if(swapPending==2){

         // Busy wait while card not detected
         while(!sdspi_debounce(&SD[0]));

         // Mount SD card
         retries = 0;

         for (rc = 1; rc; ) {  // Retry loop
            rc = fat_AutoMount(FAT_SD_DEVICE | FDDF_MOUNT_PART_0 |
                     FDDF_COND_DEV_FORMAT | FDDF_COND_PART_FORMAT
                      );

            if(rc)   // If failed to mount SD card
            {
	            retries++;
	            printf("%sERROR: fat_AutoMount() returned  %d, Retrying\n%s",
	                      RED, rc, BLACK);

	             if(retries < NRETRY)
	             {
	               fatftc_flushall(FTC_WAIT | FTC_PURGE);
	               _fatftc_init();
	             }
	             else {
	                nErrs++;
	                break;
	             }
	         }
         }

         if (rc) {
            break;
         }
         else
         {
            printf("\n%sSD card mounted...\n", BLUE);
            printf("\nHit KB key when ready to swap cards\n%s", BLACK);
            swapPending = 0;
         }
      }

      if(swapPending==1){

         //*** UnmountDevice will flush SD card with WTC_PURGE flag
         rc = fat_UnmountDevice(FAT_SD_PART->dev);
         if(rc)
         {  // Unmount failure
            printf("%sERROR: fat_UnmountDevice() returned %d.\n%s",
                     RED, rc, BLACK);
            nErrs++;
            continue;  // Abort while loop
         }
         else {
            printf("\n%sCard unmounted, Switch SD cards now\n%s",
                    RED, BLACK);
         }

         // Busy wait while card detected
         while(sdspi_debounce(&SD[0]));

         printf("\n%sCard Removed, put new or same card in%s\n\n",
                       RED, BLACK);
         swapPending = 2;   // Ready to auto-mount
      }
      else   // Test file operations
      {
         for(j=0; j< NFILES ; j++){ // use NFILES files per device

            // Key hit while focus on stdio window
            if(kbhit()){
               swapPending = 1;
               getchar();  // Clear key hit
               break;
            }

            strcpy(filename, "file");
            itoa(j, buf);
            strcat(filename,buf);
            strcat(filename,".txt");

            rc = fat_Open(FAT_SD_PART, filename,
                    FAT_FILE, FAT_CREATE, &my_file, NULL);
            if (rc){
               printf("%sERROR: fat_Open() returned result code %d.\n%s",
                       RED, rc, BLACK);
               nErrs++;
               break;  // Abort for loop
            }
              // WRITE TO THE FILE
            memset(fbuff, j, FILESIZE);
            if(rc=fat_Seek(&my_file, 0, SEEK_SET )){
               printf("%sERROR: fat_Seek() returned result code %d.\n%s",
                      RED, rc, BLACK);
               nErrs++;
               break;  // Abort for loop
            }
            rc = fat_Write(&my_file,fbuff,FILESIZE);
            if(rc<0){
               printf("%sERROR: fat_Write() %d \n%s",RED,rc,BLACK);
               nErrs++;
               break;  // Abort for loop
            }
            memset(fbuff,0,FILESIZE);

            if(rc=fat_Seek(&my_file, 0, SEEK_SET )){
               printf("%sERROR: fat_Seek() returned result code %d.\n%s",
                     RED, rc, BLACK);
               nErrs++;
               break;  // Abort for loop
            }

            // READ A FILE
            rc=fat_Read(&my_file,fbuff,FILESIZE);
            if(rc<0){
               printf("%sERROR: fat_Read() returned result code %d.\n%s",
                     RED, rc, BLACK);
               nErrs++;
               break;  // Abort for loop
            }

            for(i=0; i < FILESIZE; i++){
               if( fbuff[i] != j){
                  printf("%sERROR: fat_Read() bad value\n%s",RED,BLACK);
                  j = 0;
                  nErrs++;
                  break; // Abort for loop
               }
            }
            rc = fat_Close(&my_file);
            if(rc){
               printf("%sERROR: fat_Close() returned result code %d.\n%s",
                     RED, rc, BLACK);
               nErrs++;
               break;  // Abort for loop
            }
            rc = fat_Delete( FAT_SD_PART, FAT_FILE, filename);
            if(rc){
               printf("%sERROR: fat_Delete() returned result code %d.\n%s",
                     RED, rc, BLACK);
               nErrs++;
               break;  // Abort for loop
            }
         } // End for j

         ntests++;

         if(swapPending) continue; // Start next while loop iteration

         if(j !=NFILES){
            printf("%sFAT error \n%s", RED, BLACK);
         }
      } // End else
   }  // End while

   if(!nErrs){
      printf("\n%s SUCCESS \n%s", BLUE, BLACK);
   }
   else {
      printf("\n%s %d FAT Errors occurred \n%s", RED, nErrs, BLACK);
   }

   // Unmount and loop so we don't wear out flash if program left
   //  running in run mode
   printf("\n%sUnmounting the SD Card, please wait.\n%s", RED, BLACK);
   fat_UnmountDevice(FAT_SD_PART->dev);
   printf("\n%sSD Card unmounted, press any key to exit.\n%s", RED, BLACK);
   while(1) if(kbhit()) break;
}



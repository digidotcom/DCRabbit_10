/*****************************************************************************
sdflash_inspect.c
 Copyright (c) 2007 Digi International Inc., All Rights Reserved

This program is a handy utility for inspecting the contents of a SD card.
When it starts it attempts to initialize a SD card on serial port B. If
one is found then the user can perform five different commands:
'p' prints out the contents of a specified page on the SD flash card
'r' prints out the contents of a range of pages on the SD flash card
'c' clears (sets to zero) all of the bytes in a specified page
'f' sets all bytes on the specified page to the given value
't' writes user specified text into a selected page

The program prints out a single line for a page if all bytes in the page are
set to the same value.  Otherwise it prints a hex/ascii dump of the page.

*****************************************************************************/

//#define SDFLASH_DEBUG

#use "sdflash.lib"

char flash_buf[512];

// Gets positive numeric input from keyboard and returns int value when enter key
// is pressed.  Returns -1 if non-numeric keys are pressed. (Allows backspace)
long input_number()
{
	long number;
   char inchar;

   number = 0L;
   while(1)
   {
   	inchar = getchar();
      putchar(inchar); //echo input
      if(inchar == '\n' || inchar == '\r')
      {
      	return number;
      }
      else if(inchar == '\b')
      {
         //backspace
      	number = number / 10;
      }
      else if(inchar >= '0' && inchar <= '9')
      {
      	number = number*10 + (inchar - '0');
      }
      else
      {
      	//bad input
         return -1;
      }
   }	// end of while
}
void print_command()
{
   printf("\npress c to clear a page\n");
   printf("press p to print a page\n");
   printf("press r to print a range of pages\n");
   printf("press f to fill a page with a specified value\n");
   printf("press t to write text into a specified page\n");
}
int main()
{
	char ascii_buffer[17];
   char fbyte, inchar;
   int i, j, k, value, rc, error_code;
   long pagenum, start, end;
   char linebuf[80], *p, *buf, ch;
   sd_device *dev;

   dev = &SD[0];
	if (rc = sdspi_initDevice(0, &SD_dev0))
   {
   	printf("Flash init failed (%d): %ls\n\n", rc, strerror(rc));
      exit(rc);
   }
   else
   {
   	printf("Flash init OK\n");
      printf("# of blocks: %ld\n", dev->sectors);
      printf("Size of block: 512\n\n");
   }

   while(1)
   {
      print_command();
   	inchar = getchar();
      if(inchar == 'c')
      {
         printf("Page number to clear?");
         pagenum = input_number();
 			if(pagenum >= 0 && pagenum < dev->sectors)
         {
         	printf("\nClearing page %ld\n", pagenum);
	         memset (flash_buf, 0, 512);
            if (rc = sdspi_write_sector(dev, pagenum, flash_buf))
            {
               printf("\nSD write error (%d): %ls\n", rc, strerror(rc));
            }
         }
         else
         {
         	printf("\nERROR: invalid page number\n");
         }
      }  //end if(inchar =='c')
      else if((inchar == 'p') || (inchar == 'r'))
      {
         if (inchar == 'p') {
            // Use start for page to print
            printf("Page number to print out?");
         	start = input_number();
            // Check that it is a valid page
 		   	if(start < 0 || start >= dev->sectors) {
               printf("\nERROR: invalid page number.\n");
               continue;
            }
            // Set single page range for 'p' command
            end = start;
         }
         else {
            printf("Starting page number to print out?");
         	start = input_number();
            // Check that it is a valid page
 		   	if(start < 0 || start >= dev->sectors) {
               printf("\nERROR: invalid page number.\n");
               continue;
            }
            printf("\nEnding page number to print out?");
            end = input_number();
 		   	if(end < start || end >= dev->sectors) {
               printf("\nERROR: invalid page number.\n");
               continue;
            }
         }
         // Loop through range of pages (range of 1 page for 'p' command)
         for (pagenum = start; pagenum <= end; pagenum++)
         {
	     		printf("\nPage %ld", pagenum);
            if (rc = sdspi_read_sector(dev, pagenum, flash_buf))
            {
               printf("\nSD read error (%d): %ls\n", rc, strerror(rc));
               break;
            }

            // Test if entire buffer filled with a single value
            buf = flash_buf;
            for (j = k = 0, ch = *buf; j < 512; j++) {
               if(ch != *buf++) {
                  k = 1;
                  break;
               }
            }
            // See if page is all the same value
            if (k) {
               printf("\n");  // No, drop through to print data
            }
            else {  // Yes, print out message instead
               printf("   ALL = 0x%02x\n", ch);
               continue;
            }
				ascii_buffer[16] = 0;
	         for (j = 0, buf = flash_buf; j < 512; j++)
            {
	         	if (j % 16 == 0)
               {
	         		p = linebuf;
	         		p += sprintf (p, "%04x: ", j);
	         	}
	         	fbyte = *buf++;

					if (j >= 512)
               {
						p += sprintf (p, "   ");
						ascii_buffer[j % 16] = ' ';
					}
               else
               {
	               p += sprintf (p, "%02x ", fbyte);
	               ascii_buffer[j % 16] = isprint (fbyte) ? fbyte : '.';
					}
	         	if (j % 16 == 15)
               {
	         		printf ("%s   %s\n", linebuf, ascii_buffer);
	         	}
	         }
         }
      }  // end if((inchar =='p') || (inchar == 'r'))
      else if(inchar == 'f')
      {
         printf("Page number to fill with specified value?  ");
         pagenum = input_number();
 			if(pagenum >= 0 && pagenum < dev->sectors)
         {
         	printf("\nPage %ld\n", pagenum);
         	printf("Enter fill value  "  );
         	value = (int)input_number();
            printf("\nValue is %d dec is %02x hex", value,value);
         	printf("\nFilling page %ld with value %02x hex\n", pagenum,value);
	         memset (flash_buf, value, 512);
            if (rc = sdspi_write_sector(dev, pagenum, flash_buf))
            {
               printf("\nSD write error (%d): %ls\n", rc, strerror(rc));
            }
         }
         else
         {
         	printf("\nERROR: invalid page number\n");
         }
 		}	// end of if(inchar == 'f')
       else if(inchar == 't')
      {
         printf("page number in which to write text? ");
         pagenum = input_number();
 			if(pagenum >= 0 && pagenum < dev->sectors)
         {
         	printf("\nPage %ld\n", pagenum);
         	printf("Enter character string followed by RETURN  \n"  );
				gets (flash_buf);
            printf("Storing the following text ==> %s \n",flash_buf);
            if (rc = sdspi_write_sector(dev, pagenum, flash_buf))
            {
               printf("\nSD write error (%d): %ls\n", rc, strerror(rc));
            }
         }
         else
         {
         	printf("\nERROR: invalid page number\n");
         }
 		}	// end of if(inchar == 't')
   }	// end of while
}





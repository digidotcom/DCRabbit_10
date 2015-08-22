/*****************************************************************************
sflash_inspect.c

COPYRIGHT - Digi International, Inc. (c) 2006 - 2008

This program is a handy utility for inspecting the contents of a serial
flash chip. When it starts it attempts to initialize a serial flash
chip on serial port C. If one is found then the user can perform five
different commands:
'p' prints out the contents of a specified page in the serial flash
'r' prints out the contents of a range of pages from the serial flash
'c' clears (sets to zero) all of the bytes in a specified page
'f' sets all bytes on the specified page to the given value
't' writes user specified text into a selected page

The program prints out a single line for a page if all bytes in the page are
set to the same value.  Otherwise it prints a hex/ascii dump of the page.

*****************************************************************************/

#use "BL4S1xx.lib"

#use "sflash.lib"

char flash_buf[1056];

// Gets positive numeric input from keyboard and returns int value when enter key
// is pressed.  Returns -1 if non-numeric keys are pressed. (Allows backspace)
int input_number()
{
	int number;
   char inchar;

   number = 0;
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
   printf("press t to write text into a specified page\n\n");
}
int main()
{
	char ascii_buffer[17];
   char fbyte, inchar;
   int i, j, k, pagenum, value, start, end;
   char linebuf[80], *p, *buf, ch;

   brdInit();
   sfspi_init();
	if (sf_init())
   {
   	printf("Flash init failed\n");
      exit(-1);
   }
   else
   {
   	printf("Flash init OK\n");
      printf("# of blocks: %d\n", sf_blocks);
      printf("size of block: %d\n\n", sf_blocksize);
   }
   print_command();

   while(1)
   {
   	inchar = getchar();
      if(inchar == 'c')
      {
         printf("page number to clear?");
         pagenum = input_number();
 			if(pagenum >= 0 && pagenum < sf_blocks)
         {
         	printf("\nClearing page %d\n", pagenum);
	         memset (flash_buf, 0, sf_blocksize);
	         sf_writeRAM(flash_buf, 0, sf_blocksize);
         	sf_RAMToPage(pagenum);
         }
         else
         {
         	printf("ERROR: invalid page number\n");
         }
      }  //end if(inchar =='c')
      else if((inchar == 'p') || (inchar == 'r'))
      {
         if (inchar == 'p') {
            // Use start for page to print
            printf("Page number to print out?");
         	start = input_number();
            // Check that it is a valid page
 		   	if(start < 0 || start >= sf_blocks) {
               printf("Invalid page number.\n\n");
               continue;
            }
            // Set single page range for 'p' command
            end = start;
         }
         else {
            printf("Starting page number to print out?");
         	start = input_number();
            // Check that it is a valid page
 		   	if(start < 0 || start >= sf_blocks) {
               printf("Invalid page number.\n\n");
               continue;
            }
            printf("\nEnding page number to print out?");
            end = input_number();
 		   	if(end < start || end >= sf_blocks) {
               printf("Invalid page number.\n\n");
               continue;
            }
         }
         // Loop through range of pages (range of 1 page for 'p' command)
         for (pagenum = start; pagenum <= end; pagenum++)
         {
	     		printf("\nPage %d", pagenum);
	  	    	sf_pageToRAM(pagenum);
	         sf_readRAM(flash_buf, 0, sf_blocksize);

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
				k = (sf_blocksize & 0xFFF0) + ((sf_blocksize & 0x000F) ? 16 : 0);
				ascii_buffer[16] = 0;
	         for (j = 0, buf = flash_buf; j < k; j++)
            {
	         	if (j % 16 == 0)
               {
	         		p = linebuf;
	         		p += sprintf (p, "%04x: ", j);
	         	}
	         	fbyte = *buf++;

					if (j >= sf_blocksize)
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
         printf("page number to fill with specified value?  ");
         pagenum = input_number();
 			if(pagenum >= 0 && pagenum < sf_blocks)
         {
         	printf("\nPage %d\n", pagenum);
         	printf("enter fill value  "  );
         	value = input_number();
            printf("\nValue is %d dec is %02x hex", value,value);
         	printf("\nFilling page %d with value %02x hex\n", pagenum,value);
	         memset (flash_buf, value, sf_blocksize);
	         sf_writeRAM(flash_buf, 0, sf_blocksize);
         	sf_RAMToPage(pagenum);
         }
         else
         {
         	printf("ERROR: invalid page number\n");
         }
 		}	// end of if(inchar == 'f')
       else if(inchar == 't')
      {
         printf("page number in which to write text? ");
         pagenum = input_number();
 			if(pagenum >= 0 && pagenum < sf_blocks)
         {
         	printf("\nPage %d\n", pagenum);
         	printf("enter character string followed by RETURN  \n"  );
				gets (flash_buf);
            printf("Storing the following text ==> %s \n",flash_buf);
	         sf_writeRAM(flash_buf, 0, sf_blocksize);
         	sf_RAMToPage(pagenum);
         }
         else
         {
         	printf("ERROR: invalid page number\n");
         }
 		}	// end of if(inchar == 't')
      print_command();
   }	// end of while
}





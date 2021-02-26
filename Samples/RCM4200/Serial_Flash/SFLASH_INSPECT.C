/*
  Original work Copyright (c) 2015, Digi International Inc.
  Copyright (c) 2021 Mircea Neacsu

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

/*****************************************************************************
sflash_inspect.c

This program is a handy utility for inspecting the contents of a serial
flash chip. It has been modified to work with serial flash functions included
in RCM42xx.LIB

User can perform a number of different commands:
'b' erases a block (8 pages)
'e' erases a page
'l' lists an address range
'p' prints out the contents of a specified page in the serial flash
'r' prints out the contents of a range of pages from the serial flash
'f' sets all bytes on the specified page to the given value
's' erases a sector (1024 pages)
't' writes user specified text into a selected page
'w' writes a pattern to an address range
'z' erases the whole chip

The program prints out a single line for a page if all bytes in the page are
set to the same value.  Otherwise it prints a hex/ascii dump of the page.

*****************************************************************************/

//#define RCM42XX_DEBUG
#use RCM42xx.lib

char flash_buf[1056];


void erase_flash ()
{
  int stat;
  long t;
  sf_erase_chip ();
  printf ("\n");
  t = SEC_TIMER;
  while (((stat = sf_read_status()) & SF_STATRDY_MASK) == 0)
  {
    if (SEC_TIMER > t+1)
    {
      printf (".");
      t = SEC_TIMER;
    }
  }
  printf ("Done!\n");
}

// Gets positive numeric input from keyboard and returns int value when enter key
// is pressed.  Returns -1 if non-numeric keys are pressed. (Allows backspace)
long input_number()
{
  long number;
  char inchar;

  number = 0;
  while(1)
  {
    inchar = getchar();
    putchar(inchar); //echo input
    fflush(stdout);
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
  }  // end of while
}

void print_command()
{
   printf("Valid commands are:\n"
          " B - earse block\n"
          " E - erase page\n"
          " L - list an address range\n"
          " P - print page\n"
          " R - print a range of pages\n"
          " F - fill a page with a specified value\n"
          " S - erase sector\n"
          " T - write text into a specified page\n"
          " W - write an address range\n"
          " Z - erase the whole chip\n\n");
}

void list_range (long addr, long count)
{
  char __far *buffer;
  long i, sz, page;
  int off;
  sz = count;
  buffer = (char __far *)_xalloc (&sz, 0, XALLOC_ANY);
  if (sz < count)
  {
    printf ("\nCannot allocated enough RAM buffer!!\n\n");
    return;
  }
  sf_read (addr, buffer, count);
  i = 0;
  page = addr / sf_rcm4200.pagesize;
  off = (int)(addr % sf_rcm4200.pagesize);
  printf ("\n%5ld - %04x :", page, off);
  while (i < count)
  {
    printf (" %02x", buffer[i++]);
    addr++;
    page = addr / sf_rcm4200.pagesize;
    off = (int)(addr % sf_rcm4200.pagesize);
    if (off % 16 == 0)
      printf ("\n%5ld - %04x :", page, off);
  }
  printf ("\n");
  xrelease ((long)buffer, sz);
}

void write_range (long addr, long count)
{
  char __far *buffer;
  long i, sz, page;
  int off;
  char chr;
  sz = count;
  buffer = (char __far *)_xalloc (&sz, 0, XALLOC_ANY);
  BitWrPortI (PEDR, &PEDRShadow, 0, 6);
  if (sz < count)
  {
    printf ("\nCannot allocate enough RAM buffer!!\n\n");
    return;
  }
  chr = 0;
  for (i=0; i<count; i++)
    buffer[i] = chr++;

  sf_write (addr, buffer, count);
  xrelease ((long)buffer, sz);
  printf ("\nDone!\n");
}

void print_pages (int start, int end)
{
  char ascii_buffer[17], fbyte;
  char linebuf[80], *p, *buf, ch;
  int pagenum, i, j , k;
  // Loop through range of pages (range of 1 page for 'p' command)
  for (pagenum = start; pagenum <= end; pagenum++)
  {
    printf("\nPage %d", pagenum);
    sf_read_page (pagenum, flash_buf);

    // Test if entire buffer filled with a single value
    buf = flash_buf;
    ch = *buf;
    k = 0;
    for (j = 0; j < sf_rcm4200.pagesize; j++)
    {
      if(ch != *buf++)
      {
        k = 1;
        break;
      }
    }
    // See if page is all the same value
    if (k)
    {
      printf("\n");  // No, drop through to print data
    }
    else
    {
      // Yes, print out message instead
      printf("   ALL = 0x%02x\n", ch);
      continue;
    }
    ascii_buffer[16] = 0;
    k = ((sf_rcm4200.pagesize>>4) +1) * 16;
    for (j = 0, buf = flash_buf; j < k; j++)
    {
      if (j % 16 == 0)
      {
        p = linebuf;
        p += sprintf (p, "%04x: ", j);
      }
      fbyte = *buf++;
      if (j < sf_rcm4200.pagesize)
      {
        p += sprintf (p, "%02x ", fbyte);
        ascii_buffer[j % 16] = isprint (fbyte) ? fbyte : '.';
      }
      else
      {
        p += sprintf (p, "   ");
        ascii_buffer[j % 16] = ' ';
      }
      if (j % 16 == 15)
      {
        printf ("%s   %s\n", linebuf, ascii_buffer);
      }
    }
  }
}

int main ()
{
  char fbyte, inchar, buf[256];
  char ascii_buffer[20], *pchr;
  int pagenum, start, end, i;
  long flash_sz, addr, length;
  brdInit();

  sf_reset ();

  sf_read_id (buf);
  printf ("Flash ID: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
  buf[0], buf[1], buf[2], buf[3], buf[4]);
  if (memcmp (buf, "\x1f\x28\x00\x01\x00", 5))
    printf ("\nUNEXPECTED FLASH ID!!!!\n");

  sf_read_security_reg (buf);
  printf ("Security register:\n");
  pchr = ascii_buffer;
  for (i=0; i<128; i++)
  {
    if (i%16 == 0 && i > 0)
    {
      *pchr = 0;
      printf (" %s\n", ascii_buffer);
      pchr = ascii_buffer;
    }
    printf (" %02x", buf[i]);
    *pchr++ = isprint(buf[i])? buf[i] : '.';
  }
  printf (" %s\n\n", ascii_buffer);
  print_command();

  while (1)
  {
    printf ("Command? ");
    inchar = getchar();
    putchar(inchar); //echo input
    inchar = tolower(inchar);
    printf ("\n");
    switch (inchar)
    {
    case 'b':
      printf("block number to erase?");
      pagenum = (int)input_number();
      if(pagenum >= 0 && pagenum < sf_rcm4200.pages/8)
      {
        printf("\nErasing block %d\n", pagenum);
        sf_erase_block (pagenum);
      }
      else
        printf("ERROR: invalid block number\n");
      break;

    case 'e':
      printf("page number to erase?");
      pagenum = (int)input_number();
      if(pagenum >= 0 && pagenum < sf_rcm4200.pages)
      {
        printf("\nErasing page %d\n", pagenum);
        sf_erase_page (pagenum);
        printf ("Done!\n");
      }
      else
        printf("ERROR: invalid page number\n");
      break;

    case 'l':
      printf("Starting address?");
      addr = input_number();
      printf ("\nNumber of bytes?");
      length = input_number ();
      list_range (addr, length);
      break;

    case 'p':
    case 'r':
      if (inchar == 'p')
      {
        // Use start for page to print
        printf("Page number to print out?");
        start = (int)input_number();
        // Check that it is a valid page
        if(start < 0 || start >= sf_rcm4200.pages)
        {
          printf("Invalid page number.\n\n");
          continue;
        }
        // Set single page range for 'p' command
        end = start;
      }
      else
      {
        printf("Starting page number to print out?");
        start = (int)input_number();
        // Check that it is a valid page
        if(start < 0 || start >= sf_rcm4200.pages)
        {
          printf("Invalid page number.\n\n");
          continue;
        }
        printf("\nEnding page number to print out?");
          end = (int)input_number();
        if(end < start || end >= sf_rcm4200.pages)
        {
          printf("Invalid page number.\n\n");
          continue;
        }
      }
      print_pages (start, end);
      break;

    case 'f':
      printf("page number to fill with specified value?  ");
      pagenum = (int)input_number();
      if(pagenum >= 0 && pagenum < sf_rcm4200.pages)
      {
        printf("\nPage %d\n", pagenum);
        printf("enter fill value (decimal)  "  );
        fbyte = (int)input_number();
        printf("\nFilling page %d with value 0x%02x\n", pagenum,fbyte);
        memset (flash_buf, fbyte, sf_rcm4200.pagesize);
        sf_write(pagenum * sf_rcm4200.pagesize, flash_buf, sf_rcm4200.pagesize);
      }
      else
        printf("ERROR: invalid page number\n");
      break;

    case 't':
      printf("page number in which to write text? ");
      pagenum = (int)input_number();
      if(pagenum >= 0 && pagenum < sf_rcm4200.pages)
      {
        printf("\nPage %d\n", pagenum);
        printf("enter character string followed by RETURN  \n"  );
        gets (flash_buf);
        printf("Storing the following text ==> %s \n",flash_buf);
        sf_write(pagenum * sf_rcm4200.pagesize, flash_buf, sf_rcm4200.pagesize);
      }
      else
        printf("ERROR: invalid page number\n");
      break;

    case 'w':
      printf("Starting address?");
      addr = input_number();
      printf ("\nNumber of bytes to write?");
      length = input_number ();
      write_range (addr, length);
      break;

    case 'z':
      erase_flash ();
      break;

    default:
      print_command();
      break;
    }
  }  // end of while
}





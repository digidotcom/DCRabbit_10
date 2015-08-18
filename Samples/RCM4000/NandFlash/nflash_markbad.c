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
/*******************************************************************************

	nflash_markbad.c

	This program is used with Rabbit core modules equipped with a supported
   NAND flash device.

	Description
	===========
	This program is a utility which can either scan a NAND flash device for
   any uncorrectable read errors and if found mark these erase pages as bad.
   Otherwise, the user can specify a sector on the device and the erase
   page which contains that sector will be run through an extended test
   involving writing several test patterns and verifying proper ECC read
   back to insure the erase page appears to be completely functional.
   If the erase page is found to have problems, it is marked as bad.
   Defining FULL_TEST will enable a full destructive pattern test of the
   entire NAND device.  This not only destroys all data, but can take a
   long time.

	When the program starts it attempts to initialize the user selected
   NAND flash device.  If initialization is successful, the user is then
   prompted for whether a scan should be performed or an extended page test.

	Instructions
	============
	1. Compile and run this program.
	2. Follow the prompts.

*******************************************************************************/

// These defines redirect run mode STDIO to serial port A at 57600 baud.
#define STDIO_DEBUG_SERIAL SADR
#define STDIO_DEBUG_BAUD 57600

// This define adds carriage returns ('\r') to each newline char ('\n') when
//  sending STDIO output to a serial port.
#define STDIO_DEBUG_ADDCR

// Uncomment to enable full destructive test mode across entire NAND device
//#define FULL_TEST

// Uncomment this define to force both run mode and debug mode STDIO to the
//  serial port specified above.
//#define STDIO_DEBUG_FORCEDSERIAL

#class auto
#if (_BOARD_TYPE_ & 0xFF00) == RCM4000
#use "rcm40xx.lib"	// sample library to use with this application
#endif

//#define NFLASH_VERBOSE
//#define NFLASH_DEBUG
#use "nflash.lib"	// base nand flash driver library
#use "rand.lib"

// local function prototypes
void fill_page(int);
int check_sector(long);
void mark_bad(long);
long input_number(void);

// global variables
far char buffer[512], spare[16], spare2[17];
const long test_pattern[8][2] = {
  0x00000000L, 0x00000000L,
  0x11224488L, 0xAA55AA55L,
  0xAA55AA55L, 0x11224488L,
  0x3366CC99L, 0x88112244L,
  0x44221188L, 0x99CC6633L,
  0x189463C1L, 0xA79B812EL,
  0x9B81A712L, 0xC118539BL,
  0xFFFFFFFFL, 0xFFFFFFFFL
};

// protected in separate battery backed /CS2 SRAM, when one is available
protected nf_device nandFlash;

int main()
{
	auto char inchar;
	auto int i, status, rc;
	auto long addr, blocknum, blockstart, blockend, blockinc, blockMax, sector;

	_sysIsSoftReset();	// restore any protected variables

	brdInit();
   srandf((unsigned long)SEC_TIMER);

	do {
	   printf("Which NAND flash device? (0 == soldered, 1 == socketed):  ");
	   inchar = getchar();
	   printf("%c\n", inchar);
   	switch (inchar) {
   	case '0':
		case '1':
			break;
		default:
			printf("Unknown selection, try again.\n");
   	}
   } while (('0' != inchar) && ('1' != inchar));

	if (nf_initDevice(&nandFlash, (int) (inchar - '0'))) {
   	printf("\nNAND flash init failed!\n");
      exit(-1);
   } else {
     	blockMax = nf_getPageCount(&nandFlash);
   	printf("\nNAND flash init OK.\n");
      printf("# of blocks: 0x%08LX\n", blockMax);
      printf("size of block: 0x%08LX\n\n", 1L << nandFlash.erasebitshift);
   }

	do {
#ifndef FULL_TEST
	   printf("S to Scan NAND flash or T to Test specific page (destructive): ");
#else
	   printf("S to Scan NAND flash or T to Test entire device (destructive): ");
#endif
	   inchar = getchar();
	   printf("%c", inchar);
   	switch (inchar) {
      case 's':
      case 't':
         inchar -= 32;
   	case 'S':
		case 'T':
         break;
		default:
			printf("\nUnknown selection, try again.\n");
   	}
   } while (('S' != inchar) && ('T' != inchar));

   if (inchar == 'S') {
      // Scan flash and mark any blocks with uncorrectable ECC errors as bad
	   for (blocknum = nandFlash.startblock; blocknum < blockMax; blocknum++)
	   {
	      blockstart = blocknum << nandFlash.erasebitshift;
	      blockend = blockstart + (1 << nandFlash.erasebitshift);

         if (_nf_deviceCheckBlock(&nandFlash, addr) == 0) {
	         // See if any read errors on this block
	         for (addr = blockstart; addr < blockend; addr += 512) {
	            status = check_sector(addr);
	            if (status == 3) {
	               mark_bad(blocknum);
	               break;
	            }
	         }

	         if (status != 3) {
	            // keep scan speed up by periodically reporting scanned blocks
	            if ((blocknum & 15L) == 15L) {
	               printf("Scanned block #0x%08lx.\r", blocknum);
	            }
	            continue;    // No read error, so continue to next block
	         }
         }
	   }
   }
   else {
      while (1) {
#ifndef FULL_TEST
	      // Get sector number to identify which block to perform expanded testing
	      printf("\nEnter sector number to test: ");
	      sector = input_number();

	      if (sector >= 0) {
            blocknum = sector >> (nandFlash.erasebitshift - 9);
#else
         printf("DESTRUCTIVE pattern test of ENTIRE flash, continue? (Y/N) ");
         inchar = getchar();
         printf("%c\n\n", inchar);
         if (inchar != 'Y' && inchar != 'y') {
            break;    // Exit if continue not confirmed
         }
         blockinc = 1 << nandFlash.erasebitshift;
         for (blocknum=nandFlash.startblock; blocknum < blockMax; blocknum += 4)
         {
#endif
            printf("\rTesting block #0x%06lx", blocknum);
	         blockstart = blocknum << nandFlash.erasebitshift;
	         blockend = blockstart + (blockinc << 2);  // Do 4 blocks each pass
            for (i = 0; i < 16; i++) {
	            // Erase blocks first (and wait for erasure to finish)
               for (addr = blocknum; addr < (blocknum + 4); addr++)
               {
                  if (_nf_deviceCheckBlock(&nandFlash,
                                  addr << nandFlash.erasebitshift) == 0)
                  {
	                  status = nf_eraseBlock(&nandFlash, addr);
	                  if (status < 0) {
	                     if (status == -3) {
	                        printf("\nBlock #0x%06lx is marked bad.\n", addr);
	                     }
	                     else {
	                        printf("\nBlock #0x%06lx does not erase.\n", addr);
	                     }
	                     break;
	                  }
                     nf_waitforstatus(&nandFlash, rc);
	                  if (rc & 1) {
	                     printf("\nErase failed on block #0x%06lx.\n", addr);
	                     mark_bad(addr);
	                     break;
	                  }
                  }
	            }

               // Setup fill pattern in buffers
               fill_page(i);

	            // Write pattern to all sectors on this block
	            for (addr = blockstart; addr < blockend; addr += 512) {
                  if (_nf_deviceCheckBlock(&nandFlash, addr) == 0) {
	                  // Write page to set bad block marker
	                  _nf_deviceWritePage(&nandFlash, buffer, spare2, addr);

	                  // Wait for write to complete
                     nf_waitforstatus(&nandFlash, rc);
	                  if (rc & 1) {
	                     printf("\nWrite failed on page #0x%08lx.\n", addr >> 9);
	                     mark_bad(addr);
	                     addr = (addr & (~(blockinc - 1))) + blockinc - 512;
	                  }
                  }
               }

	            // See if any read errors on this block
	            for (addr = blockstart; addr < blockend; addr += 512) {
                  if (_nf_deviceCheckBlock(&nandFlash, addr) == 0) {
	                  if (check_sector(addr) == 3) {
	                     mark_bad(addr);
	                     addr = (addr & (~(blockinc - 1))) + blockinc - 512;
	                  }
                  }
               }
            }
	      }
#ifndef FULL_TEST
         else {
            printf("\nInput error.\n");
         }
#else
         break;
#endif
      }
   }
}

// Fills buffer and spare2 with selected test pattern and proper ECC values
void fill_page(int index)
{
   int i, a, b;
   long far *ptr, *end;

   a = index & 1;
   b = a ^ 1;
   index >>= 1;
   // Write selected test pattern to buffer
   for (ptr = (long far *)buffer, end = ptr + 128, i = 0; ptr < end; i++, ptr++)
   {
      if (((i >> 3) & 1) == a) {
         *ptr = test_pattern[index][a] & (unsigned long)(randf()*0xFFFFFFFL);
      }
      else {
         *ptr = test_pattern[index][b] | (unsigned long)(randf()*0xFFFFFFFL);
      }
   }

   // Write test pattern to spare data (except bad block byte)
   *((long far *)spare2) = test_pattern[index][0];
   *((long far *)&spare2[4]) = test_pattern[index][1];
   spare2[5] = 0xFF;

   // calculate new ECC's from NAND flash buffer just filled
   *((long far *)&spare2[13]) = calculateECC256(buffer);
   *((long far *)&spare2[8])  = calculateECC256(buffer + 256);
   *((unsigned far *)&spare2[11]) = calculateECC8(spare2);
}


//Reads sector from given address and verifies all data can be corrected by ECC
int check_sector(long addr)
{
   auto int status;
   auto long oldECC0, oldECC1, newECC0, newECC1;
   auto unsigned secc1, secc2;

   // Read page to check for ECC errors
   status = _nf_deviceReadPage(&nandFlash,buffer,spare, addr);
   if (status) {
      printf("Error in reading the NAND device, scan aborted.\n");
      exit(0);
   }

   // get previous ECC's, stored in NAND flash page's "spare" data
   oldECC0 = *((long far *)&spare[13]) & 0x00FFFFFFL;
   oldECC1 = *((long far *)&spare[8]) & 0x00FFFFFFL;

   // calculate new ECC's from NAND flash page's main data just read
   newECC0 = calculateECC256(buffer);
   newECC1 = calculateECC256(buffer + 256);

   // check ECCs and correct data (or old ECC) if necessary
   status = chkCorrectECC256(buffer, &oldECC0, &newECC0);
   if (status == 3) {
      return 3;    // Read error found in block
   }
   status = chkCorrectECC256(buffer + 256,&oldECC1,&newECC1);
   if (status == 3) {
      return 3;    // Read error found in block
   }

   // calculate and check ECC's on NAND flash spare data just read
   secc1 = *((word __far *)(spare + 11));
   secc2 = calculateECC8(spare);
   status = chkCorrectECC8(spare, (word __far *)(&secc1), secc2);
   return status;
}


// Mark the given block as bad
void mark_bad(long addr)
{
   int sector;
   long blocknum, blockstart, blockend;

   blocknum = addr >> nandFlash.erasebitshift;
   // Read error on current block, mark it bad
   printf("\nBlock #0x%06lx has error at #0x%08lx, marking bad.\n",
              blocknum, addr);

   // Erase block first
   nf_eraseBlock(&nandFlash, blockstart);
   // Wait for erase to complete
   while (nf_isBusyStatus(&nandFlash));

   // Setup empty page with zeroed spare data and FF's for all ECC's
   _f_memset(buffer, 0xFF, 512);
   _f_memset(spare, 0, 8);
   _f_memset(spare+8, 0xFF, 8);
   blockstart = blocknum << nandFlash.erasebitshift;
   blockend = blockstart + (1 << nandFlash.erasebitshift);
   for (addr = blockstart; addr < blockend; addr += 512) {
      // Write page to set bad block marker
      _nf_deviceWritePage(&nandFlash, buffer, spare, addr);

      // Wait for write to complete
      while (nf_isBusyStatus(&nandFlash));
   }
}

// Gets positive numeric input from keyboard and returns long value when enter
// key is pressed.  Returns -1 if non-numeric keys are pressed.  (Allows
// backspace.)
long input_number()
{
	auto long number;
   auto char inchar;

   number = 0L;
   while (1) {
   	inchar = toupper(getchar());
      if (inchar == '\n' || inchar == '\r') {
         printf("\n");
      	return number;
      }
      else {
         printf("%c", inchar); //echo input
	      if (inchar == '\b') {
	         number = number / 10L;
	      }
	      else {
	         number *= 10L;
	         if (inchar >= '0' && inchar <= '9') {
	            number += inchar - '0';
	         }
	         else {
	            //bad input
	            return -1L;
	         }
	      }
      }
   }	// end of while
}




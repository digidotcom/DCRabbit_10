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

	nflash_scan.c

	This program is used with Rabbit core modules equipped with a supported
   NAND flash device.

	Description
	===========
	This program is a utility which will scan a NAND flash device for any
   bad block markers and (optional) uncorrectable read errors.  If found
   these markers or errors will be reported.  Bad block markers will also
   include the spare data area from the marked sectors within the block
   unless the bad block marks are across the entire block, which is the
   proper way for a bad block to be marked.  A properly marked bad block
   will display like this: 'Bad Block Marker on Block 0x4F, All Sectors'

	When the program starts it attempts to initialize the user selected
   NAND flash device.  If initialization is successful, the scan begins
   and the report is generated.

	Instructions
	============
	1. Compile and run this program.
	2. Follow the prompts.

*******************************************************************************/

// Uncomment this line to include uncorrectable read errors in the scan
//#define TEST_ECC

// These defines redirect run mode STDIO to serial port A at 57600 baud.
#define STDIO_DEBUG_SERIAL SADR
#define STDIO_DEBUG_BAUD 57600

// This define adds carriage returns ('\r') to each newline char ('\n') when
//  sending STDIO output to a serial port.
#define STDIO_DEBUG_ADDCR

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

// global variables

// protected in separate battery backed /CS2 SRAM, when one is available
protected nf_device nandFlash;

int main()
{
   static __far char buffer[512], spare[16], spare2[16];
	auto char inchar;
	auto int i, count, max, status;
	auto long addr, blocknum, blockstart, blockend, blockMax;
   auto long oldECC0, oldECC1, newECC0, newECC1;
   auto word secc1, secc2;

	_sysIsSoftReset();	// restore any protected variables

	brdInit();

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
      printf("size of block: 0x%08LX\n", 1L << nandFlash.erasebitshift);
   }

   count = 0;
   max = 1 << (nandFlash.erasebitshift - 9);
   for (blocknum = nandFlash.startblock; blocknum < blockMax; blocknum++)
   {
      blockstart = blocknum << nandFlash.erasebitshift;
      blockend = blockstart + (1 << nandFlash.erasebitshift);

      // See if any read errors on this block
		for (addr = blockstart; addr < blockend; addr += 512) {
         // Read page to check for ECC errors
#ifdef TEST_ECC
			status = _nf_deviceReadPage(&nandFlash, buffer, spare, addr);
#else
			status = _nf_deviceReadPage(&nandFlash, (char __far *)NULL,spare,addr);
#endif
         if (status) {
            printf("Error in reading the NAND device, scan aborted.\n");
            exit(0);
			}
	      secc1 = *((word __far *)(spare + 11));
	      secc2 = calculateECC8(spare);
	      status = chkCorrectECC8(spare, (word __far *)(&secc1), secc2);

			if (spare[5] != 0xFF) {
            if ((int)((addr - blockstart) >> 9) == count) {
               if (!count) {
                  _f_memcpy(spare2, spare, 16);  // Save first spare marker
               }
               if (++count >= max) {
                  printf("\nBad Block Marker on Block 0x%LX, All Sectors\n",
                           blocknum);
                  count = 0;
               }
            }
            else {
               printf("\nBad Block Marker on Block 0x%LX, Sector %d\n",
                       blocknum, (addr - blockstart) >> 9);
               _nf_print_data("\nSpare", addr >> 9, spare, 16);
            }
         }
         else {
            if (count) {
               printf("\n\nBad Block Marker on Block 0x%LX, Sector 0",
                       blocknum);
               if (--count) {
                  printf(" thru %d", count);
               }
               _nf_print_data("\n\nSpare", addr >> 9, spare2, 16);
               count = 0;
            }
	         if (status == 3) {
	            printf("\nBlock 0x%0LX, sector %d has uncorrectable read errors.\n",
	                      blocknum, (addr - blockstart) >> 9);
               _nf_print_data("\nSpare", addr >> 9, spare, 16);
	            continue;
	         }
         }

#ifdef TEST_ECC
			// get previous ECC's, stored in NAND flash page's "spare" data
			oldECC0 = *((long __far *)&spare[13]) & 0x00FFFFFFL;
			oldECC1 = *((long __far *)&spare[8]) & 0x00FFFFFFL;

			// calculate new ECC's from NAND flash page's main data just read
			newECC0 = calculateECC256(buffer);
			newECC1 = calculateECC256(buffer + 256);

			// check ECCs and correct data (or old ECC) if necessary
			status = chkCorrectECC256(buffer, &oldECC0, &newECC0);
			if (status == 3) {
      		// Read error on current block, mark it bad
      		printf("\nBlock 0x%0LX, sector %d has uncorrectable read errors.\n",
            		    blocknum, (addr - blockstart) >> 9);
            continue;
         }
			status = chkCorrectECC256(buffer + 256, &oldECC1, &newECC1);
			if (status == 3) {
      		// Read error on current block, mark it bad
      		printf("\nBlock 0x%0LX, sector %d has uncorrectable read errors.\n",
            		    blocknum, (addr - blockstart) >> 9);
	         continue;
         }
#endif
      }

		// keep erase speed up by only periodically reporting scanned blocks
		if ((blocknum & 15L) == 15L) {
			printf("Scanned block #0x%08lx.\r", blocknum);
		}
	}
}




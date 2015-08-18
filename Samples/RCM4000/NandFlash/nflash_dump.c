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

	nflash_dump.c

	This program is used with RCM4000 series controllers equipped with a
   supported NAND flash device.


	Description
	===========
	This program is a handy utility for dumping the non-erased content of a NAND
	flash device to STDIO, which may be redirected to a serial port.  When the
	program starts it attempts to initialize the user selected NAND flash device.
	If initialization is successful and the main page size is acceptable,
	non-erased (non 0xFF) NAND flash page content is dumped to STDIO.


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

// Uncomment this define to force both run mode and debug mode STDIO to the
//  serial port specified above.
//#define STDIO_DEBUG_FORCEDSERIAL

#class auto
#if (_BOARD_TYPE_ & 0xFF00) == RCM4000
#use "rcm40xx.lib"	// sample library to use with this application
#endif

// Caution:  If NFLASH_CANERASEBADBLOCKS is defined before NFLASH.LIB is used,
//           then the nand flash driver will allow bad blocks to be erased,
//           including the bad block marker itself.  Thereafter, data that is
//           stored in the unmarked bad block may not be recoverable.
//#define NFLASH_CANERASEBADBLOCKS	// to also allow reading initial bad blocks

// if NFLASH_USEERASEBLOCKSIZE is not defined, its value defaults to 1
//  0 == use 512 byte main data page size; 1 == use 16 KB main data page size
#define NFLASH_USEERASEBLOCKSIZE 0	// must use 512 byte main data page size

//#define NFLASH_VERBOSE
//#define NFLASH_DEBUG
#use "nflash.lib"	// base nand flash driver library

// local function prototypes
int non_erased(char far * dataBuffer, long dataSize);
int all_same(char far * dataBuffer, long dataSize);

// global variables
// protected in separate battery backed /CS2 SRAM, when one is available
protected nf_device nandFlash;


int main()
{
	static far char myMainBuffer[512];
	auto char inchar;
	auto int status;
	auto long bufSize, pagenum;
   auto unsigned secc1, secc2;

	_sysIsSoftReset();	// restore any protected variables

	brdInit();

	do {
		printf("Dump non-erased contents of which NAND flash device?\n");
		printf(" (0 == soldered, 1 == socketed)  ");
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
		printf("\nNAND flash init OK.\n");
		printf("# of blocks: 0x%08lx\n", nf_getPageCount(&nandFlash));
		printf("size of block: 0x%08lx\n", nf_getPageSize(&nandFlash));
	}

	bufSize = nf_getPageSize(&nandFlash);
	if ((512L < bufSize) || (0L > bufSize)) {
		printf("\nNAND flash page size is out of range (%ld)!\n", bufSize);
		exit(-2);
	}

	for (pagenum = 0; pagenum < nf_getPageCount(&nandFlash); ++pagenum) {
		status = nf_readPage(&nandFlash, myMainBuffer, pagenum);
		if (status) {
			printf("\nError, nf_readPage(%08lx) result is %d!\n", pagenum, status);
		}
		// only dump page's non-erased main data
		if (non_erased(myMainBuffer, nf_getPageSize(&nandFlash))) {
         if (all_same(myMainBuffer, nf_getPageSize(&nandFlash))) {
            printf("\nApplication's main data page (page %d):\n All %02x\n",
                    pagenum, myMainBuffer[0]);
         }
         else {
			   _nf_print_data("\nApplication's main", pagenum, myMainBuffer,
			              nf_getPageSize(&nandFlash));
         }
		}
		// only dump page's non-erased spare data
		if (non_erased(nandFlash.sparebuffer, nandFlash.sparesize)) {
	      secc1 = *((word __far *)(nandFlash.sparebuffer + 11));
	      secc2 = calculateECC8(nandFlash.sparebuffer);
	      status = chkCorrectECC8(nandFlash.sparebuffer,
                                     (word __far *)(&secc1), secc2);
	      if (status == 3) {
      		printf("\nPage 0x%08LX has uncorrectable spare read error.\n",
            		     pagenum);
	      }
         else {
	         _nf_print_data("\nApplication's spare", pagenum,
	                         nandFlash.sparebuffer, nandFlash.sparesize);
         }
		}
	}
}


// Checks non-erased (non 0xFF) condition of far buffer.
// Returns 1 if any byte in the buffer is not an 0xFF, otherwise returns 0.
int non_erased(char far * dataBuffer, long dataSize)
{
	while (dataSize % (long) sizeof(long)) {
		if ('\xFF' != *dataBuffer) {
			return 1;
		}
		++dataBuffer;
		--dataSize;
	}
	while (dataSize) {
		if (0xFFFFFFFFL != *((long far *)dataBuffer)) {
			return 1;
		}
		dataBuffer += sizeof(long);
		dataSize -= (long) sizeof(long);
	}
	return 0;
}

// Checks if all bytes in the far buffer are the same.
// Returns 1 if all bytes in the buffer are the same, otherwise returns 0.
int all_same(char far * dataBuffer, long dataSize)
{
   long far * ptr;

   if ((*dataBuffer == dataBuffer[1]) && (*dataBuffer == dataBuffer[2]) &&
        (*dataBuffer == dataBuffer[3]))
   {
	   if (dataSize % (long) sizeof(long)) {
	      ptr = (long far *)(dataBuffer + (dataSize % (long) sizeof(long)));
	      dataSize -= dataSize % (long) sizeof(long);
	   }
      else {
	      ptr = (long far *)dataBuffer;
      }

	   while (dataSize) {
	      if (*ptr != *((long far *)dataBuffer)) {
	         return 0;
	      }
	      ptr++;
	      dataSize -= (long) sizeof(long);
	   }
   }
   else {
     return 0;
   }
	return 1;
}



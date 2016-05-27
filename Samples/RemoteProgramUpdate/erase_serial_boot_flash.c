/*
   Copyright (c) 2016, Digi International Inc.

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
	Simple program used to erase all of the serial boot flash, except for the
	UserBlock and System ID Block.  Use Samples/UserBlock/userblock_clear.c
	to erase the UserBlock, and the programs in Utilities/Write_ID to rewrite
	the System ID Block.
	
	Use Samples/RemoteProgramUpdate/firmware_report.c to confirm firmware
	erasure.
*/

#if !_SERIAL_BOOT_FLASH_
	#fatal This program only works for boards with a serial boot flash.
#endif

int main()
{
   char linebuf[40];
   unsigned long offset;
   unsigned long page, page_count;
   int rc;

	printf("This program will erase all but the User and SystemID Blocks.\n");
	printf("Type YES and press enter to continue.\n");
	printf("Are you sure you want to do this: ");
	getsn(linebuf, sizeof linebuf);
	
	if (strcmpi("YES", linebuf))
	{
		printf("Exiting...\n");
		return 0;
	}
	
	page = 0;
	page_count = (SBF_USERBLOCK_BEGIN / _sfb_dev.pagesize) - 1;
	for (offset = 0; offset < SBF_USERBLOCK_BEGIN; offset += _sfb_dev.pagesize)
	{
		printf(" erase page %lu/%lu (%u%%)\r", page, page_count,
			(unsigned int)(page * 100 / page_count));
		rc = _sbf_ErasePage(offset);
		if (rc)
		{
			printf("Error %d erasing page %lu\n", page);
			return 0;
		}
		++page;
	}
	
	printf("\nErase complete.\n");
}
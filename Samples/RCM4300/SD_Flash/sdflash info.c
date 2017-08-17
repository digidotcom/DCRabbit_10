/*
   Copyright (c) 2017, Digi International Inc.

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
sdflash_info.c

This program dumps information about the SD card, including size information
and data parsed from the CSD (Card-Specific Data), SCR (SD Configuration
Register) and CID (Card Identification).

*****************************************************************************/


#define SDFLASH_DEBUG
//#define SDFLASH_VERBOSE

#if CPU_ID_MASK(_CPU_ID_) >= R6000
	#use "r6000_bios.lib"
#endif

#use "sdflash.lib"

#define dump_CSD_char(x) printf("CSD." #x ": 0x%02X %u\n", sd->CSD. ## x, sd->CSD. ## x)
#define dump_CSD_int(x) printf("CSD." #x ": 0x%04X %u\n", sd->CSD. ## x, sd->CSD. ## x)
#define dump_SCR(x) printf("SCR." #x ": 0x%02X\n", sd->SCR. ## x)

void print_sd_device(const sd_device *sd)
{
	printf("sectors: %lu (%lu MB, %lu MiB)\n", sd->sectors,
		sd->sectors * 8 / 15625, // (512 / 1,000,000 reduces to 8/15,625)
		sd->sectors / (1024 * 1024 / 512) );
	printf("pagesize: %u\n", sd->pagesize);
	printf("pagebitshift: %d (%lu)\n", sd->pagebitshift, 1UL << sd->pagebitshift);
	switch (sd->CSD.CSD_STR)
	{
		case SD_CSD_STR_V1:
			printf("CSD Version 1.0\n");
			break;
		case SD_CSD_STR_V2:
			printf("CSD Version 2.0\n");
			break;
		default:
			printf("invalid CSD version %u\n", sd->CSD.CSD_STR);
	}
   dump_CSD_char(init_ok);
   dump_CSD_char(CSD_STR);
   dump_CSD_char(TAAC);
   dump_CSD_char(NSAC);
   dump_CSD_char(TRANSPEED);
   dump_CSD_int( CCC);
   dump_CSD_char(R_BL_LEN);
   dump_CSD_char(DSR_IMP);
   dump_CSD_int( CSIZE);
   
   if (sd->CSD.CSD_STR == SD_CSD_STR_V1) {
	   dump_CSD_char(C_SIZE_M);
   }

   dump_CSD_char(ERASE_BLK_EN);
   dump_CSD_char(DEF_ECC);
   dump_CSD_char(R2W_FACT);
   dump_CSD_char(WR_BL_LEN);
   dump_CSD_char(WR_BL_PAR);
   dump_CSD_char(COPY);
   dump_CSD_char(P_WR_PROT);
   dump_CSD_char(T_WR_PROT);
   dump_CSD_char(CRC7);

	dump_SCR(init_ok);
	dump_SCR(SCR_STRUCTURE);
	dump_SCR(SD_SPEC);
	dump_SCR(DATA_STAT_AFTER_ERASE);
	dump_SCR(SD_SECURITY);
	dump_SCR(SD_BUS_WIDTHS);
}

int main()
{
   int rc;
   sd_device *dev;
   SD_CID_TYPE cid;

	printf("SD Info on %s at %s.\n", __DATE__, __TIME__);
   dev = &SD[0];
	if (rc = sdspi_initDevice(0, &SD_dev0))
   {
   	printf("Flash init failed (%d): %ls\n\n", rc, strerror(rc));
      exit(rc);
   }
   
   print_sd_device(dev);
   
   if (sdspi_get_cid(dev, &cid) == 0) {
      sdspi_print_cid(&cid);
   }
}
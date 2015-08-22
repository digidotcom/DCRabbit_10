/***********************************************************************
	idblock_report.c


	Reports system ID Block information.

 ***********************************************************************


 Instructions:

 1. Compile the program to the target and run it.  The contents of the
    system ID block will be printed (or any errors reported) to stdio.


 ***********************************************************************


 Version History:
 3.01 - Now checks MB_TYPE macro to find true match from TCData.ini
 3.00 - Now #ximports TCData.ini to obtain all boards' information.
 2.14 - Added RCM4020, RCM4400W board types.
 2.13 - Added RCM4300, RCM4310 and RCM39x9 family board types.
 2.12 - Added RCM4510W board type.
 2.11 - Added RCM4100, RCM4120 and more User block info.
 2.10 - Added RCM4200 and RCM4210
 2.09 - Added separate RCM4000A and RCM4010
 2.08 - Added RCM4000A
 2.07 - Added RCM4000
 2.06 - Added RCM4110
 2.05
 2.04 - Added RCM3305, RCM3315, RCM3450 board types.
 2.03 - Added RCM3365, RCM3375 board types.  Corrected OP7210 board
        information.
 2.02 - Added RCM3760 board types.
 2.01 - Added RCM3420, RCM3750 board types.
 2.00 - Added support for version 5 ID block, corrected and updated
        PowerCore FLEX series' board information.
 1.08 - Added RCM2260, RCM3360, RCM3370, RCM3720, PowerCore Flex board types.
 1.07 - Corrected BL26xx series' board information.
 1.06 - Added BL2600A, BL2600B, BL2600C, BL2600D, RCM3310A board types.
 1.05 - Added RCM3300A, RCM3600H, RCM3610H, RCM3700H, RCM3710H board
        types.
 1.04 - Corrected conditional definition of RC3410A product ID.  Added
        RCM3600A, RCM3610A, RCM3700A, RCM3710A board types.
 1.03 - Added missing compile mode macro checks.
 1.02 - Added a flash type, reorganized the presentation of information,
        eased future flash writeMode type additions.
 1.01 - Updated board and flash information.
 1.00 - Initial version, based on write_idblock.c.


 **********************************************************************/


#class auto

#use "idblock_api.lib"

#define VERSION "3.00"
#define PROGRAM_NAME "idblock_report.c"

#define STDIO_ENABLE_LONG_STRINGS


// check for undefined compile mode macros


#ifndef FAST_RAM_COMPILE
 #define FAST_RAM_COMPILE 0
#endif
#ifndef FLASH_COMPILE
 #ifdef _FLASH_
  #define FLASH_COMPILE 1
 #else
  #define FLASH_COMPILE 0
 #endif
#endif
#ifndef __SEPARATE_INST_DATA__
 #define __SEPARATE_INST_DATA__ 0
#endif
#ifndef _SERIAL_BOOT_FLASH_
	#define _SERIAL_BOOT_FLASH_ 0
#endif

// local function prototypes


int GetFlashID(void);
void PrintFlashDescription(char *which, int FlashID, int FlashType);
void PrintProductDescription(int ProductID);
void PrintSysIDBlockInfo(void);
int _GetDevSpecIndex(int n, MemDevSpec *devspec);


/********************************************************************/


void main(void)
{
	static struct userBlockInfo uBI;

	printf("%s, version %s.\n", PROGRAM_NAME, VERSION);

	PrintSysIDBlockInfo();
	PrintProductDescription(SysIDBlock.productID);
	PrintFlashDescription("Flash ID", (int) SysIDBlock.flashID,
	                      SysIDBlock.flashType);
	PrintFlashDescription("Flash2 ID", (int) SysIDBlock.flash2ID,
	                      SysIDBlock.flash2Type);
#if FLASH_COMPILE || FAST_RAM_COMPILE
	PrintFlashDescription("GetFlashID() result", getFlashId(),
	                      getFlashWriteMode());
#endif
	printf("\n");

	GetUserBlockInfo(&uBI);
	if (!uBI.blockSize) {
		printf("\nNo valid User block found on this board!\n");
	} else {
		printf("\nID+User blocks area top address = 0x%08LX\n", uBI.topAddr);
		printf("   User block image A address   = 0x%08LX\n", uBI.addrA);
		printf("   User block image B address   = 0x%08LX\n", uBI.addrB);
		printf("   User block size              = 0x%08LX\n", uBI.blockSize);
		printf("   User block image %s is valid.\n", uBI.blockAvalid ? "A" : "B");
	}
}

struct _FlashDescription {
	int fType;
   char *fSectorType;
	char *fSectorLayout;
   char *fErase;
   char *fWrite;
};

const struct _FlashDescription FlashDesc[] = {
	{1,    "small sector, ", ""                  , "sector erase (0x30) ", "byte write"},
	{2,    "small sector, ", ""                  , ""                    , "sector write"},
	{3,    "small page, "  , ""                  , "page erase (0x50) "  , "byte write"},
	{4,    "small sector, ", ""                  , "sector erase (0x20) ", "byte write"},
   {5,    "small sector, ", ""                  , "sector erase (0x30) ", "word write"},
   {0x10, "large sector", ""                  , "", ""},
	{0x11, "large sector " , "(128/96/8/8/16), " , "sector erase (0x30) ", "byte write"},
	{0x12, "large sector " , "(16/8/8/96/128), " , "sector erase (0x30) ", "byte write"},
	{0x13, "large sector " , "(16*7/4/4/8), "    , "sector erase (0x30) ", "byte write"},
	{0x14, "large sector " , "(8/4/4/16*7), "    , "sector erase (0x30) ", "byte write"},
	{0x15, "large sector " , "(64*3/32/8/8/16), ", "sector erase (0x30) ", "byte write"},
	{0x16, "large sector " , "(16/8/8/32/64*3), ", "sector erase (0x30) ", "byte write"},
	{0x17, "large sector " , "(64*7/32/8/8/16), ", "sector erase (0x30) ", "byte write"},
	{0x18, "large sector " , "(16/8/8/32/64*7), ", "sector erase (0x30) ", "byte write"},
	{0x19, "large sector " , "(64*8), "          , "sector erase (0x30) ", "byte write"},
	{0x1A, "large sector " , "(64*8), "          , "sector erase (0x30) ", "byte write"},
	{0x1B, "large sector " , "(64*4), "          , "sector erase (0x30) ", "byte write"},
	{0x1C, "large sector " , "(64*4), "          , "sector erase (0x30) ", "byte write"},
	{0x1D, "large sector " , "(128/96/8/8), "    , "sector erase (0x30) ", "byte write"},
	{0x1E, "large sector " , "(32*8), "          , "sector erase (0x30) ", "byte write"},
	{0x1F, "large sector " , "(8/4/4/16*7), "    , "sector erase (0x30) ", "byte write"},
	{0x20, "large sector " , "(16*8), "          , "sector erase (0x30) ", "byte write"},
	{0x21, "large sector " , "(16*8), "          , "sector erase (0x30) ", "byte write"},
	{0x22, "large sector " , "(64*15/32/2*8/16),", "sector erase (0x30) ", "byte write"},
	{0, "unknown.\n  If valid, a description should be added herein", "", "", ""}
};

struct _Flash {
	int fID;
	char *name1;
   char *name2;
};

const struct _Flash Flash[] = {
	{0x0134, "n AMD AM29F0",  "02BB"},
	{0x0140, "n AMD AM29LV00", "2BT"},
	{0x016D, "n AMD AM29LV00", "1BB"},
	{0x0177, "n AMD AM29F0",  "04BT"},
	{0x017B, "n AMD AM29F0",  "04BB"},
	{0x01A4, "n AMD AM29F0",  "40B"},
	{0x01B0, "n AMD AM29F0",  "02BT"},
	{0x01B5, "n AMD AM29LV00", "4BT"},
	{0x01B6, "n AMD AM29LV00", "4BB"},
	{0x01C2, "n AMD AM29LV00", "2BB"},
	{0x01ED, "n AMD AM29LV00", "1BT"},

	{0x0434, " Fujitsu MBM29F002", "BC"},
	{0x04B0, " Fujitsu MBM29F002", "TC"},

	{0x1F07, "n Atmel AT49F", "002"},
	{0x1F08, "n Atmel AT49F", "002T"},
	{0x1F24, "n Atmel 1M ", "Serial Boot Flash"},
	{0x1F25, "n Atmel AT29C", "1024"},
	{0x1F26, "n Atmel AT29L", "V1024"},
	{0x1F2C, "n Atmel 2M ", "Serial Boot Flash"},
	{0x1F34, "n Atmel 4M ", "Serial Boot Flash"},
	{0x1F35, "n Atmel AT29L", "V010 / AT29BV010A"},
	{0x1FA4, "n Atmel AT29C", "040"},
	{0x1FBA, "n Atmel AT29L", "V020 / AT29BV020"},
	{0x1FC4, "n Atmel AT29L", "V040 / AT29BV040"},
	{0x1FD5, "n Atmel AT29C", "010"},
	{0x1FDA, "n Atmel AT29C", "020"},

	{0x2023, "n STMicroelectronics M29W0", "10B"},
	{0x20E2, "n STMicroelectronics M29F0", "40B"},
	{0x20E3, "n STMicroelectronics M29W0", "40B"},

	{0x4001, " Mosel/Vitelic V29C5100", "1T"},
	{0x4002, " Mosel/Vitelic V29C5100", "2T"},
	{0x4003, " Mosel/Vitelic V29C5100", "4T"},
	{0x4060, " Mosel/Vitelic V29LC510", "01"},
	{0x4063, " Mosel/Vitelic V29C3100", "4T"},
	{0x4073, " Mosel/Vitelic V29C3100", "4B"},
	{0x4082, " Mosel/Vitelic V29LC510", "02"},
	{0x40A1, " Mosel/Vitelic V29C5100", "1B"},
	{0x40A2, " Mosel/Vitelic V29C5100", "2B"},
	{0x40A3, " Mosel/Vitelic V29C5100", "4B"},

	{0xAD34, " Hyundai", " HY29F002B"},
	{0xADB0, " Hyundai", "/Hynix HY29F002T"},

	{0xBF07, "n SST SST29EE", "010"},
	{0xBF08, "n SST SST29LE", "010 / SST29VE010"},
	{0xBF10, "n SST SST29EE", "020"},
	{0xBF12, "n SST SST29LE", "020 / SST29VE020"},
	{0xBF13, "n SST SST29SF", "040"},
	{0xBF14, "n SST SST29VF", "040"},
	{0xBF20, "n SST SST29SF", "512"},
	{0xBF21, "n SST SST29VF", "512"},
	{0xBF22, "n SST SST29SF", "010"},
	{0xBF23, "n SST SST29VF", "010"},
	{0xBF24, "n SST SST29SF", "020"},
	{0xBF25, "n SST SST29VF", "020"},
	{0xBF3D, "n SST SST29LE", "512 / SST29VE512"},
	{0xBF5D, "n SST SST29EE", "512"},
	{0xBFB4, "n SST SST39SF", "512"},
	{0xBFB5, "n SST SST39SF", "010"},
	{0xBFB6, "n SST SST39SF", "020"},
	{0xBFB7, "n SST SST39SF", "040"},
	{0xBFD4, "n SST SST39LF", "512 / SST39VF512"},
	{0xBFD5, "n SST SST39LF", "010 / SST39VF010"},
	{0xBFD6, "n SST SST39LF", "020 / SST39VF020"},
	{0xBFD7, "n SST SST39LF", "040 / SST39VF040"},
   {0x2780, "n SST SST39LF", "400A / SST39VF400A"},
   {0x2781, "n SST SST39LF", "800A / SST39VF800A"},

	{0xC234, " Macronix MX29F0", "02B"},
	{0xC2B0, " Macronix MX29F0", "2T"},

	{0xDA45, " Winbond W29", "C020CT"},
	{0xDA46, " Winbond W29", "C040"},
	{0xDAB5, " Winbond W39", "L020"},
	{0xDAC1, " Winbond W29", "EE011"},
	{0x01DA, " Spansion S29AL00D8", " "},

	{0, "n unlisted type.\n  If valid, a description should be added herein", ""}
};

void PrintFlashDescription(char *which, int FlashID, int FlashType)
{
	int i;

	i = 0;
	while (Flash[i].fID) {
		if (Flash[i].fID == FlashID) {
			break;
		}
		i++;
	}
	printf("\n\n%s 0x%04X is a%s%s.\n", which, FlashID, Flash[i].name1,
          Flash[i].name2);

	i = 0;
	while (FlashDesc[i].fType) {
		if (FlashDesc[i].fType == FlashType) {
			break;
		}
		i++;
	}
	printf("  Type 0x%04X is %s%s%s%s.\n", FlashType, FlashDesc[i].fSectorType,
          FlashDesc[i].fSectorLayout, FlashDesc[i].fErase, FlashDesc[i].fWrite);
}

#ximport "TCData.ini" TCData_ini

uint32 FindNextLBracket(uint32 begin, uint32 end)
{
	auto int temp;

	if (begin < end) {
		while (begin < end) {
			temp = xgetint(begin) & 0xFF;
			if ((int) '[' == temp) {
				break;
			}
			++begin;
		}
	} else {
		begin = end + 1ul;
	}
	return begin;
}

char *ProductDescription(int pID, uint32 beginAddr, uint32 endAddr)
{
	auto char *pidPtr, *pmacPtr, *result;
	auto long test;
	static char infoBuffer[4096];

	result = NULL;	// default to product description not found
	if (endAddr - beginAddr > (sizeof(infoBuffer) - 1ul)) {
		endAddr = beginAddr + sizeof(infoBuffer) - 1ul;
	}
	memset(infoBuffer, 0, sizeof(infoBuffer));
	xmem2root(infoBuffer, beginAddr, (unsigned) (endAddr - beginAddr));
	pidPtr = strstr(infoBuffer, "id");
	if (pidPtr) {
		pidPtr = strstr(pidPtr, "\"0x");
	}
	if (pidPtr) {
		pidPtr += strlen("\"0x");	// step past the opening "0x
		test = strtol(pidPtr, NULL, 16);
		if ((int) test == pID) {
         pmacPtr = strstr(pidPtr, "macro =");
         if (pmacPtr) {
            pmacPtr = strstr(pmacPtr, "MB_TYPE");
            if (pmacPtr) {
               pmacPtr = strstr(pmacPtr, "0x");
            }
            if (pmacPtr) {
               pmacPtr += 2;
               test = strtol(pmacPtr, NULL, 16);
               if ((int) test != (_DC_MB_TYPE_ & 0xFF00)) {
                  return result;
               }
            }
#if (_DC_MB_TYPE_ != 0)
            else {
               return result;
            }
#endif
	         result = infoBuffer;
	         pidPtr = strrchr(result, '\"');
	         if (pidPtr) {
	            ++pidPtr;
	            *pidPtr = '\0';
            }
			}
		}
	}
	return result;
}

void PrintProductDescription(int ProductID)
{
	auto char *pDesc;
	auto unsigned long infoAddr, infoEnd, sectionEnd;

	infoAddr = TCData_ini + sizeof(unsigned long);
	infoEnd = infoAddr + (unsigned long) xgetlong(TCData_ini);

	while (infoAddr < infoEnd) {
		sectionEnd = FindNextLBracket(infoAddr + 1ul, infoEnd);
		pDesc = ProductDescription(ProductID, infoAddr, sectionEnd);
		if (sectionEnd <= infoEnd && pDesc) {
			break;
		} else {
			infoAddr = sectionEnd;
		}
	}
	if (pDesc) {
		printf("\n%s\n", pDesc);
	} else {
		printf("\nProduct ID 0x%04X is not found in the TCData.ini file!\n",
		       ProductID);
	}
}

void PrintSysIDBlockInfo(void)
{
	auto int i;
   auto char buf[MACRO_NAME_SIZE + 5];
   auto unsigned long value;
   auto MemDevSpec devspec;

	printf("\nSystem ID Block content:\n\n");
	printf("   tableVersion  = %d\n", SysIDBlock.tableVersion);
	printf("   productID     = 0x%04X\n", SysIDBlock.productID);
	printf("   vendorID      = %d\n", SysIDBlock.vendorID);
	printf("   timestamp     = %02d/%02d/%02d%02d  %02d:%02d:%02d\n\n",
		SysIDBlock.timestamp[2], SysIDBlock.timestamp[3],
		SysIDBlock.timestamp[0], SysIDBlock.timestamp[1],
			SysIDBlock.timestamp[4], SysIDBlock.timestamp[5],
			SysIDBlock.timestamp[6]);

	printf("   flashID       = 0x%08LX\n", SysIDBlock.flashID);
	printf("   flashType     = 0x%04X\n", SysIDBlock.flashType);
	printf("   flashSize     = %d Kbytes\n", SysIDBlock.flashSize * 4);
	printf("   sectorSize    = %d bytes\n", SysIDBlock.sectorSize);
	printf("   numSectors    = %d\n", SysIDBlock.numSectors);
	printf("   flashSpeed    = %d nS\n\n", SysIDBlock.flashSpeed);

	printf("   flash2ID      = 0x%08LX\n", SysIDBlock.flash2ID);
	printf("   flash2Type    = 0x%04X\n", SysIDBlock.flash2Type);
	printf("   flash2Size    = %d Kbytes\n", SysIDBlock.flash2Size * 4);
	printf("   sector2Size   = %d bytes\n", SysIDBlock.sector2Size);
	printf("   num2Sectors   = %d\n", SysIDBlock.num2Sectors);
	printf("   flash2Speed   = %d nS\n\n", SysIDBlock.flash2Speed);

	printf("   ramID         = 0x%08LX\n", SysIDBlock.ramID);
	printf("   ramSize       = %d Kbytes\n", SysIDBlock.ramSize * 4);
	printf("   ramSpeed      = %d nS\n\n", SysIDBlock.ramSpeed);

	printf("   cpuID         = Rabbit %u (rev. %u)\n",
	       ((SysIDBlock.cpuID >> 8) & 0xFF) * 1000 + 2000,
	       SysIDBlock.cpuID & 0xFF);
	printf("   crystalFreq   = %.4f MHz\n\n", SysIDBlock.crystalFreq * 1.0e-6);

	printf("   macAddr       = %02X:%02X:%02X:%02X:%02X:%02X\n",
	       SysIDBlock.macAddr[0], SysIDBlock.macAddr[1], SysIDBlock.macAddr[2],
	       SysIDBlock.macAddr[3], SysIDBlock.macAddr[4], SysIDBlock.macAddr[5]);
	printf("   serialNumber  = '%s'\n", SysIDBlock.serialNumber);
	printf("   productName   = '%s'\n\n", SysIDBlock.productName);

   if (SysIDBlock.tableVersion >= 5)
   {
  	   printf("   flashMBC      = 0x%02X\n", SysIDBlock.idBlock2.flashMBC);
  	   printf("   flash2MBC     = 0x%02X\n", SysIDBlock.idBlock2.flash2MBC);
  	   printf("   ramMBC        = 0x%02X\n", SysIDBlock.idBlock2.ramMBC);
  	   printf("   devSpecLoc    = 0x%08LX\n", SysIDBlock.idBlock2.devSpecLoc);
  	   printf("   macrosLoc     = 0x%08LX\n", SysIDBlock.idBlock2.macrosLoc);
	   printf("   driversLoc    = 0x%08LX\n", SysIDBlock.idBlock2.driversLoc);
	   printf("   ioDescLoc     = 0x%08LX\n", SysIDBlock.idBlock2.ioDescLoc);
	   printf("   ioPermLoc     = 0x%08LX\n", SysIDBlock.idBlock2.ioPermLoc);
	   printf("   persBlockLoc  = 0x%08LX\n", SysIDBlock.idBlock2.persBlockLoc);
	   printf("   userBlockSiz2 = 0x%04X\n", SysIDBlock.idBlock2.userBlockSiz2);
	   printf("   idBlockCRC2   = 0x%04X\n", SysIDBlock.idBlock2.idBlockCRC2);
   }
	printf("   reserved[0]   = 0x%02X\n\n", SysIDBlock.reserved[0]);
	printf("   idBlockSize   = 0x%08LX bytes\n", SysIDBlock.idBlockSize);
	printf("   userBlockSize = 0x%04X\n", SysIDBlock.userBlockSize);
	printf("   userBlockLoc  = 0x%04X\n\n", SysIDBlock.userBlockLoc);

  	printf("   idBlockCRC    = 0x%04X\n", SysIDBlock.idBlockCRC);
	printf("   marker        = %02X %02X %02X %02X %02X %02X\n",
	       SysIDBlock.marker[0], SysIDBlock.marker[1], SysIDBlock.marker[2],
	       SysIDBlock.marker[3], SysIDBlock.marker[4], SysIDBlock.marker[5]);

   if (SysIDBlock.tableVersion >= 5)
   {
     	printf("\nSystem Macro Table contents:\n");
   	if(SysIDBlock.idBlock2.macrosLoc) {
   		i = 0;
      	while(_GetSysMacroIndex(i, buf, &value) == 0) {
	         printf("   %-13s = 0x%08LX\n", buf, value);
            if(strncmp(buf, "_DC_BRD_OPT0_", 13) == 0) {
	            if(value & 1) {
	               printf("      16 Bit FLASH\n");
	            }
	            if(value & 2) {
	               printf("      16 Bit RAM\n");
	            }
	            if(value & 4) {
	               printf("      12 Bit ADC\n");
	            }
	            if(value & 8) {
	               printf("      14 Bit ADC\n");
	            }
	            if(value & 16) {
	               printf("      32MB NAND FLASH\n");
	            }
	            if(value & 32) {
	               printf("      Prog port PB1 is unavailable or not pulled up\n");
	            }
            }
            i++;
         }
      }
      else {
			printf("   No entries.\n");
      }
     	printf("\nDevice Specification Table contents:");
   	if(SysIDBlock.idBlock2.devSpecLoc) {
   		i = 0;
      	while(_GetDevSpecIndex(i, &devspec) == 0) {
	         printf("\n   Dev ID        = 0x%08LX\n", devspec.ID);
	         printf("   Dev Type      = 0x%04X\n", devspec.type);
	         printf("   Dev Size      = %d Kbytes\n", devspec.devSize * 4);
	         printf("   sectorSize    = %d bytes\n", devspec.sectorSize);
	         printf("   numSectors    = %d\n", devspec.numSectors);
	         printf("   Dev Speed     = %d nS\n", devspec.devSpeed);
	         printf("   Dev MBC       = 0x%02X\n", devspec.MBC);
	         i++;
         }
      }
      else {
			printf("\n   No entries.\n");
      }
   }
}



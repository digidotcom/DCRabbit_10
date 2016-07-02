/*
   Copyright (c) 2016 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
// BL_triplets.cpp
// Serial Boot Loader triplet utility
// Original Author: Bryce Hathaway
// Pirated and otherwise mangled by: Joel Helmich
// Date: June 2008

//The original triplet utility has been modified to produce the boot loader triplets that get placed before the BIOS in serial flash.
//The output is a text file rather than a binary file, as the result will be #used by StdBios.c

#include <iostream>
#include <iomanip>
#include <map>
#include <fstream>
#include <sstream>

using namespace std;

map<string, unsigned short> io_map;

#define MAX_LOADER_LENGTH 512
unsigned char loader_bytes[MAX_LOADER_LENGTH];
int curr_index = 0;

// Here is the array used to initialize the map.
struct MyPair
{
   const char * first;
   unsigned short second;
};

MyPair io_map_data[] =
{
   {"GCSR", 0x00},
   {"RTCCR", 0x01},
   {"RTC0R", 0x02},
   {"RTC1R", 0x03},
   {"RTC2R", 0x04},
   {"RTC3R", 0x05},
   {"RTC4R", 0x06},
   {"RTC5R", 0x07},
   {"WDTCR", 0x08},
   {"WDTTR", 0x09},
   {"GCM0R", 0x0A},
   {"GCM1R", 0x0B},
   {"GPSCR", 0x0D},
   {"GOCR", 0x0E},
   {"GCDR", 0x0F},  

   {"MMIDR", 0x10},
   {"STACKSEG", 0x11},
   {"STACKSEGL", 0x1A},
   {"STACKSEGH", 0x1B},
   {"DATASEG", 0x12},
   {"DATASEGL", 0x1E},
   {"DATASEGH", 0x1F},
   {"SEGSIZE", 0x13},
   {"MB0CR", 0x14},
   {"MB1CR", 0x15},
   {"MB2CR", 0x16},
   {"MB3CR", 0x17},
   {"MECR", 0x18},
   {"MTCR", 0x19},
   {"BDCR", 0x1C},

   {"SPD0R", 0x20},
   {"SPD1R", 0x21},
   {"SPD2R", 0x22},
   {"SPSR", 0x23},
   {"SPCR", 0x24},  
   {"GROM", 0x2C},
   {"GRAM", 0x2D},  
   {"GCPU", 0x2E},  
   {"GREV", 0x2F},

   {"PADR", 0x30},  

   {"PBDR", 0x40},  
   {"PBDDR", 0x47},  

   {"PCDR", 0x50},  
   {"PCFR", 0x55},  

   {"ICCSR", 0x56},  
   {"ICCR", 0x57},  
   {"ICT1R", 0x58},
   {"ICS1R", 0x59},  
   {"ICL1R", 0x5A},  
   {"ICM1R", 0x5B},
   {"ICT2R", 0x5C},
   {"ICS2R", 0x5D},
   {"ICL2R", 0x5E},
   {"ICM2R", 0x5F},

   {"PDDR", 0x60},
   {"PDCR", 0x64},
   {"PDFR", 0x65},
   {"PDDCR", 0x66},
   {"PDDDR", 0x67},
   {"PDB0R", 0x68},  
   {"PDB1R", 0x69},
   {"PDB2R", 0x6A},  
   {"PDB3R", 0x6B},  
   {"PDB4R", 0x6C},
   {"PDB5R", 0x6D},
   {"PDB6R", 0x6E},  
   {"PDB7R", 0x6F},

   {"PEDR", 0x70},  
   {"PECR", 0x74},
   {"PEFR", 0x75},
   {"PEDDR", 0x77},
   {"PEB0R", 0x78},
   {"PEB1R", 0x79},  
   {"PEB2R", 0x7A},  
   {"PEB3R", 0x7B},
   {"PEB4R", 0x7C},  
   {"PEB5R", 0x7D},  
   {"PEB6R", 0x7E},
   {"PEB7R", 0x7F},  

   {"IB0CR", 0x80},
   {"IB1CR", 0x81},  
   {"IB2CR", 0x82},  
   {"IB3CR", 0x83},
   {"IB4CR", 0x84},
   {"IB5CR", 0x85},  
   {"IB6CR", 0x86},
   {"IB7CR", 0x87},

   {"PWL0R", 0x88},
   {"PWM0R", 0x89},  
   {"PWL1R", 0x8A},  
   {"PWM1R", 0x8B},
   {"PWL2R", 0x8C},  
   {"PWM2R", 0x8D},  
   {"PWL3R", 0x8E},
   {"PWM3R", 0x8F},  

   {"QDCSR", 0x90},
   {"QDCR", 0x91},  
   {"QDC1R", 0x94},  
   {"QDC1HR", 0x95},
   {"QDC2R", 0x96},  
   {"QDC2HR", 0x97},  

   {"I0CR", 0x98},  
   {"I1CR", 0x99},  

   {"TACSR", 0xA0},  
   {"TAPR", 0xA1},
   {"TAT1R", 0xA3},
   {"TACR", 0xA4},  
   {"TAT2R", 0xA5},  
   {"TAT8R", 0xA6},
   {"TAT3R", 0xA7},  
   {"TAT9R", 0xA8},
   {"TAT4R", 0xA9},
   {"TAT10R", 0xAA},  
   {"TAT5R", 0xAB},  
   {"TAT6R", 0xAD},
   {"TAT7R", 0xAF},  

   {"TBCSR", 0xB0},
   {"TBCR", 0xB1},  
   {"TBM1R", 0xB2},  
   {"TBL1R", 0xB3},
   {"TBM2R", 0xB4},  
   {"TBL2R", 0xB5},  
   {"TBCMR", 0xBE},
   {"TBCLR", 0xBF},  

   {"SADR", 0xC0},
   {"SAAR", 0xC1},  
   {"SALR", 0xC2},
   {"SASR", 0xC3},
   {"SACR", 0xC4},  
   {"SAER", 0xC5},  

   {"SEDR", 0xC8},  
   {"SEAR", 0xC9},  
   {"SELR", 0xCA},
   {"SESR", 0xCB},  
   {"SECR", 0xCC},  
   {"SEER", 0xCD},

   {"SBDR", 0xD0},  
   {"SBAR", 0xD1},
   {"SBLR", 0xD2},  
   {"SBSR", 0xD3},  
   {"SBCR", 0xD4},
   {"SBER", 0xD5},  

   {"SFDR", 0xD8},
   {"SFAR", 0xD9},
   {"SFLR", 0xDA},  
   {"SFSR", 0xDB},
   {"SFCR", 0xDC},  
   {"SFER", 0xDD},  

   {"SCDR", 0xE0},  
   {"SCAR", 0xE1},  
   {"SCLR", 0xE2},
   {"SCSR", 0xE3},
   {"SCCR", 0xE4},  
   {"SCER", 0xE5},

   {"SDDR", 0xF0},  
   {"SDAR", 0xF1},
   {"SDLR", 0xF2},  
   {"SDSR", 0xF3},  
   {"SDCR", 0xF4},
   {"SDER", 0xF5},  

   {"RTUER", 0x300},
   {"SPUER", 0x320}, 
   {"PAUER", 0x330}, 
   {"PBUER", 0x340},
   {"PCUER", 0x350}, 
   {"PDUER", 0x360}, 
   {"PEUER", 0x370},
   {"ICUER", 0x358}, 
   {"IBUER", 0x380}, 
   {"PWUER", 0x388},
   {"QDUER", 0x390}, 
   {"IUER", 0x398}, 
   {"TAUER", 0x3A0},
   {"TBUER", 0x3B0}, 
   {"SAUER", 0x3C0}, 
   {"SBUER", 0x3D0},
   {"SCUER", 0x3E0},
   {"SDUER", 0x3F0}, 
   {"SEUER", 0x3C8},
   {"SFUER", 0x3D8}, 

   {"EDMR", 0x420},
   {"WPCR", 0x440}, 

   {"STKCR", 0x444},
   {"STKLLR", 0x445}, 
   {"STKHLR", 0x446},

   {"RAMSR", 0x448},

   {"WPLR", 0x460},
   {"WPHR", 0x461},

   {"WPSAR", 0x480},
   {"WPSALR", 0x481},
   {"WPSAHR", 0x482},
   {"WPSBR", 0x484},
   {"WPSBLR", 0x485},
   {"WPSBHR", 0x486},

   {"STKSEGL", 0x001A},
   {"STKSEGH", 0x001B},
   {"MACR", 0x001D},
   {"DATSEGL", 0x001E},
   {"DATSEGH", 0x001F},
   {"IHCR", 0x0028},
   {"IHSR", 0x0029},
   {"IHTR", 0x002A},

   {"TBSL1R", 0x00BA},
   {"TBSM1R", 0x00BB},
   {"TBSL2R", 0x00BC},
   {"TBSM2R", 0x00BD},

   {"PWBAR", 0x00E8},
   {"PWBPR", 0x00E9},

   {"ACS0CR", 0x0410},
   {"ACS1CR", 0x0411},

   {"VRAM00", 0x0600},
   {"VRAM01", 0x0601},
   {"VRAM02", 0x0602},
   {"VRAM03", 0x0603},
   {"VRAM04", 0x0604},
   {"VRAM05", 0x0605},
   {"VRAM06", 0x0606},
   {"VRAM07", 0x0607},
   {"VRAM08", 0x0608},
   {"VRAM09", 0x0609},
   {"VRAM0A", 0x060A},
   {"VRAM0B", 0x060B},
   {"VRAM0C", 0x060C},
   {"VRAM0D", 0x060D},
   {"VRAM0E", 0x060E},
   {"VRAM0F", 0x060F},
   {"VRAM10", 0x0610},
   {"VRAM11", 0x0611},
   {"VRAM12", 0x0612},
   {"VRAM13", 0x0613},
   {"VRAM14", 0x0614},
   {"VRAM15", 0x0615},
   {"VRAM16", 0x0616},
   {"VRAM17", 0x0617},
   {"VRAM18", 0x0618},
   {"VRAM19", 0x0619},
   {"VRAM1A", 0x061A},
   {"VRAM1B", 0x061B},
   {"VRAM1C", 0x061C},
   {"VRAM1D", 0x061D},
   {"VRAM1E", 0x061E},
   {"VRAM1F", 0x061F},

   {"B0M0R", 0x0308},
   {"B0M1R", 0x0309},
   {"B0M2R", 0x030A},
   {"B0CR", 0x030B},
   {"B0A0R", 0x030C},
   {"B0A1R", 0x030D},
   {"B0A2R", 0x030E},
   {"B1M0R", 0x0318},
   {"B1M1R", 0x0319},
   {"B1M2R", 0x031A},
   {"B1CR", 0x031B},
   {"B1A0R", 0x031C},
   {"B1A1R", 0x031D},
   {"B1A2R", 0x031E},
   {"B2M0R", 0x0328},
   {"B2M1R", 0x0329},
   {"B2M2R", 0x032A},
   {"B2CR", 0x032B},
   {"B2A0R", 0x032C},
   {"B2A1R", 0x032D},
   {"B2A2R", 0x032E},
   {"B3M0R", 0x0338},
   {"B3M1R", 0x0339},
   {"B3M2R", 0x033A},
   {"B3CR", 0x033B},
   {"B3A0R", 0x033C},
   {"B3A1R", 0x033D},
   {"B3A2R", 0x033E},
   {"B4M0R", 0x0348},
   {"B4M1R", 0x0349},
   {"B4M2R", 0x034A},
   {"B4CR", 0x034B},
   {"B4A0R", 0x034C},
   {"B4A1R", 0x034D},
   {"B4A2R", 0x034E},
   {"B5M0R", 0x0368},
   {"B5M1R", 0x0369},
   {"B5M2R", 0x036A},
   {"B5CR", 0x036B},
   {"B5A0R", 0x036C},
   {"B5A1R", 0x036D},
   {"B5A2R", 0x036E},
   {"B6M0R", 0x0378},
   {"B6M1R", 0x0379},
   {"B6M2R", 0x037A},
   {"B6CR", 0x037B},
   {"B6A0R", 0x037C},
   {"B6A1R", 0x037D},
   {"B6A2R", 0x037E},

   {"WP0R", 0x0460},          
   {"WP1R", 0x0461},
   {"WP2R", 0x0462},
   {"WP3R", 0x0463},
   {"WP4R", 0x0464},
   {"WP5R", 0x0465},
   {"WP6R", 0x0466},
   {"WP7R", 0x0467},
   {"WP8R", 0x0468},
   {"WP9R", 0x0469},
   {"WP10R", 0x046A},
   {"WP11R", 0x046B},
   {"WP12R", 0x046C},
   {"WP13R", 0x046D},
   {"WP14R", 0x046E},
   {"WP15R", 0x046F},
   {"WP16R", 0x0470},
   {"WP17R", 0x0471},
   {"WP18R", 0x0472},
   {"WP19R", 0x0473},
   {"WP20R", 0x0474},
   {"WP21R", 0x0475},
   {"WP22R", 0x0476},
   {"WP23R", 0x0477},
   {"WP24R", 0x0478},
   {"WP25R", 0x0479},
   {"WP26R", 0x047A},
   {"WP27R", 0x047B},
   {"WP28R", 0x047C},
   {"WP29R", 0x047D},
   {"WP30R", 0x047E},
   {"WP31R", 0x047F},

   {"TCUER", 0x03F8},

   {"PCDDR", 0x0051}, 
   {"PCDCR", 0x0054},
   {"PCALR", 0x0052},
   {"PCAHR", 0x0053},
   {"PDALR", 0x0062},
   {"PDAHR", 0x0063},
   {"PEALR", 0x0072},
   {"PEAHR", 0x0073},
   {"PEDCR", 0x0076},

   {"TCCSR", 0x0500},
   {"TCCR", 0x0501},
   {"TCDLR", 0x0502},
   {"TCDHR", 0x0503},
   {"TCS0LR", 0x0508},
   {"TCS0HR", 0x0509},
   {"TCR0LR", 0x050A},
   {"TCR0HR", 0x050B},
   {"TCS1LR", 0x050C},
   {"TCS1HR", 0x050D},
   {"TCR1LR", 0x050E},
   {"TCR1HR", 0x050F},
   {"TCS2LR", 0x0518},
   {"TCS2HR", 0x0519},
   {"TCR2LR", 0x051A},
   {"TCR2HR", 0x051B},
   {"TCS3LR", 0x051C},
   {"TCS3HR", 0x051D},
   {"TCR3LR", 0x051E},
   {"TCR3HR", 0x051F},

   {"TCBAR", 0x00F8},
   {"TCBPR", 0x00F9},

   {"SADLR", 0x00C6},
   {"SADHR", 0x00C7},
   {"SBDLR", 0x00D6},
   {"SBDHR", 0x00D7},
   {"SCDLR", 0x00E6},
   {"SCDHR", 0x00E7},
   {"SDDLR", 0x00F6},
   {"SDDHR", 0x00F7},
   {"SEDLR", 0x00CE},
   {"SEDHR", 0x00CF},
   {"SFDLR", 0x00DE},
   {"SFDHR", 0x00DF},

   {"DMCSR", 0x0100},
   {"DMALR", 0x0101},
   {"DMHR", 0x0102},
   {"DMCR", 0x0104},
   {"DMTCR", 0x0105},
   {"DMR0CR", 0x0106},
   {"DMR1CR", 0x0107},
   {"DTRCR", 0x0115},
   {"DTRDLR", 0x0116},
   {"DTRDHR", 0x0117},

   {"D0BCR", 0x0103},
   {"D0TBR", 0x0108},
   {"D0TMR", 0x0109},
   {"D0BU0R", 0x010A},
   {"D0BU1R", 0x010B},
   {"D0IA0R", 0x010C},
   {"D0IA1R", 0x010D},
   {"D0IA2R", 0x010E},
   {"D0SMR", 0x0180},
   {"D0CR", 0x0181},
   {"D0L0R", 0x0182},
   {"D0L1R", 0x0183},
   {"D0SA0R", 0x0184},
   {"D0SA1R", 0x0185},
   {"D0SA2R", 0x0186},
   {"D0DA0R", 0x0188},
   {"D0DA1R", 0x0189},
   {"D0DA2R", 0x018A},
   {"D0LA0R", 0x018C},
   {"D0LA1R", 0x018D},
   {"D0LA2R", 0x018E},

   {"D1BCR", 0x0113},
   {"D1TBR", 0x0118},
   {"D1TMR", 0x0119},
   {"D1BU0R", 0x011A},
   {"D1BU1R", 0x011B},
   {"D1IA0R", 0x011C},
   {"D1IA1R", 0x011D},
   {"D1IA2R", 0x011E},
   {"D1SMR", 0x0190},
   {"D1CR", 0x0191},
   {"D1L0R", 0x0192},
   {"D1L1R", 0x0193},
   {"D1SA0R", 0x0194},
   {"D1SA1R", 0x0195},
   {"D1SA2R", 0x0196},
   {"D1DA0R", 0x0198},
   {"D1DA1R", 0x0199},
   {"D1DA2R", 0x019A},
   {"D1LA0R", 0x019C},
   {"D1LA1R", 0x019D},
   {"D1LA2R", 0x019E},

   {"D2BCR", 0x0123},
   {"D2TBR", 0x0128},
   {"D2TMR", 0x0129},
   {"D2BU0R", 0x012A},
   {"D2BU1R", 0x012B},
   {"D2IA0R", 0x012C},
   {"D2IA1R", 0x012D},
   {"D2IA2R", 0x012E},
   {"D2SMR", 0x01A0},
   {"D2CR", 0x01A1},
   {"D2L0R", 0x01A2},
   {"D2L1R", 0x01A3},
   {"D2SA0R", 0x01A4},
   {"D2SA1R", 0x01A5},
   {"D2SA2R", 0x01A6},
   {"D2DA0R", 0x01A8},
   {"D2DA1R", 0x01A9},
   {"D2DA2R", 0x01AA},
   {"D2LA0R", 0x01AC},
   {"D2LA1R", 0x01AD},
   {"D2LA2R", 0x01AE},

   {"D3BCR", 0x0133},
   {"D3TBR", 0x0138},
   {"D3TMR", 0x0139},
   {"D3BU0R", 0x013A},
   {"D3BU1R", 0x013B},
   {"D3IA0R", 0x013C},
   {"D3IA1R", 0x013D},
   {"D3IA2R", 0x013E},
   {"D3SMR", 0x01B0},
   {"D3CR", 0x01B1},
   {"D3L0R", 0x01B2},
   {"D3L1R", 0x01B3},
   {"D3SA0R", 0x01B4},
   {"D3SA1R", 0x01B5},
   {"D3SA2R", 0x01B6},
   {"D3DA0R", 0x01B8},
   {"D3DA1R", 0x01B9},
   {"D3DA2R", 0x01BA},
   {"D3LA0R", 0x01BC},
   {"D3LA1R", 0x01BD},
   {"D3LA2R", 0x01BE},

   {"D4BCR", 0x0143},
   {"D4TBR", 0x0148},
   {"D4TMR", 0x0149},
   {"D4BU0R", 0x014A},
   {"D4BU1R", 0x014B},
   {"D4IA0R", 0x014C},
   {"D4IA1R", 0x014D},
   {"D4IA2R", 0x014E},
   {"D4SMR", 0x01C0},
   {"D4CR", 0x01C1},
   {"D4L0R", 0x01C2},
   {"D4L1R", 0x01C3},
   {"D4SA0R", 0x01C4},
   {"D4SA1R", 0x01C5},
   {"D4SA2R", 0x01C6},
   {"D4DA0R", 0x01C8},
   {"D4DA1R", 0x01C9},
   {"D4DA2R", 0x01CA},
   {"D4LA0R", 0x01CC},
   {"D4LA1R", 0x01CD},
   {"D4LA2R", 0x01CE},

   {"D5BCR", 0x0153},
   {"D5TBR", 0x0158},
   {"D5TMR", 0x0159},
   {"D5BU0R", 0x015A},
   {"D5BU1R", 0x015B},
   {"D5IA0R", 0x015C},
   {"D5IA1R", 0x015D},
   {"D5IA2R", 0x015E},
   {"D5SMR", 0x01D0},
   {"D5CR", 0x01D1},
   {"D5L0R", 0x01D2},
   {"D5L1R", 0x01D3},
   {"D5SA0R", 0x01D4},
   {"D5SA1R", 0x01D5},
   {"D5SA2R", 0x01D6},
   {"D5DA0R", 0x01D8},
   {"D5DA1R", 0x01D9},
   {"D5DA2R", 0x01DA},
   {"D5LA0R", 0x01DC},
   {"D5LA1R", 0x01DD},
   {"D5LA2R", 0x01DE},

   {"D6BCR", 0x0163},
   {"D6TBR", 0x0168},
   {"D6TMR", 0x0169},
   {"D6BU0R", 0x016A},
   {"D6BU1R", 0x016B},
   {"D6IA0R", 0x016C},
   {"D6IA1R", 0x016D},
   {"D6IA2R", 0x016E},
   {"D6SMR", 0x01E0},
   {"D6CR", 0x01E1},
   {"D6L0R", 0x01E2},
   {"D6L1R", 0x01E3},
   {"D6SA0R", 0x01E4},
   {"D6SA1R", 0x01E5},
   {"D6SA2R", 0x01E6},
   {"D6DA0R", 0x01E8},
   {"D6DA1R", 0x01E9},
   {"D6DA2R", 0x01EA},
   {"D6LA0R", 0x01EC},
   {"D6LA1R", 0x01ED},
   {"D6LA2R", 0x01EE},

   {"D7BCR", 0x0173},
   {"D7TBR", 0x0178},
   {"D7TMR", 0x0179},
   {"D7BU0R", 0x017A},
   {"D7BU1R", 0x017B},
   {"D7IA0R", 0x017C},
   {"D7IA1R", 0x017D},
   {"D7IA2R", 0x017E},
   {"D7SMR", 0x01F0},
   {"D7CR", 0x01F1},
   {"D7L0R", 0x01F2},
   {"D7L1R", 0x01F3},
   {"D7SA0R", 0x01F4},
   {"D7SA1R", 0x01F5},
   {"D7SA2R", 0x01F6},
   {"D7DA0R", 0x01F8},
   {"D7DA1R", 0x01F9},
   {"D7DA2R", 0x01FA},
   {"D7LA0R", 0x01FC},
   {"D7LA1R", 0x01FD},
   {"D7LA2R", 0x01FE},

   {"NADR", 0x0200},
   {"NALDR", 0x0201},
   {"NATSR", 0x0202},
   {"NARSR", 0x0203},
   {"NACSR", 0x0204},
   {"NASR", 0x0205},
   {"NARR", 0x0206},
   {"NACR", 0x0207},
   {"NAPCR", 0x0208},
   {"NATCR", 0x020A},
   {"NARCR", 0x020B},

   {"NAPA0R", 0x0210},
   {"NAPA1R", 0x0211},
   {"NAPA2R", 0x0212},
   {"NAPA3R", 0x0213},
   {"NAPA4R", 0x0214},
   {"NAPA5R", 0x0215},

   {"NAMF0R", 0x0218},
   {"NAMF1R", 0x0219},
   {"NAMF2R", 0x021A},
   {"NAMF3R", 0x021B},
   {"NAMF4R", 0x021C},
   {"NAMF5R", 0x021D},
   {"NAMF6R", 0x021E},
   {"NAMF7R", 0x021F},

   {"NAMHR", 0x0220},
   {"NACDR", 0x0221},
   {"NAAER", 0x0222},
   {"NACER", 0x0223},
   {"NAC0R", 0x0224},
   {"NAC1R", 0x0225},
   {"NAMFR", 0x0226},

   {"SWDTR", 0x0C},
};

void usage()
{
   cerr << "This is the serial flash boot loader triplet generator utility.\n";
   cerr << "Format: BL_triplets <source_name> <output_name>\n";
   cerr << "It takes the name of the source file as input (without the extension!)\n";
   cerr << "and assumes that both the .C file and .bin file exist.  Compile your\n";
   cerr << "boot loader code using dynamic C first to produce the .bin file.  The\n";
   cerr << "output will be a text file.\n";
   cerr << "Example: BL_triplets BOOTLOAD serial_flash_boot_loader.lib\n";
}

// Gets the the next line denoted by //@ in the c source file
string next_command(istream & in)
{
   string result("__EOF");
   string input;
   bool no_command = true;

   while (!in.eof() && no_command) {
      getline(in, input);
      string::size_type loc = input.find( "//@", 0 );
      if (loc != string::npos) {
         result = input.substr(loc+3);
         no_command = false;
      }
   }

   return result;
}

bool nextchar(unsigned char & b, istream & in)
{
   char input;
	int i;

   in.read(&input, 1); // Read a character
   if (in.eof()) {
      return false;
   }
   b = input;
   if (input == '\x76') {
      // This is the tricky case. I need to check for 3 more of these _AND_ EOF,
      // and restore the pointer if it's not the case.
      istream::pos_type place = in.tellg();

		for (i = 3; i; --i)
		{
			in.read(&input, 1); // Read another character
			if (in.eof() || input != '\x76') {
				in.seekg(place);
				return true;
			}
		}

      // We've successfully read 4 0x76 bytes, marking the end of the code.
      // Signal a stop.
      return false;
   }
   else {
      return true;
   }
}

void write_triplet( unsigned short address, unsigned char value)
{
	if (curr_index + 3 > MAX_LOADER_LENGTH)
	{
      cerr << "Bootloader would exceed " << MAX_LOADER_LENGTH << " bytes." << endl;
      exit(10);
	}
	loader_bytes[curr_index++] = (unsigned char) (address >> 8);
	loader_bytes[curr_index++] = (unsigned char) (address & 0xFF);
	loader_bytes[curr_index++] = (unsigned char) value;         
}	

int main(int argc, char * argv[])
{
   unsigned short start_address = 0;

   if (argc != 3) {
      usage();
      exit(1);
   }
   string c_source_name(argv[1]);
   string bin_source_name(argv[1]);
   string txt_output_name(argv[2]);

   c_source_name += ".c";
   bin_source_name += ".bin";

   // Try opening the files.
   ifstream c_source_file(c_source_name.c_str());
   if (c_source_file.fail()) {
      cerr << "I couldn't open " << c_source_name << endl;
      exit(2);
   }

   ifstream bin_source_file(bin_source_name.c_str(), ios::binary);
   if (bin_source_file.fail()) {
      cerr << "I couldn't open " << bin_source_name << endl;
      exit(3);
   }

   ofstream txt_output_file(txt_output_name.c_str());
   if (txt_output_file.fail()) {
      cerr << "I couldn't open " << txt_output_name << endl;
      exit(4);
   }
   
   // Initialize the io map.
   for (unsigned int i = 0; i < sizeof io_map_data / sizeof io_map_data[0]; ++i)
   {
      io_map[io_map_data[i].first] = io_map_data[i].second;
   }

   // First thing to do is look at the c source and find all the special
   // comments, and emit the io triplets that they specify. the "start" command
   // is a special command that signals the end of input and specifies the
   // starting address for the cold boot code.
   string command = next_command(c_source_file);
   while (command != "__EOF") {
      cout << "Got command: " << command << endl;
      stringstream ss(command);
      string io_reg;
      string value_string;

      ss >> io_reg;
      if (ss.fail()) {
         cerr << "I could not understand \"" << command
              << "\" in the c source file." << endl;
         exit(5);
      }

      ss >> hex >> value_string;
      if (ss.fail()) {
         cerr << "I could not understand \"" << command
              << "\" in the c source file." << endl;
         exit(6);
      }
      unsigned short value = static_cast<unsigned short>
         (strtoul(value_string.c_str(), NULL, 0));

      if (io_reg == "start") {
         start_address = value;
         command = "__EOF";
      }
      else {
			write_triplet( io_map[io_reg] | 0x8000, value);
         command = next_command(c_source_file);
      }
   }

   // Next we emit everything in the binary up to the 0x76767676 that marks the
   // end of the code
   unsigned char code_byte;
   unsigned short address = start_address;

	while (nextchar(code_byte, bin_source_file)) {
		write_triplet( address, code_byte);
		++address;
	}

	// This portion handles commands that follow the "start" command, and will
	// be appended to the program triplets.
	command = next_command(c_source_file);
	while (command != "__EOF") {
		cout << "Got command: " << command << endl;
		stringstream ss(command);
		string io_reg;
		string value_string;

		ss >> io_reg;
		if (ss.fail()) {
			cerr << "I could not understand \"" << command
			<< "\" in the c source file." << endl;
			exit(7);
		}

		ss >> hex >> value_string;
		if (ss.fail()) {
			cerr << "I could not understand \"" << command
			<< "\" in the c source file." << endl;
			exit(8);
		}
		unsigned short value = static_cast<unsigned short>
		(strtoul(value_string.c_str(), NULL, 0));

		if (io_reg == "patch") {
			string value2_string;
			ss >> hex >> value2_string;
			if (ss.fail()) {
				cerr << "I could not understand \"" << command
					 << "\" in the c source file." << endl;
				exit(9);
			}
			unsigned char value2 = static_cast<unsigned char>(strtoul(value2_string.c_str(), NULL, 0));
			write_triplet( start_address + value, value2);
		}
		else {
			write_triplet( io_map[io_reg] | 0x8000, value);
		}
		command = next_command(c_source_file);
	}

	// Execution will start at address 0x0000, so insert a jump to our start address.
	write_triplet( 0x0000, 0xC3);							// jp opcode
	write_triplet( 0x0001, start_address & 0xFF);	// low byte of jump destination
	write_triplet( 0x0002, start_address >> 8);		// high byte of jump destination
	write_triplet( 0x8024, 0x80);							// Set SPCR to 0x80 (and start execution)
	
	//next we need to produce the text file
	txt_output_file << "/* THIS FILE WAS AUTOMATICALLY GENERATED.  DO NOT MODIFY. */\n";
	txt_output_file << "/* Tripleted serial bootloader for Rabbit 5000 and later. */\n";
	txt_output_file << "/*** BeginHeader */\n";
	txt_output_file << "#define	SERIAL_FLASH_BOOT_LOADER \\\n\tdb 0x";
	txt_output_file << setfill('0') << setw(2) << hex << (unsigned int)loader_bytes[0];
	for (int i = 1; i < curr_index; ++i) {
		if (i % 3 == 0)
			txt_output_file << " \\\n\t";
		txt_output_file << ",  0x" << setfill('0') << setw(2) << hex << (unsigned int)loader_bytes[i];
	}
	txt_output_file << "\n";
	txt_output_file << "/*** EndHeader */\n";
}


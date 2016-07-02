#ifndef __BIOS_ALREADY_COMPILED
   #define __BIOS_ALREADY_COMPILED
#else
   #fatal "Cannot compile the BIOS as a user program!"
#endif

#use "SYSIODEFS.LIB"
#use "BOARDTYPES.LIB"   // board-specific initialization header
#use "SYSCONFIG.LIB"    // All user-defined macros for BIOS configuration

#pragma CompileBIOS

#undef MECR_VALUE
#define MECR_VALUE 0
#orgstart

#orgdef rcodorg rootcode above phy 0 log 0x6000 size 0x1800
#orgdef rvarorg rootdata above rootcode size 0x3000
#orgdef rcodorg intvec above rootdata size 0x200
// These origins must not be used by the pilot. They are here to appease the
// compiler.
#orgdef xcodorg xmemcode above intvec size 0x4000
#orgdef xvarorg xmemdata above xmemcode size 0x1000

#orgdef resvorg flash_buffer below phy 0x10000 size 0x1000

#orgend

#orgmac FLASH_BUFFER_PHYSICAL = flash_buffer physical start
#orgmac _PBINTVEC_BASE = intvec logical start

#define FLASH_BUF_0015 (FLASH_BUFFER_PHYSICAL & 0xFFFF)
#define FLASH_BUF_1619 ((FLASH_BUFFER_PHYSICAL >> 16) & 0x000F)
#define FLASH_BUF_XPC (((FLASH_BUFFER_PHYSICAL - 0xE000) >> 12) & 0xFF)
#define FLASH_BUF_ADDR (((FLASH_BUFFER_PHYSICAL & 0x0FFF) | 0xE000) & 0xFFFF)

#orgact rootcode apply
#orgact rootdata apply
#orgact xmemcode apply

enum OriginType { UNKNOWN_ORG = 0, RCODORG, XCODORG, WCODORG, RVARORG, XVARORG,
                  WVARORG, RCONORG, RESVORG };

typedef struct {
   char type;
   char flags;                //
   unsigned long  paddr;      // Physical address
   unsigned    laddr;         // Logical address
   unsigned long  usedbytes;  // Actual number of bytes used or reserved
                              // starting from paddr/laddr
   unsigned long  totalbytes; // Total space allocated
} _sys_mem_origin_t;

// These are defined by the compiler.
extern int _orgtablesize;
extern _sys_mem_origin_t _orgtable[];

#define _cexpr(X) 0+(X)

#flushlib
#flushlib

#flushcompile

#flushlib
#flushlib
#flushlib

//#pragma CompileProgram

#asm
StartUserCode::
#endasm



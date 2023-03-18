/*
   Copyright (c) 2023, Digi International Inc.

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
	extract_flash.c          v10.0.1
	
	A program you can compile to RAM on Rabbit hardware in order to dump a copy
	of the firmware stored on the flash.  You can use the Rabbit Field Utility
	(RFU) to install the firmware on a new board after converting the Base64
	dump to a binary file (using the built-in Windows command-line CERTUTIL.EXE).

	Limitations
	===========
	Newly ported from Dynamic C 9 to Dynamic C 10 with very limited testing.
	Consider this an Alpha release that still needs a lot of work.
	
	Tested on RCM4510W (512KB parallel flash, MSB_BIT of 19, 256KB quadrants).

	If you need to comment out the compiler warnings in BOARDTYPES.LIB (see
	above), this process will erase the board's non-volatile, battery-backed RAM.
	
	If extracting the entire flash, look for a large run of 0xFF bytes to find
	the end of the program.
	
	How to use it
	=============
	Run from Dynamic C and confirm that the program finds the firmware_info_t
	structure at the start of the flash.  It uses that structure to
	determine the firmware image size.  Without it, it will just dump the
	entire flash and you'll have to manually truncate it.
	
	Then run from the command line and confirm that it still works.  If you leave
	the IDE running, be sure to choose "Close Connection" from the "Run" menu
	to free up the serial port.
	
	The -mr option runs the program from RAM (to preserve flash contents) and
	the -wn option disables compiler warnings which would interfere with the
	base64 encoding of the output file.

		C:\DCRABBIT_10.72>dccl_cmp Samples\extract_flash.c -mr -wn -d VERBOSE

	Then run again with DUMP_FIRMWARE defined to create a base64-encoded file.
	NOTE: This will take minutes to complete -- be patient!  You can monitor
	progress by checking the file size in another window.
	
		C:\DCRABBIT_10.72>dccl_cmp Samples\extract_flash.c -mr -wn -d DUMP_FIRMWARE > firmware.b64

	Then use the Windows program CERTUTIL.EXE to convert it to a binary file:
	
		C:\DCRABBIT_10.72>certutil -decode firmware.b64 firmware.bin

	You should now be able to program a new board with firmware.bin using RFU.
	
	If you want an image of the entire flash, define the macro DUMP_FLASH
	instead of DUMP_FIRMWARE.  In this case, you'll need to locate the end
	of the actual firmware and truncate the file.  Expect to see a System ID
	Block at the top of the flash, User Data Blocks below that, and then a
	run of 0xFF bytes before the actual firmware image.

	Configuration Options
	=====================
	By default, this program just tries to determine the size of the firmware
	stored in the flash by looking for a structure called _firmware_info stored
	in the BIOS of the firmware.
	
	Define the macro DUMP_FIRMWARE to actually dump the firmware image (if it
	can determine the size) as Base64-encoded data.
	
	Or, define the macro DUMP_FLASH to skip image size detection and dump the
	entire flash (including UserBlock(s) and SystemIdBlock).
*/
	
#if ! (RAM_COMPILE || SUPPRESS_FAST_RAM_COPY)
	#error "Must compile to RAM so you don't overwrite the flash!"
	#fatal "Choose Options/Project Options/Compiler/Store Program In/RAM"
#endif
#if _SERIAL_BOOT_FLASH_
	#use "BOOTDEV_SF_API.lib"
#endif

#use "base64.lib"

#if !defined(DUMP_FLASH)
	#define SEARCH_FIRMWARE_INFO
#endif
#if defined(DUMP_FIRMWARE) || defined(DUMP_FLASH)
	#define DUMPING_BASE64
#endif

// Helper function copied from board_update.lib
int _buFindFirminfo(const byte __far *buffer, int bufsize)
{
	auto int addr;
	auto int maxaddr;

	maxaddr = bufsize - sizeof(firmware_info_t);
	for (addr = 0; addr <= maxaddr; addr += 64)
	{
		if ((*(__far unsigned long *)(buffer + addr)) == _FIRMINFO_MAGIC_NUMBER)
		{
			return addr;
		}
	}

	// couldn't find the firminfo structure
	return -ENODATA;
}

int read_flash(void __far *dest, unsigned long offset, int bytes)
{
	int retval;
	#if _SERIAL_BOOT_FLASH_
		do {
			retval = sbf_far_Read(dest, offset, (unsigned) bytes);
		} while (retval > 0);
	#else
		_f_memcpy(dest, (void __far *)((1LU<<MSB_BIT) + offset), bytes);
	#endif
	return bytes;
}

// keep a buffer of the start of flash so we can hopefully find the prog_param
char bios_buffer[1024+4];

#define CHUNK_SIZE 57   // 57 bytes allows for 76-byte lines of base64 output
int main()
{
	char base64_buffer[((CHUNK_SIZE+2)/3*4)+1];
	char flash_buffer[CHUNK_SIZE];
	const __far firmware_info_t *fi = NULL;
	char *p, *end;
	int i;
	unsigned int copy;
	unsigned long addr;
	unsigned long dump_bytes, firmware_size;
	
	unsigned long flash_bytes;
		
	flash_bytes = (SysIDBlock.flashSize + SysIDBlock.flash2Size) << 12lu;
	#if _SERIAL_BOOT_FLASH_
		flash_bytes = SBF_FLASH_BYTES;
	#else
		// map flash into MB2 and MB3 quadrant
		WrPortI(MB2CR, &MB2CRShadow, FLASH_WSTATES | 0x00);
		WrPortI(MB3CR, &MB3CRShadow, FLASH_WSTATES | 0x00);
      // re-initialize flash driver so it knows flash is in second two quadrants
      _InitFlashDriver((1ul << (MSB_BIT + 1)) - (1ul << (MSB_BIT - 1)),
      	(1ul << (MSB_BIT + 1)) - (1ul << (MSB_BIT - 1)));
		flash_bytes = _FLASH_SIZE_ * 4096UL;
	#endif

	dump_bytes = flash_bytes;

#ifdef SEARCH_FIRMWARE_INFO
	read_flash(bios_buffer, 0, sizeof bios_buffer);
	i = _buFindFirminfo(bios_buffer, sizeof bios_buffer);
	if (i > 0) {
		fi = (void *)&bios_buffer[i];
		dump_bytes = fi->length;
	}	
#ifndef DUMPING_BASE64
	if (i < 0) {
		printf("Did not locate firmware_info_t; unknown image size.\n");
	} else {
		printf("Found firmware_info_t at 0x%04X.\n", i);
		fiDump(fi);
	}
#endif // ! DUMPING_BASE64
#endif // SEARCH_FIRMWARE_INFO

#ifndef DUMPING_BASE64
	if (dump_bytes > flash_bytes) {
		printf("Calculated %lu-byte firmware larger than %lu-byte flash\n",
			dump_bytes, flash_bytes);
	} else if (dump_bytes != flash_bytes) {
		printf("Define DUMP_FIRMWARE to extract %lu-byte firmware image.\n",
			dump_bytes);
	}
	printf("Define DUMP_FLASH to extract entire %uKB flash.\n",
		(unsigned)(flash_bytes / 1024));
	return 0;
#endif // ! DUMPING_BASE64
	
	addr = 0;
	while (dump_bytes) {
		copy = CHUNK_SIZE;
		if (copy > dump_bytes) {
			copy = (unsigned int) dump_bytes;
		}
		
		read_flash(flash_buffer, addr, copy);
		base64_encode(base64_buffer, flash_buffer, copy);
		printf("%s\n", base64_buffer);
		
		dump_bytes -= copy;
		addr += copy;
	}
	
	return 0;
}



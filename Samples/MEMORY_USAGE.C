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
	memory_usage.c

	A useful program to show memory usage.

	For best table format results use "Terminal" for the Stdio window font.
*******************************************************************************/
#class auto

void main(void)
{
	unsigned long addr, shad;
	unsigned oaddr, oshad, caddr, cshad, dot;

	//********** Show Flash Present ***************
	printf("Dynamic C detects a flash of %08lx bytes.\n",
	       (long) (_FLASH_SIZE_) << 12);
#if _FLASH_SIZE_ != FLASH_SIZE
	printf("   However, the BIOS set FLASH_SIZE to %08lx bytes.\n",
	       (long) (FLASH_SIZE) << 12);
#endif
#if _SERIAL_BOOT_FLASH_
	printf("   This program is compiled to serial flash and run in RAM.\n\n");
#elif FLASH_COMPILE
	printf("   This program is compiled to parallel flash and run in flash.\n");
	printf("   Parallel flash memory starts at physical address 0x000000.\n\n");
#elif RAM_COMPILE || SUPPRESS_FAST_RAM_COPY
	printf("   This program is compiled to RAM and run in RAM.\n");
	printf("   Parallel flash memory is mapped into the MB3CR quadrant.\n\n");
#else
	printf("   This program is compiled to parallel flash and run in RAM.\n");
	printf("   Parallel flash memory is mapped into the MB3CR quadrant.\n\n");
#endif

	//********** Show RAM Present ***************
	printf("Dynamic C detects a RAM of %08lx bytes.\n",
	       (long) (_RAM_SIZE_) << 12);
#if _RAM_SIZE_ != RAM_SIZE
	printf("   However, the BIOS set RAM_SIZE to %08lx bytes.\n",
	       (long) (RAM_SIZE) << 12);
#endif
	printf("   RAM starts at physical address %08lx.\n\n",
	       (long) (RAM_START) << 12);

	//********** Display separate I&D mode selection ***************
#if __SEPARATE_INST_DATA__
	printf("Separate instruction and data space is enabled.\n");
#else
	printf("Separate instruction and data space is not enabled.\n");
#endif

	printf("\nStandard MEM.LIB tables...\n\n");

	//********** Display MMU/MIU Registers ***************
	mmu_miu_regs();

	//********** Display Memory Usage Tables ***************
	memory_usage_table();
	basic_program_stats();
	printf("\n");
	orgtable_stats(NULL, 0);
	printf("\n");
	xalloc_stats(xubreak);

	printf("\nDynamic allocation (malloc)...\n\n");

	printf("_SYS_MALLOC_BLOCKS = %lu (=%lu bytes)\n",
					(long)_SYS_MALLOC_BLOCKS,
					(long)_SYS_MALLOC_BLOCKS * 4096);
	printf("_APP_MALLOC_BLOCKS = %lu (=%lu bytes)\n",
					(long)_APP_MALLOC_BLOCKS,
					(long)_APP_MALLOC_BLOCKS * 4096);
	printf("_MALLOC_SYS_EXIT_ON_ERROR = %d\n", _MALLOC_SYS_EXIT_ON_ERROR);
	printf("_MALLOC_APP_EXIT_ON_ERROR = %d\n", _MALLOC_APP_EXIT_ON_ERROR);
	_init_sys_mem_space();
	_init_app_mem_space();
	_init_root_mem_space();
	printf("\nXALLOC table after malloc initialization (compare to above)...\n\n");
	xalloc_stats(xubreak);

	#ifdef _MALLOC_HWM_STATS
	printf("\nCurrent system (DC library) malloc usage...\n");
	_sys_malloc_stats();
	printf("\nCurrent application malloc usage...\n");
	malloc_stats();
	printf("\nCurrent root malloc usage...\n");
	_root_malloc_stats();
	#else
	printf("\nNote: add _MALLOC_HWM_STATS to the project defines box to\n"
			" print additional dynamic memory allocation use.\n");
	#endif

	printf("\nMemory shadow detection...\n");
	printf("Note: performed on 64k block boundaries.  Empty if no lower\n");
	printf(" shadow block, else is lowest shadow block number detected (hex)\n");
	printf(" Dot means block was not writable\n");

	printf("\nMeg  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	printf(  "---  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n");
	for (addr = 0; addr < 0x400000; addr += 0x10000) {
		if (!(addr & 0xF0000))
			printf("%3lu  ", addr >> 20);
		dot = 0;
		for (shad = 0; shad < addr; shad += 0x10000) {
		#asm
			ld	px,(sp+@sp+addr)
			ld py,(sp+@sp+shad)
			ipset 3
			ld	a,(py)
			ld	e,a
			ld	a,(px)
			ld d,a
			cpl
			ld (px),a
			ld a,(px)
			ld b,a
			ld	a,(py)
			ld c,a
			ld a,d
			ld (px),a
			ipres
			clr hl
			ld l,e
			ld (sp+@sp+oshad),hl	; original at shadow
			ld l,d
			ld (sp+@sp+oaddr),hl	; original at addr
			ld l,c
			ld (sp+@sp+cshad),hl ; complement of addr, retrieved at shadow
			ld l,b
			ld (sp+@sp+caddr),hl ; complement of addr

		#endasm
			if (oshad != cshad)
				break;
			dot |= (caddr & 0xFF) != (~oaddr & 0xFF);
		}
		if (shad < addr)
			printf("%02lX%c", shad >> 16, dot ? '.' : ' ');
		else
			printf("  %c", dot ? '.' : ' ');
		if ((addr & 0xF0000) == 0xF0000)
			printf("\n");

	}

}


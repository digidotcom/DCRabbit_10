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
/*
	Samples/RemoteProgramUpdate/compare_firmware.c

	This is a modified version of verify_firmware.c that reads 4KB blocks of
	the Boot and Running firmware images for comparison, and to identify
	memory locations where the Running firmware no longer matches the Boot
	firmware.
	
	You can use this program to narrow down the corrupted addresses if
	verify_firmware.c reports a problem with the running program, which
	could happen if a coding error results in writing to memory used
	for program storage.
*/

#if CC_FLAGS_RAM_COMPILE
	// Menu item -- Options: Project Options: Compiler: Store Program in: Flash
	#fatal "This sample must be compiled to flash."
#endif
#if !_RUN_FROM_RAM
	#fatal "This sample was designed for hardware that runs code from RAM."
#endif

#use "board_update.lib"
#use "crc32.lib"

int compare_firmware(int verbose)
{
	#define BLOCKSIZE 512
	#define BLOCKCOUNT ((MAX_FIRMWARE_BINSIZE + BLOCKSIZE - 1) / BLOCKSIZE)
	static uint32 crc_cache[BLOCKCOUNT];
	static char block_buffer[BLOCKSIZE];
	
	auto int err, pass, bytes_read, block;
	auto int blocks_changed = 0;
	auto uint32 crc;

	// Three-pass verification of firmware:
	// Pass 1: Store crc32 of BLOCKSIZE blocks of Running firmware.
	// Pass 2: Compare each block to crc32 of Boot firmware.
	// Pass 3: Dump out Running firmware blocks that don't match boot.
	for (pass = 0; pass < 3; ++pass)
	{
		if (pass == 1) {
			err = buOpenFirmwareBoot(BU_FLAG_NONE);
		} else {
			err = buOpenFirmwareRunning(BU_FLAG_NONE);
		}
		if (err)
		{
			if (verbose)
			{
				printf("error %d calling %s\n", err,
					pass == 1 ? "buOpenFirmwareBoot" : "buOpenFirmwareRunning");
			}
			return err;
		}
		for (block = 0; block < BLOCKCOUNT; ++block)
		{
			bytes_read = buReadFirmware(block_buffer, BLOCKSIZE);
			if (bytes_read == -EEOF || bytes_read == 0)
			{
				if (verbose)
				{
					printf("ending pass %d on block %d\n", pass, block);
				}
				break;
			}
			else if (bytes_read < 0)
			{
				if (verbose)
				{
					printf("error %d reading block %d in pass %d\n",
						bytes_read, block, pass);
				}
				return err;
			}
			
			crc = crc32_calc(block_buffer, bytes_read, 0);
			if (pass && crc != crc_cache[block])
			{
				if (pass == 1)
				{
					++blocks_changed;
				}
				if (verbose)
				{
					printf("Pass %d, offset 0x%x (crc=0x%08lx):\n",
						pass, block * BLOCKSIZE, crc);
					if (verbose > 1)
					{
						mem_dump(&block_buffer, bytes_read);
					}
				}
			}
			crc_cache[block] = crc;
		}
		buCloseFirmware();
	}
	
	return blocks_changed;
}

void main()
{
	firmware_info_t	fi;
	int err;

	err = fiProgramInfo(&fi);
	if (err)
	{
		printf("error %d calling %s\n", err, "fiProgramInfo");
		exit(err);
	}

	printf("Firmware information embedded in BIOS:\n");
	fiDump(&fi);

	err = compare_firmware(2);
	if (err < 0)
	{
		printf("error %d calling %s\n", err, "compare_firmware");
	}
	else if (err > 0)
	{
		printf("%s reported %d blocks changed\n", "compare_firmware", err);
	}
}
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
	Samples/RemoteProgramUpdate/verify_firmware.c

	Verify that the firmware image stored on the boot flash is valid, and verify
	that the program running in fast SRAM is valid as well.  Works on all boards,
	not just the ones supported by Remote Program Update.  View function
	help on firmware_info_t to learn about the elements of that structure,
	and look at the source to fiDump (in firmware_info.lib) to see how
	it makes use of that information.
*/

#if CC_FLAGS_RAM_COMPILE
	// Menu item -- Options: Project Options: Compiler: Store Program in: Flash
	#fatal "This sample must be compiled to flash."
#endif

#use "board_update.lib"

int verify_firmware( const char *label)
{
	auto int error;
	auto int progress;

	printf( "\n\nVerifying %s firmware.\n", label);

	progress = 0;
	do {
      printf( "\r verify %u.%02u%%\r", progress / 100, progress % 100);
		error = buVerifyFirmware( &progress);
	} while (error == -EAGAIN);

   if (error)
   {
      printf( "\n verify error %d\n", error);
      return error;
   }

   printf( " verify complete\n");
	return 0;
}

void main()
{
	firmware_info_t	fi;
	int err;

	err = fiProgramInfo( &fi);
	if (err)
	{
		printf( "error %d calling %s\n", err, "fiProgramInfo");
		exit(err);
	}

	printf( "Firmware information embedded in BIOS:\n");
	fiDump( &fi);

	err = buOpenFirmwareBoot( BU_FLAG_NONE);
	if (err)
	{
		printf( "error %d calling %s\n", err, "buOpenFirmwareBoot");
		exit(err);
	}
	verify_firmware( "boot");

	#if _RUN_FROM_RAM
		// On boards that run firmware from RAM instead of flash, you can use
		// buOpenFirmwareRunning to verify that the code and constants from
		// the firmware image remain unchanged.
	   err = buOpenFirmwareRunning( BU_FLAG_NONE);
	   if (err)
	   {
	      printf( "error %d calling %s\n", err, "buOpenFirmwareRunning");
	      exit(err);
	   }
	   verify_firmware( "running");
	#endif
}
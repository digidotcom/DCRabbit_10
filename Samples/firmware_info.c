/*
	Samples/firmware_info.c
	Digi International, Copyright ©2009.  All rights reserved.

	Display this program's firmware info contents.  Works on all boards,
	not just the ones supported by Remote Program Update.  View function
	help on firmware_info_t to learn about the elements of that structure,
	and look at the source to fiDump (in firmware_info.lib) to see how
	it makes use of that information.
*/

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
}
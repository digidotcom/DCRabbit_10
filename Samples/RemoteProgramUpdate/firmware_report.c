/*
	RemoteProgramUpdate/firmware_report.c
	Digi International, Copyright ©2009.  All rights reserved.

	Be sure to define BU_ENABLE_SECONDARY in your project defines if you're
	using the Powerfail-safe version of Remote Program Update.  Otherwise,
	this program won't be able to read the A and B markers on the flash.

	Compile this sample to RAM to have it report on what firmware image(s) are
	currently installed on the flash.  Only compatible with boards using a
	serial boot flash (e.g. RCM4300 series, BL4S100 series, BL4S200, RCM5600W
	series and Rabbit 6000 based boards).
*/

#if ! _SERIAL_BOOT_FLASH_
	#fatal "This sample only works on boards with a serial boot flash."
#endif
#if ! (RAM_COMPILE || SUPPRESS_FAST_RAM_COPY)
	#fatal "This sample must be compiled to RAM instead of flash."
#endif

#use "board_update.lib"

// Returns 1 if this board will boot from a secondary image (either A or B).
// Returns 0 if it will boot from the original Z-image installed on the device.
int secondary_report()
{
#ifndef BU_ENABLE_SECONDARY
   return 0;       // no A-image or B-image on device
#else
	firmware_marker_t		a_marker, b_marker;
	int a_valid, b_valid;
	char boot;
	int error;

	// check the A marker first
	error = buMarkerRead( &a_marker, BU_FLAG_IMAGE_A);
	if (! error)
	{
		error = buMarkerRead( &b_marker, BU_FLAG_IMAGE_B);
	}

	if (error)
	{
		printf( "error %d reading markers\n", error);
		return error;
	}

	a_valid = (buMarkerVerify( &a_marker) == 0);
	b_valid = (buMarkerVerify( &b_marker) == 0);

	boot = 'Z';
	if (a_valid)
	{
		boot = 'A';
		printf( "\nFirmware installed as A-image:\n");
		buMarkerDump( &a_marker);
	}
	else
	{
		printf( "\nThere is not a valid A-image installed on this flash.\n");
	}

	if (b_valid)
	{
      if (! a_valid || b_marker.sequence - a_marker.sequence < 0x8000u)
      {
      	boot = 'B';
      }
		printf( "\nFirmware installed as B-image:\n");
		buMarkerDump( &b_marker);
	}

	printf( "\n\nThis board will boot from the %c-image.\n\n", boot);

	return (a_valid || b_valid);
#endif
}

void main()
{
	firmware_marker_t		marker;
	int secondary;

   secondary = secondary_report();

	// Have board_update.lib create a virtual marker for the Z-image (the
	// firmware image installed by Dynamic C or the RFU).
	buMarkerBuild( &marker, 0, 0);

	if (secondary)
	{
		printf( "Bootloader (first 1KB of flash) came from:\n");
		fiDump( &marker.firminfo);
	}
	else
	{
		printf( "Firmware installed as Z-image:\n");
	   buMarkerDump( &marker);
	}

}
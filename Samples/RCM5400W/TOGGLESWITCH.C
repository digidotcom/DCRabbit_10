/*******************************************************************

	toggleswitch.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

	This program is used with RCM5400W series controllers
	with prototyping boards.

	Description
	===========
	This program uses costatements to detect switches with
	press and release method debouncing. Corresponding LED's,
	DS2 and DS3, will turn on or off.

	I/O control			On proto-board
	--------------		----------------------
	Port B bit 2		DS2, LED
	Port B bit 3		DS3, LED
	Port B bit 4		S2, switch
	Port B bit 5		S3, switch

	Instructions
	============
	1. Compile and run this program.
	2. Press and release S2 switch to toggle DS2 LED on/off.
	3. Press and release S3 switch to toggle DS3 LED on/off.

*******************************************************************/
#class auto

#use RCM54xxW.LIB

#define DS2_BIT 2
#define DS3_BIT 3
#define S2_BIT  4
#define S3_BIT  5

main()
{
	auto int sw2, sw3, led2, led3;

	// Initialize I/O pins
	brdInit();

	led2=led3=1;			//initialize leds to off value
	sw2=sw3=0;				//initialize switches to false value

	while (1)
	{
		costate
		{
			if (BitRdPortI(PBDR, S2_BIT))		//wait for switch S2 press
				abort;
			waitfor(DelayMs(50));					//switch press detected if got to here
			if (BitRdPortI(PBDR, S2_BIT))		//wait for switch release
			{
				sw2=!sw2;								//set valid switch
				abort;
			}
		}

		costate
		{
			if (BitRdPortI(PBDR, S3_BIT))		//wait for switch S3 press
				abort;
			waitfor(DelayMs(50));					//switch press detected if got to here
			if (BitRdPortI(PBDR, S3_BIT))		//wait for switch release
			{
				sw3=!sw3;								//set valid switch
				abort;
			}
		}

		costate
		{	// toggle DS2 led upon valid S2 press/release and clear switch
			if (sw2)
			{
				BitWrPortI(PBDR, &PBDRShadow, led2=led2?0:1, DS2_BIT);
				sw2=!sw2;
			}
		}

		costate
		{	// toggle DS3 upon valid S3 press/release and clear switch
			if (sw3)
			{
				BitWrPortI(PBDR, &PBDRShadow, led3=led3?0:1, DS3_BIT);
				sw3=!sw3;
			}
		}

	}
}
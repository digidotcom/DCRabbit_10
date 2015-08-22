/*******************************************************************

	flashled2.c
  	Rabbit Semiconductor, 2007

	This program is used with RCM4400W series controllers
	with prototyping boards.

	Description
	===========
	This program uses cofunction and costatements to flash
	LED's, DS2 and DS3, on the prototyping board at different
	intervals.

	I/O control			On proto-board
	--------------		----------------------
	Port B bit 2		DS2, LED
	Port B bit 3		DS3, LED

	Instructions
	============
	1.  Compile and run this program.
	2.  DS2 and DS3 LED's flash on/off at different times.

*******************************************************************/
#class auto

#use RCM44xxW.LIB

#define DS2 0
#define DS3 1

#define DS2_BIT 2
#define DS3_BIT 3

///////////////////////////////////////////////////////////
// DS2 led on protoboard is controlled by PB2
// DS3 led on protoboard is controlled by PB3
// turns oon when port bit is set to 0
///////////////////////////////////////////////////////////
void pbledon(int led)
{
	if(led == DS2)
   	BitWrPortI(PBDR, &PBDRShadow, 0, DS2_BIT);
	else
   	BitWrPortI(PBDR, &PBDRShadow, 0, DS3_BIT);
}

///////////////////////////////////////////////////////////
// DS2 led on protoboard is controlled by PB3
// DS3 led on protoboard is controlled by PB3
// turns off when port bit is set to 1
///////////////////////////////////////////////////////////
void pbledoff(int led)
{
	if(led == DS2)
   	BitWrPortI(PBDR, &PBDRShadow, 1, DS2_BIT);
	else
   	BitWrPortI(PBDR, &PBDRShadow, 1, DS3_BIT);
}

///////////////////////////////////////////////////////////
// set DS2 and DS3 leds to stay on and off at intervals
///////////////////////////////////////////////////////////
cofunc flashled[2](int led, int ontime, int offtime)
{
	for(;;)
	{
		pbledon(led);
		waitfor(DelayMs(ontime));
		pbledoff(led);
		waitfor(DelayMs(offtime));
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
main()
{
	// Initialize I/O pins
	brdInit();

	for(;;)
	{
		costate
		{
			wfd
			{
				flashled[0](DS2, 50, 50);    	// Flashes DS2 on/off 50 ms
				flashled[1](DS3, 500, 100);   // Flashes DS3 on 200 ms, off 100 ms
			}
		}
	}
}
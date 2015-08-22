/*******************************************************************

	flashled1.c
  	Rabbit Semiconductor, 2007

   This program is used with RCM4500W series controllers
   with prototyping boards.

   Description
   ===========
   This assembly program uses costatements to flash LED's,
   DS2 and DS3, on the prototyping board at different intervals.
   brdInit() is not called in this demonstration.

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
#use RCM45xxW.lib

main()
{
	brdInit();
	while (1)
	{
		costate
		{	// DS2 LED
			DS2led(ON);							//on for 50 ms
			waitfor(DelayMs(50));
			DS2led(OFF);						//off for 100 ms
			waitfor(DelayMs(100));
		}

		costate
		{	// DS3 LED
			DS3led(ON);							//on for 200 ms
			waitfor(DelayMs(200));
			DS3led(OFF);						//off for 50 ms
			waitfor(DelayMs(50));
		}
	}
}
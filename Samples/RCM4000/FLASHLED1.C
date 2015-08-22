/*******************************************************************

	flashled1.c
  	Rabbit Semiconductor, 2006

   This program is used with RCM4000 series controllers
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

#define DS2 2
#define DS3 3

#define ON  0		//state to turn on led
#define OFF 1		//state to turn off led

///////////////////////////////////////////////////////////
// Initializes the two pins used for DS2 and DS3
// See brdInit() for other port initializing
///////////////////////////////////////////////////////////
void initPort()
{
#asm
	ld		a, (PBDRShadow)
	res	DS2, a
	res	DS3, a
	ld		(PBDRShadow), a
	ioi	ld (PBDR), a
	ld		a, (PBDDRShadow)
	set	DS2, a
	set	DS3, a
	ld		(PBDDRShadow), a
	ioi	ld (PBDDR), a
#endasm
}

///////////////////////////////////////////////////////////
// DS2 led on protoboard is controlled by port B2
// turns on if state = 0
// turns off if state = 1
///////////////////////////////////////////////////////////
void DS2led(int state)
{
	if (state == ON)
	{
#asm
	ld		a, (PBDRShadow)   ;read port to keep other bit values
	res	DS2, a				;clear bit
	ld		(PBDRShadow), a
	ioi	ld (PBDR), a		;write data to port
#endasm
	}
	else
	{
#asm
	ld		a, (PBDRShadow)      ;read port to keep other bit values
	set	DS2, a					;set bit
	ld		(PBDRShadow), a
	ioi	ld (PBDR), a			;write data to port
#endasm
	}
}

///////////////////////////////////////////////////////////
// DS3 led on protoboard is controlled by port B3
// turns on if state = 0
// turns off if state = 1
///////////////////////////////////////////////////////////
void DS3led(int state)
{
	if (state == ON)
	{
#asm
	ld		a, (PBDRShadow)   ;read port to keep other bit values
	res	DS3, a				;clear bit
	ld		(PBDRShadow), a
	ioi	ld (PBDR), a		;write data to port
#endasm
	}
	else
	{
#asm
	ld		a, (PBDRShadow)   ;read port to keep other bit values
	set	DS3, a			 	;set bit
	ld		(PBDRShadow), a
	ioi	ld (PBDR), a		;write data to port
#endasm
	}
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
main()
{
	initPort();

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
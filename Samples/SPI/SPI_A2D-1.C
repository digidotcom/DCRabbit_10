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
	Samples\SPI\spi_a2d-0.c

	This program has been written by the Technical Support Staff at Z-World in
	response to several customer requests.  As such, it has NOT had the testing and
	validation procedures which our "standard" software products have.  It is being
	made available as a sample.  There is no warranty, implied or otherwise.

	This program will set up and read an LTC1294 12 bit A/D convertor which uses an SPI.
	It uses serial port B in its synchronous mode and uses PD0 for the chip select.

	IMPORTANT: The accuracy is very dependant on the reference voltages used.
	They should be filtered adequately and be very stable.  This sample uses
	the power supply voltage (5.0V) and ground.
*/
#class auto

float ReadAD ( int Channel, int Samples );
int SwapBytes ( int i );

float ScaleFactor;

#define SPI_SER_B
#define SPI_CLK_DIVISOR			5
#use SPI.LIB

///////////////////////////////////////////////////////////////////////

void main ()
{
	int Value;
	float volts;

// set up chip select port
	BitWrPortI ( PDDR, &PDDRShadow, 1, 0 );	// turn off /CS (1=off)
	BitWrPortI ( PDDCR, &PDDCRShadow, 0, 0 );	// bit 0 = "normal" output
	WrPortI ( PDCR, &PDCRShadow, 0 );			// bits 0..3 = clocked by perclk/2
	BitWrPortI ( PDDDR, &PDDDRShadow, 1, 0 );	// bit 0 = output

	ScaleFactor = 5.0/4096.0;
	SPIinit();

	while (1)
	{
		volts = ReadAD ( 1, 500 );			// average 500 readings from channel 1
		printf ( "Value = %5.3f  \r", volts );
	}
}


/* ReadAD - this function will command the LTC1294 to take a reading
on the selected single ended channel.  It will take the average of the
specified number of readings and convert the value to volts using the
predefined scale factor.

Input Parameter 1: (int) channel from 0..7
Input parameter 2: (int) number of reading to average
Input Global: (float) ScaleFactor
Return value: (float) volts

In order to scale the returned value to a real voltage you will
need to multiply the result by the scale factor: (Ref+ - Ref-)/4096.

This sample has been designed with the following:
1) take single ended, unipolar readings
2) use power as Ref+ and ground as Ref-.
3) single power supply (5v)

A single reading takes about 68 usec with an 18 MHz CPU and SPI_CLK_DIVISOR of 2.
There is about 37 usec between consecutive samples.
*/

#define START		0x80
#define SINGLE		0x40
#define UNIPOLAR	0x04
#define MSBFIRST	0x02
#define POWERON	0x01


float ReadAD ( int Channel, int Samples )
{
	int Count, Command, j;
	unsigned long i;
	float Voltage;
	struct									// structure for returned data
	{	char	b;
		int	i;
	} Data;

	Command = START|SINGLE|UNIPOLAR|POWERON|MSBFIRST | ((Channel/2)<<3) | ((Channel&1)<<5);
	Command <<= 5;							// shift so result comes back LSB justified
	Command = SwapBytes ( Command );	// adjust so high byte goes first
	
	i = 0L;									// init accumulator

	for ( Count = 1; Count<=Samples; Count++ )
	{	BitWrPortI ( PDDR, &PDDRShadow, 0, 0 );	// enable /CS
		SPIWrRd ( &Command, &Data, 3 );				// 3rd xmit byte is "don't care"
		BitWrPortI ( PDDR, &PDDRShadow, 1, 0 );	// turn off /CS (1=off)
		j = Data.i;
		j = SwapBytes ( j ) & 0x0FFF;					// put LSB first and keep only 12 bits
		i += j;												// update accumulator
	}

	Voltage = (float) i * ScaleFactor / (float) Samples;		
}

// swap the high and low bytes of an int
// enter and exit with the int in HL
#asm
SwapBytes::
	ld		a, L		; save the low byte
	ld		L, H		; copy high byte to low byte
	ld		H, A		; copy saved low byte to high byte
	ret
#endasm

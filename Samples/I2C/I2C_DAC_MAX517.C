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
/****************************************************************************
	Samples\I2C\i2c_dac_MAX517.c

	Tests I2CSetDAC function. The Rabbit must be connected to a MAX517/518/519
	chip with pull-up resistors on both the clock(SCL) and data(SDA) lines.
	The I2C lines are defined in I2C.LIB. They default to:
		SCL - PD6
		SDA - PD7

	This sample will cycle through DAC values 0-255 creating a sawtooth output.
	The DAC_ADDRESS may need to be modified to reflect the current device
	address of the DAC chip.

****************************************************************************/
#class auto

#use "I2C_DEVICES.LIB"

#define DAC_ADDRESS 0x2c
int I2CSetDAC(char, char, char);

void main()
{
	int i;

	i2c_init();

	while(1)
	{
		for(i = 0;i < 256;i++)
		{
			if(I2CSetDAC(DAC_ADDRESS, 0, i))
				printf("Failed to send to channel 0\n");
		}
	}
}


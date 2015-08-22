/****************************************************************************
	Samples\I2C\i2c_dac_MAX517.c
	ZWorld, 2001

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


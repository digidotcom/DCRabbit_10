/*************************************************************************
	Samples\SPI\spi_test.c

	ZWorld, 2001

	test out SPI driver with an NS ADC0831 chip. Uses serial channel B for
	the SPI data.

	PB7 acts as the CS line on the ADC
	PB0 is the serial B clock line(SCLK)

	PC4 is the data output(MOSI)
	PC5 is the data input(MISO)

	Reads two bytes worth with each chip select.
	The first two bits are not part of the data. They are always 1 and
	then 0 .  This is followed by 8 bits of data for the sample, and
	then 6 extra bits.
  
************************************************************************/
#class auto


#define SPI_SER_B
#define SPI_CLK_DIVISOR 100

#use "spi.lib"

void main()
{
	char adc_reading[2];
	int i;
	int adc_sample;
	
	SPIinit();
	while(1)
	{
		//hold CS low for conversion time
		for(i = 0;i < 10;i++)
		{
			BitWrPortI(PBDR, &PBDRShadow, 0, 7);	// chip select low
		}
		
		SPIRead(adc_reading, 2);
		BitWrPortI(PBDR, &PBDRShadow, 1, 7);	// chip select high
		printf("SPI bytes = 0x%x 0x%x\n", adc_reading[0], adc_reading[1]); 		
		adc_sample = (adc_reading[0] <<2 ) + (adc_reading[1] >> 6) & 0xff;
		printf("ADC value: %d\n", adc_sample);	
	}
}

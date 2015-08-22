/******************************************************************
	Samples\I2C\i2c_24LC16.c
	ZWorld, 2001

	sample I2C interface with a Microchip 24LC16 serial EEPORM.
	The I2C library defaults to using PD6 as SCL and PD7 as SDA. These
	should be connected to the EEPROM chip with pull up resistors.

	This sample will attempt to write a string to the beginning of the
	memory space and then read it back.

******************************************************************/
#class auto

#use "i2c_devices.lib"

//number of milliseconds to wait between write operations
#define WRITE_TIME 5

//I2C device address of EEPROM chip
#define EEPROM_ADDRESS 0xA2

char test_string[] = "Hey There!";

void main()
{
	int return_code;
	char read_string[20];
	unsigned long t;

	i2c_init();

	return_code =
		I2CWrite(EEPROM_ADDRESS, 0, test_string, strlen(test_string));
	printf("I2CWrite returned:%d\n", return_code);

	t = MS_TIMER;
	while((long)(MS_TIMER - t) < WRITE_TIME);

	return_code =
		I2CRead(EEPROM_ADDRESS, 0, read_string, strlen(test_string));
	printf("I2Cread returned:%d\n", return_code);

	read_string[strlen(test_string)] = 0;
	printf("Read:%s\n", read_string);

}
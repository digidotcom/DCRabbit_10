/***************************************************************************
	adc_cal_diff.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
   This program demonstrates how to recalibrate a differential
   ADC channel using two known voltages to generate calibration
   constants for the given channel and will write the data into
   reserved eeprom.

	This program will also continuously display the voltage of
	the channel that was calibrated.

	Connections:
	============
	For this calibration procedure you will need a power supply
	that has one floating output and a second output or a second
   power supply.

	NOTE:	Before doing the following steps, set the power supply(s)
	      to zero volts and then turn it OFF.

   1. Remove all jumpers on header JP4.

	2. Initially connect the positive side of the floating power supply to AGND
   	and the negative side to the negative side of one of the following ADC
      differential channel pairs.  This will create a negative voltage.

      Channel   	DIFF Pairs on J12
	    -------  -----------------------------
	      0 		 +AIN0 (pin 12)  -AIN1 (pin 5)
	      2		 +AIN2 (pin 11)  -AIN3 (pin 4)
	      4		 +AIN4 (pin 3)	  -AIN5 (pin 9)
	      6		 +AIN6 (pin 2)	  -AIN7 (pin 8)

	Note: AGND is available on J12 on pins: 1, 6, 10, and 14.

   3. Connect the negative side of the other power supply to AGND and the
   	positive side to the positive side of one of the ADC differential
      channel pairs from above.

   4. Connect a voltmeter across the differential pair to monitor the total
   	voltage across the ADC channel being calibrated.
		(For best results use a 4 1/2 digit voltmeter)

	Example connections for channel 0:
	   Floating Power Supply	Power Supply 2			Voltmeter
		---------------------	--------------		----------------
      +	<->   AGND				+	<->	AIN0		+V		<->   AIN0
	   -	<->   AIN1				-	<->	AGND		COM	<->	AIN1


	Instructions:
	=============
	1. Power-on the controller.
	2. Compile and run this program.
	3. Turn ON the power supply for the ADC calibration.
	4. Follow the instructions as displayed.

***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include BLxS2xx series library
#use "BLxS2xx.lib"

#define MAX_ERRORS	2  // maximum number of errors allowed during 10 reads
#define NUM_SAMPLES	10 // number of samples for getting an average read.

const float vmax[] = {
	20.0,
	10.0,
	5.0,
	4.0,
	2.5,
	2.0,
	1.25,
	1.00
};

// blank the stdio screen
void  blankScreen(void)
{
   printf("\x1Bt");
}

void printrange()
{
	int i;
	printf("\ngain_code\tVoltage range\n");
	printf("---------\t-------------\n");
	for (i = 0; i < BL_MAX_GAINS; ++i)
   {
		printf("    %d    \t +- %5.2fv\n", i, vmax[i]);
   }
}

int anaInAvg(int channel, int gaincode, int num_samples)
{
	long value;
   int rawdata;
   int error_count;
   int i;

   value = 0;
   error_count = 0;
   for (i = 0; i < num_samples; ++i)
   {
      rawdata = anaIn(channel, gaincode);
      if (rawdata < BL_ERRCODESTART)
      {
      	// error during read
         if (++error_count > MAX_ERRORS)
         {
         	printf("Too many ADC errors.  Exiting.");
            exit(rawdata);
         }
         else
         {
         	// re-read value
         	--i;
         }
      }
      else
      {
      	value += rawdata;
      }
   }
   value = (value + (num_samples / 2)) / num_samples;
	return (int) value;
}

void main ()
{
	int value1, value2;
	int key;
	int gaincode, channel;
	float voltage, volts1, volts2;
	float cal_voltage;
	char buffer[64];
   int errorCode;

   // Initialize the controller
	brdInit();

   printf("\n !!!Caution this will overwrite the calibration constants set "\
   				 "at the factory.");
   printf(" Do you want to continue(Y/N)?");

	do {
		while(!kbhit());
		key = getchar();
		if (key == 'N' || key == 'n')
		{
			exit(0);
		}
	} while(key != 'Y' && key != 'y');

	while (1)
	{
	   blankScreen();
      printf("This sample will calibrate a differential bipolar ADC channel.");
		printf("\n\nChannel Code\tInputs\n");
		printf("------------\t-------------\n");
		printf("\t0\t+AIN0 -AIN1\n");
		printf("\t2\t+AIN2 -AIN3\n");
		printf("\t4\t+AIN4 -AIN5\n");
		printf("\t6\t+AIN6 -AIN7\n\n");
		printf("Choose the differential ADC channel 0, 2, 4, or 6 = ");
		do
		{
			channel = getchar() - '0';
      } while (channel < 0 || channel >= BL_ANALOG_IN || channel & 0x01);
		blankScreen();
		printf("\n Channel = %d\n", channel);

	   // Configure channel for differential bipolar mode of operation.
	   // (Max voltage range is ±20V)
	   anaInConfig(channel, DIFF_MODE);

		printrange();
		printf("\nChoose gain code (0-%d) =  ", BL_MAX_GAINS - 1);
		do
		{
			gaincode = getchar() - '0';
		} while (gaincode < 0 || gaincode >= BL_MAX_GAINS);
		blankScreen();
      printf("\n Channel = %d\n", channel);
		printf(" Gaincode = %d\n\n", gaincode);

		cal_voltage = (vmax[gaincode] / 2) *.9;
		printf("Adjust voltage connected to AIN%d to approx. %.2fV\n", channel,
      		 cal_voltage);
		printf("Adjust voltage connected to AIN%d to approx. -%.2fV\n",
      		 channel + 1, cal_voltage);
		printf("ENTER actual differential voltage across both channels = ");
		gets(buffer);

		volts1 = atof(buffer);
		value1 = anaInAvg(channel, gaincode, NUM_SAMPLES);

		printf("\n\nSwap differential connections (AIN%d and AIN%d) and then " \
      		 "PRESS any key\n", channel, channel + 1);
		while (!kbhit());
      getchar();

		volts2 = -volts1;
		value2 = anaInAvg(channel, gaincode, NUM_SAMPLES);

      errorCode = anaInCalib(channel, DIFF_MODE, gaincode, value1, volts1,
      								value2, volts2);
		if (errorCode)
		{
			printf("Cannot make calibrations.\n");
			exit(errorCode);
		}

      printf("\nVary differential voltage within +-%.3fV.... \n",
      		 vmax[gaincode]);
      printf("Press any key to calibrate another channel.\n\n");
      do
		{
			voltage = anaInDiff(channel, gaincode);
         if (voltage > BL_ERRCODESTART)
         {
            // valid read
            printf("\rChannel = %d&%d Voltage = %.3fV              ",
            		channel, channel + 1, voltage);
         }
         else if (voltage == BL_NOT_CAL)
         {
            printf("\rChannel = %d&%d Voltage = Not Calibrated     ",
                     channel, channel + 1);
         }
         else
         {
            printf("\rChannel = %d&%d Voltage = Exceeded Range     ",
                     channel, channel + 1);
         }
		} while (!kbhit());
	} // while(1)
} // main()
///////////////////////////////////////////////////////////////////////////


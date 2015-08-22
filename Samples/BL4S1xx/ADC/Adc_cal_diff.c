/***************************************************************************
	adc_cal_diff.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
   This program demonstrates how to recalibrate a differential
   ADC channel using two known voltages to generate calibration
   constants for the given channel and will write the data into
   flash.

	This program will also display the voltage of the channel
   being calibrated.

	Connections:
	============
	For this calibration procedure you will need a power supply
	that has two independant outputs, or two power supplies.

	NOTE:	Before doing the following steps, set the power supply(s)
	      to zero volts and then turn it OFF.

   1. Remove jumpers across pins 1-2 and 5-6 on headers J10 and J11.
	2. Initially connect the negative side of both outputs of the power
   	supply to AGND.
   3. Connect the positive side of one output of the power supply to the
   	positive side of one of the following ADC differential channel pairs.

	    Channel   DIFF Pairs
	    -------  ------------
	      0 		 +AIN0  -AIN1
	      2		 +AIN2  -AIN3
	      4		 +AIN4  -AIN5
	      6		 +AIN6  -AIN7

   4. Connect the positive side of the other output of the power supply to
   	the negative side of one of the ADC differential channel pairs from
      above.
   5. Connect a voltmeter across the differential pair to monitor the total
   	voltage across the ADC channel being calibrated.
		(For best results use a 4 1/2 digit voltmeter)

	Example connections for channel 0:
	   Power Supply 1		Power Supply 2			Voltmeter
		--------------		--------------		----------------
      +	<->   AIN0		+	<->	AIN1		+V		<->   AIN0
	   -	<->   AGND		-	<->	AGND		COM	<->	AIN1

	Instructions:
	=============
	1. Compile and run this program.
	2. Turn ON the power supply for the ADC calibration.
	3. Follow directions in STDIO window.

***************************************************************************/

#class auto	 // Change local var storage default to "auto"

// include BL4S1xx series library
#use "BL4S1xx.lib"

#define MAX_ERRORS	2  // maximum number of errors allowed during 10 reads
#define NUM_SAMPLES	10 // number of samples for getting an average read.

//print voltage ranges
void printrange( void )
{
	int i;
	printf("\n\n");
   printf(" Gain code\tVoltage range\n");
   printf(" ---------\t-------------\n");
	for (i = 0; i < BL_MAX_GAINS; ++i)
   {
		printf("     %d    \t +- %5.2fv\n", i, 20.0 / _gainTable[i]);
   }
}

// blank the stdio screen
void  blankScreen(void)
{
   printf("\x1Bt");
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
	float cal_neg_voltage, cal_pos_voltage;
   float max_voltage;
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

		max_voltage = 20.0 / _gainTable[gaincode];
		cal_neg_voltage = max_voltage * 0.1;
		cal_pos_voltage = max_voltage * 0.9;
		printf("Adjust voltage connected to AIN%d to approx. %.2fV\n", channel,
      		 cal_pos_voltage);
		printf("Adjust voltage connected to AIN%d to approx. %.2fV\n",
      		 channel + 1, cal_neg_voltage);
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

      printf("\nVary differential voltage within +-%.3fV... \n", max_voltage);
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


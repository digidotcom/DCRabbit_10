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
/***************************************************************************
	adc_cal_se_bipolar.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
	This program demonstrates how to recalibrate a single-ended
   bipolar ADC channel using two known voltages to generate
   calibration constants for the given channel and will write
   the data into reserved eeprom.

   It will also continuously display the voltages being monitored.

	Connections:
	============
	For this calibration procedure you will need a power supply
	that has a floating output.

   1. Remove all jumpers from JP4 for voltage measurements.

	2. Before doing the following steps, set the power supply to
      zero volts and then turn it OFF.

	3. Initially connect the positive side of the power supply to
	   the channel (0 -7) being calibrated.

   4. Connect the negative output of the power supply to AGND on
      the controller.

   5. Connect a voltmeter to monitor the ADC channel being calibrated
      at the same points you connected the power supply in steps 3 and 4.
      (For best results use a 4 1/2 digit voltmeter)

	Instructions:
	=============
	1. Power-on the controller.
	2. Compile and run this program.
	3. Turn ON the power supply for the ADC calibration.
	4. Follow the instructions as displayed.
***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

#define MAX_ERRORS	2  // maximum number of errors allowed during 10 reads
#define NUM_SAMPLES	10 // number of samples for getting an average read.

const float vmax[] = {
	10.0,
	5.0,
   2.5,
	2.0,
	1.25,
	1.0,
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
	for (i = 0; i < 6; ++i)
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
	int channel, gaincode;
	int key;
	float voltage, volts1, volts2, cal_voltage;
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
      printf("This sample will calibrate a single ended bipolar ADC channel.");
		printf("\n\nPlease enter an ADC channel, 0 thru 7....");
		do
		{
			channel = getchar() - '0';
		} while (channel < 0 || channel >= BL_ANALOG_IN);
		blankScreen();
		printf("\n Channel = %d\n", channel);

	   // Configure channel for Single-Ended bipolar mode of operation.
	   // (Max voltage range is ±10V)
	   anaInConfig(channel, SE1_MODE);

		printrange();
		printf("\nChoose gain code (0-5) =  ");
		do
		{
			gaincode = getchar() - '0';
		} while (gaincode < 0 || gaincode > 5);
		blankScreen();
      printf("\n Channel = %d\n", channel);
		printf(" Gaincode = %d\n\n", gaincode);

     	cal_voltage = vmax[gaincode]*.8;
		printf("\nAdjust voltage connected to AIN%d to approx. %.2fV\n", channel,
      		 cal_voltage);
		printf("and then enter actual voltage = ");
		gets(buffer);

		volts1 = atof(buffer);
      value1 = anaInAvg(channel, gaincode, NUM_SAMPLES);
      printf("Positive:  channel=%d raw=%d\n", channel, value1);

		printf("\n\nSwap power supply connections and then PRESS any key\n");
		while (!kbhit());
      getchar();

		volts2 = -volts1;
      value2 = anaInAvg(channel, gaincode, NUM_SAMPLES);
      printf("Negative:  channel=%d raw=%d\n", channel, value2);

      errorCode = anaInCalib(channel, SE1_MODE, gaincode, value1, volts1,
      								value2, volts2);
		if (errorCode)
      {
			printf("Cannot make calibrations.\n");
			exit(errorCode);
		}

		printf("\nVary voltage within the range selected (-%.3f to %.3f)V.... \n",
      		 vmax[gaincode], vmax[gaincode]);
      printf("Press any key to calibrate another channel.\n\n");
      do
		{
			voltage = anaInVolts(channel, gaincode);
         if (voltage > BL_ERRCODESTART)
         {
            // valid read
            printf("\rChannel = %d Voltage = %.3fV              ",
            		channel, voltage);
         }
         else if (voltage == BL_NOT_CAL)
         {
            printf("\rChannel = %d Voltage = Not Calibrated     ", channel);
         }
         else
         {
            printf("\rChannel = %d Voltage = Exceeded Range     ", channel);
         }
		} while (!kbhit());
	} // while(1)
} // main()
///////////////////////////////////////////////////////////////////////////


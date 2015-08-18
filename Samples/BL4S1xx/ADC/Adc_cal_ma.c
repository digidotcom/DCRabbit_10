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
	adc_cal_ma.c

	This sample program is for the BL4S1xx series SBCs.

	Description
	===========
	This program demonstrates how to recalibrate an ADC milliamp channel
	using two known currents to generate two coefficients, gain and offset,
	which will be rewritten into reserved eeprom. It will also continuously
	displays the current that is being monitored.

	Connections
	============
   1. Place jumpers across pins 1-2 and 5-6 on headers J10 and J11.
   2. Connect a current meter in series as shown below with it set to
   read 4 to 20 milliamps of current.

	-----------------|									 |---------------------------
						  |									 | 4-20ma mode of operation
	Power supply	  |          + |-------| -		 |
	0 - 2.5v		 POS |------------|current|-------| AIN channels 0 - 3
	                 |	         | meter |		 |
	                 |            ---------       |
	                 |    		                   |
                    |                            |
                    |                            |
                    |                            |
 	                 |                            |
	             NEG |----------------------------| AGND
					     |									 |
	-----------------|									 |---------------------------


	Instructions
	============
	1. Compile and run this program.
	2. Follow directions in STDIO window.
	3. Vary voltage (0-2.5v) on power supply to see the CURRENT meter track
	what's being displayed by Dynamic C (4-20ma).

	Note: For best results use a 4 1/2 digit current meter
***************************************************************************/

#class auto	 // Change local var storage default to "auto"

// include BL4S1xx series library
#use "BL4S1xx.lib"

#define MAX_ERRORS	2  // maximum number of errors allowed during 10 reads
#define NUM_SAMPLES	10 // number of samples for getting an average read.

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
	float locurrent, hicurrent;
	float current;
	char buffer[64];
	int key;
	int channel;
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
      printf("\nThis sample will calibrate a 4-20mA ADC channel.");
		printf("\nChoose the ADC 4-20ma channel (0-3) = ");
		do
		{
			channel = getchar() - '0';
		} while (channel < 0 || channel >= BL_ANALOG_4TO20);
		blankScreen();
		printf("\n Channel = %d\n", channel);

	   // Configure channel for milliamp mode of operation.
	   // (Max current range is 20mA)
	   anaInConfig(channel, mAMP_MODE);

		// Get two data points using known currents
      while(1)
      {
      	printf("\n\nAdjust the current to 5.0mA and then ENTER the actual\n");
			printf("current being measured, (floating point value) = ");
			gets(buffer);
			locurrent = atof(buffer);
			if (locurrent < 4.00 || locurrent > 20.0)
			{
		  		printf("\n>>> Current value must be within 4.0 - 20.0 mA\n\n");
			} else
         {
         	break;
         }
      }
      value1 = anaInAvg(channel, mAMP_GAINCODE, NUM_SAMPLES);

      while(1)
      {
	      printf("\n\nAdjust the current to 19.0mA and ENTER the actual\n");
	      printf("current being measured, (floating point value) = ");
	      gets(buffer);
	      hicurrent = atof(buffer);
	      if (hicurrent < 4.00 || hicurrent > 20.0)
	      {
	         printf("\n>>> Current value must be within 4.0 - 20.0 mA\n\n");
	      } else
         {
         	break;
         }
      }

      value2 = anaInAvg(channel, mAMP_GAINCODE, NUM_SAMPLES);

		// Calculate gains and offsets
      errorCode = anaInCalib(channel, mAMP_MODE, mAMP_GAINCODE, value1,
                             locurrent, value2, hicurrent);
      if (errorCode)
      {
			printf("Cannot make calibrations.\n");
			exit(errorCode);
		}

		printf("\nVary current within 4 to 20 mA\n");
      printf("Press any key to calibrate another channel.\n\n");
      do
		{
			current = anaInmAmps(channel);
         if (current > BL_ERRCODESTART)
         {
            // valid read
            printf("\rChannel = %d Current = %.3fmA             ",
            		channel, current);
         }
         else if (current == BL_NOT_CAL)
         {
            printf("\rChannel = %d Current = Not Calibrated     ", channel);
         }
         else
         {
            printf("\rChannel = %d Current = Exceeded Range     ", channel);
         }
		} while (!kbhit());
	} // while(1)
} // main()
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
	adc_rd_se_averaging.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
   Reads and displays the voltage of all single-ended analog input
   channels using a sliding window. The voltage is calculated from
   coefficients read from the reserved eeprom storage device.

   Connections for unipolar mode of operation, 0 - 20V:
   ====================================================
   1. Remove all jumpers from JP4 for voltage measurements.
   2. Connect the positive power supply lead to an input channel.
	3.	Connect the negative power supply lead to AGND on the controller.

	Instructions:
	=============
	1. Compile and run this program.
	2. Follow the prompted directions of this program during execution.
	3. Voltage will be continuously displayed for all channels.

***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include BLxS2xx series library
#use "BLxS2xx.lib"

// size of circular buffer
#define NUM_SAMPLES 10

// calibration values
calib_t adc_calibration[BL_ANALOG_IN];

// circular buffer
int adc_data_array[BL_ANALOG_IN][NUM_SAMPLES];

// running total of window
long adc_window_total[BL_ANALOG_IN];

// circular buffer position
int adc_data_pos[BL_ANALOG_IN];

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// blank the stdio screen
void  blankScreen(void)
{
   printf("\x1Bt");
}

void printrange( void )
{
	printf("\n\n");
   printf(" Gain code\tVoltage range\n");
	printf(" ---------\t-------------\n");
	printf("     0    \t0 - 20v\n");
	printf("     1    \t0 - 10v\n");
	printf("     2    \t0 - 5v\n");
	printf("     3    \t0 - 4v\n");
	printf("     4    \t0 - 2.5v\n");
	printf("     5    \t0 - 2v\n");
	printf("     6    \t0 - 1.25v\n");
	printf("     7    \t0 - 1v\n\n");
}

// add rawdata to circular buffer and return average
int addToBufferAvg(int channel, int rawdata)
{
	auto int *pos;

	// store pointer to array for speedup
   pos = &adc_data_array[channel][adc_data_pos[channel]];

   // adjust total value in window
   adc_window_total[channel] += rawdata - *pos;

	// add new value to circular buffer
   *pos = rawdata;

   // adjust position to be within bounds
   if (++adc_data_pos[channel] >= NUM_SAMPLES)
   {
   	adc_data_pos[channel] = 0;
   }

   // return new average
   return (int) ((adc_window_total[channel] + (NUM_SAMPLES / 2)) / NUM_SAMPLES);
}

// convert from rawdata to volts
float convertToVolts(int channel, int rawdata)
{
  	rawdata = adc_calibration[channel].offset - rawdata;
	return adc_calibration[channel].gain * rawdata;
}

void main ()
{
	int rawdata;
	int channel;
	int gaincode;
	float voltage;
   char s[128];
   int sample;

   // Initialize the controller
	brdInit();

   // initialize ADC
	for (channel = 0; channel < BL_ANALOG_IN; ++channel)
   {
	   // Configure channel 0 - 7 for Single-Ended unipolar mode of operation.
   	// (Max voltage range is 0 - 20v)
	   anaInConfig(channel, SE0_MODE);

      // initialize circular buffer index
      adc_data_pos[channel] = 0;
   }

	while (1)
	{
     	printrange();
		printf(" Choose gain code (0-%d) =  ", BL_MAX_GAINS - 1);
		do
		{
			while(!kbhit()); // wait for key hit
			gaincode = getchar() - '0';  // convert ascii key into number
		} while ((gaincode < 0) || (gaincode >= BL_MAX_GAINS));
		printf("%d", gaincode);

      // load calibration constants and init circular buffer
		for (channel = 0; channel < BL_ANALOG_IN; ++channel)
      {
	      _anaInEERd(channel, SE0_MODE, gaincode, &adc_calibration[channel]);
   	}

      // initialize circular buffer with zeros
      memset(adc_data_array, 0, sizeof(adc_data_array));
      // initialize total values with zeros
      memset(adc_window_total, 0, sizeof(adc_window_total));

      blankScreen();
		DispStr(1, 2,  "A/D input voltage for channels 0 - 7");
		DispStr(1, 3,  "------------------------------------");
   	DispStr(1, 14, "Press key to select another gain option.");

   	while(1)
      {
			for (channel = 0; channel < BL_ANALOG_IN; ++channel)
			{
      		rawdata = anaIn(channel, gaincode);
            if (rawdata > BL_ERRCODESTART)
            {
               sample = addToBufferAvg(channel, rawdata);
               if (((long)(adc_calibration[channel].gain)) != 0x80000000)
               {
	               voltage = convertToVolts(channel, sample);
	               sprintf(s, "Channel = %2d Raw = %5d Voltage = %.3fV         "\
	                          "     ", channel, sample, voltage);
               }
               else
               {
	               sprintf(s, "Channel = %2d Raw = %5d Voltage = Not Calibrated"\
	                          "     ", channel, sample);
               }
            }
            else if (rawdata == BL_OVERFLOW)
            {
               sprintf(s, "Channel = %2d Voltage = Exceeded Range     ",
               		  channel);
            }
            else
            {
               sprintf(s, "Channel = %2d Voltage = Error During Read  ",
               		  channel);
				}
            DispStr(1,channel + 4, s);
			}

         if (kbhit())
			{
				getchar();
            blankScreen();
            break;
			}
      }
   }
}	//end main


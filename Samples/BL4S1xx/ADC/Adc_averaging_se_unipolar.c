/***************************************************************************
   adc_rd_se_averaging.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
   ============
   Reads and displays the average voltage of each of the single-ended
   analog input channels using a sliding window. The voltage is
   calculated from coefficients read from the flash.

	Connections:
	============
   1. Remove jumpers across pins 1-2 and 5-6 on headers J10 and J11.
   2. Connect the positive power supply lead to an input channel.
   3. Connect the negative power supply lead to AGND on the controller.

   Instructions:
   =============
   1. Compile and run this program.
   2. Follow directions in STDIO window.
   3. Voltage will be continuously displayed for all channels.

***************************************************************************/
#class auto  // Change local var storage default to "auto"

// include series BL4S1xx library
#use "BL4S1xx.LIB"

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

//print voltage ranges
void printrange( void )
{
   int i;
   printf("\n\n");
   printf(" Gain code\tVoltage range\n");
   printf(" ---------\t-------------\n");
   for (i = 0; i < BL_MAX_GAINS; ++i)
   {
      printf("     %d    \t0 - %5.2fv\n", i, 20.0 / _gainTable[i]);
   }
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
      } while (gaincode < 0 || gaincode >= BL_MAX_GAINS);
      printf("%d", gaincode);

      // load calibration constants and init circular buffer
      for (channel = 0; channel < BL_ANALOG_IN; ++channel)
      {
         anaInRdCalib(channel, SE0_MODE, gaincode, &adc_calibration[channel]);
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
         for(channel = 0; channel < BL_ANALOG_IN; channel++)
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
}  //end main


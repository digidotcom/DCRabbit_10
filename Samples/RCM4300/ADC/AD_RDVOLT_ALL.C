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

   ad_rdvolt_all.c

   This sample program is for the RCM4300 series controllers with
   prototyping boards.

   Description
   ===========
   This program reads and displays voltage and equivalent values of each
   single-ended analog input channel. Coefficients are read from the
   simulated EEPROM (Flash)	to compute equivalent voltages and cannot
   be run in RAM.  Gain defaults to x 1 (0 - 20 volts) and can be changed
   by a menu selection.  All channels are read at the same gain setting.

   Computed raw data and equivalent voltages will be displayed.  Channels
   which show a raw count but give a voltage of zero need to be calibrated,
   see AD_CAL_CHAN.c or AD_CAL_ALL.c to perform channel calibration.

   This sample is not intended for "Code and BIOS in RAM" compile mode.

   Prototyping board connections
   =============================
   Connect the power supply positive output to channels LN0IN-LN6IN on the
   prototyping board and the negative output to GND on the controller.
   Connect a volt meter to monitor the voltage inputs.

   NOTE: LN7IN is used as a thermistor input and therefore not used in
         this demonstration.

   Instructions
   ============
   1. Connect a power supply of 0-20 volts to input channels.
   2. Compile and run this program.
   3. Follow the prompted directions of this program during execution.

***************************************************************************/
#class auto
#use RCM43xx.LIB
#ifndef ADC_ONBOARD
   #error "This core module does not have ADC support.  ADC programs will not "
   #fatal "   compile on boards without ADC support.  Exiting compilation."
#endif

#define STARTCHAN	0
#define ENDCHAN 6

const char gainstr[8][23] = {
   "x 1  (0 - 20.00 Volts)",
   "x 2  (0 - 11.11 Volts)",
   "x 4  (0 -  5.56 Volts)",
   "x 5  (0 -  4.44 Volts)",
   "x 8  (0 -  2.78 Volts)",
   "x 10 (0 -  2.22 Volts)",
   "x 16 (0 -  1.39 Volts)",
   "x 20 (0 -  1.11 Volts)" };

//---------------------------------------------------------
//	displays both the raw data count and voltage equivalent
//	_adsCalibS is address for single ended channels
//---------------------------------------------------------
void anaInInfo (unsigned int channel, unsigned int gain, unsigned int *rd,
                     float *ve)
{
   auto unsigned value;
   auto float volt;

   value = anaIn(channel, SINGLE, gain);
   volt = (value - _adcCalibS[channel][gain].offset) *
                        (_adcCalibS[channel][gain].kconst);

   if (value == ADOVERFLOW)
   {
      *rd = ADOVERFLOW;
      *ve = ADOVERFLOW;
   }
   else
   {
      *rd = value;
      if (value <= 0.00)
         *ve = 0.000;
      else
         *ve = volt;
   }
}


main ()
{
   auto unsigned int rawdata, gain;
   auto int inputnum, slotnum, keypress, msgcode;
   auto float voltequ;

   brdInit();			//reads calibration constants

   gain = 0;
   while (1)
   {
      printf("\nCurrent Gain = %s\n", gainstr[gain]);
      printf("\nChoose:\n");
      printf("  1 to display raw data only\n");
      printf("  2 to display voltage only\n");
      printf("  3 to display both\n");
      printf("  4 to change gain\n\n");

      switch (getchar())
      {
         case '1':
            for (inputnum=STARTCHAN; inputnum<=ENDCHAN; inputnum++)
            {
               rawdata = anaIn(inputnum, SINGLE, gain);
               printf("CH%2d raw data %d\n", inputnum, rawdata);
            }
            break;
         case '2':
            for (inputnum=STARTCHAN; inputnum<=ENDCHAN; inputnum++)
            {
               voltequ = anaInVolts(inputnum, gain);
               printf("CH%2d is %.5f V\n", inputnum, voltequ);
            }
            break;
         case '3':
            for (inputnum=STARTCHAN; inputnum<=ENDCHAN; inputnum++)
            {
               anaInInfo(inputnum, gain, &rawdata, &voltequ);
               printf("CH%2d is %.5f V from raw data %d\n", inputnum, voltequ, rawdata);
            }
            break;
         case '4':
            printf("\nCode  Voltage Range\n");
				for (inputnum=0; inputnum<8; inputnum++)
				{
					printf("  %d = %s\n", inputnum, gainstr[inputnum]);
				}
            printf("Select new gain code: ");
            gain = getchar() - 0x30;
            if (gain > 7) {
               printf("\nInvalid gain, setting to 0 (1x - 0 to 20 Volts).\n\n");
               gain = 0;
            }
            else {
               printf("\n\n");
            }
				break;
			default:
			break;
		}
	}
}


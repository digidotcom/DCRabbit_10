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
   dac_volts_sync.c

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
   This program outputs a voltage that can be read with a voltmeter.
   The output voltage is computed with using the calibration constants
   that are located in reserved eeprom.

   The DAC circuit is setup for synchronous mode of operation which
   updates all DAC outputs at the same time when the anaOutStrobe
   function executes. The outputs all updated with values previously
   written with anaOutVolts and/or anaOut functions.

	Instructions:
   =============
   1. Set jumpers to connect pins 1-3 and 2-4 on JP5 for voltage outputs
	2. Set jumpers to connect pins 1-2 and 3-4 on JP3 for channel 0 unipolar;
   	or, set jumper to connect pins 5-6 on JP3 for channel 0 bipolar
      NOTE: Set channel 0 & 1 to the same mode, both unipolar or both bipolar
	3. Set jumpers to connect pins 1-2 and 3-4 on JP6 for channel 1 unipolar;
		or, set jumper to connect pins 5-6 on JP6 for channel 1 bipolar
	4. Connect a voltmeter to one of the DAC output channels labeled
      AOUT0 - AOUT1 on PCB.
	5. Compile and run this program.
	6. Follow the prompted directions of this program during execution.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series lbrary
#use "BLxS2xx.lib"

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while ((long)(MS_TIMER - done_time) < 0 );
}

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// clear lines in the stdio window
void  clearLines(int start, int end)
{
	auto char buffer[127];
   auto int i;

 	memset(buffer, ' ', sizeof(buffer) - 1);
   buffer[sizeof(buffer)-1] = '\0';
   for (i = start; i <= end; ++i)
   {
   	DispStr(0, i, buffer);
   }
}

void main()
{
	int dac_mode;
   float voltout, min_volts, max_volts;
   int channel;
   int key;
   float internal_volts[2], output_volts[2];
	char display[128];
	char tmpbuf[128];

   // Initialize controller
	brdInit();

   DispStr(1, 2, "DAC output voltage configuration");
   DispStr(1, 3, "Dependant on jumper settings (see Instructions)");
   DispStr(1, 4, "--------------------------------");
   DispStr(1, 5, "0 = Unipolar  0  to +10v");
   DispStr(1, 6, "1 = Bipolar  -10 to +10v");
   DispStr(1, 8, "Please enter the DAC configuration 0 - 1 = ");
   do
   {
      key = getchar() - '0';
   } while (key < 0 || key > 1);
   printf("%d", key);

   // Configure the DAC for given configuration
   dac_mode = (key == 0 ? DAC_UNIPOLAR : DAC_BIPOLAR);
   anaOutConfig(dac_mode, DAC_SYNC);

   // Initialize the DAC to output zero volts at the start
   anaOutVolts(0, 0);
   anaOutVolts(1, 0);
   anaOutStrobe(3);

   // Initialize the internal and output voltage shadows
   internal_volts[0] = 0;
   internal_volts[1] = 0;
   output_volts[0] = 0;
   output_volts[1] = 0;

   // set the min and max voltage based on DAC mode.
   max_volts = 10.0;
   min_volts = (dac_mode == DAC_BIPOLAR ? -10.0 : 0.0);

   // print the values table title for the DAC once here, values printed
   // below in "values table"
   DispStr(2, 10,   "DAC CH\tInternal Register\tOutput");
   DispStr(2, 11,   "------\t-----------------\t------");
   // lines 12-13 are "values table" printed below.

   // print the menu once here, but it is located below "values table".
   DispStr(2, 15, "User Options:");
   DispStr(2, 16, "-------------");
   DispStr(2, 17, "1. DAC 0: Write voltage to internal register");
   DispStr(2, 18, "2. DAC 1: Write voltage to internal register");
   DispStr(2, 19, "3. Strobe DAC chip, all DAC channels will be updated");
   DispStr(2, 21, "Note: Must strobe DAC for outputs to be updated!");

   while(1)
   {
   	// NOTE: values table title goes here sequentually, it is printed
      // once above.

      // print values table.
      sprintf(display, "  0   \t     %.2fV\t\t%.2fV    ", internal_volts[0],
              output_volts[0]);
      DispStr(2, 12, display);
      sprintf(display, "  1   \t     %.2fV\t\t%.2fV    ", internal_volts[1],
              output_volts[1]);
      DispStr(2, 13, display);

      // NOTE: menu goes here sequentually, it is printed once above.

      // get response for menu choice
      do
      {
         key = getchar() - '0';
      } while (key < 1 || key > 3);

      switch (key)
      {
         case 1:  //DAC 0: Write voltage to internal register
         case 2:  //DAC 1: Write voltage to internal register
				channel = key - 1; // calculate channel from menu option.
            do
            {
               // display DAC voltage message
               clearLines(23, 23); // make sure the line is clear
               sprintf(display, "DAC %d: Desired voltage (%.0f to %.0fV) = ",
                        channel, min_volts, max_volts);
               DispStr(3, 23, display);
               voltout = atof(gets(tmpbuf));
            } while (voltout < min_volts || voltout > max_volts);

            // send voltage value to DAC for it to output the voltage
            anaOutVolts(channel,  voltout);

            // update the internal storage shadow
            internal_volts[channel] = voltout;
            break;
         case 3: //Strobe DAC chip, all DAC channels will be updated
            // strobe both the outputs
            anaOutStrobe(3);
            // update the output shadows
            output_volts[0] = internal_volts[0];
            output_volts[1] = internal_volts[1];
            DispStr(3, 23, "DAC outputs have been updated...");
            msDelay(1000);
            break;
      }

      clearLines(23, 23); // clear the optional input line after the menu
	} // while (1)
}
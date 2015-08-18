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
   dac_ma_sync.c

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
   This program generates a 4-20ma current that can be monitored with a
   multimeter. The output current is computed with using the calibration
   constants that are located in reserved eeprom.

   The DAC circuit is setup for synchronous mode of operation which
   updates all DAC outputs at the same time when the anaOutStrobe
   function executes. The outputs all updated with values previously
   written with anaOutmAmps and/or anaOut functions.

	Connections:
	============
   1. Set jumpers to connect pins 3-5 and 4-6 on JP5 for current outputs
   2. Set jumpers to connect pins 1-2 and 3-4 on JP3 for channel 0
   3. Set jumpers to connect pins 1-2 and 3-4 on JP6 for channel 1
	4. Connect a current meter in series to the DAC output to be
      monitored thru a resistor between 50 and 400 ohms to GND.

                         |-----------------|       50 - 400 Ohms
      AOUT0 - AOUT1 -----|  Current Meter  |-------/\/\/\/\------
                         |-----------------|                    |
                                                                |
                                                               ---
                                                               GND
	Instructions:
	=============
	1. Compile and run this program.
	2. Follow the prompted directions of this program during execution.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.LIB"

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
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
   float mAmps;
   int channel;
   int key;
   float internal_mAmp[2], output_mAmp[2];
	char display[128];
	char tmpbuf[128];

   // Initialize controller
	brdInit();

   // Configure the DAC for desired output configuration.
   anaOutConfig(DAC_UNIPOLAR, DAC_SYNC);

   // Initialize the DAC to output 4mA at the start
   anaOutmAmps(0, 4);
   anaOutmAmps(1, 4);
   anaOutStrobe(3);

   // Initialize the internal and output current shadows
   internal_mAmp[0] = 4;
   internal_mAmp[1] = 4;
   output_mAmp[0] = 4;
   output_mAmp[1] = 4;

   // print the values table title for the DAC once here, values printed
   // below in "values table"
   DispStr(2, 0,   "DAC CH\tInternal Register\tOutput");
   DispStr(2, 1,   "------\t-----------------\t------");
   // lines 2-3 are "values table" printed below.

   // print the menu once here, but it is located below "values table".
   DispStr(2, 5, "User Options:");
   DispStr(2, 6, "-------------");
   DispStr(2, 7, "1. DAC 0: Write mAmps to internal register");
   DispStr(2, 8, "2. DAC 1: Write mAmps to internal register");
   DispStr(2, 9, "3. Strobe DAC chip, all DAC channels will be updated");
   DispStr(2, 11, "Note: Must strobe DAC for outputs to be updated!");

   while(1)
   {
   	// NOTE: values table title goes here sequentually, it is printed
      // once above.

      // print values table.
      sprintf(display, "  0   \t     %.2fmA\t\t%.2fmA    ", internal_mAmp[0],
              output_mAmp[0]);
      DispStr(2, 2, display);
      sprintf(display, "  1   \t     %.2fmA\t\t%.2fmA    ", internal_mAmp[1],
              output_mAmp[1]);
      DispStr(2, 3, display);

      // NOTE: menu goes here sequentually, it is printed once above.

      // get response for menu choice
      do
      {
         key = getchar() - '0';
      } while (key < 1 || key > 3);

      switch (key)
      {
         case 1:  //DAC 0: Write mAmps to internal register
         case 2:  //DAC 1: Write mAmps to internal register
				channel = key - 1; // calculate channel from menu option.
            do
            {
               // display DAC current message
               clearLines(13, 13); // make sure the line is clear
               sprintf(display, "DAC %d: Desired current (4 to 20mA) = ",
                        channel);
               DispStr(3, 13, display);
               mAmps = atof(gets(tmpbuf));
            } while (mAmps < 4 || mAmps > 20);

            // send current value to DAC for it to output the current
            anaOutmAmps(channel,  mAmps);

            // update the internal storage shadow
            internal_mAmp[channel] = mAmps;
            break;
         case 3: //Strobe DAC chip, all DAC channels will be updated
            // strobe both the outputs
            anaOutStrobe(3);
            // update the output shadows
            output_mAmp[0] = internal_mAmp[0];
            output_mAmp[1] = internal_mAmp[1];
            DispStr(3, 13, "DAC outputs have been updated...");
            msDelay(1000);
            break;
      }

      clearLines(13, 13); // clear the optional input line after the menu
	} // while (1)
}



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
   dac_cal_ma.c

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
	This program demonstrates how to recalibrate a DAC channel
   using a known current to generate calibration constants for
   the given channel and then will write the data into reserved
   eeprom.

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

#define HICOUNT 3695		// Sets up a low voltage calibration point
#define LOCOUNT 400     // Sets up a high voltage calibration point

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

///////////////////////////////////////////////////////////////////////////

void main()
{
   float current1, current2, mAmps;
   int channel;
 	int key;
   char tmpbuf[128];
	char display[128];

   // Initialize controller
	brdInit();

   // Configure the DAC for given configuration
   anaOutConfig(DAC_UNIPOLAR, DAC_ASYNC);

   // display what this sample does
   DispStr(1, 1, "Calibrate DAC 4-20mA outputs");

   // display user options
   DispStr(1, 3, "User Options:");
   DispStr(1, 4, "-------------");
   DispStr(1, 5, "1. DAC 0: Calibrate");
   DispStr(1, 6, "2. DAC 1: Calibrate");
   DispStr(1, 7, "3. DAC 0: Set 4 to 20mA output");
   DispStr(1, 8, "4. DAC 1: Set 4 to 20mA output");

   while(1)
   {
	   while(!kbhit());
	   key = getchar() - '0';
	   switch (key)
	   {
	      case 1: // DAC 0: Calibrate
	      case 2: // DAC 1: Calibrate
	         channel = key - 1; // calculate channel from the menu option

	         DispStr(1, 11, "!!!Caution this will overwrite the calibration "\
	                "constants set at the factory.");
	         DispStr(1, 12, " Do you want to continue(Y/N)?");
	         do
            {
	            while(!kbhit());
	            key = getchar();
	            if (key == 'N' || key == 'n')
	            {
	               exit(0);
	            }
	         } while(key != 'Y' && key != 'y');

	         // set two known current points using rawdata values, the
	         // user will then type in the actual current for each point
	         anaOut(channel, HICOUNT);
	         sprintf(display, "DAC %d: ENTER the current reading from meter "\
	                          "(approx. 5.5mA)  = ", channel);
	         DispStr(1, 14, display);
	         current1 = atof(gets(tmpbuf));

	         anaOut(channel, LOCOUNT);
	         sprintf(display, "DAC %d: ENTER the current reading from meter "\
	                          "(approx. 18mA) = ", channel);
	         DispStr(1, 15, display);
	         current2 = atof(gets(tmpbuf));

	         // Create calibration data and store into flash memory
	         anaOutCalib(channel, 2, HICOUNT, current1, LOCOUNT, current2);

	         sprintf(display, "DAC %d: now calibrated...", channel);
	         DispStr(1, 17, display);

            DispStr(1, 18, "(press any key to continue)");
            while(!kbhit());
            getchar();
            clearLines(11, 18);
				break;
	      case 3: // DAC 0: Set 4 to 20mA output
	      case 4: // DAC 1: Set 4 to 20mA output
	         channel = key - 3; // calculate the channel from the menu option.
	         do
	         {
	            // display DAC current message
               clearLines(11, 11);
	            sprintf(display, "DAC %d: ENTER desired current (4 to 20mA) = ",\
               		  channel);
               DispStr(1, 11, display);
               mAmps = atof(gets(tmpbuf));
	         } while (mAmps < 4 || mAmps > 20);

	         // send current value to DAC for it to output
	         anaOutmAmps(channel,  mAmps);
            sprintf(display, "DAC %d: Observe current on meter...", channel);
            DispStr(1, 12, display);

            DispStr(1, 13, "(press any key to continue)");
            while(!kbhit());
            getchar();
            clearLines(11, 13);
				break;
	   }
	} // while(1)
}


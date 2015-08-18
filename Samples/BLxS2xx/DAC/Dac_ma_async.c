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
   dac_ma_async.c

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
   This program generates a 4-20ma current that can be monitored with a
   multimeter. The output current is computed with using the calibration
   constants that are located in reserved eeprom.

   The DAC circuit is setup for Asynchronous mode of operation which
   will update the DAC output at the time it's being written via the
   anaOut or anaOutmAmps functions.

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
	char display[128];
 	auto char tmpbuf[128];

   // Initialize controller
	brdInit();

   // Configure the DAC for desired output configuration.
   anaOutConfig(DAC_UNIPOLAR, DAC_ASYNC);

   // print the menu once here.
   DispStr(2, 1, "User Options:");
   DispStr(2, 2, "-------------");
   DispStr(2, 3, "1. DAC 0: Set new current (in mA)");
   DispStr(2, 4, "2. DAC 1: Set new current (in mA)");
   DispStr(2, 6, "Note: Current is immediately output by the DAC. " \
   					"(No need to strobe)");

   while(1)
   {
      // get response for menu choice
      do
      {
         key = getchar() - '0';
      } while (key < 1 || key > 2);

      channel = key - 1; // calculate channel from menu selection.
      						 // menu 1 = ch 0 and menu 2 = ch 1
      do
      {
         // display DAC current message
         clearLines(8, 8); // make sure the line is clear
         sprintf(display, "DAC %d: Desired current (4 to 20mA) = ",
                  channel);
         DispStr(3, 8, display);
         mAmps = atof(gets(tmpbuf));
      } while (mAmps < 4 || mAmps > 20);

      // send current value to DAC for it to output
      anaOutmAmps(channel,  mAmps);

      clearLines(8, 8); // clear the optional input line after the menu
	} // while (1)
}



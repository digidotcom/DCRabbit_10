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
/********************************************************************
	ppm.c

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
   This program demonstrates the use of four Pulse Position Modulated (PPM)
   channels on the digital output pins DIO 0, 2, 4 & 6.  The PPM signals are
   set for a frequency of 200 Hz with the duty cycle adjustable from 0 to 100%
   and an offset adjustable from 0 to 100% by the user.  These pins can be
   connected to an oscilloscope to view the waveform being generated.  The
   overall frequency can be adjusted with the PPM_FREQ definition.

   Note: A warning will be displayed when the offset + duty > 100%.  In these
   cases, the library will truncate the duty.

	Connections:
	============
	1. Connect a wire from the controller J10 GND to the oscilloscope GND.

	2. Connect a scope probe to J10 DIO 0, 2, 4 or 6 pins to see the PPM
      waveform from that pin.

   3. Check that jumper on JP9 is in its default position of 3-4

	Instructions:
	=============
   1. Compile and run this program.
   2. Change duty cycle and offsets for a given PPM channel via STDIO
      and watch the change in waveforms on the oscilloscope.  Signals
      on DIO0 (PPM0) and DIO2 (PPM1) will all be in synchronization
      with each other as they share the same overall counter block which
      sets the cycle frequency.  The same is true for PPM signals on
      DIO4 (PPM2) and DIO6 (PPM3).  The two blocks may have a phase
      shift from each other, but will run at the same frequency.

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

// PPM inversion
#define PPM_INVERT 0

// base PPM frequency
#define PPM_FREQ 200.0

// number of PPM channels, do not modify
#define NUM_PPM 4

// Offset / Duty array references
enum {
  OFFSET = 0,
  DUTY
};

// global strings that store the name of the parameters
const char param_string[][8] = {"offset", "duty"};

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// channels to use for PPM
const int PPM_CH[NUM_PPM] = {0, 2, 4, 6};

void main()
{
	int channel;
   int cur_channel;
   int option;
   int select;
   int settings[2][NUM_PPM];
   int val_input;
   char s1[80];
   char s2[80];
   char *ptr, *ptr2;

   // Initialize the controller
	brdInit();

   // Start PPM with default duty cycle and offsets for each channel
	for (channel = 0; channel < NUM_PPM; ++channel)
   {
   	// offset defaults will be 0, ..., 40 percent
   	settings[OFFSET][channel] = (int)(40.01 / (NUM_PPM - 1) * channel);
   	// duty cycle defaults will be 10, ..., 50 percent
   	settings[DUTY][channel] = 10 + (int)(40.01 / (NUM_PPM - 1) * channel);
      setPPM(PPM_CH[channel], PPM_FREQ, (float)(settings[OFFSET][channel]),
                (float)(settings[DUTY][channel]), PPM_INVERT, 0, 0);
   }

	DispStr(8, 1, " <<< Configurable Outputs... Setup as PPMs >>>");

   // display names of PPM channels
   ptr = s1;
   ptr += sprintf(s1, "\t");
   sprintf(s2, "\t");
   for(channel = 0; channel < NUM_PPM; ++channel)
   {
   	ptr += sprintf(ptr, " PPM%d\t", channel);
      strcat(s2, " ----\t");
   }
   DispStr(8, 3, s1);
	DispStr(8, 4, s2);

   // set current channel to 0
   cur_channel = 0;

   // Generate a PPM signal where the user can change the duty cycle
	while (1)
	{

   	// Display current PPM offset and duty values
      ptr = s1;
      ptr2 = s2;
      ptr += sprintf(ptr, "offset\t");
      ptr2 += sprintf(ptr2, "duty  \t");
      for(channel = 0; channel < NUM_PPM; ++channel)
      {
      	ptr += sprintf(ptr, "%4d\t", settings[OFFSET][channel]);
      	ptr2 += sprintf(ptr2, "%4d\t", settings[DUTY][channel]);
      }
      DispStr(8, 5, s1);
      DispStr(8, 6, s2);

      // Display usage message
      sprintf(s1, "\tPPM control Menu\tCurrent PPM Channel = %d", cur_channel);
      DispStr(1, 10, s1);
      DispStr(1, 11,  "\t----------------");
      DispStr(1, 12, "\t1. Increment PPM offset");
      DispStr(1, 13, "\t2. Decrement PPM offset");
      DispStr(1, 14, "\t3. Set PPM offset");
      DispStr(1, 15, "\t4. Increment PPM duty cycle");
      DispStr(1, 16, "\t5. Decrement PPM duty cycle");
      DispStr(1, 17, "\t6. Set PPM duty cycle");
      DispStr(1, 18, "\t7. Select another PPM channel");
      DispStr(1, 20, "Select Option 1 - 7 = ");

      do
      {
      	while(!kbhit());  // allow user to debug easier
         option = getchar() - '0';
      } while (option < 0 || option > 7);
      printf("%d", option);

      select = DUTY; // default to DUTY
      switch (option)
      {
			case 1:  // Increment PPM offset
         	select = OFFSET;
            // fall through to increment
         case 4:  // Increment PPM duty cycle
				if (settings[select][cur_channel] < 100)
            {
         		settings[select][cur_channel] += 1;
            }
            break;
			case 2:  // Decrement PPM offset
         	select = OFFSET;
            // fall through to decrement
         case 5:  // Decrement PPM duty cycle
				if (settings[select][cur_channel] > 0)
            {
         		settings[select][cur_channel] -= 1;
            }
            break;
			case 3:  // Set PPM offset
         	select = OFFSET;
            // fall throught to setting code
			case 6:  // Set PPM duty cycle
				// print prompt
				sprintf(s1, "\tEnter new PPM %s [0-100]: ", param_string[select]);
            DispStr(1, 21, s1);

            // get response
            val_input = atoi(gets(s1));
	         if (val_input >= 0 && val_input <= 100)
	         {
	            settings[select][cur_channel] = val_input;
	         }
            break;
			case 7:  // Select other PPM channel
         	sprintf(s1, "\tEnter new PPM channel [0-%d]: ", NUM_PPM - 1);
				DispStr(1, 21, s1);
	         do
	         {
		      	while(!kbhit());  // allow user to debug easier
	            option = getchar() - '0';
	         } while (option < 0 || option >= NUM_PPM);
				cur_channel = option;
            break;
      }

      // update the offset and duty of the selected PPM
      setOffset(PPM_CH[cur_channel], (float) (settings[OFFSET][cur_channel]));
      setDuty(PPM_CH[cur_channel], (float) (settings[DUTY][cur_channel]));

      // check to make sure offset and duty not greater than 100%
		for (channel = 0; channel < NUM_PPM; ++channel)
      {
	      if (settings[OFFSET][channel] + settings[DUTY][channel] > 100)
         {
         	// display warning
            sprintf(s1, ">>> Warning: Offset + Duty for PPM%d is over 100%%. "\
            				"Duty truncated by library.", channel);
            DispStr(1, 23 + channel, s1);
         } else {
				// clear warning
            DispStr(1, 23 + channel, "                                       " \
            			"                                        ");
         }
      }

      // clear the buffer of certain lines
  		DispStr(1, 20, "                                                     ");
  		DispStr(1, 21, "                                                     ");
	}
}






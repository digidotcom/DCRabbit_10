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
	pulse_capture.c

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
   This program demonstrates the use of two pulse capture inputs tied
   to PPM channels on the configurable digital I/O pins on connector
   J10.  Pulse capture allows measurement of the begin and end positions
   of a pulse in a given time window.  We take advantage of the counter
   synchronization feature of the underlying RIO chip to create capture
   windows and pulse modulation windows that are synchronized.  This
   guarantees that we always catch the begin edge first on a quickly
   repeating waveform.  This was done to create an interactive element
   to this demo, but capturing of real world repetitive signals will
   usually not have this advantage. Refer to Section X of the BL4S1xx
   User's Manual for more information on how to use pulse capture.

   Note: A warning will be displayed when the offset + duty > 100%.  In these
   cases, the library will truncate the duty.

   Connections:
	============
   1. Enable 5V pull-ups on the inputs by jumpering pins 5-6 on J13.
   2. Connect the following pins together on the BL4S1xx:
   		IN0 - OUT0
         IN3 - OUT2

	3. Optionally connect the GND wire from the controller J10 to an
      oscilloscope GND.  Connect probes to the IN0 & OUT0 pair or the
      IN3 & OUT2 pair to view the PPM signals on the oscilloscope.

   Instructions:
	=============
   1. Compile and run this program.
   2. Change offset and duty cycle for a given PPM channel via STDIO and
      watch the change in begin and end times measured on the pulse
      capture inputs.  The PPM frequency can be changed through the
      definition of PPM_FREQ.

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// base PPM frequency
#define PPM_FREQ 200.0

// Setup matching limit value for capture synchronization using clock
//  frequency constants from the BL4S1xx library and a 16 bit counter
// If PPM Frequency is too slow to use main clock, switch to prescaler
#if (PPM_FREQ > (RIO_CLOCK_FREQ / 65536))
 #define CAPTURE_CLOCK_FREQ RIO_CLOCK_FREQ
 #define USE_PRESCALE 0
#else
 #define CAPTURE_CLOCK_FREQ RIO_PRESCALE_FREQ
 #define USE_PRESCALE BL_PRESCALE
#endif

// Offset / Duty array references
enum {
  OFFSET = 0,
  DUTY
};

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// channels to use for PPM and capture.
const int PPM_CH[] = {0, 2};
const int CAPTURE_CH[] = {0, 3};

void main()
{
	int channel;
   int cur_channel;
   int option;
   int select;
   int settings[2][2];
   int val_input;
   word begin[2];
   word end[2];
   char s1[80];
   float freq;

   // set the initial PPM frequency
   freq = PPM_FREQ;

   // Initialize the controller
	brdInit();

   // Start PPM with default duty cycle for each channel
	for (channel = 0; channel < 2; ++channel)
   {
	   // defaults will be 10 and 50 percent with 25 percent offset
	   settings[OFFSET][channel] = 25;
	   settings[DUTY][channel] = 10 + (40 * channel);
	   setPPM(PPM_CH[channel], PPM_FREQ, 25.0,
	                                (float)(settings[DUTY][channel]), 0, 0, 0);
	   // Setup input capture for begin=rise & end=fall on the same channel
	   setCapture(CAPTURE_CH[channel], BL_CNT_RUN, BL_EVENT_RISE,
	                 BL_SAME_CHANNEL | BL_EVENT_FALL | USE_PRESCALE);
	   // Set capture input to same limit value as PPM
	   setLimit(CAPTURE_CH[channel],
      			(word)((CAPTURE_CLOCK_FREQ / freq) + 0.5) - 1);
	   // Setup global sync control on both PPM and Capture input
	   setSyncIn(CAPTURE_CH[channel], -1, BL_EDGE_RISE);
	   setSyncOut(PPM_CH[channel], -1, BL_EDGE_RISE);
	}

   // Synchronize both PPM's and Capture inputs
   globalSync();

	DispStr(8, 1, " <<< Configurable Outputs... Setup as PPMs >>>");

   // display names of PPM channels
   DispStr(8, 3, "  PPM0\tOffset\tDuty\t  PPM1\tOffset\tDuty");
	DispStr(8, 4, "\t------\t-----\t\t------\t-----");

   // set current channel to 0
   cur_channel = 0;

   // Generate a PPM signal where the user can change the duty and offset
	while (1)
	{
   	// Display current PPM values
      sprintf(s1, "\t%5d\t%5d\t\t%5d\t%5d   ", settings[OFFSET][0],
               settings[DUTY][0], settings[OFFSET][1], settings[DUTY][1]);
      DispStr(8, 5, s1);

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
      DispStr(1, 18, "\t7. Select other PPM channel");

      // Read captured begin and end values from pulses
		for (channel = 0; channel < 2; ++channel)
      {
	      getBegin(CAPTURE_CH[channel], &(begin[channel]));
	      getEnd(CAPTURE_CH[channel], &(end[channel]));
      }

      // Display current captured begin and end values from pulses
      sprintf(s1,"\tCh. 0 Begin = %u, End = %u\tCh. 1 Begin = %u, End = %u     ",
                 begin[0], end[0], begin[1], end[1]);
      DispStr(1, 7, s1);

      // Display selection prompt
      DispStr(1, 20, "\tSelect Option 1 - 8 = ");
      do
      {
         option = getchar() - '0';
      } while (!((option >= 1) && (option <= 8)));
      printf("%d", option);

      select = DUTY; // default to DUTY
      switch (option)
      {
			case 1:  // Increment PPM offset
         	select = OFFSET;
         case 4:  // Increment PPM duty cycle
				if (settings[select][cur_channel] < 100)
            {
         		settings[select][cur_channel] += 1;
            }
            break;
			case 2:  // Decrement PPM offset
         	select = OFFSET;
         case 5:  // Decrement PPM duty cycle
				if (settings[select][cur_channel] > 0)
            {
         		settings[select][cur_channel] -= 1;
            }
            break;
			case 3:  // Set PPM offset
         	select = OFFSET;
			case 6:  // Set PPM duty cycle
				if (OFFSET == select)
            {
		      	DispStr(1, 21, "\tEnter new PPM offset [0-100]: ");
				} else {
		      	DispStr(1, 21, "\tEnter new PPM duty [0-100]: ");
				}
	         val_input = atoi(gets(s1));
	         if (val_input >= 0 && val_input <= 100)
	         {
	            settings[select][cur_channel] = val_input;
	         }
            break;
			case 7:  // Select other PPM channel
            cur_channel ^= 1;
            break;
      }

      // update the offset and duty of the selected PPM
      setOffset(PPM_CH[cur_channel], (float) (settings[OFFSET][cur_channel]));
      setDuty(PPM_CH[cur_channel], (float) (settings[DUTY][cur_channel]));

      // check to make sure offset and duty not greater than 100%
		for (channel = 0; channel < 2; ++channel)
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




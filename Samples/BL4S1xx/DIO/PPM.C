/********************************************************************
	ppm.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
   This program demonstrates the use of up to eight PPM channels on the
   digital output pins.  The PPM signals are set for a frequency of 200 Hz
   with the duty cycle adjustable from 0 to 100% and an offset adjustable \
   from 0 to 100% by the user.  These pins can be connected to an oscilloscope
   to view the waveform being generated.  The overall frequency can be adjusted
   with the PPM_FREQ definition.

   Note: The digital outputs do not have an internal pull-up and so will
   not register on the oscilloscope unless a pull-up is attached.  The DEMO
   board has pull-ups, so the PPMs could be connected to the switches when they
   set to be active low.

	Connections:
	============
	1. Connect a wire from the controller GND to oscilloscope GND.

	2. Connect a scope probe to any of the OUT# pins to see the PPM
      waveform from that pin.  Make sure there is a pull-up on the
      channel (see note above).

	Instructions:
	=============
   1. Compile and run this program.
   2. Change duty cycle and offsets for a given PPM channel via STDIO
      and watch the change in waveforms on the oscilloscope.  Signals on
      the same RIO counter block (OUT0 and OUT1 for instance) will all be
      in synchronization with each other.  Different blocks may have a
      phase shift from each other, but will run at the same frequency.
      Global sync can be used to synchronize different block on the RIO,
      but is not demonstrated in this sample.

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// PPM inversion
#define PPM_INVERT 0

// base PPM frequency
#define PPM_FREQ 200.0

// number of PPM channels, must be greater than 1
#define NUM_PPM 8

// array index for PPM settings, and offset/duty strings
enum
{
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
      setPPM(channel, PPM_FREQ, (float)(settings[OFFSET][channel]),
                (float)(settings[DUTY][channel]), PPM_INVERT, 0, 0);
   }

	DispStr(8, 1, " <<< Digital Outputs... Setup as PPMs >>>");

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
      setOffset(cur_channel, (float) (settings[OFFSET][cur_channel]));
      setDuty(cur_channel, (float) (settings[DUTY][cur_channel]));

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






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
	pwm.c

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
   This program demonstrates the use of eight PWM channels on the digital
   output pins OUT0-OUT7.  The PWM signals are set for a frequency of
   200 Hz with the duty cycle adjustable from 0 to 100% by the user.
   These pins can be connected to an oscilloscope to view the waveform
   being generated.  The overall frequency can be adjusted with the
   PWM_FREQ definition.

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
   2. Change duty cycle for a given PPM channel via STDIO and watch the
   	change in waveforms on the oscilloscope.  Signals on the same RIO
      counter block (OUT0 and OUT1 for instance) will all be in
      synchronization with each other.  Different blocks may have a
      phase shift from each other, but will run at the same frequency.
      Global sync can be used to synchronize different block on the RIO,
      but is not demonstrated in this sample.

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// PWM inversion
#define PWM_INVERT 0

// base PWM frequency
#define PWM_FREQ 200.0

// number of PWM channels, must be greater than 1
#define NUM_PWM 8

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
   int duty[NUM_PWM];
   int duty_input;
   char s1[80];
   char s2[80];
   char *ptr;

   // Initialize the controller
	brdInit();

   // Start PWM with default duty cycle for each channel
	for (channel = 0; channel < NUM_PWM; ++channel)
   {
   	// defaults will be 0, ..., 100 percent
   	duty[channel] = (int)((100.0/(NUM_PWM - 1)) * (float)channel);
      setPWM(channel, PWM_FREQ, (float)(duty[channel]), PWM_INVERT, 0);
   }

	DispStr(8, 1, " <<< Digital Outputs... Setup as PWMs >>>");

   // display names of PWM channels
   ptr = s1;
   s2[0] = '\0';
   for(channel = 0; channel < NUM_PWM; ++channel)
   {
   	ptr += sprintf(ptr, " PWM%d\t", channel);
      strcat(s2, " ----\t");
   }
   DispStr(8, 3, s1);
	DispStr(8, 4, s2);

   // set current channel to 0
   cur_channel = 0;

   // Generate a PWM signal where the user can change the dutycycle
	while (1)
	{

   	// Display current PWM values
      ptr = s1;
      for(channel = 0; channel < NUM_PWM; ++channel)
      {
      	ptr += sprintf(ptr, "%4d\t", duty[channel]);
      }
      DispStr(8, 5, s1);

      // Display usage message
      sprintf(s1, "\tPWM control Menu\tCurrent PWM Channel = %d", cur_channel);
      DispStr(1, 9, s1);
      DispStr(1, 10,  "\t----------------");
      DispStr(1, 11, "\t1. Increment PWM duty cycle");
      DispStr(1, 12, "\t2. Decrement PWM duty cycle");
      DispStr(1, 13, "\t3. Set PWM duty cycle");
      DispStr(1, 14, "\t4. Select new PWM channel");
      DispStr(1, 16, " Select Option 1 - 4 = ");

      do
      {
      	while(!kbhit());  // allow user to debug easier
         option = getchar() - '0';
      } while (option < 0 || option > 4);
      printf("%d", option);

      switch (option)
      {
			case 1:  // Increment PWM duty cycle
	         if (duty[cur_channel] < 100)
	         {
	            duty[cur_channel] += 1;
	         }
            break;
			case 2:  // Decrement PWM duty cycle
	         if (duty[cur_channel] > 0)
	         {
	            duty[cur_channel] -= 1;
	         }
            break;
			case 3:  // Set PWM duty cycle
	         DispStr(1, 17, "\tEnter new PWM duty [0-100]: ");
	         duty_input = atoi(gets(s1));
	         if (duty_input >= 0 && duty_input <= 100)
	         {
	            duty[cur_channel] = duty_input;
	         }
            break;
			case 4:  // Select new PWM channel
	         // we are changing the channel
      		sprintf(s1, "Enter new PWM channel [0-%d]: ", NUM_PWM - 1);
	         DispStr(1, 17, s1);
	      	while(!kbhit());  // allow user to debug easier
	         option = getchar() - '0';
	         if (option >= 0 && option < NUM_PWM)
	         {
	            // valid channel selected, change to new channel
	            cur_channel = option;
	         }
            break;
      }

      // set the duty for the current channel.
      setDuty(cur_channel, (float)(duty[cur_channel]));

      // clear the buffer of certain lines
  		DispStr(1, 16, "                                                     ");
  		DispStr(1, 17, "                                                     ");
	}
}
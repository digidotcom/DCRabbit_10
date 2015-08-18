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
	interrupts.c

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
   This program demonstrates the use of the RIO interrupt service
   capabilities on the BLxS2xx series.  The demo sets up two interrupt
   sources, one is an external interrupt which will be tied to a push
   button switch, the other a rollover interrupt tied to a timer which
   is producing a PWM output.

   The STDIO window will show a count of rollovers that have occurred
   since the PWM signal was started.  The window will also display a
   message of 'Button Pressed' each time the push button switch is
   depressed.  The message will display for 1 second unless the button
   is pressed again.  Each time the button is pressed it will reset the
   timeout timer that removes the message, so you can keep the message
   on the screen indefinitely as long as you keep pressing the button
   in time.

	Connections:
	============
	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.

	2. Connect a wire from the controller J10 pin 5 GND to the DEMO board GND.

	3. Connect a wire from the controller J7 pin 6 (+5V) to the DEMO board +V.

	4. Connect DIO0 (J10 pin 9) to SW1 on the demo board.

	Instructions:
	=============
   1. Compile and run this program.
   2. Observe the counter updating and press SW1 to see the 'Button Pressed'
      message

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

char button_event, rollover_event;
int  bp_handle, ro_handle;
unsigned long rollover_count, time_stamp;

// Button Press Interrupt Handler (ISR)
root void button_press()
{
   // Save time of button press and set event flag
   time_stamp = MS_TIMER;
   button_event = 1;

   // Turn off all interrupt flags serviced by this handler
   RSB_CLEAR_ALL_IRQ(bp_handle);
}

// Rollover Interrupt Handler (ISR)
root void rollover()
{
   // Increment count and set event flag
   ++rollover_count;
   rollover_event = 1;

   // Turn off the "rollover on increment" interrupt flag
   RSB_CLEAR_IRQ(ro_handle, BL_IER_ROLL_I);
}

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

void main()
{
   char buf[11];
   int button;

   // Initialize board to defaults
   brdInit();

   // Setup interrupt on channel 0 and attach button_press ISR handler
   //  IER mask not yet known, so set it to zero for now
   bp_handle = addISR(0, BL_INPUT_BLOCK, 0, &button_press);

   // Setup interrupt on channel 3 and attach rollover ISR handler
   ro_handle = addISR(3, BL_OUTPUT_BLOCK, BL_IER_ROLL_I, &rollover);

   if (bp_handle < 0 || ro_handle < 0)
   {
   	// Error: need to increase the number of interrupts allowed. (RSB_MAX_ISR)
		printf("Error: need to increase number of interrupts allowed. " \
      		 "(RSB_MAX_ISR)");
      exit(-ENOSPC);
	}

   // Setup external interrupt on falling edge for channel 0
   setExtInterrupt(0, BL_IRQ_FALL, bp_handle);

   // Setup PWM on channel 3 with 5 Hz frequency
   setPWM(3, 5.0, 50.0, 0, 0);

   // Initialize event flags, counter and time stamp
   rollover_event = 0;
   button_event = 0;
   button = 0;
   rollover_count = 0;
   time_stamp = 0;

   // Enable both ISR's
   enableISR(bp_handle, 1);
   enableISR(ro_handle, 1);

   // Write initial STDIO window strings
   DispStr(1, 2, "Interrupt Service Sample");
   DispStr(1, 4, "Rollover Count:          0");

   // Foreground event processing loop
   while (1)
   {
      // Look for counter rollover event
      if (rollover_event)
      {
         // Update count on the STDIO window
         sprintf(buf, "%10lu", rollover_count);
         DispStr(17, 4, buf);
         rollover_event = 0;
      }

      // Look for button press event
      if (button_event)
      {
         // See if button message is not currently displayed
         if (!button)
         {
            DispStr(1, 7, "Button Pressed");
            button = 1;
         }
         button_event = 0;
      }

      // Look for button message timeout
      if (button && ((MS_TIMER - time_stamp) > 1000))
      {
         // Clear message on screen and message flag
         DispStr(1, 7, "              ");
         button = 0;
      }
   }
}



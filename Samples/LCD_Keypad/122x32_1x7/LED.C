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
/**************************************************************************
	led.c

	This sample program is used with 122x32 display and 1x7 keypad module
	with LED's.

   NOTE: Not currently supported on RCM4xxx modules.

	This program demonstrates how to toggle the LED's on the display
	module/controller.

	The program will toggle the following LED's:

	LED#	 Silkscreen Label
   ----	 -----------------
	 0  	 	LED DS1
	 1  		LED DS2
	 2  	 	LED DS3
	 3  		LED DS4
	 4  	 	LED DS5
	 5  	 	LED DS6
	 6  		LED DS7

**************************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

#define LEDOFF 0  //Value used to turn the LED OFF
#define LEDON	1	//Value used to turn the LED ON

///////////////////////////////////////////////////////////////////////////

void main()
{
	auto unsigned int i;	 	// variable for the loop counter
	auto  int led; 	      // variable used to cycle thru all the LED's

	brdInit();		// Initialize the controller
	dispInit();		// Initialize display module

	for(;;)	// begin an endless loop
	{
		for(led = 0; led <= 6; led++)
		{
			//Turn-on the LED indicated by the led variable
			dispLedOut(led, LEDON);
			for(i=0; i<20000; i++); // time delay loop

			//Turn-off the LED indicated by the led variable
			dispLedOut(led, LEDOFF);
			for(i=0; i<20000; i++); // time delay loop
		}
	}
}
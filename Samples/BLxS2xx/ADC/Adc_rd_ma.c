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
	adc_rd_ma.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
	This program demonstrates how to read an ADC milliamp channel using
	previously defined coefficients. It will also continuously display
	the current that is being monitored.


	Connections:
	============
   1. Set jumpers to connect pins 1-2, 3-4, 5-6, and 7-8 on JP4 for
   4-20 mA current measurements.
	2. Connect a current meter in series as shown below with it set to
   read 4 to 20 milliamps of current.

	-----------------|									 |-------------------------
						  |									 | 4-20ma mode of operation
	Power supply	  |          + |-------| -		 |
	0 - 2.5v		 POS |------------|current|-------| AIN channels 0 - 3
	                 |	         | meter |		 |
	                 |            ---------       |
	                 |    		                   |
                    |                            |
                    |                            |
                    |                            |
 	                 |                            |
	             NEG |----------------------------| AGND
					     |									 |
	-----------------|									 |-------------------------

	Instructions:
	=============
	1. Compile and run this program.
	2. Follow the prompted directions of this program during execution.
	3. Vary voltage (0-2.5v) on power supply to see the CURRENT meter track
	what's being displayed by Dynamic C (4-20ma).

	Note: For best results use a 4 1/2 digit current meter
***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include BLxS2xx series lbrary
#use "BLxS2xx.lib"

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// blank the stdio screen
void  blankScreen(void)
{
   printf("\x1Bt");
}

void main ()
{
	auto int channel;
	auto float current;
	auto int key;
	auto char s[128];

   // Initialize the controller
	brdInit();

   // Configure channels 0-3 for 4-20ma mode of operation
	for (channel = 0; channel < BL_ANALOG_4TO20; ++channel)
   {
   	anaInConfig(channel, mAMP_MODE);
   }

   blankScreen();
	DispStr(2, 2, "A/D input current for channels 0 - 3");
	DispStr(2, 3, "------------------------------------");
   DispStr(2, 10, "Press F4 to return to Edit Mode.");

   while(1)
   {
		for (channel = 0; channel < BL_ANALOG_4TO20; ++channel)
		{
     		current = anaInmAmps(channel);
         if (current > BL_ERRCODESTART)
         {
        		sprintf(s, "Channel = %2d Current = %.3fmA                ",
                     channel, current);
         }
         else if (current == BL_NOT_CAL)
         {
            sprintf(s, "Channel = %2d Current = Not Calibrated     ",channel);
         }
         else
         {
            sprintf(s,"Channel = %2d Current = Exceeded Range!!!     ",channel);
         }
         DispStr(2,channel + 4, s);
		}
   }
}	//end main


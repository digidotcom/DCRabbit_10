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
   ain_sample.c

	This sample program is intended for RabbitNet RN1100 Digital I/O boards.

	Description
	===========
	This program demonstrates how to use the analog input functions on voltage
   inputs.  The program will continuously display the voltage (average
 	of 10 samples) that is present on the A/D channels.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1100 as the search criteria.

	Instructions
	============
	1. Connect a voltmeter to the output of the power supply that you will
   monitor.

	2. Preset the voltage on the power supply to be within the voltage
	range of the A/D converter channel that your going to test. For
	input channels AIN00 and AIN01 input voltage range is 0 to +10V.
   AIN02 input voltage is 0 to +1V and AIN03 input voltage is +/-0.25V.

	3. Power-on the controller.

	4. Connect the output of power supply to one of the AIN channels.

	5. Compile and run this program.

	6. Vary the voltage on a AIN channel, the voltage displayed in the STDIO
	window and voltmeter should track each other.


***************************************************************************/
#class auto					/* Change local var storage default to "auto" */

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board

#define NUMSAMPLES 10
#define STARTCHAN 0
#define ENDCHAN 3


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


///////////////////////////////////////////////////////////////////////////

void main ()
{
	auto int channel, status, device0;
	auto char s[80];
	auto float voltinput[ENDCHAN+1];
	auto rn_search newdev;

	brdInit();                 //initialize controller
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   //search for device match
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

   // enable on all channels for conversions
	for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
	{
		status = rn_anaInConfig(device0, channel, ADCENABLE, 0, 0);
   }

	sprintf(s, "A/D input voltage for channels %d - %d", STARTCHAN, ENDCHAN );
	DispStr(2, 2, s);
	DispStr(2, 3, "--------------------------------------");


	for(;;)
	{
		for (channel = STARTCHAN; channel < ENDCHAN; channel++)
		{
			// sample each channel
			status = rn_anaInVolts(device0, channel, &voltinput[channel], NUMSAMPLES, 0);
		}
      // read channel 3
		status = rn_anaInDiff(device0, channel, &voltinput[channel], NUMSAMPLES, 0);


		for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
		{
			sprintf(s, "Channel = %2d Voltage = %.3f  ", channel, voltinput[channel]);
			DispStr(2, channel+ 6, s);
		}
	}
}
///////////////////////////////////////////////////////////////////////////


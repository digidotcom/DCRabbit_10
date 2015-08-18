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

   ad_sample.c

	This sample program is for the RCM4300 series controllers with
	prototyping boards.

	Description
	===========
	This program demonstrates how to use the A/D low level driver on single-
	ended inputs.  The program will continuously display the voltage (average
	of 10 samples set by NUMSAMPLES) that is present on the A/D channels.
   The gain used for all channels is set by GAINSET.  Both GAINSET and
   NUMSAMPLES can be changed to get different gains or sample sizes.
   Channels which have not been calibrated will always show 0 volts.
   Run ad_cal_chan.c or ad_cal_all.c to calibrate the ADC channels.

	NOTE:	LN7IN is used as a thermistor input and therefore not used in
			this demonstration.

	This sample is not intended for "Code and BIOS in RAM" compile mode.

	Instructions:
	-------------
	1. Connect a voltmeter to the output of the power supply that your
	going to be using.

	2. Preset the voltage on the power supply to be within the voltage
	range of the A/D converter channel that your going to test. For
	A/D Channels LN0IN - LN6IN, input voltage range is 0 to +20V .

	3. Power-on the controller.

	4. Connect the output of power supply to one of the A/D channels.

	5. Compile and run this program.

	6. Vary the voltage on a A/D channel, the voltage displayed in the STDIO
	window and voltmeter should track each other.

***************************************************************************/
#class auto
#use RCM43xx.LIB
#ifndef ADC_ONBOARD
   #error "This core module does not have ADC support.  ADC programs will not "
   #fatal "   compile on boards without ADC support.  Exiting compilation."
#endif
#define NUMSAMPLES 10		//change number of samples here
#define STARTCHAN 0
#define ENDCHAN 6
#define GAINSET GAIN_1		//other gain macros
									//GAIN_1     (0 - 20.00 Volts)
									//GAIN_2     (0 - 11.11 Volts)
									//GAIN_4     (0 -  5.56 Volts)
									//GAIN_5     (0 -  4.44 Volts)
									//GAIN_8     (0 -  2.78 Volts)
									//GAIN_10    (0 -  2.22 Volts)
									//GAIN_16    (0 -  1.39 Volts)
									//GAIN_20    (0 -  1.11 Volts)

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// read the A/D with using the low level driver
nodebug
unsigned int sample_ad(int channel, int num_samples)
{
	auto unsigned long rawdata;
	auto unsigned int sample;
	auto unsigned int cmd;

	//convert channel and gain to ADS7870 format
	// in a direct mode
	cmd = 0x80|(GAINSET*16+(channel|0x08));

	for (rawdata=0, sample=num_samples; sample>0; sample--)
	{
		// execute low level A/D driver
		rawdata += (long) anaInDriver(cmd);
	}
	return ((unsigned int) (rawdata/num_samples));
}

float convert_volt(int channel, int value)
{
	auto float voltage;

	// convert the averaged samples to a voltage
	voltage = (value - _adcCalibS[channel][GAINSET].offset) *
		(_adcCalibS[channel][GAINSET].kconst);

	return voltage;
}

//////////////////////////////////////////////////////////
// millisecond delay
//////////////////////////////////////////////////////////
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

///////////////////////////////////////////////////////////////////////////
main ()
{
	auto int channel;
	auto unsigned int avg_sample;
	auto char s[80];
	auto char display[80];
	auto float ad_inputs[ENDCHAN+1];

	brdInit();

	//initially start up A/D oscillator and charge up cap
	anaIn(0,SINGLE,GAINSET);

	DispStr(1, 1, "\t\t<<< Analog input channels 0 - 6: >>>");
	DispStr(1, 3, "\t LN0IN\t LN1IN\t LN2IN\t LN3IN\t LN4IN\t LN5IN\t LN6IN");
	DispStr(1, 4, "\t------\t------\t------\t------\t------\t------\t------");

	for(;;)
	{
		for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
		{
			// sample each channel
			avg_sample = sample_ad(channel, NUMSAMPLES);
			ad_inputs[channel] = convert_volt(channel, avg_sample);
		}

		display[0] = '\0';
		for(channel =  STARTCHAN; channel <= ENDCHAN; channel++)
		{
			sprintf(s, "\t%6.3f", ad_inputs[channel]);
			strcat(display, s);
		}

		DispStr(1, 5, display);
		msDelay(1000);				//delay one second for viewing
	}
}



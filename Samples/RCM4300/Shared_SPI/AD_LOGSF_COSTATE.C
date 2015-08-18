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
/*****************************************************************

   AD_LOGSF_COSTATE.C

	This sample program is for RCM4300 controllers with
	prototyping boards.

	Description
	===========
	This program is a modification of AD_SAMPLE.C. It demonstrates
   some cooperative multi-tasking techniques for using costatements
   with SPI device API functions when accessing multiple devices
   on the SPI port.

   It also demonstrates use of sbfWriteFlash() and sbfRead().

   It logs ADC data into the non-User Block, non-program area of the
   serial flash once per minute in a circular fashion, and displays
   the logged data in the stdio window.

	Instructions:
	-------------
   1. Calibrate all channels for the gain (set with GAINSET) by running
   either AD_CAL_ALL.C or AD_CAL_CHAN.C and following the instructions
   for these samples.  Calibration factors must be created to get valid
   output from this sample.

	2. Connect a voltmeter to the output of the power supply that you're
	going to be using.

	3. Preset the voltage on the power supply to be within the voltage
	range of the A/D converter channel that you're going to test. For
	A/D Channels LN0IN - LN6IN, input voltage range is 0 to +20V .

	4. Power-on the controller.

	5. Connect the output of power supply to one of the A/D channels.

	6. Compile and run this program.

	7. Vary the voltage on a A/D channel, the voltage displayed in the STDIO
	window and voltmeter should track each other.
****************************************************************/

#memmap xmem
#class auto

#use "RCM43xx.LIB"
#use "bootdev_sf_api.lib"
#use "idblock_api.lib"


#define NUMSAMPLES 10		// change number of samples here
#define STARTCHAN 0
#define ENDCHAN 6
#define GAINSET GAIN_1		// other gain macros

#define MINFLASHADDR 0x00100000UL  // The lowest addr. unvailable for programs
#define MAXFLASHADDR 0x001F0000UL  // Top of flash - 64K

#ifndef ADC_ONBOARD
   #error "This core module does not have ADC support.  ADC programs will not "
   #fatal "   compile on boards without ADC support.  Exiting compilation."
#endif

int readADC();
int logADC();
int displayLog();

int  sampleComplete;        // global flag
int ad_inputsRaw[ENDCHAN+1];
unsigned long logAddr;    // current log position

void main()
{
   auto int rc;

	brdInit();

	// initially start up A/D oscillator and charge up cap
	anaIn(0, SINGLE, GAINSET);

	printf("\t\t<<< Analog input channels 0 - 6: >>>\n");
	printf("\t LN0IN\t LN1IN\t LN2IN\t LN3IN\t LN4IN\t LN5IN\t LN6IN\n");
	printf("\t------\t------\t------\t------\t------\t------\t------\n");

  logAddr = MINFLASHADDR;

   while(1)
   {
 		costate{   // Task 1
			waitfor(ADSPIBUSY != (rc=readADC()));
         if(rc<0){
         	 exit(rc);             // 0 = successful finish
         }
     		waitfor(DelaySec(60));     // Delay, yield to other costate 1 minute
      }

 		costate{  // Task 2
	      waitfor(sampleComplete);   // wait for flag to log & display data
			logADC();
      }
   }
}

// read the A/D with using the low level driver
int sample_ad(int channel, int num_samples)
{
	static unsigned long rawdata;
	static unsigned int sample;
	static unsigned int cmd;
   static int rc;

   costate{
		//convert channel and gain to ADS7870 format
		// in a direct mode
		cmd = 0x80|(GAINSET*16+(channel|0x08));

		for (rawdata=0, sample=num_samples; sample>0; sample--)
		{
			// execute low level A/D driver
         // ADSPIBUSY means the shared SPI is in use by another device.
         // When rc=ADSPIBUSY, execution skips to the end of the costate,
         //  and the function exits.
   	   waitfor(ADSPIBUSY != (rc=anaInDriver(cmd) ));
         if(abs(rc) < 4094){
				rawdata += rc;
         }
         else {
            exit(rc);
         }
         // This gives other tasks some time by jumping
         // the end of the costatement. Execution will
         // proceed after the yield the next time the
         // costatement is entered.
	      yield;
      }
      rc = (int)rawdata/num_samples;

	}   // End costate
	return rc;
}

float convert_volt(int channel, int value)
{
	auto float voltage;

	// convert the averaged samples to a voltage
	voltage = (value - _adcCalibS[channel][GAINSET].offset) *
		(_adcCalibS[channel][GAINSET].kconst);

	return voltage;
}


int logADC()
{
   static int rc;

   costate{

      // -EBUSY means the serial flash is busy writing or erasing a sector.
      //  sbfWriteFlash will return this value many times before any write
      //  is completed because these operations take several milliseconds.
      //  We don't want to busy wait while this happens because an application
      //  will normally have lots of other work to do. So if  sbfWriteFlash
      //  returns -EBUSY, execution jumps to the end of the costate and we
      //  exit the function so that other tasks can run.
  	   waitfor(-EBUSY !=
      	        (rc = sbfWriteFlash(logAddr, ad_inputsRaw,
                                sizeof(int)*(1+ENDCHAN-STARTCHAN)))
        	    );

      if(rc == -1){
          printf("Illegal serial flash write attempt");
          exit(rc);
      }
      else if(!rc)   // 0 = success,
      {              //  rc > means SPI in use by another device, so we just
      					//  fall through and exit the function.

  		   sampleComplete = 0;

         // rc > 0 means SPI in use by another device
         //  if rc > 0, execution jumps to the end of the costate
         //  the function exits.
	  		waitfor((rc=displayLog(logAddr)) <= 0);

         // rc=0 (success) is the only other possibility at this point.

	      logAddr += sizeof(int)*(1+ENDCHAN-STARTCHAN);

      	// Reset to bottom of top half if needed.
         // Additonal handling of the wrap-around would be needed to track the
         // wrap-around if another task were to need the log data - but
         // wrap-around would take about 49 days, and this is just a sample!
		   if(logAddr > MAXFLASHADDR)
		  	{
  			   logAddr = MINFLASHADDR;
		   }
	      sampleComplete = 0;     // reset flag
     	}

	}   // End costate
   return rc;
}

int displayLog(unsigned long logpos)
{
   static float ad_input;
   static char display[80],strbuff[14];
   static int rc, channel;

   costate{
	   // Read back from serial flash. rc = 0 menas successful finish.
      // If rc is non-zero execution skips to the end of the costate and
      // and the function exits.
	   waitfor(!(rc =
         sbfRead(ad_inputsRaw, logpos, sizeof(int)*(1+ENDCHAN-STARTCHAN))));

	   display[0] = 0;

	   for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
	   {
	      ad_input = convert_volt(channel, ad_inputsRaw[channel]);
	      sprintf(strbuff, "\t%6.3f", ad_input);
	      strcat(display, strbuff);
	   }
	   strcat(display, "\n");
	   printf("%s",display);

	}   // End costate
   return rc;
}

int readADC()
{
   // Read all channels into ad_inputsRaw[] array every minute
   // Possible return values: 0 = success, ADSPIBUSY = SPI port in use
   // Error exits for other ADC errors

   static int rc, avg_sample, channel;

   costate{
      sampleComplete = 0;

		for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
		{
			//***** Sample each channel *****
			waitfor(ADSPIBUSY != (rc = sample_ad(channel, NUMSAMPLES)) );

         if(abs(rc) > 4094){
            // ADC Error occurred
           	exit(rc);
         }
         ad_inputsRaw[channel] = rc;
		}
      sampleComplete = 1;        // Flag that we're ready to log sample
      rc = 0;
   }
   return rc;
}


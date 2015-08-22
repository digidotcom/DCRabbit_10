/*****************************************************************

   AD_LOGSF_UCOS.C

	Rabbit Semiconductor, 2007

	This sample program is for RCM4300 controllers with
	prototyping boards.

	Description
	===========
	This program is a modification of AD_SAMPLE.C. It demonstrates
   setting up uC/OS-II to use _rcm43_InitUCOSMutex() to protect
   against multiple devices accessing the SPI from different tasks
   (although that isn't actually a danger in this simple sample).

   It also demonstrates use of sbfWriteFlash() and sbfRead().

   It logs ADC data into the non-User Block, non-program area of the
   serial flash once per minute in a circular fashion, and displays
   the logged data in the stdio window.

   This program requires the Dynamic C uC/OS-II Module.

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



#define NUMSAMPLES 10		// change number of samples here
#define STARTCHAN 0
#define ENDCHAN 6
#define GAINSET GAIN_1		// other gain macros

#define MINFLASHADDR 0x00100000UL  // The lowest addr. unvailable for programs
#define MAXFLASHADDR 0x001F0000UL  // Top of flash - 64K

#define _SPI_USE_UCOS_MUTEX        // Use mutex to prevent SPI device conflicts
#define OS_MUTEX_EN		       1	  // Must enable u/COS-II mutexes to use the
                                   //  internal mutex mechanism in the SPI API

#define OS_TIME_DLY_HMSM_EN	 1   // Enable hour, min, sec, ms delays

#define OS_MAX_EVENTS          1   // Maximum events - need 1 one for SPI mutex
#define OS_TASK_CREATE_EN	    1	  // Enable normal task creation


#ifndef ADC_ONBOARD
   #error "This core module does not have ADC support.  ADC programs will not "
   #fatal "   compile on boards without ADC support.  Exiting compilation."
#endif

// Bring in library
#use "ucos2.lib"

#use "RCM43xx.LIB"
#use "bootdev_sf_api.lib"
#use "idblock_api.lib"

int  sampleComplete;        // global flag
void readADC(void* pdata);
void logADC(void* pdata);
void displayLog();

int ad_inputsRaw[ENDCHAN+1];

void main()
{
   char rc;
   OSInit();

   // MUST Initialize SPI mutex at higher priority than other tasks.
   // MUST Initialize after OSInit() call and before brdInit() call,
   //  because brdInit() will use the mutex initializing the ADC.
   _rcm43_InitUCOSMutex(4);

	brdInit();

   rc = OSTaskCreate(readADC, NULL, 512, 5);
   rc = OSTaskCreate(logADC,  NULL, 512, 6);   // Logging is lowest priority

	printf("\t\t<<< Analog input channels 0 - 6: >>>\n");
	printf("\t LN0IN\t LN1IN\t LN2IN\t LN3IN\t LN4IN\t LN5IN\t LN6IN\n");
	printf("\t------\t------\t------\t------\t------\t------\t------\n");

   OSStart();    // Start multi-tasking
}

// read the A/D with using the low level driver
int sample_ad(int channel, int num_samples)
{
	auto unsigned long rawdata;
	auto unsigned int sample, cmd;

   //convert channel and gain to ADS7870 format
	// in a direct mode
	cmd = 0x80|(GAINSET*16+(channel|0x08));

	for (rawdata=0, sample=num_samples; sample>0; sample--)
	{
		rawdata += anaInDriver(cmd);
	}
	return (int)rawdata/num_samples;;
}

float convert_volt(int channel, int value)
{
	auto float voltage;

	// convert the averaged samples to a voltage
	voltage = (value - _adcCalibS[channel][GAINSET].offset) *
		(_adcCalibS[channel][GAINSET].kconst);

	return voltage;
}

void logADC(void* pdata)
{
	auto unsigned long logAddr;  // current log position

   logAddr = 0x00100000UL;

   while(1){
      if(sampleComplete==1)
		{
			if(sbfWriteFlash(logAddr, ad_inputsRaw,
               sizeof(int)*(1+ENDCHAN-STARTCHAN)) )
         {
         	printf("Error writing flash");
         }

   		displayLog(logAddr);

		   logAddr += sizeof(int)*(1+ENDCHAN-STARTCHAN);

         // Reset to bottom of top half if needed
		   if(logAddr > MAXFLASHADDR)
		  	{
    		   logAddr = MINFLASHADDR;
		   }
         sampleComplete=0;     // Wait for flag again
      }
   }
}

void displayLog(unsigned long logpos)
{
   auto float ad_input;
   auto char display[80],strbuff[14];
   auto int channel;

   // Read back from serial flash
	sbfRead(ad_inputsRaw, logpos, sizeof(int)*(1+ENDCHAN-STARTCHAN));

   display[0] = 0;

   for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
   {
		ad_input = convert_volt(channel, ad_inputsRaw[channel]);
		sprintf(strbuff, "\t%6.3f", ad_input);
		strcat(display, strbuff);
   }
	strcat(display, "\n");
   printf("%s",display);
}

void readADC(void* pdata)
{
   auto int avg_sample, channel;
   auto int rc;

   sampleComplete = 0;

	// initially start up A/D oscillator and charge up cap
	anaIn(0, SINGLE, GAINSET);

   while(1){

		for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
		{
			// sample each channel
			ad_inputsRaw[channel] = sample_ad(channel, NUMSAMPLES);
		}
      sampleComplete = 1;        // Flag that we're ready to log sample
		OSTimeDlyHMSM(0,1,0,0);    // Delay, yield to other tasks 1 minute
   }
}


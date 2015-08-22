/***************************************************************************
   dac_cal_volts.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
	This program demonstrates how to recalibrate a DAC channel
   using a known voltage to generate calibration constants for
   the given channel and then will write the data into reserved
   eeprom.

	Connections:
	============
   1. Set jumpers to connect pins 1-3 and 2-4 on JP5 for voltage outputs
	2. Set jumpers to connect pins 1-2 and 3-4 on JP3 for channel 0 unipolar;
   	or, set jumper to connect pins 5-6 on JP3 for channel 0 bipolar
      NOTE: Set channel 0 & 1 to the same mode, both unipolar or both bipolar
	3. Set jumpers to connect pins 1-2 and 3-4 on JP6 for channel 1 unipolar;
		or, set jumper to connect pins 5-6 on JP6 for channel 1 bipolar
	4. Connect a voltmeter to one of the DAC output channels labeled
      AOUT0 - AOUT1 on PCB.

	Instructions:
	=============
	1. Compile and run this program.
	2. Follow the prompted directions of this program during execution.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series lbrary
#use "BLxS2xx.lib"

// Note: The DAC output goes thru a inverting amplifier, the higher
// rawdata value the lower the output voltage.
#define HICOUNT 3695		// Sets up a low voltage calibration point
#define LOCOUNT 400     // Sets up a high voltage calibration point

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// clear lines in the stdio window
void  clearLines(int start, int end)
{
	char buffer[127];
   int i;

 	memset(buffer, ' ', sizeof(buffer) - 1);
   buffer[sizeof(buffer)-1] = '\0';
   for (i = start; i <= end; ++i)
   {
   	DispStr(0, i, buffer);
   }
}

///////////////////////////////////////////////////////////////////////////

void main()
{
   float volt1, volt2, voltout;
   float min_volts, max_volts;
   int channel;
   char dac_mode;
 	int key;
   char tmpbuf[128];
	char display[128];

   // Initialize controller
	brdInit();

   DispStr(1, 2, "DAC output voltage configuration");
   DispStr(1, 3, "Dependant on jumper settings (see Instructions)");
   DispStr(1, 4, "--------------------------------");
   DispStr(1, 5, "0 = Unipolar  0  to +10v");
   DispStr(1, 6, "1 = Bipolar  -10 to +10v");
   DispStr(1, 8, "Please enter the DAC configuration 0 - 1 = ");
   do
   {
      key = getchar() - '0';
   } while (key < 0 || key > 1);
   printf("%d", key);

   // Configure the DAC for given configuration
   dac_mode = (key == 0 ? DAC_UNIPOLAR : DAC_BIPOLAR);
   anaOutConfig(dac_mode, DAC_ASYNC);

   // set the min and max voltage based on DAC mode.
   max_volts = 10.0;
   min_volts = (dac_mode == DAC_BIPOLAR ? -10.0 : 0.0);

   // display user options
   DispStr(1, 10, "User Options:");
   DispStr(1, 11, "-------------");
   DispStr(1, 12, "1. DAC 0: Calibrate");
   DispStr(1, 13, "2. DAC 1: Calibrate");
   DispStr(1, 14, "3. DAC 0: Set voltage output");
   DispStr(1, 15, "4. DAC 1: Set voltage output");

   while(1)
   {
	   while(!kbhit);
	   key = getchar() - '0';
	   switch (key)
	   {
	      case 1: // DAC 0: Calibrate
	      case 2: // DAC 1: Calibrate
	         channel = key - 1; // calculate channel from the menu option

	         DispStr(1, 18, "!!!Caution this will overwrite the calibration "\
	                "constants set at the factory.");
	         DispStr(1, 19, " Do you want to continue(Y/N)?");
	         do
            {
	            while(!kbhit());
	            key = getchar();
	            if (key == 'N' || key == 'n')
	            {
	               exit(0);
	            }
	         } while(key != 'Y' && key != 'y');

	         // set two known voltage points using rawdata values, the
	         // user will then type in the actual voltage for each point
	         anaOut(channel, HICOUNT);
	         sprintf(display, "DAC %d: ENTER the voltage reading from meter "\
	                          "(approx. %.2fV)  = ", channel,
            					  (.08 * (max_volts - min_volts)) + min_volts);
	         DispStr(1, 21, display);
	         volt1 = atof(gets(tmpbuf));

	         anaOut(channel, LOCOUNT);
	         sprintf(display, "DAC %d: ENTER the voltage reading from meter "\
	                          "(approx. %.2fV) = ", channel,
	            				  (.92 * (max_volts - min_volts)) + min_volts);
	         DispStr(1, 22, display);
	         volt2 = atof(gets(tmpbuf));

	         // Create calibration data and store into flash memory
	         anaOutCalib(channel, dac_mode, HICOUNT, volt1, LOCOUNT, volt2);

	         sprintf(display, "DAC %d: now calibrated...", channel);
	         DispStr(1, 23, display);

            DispStr(1, 24, "(press any key to continue)");
            while(!kbhit);
            getchar();
            clearLines(18, 24);
				break;
	      case 3: // DAC 0: Set voltage output
	      case 4: // DAC 1: Set voltage output
	         channel = key - 3; // calculate the channel from the menu option.
	         do
	         {
	            // display DAC voltage message
               clearLines(18, 18);
	            sprintf(display, "DAC %d: ENTER desired voltage (%.2f to %.2fV)"\
                						"= ", channel, min_volts, max_volts);
               DispStr(1, 18, display);
               voltout = atof(gets(tmpbuf));
	         } while (voltout < min_volts || voltout > max_volts);

	         // send voltage value to DAC for it to output the voltage
	         anaOutVolts(channel,  voltout);
            sprintf(display, "DAC %d: Observe voltage on meter...", channel);
            DispStr(1, 19, display);

            DispStr(1, 20, "(press any key to continue)");
            while(!kbhit);
            getchar();
            clearLines(18, 20);
				break;
	   }
	} // while(1)
}


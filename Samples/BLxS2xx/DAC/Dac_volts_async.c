/***************************************************************************
   dac_volts_async.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
   This program outputs a voltage that can be read with a voltmeter.
   The output voltage is computed with using the calibration constants
   that are located in reserved eeprom.

   The DAC circuit is setup for Asynchronous mode of operation which
   will update the DAC output at the time it's being written via the
   anaOut or anaOutVolts functions.

	Instructions:
   =============
   1. Set jumpers to connect pins 1-3 and 2-4 on JP5 for voltage outputs
	2. Set jumpers to connect pins 1-2 and 3-4 on JP3 for channel 0 unipolar;
   	or, set jumper to connect pins 5-6 on JP3 for channel 0 bipolar
      NOTE: Set channel 0 & 1 to the same mode, both unipolar or both bipolar
	3. Set jumpers to connect pins 1-2 and 3-4 on JP6 for channel 1 unipolar;
		or, set jumper to connect pins 5-6 on JP6 for channel 1 bipolar
	4. Connect a voltmeter to one of the DAC output channels labeled
      AOUT0 - AOUT1 on PCB
	5. Compile and run this program
	6. Follow the prompted directions of this program during execution

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series lbrary
#use "BLxS2xx.lib"

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
	auto char buffer[127];
   auto int i;

 	memset(buffer, ' ', sizeof(buffer) - 1);
   buffer[sizeof(buffer)-1] = '\0';
   for (i = start; i <= end; ++i)
   {
   	DispStr(0, i, buffer);
   }
}

void main()
{
   int dac_mode;
   float voltout;
   float min_volts, max_volts;
   int channel;
   int key;
	char display[128];
 	auto char tmpbuf[128];

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

   // print the menu once here.
   DispStr(2, 10, "User Options:");
   DispStr(2, 11, "-------------");
   DispStr(2, 12, "1. DAC 0: Set new voltage");
   DispStr(2, 13, "2. DAC 1: Set new voltage");
   DispStr(2, 15, "Note: Voltages are immediately output by the DAC. " \
   					"(No need to strobe)");

   while(1)
   {
      // get response for menu choice
      do
      {
         key = getchar() - '0';
      } while (key < 1 || key > 2);

      channel = key - 1; // calculate channel from menu selection.
      						 // menu 1 = ch 0 and menu 2 = ch 1
      do
      {
         // display DAC voltage message
         clearLines(17, 17); // make sure the line is clear
         sprintf(display, "DAC %d: Desired voltage (%.0f to %.0fV) = ",
                  channel, min_volts, max_volts);
         DispStr(3, 17, display);
         voltout = atof(gets(tmpbuf));
      } while (voltout < min_volts || voltout > max_volts);

      // send voltage value to DAC for it to output
      anaOutVolts(channel,  voltout);

      clearLines(17, 17); // clear the optional input line after the menu
	} // while (1)
}



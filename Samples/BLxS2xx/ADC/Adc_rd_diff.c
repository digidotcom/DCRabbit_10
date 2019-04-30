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
	adc_rd_diff.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
	This program reads and displays voltage and equivalent values
   of a differential ADC channel. The A/D rawdata and equivalent
   voltage is selectable by the user for display.

	Connections:
	============
	To read differential voltages you will need a power supply
	that has one floating output and a second output or a second
   power supply.

	NOTE:	Before doing the following steps, set the power supply(s)
	      to zero volts and then turn it OFF.

   1. Remove all jumpers on header JP4.

	2. Initially connect the positive side of the floating power supply to AGND
   	and the negative side to the negative side of one of the following ADC
      differential channel pairs.  This will create a negative voltage.

      Channel   	DIFF Pairs on J12
	    -------  -----------------------------
	      0 		 +AIN0 (pin 12)  -AIN1 (pin 5)
	      2		 +AIN2 (pin 11)  -AIN3 (pin 4)
	      4		 +AIN4 (pin 3)	  -AIN5 (pin 9)
	      6		 +AIN6 (pin 2)	  -AIN7 (pin 8)

	Note: AGND is available on header J12 on pins: 1, 6, 10, and 14.

   3. Connect the negative side of the other power supply to AGND and the
   	positive side to the positive side of one of the ADC differential
      channel pairs from above.

	Example connections for channel 0:
	   Floating Power Supply	Power Supply 2
		---------------------	--------------
      +	<->   AGND				+	<->	AIN0
	   -	<->   AIN1				-	<->	AGND

	Instructions:
	=============
	1. Power-on the controller.
	2. Compile and run this program.
	3. Turn ON the power supply for the ADC calibration.
	4. Follow the prompted directions of this program during execution.

***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include BLxS2xx series library
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

void printrange()
{
	printf("\n\n");
	printf(" Gain Code\tVoltage range\n");
	printf(" ---------\t-------------\n");
	printf("    0     \t +- 20 \n");
	printf("    1     \t +- 10\n");
	printf("    2     \t +- 5\n");
	printf("    3     \t +- 4\n");
	printf("    4     \t +- 2.5\n");
	printf("    5     \t +- 2\n");
	printf("    6     \t +- 1.25\n");
	printf("    7     \t +- 1\n\n");
}


main ()
{
	int channel, gaincode;
	float voltage;
	char s[128];

   // Initialize the controller
	brdInit();

   // Configure all channel pairs for differential mode of operation
   // (Max voltage range is ±20V)
   for (channel = 0; channel < BL_ANALOG_IN; channel += 2)
   {
      anaInConfig(channel, DIFF_MODE);
   }

	while (1)
	{
     	printrange();
		printf(" Choose gain code (0-%d) =  ", BL_MAX_GAINS - 1);
		do
		{
			while(!kbhit()); // wait for key hit
			gaincode = getchar() - '0';  // convert ascii key into number
		} while ((gaincode < 0) || (gaincode >= BL_MAX_GAINS));
		printf("%d", gaincode);

      blankScreen();
		DispStr(1, 2, "A/D voltage for channel pairs 0&1, 2&3, 4&5 and 6&7");
		DispStr(1, 3, "---------------------------------------------------");
   	DispStr(1, 10,"Press key to select another gain option.");

   	while (1)
      {
			for (channel = 0; channel < BL_ANALOG_IN; channel += 2)
			{
      		voltage = anaInDiff(channel, gaincode);
            if (voltage > BL_ERRCODESTART)
            {
            	// valid read
         		sprintf(s,
                "Channel = %d&%d Voltage = %.3fV                              ",
                           channel, channel+1, voltage);
         	}
            else if (voltage == BL_NOT_CAL)
            {
               sprintf(s, "Channel = %d&%d Voltage = Not Calibrated     ",
                        channel, channel + 1);
            }
            else
            {
               sprintf(s, "Channel = %d&%d Voltage = Exceeded Range     ",
                        channel, channel + 1);
            }
            DispStr(1,(channel / 2) + 4, s);
			}
         if (kbhit())
			{
				getchar();
            blankScreen();
            break;
			}
		}
   }
}
///////////////////////////////////////////////////////////////////////////


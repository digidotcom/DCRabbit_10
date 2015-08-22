/***************************************************************************
	adc_rd_diff.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
   Reads and displays the voltage of all differential analog input
   channels. The voltage is calculated from coefficients read from
   the flash.

	Connections:
	============
	TO read differential voltages you will need a power supply
	that has two independant outputs, or two power supplies.

	NOTE:	Before doing the following steps, set the power supply(s)
	      to zero volts and then turn it OFF.

   1. Remove jumpers across pins 1-2 and 5-6 on headers J10 and J11.
	2. Initially connect the negative side of both outputs of the power
   	supply to AGND.
   3. Connect the positive side of one output of the power supply to the
   	positive side of one of the following ADC differential channel pairs.

	    Channel   DIFF Pairs
	    -------  ------------
	      0 		 +AIN0  -AIN1
	      2		 +AIN2  -AIN3
	      4		 +AIN4  -AIN5
	      6		 +AIN6  -AIN7

   4. Connect the positive side of the other output of the power supply to
   	the negative side of one of the ADC differential channel pairs from
      above.

	Example connections for channel 0:
	   Power Supply 1		Power Supply 2
		--------------		--------------
      +	<->   AIN0		+	<->	AIN1
	   -	<->   AGND		-	<->	AGND

	Instructions:
	=============
	1. Compile and run this program.
	2. Follow directions in STDIO window.
	3. Turn ON the power supply for the ADC calibration.
	4. Voltage will be continuously displayed for all channels.

***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include series BL4S1xx library
#use "BL4S1xx.LIB"

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

//print voltage ranges
void printrange( void )
{
	int i;
	printf("\n\n");
   printf(" Gain code\tVoltage range\n");
   printf(" ---------\t-------------\n");
	for (i = 0; i < BL_MAX_GAINS; ++i)
   {
		printf("     %d    \t +- %5.2fv\n", i, 20.0 / _gainTable[i]);
   }
}

main ()
{
	int channel, gaincode;
	float voltage;
	int key;
	char s[128];

   // Initialize the controller
	brdInit();

   // Configure channel pair 0 & 1, 2 & 3, 4 & 5, and 6 & 7 for
   // differential mode of operation
   // (Max voltage range is ±20V)
	for (channel = 0; channel < BL_ANALOG_IN; channel+=2)
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
			gaincode = getchar() - '0';
		} while (gaincode < 0 || gaincode >= BL_MAX_GAINS);
		printf("%d", gaincode);

      blankScreen();
		DispStr(1, 2, "A/D voltage for channel pairs 0&1, 2&3, 4&5 and 6&7");
		DispStr(1, 3, "---------------------------------------------------");
   	DispStr(1, 10,"Press key to select another gain option.");

   	while(1)
      {
			for(channel = 0; channel < BL_ANALOG_IN; channel+=2)
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
            DispStr(1, (channel / 2) + 4, s);
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


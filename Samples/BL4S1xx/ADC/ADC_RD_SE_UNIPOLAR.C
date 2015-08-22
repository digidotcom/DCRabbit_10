/***************************************************************************
	adc_rd_se_unipolar.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
   Reads and displays the voltage of all single-ended analog input
   channels. The voltage is calculated from coefficients read from
   the flash.

	Connections:
	============
   1. Remove jumpers across pins 1-2 and 5-6 on headers J10 and J11.
   2. Connect the positive power supply lead to an input channel.
	3.	Connect the negative power supply lead to AGND on the controller.

	Instructions:
	=============
	1. Compile and run this program.
	2. Follow directions in STDIO window.
	3. Voltage will be continuously displayed for all channels.

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
		printf("     %d    \t0 - %5.2fv\n", i, 20.0 / _gainTable[i]);
   }
}

void main ()
{
	auto int channel;
	auto int key;
	auto int gaincode;
	auto float voltage;
   auto char s[128];

   // Initialize the controller
	brdInit();

   // Configure channels 0-7 for Single-Ended unipolar mode of operation.
   // (Max voltage range is 0 - 20v)
	for (channel = 0; channel < BL_ANALOG_IN; ++channel)
   {
   	anaInConfig(channel, SE0_MODE);
   }

	while (1)
	{
     	printrange();
		printf(" Choose gain code (0-%d) =  ", BL_MAX_GAINS - 1);
		do
		{
      	while(!kbhit()); // wait for key hit
			gaincode = getchar() - '0'; // convert ascii key into number
		} while (gaincode < 0 || gaincode >= BL_MAX_GAINS);
		printf("%d", gaincode);

      blankScreen();
		DispStr(1, 2,  "A/D input voltage for channels 0 - 7");
		DispStr(1, 3,  "------------------------------------");
   	DispStr(1, 14, "Press key to select another gain option.");

   	while(1)
      {

			for(channel = 0; channel < BL_ANALOG_IN; ++channel)
			{
         	voltage = anaInVolts(channel, gaincode);
            if (voltage > BL_ERRCODESTART)
            {
            	// valid read
         		sprintf(s, "Channel = %2d Voltage = %.3fV              ",
               			channel, voltage);
         	}
            else if (voltage == BL_NOT_CAL)
            {
               sprintf(s, "Channel = %2d Voltage = Not Calibrated     ",
                        channel);
            }
            else
            {
               sprintf(s, "Channel = %2d Voltage = Exceeded Range     ",
                        channel);
            }
            DispStr(1,channel + 4, s);
         }

         if (kbhit())
			{
				getchar();
            blankScreen();
            while (kbhit()) { getchar(); }
            break;
			}
      }
   }
}	//end main


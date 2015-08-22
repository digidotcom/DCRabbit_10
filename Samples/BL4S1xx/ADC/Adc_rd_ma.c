/***************************************************************************
	adc_rd_ma.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
   Reads and displays the current of all 4-20mA analog input
   channels. The voltage is calculated from coefficients read from
   the flash.


	Connections:
	============
   1. Place jumpers across pins 1-2 and 5-6 on headers J10 and J11.
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
	2. Follow directions in STDIO window.
	3. Vary voltage (0-2.5v) on power supply to see the CURRENT meter track
	what's being displayed by Dynamic C (4-20ma).

	Note: For best results use a 4 1/2 digit current meter
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
   DispStr(2, 10, "Press Q or q to exit program.");

   while(1)
   {
		for(channel = 0; channel < BL_ANALOG_4TO20; channel++)
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
            sprintf(s,"Channel = %2d Current = Exceeded Range      ",channel);
         }
         DispStr(2, channel + 4, s);
		}
      if(kbhit())
		{
			key = getchar();
			if (key == 'Q' || key == 'q')		// check if it's the q or Q key
			{
      		break;
     		}
		}
   }
}	//end main


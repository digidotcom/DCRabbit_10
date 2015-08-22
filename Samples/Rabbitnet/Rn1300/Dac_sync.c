/***************************************************************************
   dac_sync.c
   Digi International, Copyright © 2005-2008.  All rights reserved.

   This sample program is intended for RabbitNet RN1300 DAC boards.

   Description
   ===========
   This program outputs a voltage that can be read with a voltmeter.
   The output voltage is computed with using the calibration constants
   that are located on the DAC board EEPROM.

   The DAC board is setup for synchronously mode of operation which
   updates all DAC outputs at the same time when the anaOutStrobe
   function executes. The outputs all updated with values previously
   written with anaOutVolts and/or anaOut functions.

   Note: For Asynchronously mode of operation, please see the sample
   program dac_async.c.


	Instructions:
   -------------
   1.Verify that the power supply for the DAC board meets the requirements
     for both your application and the DAC board (i.e. Power supply must be
     3 volts greater than the MAX DAC output voltage range selected).
	2. Connect a voltage meter to an output channel DAC0 - DAC7.
	3. Compile and run this program.
	4. Follow the prompted directions of this program during execution.
***************************************************************************/

#class auto					/* Change local var storage default to "auto" */

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1300			//match DAC board product ID

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



void  blankScreen(int start, int end)
{
	auto char buffer[256];
   auto int i;

   memset(buffer, 0x00, sizeof(buffer));
 	memset(buffer, ' ', sizeof(buffer));
   buffer[sizeof(buffer)-1] = '\0';
   for(i=start; i < end; i++)
   {
   	DispStr(start, i, buffer);
   }
}


void main()
{
	auto int device0, status;
	auto char tmpbuf[24];
	auto int done, command;
   auto rn_search newdev;
   auto DacCal DacCalTable1;
   auto float voltout;
   auto int channel, selectChannel;
   auto int key;

	brdInit();		// Required for controllers
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   //search for device match
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

	for(;;)
	{
   	blankScreen(0, 28);

		DispStr(2, 2, "DAC Board   CH0&1   CH2-7 ");
		DispStr(2, 3, "--------------------------");

		DispStr(2, 4, "Config = 0  2.5v    10v");
		DispStr(2, 5, "Config = 1  5.0v    10v");
		DispStr(2, 6, "Config = 2  10v     10v");
		DispStr(2, 7, "Config = 3  5v      20v");
		DispStr(2, 8, "Config = 4  10v     20v");
		DispStr(2, 9, "Config = 5  20v     20v");
		DispStr(2, 10, "Please enter the DAC configuration 0 - 5....");
		do
		{
			command = getchar();
		} while (!((command >= '0') && (command <= '5')));
		printf("Config  = %d", command-=0x30);
      rn_anaOutConfig(device0, command, 1, 0);
      for(channel=0; channel < 8; channel++)
      {
      	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
      }


     	done = FALSE;
      selectChannel = TRUE;
		while (!done)
		{
         if(selectChannel)
         {
         	DispStr(2, 12, "DAC0 - DAC7 Voltage Out Program");
				DispStr(2, 13, "-------------------------------");
				DispStr(2, 14, "Please enter an output channel (0 - 7) = ");
				do
				{
		  			channel = getchar();
				} while (!((channel >= '0') && (channel <= '7')));
				printf("%d", channel-=0x30);
            selectChannel = FALSE;
        	}

         // display DAC voltage message
			DispStr(2, 16, "Type a desired voltage (in Volts) =  ");

			// get user voltage value for the DAC thats being monitored
			voltout = atof(gets(tmpbuf));

 			// send voltage value to DAC for it to output the voltage
     		rn_anaOutVolts(device0, channel,  voltout, &DacCalTable1, 0);

 			// display user options
			DispStr(2, 19, "User Options:");
			DispStr(2, 20, "-------------");
			DispStr(2, 21, "1. Write voltage value to DAC channel (internal register)");
         DispStr(2, 22, "2. Strobe DAC chip, all DAC channels will be updated");
         DispStr(2, 23, "3. Change to another DAC channel");
			DispStr(2, 24, "4. Change overall DAC output configuration");
			DispStr(2, 25, "5. Exit Program");

         DispStr(2, 28, "Note: Must strobe DAC for outputs to be updated!");
         while(1)
			{
				// wait for a key to be pressed
				while(!kbhit());
				key = getchar();
            if(key == '1')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               blankScreen(16, 28);
   				break;
   			}

            if(key == '2')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               rn_anaOutStrobe(device0, 0);
               DispStr(2, 27, "DAC outputs have been updated");
               msDelay(1000);
               DispStr(2, 27, "                                ");
   			}

            if(key == '3')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               blankScreen(12, 28);
               selectChannel = TRUE;
   				break;
   			}

				if(key == '4')
				{
					// exit while loop and clear previous calibration infor
					done = TRUE;
  					blankScreen(15, 28);
					// empty keyboard buffer
					while(kbhit()) getchar();
					break;
				}
				if (key == '5')		// check if it's the q or Q key
				{
					// exit sample program
     				exit(0);
   			}
			}
		}
	}
}
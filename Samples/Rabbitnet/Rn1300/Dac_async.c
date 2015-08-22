/***************************************************************************
   dac_async.c
   Digi International, Copyright © 2005-2008.  All rights reserved.

   This sample program is intended for RabbitNet RN1300 DAC boards.

   Description
   ===========
   This program outputs a voltage that can be read with a voltmeter.
   The output voltage is computed with using the calibration constants
   that are located on the DAC board EEPROM.

   The DAC board is setup for Asynchronously mode of operation which
   updates a DAC output at the time its being accessed via the anaOutVolts,
   or anaOut functions. (i.e. anaOutStrobe function is not used to update
   DAC outputs).

   Note: For Synchronously mode of operation, please see the sample
   program dac_sync.c.


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

//////
// This function for this demonstration only by leaving chip
// select asserted to force a hardware watchdog timeout
//////
int watchdogTimeout(int handle, int regno, char *recdata, int datalen)
{
	auto rn_devstruct *devaddr;
   auto rnDataSend ds;
   auto rnDataRec dr;
	auto unsigned long done_time;

   // slight delay
   done_time = MS_TIMER + 200;
   while( (long) (MS_TIMER - done_time) < 0 );

	devaddr = (rn_devstruct *)handle;

	//assemble data
   ds.cmd = RCMD|regno;
   memset(ds.mosi, ds.cmd, datalen);
   datalen++;

   // Asserting chip-select and sending a command byte will cause the microcontroller
   // to wait for additional bytes without hitting the Watchdog timer.
	rn_sp_enable(devaddr->portnum);
 	_mosi_driver(datalen, &ds, &dr, &devaddr->cmdtiming, &rn_spi[devaddr->portnum]);

   return (dr.statusbyte);
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

//#define nodebug
void main()
{
	auto int device0, status;
 	auto char tmpbuf[24];
   auto char recbyte;
   auto int done, command;
   auto rn_search newdev;
   auto DacCal DacCalTable1;
   auto float voltout;
   auto int channel, chSelected, selectChannel;
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
   	blankScreen(0, 30);

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
      rn_anaOutConfig(device0, command, 0, 0);
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
            chSelected = channel;
            selectChannel = FALSE;
         }

         // display DAC voltage message
			DispStr(2, 16, "Type a desired voltage (in Volts) =  ");

			// get user voltage value for the DAC thats being monitored
			voltout = atof(gets(tmpbuf));

 			// send voltage value to DAC for it to output the voltage
     		rn_anaOutVolts(device0, chSelected,  voltout, &DacCalTable1, 0);
			DispStr(2, 17, "Observe voltage on meter.....");

			// display user options
			DispStr(2, 19, "User Options:");
			DispStr(2, 20, "-------------");
			DispStr(2, 21, "1. Change the voltage on current DAC channel");
  			DispStr(2, 22, "2. Change to another DAC channel");
			DispStr(2, 23, "3. Change overall DAC output configuration");
			DispStr(2, 24, "4. Software Reset...No change on DAC outputs, keeps configuration");
         DispStr(2, 25, "5. Hardware Reset...All outputs go to ~0 volts, must re-configure DAC");


			while(1)
			{
				// wait for a key to be pressed
				while(!kbhit());
				key = getchar();
            if(key == '1')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               blankScreen(16, 30);
   				break;
   			}
            if(key == '2')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               blankScreen(12, 30);
               selectChannel = TRUE;
   				break;
   			}

				if(key == '3')
				{
					// exit while loop and clear previous calibration infor
					done = TRUE;
  					blankScreen(15, 30);
					// empty keyboard buffer
					while(kbhit()) getchar();
					break;
				}
				if (key == '4')
				{
                // first set SW watchdog timeout
					rn_sw_wdt(device0, 2.5);

   				// then enable SW watchdog
					rn_enable_wdt(device0, 2);

               DispStr(2, 28, "Waiting for Software reset");
            	msDelay(3000);

            	// reading clears reset status register
					status = rn_rst_status(device0, &recbyte);
            	if ((status&0x01) && (recbyte&0x40))    //check SW RST bit set
            	{
				  		DispStr(2, 29, "Software reset occurred as expected");
            	}
               else
               {
                  DispStr(2, 29, " Error! Software reset did not occur as expected");
               }
               msDelay(2000);
               DispStr(2, 28, "                                                            ");
               DispStr(2, 29, "                                                            ");

    			}
            if (key == '5')
				{
            	rn_enable_wdt(device0, 1);  //enable device hardware watchdog

                // Force watchdog timeout
 					watchdogTimeout(device0, 0000, &recbyte, 1);
               DispStr(2, 28, "Waiting for Watchdog hardware reset");
            	msDelay(2000);

               // Check if watchdog timeout occurred
               status = rn_rst_status(device0, &recbyte);
         		if ((status&0x01) && (recbyte&0x80))	//check watchdog timeout bit set
         		{
         	  			DispStr(2, 29, "Watchdog hardware reset occurred as expected");
               }
               else
               {
                  DispStr(2, 29, "Error! Watchdog hardware reset did not occur as expected");
               }
               msDelay(2000);
               DispStr(2, 30, "Re-initializing DAC outputs to selected configuration");
               rn_anaOutConfig(device0, command, 0, 0);
      			for(channel=0; channel < 8; channel++)
      			{
      				rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
      			}
               msDelay(4000);
               DispStr(2, 28, "                                                                 ");
               DispStr(2, 29, "                                                                 ");
               DispStr(2, 30, "                                                                 ");

             }
			}
		}
	}
}
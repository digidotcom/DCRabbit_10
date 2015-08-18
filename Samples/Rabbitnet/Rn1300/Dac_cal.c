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
   dac_cal.c

   This sample program is intended for RabbitNet RN1300 DAC boards.

   Description
   ===========
 	This program demonstrates how to recalibrate an DAC channel using
	two known voltages and defines the two coefficients, gain and offset,
	which will be rewritten into the DAC's EEPROM.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1300 as the search criteria.

	!!!Caution this will overwrite the calibration constants set at
	   the factory.


	Instructions:
   -------------
   1.Verify that the power supply for the DAC board meets the requirements
     for both your application and the DAC board (i.e. Power supply must be
     3 volts greater than the MAX DAC output voltage range selected).
	2.Connect a voltage meter to an output channel.
	3.Compile and run this program.
	4.Follow the prompted directions of this program during execution.
***************************************************************************/
#class auto

#define LOCOUNT 400		//sets up a high voltage calibration point
#define HICOUNT 3695    //sets up a low voltage calibration point


//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1300			//match DAC board product ID
#define ESC		 27



typedef struct {
	int value1, value2;			// keeps track of data for calibrations
	float volts1, volts2;		// keeps track of data for calibrations
	} _line;

_line ln[16];

const char vstr[][] = {
	"0\tConfig = 0  2.5v  10v\n",
	"1\tConfig = 1  5.0v  10v\n",
	"2\tConfig = 2  10v   10v\n",
	"3\tConfig = 3  5v    20v\n",
	"4\tConfig = 4  10v   20v\n",
	"5\tConfig = 5  20v   20v\n"

   };


// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

void printrange()
{
	auto int i;

	printf("\ngain_code\tVoltage range\n");
	printf("---------\t-------------\n");
   for (i=0; i<8; i++)
   {
		printf("\t%s", vstr[i]);
   }
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

///////////////////////////////////////////////////////////////////////////

void main()
{
	auto int device0, status;
   auto rn_search newdev;
   auto DacCal DacCalTable1;
   auto float volt1, volt2, voltout;
   auto int data1, data2;
   auto int channel, selectChannel, configureDAC;
   auto char tmpbuf[24];
   auto char command;
 	auto int key, done, cal_error;


	brdInit();
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   //search for device match
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }


	while(1)
	{
		DispStr(1, 1,"!!!Caution this will overwrite the calibration constants set at the factory.");
		DispStr(1, 2,"Do you want to continue(Y/N)?");

		while(!kbhit());
		key = getchar();
		if(key == 'Y' || key == 'y')
		{
			break;
		}
		else if(key == 'N' || key == 'n')
		{
			exit(0);
		}
	}

   configureDAC = TRUE;
  	selectChannel = TRUE;

 	for(;;)
	{

      if(configureDAC)
      {
      	blankScreen(0, 28);
   		DispStr(2, 1, "DAC Board   CH0&1   CH2-7");
			DispStr(2, 2, "-------------------------");

			DispStr(2, 3, "Config = 0  2.5v    10v");
			DispStr(2, 4, "Config = 1  5.0v    10v");
			DispStr(2, 5, "Config = 2  10v     10v");
			DispStr(2, 6, "Config = 3  5v      20v");
			DispStr(2, 7, "Config = 4  10v     20v");
			DispStr(2, 8, "Config = 5  20v     20v");
			DispStr(2, 9, "Please enter the DAC configuration 0 - 5....");
			do
			{
				command = getchar();
			} while (!((command >= '0') && (command <= '5')));
			printf("Config  = %d", command-=0x30);
      	rn_anaOutConfig(device0, command, 0, 0);
         configureDAC = FALSE;
      }

      if(selectChannel)
      {
      	blankScreen(11, 28);
 			DispStr(2, 11, "DAC0 - DAC7 Calibration Program");
			DispStr(2, 12, "-------------------------------");
			DispStr(2, 13, "Please enter an output channel (0 - 7) = ");
			do
			{
				channel = getchar();
			} while (!((channel >= '0') && (channel <= '7')));
			printf("%d", channel-=0x30);
         selectChannel = FALSE;
      }

      // set two known voltage points using rawdata values, the
		// user will then type in the actual voltage for each point
      rn_anaOut(device0, channel, LOCOUNT, 0);
		DispStr(2, 15, "ENTER the voltage reading from meter(~10% of max voltage) = ");
		volt1 = atof(gets(tmpbuf));

		rn_anaOut(device0, channel, HICOUNT, 0);
		DispStr(2, 16, "ENTER the voltage reading from meter(~90% of max voltage) = ");
		volt2 = atof(gets(tmpbuf));

      cal_error = FALSE;
     	rn_anaOutCalib(channel, LOCOUNT, volt1, HICOUNT, volt2, &DacCalTable1, 0);

      // Store coefficients into eeprom
      rn_anaOutWrCalib(device0, channel, &DacCalTable1, 0);
      memset(&DacCalTable1, 0x00, sizeof(&DacCalTable1));
      rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
  		DispStr(2, 17, "Calibration constants has been written to the eeprom");

 		done = FALSE;
		while (!done && !cal_error)
		{
			// display DAC voltage message
			DispStr(2, 20, "Type a desired voltage (in Volts) =  ");

			// get user voltage value for the DAC thats being monitored
			voltout = atof(gets(tmpbuf));

			// send voltage value to DAC for it to output the voltage
     		rn_anaOutVolts(device0, channel,  voltout, &DacCalTable1, 0);
			DispStr(2, 21, "Observe voltage on meter.....");

			// display user options
			DispStr(2, 23, "User Options:");
			DispStr(2, 24, "-------------");
			DispStr(2, 25, "1. Change the voltage on current channel");
			DispStr(2, 26, "2  Calibrate another DAC channel");
			DispStr(2, 27, "3. Change overall DAC output configuration");
         DispStr(2, 28, "4. Exit Program");


			// wait for a key to be pressed
			while(1)
			{
				while(!kbhit());
				key = getchar();
				if(key == '1')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               blankScreen(19, 28);
 					break;
				}
            if(key == '2')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               done = TRUE;
               selectChannel = TRUE;
               break;
   			}
            if(key == '3')
				{
   				// empty the keyboard buffer and clear user options
   				while(kbhit()) getchar();
               configureDAC = TRUE;
               selectChannel = TRUE;
               done = TRUE;
               break;
   			}
				if(key == '4')
				{
					// exit sample program
     				exit(0);
   			}
 		}
		}
	}
}
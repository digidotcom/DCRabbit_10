/***************************************************************************
	ad_calse_all.c
	Z-World Inc 2003

	This sample program is intended for RabbitNet RN1200 Analog-to-Digital
   Converter boards.

	Description
	===========
	This program demonstrates how to recalibrate all single-ended ADC channels
	for one gain, using two known voltages to generate constants for each
	channel and	rewritten RN1200 board flash.  After writing constants, a
   hardward reset will be issued to complete writes to flash and the hardware
   watchdog will be set.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1200 as the search criteria.

	Connections
	===========
	Connect the power supply positive output to AD channels AIN0-AIN7 and the
	negative output to GND on the controller.

	Connect a volt meter to monitor the voltage inputs.


	Instructions
	============
	1. Compile and run this program.
	2. Follow directions in STDIO window.

***************************************************************************/
#class auto					/* Change local var storage default to "auto" */

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200			//match ADC board product ID   

#define NUMSAMPLES 10

const float vmax[] = {
	20.0,
	10.0,
	5.0,
	4.0,
	2.5,
	2.0,
	1.25,
	1.00
};


typedef struct {
	int value1, value2;			// keeps track of data for calibrations
	float volts1, volts2;		// keeps track of data for calibrations
	} _line;

_line ln[16];

const char vstr[][16] = {
	"0\t0 - 20 \n",
	"1\t0 - 10\n",
	"2\t0 - 5\n",
	"3\t0 - 4\n",
	"4\t0 - 2.5\n",
	"5\t0 - 2\n",
	"6\t0 - 1.25\n",
	"7\t0 - 1\n\n"
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


void main ()
{
	auto int device0, status;
	auto char buffer[64];
   auto rn_search newdev;
   auto rn_AinData aindata;
   auto float voltage, cal_voltage;
	auto int rawdata;
   auto int gaincode;
   auto int data1, data2;
   auto int channel;
   auto int key;


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
	printf("\n");
	while(kbhit()) getchar();

   while (1)
	{
		// display the voltage that was read on the A/D channels
   	printrange();
		printf("\nChoose Voltage Configuration for the ADC Board 0 - 7.... ");
		do
		{
			gaincode = getchar();
		} while (!( (gaincode >= '0') && (gaincode <= '7')) );
		gaincode = gaincode - 0x30;
		printf("%d", gaincode);
		while(kbhit()) getchar();

	   // enable on all channels for conversions
		for(channel = 0; channel < 8; channel++)
		{
			status = rn_anaInConfig(device0, channel, RNSINGLE, gaincode, 0);
	   }

     	cal_voltage = .1*vmax[gaincode];
		printf("\nAdjust to approx. %.4f and then enter actual voltage = ", cal_voltage);
		gets(buffer);
		for (channel=0; channel < 8; channel++)
		{
			ln[channel].volts1 = atof(buffer);
			status = rn_anaIn(device0, channel, &data1, NUMSAMPLES, 0);
			ln[channel].value1 = data1;
			if (ln[channel].value1 == ADOVERFLOW)
				printf("lo:  channel=%d overflow\n", channel);
			else
				printf("lo:  channel=%d raw=%d\n", channel, ln[channel].value1);
		}

   	cal_voltage = .9*vmax[gaincode];
		printf("\nAdjust to approx. %.4f and then enter actual voltage = ", cal_voltage);
		gets(buffer);
		for (channel=0; channel < 8; channel++)
		{
			ln[channel].volts2 = atof(buffer);
			status = rn_anaIn(device0, channel, &data2, NUMSAMPLES, 0);
			ln[channel].value2 = data2;
			if (ln[channel].value2 == ADOVERFLOW)
				printf("hi:  channel=%d overflow\n", channel);
			else
				printf("hi:  channel=%d raw=%d\n", channel, ln[channel].value2);
		}

		printf("\nstore all constants to flash\n");
		for (channel=0; channel < 8; channel++)
		{
 			rn_anaInCalib(channel, RNSINGLE, gaincode, ln[channel].value1,
         	ln[channel].volts1,ln[channel].value2, ln[channel].volts2, &aindata);
	   	printf("gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);
			status = rn_anaInWrCalib(device0, channel, RNSINGLE, gaincode, aindata, 0);
		}

		printf("\nread back constants\n");
		for (channel=0; channel < 8; channel++)
		{
			status = rn_anaInRdCalib(device0, channel, RNSINGLE, gaincode, &aindata, 0);
   		printf("read back gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);
		}

      //After writing constants to flash, you must hard reset to close
      // off flash writes.  Wait 1 second to make sure device reinitializes
      // and clear the reset register.
      rn_reset(device0, 0);
      rn_msDelay(1000);

      //Check and clear reset status
      if(!((status = rn_rst_status(device0, buffer)) & 0x01))
     	{
      	printf("Error! ADC board didn't reset");
         exit(1);
      }
     	status =  rn_enable_wdt(device0, 1);  //enable device hardware watchdog

	   // Must enable on all channels for conversions again after reset
		for(channel = 0; channel < 8; channel++)
		{
			status = rn_anaInConfig(device0, channel, RNSINGLE, gaincode, 0);
	   }
  		printf("\nVary power supply for the voltage range selected.... \n\n");
		do
		{
      	for(channel = 0; channel < 8; channel++)
         {
         	status = rn_anaInVolts(device0, channel, &voltage, NUMSAMPLES, 0);
				printf("Ch %2d Volt=%.5f \n", channel, voltage);
         }
			printf("Press ENTER key to read value again or 'Q' to select another gain option\n\n");
			while(!kbhit());
			key = getchar();
			while(kbhit()) getchar();

		}while(key != 'q' && key != 'Q');
	}
}
///////////////////////////////////////////////////////////////////////////
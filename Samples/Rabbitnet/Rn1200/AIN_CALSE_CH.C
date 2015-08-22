/***************************************************************************
	ain_calse_ch.c
	Z-World Inc 2003

	This sample program is intended for RabbitNet RN1200 Analog-to-Digital
   Converter boards.

	Description
	===========
	This program demonstrates how to recalibrate one single-ended ADC channel
	using two known voltages to generate constants for that channel and
	rewritten RN1200 board flash.  After writing constants, a hardward reset
   will be issued to complete writes to flash and the hardware watchdog will
   be set.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1200 as the search criteria.

	Connections
	===========
	Connect the power supply positive output to the AD channels and the
	negative output to GND on the controller.

	Connect a volt meter to monitor the voltage input.


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


void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}


void printrange( void )
{
	printf("\n gain_code\tVoltage range\n");
	printf("-----------\t-------------\n");
	printf("\t0\t0 - 20 \n");
	printf("\t1\t0 - 10\n");
	printf("\t2\t0 - 5\n");
	printf("\t3\t0 - 4\n");
	printf("\t4\t0 - 2.5\n");
	printf("\t5\t0 - 2\n");
	printf("\t6\t0 - 1.25\n");
	printf("\t7\t0 - 1\n\n");
}

void main ()
{
	auto int device0, status;
 	auto char buffer[64];
	auto rn_search newdev;
	auto rn_AinData aindata;
   auto float volts1, volts2, cal_voltage, voltage;
   auto int value1, value2;
   auto int channel;
   auto int rawdata;
   auto int gaincode;
   auto int key;

	brdInit();                 //initialize controller
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
   	printrange();
		printf("\nChoose gain code .... ");
		do
		{
			gaincode = getchar();
		} while (!( (gaincode >= '0') && (gaincode <= '7')) );
		gaincode = gaincode - 0x30;
		printf("%d", gaincode);
		while(kbhit()) getchar();

      printf("\n\nPlease enter an ADC channel, 0 thru 7....");
		do
		{
			channel = getchar();
		} while (!( (channel >= '0') && (channel <= '7')) );
		channel = channel - 0x30;
		printf("%d", channel);
		while(kbhit()) getchar();


   	// enable channel for conversions
		status = rn_anaInConfig(device0, channel, RNSINGLE, gaincode, 0);

 		cal_voltage = .1*vmax[gaincode];
		printf("\nAdjust to approx. %.4f and then enter actual voltage = ", cal_voltage);
		gets(buffer);
		volts1 = atof(buffer);
		status = rn_anaIn(device0, channel, &value1, NUMSAMPLES, 0);
		if (value1 == ADOVERFLOW)
			printf("lo:  channel=%d overflow\n", channel);
		else
			printf("lo:  channel=%d raw=%d\n", channel, value1);

		cal_voltage = .9*vmax[gaincode];
		printf("\nAdjust to approx. %.4f and then enter actual voltage = ", cal_voltage );
		gets(buffer);
		volts2 = atof(buffer);
		status = rn_anaIn(device0, channel, &value2, NUMSAMPLES, 0);
		if (value2 == ADOVERFLOW)
			printf("hi:  channel=%d overflow\n", channel);
		else
			printf("hi:  channel=%d raw=%d\n", channel, value2);

		rn_anaInCalib(channel, RNSINGLE, gaincode, value1, volts1, value2, volts2, &aindata);
   	printf("gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);

		printf("\nstore constants to flash\n");
		status = rn_anaInWrCalib(device0, channel, RNSINGLE, gaincode, aindata, 0);

		printf("\nread back constants\n");
		status = rn_anaInRdCalib(device0, channel, RNSINGLE, gaincode, &aindata, 0);
   	printf("read back gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);

      //After writing constants to flash, you must hard reset to close
      // off flash writes.  Wait 2 seconds to make sure device reinitializes
      // and clear the reset register.
      rn_reset(device0, 0);
      rn_msDelay(2000);

      //clear reset status
     	if(!((status = rn_rst_status(device0, buffer)) & 0x01))
      {
      	printf("Error! ADC board didn't reset.");
         exit(1);
      }
		status =  rn_enable_wdt(device0, 1);  //enable device hardware watchdog

   	// Must enable channel for conversions again after reset
		status = rn_anaInConfig(device0, channel, RNSINGLE, gaincode, 0);

  		printf("\nVary power supply for the voltage range selected.... \n\n");
		do
		{
   		status = rn_anaInVolts(device0, channel, &voltage, NUMSAMPLES, 0);
			printf("Ch %2d Volt=%.5f \n", channel, voltage);

			printf("Press ENTER key to read value again or 'Q' to select another channel\n\n");
			while(!kbhit());
			key = getchar();
			while(kbhit()) getchar();

		}while(key != 'q' && key != 'Q');
	}
}
///////////////////////////////////////////////////////////////////////////


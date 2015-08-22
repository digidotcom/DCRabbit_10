/***************************************************************************
	ain_caldiff_ch.c
	Z-World Inc 2003

	This sample program is intended for RabbitNet RN1200 ADC boards

	Description
	===========
	This program demonstrates how to recalibrate one differential ADC channel
	using two known voltages to generate constants for that channel and
	rewritten into RN1200 board flash.  It will also continuously
	display the voltages being monitored.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1200 as the search criteria.

  	Connections
	===========
	For this calibration procedure you will need a power supply
	that has a floating output.

	NOTE:	Before doing the following steps, set the power supply
	      to zero volts and then turn it OFF.


	1. Initially connect the positive side of the power supply to
	   the positive side to one of the following ADC differential
	   channel pairs.

	    Channel   DIFF Pairs
	    -------  ------------
	      0 		 +AIN0  -AIN1
	      2		 +AIN2  -AIN3
	      4		 +AIN4  -AIN5
	      6		 +AIN6  -AIN7

	2.	Connect the negative side of the power supply to the
	   negative side to one of the following ADC differential
	   channel pairs. (Same DIFF pair from step 1)

	    Channel    DIFF Pairs
	    -------   ------------
	      0		  +AIN0   -AIN1
	      2		  +AIN2   -AIN3
	      4		  +AIN4   -AIN5
	      6 		  +AIN6   -AIN7


	Instructions
	============
	1. Power-on the controller.
	2. Compile and run this program.
	3. Turn ON the power supply for the ADC calibration.
	4. Follow the instructions as displayed.


***************************************************************************/
#class auto					/* Change local var storage default to "auto" */

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200			//match ADC board product ID   

const char vstr[][] = {
	" +- 20V  ",
	" +- 10V  ",
	" +- 5V   ",
	" +- 4V   ",
	" +- 2.5V ",
	" +- 2V   ",
	" +- 1.25V",
	" +- 1V   "
};



void printrange()
{
	printf("\ngain_code\tVoltage range\n");
	printf("---------\t-------------\n");
	printf("\t0\t +- 20 \n");
	printf("\t1\t +- 10\n");
	printf("\t2\t +- 5\n");
	printf("\t3\t +- 4\n");
	printf("\t4\t +- 2.5\n");
	printf("\t5\t +- 2\n");
	printf("\t6\t +- 1.25\n");
	printf("\t7\t +- 1\n\n");
}

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

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

void main ()
{
	auto int device0, status;
 	auto char buffer[64];
 	auto rn_search newdev;
   auto rn_AinData aindata;
   auto float voltage, cal_voltage, volts1, volts2;
   auto int value1, value2;
   auto int rawdata;
   auto int gaincode;
   auto int channel;
   auto int valid;
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

		printf("\nchannel_code\tInputs\n");
		printf("------------\t-------------\n");
		printf("\t0\t+AIN0 -AIN1\n");
		printf("\t2\t+AIN2 -AIN3\n");
		printf("\t4\t+AIN4 -AIN5\n");
		printf("\t6\t+AIN6 -AIN7\n\n");
		printf("\nChoose the AD channel 0,2,4, or 6 = ");
		do
		{
			channel = getchar();
			switch(channel)
			{
				case '0':
				 valid = TRUE;
				 break;
				case '2':
				 valid = TRUE;
				 break;
				case '4':
				 valid = TRUE;
				 break;
				case '6':
				 valid = TRUE;
				 break;
			}
		} while (!valid);
		channel = channel - 0x30;
		printf("%d", channel);
		while(kbhit()) getchar();

 		printf("\n");
		printrange();
		printf("Choose gain code (0-7) =  ");
		do
		{
			gaincode = getchar();
		} while (!( (gaincode >= '0') && (gaincode <= '7')) );
		gaincode = gaincode - 0x30;
		printf("%d\n\n", gaincode);
		while(kbhit()) getchar();
      status = rn_anaInConfig(device0, channel, RNDIFF, gaincode, 0);

		cal_voltage = vmax[gaincode]*.9;
		printf("\nAdjust Power connected to AIN%d to approx. %.2f\n", channel, cal_voltage);
		printf("and then enter actual voltage = ");
		gets(buffer);

		volts1 = atof(buffer);
 		status = rn_anaIn(device0, channel, &value1, 10, 0);
		if (value2 == -1)
			printf("channel=%d overflow\n", channel);
		else
			printf("channel=%d raw=%d difference=%.3fV\n", channel, value1, volts1);

		printf("\nSwap power supply connections and then PRESS any key\n");
		while(!kbhit());
		while(kbhit()) getchar();

		volts2 = -volts1;
 		status = rn_anaIn(device0, channel, &value2, 10, 0);
		if (value2 == -1)
			printf("channel=%d overflow\n", channel);
		else
			printf("channel=%d raw=%d difference=%.3fV\n", channel, value2, volts2);

  		rn_anaInCalib(channel, RNDIFF, gaincode, value1, volts1, value2, volts2, &aindata);
   	printf("gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);

		printf("\nstore constants to flash\n");
		status = rn_anaInWrCalib(device0, channel, RNDIFF, gaincode, aindata, 0);

		printf("\nread back constants\n");
		status = rn_anaInRdCalib(device0, channel, RNDIFF, gaincode, &aindata, 0);
   	printf("read back gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);

		printf("\nVary power supply for the voltage range selected.... \n\n");
		do
		{
			status = rn_anaInDiff(device0, channel, &voltage, 1, 0);
			printf("Ch %2d Volt=%.5f \n", channel, voltage);

			printf("Press ENTER key to read value again or 'Q' to calibrate another\n\n");
			while(!kbhit());
			key = getchar();
			while(kbhit()) getchar();

		}while(key != 'q' && key != 'Q');

	}
}

///////////////////////////////////////////////////////////////////////////


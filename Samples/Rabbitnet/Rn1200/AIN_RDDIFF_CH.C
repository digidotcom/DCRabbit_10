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
	ain_rddiff_ch.c

	This sample program is intended for RabbitNet RN1200 ADC boards

	Description
	===========
   Demonstrates reading a differential A/D converter channel using
   two known voltages and constants for that channel. The voltage
   being monitored will be displayed continuously in the STDIO
   window.

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
   3. Follow the instructions as displayed.

***************************************************************************/
#class auto					// local var storage default to auto

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
 	auto rn_search newdev;
   auto rn_AinData aindata;
   auto float voltage, volts1, volts2, v1, v2;
   auto int channel;
   auto int gaincode;
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

 		printf("\nVary power supply for the voltage range selected.... \n\n");
		do
		{
			status = rn_anaInDiff(device0, channel, &voltage, 1, 0);
	      if(voltage != ADOVERFLOW)
         	printf("Ch %2d Volt=%.5f \n", channel, voltage);
         else
         	printf("Ch %2d Volt=Exceeded Voltage Range  \n", channel);
			printf("Press ENTER key to read value again or 'Q' to select another channel\n\n");
			while(!kbhit());
			key = getchar();
			while(kbhit()) getchar();

		}while(key != 'q' && key != 'Q');
	}
}
///////////////////////////////////////////////////////////////////////////


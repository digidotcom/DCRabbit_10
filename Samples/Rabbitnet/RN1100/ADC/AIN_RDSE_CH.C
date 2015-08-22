/***************************************************************************
	ain_rdse_ch.c
   Z-World Inc 2003

	This sample program is intended for RabbitNet RN1100 Digital I/O boards

	Description
	===========
	This program reads and displays voltage and equivalent values of one
	single-ended analog input channel.  Coefficients are read from the
   RabbitNet board.  Computed raw data and equivalent voltages will be
   displayed.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1100 as the search criteria.

	Instructions
	============
	1. Connect a power supply of 0-10 volts to AIN00 or AIN01.
   	Connect a power supply of 0-1 volts to AIN02.
 	2. Set PRINTSTATS below to display status byte descriptions.
   3. Compile and run this program.
	4. Follow the prompted directions of this program during execution.
	5. Values will continuously display.

***************************************************************************/
#class auto					/* Change local var storage default to "auto" */


//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board

//////
// Define as 1 or 0 to display or not display status bit descriptions
//////
#define PRINTSTATS 1

#define STARTCHAN	0		//AIN00
#define ENDCHAN 2       //AIN02

const char statstr[8][128] = {
	"  bit 0 Reset occured, check control register\0",
	"  bit 1 Command rejected, try again\0",
	"  bit 2 Reserved\0",
	"  bit 3 ADC updated, check ADC control register",
	"  bit 4 Communication error, check comm status register\0",
	"  bit 5 Reserved\0",
	"  bit 6 Device Ready\0",
	"  bit 7 Device Busy, try again\0"
   };

void printstat(char statusbyte)
{
	auto int i;

   printf("Status 0x%02x description:\n", statusbyte);
	for (i=0; i<8; i++)
   {
   	if ((statusbyte>>i)&1)
	      printf("%s\n", statstr[i]);
   }
   printf("\n");
}

//////
/// Use with choice 3 to convert rawdata to equivalent voltage
//////
float convert2volt(int channel, int rawdata, float gain, int offset)
{
   if (rawdata == ADOVERFLOW)
   	return ADOVERFLOW;

	return ((rawdata - offset)*gain);
}


void main()
{
	auto int rawdata, device0, status;
	auto int channel, keypress;
	auto float voltequ;
	auto char buffer[64];
   auto rn_AinData aindata;
	auto rn_search newdev;

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

   // enable on all channels for conversions
	for(channel = STARTCHAN; channel <= ENDCHAN; channel++)
	{
		status = rn_anaInConfig(device0, channel, ADCENABLE, 0, 0);
   }

	while (1)
	{
		printf("\nChoose the AIN channel %d to %d .... ", STARTCHAN, ENDCHAN);
		gets(buffer);
		channel = atoi(buffer);

		printf("\nChoose:\n");
		printf("  1 to display raw data only\n");
		printf("  2 to display voltage only\n");
		printf("  3 to display both\n  .... ");
		gets(buffer);
		keypress = atoi(buffer);

		while (strcmp(buffer,"q") && strcmp(buffer,"Q"))
		{
			switch (keypress)
			{
				case 1:
            	//sample 10 times
					status = rn_anaIn(device0, channel, &rawdata, 10, 0);
					printf("CH%2d raw data %d\n", channel, rawdata);
               if (PRINTSTATS) printstat(status);
					break;
				case 2:
            	//sample 10 times before returning with voltage equivalent
					status = rn_anaInVolts(device0, channel, &voltequ, 10, 0);
					printf("CH%2d is %.5f V\n", channel, voltequ);
               if (PRINTSTATS) printstat(status);
					break;
				case 3:
            	//get calibrations
					status = rn_anaInRdCalib(device0, channel, RNSINGLE, 0, &aindata, 0);
               //sample 10 times before computing voltage equivalent
   				status = rn_anaIn(device0, channel, &rawdata, 10, 0);
					voltequ = convert2volt(channel, rawdata, aindata.gain, aindata.offset);
					printf("CH%2d is %.5f V from raw data %d\n", channel, voltequ, rawdata);
               if (PRINTSTATS) printstat(status);
					break;
			}
			printf("\npress enter key to read value again or 'Q' or read another channel\n\n");
			gets(buffer);
		}
	}
}	//end main


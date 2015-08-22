/***************************************************************************
	ain_rdse_ch.c
   Z-World Inc 2003

	This sample program is intended for RabbitNet RN1200 ADC boards

	Description
	===========
   Reads and displays the voltage and equivalent values of one single-ended
   analog input channel. Coefficients are read from the A/D Converter Board.
   The computed raw data and equivalent voltages will be displayed.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1200 as the search criteria.
   Device hardware watchdog is enabled.

	Instructions
	============
	1. Connect a power supply of 0-10 volts.
	2. Compile and run this program.
	3. Follow the prompted directions of this program during execution.
	4. Values will continuously display.

***************************************************************************/
#class auto					/* Change local var storage default to "auto" */

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200         //match ADC board product ID   

#define NUMSAMPLES 10


//////
/// Use with choice 3 to convert rawdata to equivalent voltage
//////
float convert2volt(int channel, int rawdata, float gain, int offset)
{
	auto float f;

	f = (rawdata - offset)*gain;
	if (f <= 0.00)
		f = 0.000;

   return (f);
}


const float vmin[8] = {1.0, 1.0, 1.0, 1.0, 0.5, 0.5, 0.1, 0.1};
const float vmax[8] = {19.0, 9.0, 4.0, 3.0, 2.0, 1.5, 1.2, 0.9};

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


void main()
{
	auto int device0, status;
	auto rn_search newdev;
   auto rn_AinData aindata;
   auto float voltequ;
   auto int rawdata;
   auto int channel;
   auto int gaincode;
   auto int key, keypress;

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

	status =  rn_enable_wdt(device0, 1);  //enable device hardware watchdog

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

   	status = rn_anaInConfig(device0, channel, RNSINGLE, gaincode, 0);

      printf("\nChoose:\n");
		printf("  1 to display raw data only\n");
		printf("  2 to display voltage only\n");
		printf("  3 to display both\n  .... ");
		do
		{
			keypress = getchar();
		} while (!( (keypress >= '0') && (keypress <= '3')) );
		keypress = keypress - 0x30;
		printf("%d\n\n", keypress);
		while(kbhit()) getchar();
		do
		{
			switch (keypress)
			{
				case 1:
            	//sample 10 times
					status = rn_anaIn(device0, channel, &rawdata, NUMSAMPLES, 0);
					printf("Status 0x%02x CH%2d raw data %d\n", status, channel, rawdata);
					break;
				case 2:
            	//sample 10 times before returning with voltage equivalent
					status = rn_anaInVolts(device0, channel, &voltequ, NUMSAMPLES, 0);
					printf("Status 0x%02x CH%2d is %.5f V\n", status, channel, voltequ);
					break;
				case 3:
            	//get calibrations
					status = rn_anaInRdCalib(device0, channel, RNSINGLE, gaincode, &aindata, 0);
               //sample 10 times before computing voltage equivalent
   				status = rn_anaIn(device0, channel, &rawdata, NUMSAMPLES, 0);
					voltequ = convert2volt(channel, rawdata, aindata.gain, aindata.offset);
					printf("Status 0x%02x CH%2d is %.5f V from raw data %d\n",
                       status, channel, voltequ, rawdata);
					break;
				default:
				break;
			}
			printf("\npress enter key to read value again or 'Q' or read another channel\n\n");
         while(!kbhit());
			key = getchar();
			while(kbhit()) getchar();
		}while(key != 'q' && key != 'Q');
	}
}	//end main


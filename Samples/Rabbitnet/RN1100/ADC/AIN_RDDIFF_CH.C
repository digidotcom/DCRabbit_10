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

	This sample program is intended for RabbitNet RN1100 Digital I/O boards

	Description
	===========
	This program demonstrates reading one differential ADC channel
	using two known voltages and constants for that channel.
	It will also continuously display the voltage being monitored.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1100 as the search criteria.

	Connections
	===========

   Power supply+ <-----(1.2K ohm)---+-----> AIN03+  <----- voltmeter+
 	(0 to +10V)								|
                                    |
                                (51 ohm)
                                		|
                                    |
   Power supply- <-----(1.2K ohm)---+-----> AIN03-  <----- voltmeter-


   1. Using 1.2K and 51 ohm resistors and a 0 to 10V power supply,
   	make the above diagram connections to analog inputs AIN03+
      and AIN03-.
   2. Connect a voltmeter as indicated in above diagram.


	Instructions
	============
	1. Adjust power supply to approximately +0.20 volts on the voltmeter.
	2. Compile and run this program.
	3. Vary the voltage from 0 to +.25V and observe voltage on voltmeter
   	and in STDIO window.
	4. Switch power supply+ connection to AIN03- and connect power
   	supply- to AIN03+.
	5. Vary the voltage from 0 to -.25V and observe voltage on voltmeter
   	and in STDIO window.
 	6. Set PRINTSTATS below to display status byte descriptions.


***************************************************************************/
#class auto					// local var storage default to auto

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1100			//RN1100 DI/0 board

//////
// Define as 1 or 0 to display or not display status bit descriptions
//////
#define PRINTSTATS 1


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

const char vstr[][16] = {
	"0 - 10V\0",
	"0 - 10V\0",
	"0 -  1V\0",
	"+/- 0.25V\0"};

const float vmin[4] = {1.0, 1.0, 0.1, -0.20};
const float vmax[4] = {9.0, 9.0, 0.9, 0.20};


//////
/// Use with choice 3 to convert rawdata to equivalent voltage
//////
float convert2volt(int channel, int rawdata, float gain, int offset)
{
   if (rawdata == ADOVERFLOW)
   	return ADOVERFLOW;

	return (((rawdata - offset)*gain));
}


void main ()
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

   // enable channel for conversions
	channel = 3;
	status = rn_anaInConfig(device0, channel, ADCENABLE, 0, 0);

	printf("\nReading AIN channel 3 .... ");
	printf("\nVary voltage from -0.25 to +0.25  .... \n\n");

	while (1)
	{
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
					status = rn_anaInDiff(device0, channel, &voltequ, 10, 0);
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
}
///////////////////////////////////////////////////////////////////////////


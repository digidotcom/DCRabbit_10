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
	ain_calse_ch.c

	This sample program is intended for RabbitNet RN1100 Digital I/O boards

	Description
	===========
	This program demonstrates how to recalibrate one single-ended ADC channel
	using two known voltages to generate constants for that channel and
	rewritten RN1100 board flash.  It will also continuously display the voltages
   being monitored.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1100 as the search criteria.

	Connections
	===========
	Connect the power supply positive output to the AD channels and the
	negative output to GND on the controller.

	Connect a volt meter to monitor the voltage input.


	Instructions
	============
	1. Compile and run this program.
	2. Set PRINTSTATS below	to display status byte descriptions.
	3. Follow directions in STDIO window.

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


nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


void main ()
{
	auto int channel, value1, value2, device0, status;
	auto float voltage, volts1, volts2;
	auto unsigned int rawdata, gaincode;
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
	for(channel = 0; channel <= 2; channel++)
	{
		status = rn_anaInConfig(device0, channel, ADCENABLE, 0, 0);
   }

	while (1)
	{
		printf("\nChoose the AIN channel 0 - 2 .... ");
		gets(buffer);
		channel = atoi(buffer);

		printf("\nAdjust voltage to %5.2f and enter actual =  ", vmin[channel]);
		gets(buffer);
		volts1 = atof(buffer);
		status = rn_anaIn(device0, channel, &value1, 10, 0);
		printf("lo:  channel=%d raw=%d\n", channel, value1);

		printf("\nAdjust voltage to %5.2f and enter actual =  ", vmax[channel]);
		gets(buffer);
		volts2 = atof(buffer);
		status = rn_anaIn(device0, channel, &value2, 10, 0);
		printf("hi:  channel=%d raw=%d\n", channel, value2);

		rn_anaInCalib(channel, RNSINGLE, 0, value1, volts1, value2, volts2, &aindata);
   	printf("gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);

		printf("\nstore constants to flash\n");
		status = rn_anaInWrCalib(device0, channel, RNSINGLE, 0, aindata, 0);

		printf("\nread back constants\n");
		status = rn_anaInRdCalib(device0, channel, RNSINGLE, 0, &aindata, 0);
   	printf("read back gain=%8.5f offset=%d\n", aindata.gain, aindata.offset);

      //After writing constants to flash, you must hard reset to close
      // off flash writes.  Wait 2 seconds to make sure device reinitializes
      // and clear the reset register.
      rn_reset(device0, 0);
      msDelay(2000);
		status = rn_rst_status(device0, buffer);  //clear reset
		printf("\nVary voltage %s .... \n", vstr[channel]);

		while (strcmp(buffer,"q") && strcmp(buffer,"Q"))
		{
			status = rn_anaInVolts(device0, channel, &voltage, 1, 0);
			printf("Ch %2d Volt=%8.5f \n", channel, voltage);
         if (PRINTSTATS) printstat(status);
			printf("press enter key to read values again or 'Q' to calibrate another channel\n\n");
			gets(buffer);
		}
	}
}
///////////////////////////////////////////////////////////////////////////


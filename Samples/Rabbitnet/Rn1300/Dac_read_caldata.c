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
   dac_read_caldata.c

   This sample program is intended for RabbitNet RN1300 DAC boards.

	Description
	===========
	This program dumps the calibration data for all the DAC channels
   and the modes of operation. The program will display the calibration
   gain factor and offset value via the STDIO window for each channel
   and mode of operation.

	Instructions
	============
	1. Compile and run this program.
	2. View STDIO window for calibration data values.

***************************************************************************/
#class auto
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


main ()
{
  	auto int device0, status;
	auto rn_search newdev;
   auto DacCal DacCalTable1;
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

   printf("\n\nDAC Calibration Data for CH0-1 2.5V,  CH2-7 10V\n");
   rn_anaOutConfig(device0, 0, 0, 0);
  	for (channel=0; channel < 8; channel++)
	{
     	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
   	printf("Ch=%d  gain=%8.5f offset=%d\n",
             channel, DacCalTable1.cal[channel].gain, DacCalTable1.cal[channel].offset);
	}

  	printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nDAC Calibration Data for CH0-1 5V,  CH2-7 10V\n");
   rn_anaOutConfig(device0, 1, 0, 0);
  	for (channel=0; channel < 8; channel++)
	{
     	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
   	printf("Ch=%d  gain=%8.5f offset=%d\n",
             channel, DacCalTable1.cal[channel].gain, DacCalTable1.cal[channel].offset);
	}

  	printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nDAC Calibration Data for CH0-1 10V,  CH2-7 10V\n");
   rn_anaOutConfig(device0, 2, 0, 0);
  	for (channel=0; channel < 8; channel++)
	{
     	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
   	printf("Ch=%d  gain=%8.5f offset=%d\n",
             channel, DacCalTable1.cal[channel].gain, DacCalTable1.cal[channel].offset);
	}

  	printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nDAC Calibration Data for CH0-1 5V,  CH2-7 20V\n");
   rn_anaOutConfig(device0, 3, 0, 0);
  	for (channel=0; channel < 8; channel++)
	{
     	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
   	printf("Ch=%d  gain=%8.5f offset=%d\n",
             channel, DacCalTable1.cal[channel].gain, DacCalTable1.cal[channel].offset);
	}

  	printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nDAC Calibration Data for CH0-1 10V,  CH2-7 20V\n");
   rn_anaOutConfig(device0, 4, 0, 0);
  	for (channel=0; channel < 8; channel++)
	{
     	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
   	printf("Ch=%d  gain=%8.5f offset=%d\n",
             channel, DacCalTable1.cal[channel].gain, DacCalTable1.cal[channel].offset);
	}

  	printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nDAC Calibration Data for CH0-1 20V,  CH2-7 20V\n");
   rn_anaOutConfig(device0, 5, 0, 0);
  	for (channel=0; channel < 8; channel++)
	{
     	rn_anaOutRdCalib(device0, channel, &DacCalTable1, 0);
   	printf("Ch=%d  gain=%8.5f offset=%d\n",
             channel, DacCalTable1.cal[channel].gain, DacCalTable1.cal[channel].offset);
	}

  	printf("\n\nPress any key to exit...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();
}
///////////////////////////////////////////////////////////////////////////


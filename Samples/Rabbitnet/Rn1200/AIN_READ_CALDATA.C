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
 	ain_read_caldata.c

	This sample program is intended for RabbitNet RN1200 ADC boards.

	Description
	===========
	This program dumps the calibration data for all the ADC channels
   and the modes of operation. The program will display the calibration
   gain factor and offset value via the STDIO window for each channel
   and mode of operation.

	Instructions
	============
	1. Compile and run this program.
	2. View STDIO window for calibration data values.

***************************************************************************/

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200			//match ADC board product ID   


nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}


main ()
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

   printf("\n\nCalibration Data for Single-Ended mode, Ch = 0-7 Gaincode = 0-7.\n");
   for(gaincode = 0; gaincode < 8; gaincode++)
   {
   	for (channel=0; channel < 8; channel++)
		{
			status = rn_anaInRdCalib(device0, channel, RNSINGLE, gaincode, &aindata, 0);
   		printf("Ch=%d Gaincode=%d,  gain=%8.5f offset=%d\n",
                channel, gaincode,
                aindata.gain, aindata.offset);
		}
      printf("\n");
   }

   printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nCalibration Data for Diff mode, Ch = 0,2,4 & 6 Gaincode = 0-7.\n");
	for(gaincode = 0 ; gaincode < 8; gaincode++)
	{
      for (channel=0; channel < 8; channel+=2)
		{
			status = rn_anaInRdCalib(device0, channel, RNDIFF, gaincode, &aindata, 0);
         printf("Ch=%d Gaincode=%d,  gain=%8.5f offset=%d\n",
                channel, gaincode,
                aindata.gain, aindata.offset);
		}
      printf("\n");
	}

   printf("\n\nPress any key to continue...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

   printf("\n\nCalibration Data for 4-20ma mode, Ch = 0-3 Gaincode = 4\n");
   for (channel=0; channel < 4; channel++)
	{
		status = rn_anaInRdCalib(device0, channel, RNmAMP, RNmAMP_GAINCODE, &aindata, 0);
      printf("Ch=%d Gaincode=%d,  gain=%8.5f offset=%d\n",
                channel, RNmAMP_GAINCODE,
                aindata.gain, aindata.offset);
	}

   printf("\n\nPress any key to exit program...\n\n");
   while(!kbhit());
   while(kbhit()) getchar();

}
///////////////////////////////////////////////////////////////////////////


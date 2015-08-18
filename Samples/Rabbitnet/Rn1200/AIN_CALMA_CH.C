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
   ain_calma_ch.c

   This sample program is intended for RabbitNet RN1200 Analog-to-Digital
   Converter boards.

   Description
   ===========
   This program demonstrates how to recalibrate an ADC milli-amp channel
   using two known currents to generate two coefficients, gain and offset,
   which will be rewritten into flash memeory. It will also continuously
   displays the current that is being monitored.

   This program will first look for a device and use the first one found
   using rn_find() and the product RN1200 as the search criteria.

   Instructions
   ============
   1. To configure the A/D channels 0 - 3 for 4-20ma mode of operation
      jumper the following:

      JP1   jumper pins 1-2, 3-4, 5-6, and 7-8.

   2. Connect the power supply as shown below to one of the AIN channels 0 - 3
      on the controller. With the 500 ohm series resistor installed as shown
      below the power supply output will need to vary from 0 to 10 volts to
      obtain the 4 - 20 milli-amps of current.

   Caution!!! If you don't have the 500 ohm series resistor as shown
              below, then the power supply voltage must not exceed
              2 volts, as the internal sensing resistor is 100 ohms,
              1/16 watts.


   -----------------|                            |---------------------------
                    |                            | 4-20ma board
   Power supply     |  500ohm    |-------|       |
                POS |--/\/\/\----|current|-------| AIN channels 0 - 3
                    |            | meter |       |[w/100 ohm sensing resistor]
                    |            ---------       |
                    |                            |
                    |                            |
                    |                            |
                    |                            |
                    |                            |
                NEG |----------------------------|
                    |                            |
   -----------------|                            |---------------------------

   3. Compile and run this program.
   4. Follow the prompted directions of this program during execution.
   5. Vary voltage on power supply to see the CURRENT meter track
   what is displayed by Dynamic C (4-20ma).

   Note: For best results use a 4 1/2 digit current meter
***************************************************************************/
#class auto

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200			//match ADC board product ID   
#define NUMSAMPLES 10

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}


void main ()
{
	auto int device0, status;
	auto char buffer[64];
  	auto rn_search newdev;
   auto rn_AinData aindata;
   auto float locurrent, hicurrent, current;
   auto int data1, data2;
   auto int rawdata;
   auto int channel;
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
      printf("\nChoose the ADC 4-20ma channel (0-3) = ");
      do
      {
         channel = getchar();
      } while (!( (channel >= '0') && (channel <= '3')) );
      channel = channel - 0x30;
      printf("%d", channel);
      while(kbhit()) getchar();


       // Enable channel for conversions
		status = rn_anaInConfig(device0, channel, RNmAMP, RNmAMP_GAINCODE, 0);

      /////Get two data points using known currents
      printf("\n\nAdjust the current to 5.0ma and then ENTER the actual\n");
      printf("current being measured, (floating point value) = ");
      gets(buffer);
      while(kbhit()) getchar();
      locurrent = atof(buffer);
      if(!(locurrent >= 4.00 && locurrent <= 20.0))
      {
         printf("Current value must be within 4.0 - 20.0 amps\n\n");
         exit(1);
      }

    	status = rn_anaIn(device0, channel, &data1, NUMSAMPLES, 0);
   	if (status&(RNCMDREJ|RNWDTO))	//cmd reject or wdtimeout
      {
       	printf("ADC access Error!");
         exit(1);
      }
      printf("data1 = %d\n", data1);

      printf("\n\nAdjust the current to ~19.0ma and ENTER the actual\n");
      printf("current being measured, (floating point value) = ");
      gets(buffer);
      while(kbhit()) getchar();
      hicurrent = atof(buffer);
      if(!(hicurrent >= 4.00 && hicurrent <= 20.0))
      {
         printf("Current value must be within 4.0 - 20.0 amps\n\n");
         exit(1);
      }

      status = rn_anaIn(device0, channel, &data2, NUMSAMPLES, 0);
   	if (status&(RNCMDREJ|RNWDTO))	//cmd reject or wdtimeout
      {
       	printf("ADC access Error!");
         exit(1);
      }
      printf("data2 = %d\n", data2);

      rn_anaInCalib(channel, RNmAMP, RNmAMP_GAINCODE,
                    data1, locurrent,
                    data2, hicurrent,
                    &aindata);

    	printf("\nStore constants to flash\n");
		status = rn_anaInWrCalib(device0, channel, RNmAMP, RNmAMP_GAINCODE, aindata, 0);

      memset(&aindata, 0x00, sizeof(aindata));
      printf("\nRead back constants\n");
		status = rn_anaInRdCalib(device0, channel, RNmAMP, RNmAMP_GAINCODE, &aindata, 0);

      printf("Vary current on channel %d\n", channel);
      do
      {
         rn_anaInmAmps(device0, channel, &current, 1,  0);
         printf("Current at CH%d is %.2fma\n", channel, current);
         printf("\nPress ENTER key to read value again or 'Q' to calibrate another channel\n\n");
         while(!kbhit());
         key = getchar();
         while(kbhit()) getchar();

      }  while(key != 'q' && key != 'Q');
   }

}  //end main
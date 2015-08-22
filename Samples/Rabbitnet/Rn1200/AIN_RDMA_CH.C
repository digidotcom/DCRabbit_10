/***************************************************************************
   ain_calma_ch.c
   Z-World Inc 2003

   This sample program is intended for RabbitNet RN1200 Analog-to-Digital
   Converter boards.

   Description
   ===========
   Demonstrates reading a 4–20 mA A/D converter channel. The current being
   monitored will be displayed continuously in the STDIO window.

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

#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200			//match ADC board product ID   


void main ()
{
	auto int device0, status;
 	auto rn_search newdev;
   auto int channel;
	auto float current;
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
		printf("\n\nPlease enter an ADC channel (0-3) = ");
		do
		{
			channel = getchar();
		} while (!( (channel >= '0') && (channel <= '3')) );
		channel = channel - 0x30;
		printf("%d", channel);
		while(kbhit()) getchar();

      // Enable channel for conversions
		status = rn_anaInConfig(device0, channel, RNmAMP, RNmAMP_GAINCODE, 0);

      printf("\n");
		printf("Vary current on channel %d\n", channel);
		do
		{
         rn_anaInmAmps(device0, channel, &current, 1,  0);
         if(current != ADOVERFLOW)
         {
         	printf("Current at CH%d is %.2fma\n", channel, current);
         }
         else
         {
         	printf("Max current limit for CH%d has been exceeded\n", channel);
         }
         printf("\nPress ENTER key to read value again or 'Q' or read another channel\n\n");
			while(!kbhit());
			key = getchar();
			while(kbhit()) getchar();

		}while(key != 'q' && key != 'Q');
	}


}	//end main


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
	ain_rdse_all.c

	This sample program is intended for RabbitNet RN1200 ADC boards.

	Description
	===========
   Reads and displays the voltage of all single-ended analog input
   channels. The voltage is calculated from coefficients read from
   the A/D Converter Board.

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
///////////////////////////////////////////////////////////////////////////
#class auto					/* Change local var storage default to "auto" */

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1200			//match ADC board product ID      


//#define NUMSAMPLES 1				// Set sample parameter to do
                                 // one conversion.


#define NUMSAMPLES 10				// Set sample parameter to do
                                 // ten conversions with averaging
                                 // the result.


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


// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

void printrange()
{
	auto int i;
   auto char buffer[256];
   auto int index;

   i = 2;
	DispStr(2,i++,"Gain_code\tVoltage range");
	DispStr(2,i++,"---------\t-------------");

   for (index = 0 ; index < 8; index++)
   {
   	sprintf(buffer, "\t%s", vstr[index]);
		DispStr(2,i++, buffer);
   }
   DispStr(2,++i, "Choose gain code for channel 0 - 7.... ");

}

void  blankScreen(int start, int end)
{
	auto char buffer[256];
   auto int i;

   memset(buffer, 0x00, sizeof(buffer));
 	memset(buffer, ' ', sizeof(buffer));
   buffer[sizeof(buffer)-1] = '\0';
   for(i=start; i < end; i++)
   {
   	DispStr(start, i, buffer);
   }

}


void main ()
{
	auto int device0, status;
 	auto rn_search newdev;
   auto rn_AinData aindata;
   auto float voltage;
   auto int channel;
   auto int gaincode;
   auto int key;
   auto int skip;
	auto char s[80];


	brdInit();
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   //search for device match
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

   for(;;)
	{
   	blankScreen(0, 18);
		// display the voltage that was read on the A/D channels
     	printrange();
 	  	do
	  	{
	  		gaincode = getchar();
	  	} while (!( (gaincode >= '0') && (gaincode <= '7')) );
	  	gaincode = gaincode - 0x30;
	  	printf("%d", gaincode);
	  	while(kbhit()) getchar();

      // Must enable all channels for conversions again after reset
		for(channel = 0; channel < 8; channel++)
		{
			status = rn_anaInConfig(device0, channel, RNSINGLE, gaincode, 0);
         if(status != RNREADY)
         {
           	sprintf(s, "\nRabbitnet config error! Status = %x\n", status);
            DispStr(2,20, s);
            channel--;
         }
 		}
      blankScreen(0, 20);
      sprintf(s, "A/D input voltages for channels 0 - %d", 8 - 1 );
		DispStr(2, 2, s);
		DispStr(2, 3, "--------------------------------------");
      DispStr(2, 16, "Press Q or q to select another gain option.");

     for(;;)
     {
   		skip = 4;
			for(channel = 0; channel < 8; channel++)
			{
      		status = rn_anaInVolts(device0, channel, &voltage, NUMSAMPLES, 0);
            if(voltage != ADOVERFLOW)
         		sprintf(s, "Channel = %2d Voltage = %.3f                    ", channel, voltage);
            else
               sprintf(s, "Channel = %2d Voltage = Exceeded Voltage Range  ", channel);
            DispStr(2,channel + skip, s);
            if(status != RNREADY)
            {
             	sprintf(s, "\nRabbitnet ADC Read Error! Status = %x\n", status);
               DispStr(2,21, s);
               channel--;
            }
			}
         if(kbhit())
			{
				key = getchar();
				if (key == 'Q' || key == 'q')		// check if it's the q or Q key
				{
					while(kbhit()) getchar();
      			break;
     			}
			}
	  }
   }
}
///////////////////////////////////////////////////////////////////////////
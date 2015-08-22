/***************************************************************************

   analog.c

   This program is to verify readings received from the RCM6600W
   on board ADC system.  Several test parameters can be set in the
   defines block below.

   Note: This program assumes that the ADC has been calibrated.  If not,
   then you will see nonsensical voltage readings, however the raw numeric
   readings will be valid.

***************************************************************************/

#use "ADC_R6000.lib"

#class auto

#define ADC8CHANNEL_DEBUG

#define ADC_CLK  0           // 0 for internal, 1 for external
#define ADC_DIV  ADC_DIV16   // Clock divider to use
#define ADC_REF  ADC_RAILS   // Voltage referrence to use
#define ADC_MODE (ADC_ON_DEMAND|ADC_USE_PE01|ADC_USE_PE23)
//#define ADC_MODE (ADC_CONTINUOUS|ADC_USE_PE01|ADC_USE_PE23)
//#define ADC_MODE (ADC_ON_DEMAND|ADC_USE_PE01)

#define STARTCHAN	0          // Lowest channel in range to read.
#define ENDCHAN   3          // Highest channel in range to read.



nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
	while( (long) (MS_TIMER - done_time) < 0 );
}

void gotoxy (int x, int y)
{
	printf( "\x1B[%d;%dH", y, x);
}

//---------------------------------------------------------
//	displays both the raw data count and calculated voltage
//	 equivalent based on selected referrence
//---------------------------------------------------------
void anaInInfo (int channel)
{
   auto float voltequ;
   auto int value;

   value = anaIn(channel);
	voltequ = anaInVolts(channel);
	gotoxy(1,channel+3);
   printf("%1d\t%5d\t%8.5f", channel, value, voltequ);
}

main ()
{
   auto int channel;

   printf("Chan\tRaw\tVolt\n----\t-----\t--------");

   // Get ADC calibration constants from simulated EEPROM
   for(channel=STARTCHAN; channel<=ENDCHAN; channel++)
   	anaInEERd(channel);

   // Initialize ADC for normal mode operation
   anaInConfig(ADC_DIV, ADC_CLK, ADC_REF, 0, ADC_MODE);

   while (1)
   {
      // Scan and display ADC channels
      for (channel = STARTCHAN; channel <= ENDCHAN; channel++)
      {
         anaInInfo(channel);
		}
      msDelay(200);
	}
}


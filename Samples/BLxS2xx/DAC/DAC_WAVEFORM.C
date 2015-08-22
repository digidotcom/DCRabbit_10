/***************************************************************************
   dac_waveform.c

	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BLxS2xx series controllers.

   Description:
   ============
   This program outputs different waveforms on the analog ouput channels
   on the BLxS2xx.

   The DAC circuit is setup for synchronous mode of operation which
   updates all DAC outputs at the same time when the anaOutStrobe
   function executes. The outputs all updated with values previously
   written with anaOutVolts and/or anaOut functions.

	Instructions:
   =============
   1. Set jumpers to connect pins 1-3 and 2-4 on JP5 for voltage outputs
	2. Set jumpers to connect pins 1-2 and 3-4 on JP3 for channel 0 unipolar;
   	or, set jumper to connect pins 5-6 on JP3 for channel 0 bipolar
      NOTE: Set channel 0 & 1 to the same mode, both unipolar or both bipolar
	3. Set jumpers to connect pins 1-2 and 3-4 on JP6 for channel 1 unipolar;
		or, set jumper to connect pins 5-6 on JP6 for channel 1 bipolar
	4. Connect an oscilloscope to the DAC output channels labeled
      AOUT0 - AOUT1 on PCB.
	5. Compile and run this program.
	6. Follow the prompted directions of this program during execution.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// define frequency of the waveform in (1-50Hz)
#define WAVE_FREQUENCY 10

// mnemonics for channels
#define CH0 0
#define CH1 1

// define output channels to strobe
#define CH0_AND_CH1 ((1 << CH0) | (1 << CH1))

#if WAVE_FREQUENCY < 1 || WAVE_FREQUENCY > 50
	#error "WAVE_FREQUENCY must be between 1 and 50 Hz."
#endif

// include BLxS2xx series lbrary
#use "BLxS2xx.lib"

typedef float (*CALC_T)() ;

float calc_square(int index, int array_size, float min_volts, float max_volts);
float calc_sine(int index, int array_size, float min_volts, float max_volts);
float calc_saw(int index, int array_size, float min_volts, float max_volts);

// different waveforms that can be used.
enum
{
	SQUARE_WAVE = 0,
	SINE_WAVE,
	SAW_TOOTH_WAVE,
	NUM_WAVES
};

const int waveform[] = {SQUARE_WAVE, SINE_WAVE, SAW_TOOTH_WAVE};

// array of function pointers for different wave types
const CALC_T calc[] = {calc_square, calc_sine, calc_saw};

// char array of different waveform names.
const char * const waveform_names[NUM_WAVES] = {"Square", "Sine", "Saw Tooth"};

// number of array elements
#define ARRAY_SIZE (1024/WAVE_FREQUENCY)

const float TWO_PI_OVER_SIZE = (2 * 3.1415926) / (1024.0 / WAVE_FREQUENCY);

// arrays of voltages to generate waveforms
far float waveform_signals[NUM_WAVES][ARRAY_SIZE];

// waveform array index for each channel
int channel_waveform[BL_ANALOG_OUT];

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// clear lines in the stdio window
void  clearLines(int start, int end)
{
	auto char buffer[127];
   auto int i;

 	memset(buffer, ' ', sizeof(buffer) - 1);
   buffer[sizeof(buffer)-1] = '\0';
   for (i = start; i <= end; ++i)
   {
   	DispStr(0, i, buffer);
   }
}

calc_waveform(far float array_signals[], int array_size, CALC_T fn, float min_volts, float max_volts)
{
	int index;
   for (index = 0; index < array_size; ++index)
   {
      array_signals[index] = fn(index, array_size, min_volts, max_volts);
   }
}

int dac0_index;
int dac1_index;

void main()
{
	int dac_mode;
   float min_volts, max_volts;
   float phase_shift;
   int index_shift;
   int channel;
   int key;
   int wave;
   int i;
   int length;
	char display[128];
	char tmpbuf[32];

   // Initialize controller
	brdInit();

   DispStr(1, 2, "DAC output voltage configuration");
   DispStr(1, 3, "Dependant on jumper settings (see Instructions)");
   DispStr(1, 4, "--------------------------------");
   DispStr(1, 5, "0 = Unipolar  0  to +10v");
   DispStr(1, 6, "1 = Bipolar  -10 to +10v");
   DispStr(1, 8, "Please enter the DAC configuration 0 - 1 = ");
   do
   {
      key = getchar() - '0';
   } while (key < 0 || key > 1);
   printf("%d", key);

   // Configure the DAC for given configuration
   dac_mode = (key == 0 ? DAC_UNIPOLAR : DAC_BIPOLAR);
   anaOutConfig(dac_mode, DAC_SYNC);

   // set the min and max voltage based on DAC mode.
   max_volts = 10.0;
   min_volts = (dac_mode == DAC_BIPOLAR ? -10.0 : 0.0);

   // Initialize the DAC to output zero volts at the start
   anaOutVolts(CH0, 0);
   anaOutVolts(CH1, 0);
   anaOutStrobe(CH0_AND_CH1);

   channel_waveform[0] = SQUARE_WAVE;
   channel_waveform[1] = SINE_WAVE;

	// initialize waveform variables
	dac1_index = dac0_index = 0;
   phase_shift = 0;
	for (wave = 0; wave < NUM_WAVES; ++wave)
   {
		calc_waveform(waveform_signals[wave], ARRAY_SIZE, calc[waveform[wave]],
      					min_volts, max_volts);
   }

   // print the menu once here, but it is located below "values table".
   DispStr(2, 16, "User Options:");
   DispStr(2, 17, "-------------");
   DispStr(2, 18, "1. Set waveform for DAC0.");
   DispStr(2, 19, "2. Set waveform for DAC1.");
   DispStr(2, 20, "3. Set phase shift.");

   while(1)
   {
   	costate
   	{
	      // clear lines
	      clearLines(12, 14);

         // print the current waveforms
	      sprintf(display, "Current Waveforms: at %dHz", (int) WAVE_FREQUENCY);
	      DispStr(2, 10, display);
	      DispStr(2, 11, "----------------------------");
	      sprintf(display, "DAC0 = %s", waveform_names[channel_waveform[0]]);
	      DispStr(2, 12, display);
	      sprintf(display, "DAC1 = %s", waveform_names[channel_waveform[1]]);
	      DispStr(2, 13, display);
	      sprintf(display, "phase shift = %.2f%%", phase_shift);
	      DispStr(2, 14, display);

         // NOTE: menu goes here sequentually, it is printed once above.

         // get response for menu choice
         do
         {
            waitfor(kbhit());
            key = getchar() - '0';
         } while (key < 0 || key > 3);

         switch (key)
         {
            case 1:  // DAC 0: re-assign waveform
            case 2:  // DAC 1: re-assign waveform
               channel = key - 1; // calculate channel from menu option.
               do
               {
                  // display waveform options
                  for (i = 0; i < NUM_WAVES; ++i)
                  {
                     sprintf(display, "%d: %s: ", i+1, waveform_names[i]);
                     DispStr(3, 23+i, display);
                  }
                  sprintf(display, "Enter waveform for channel %d: ", channel);
                  DispStr(3, 27, display);
						waitfor(kbhit());
                  wave = getchar() - '1';
               } while (wave < 0 || wave >= NUM_WAVES);
               channel_waveform[channel] = wave;
               clearLines(23, 27); // clear the optional input line after the menu
               break;
            case 3: // re-assign phase shift
               do
               {
                  sprintf(display, "Enter phase shift percentage (0 to 100): ");
                  DispStr(3, 23, display);
                  // Note: gets is a blocking function and will stop waveform.
                  phase_shift = atof(gets(tmpbuf));
               } while (phase_shift < 0 || phase_shift > 100);
               index_shift = (int) ((phase_shift * ARRAY_SIZE) / 100.0);
               dac1_index = (dac0_index + index_shift) % ARRAY_SIZE;
               clearLines(23, 23); // clear the optional input line after the menu
               break;
         }
      }

      costate
      {
         waitfor(DelayTicks(1));
         anaOutStrobe(CH0_AND_CH1);

         anaOutVolts(CH0, waveform_signals[channel_waveform[CH0]][dac0_index]);
         anaOutVolts(CH1, waveform_signals[channel_waveform[CH1]][dac1_index]);
         dac0_index = (dac0_index + 1) % ARRAY_SIZE;
         dac1_index = (dac1_index + 1) % ARRAY_SIZE;
      }

   } // while (1)
}


float calc_square(int index, int array_size, float min_volts, float max_volts)
{
	return index < array_size / 2 ? min_volts : max_volts;
}

float calc_sine(int index, int array_size, float min_volts, float max_volts)
{
	float half_volt_range, mid_volts;

	half_volt_range = (max_volts - min_volts) / 2;
	mid_volts = min_volts + half_volt_range;
	return mid_volts + sin(index * TWO_PI_OVER_SIZE) * half_volt_range;
}

float calc_saw(int index, int array_size, float min_volts, float max_volts)
{
	float fraction;

	fraction = index / (float)array_size;
	return min_volts + fraction * (max_volts - min_volts);
}


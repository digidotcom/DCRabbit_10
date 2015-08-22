/********************************************************************
	ppm_quadrature_decoder.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
   This program demonstrates the use of two PWM and two PPM channels
   being connected back to four digital inputs to simulate two quadrature
   encoders feeding signals into the BL4S1xx.  The PPMs are adjusted
   through a menu system, simulating the movement of a quadrature
   encoder.  The results of the quadrature decoder inputs are
   displayed continuously to show the effects of the PWM and PPM outputs.
   The high speed quadrature decoder counts the number of rollovers that
   occur (one per 1000 counts).  The low speed quadrature decoder
   displays the current count in the register.

   Quadrature Encoder
   Counting Up:
    		  __    __    __    __
   PWM A _|  |__|  |__|  |__|  |_  duty = 50%, offset = 0%
       	   __    __    __    __
   PPM B __|  |__|  |__|  |__|  |_ duty = 50%, offset = 25%

   Counting Down:
    		  __    __    __    __
   PWM A _|  |__|  |__|  |__|  |_  duty = 50%, offset = 0%
       	 __    __    __    __
   PPM B    |__|  |__|  |__|  |__  duty = 50%, offset = 75%


	Connections:
	============
   1. Enable 5V pull-ups on the inputs by jumpering pins 5-6 on J13.
   2. Connect the following pins together on the BL4S1xx:
	   // Quadrature Decoder 1
	   IN0 - OUT0
	   IN1 - OUT1
	   // Quadrature Decoder 2
	   IN3 - OUT2
	   IN4 - OUT3

   Note: Different pin combinations can be used, but care must be taken to
   ensure that the PWM and PPM pins simulating a quadrature encoder are on the
   same block on the RIO so that the signals are synchronized.  Also, the
   two input pins reading a quadrature input must be located on the same
   block on the RIO.

	Instructions:
	=============
   1. Compile and run this program.
   2. Change frequency/direction for a given simulated Quadrature Decoder
   	and watch the register counts on the low speed channel and register
      rollovers on the high speed channel.

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// used to index whether we are incrementing or decrementing
enum
{
	SIM_DECREMENT = 0,
   SIM_INCREMENT,
   NUM_DIRECTIONS
};

// index into high or low speeds.
enum
{
	HIGH_SPEED = 0,
   LOW_SPEED,
   NUM_SPEEDS
};

// index into I or Q (In-phase or Quadrature) pin of quadrature decoder
enum
{
	QUAD_Q = 0,
   QUAD_I,
   QUAD_PINS
};

// Global variables
shared int high_speed_count;
int  roll_i_handle, roll_d_handle;
// Quadrature decoder channels
const int DECODER_CH[NUM_SPEEDS][QUAD_PINS] = {{0, 1}, {3, 4}};
// PWM and PPM Quadrature Encoder simulation channels
const int ENCODER_CH[NUM_SPEEDS][QUAD_PINS] = {{0, 1}, {2, 3}};

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

// Rollover on Increment Interrupt Handler (ISR)
void registerRollI(void)
{
	// increment the number of overflows
 	++high_speed_count;

   // Turn off the rollover on increment interrupt flag
   RSB_CLEAR_IRQ(roll_i_handle, BL_IER_ROLL_I);
}

// Rollover on Decrement Interrupt Handler (ISR)
void registerRollD(void)
{
	// decrement the number of overflows
 	--high_speed_count;

   // Turn off the rollover on decrement interrupt flag
   RSB_CLEAR_IRQ(roll_d_handle, BL_IER_ROLL_D);
}

void simulateEncoder(char encoder, char direction, float frequency)
{
	float quad_speed;
   int inversion;
	// PWM and PPM set to 50% duty cycle.
   // The "encoder" will set the frequency to use for the encoder.
   //		HIGH_SPEED = frequency * 250 (KHz); LOW_SPEED = frequency * 0.25 (Hz)
	quad_speed = frequency * (encoder == HIGH_SPEED ? 250.0 : 0.25);
   // The "direction" will phase shift the PPM 50% by inverting the signal.
   // 	SIM_DECREMENT will invert the signal, SIM_INCREMENT will not.
	inversion = direction == SIM_DECREMENT ? 1 : 0;

   setPWM(ENCODER_CH[encoder][QUAD_I], quad_speed, 50, 0, 0);
   setPPM(ENCODER_CH[encoder][QUAD_Q], -1, 25, 50, inversion, 0, 0);
}

void main()
{
   int option;
   char s1[64];
   char display[128];
   char direction[NUM_SPEEDS];
   float frequency[NUM_SPEEDS];
   int freq_input;
   char number[16];
   int length;
   char key;
   unsigned low_speed_count;
   int quad_speed;

   // Initialize the controller
	brdInit();

   // Initialize counter to zero
   high_speed_count = 0;

	// Configure quadrature encoder input
   setDecoder(DECODER_CH[HIGH_SPEED][QUAD_I], DECODER_CH[HIGH_SPEED][QUAD_Q],
   			  -1, 0);
   setDecoder(DECODER_CH[LOW_SPEED][QUAD_I], DECODER_CH[LOW_SPEED][QUAD_Q],
   			  -1, 0);

   // set limit for high speed counter for rollover, 1000 counts
   setLimit(DECODER_CH[HIGH_SPEED][QUAD_I], 1000 - 1);
   resetCounter(DECODER_CH[HIGH_SPEED][QUAD_I]); // latch in limit register

   // Setup interrupt on high speed and attach rollover on increment ISR handler
   roll_i_handle  = addISRIn(DECODER_CH[HIGH_SPEED][QUAD_I],
   					  			  BL_IER_ROLL_I, &registerRollI);
   // Setup interrupt on high speed and attach rollover on decrement ISR handler
   roll_d_handle = addISRIn(DECODER_CH[HIGH_SPEED][QUAD_I],
   								 BL_IER_ROLL_D, &registerRollD);

   // Start PWMs and PPMs with default frequency and direction for each channel
   for (quad_speed = 0; quad_speed < NUM_SPEEDS; ++quad_speed)
   {
	   direction[quad_speed] = SIM_INCREMENT;
	   frequency[quad_speed] = 10;
	   simulateEncoder(quad_speed, direction[HIGH_SPEED], frequency[HIGH_SPEED]);
	}

   // Enable both ISR's
   enableISR(roll_i_handle, 1);
   enableISR(roll_d_handle, 1);

   // print static lines and menu
	DispStr(2, 1, " <<< Simulated Encoders Using PWMs and PPMs >>>");
	DispStr(2, 3, "Simulated Encoders-");
	DispStr(2, 4, "Speed\tFrequency\tDirection");

	DispStr(2, 8, "Decoders-");
	DispStr(2, 9, "Speed\tType     \tCount");

	DispStr(2, 13, "Menu");
	DispStr(2, 14, "1. Set High Speed Encoder frequency");
	DispStr(2, 15, "2. Set Low Speed Encoder frequency");
	DispStr(2, 16, "3. Toggle High Speed Encoder Direction");
	DispStr(2, 17, "4. Toggle Low Speed Encoder Direction");

	while (1)
	{
      // Display current decoder values
      sprintf(display, "High \tRollover\t%6d", high_speed_count);
      DispStr(2, 10, display);
      getCounter(DECODER_CH[LOW_SPEED][QUAD_I], &low_speed_count);
      sprintf(display, "Low  \tRegister\t%6u", low_speed_count);
      DispStr(2, 11, display);

      costate
      {
       	// Display current encoder values
			if (SIM_DECREMENT == direction[HIGH_SPEED])
         {
         	sprintf(display, "High \t%5.0f kHz\tDecrementing",
            			frequency[HIGH_SPEED]);
			} else {
         	sprintf(display, "High \t%5.0f kHz\tIncrementing",
            			frequency[HIGH_SPEED]);
			}
         DispStr(2, 5, display);

         if (SIM_DECREMENT == direction[LOW_SPEED])
         {
         	sprintf(display, "Low  \t%5.0f Hz \tDecrementing",
            			frequency[LOW_SPEED]);
			} else {
         	sprintf(display, "Low  \t%5.0f Hz \tIncrementing",
            			frequency[LOW_SPEED]);
			}
         DispStr(2, 6, display);

      	// clear the buffer of certain lines
	  		DispStr(1, 19, "                                                    ");
  			DispStr(1, 21, "                                                    ");

      	// handle menu key presses
			DispStr(2, 19,"Select Option 1 - 4 = ");
	      waitfor(kbhit());
         option = getchar() - '0';	// convert key to decimal
			switch(option)
         {
         	case 1:  // Set High Speed Encoder frequency
	            sprintf(s1, "Set High Speed Frequency (1 - 50kHz) = ");
	            length = 0;
               while(1) {
		            DispStr(2, 19, s1);
	               waitfor(kbhit());
	               key = getchar();
	               if (key == '\r')
	               {
	                  number[length] = '\0';
	                  break;
	               }
	               number[length] = key;
	               length++;
	               strncat(s1, &key, 1);
	            }
	            freq_input = atoi(number);
	            if (freq_input >= 1 && freq_input <= 50)
	            {
	               frequency[HIGH_SPEED] = freq_input;
	               simulateEncoder(HIGH_SPEED, direction[HIGH_SPEED],
                  						frequency[HIGH_SPEED]);
	            }
					break;
            case 2:	// Set Low Speed Encoder frequency
	            sprintf(s1, "Set Low Speed Frequency (4 - 1000Hz) = ");
	            length = 0;
	            while(1) {
		            DispStr(2, 19, s1);
	               waitfor(kbhit());
	               key = getchar();
	               if (key == '\r')
	               {
	                  number[length] = '\0';
	                  break;
	               }
	               number[length] = key;
	               length++;
	               strncat(s1, &key, 1);
	            }
	            freq_input = atoi(number);
	            if (freq_input >= 4 && freq_input <= 1000)
	            {
	               frequency[LOW_SPEED] = freq_input;
	               simulateEncoder(LOW_SPEED, direction[LOW_SPEED],
                  						frequency[LOW_SPEED]);
	            }
               break;
            case 3:  // Toggle High Speed Encoder Direction
	            // toggle High Speed Encoder direction
	            direction[HIGH_SPEED] ^= 0x01;
	            simulateEncoder(HIGH_SPEED, direction[HIGH_SPEED],
               						frequency[HIGH_SPEED]);
               break;
            case 4:	// Toggle Low Speed Encoder Direction
            	// toggle Low Speed Encoder direction
	            direction[LOW_SPEED] ^= 0x01;
	            simulateEncoder(LOW_SPEED, direction[LOW_SPEED],
               						frequency[LOW_SPEED]);
               break;
         }
      }
	}
}






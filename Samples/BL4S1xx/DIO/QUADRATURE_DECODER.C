/********************************************************************
	quadrature_decoder.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
   This program demonstrates the use of quadrature decoders on a
   RIO.  The demo board is used to show the state changes of a
   quadrature encoder which is simulated using digital outputs.


   Quadrature Encoder
   Counting Up:
    		  __    __    __    __
   PIN I _|  |__|  |__|  |__|  |_
       	   __    __    __    __
   PIN Q __|  |__|  |__|  |__|  |_

   Counting Down:
    		  __    __    __    __
   PIN I _|  |__|  |__|  |__|  |_
       	 __    __    __    __
   PIN Q    |__|  |__|  |__|  |__


	Connections:
	============
	1. DEMO board jumper settings:
			- Set switches to active low by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

   2. Connect the following wires from the controller to the DEMO board:
	      // Power and ground
	      GND   - GND
	      +5K   - +V
	      // Quadrature Decoder Input
	      IN0   - LED1
	      IN1   - LED2
	      // Quadrature Encoder Simulation Output
	      OUT0  - LED1
	      OUT1  - LED2
	      // Switch input
	      IN2   - SW1
	      IN3   - SW2
	      IN4   - SW3

   Note: There are two wires connected to each of the LED connections
   on the DEMO board.

	Instructions:
	=============
   1. Compile and run this program.
   2. Press SW1 button on DEMO board to decrement the quadrature counter.
   	If you hold down SW1 the counter will continue to decrement.
   3. Press SW2 button on DEMO board to increment the quadrature counter.
   	If you hold down SW2 the counter will continue to increment.
   4. Press SW3 button on DEMO board to reset the quadrature counter.

*********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BL4S1xx series library
#use "BL4S1xx.lib"

// defines for the channel number of the switches, LEDs, and Quadrature decoder.
#define QUAD_I	0
#define QUAD_Q	1
#define SW1		2
#define SW2		3
#define SW3		4
#define LED1   0
#define LED2   1

// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

void simulate_encoder(int encoder_state)
{
	if (encoder_state == 0)
   {
     digOut(LED1,0); // encoder pin A
     digOut(LED2,0); // encoder pin B
   }
   else if (1 == encoder_state)
   {
      digOut(LED1,1); // encoder pin A
      digOut(LED2,0); // encoder pin B
   }
   else if (2 == encoder_state)
   {
      digOut(LED1,1); // encoder pin A
      digOut(LED2,1); // encoder pin B
   }
   else
   {
      digOut(LED1,0); // encoder pin A
      digOut(LED2,1); // encoder pin B
 	}
}

void main()
{
   char s[40];
	int encoder_state;
   word count;

   // Initialize the controller
	brdInit();

   // Configure digital outputs to simulate quadrature encoder output
	setDigOut(LED1, 1);
   setDigOut(LED2, 1);

	// Configure quadrature encoder input
   setDecoder(QUAD_I, QUAD_Q, -1, 0);

   // Configure digital inputs for switches
   setDigIn(SW1);
   setDigIn(SW2);
   setDigIn(SW3);

   // initialize outputs to low
   encoder_state = 0;
	simulate_encoder(encoder_state);

   // reset quadrature decoder counter
   resetCounter(QUAD_I);

	DispStr(2, 1, "<<< Simulating a Quadrature Encoder with button presses >>>");
   DispStr(1, 3, "Press Button SW1 to decrement counter");
   DispStr(1, 4, "Press Button SW2 to increment counter");
   DispStr(1, 5, "Press Button SW3 to reset counter");

	while (1)
	{
   	costate
      {
       	// Display the counter value
         getCounter(QUAD_I, &count);
         sprintf(s, "Quadrature Decoder Count = %6u", count);
         DispStr(1, 7, s);
      }

      costate
      {
      	// decrement counter
			waitfor(!digIn(SW1)); 			// wait for switch 1 to be pressed
			waitfor(DelayMs(50));			// debounce
			if (!digIn(SW1)) {
	         --encoder_state;
	         if (encoder_state < 0)
	         {
	            encoder_state = 3;
	         }
	         simulate_encoder(encoder_state);
				waitfor(DelayMs(150));         // repeat when switch held down
         }
      }

      costate
      {
      	// increment counter
			waitfor(!digIn(SW2)); 			// wait for switch 2 to be pressed
			waitfor(DelayMs(50));			// debounce
			if (!digIn(SW2)) {
	         ++encoder_state;
	         if (encoder_state > 3)
	         {
	            encoder_state = 0;
	         }
	         simulate_encoder(encoder_state);
				waitfor(DelayMs(150));         // repeat when switch held down
         }
      }

      costate
      {
      	// reset counter
			waitfor(!digIn(SW3)); 			// wait for switch 3 to be pressed
			waitfor(DelayMs(50));			// debounce
         if (!digIn(SW3)) {
	         resetCounter(QUAD_I);      // reset quadrature decoder counter
	         waitfor(digIn(SW3));       // wait for switch 3 to be released
         }
      }
	}
}






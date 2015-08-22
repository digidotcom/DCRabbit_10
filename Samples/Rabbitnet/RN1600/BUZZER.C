/**************************************************************************

	buzzer.c
 	Z-World, 2004

	This sample program is intended for RN1600 RabbitNet Keypad/Display
   Interface card.

   Description
   ============
	This program demonstrates the control of the buzzer on the RN1600,
   it uses both API functions rn_keyBuzzer() and rn_keyBuzzerAct().

   The LED labeled "UP BAD" is also shown using rn_keyLedOut() to
   turn the LED on and off.

	Note: The Buzzer is mono-tone, however by toggling the control bit
	      faster or slower, some pitch and motor boat affects can be
	      obtained.


   Instructions
   ============
   1.  Compile and run this program.

**************************************************************************/
#class auto
#memmap xmem  // Required to reduce root memory usage

/////
//local macros
/////
#define ON 1
#define OFF 0

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1600			//RN1600 KDIF card


nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


void main()
{
	auto int i, delay, loop, counter;
	auto int device0, status;
   auto rn_search newdev;

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------
	brdInit();					// Initialize the controller
   rn_init(RN_PORTS, 1);   // Initialize controller RN ports

   // Verify that the Rabbitnet display board is connected
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

	printf("\n\n");

	// Main loop to control the buzzer
	for(loop = 0; loop < 2; loop++)
	{
		// Generate 3 long beeps
		printf("\n\nGenerate 3 beeps\n\n");
		for(i=0; i < 3; i++)
		{
        	rn_keyBuzzer(device0, ON, 0);    //on
			msDelay(400);
         rn_keyBuzzer(device0, OFF, 0);    //off
			msDelay(200);

		}
      rn_keyLedOut(device0, 0, 1, 0);  //led on
		msDelay(2000);
      rn_keyLedOut(device0, 0, 0, 0);  //led off

		// Make the buzzer go up in pitch
		printf("Make the buzzer goes up in pitch\n\n");
		for (i=20; i > 0 ; i--)
		{
         for (delay=0; delay<4; delay++)
         {
	         rn_keyBuzzer(device0, ON, 0);    //on
   	      msDelay(i);
      	   rn_keyBuzzer(device0, OFF, 0);    //off
         	msDelay(i);
         }
		}
      rn_keyLedOut(device0, 0, 1, 0);  //led on
		msDelay(2000);
      rn_keyLedOut(device0, 0, 0, 0);  //led off

		// Make the buzzer go down in pitch
		printf("Make the buzzer goes down in pitch\n\n");
		for(i=0; i<20; i++)
		{
         for (delay=0; delay<4; delay++)
         {
	        	rn_keyBuzzer(device0, ON, 0);    //on
   	      msDelay(i);
      	   rn_keyBuzzer(device0, OFF, 0);    //off
         	msDelay(i);
         }
      }
      rn_keyLedOut(device0, 0, 1, 0);  //led on
		msDelay(2000);
      rn_keyLedOut(device0, 0, 0, 0);  //led off

      printf("Use non-blocking buzzerActivate\n\n");
	 	rn_keyBuzzerAct(device0, 2000, 0);  //sound buzzer for 5 seconds
      counter = 0;
    	for(i=0; i < 600; i++)
		{
      	printf("Code Executing while buzzer is ON = %d     \r", counter++);
		}
      rn_keyLedOut(device0, 0, 1, 0);  //led on
		msDelay(3000);
      rn_keyLedOut(device0, 0, 0, 0);  //led off

	}
}
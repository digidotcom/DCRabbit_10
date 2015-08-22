/***************************************************************************
   Relay_All.c
   Z-World Inc 2003

   This sample program is intended for RabbitNet RN1400 Relay boards.

  	This program demonstrates activating/deactivating all the relays
   in parallel with using the rn_RelayAll API function.


   Instructions:
   -------------
   1. Compile and run this program.

   2. To verify the relay connections, set a breakpoint(using F2) on
   either of the following two statements in mainline for a given relay:

     	printf("All Relays COM is connected to its NO contact\n");


            				" OR "

      printf("All Relays COM is connected to its NC contact\n");

	Once you hit the breakpoint use an ohmmeter to verify that the
   contacts are connected, ohmmeter reading should be ~0 ohms for
   contacts that are connected and high impedance for the contacts
   that are NOT connected.

	Note: When toggling the relays, the LED for the given relay will
   also be toggling.

***************************************************************************/
#class auto


#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1400			//match Relay board product ID 

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
	while( (long) (MS_TIMER - done_time) < 0 );
}

main()
{
   auto char pwrSave;
   auto int i;
   auto int device0;
   auto rn_search newdev;

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

   printf("Toggle all Relays using a 5 second toggle rate\n");
   printf("----------------------------------------------\n\n");

   for(;;)
   {
   	// Activate relays 0 - 5
      rn_RelayAll(device0, 0x3F, 0);
   	printf("- All Relays ON......COM is connected to its NO contact\n\n");
      msDelay(5000);

      // Deactivate all relays
      rn_RelayAll(device0, 0x00, 0);
      printf("- All Relays OFF.....COM is connected to its NC contact\n\n");
      msDelay(5000);
   }
}







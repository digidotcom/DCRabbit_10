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
   Relay_Low_Pwr.c

   This sample program is intended for RabbitNet RN1400 Relay boards.

	This program demonstrates how to configure the relays for power-save
   mode of operation. When a relay has been configured for power-save mode,
   it will first when activated turn-on for a minimum of 50ms, after which
   the relay will be pulsed every 1 ms with a 50% duty cycle square wave,
   which should provide a power reduction of 50% for the given relay.

   Normal relay activation current is ~80ma, with power save mode the
   current for a given relay will be reduced to ~40ma.

  	Caution:
   --------
	1. Activating several relays in a short period of time may cause a power
   surge that may exceed the peak power rating of your power supply. It is
   ultimately the responsibility of the application designer to assure the
   power supply selected meets the requirements for the intended application.

	2. The power save mode will reduce relay holding force,  therefore,
   recommend not using power save mode when the relay is subject to shock
   and vibration.


  	Instructions
   ------------
  	1. Put a DC current meter in series with power supply GND lead going
   to the relay card for verification of current reduction when the relay
   is put into the power-save mode.

   2. Compile and run this program

   3. While the program is running, monitor the current meter and STDIO to
   see what the current readings are for the various relay states.

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
   auto int relay;


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

   for(;;)
   {
   	printf("Turn Relay ON, then enable/disable power-save mode\n");
   	printf("--------------------------------------------------");
   	for(relay=0; relay < 6; relay++)
      {
    		// Activate relay
      	rn_Relay(device0, relay, 1, 0);
         printf("\n\n");
         printf("-Activate Relay %d.\n", relay);
      	msDelay(3000);

      	// Relay x is already ON, now set to power-save mode. Relay x has
      	// been ON for ~3 seconds, now will be pulsed with a 50% 1ms square
      	// wave to reduce power.
      	rn_RelayPwr(device0, (0x01 << relay), 0);
      	printf("-Set Relay %d for power-save mode.\n", relay);
      	msDelay(3000);

     	 	// Deactivate relay x
      	rn_Relay(device0, relay, 0, 0);
         rn_RelayPwr(device0, (0x01 << relay), 0);
         printf("-Deactivate Relay %d.\n", relay);
      	msDelay(3000);
      }

      printf("\n\n\n");
      printf("Enable power save mode, toggle relay ON/OFF\n");
   	printf("-------------------------------------------");
      rn_RelayPwr(device0, 0x3F, 0);
      for(relay=0; relay < 6; relay++)
      {
   		// Activate relay.... will be initially turned ON for 50ms,
      	// then will be pulsed with a 50% 1ms square wave to reduce
         // power.
         printf("\n\n");
      	rn_Relay(device0, relay, 1, 0);
   		printf("-Activate Relay %d.\n", relay);
      	msDelay(3000);

     	 	// Deactivate relay x
      	rn_Relay(device0, relay, 0, 0);
      	printf("-Deactivate Relay %d.\n", relay);
      	msDelay(3000);
      }
      rn_RelayPwr(device0, 0x00, 0);
      printf("\n\n\n");
   }
}
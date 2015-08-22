/***************************************************************************
   Relay_Sequence.c
   Z-World Inc 2003

   This sample program is intended for RabbitNet RN1400 Relay boards.

	This program demonstrates how to activate the relays sequenually to
   keep the peak power surges to a minimum when relays are activated.

   Caution:
   --------
	1. Activating several relays in a short period of time may cause a
   power surge that may exceed the peak power rating of your power supply.

	2. The power save mode will reduce relay holding force,  therefore,
   recommend not using power save mode when the relay is subject to shock
   and vibration.

   It is ultimately the responsibility of the application designer to
   assure that the power supply selected meets the requirements for the
   intended application.


	Instructions:
   -------------
   1. Compile and run this program.

   2. Select the set of relays to activate via STDIO program menu.

   3. Visually verify that the relays are being sequenced via the relays LED.

***************************************************************************/
#class auto

#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1400			//match Relay board product ID 

#define RELAY_IS_OFF			0
#define RELAY_REQUEST_ON   1
#define RELAY_IS_ON        2
#define RELAY_REQUEST_OFF	3

#define MENU_OPT1	'1'
#define MENU_OPT2	'2'
#define MENU_OPT3	'3'
#define MENU_OPT4	'4'

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
	while( (long) (MS_TIMER - done_time) < 0 );
}


// set the STDIO cursor location and display a string
void DispStr(int x, int y, char *s)
{
   x += 0x20;
   y += 0x20;
   printf ("\x1B=%c%c%s", x, y, s);
}

void set_relay_state(int ch, char *CurrentState, int *relay_control)
{
  	switch(ch)
   {
   	case MENU_OPT1:
         CurrentState[0] = RELAY_REQUEST_ON;
         CurrentState[1] = RELAY_REQUEST_OFF;
         CurrentState[2] = RELAY_REQUEST_ON;
         CurrentState[3] = RELAY_REQUEST_OFF;
         CurrentState[4] = RELAY_REQUEST_ON;
         CurrentState[5] = RELAY_REQUEST_OFF;
         *relay_control = TRUE;
         break;

      case MENU_OPT2:
        	CurrentState[0] = RELAY_REQUEST_OFF;
        	CurrentState[1] = RELAY_REQUEST_ON;
         CurrentState[2] = RELAY_REQUEST_OFF;
         CurrentState[3] = RELAY_REQUEST_ON;
         CurrentState[4] = RELAY_REQUEST_OFF;
        	CurrentState[5] = RELAY_REQUEST_ON;
         *relay_control = TRUE;
         break;

 		case MENU_OPT3:
      	memset(CurrentState, RELAY_REQUEST_ON, 6);
         *relay_control = TRUE;
         break;

      case MENU_OPT4:
      	memset(CurrentState, RELAY_REQUEST_OFF, 6);
         *relay_control = TRUE;
         break;
	}
}

main()
{
   auto int device0;
   auto rn_search newdev;
   auto char s[128];
   auto int counter, i, option;

   // Array locations 0 - 5 used to indicate what state relays
   // 0 - 5 are in, here's the possible states.
   // -----------------------------------------
   //  0 = Relay OFF...no action to be taken
   //  1 = Request for relay to be activated
   //  2 = Relay actvated...no action to be taken.
   //  3 = Request for relay to be deactivated.
	//
   // The user will select a menu option which set the memory
   // locations 0 - 5 with the desired relay state. The main
   // program will detect when a change occurs in the memory
   // array, which will then update the relay(s) with the new
   // Relay state.

   auto char CurrentRelayState[6];
   auto char NewRelayState[6];
   auto int relay_control_update;
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

  	//Display user instructions and channel headings
  	DispStr(2, 1, "<<< Relay Control Menu >>>");
   DispStr(2, 2, "--------------------------");
   DispStr(2, 3, "1.Sequence Relays 0, 2, and 4 ON, set all others OFF.");
   DispStr(2, 4, "2.Sequence Relays 0, 1, 3, and 5 ON, set all others OFF.");
   DispStr(2, 5, "3.Sequence All Relays ON.");
   DispStr(2, 6, "4.Sequence All Relays OFF.");

   counter = 0;
   relay_control_update = FALSE;
   memset(CurrentRelayState, 0x00, 6);
   memset(NewRelayState, 0x00, 6);

   for(;;)
   {

   	costate
      {
      	sprintf(s,"Application program is running, counter = %d", counter++);
      	DispStr(2, 10, s);

      }
   	costate
   	{
         if(kbhit())
         {
            option = getchar();
           	set_relay_state(option, &CurrentRelayState[0], &relay_control_update);
         }
         waitfor(DelayMs(10));
      }
      costate
      {
      	if(relay_control_update)
         {
         	for(relay=0; relay < 6; relay++)
            {
            	if(NewRelayState[relay] == 1)
               {
               	// Activate given relay, then wait for 50ms
                  rn_Relay(device0, relay, 1, 0);
                  NewRelayState[relay] = RELAY_IS_ON;
                  CurrentRelayState[relay] = RELAY_IS_ON;
                  // Wait for relay to stabilize
                  waitfor(DelayMs(50));
               }
               else if(NewRelayState[relay] == 3)
               {
               	// Deactivate relay, then wait for 5ms
                  rn_Relay(device0, relay, 0, 0);
                  NewRelayState[relay] = RELAY_IS_OFF;
                  CurrentRelayState[relay] = RELAY_IS_OFF;
                  // Wait for relay power OFF completely
                  waitfor(DelayMs(50));
               }
        		}
            relay_control_update = FALSE;
         }
         else
         {
         	if(memcmp(CurrentRelayState,NewRelayState, 6) != 0)
   			{
          		memcpy(NewRelayState, CurrentRelayState, 6);
          		relay_control_update = TRUE;
         	}
         }
      }
   }
}










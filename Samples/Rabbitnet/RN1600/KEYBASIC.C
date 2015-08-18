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
	keybasic.c

	This sample program is intended for RN1600 RabbitNet Keypad/Display
   Interface card.

   Description
   ============
   This program demonstrates use of the 4x10 keypad found in the
   development kit.

   This program will display the following in the STDIO display
   window:

	-- Displays custom ASCII keypad return values.
   -- Demonstrates the use of the buzzer features.


   Keypad character assignment for this example:

 	[  1 ] [  2 ] [  3 ] [  4 ] [  5 ] [  6 ] [  7 ] [  8 ] [  9 ] [  0 ]
	[  A ] [  B ] [  C ] [  D ] [  E ] [  F ] [  G ] [  H ] [  I ] [  J ]
 	[  K ] [  L ] [  M ] [  N ] [  O ] [  P ] [  Q ] [  R ] [  S ] [  T ]
 	[  U ] [  V ] [  W ] [  X ] [  Y ] [  Z ] [  * ] [  # ] [  < ] [  > ]


   Instructions
   ============
   0. Install the 4x10 keypad on J6.
      To ensure keypad driver compatibility, the keypad
      must be installed so that a strobe line or data line
      starts on J6 pin 1.
	1. Compile and run this program.
	2. Press each key on the controller keypad.

***************************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage
/////
//local macros
/////
#define ON 1
#define OFF 0
#define KEYSTROBLINES  0x1F00		//strobe lines for 4x10 keypad

//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1600			//RN1600 KDIF card


/*****************************************************
 4x10 keypad index:
 [ 32 ] [ 33 ] [ 24 ] [ 25 ] [ 16 ] [ 17 ] [  8 ] [  9 ] [  0 ] [  1 ]
 [ 34 ] [ 35 ] [ 26 ] [ 27 ] [ 18 ] [ 19 ] [ 10 ] [ 11 ] [  2 ] [  3 ]
 [ 36 ] [ 39 ] [ 28 ] [ 31 ] [ 20 ] [ 23 ] [ 12 ] [ 15 ] [  4 ] [  7 ]
 [ 38 ] [ 37 ] [ 30 ] [ 29 ] [ 22 ] [ 21 ] [ 14 ] [ 13 ] [  6 ] [  5 ]

 Associated character assignments:
 [  1 ] [  2 ] [  3 ] [  4 ] [  5 ] [  6 ] [  7 ] [  8 ] [  9 ] [  0 ]
 [  A ] [  B ] [  C ] [  D ] [  E ] [  F ] [  G ] [  H ] [  I ] [  J ]
 [  K ] [  L ] [  M ] [  N ] [  O ] [  P ] [  Q ] [  R ] [  S ] [  T ]
 [  U ] [  V ] [  W ] [  X ] [  Y ] [  Z ] [  * ] [  # ] [  < ] [  > ]
      |  |  |  |  |  |  |  |  |  |  |  |  |
      |  |  |  |  |  |  |  |  |  |  |  |  |
     12 11 10  9  8  7  6  5  4  3  2  1  0

	Connector Pins 12,11,10,9,8 are output strobes.

 *****************************************************/
void configKeypad(int device0)
{
	rn_keyConfig (device0, 32,'1',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 33,'2',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 24,'3',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 25,'4',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 16,'5',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 17,'6',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  8,'7',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  9,'8',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  0,'9',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  1,'0',0, 0, 0,  0, 0);

	rn_keyConfig (device0, 34,'A',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 35,'B',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 26,'C',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 27,'D',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 18,'E',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 19,'F',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 10,'G',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 11,'H',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  2,'I',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  3,'J',0, 0, 0,  0, 0);

	rn_keyConfig (device0, 36,'K',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 39,'L',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 28,'M',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 31,'N',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 20,'O',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 23,'P',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 12,'Q',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 15,'R',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  4,'S',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  7,'T',0, 0, 0,  0, 0);

	rn_keyConfig (device0, 38,'U',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 37,'V',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 30,'W',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 29,'X',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 22,'Y',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 21,'Z',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 14,'*',0, 0, 0,  0, 0);
	rn_keyConfig (device0, 13,'#',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  6,'<',0, 0, 0,  0, 0);
	rn_keyConfig (device0,  5,'>',0, 0, 0,  0, 0);
}


void main (	void	)
{
	unsigned	wKey;		//	User Keypress
	int keyflag, keypad_active, done;
   auto rn_search newdev;

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------
	auto int device0, tmpdev, portnum, i, statusbyte;
	auto char sendbyte, recbyte;
   auto rn_devstruct *devaddr;

	brdInit();                 //initialize controller
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   //search for device match
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
	device0 = rn_find(&newdev);
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }


   //Configure keypad pins 12,11,10,9,8 as output strobes
   //Set key press buzzer to 10 milliseconds
   rn_keyInit(device0, KEYSTROBLINES, 10);

   configKeypad(device0);	//user assign characters to keypad index

 	rn_keyBuzzerAct(device0, 500, 0);  //activate buzzer for 1 sec

	for(;;)
	{

		costate
		{
			rn_keyProcess (device0, 0);
			waitfor ( DelayMs(10) );
		}

		costate
		{
			waitfor ( wKey = rn_keyGet(device0, 0) );	//	Wait for Keypress
         if (wKey != 0)
         {
				printf("KEY PRESSED = %c\n", wKey);
         }
		}

		costate
		{
         if (wKey == '1')
         {
         	rn_keyBuzzer(device0, ON, 0);    //on
				printf("PRESS 2 to turn buzzer off\n");
         }
		}
		costate
		{
      	if (wKey == '2')
         {
           	rn_keyBuzzer(device0, OFF, 0);    //off
				printf("PRESS 1 to turn buzzer on\n");
         }
		}

		costate
		{
      	if (wKey == '3')
         {
 				rn_keyBuzzerAct(device0, 500, 0);  //activate buzzer for 1 sec
         }
		}

	}
}
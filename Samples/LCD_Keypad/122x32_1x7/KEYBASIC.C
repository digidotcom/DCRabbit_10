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

   NOTE: Not currently supported on RCM4xxx modules.

	Sample program to demonstrate the keypad functions. This program
	will display the following in the STDIO display window:

	1. Displays default ASCII keypad return values.
	2. Displays custom ASCII keypad return values.
	3. Demonstrates keypad repeat functionality.

	Instructions:
	1. Compile and run this program.
	2. Press each key on the controller keypad at least once when prompted.
	3. When the "Key Repeat" prompt appears, press the key and hold it down
	   for a least 1-2 seconds to see the "Key Repeat" feature in action.

***************************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

void main (	void	)
{
	unsigned	wKey;		//	User Keypress
	int keyflag, keypad_active, done;

	brdInit();		// Required for all controllers.
	dispInit();		// Start-up the keypad driver
	keypadDef();	// Use the default keypad ASCII return values

	printf("\n\n");
	printf("LCD/Keypad Sample Program\n");
	printf("---------------------------\n");

	for(;;)
	{

		costate
		{								//	Process Keypad Press/Hold/Release
			keyProcess ();
			waitfor ( DelayMs(10) );
		}

		costate
		{
			//------------------------------------------------------------------------------------------
			//	Default Key demo
			//------------------------------------------------------------------------------------------

			printf("\n\nPress all keys on the keypad to see Default ASCII return values!\n\n\n");
			keypadDef();
			keyflag = 0;
			done = FALSE;

			while(!done)
			{
				waitfor ( wKey = keyGet() );	//	Wait for Keypress
				printf("KEY PRESSED = %c\n\r", wKey);
				switch(wKey)
				{
					case 'R': 	keyflag |= 0x01; 	break;
					case 'E':	keyflag |= 0x02;	break;
					case 'D':	keyflag |= 0x04;	break;
					case '-':	keyflag |= 0x08;	break;
					case '+':	keyflag |= 0x10;	break;
					case 'U':	keyflag |= 0x20;	break;
					case 'L':	keyflag |= 0x40;	break;
				}
				if(keyflag == 0x7F)
				{
					done = TRUE;
				}
			}

			//------------------------------------------------------------------------------------------
			//	Custom Key demo
			//------------------------------------------------------------------------------------------

			printf("\n\nReconfigured keypad for custom ASCII return values...Press all keys again!\n\n\n");
			keyConfig ( 3,'D',0, 0, 0,  0, 0 );
			keyConfig ( 6,'G',0, 0, 0,  0, 0 );
			keyConfig ( 2,'C',0, 0, 0,  0, 0 );
			keyConfig ( 5,'F',0, 0, 0,  0, 0 );
			keyConfig ( 1,'B',0, 0, 0,  0, 0 );
			keyConfig ( 4,'E',0, 0, 0,  0, 0 );
			keyConfig ( 0,'A',0, 0, 0,  0, 0 );
			keyflag = 0;
			done = FALSE;
			while(!done)
			{
				waitfor ( wKey = keyGet() );	//	Wait for Keypress
				printf("KEY PRESSED = %c\n\r", wKey);
				switch(wKey)
				{
					case 'A': 	keyflag |= 0x01;	break;
					case 'B':	keyflag |= 0x02;	break;
					case 'C':	keyflag |= 0x04;	break;
					case 'D':	keyflag |= 0x08;	break;
					case 'E':	keyflag |= 0x10;	break;
					case 'F':	keyflag |= 0x20;	break;
					case 'G':	keyflag |= 0x40;	break;
				}
				if(keyflag == 0x7F)
				{
					done = TRUE;
				}

			}

			//------------------------------------------------------------------------------------------
			//	Key repeat demo
			//------------------------------------------------------------------------------------------

			printf("\n\nReconfigured keys back to the default with fast repeat enabled\n\n\r");
			keypadDef();

			// Setup for FAST key repeat after holding down key for 150 ticks.
			//
			// Note: The keyConfig function "cCntHold" parameter along with the delay
			// between keyprocess determines the initial KEY hold time before it starts
			// to repeat. The delay being used to control the hold time is in the first
			// costatement in this program:
			//
			//
			//		costate
			//		{								//	Process Keypad Press/Hold/Release
			//			keyProcess ();
			//			waitfor ( DelayMs(10) );
			//		}
			//
			keyConfig (  3,'R',0, 150, 1,  1, 1 );
			keyConfig (  6,'E',0, 150, 1,  1, 1 );
			keyConfig (  2,'D',0, 150, 1,  1, 1 );
			keyConfig (  5,'+',0, 150, 1,  1, 1 );
			keyConfig (  1,'U',0, 150, 1,  1, 1 );
			keyConfig (  4,'-',0, 150, 1,  1, 1 );
			keyConfig (  0,'L',0, 150, 1,  1, 1 );

			keyflag = 0;
			done = FALSE;
			while(!done)
			{
				waitfor ( wKey = keyGet() );	//	Wait for Keypress
				printf("KEY PRESSED = %c\n\r", wKey);
				switch(wKey)
				{
					case 'R': 	keyflag |= 0x01;	break;
					case 'E':	keyflag |= 0x02;	break;
					case 'D':	keyflag |= 0x04;	break;
					case '+':	keyflag |= 0x08;	break;
					case 'U':	keyflag |= 0x10;	break;
					case '-':	keyflag |= 0x20;	break;
					case 'L':	keyflag |= 0x40;	break;
				}
				if(keyflag == 0x7F)
				{
					done = TRUE;
				}
			}
		}
	}
}
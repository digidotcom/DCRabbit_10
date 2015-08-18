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
/********************************************************************
	pong.c

	This sample program is intended for RN1600 RabbitNet
   Keypad/Display Interface card.

   Description
   ============
   This program demonstrates use of the 3x4 keypad and 2x20 display
   found in the development kit.

   Keypad character assignment for this example:

   	[  1  ] [  2  ] [  3  ] [  +  ]
		[  4  ] [  5  ] [  6  ] [  -  ]
		[  7  ] [  8  ] [  9  ] [  0  ]


	Instructions
   ============
   0. Install the 3x4 keypad on J6 and 2x20 display onto J4.
      To ensure keypad driver compatibility, the keypad
      must be installed so that a strobe line or data line
      starts on J6 pin 1.
 	1. Compile and run this program.
	2. Press '+' to increase ball speed.
   3. Press '-' to decrease ball speed.
   4. Press '0' to stop ball movement.
   5. Press '+' or '-' to start ball movement.
	6. Any key presses will display in the STDIO window.

*********************************************************************/
#class auto
#memmap xmem  // Required to reduce root memory usage
/////
//local macros
/////
#define ON 1
#define OFF 0
#define DISPROWS 2    //number of lines in display
#define DISPCOLS 20    //number of columns in display
#define KEYSTROBLINES 0x0070		//strobe lines for 3x4 keypad


//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1600			//RN1600 KDIF card


/**********************************************
   3x4 keypad index:
   [ 19  ] [ 18  ] [ 17  ] [ 16  ]
	[ 11  ] [ 10  ] [  9  ] [  8  ]
	[  3  ] [  2  ] [  1  ] [  0  ]

   Associated character assignments:
   [  1  ] [  2  ] [  3  ] [  +  ]
	[  4  ] [  5  ] [  6  ] [  -  ]
	[  7  ] [  8  ] [  9  ] [  0  ]
      |  |  |  |  |  |  |  |  |  |
      |  |  |  |  |  |  |  |  |  |
      9  8  7  6  5  4  3  2  1  0

 	Connector Pins 6,5,4 are output strobes.
**********************************************/
void configKeypad3x4(int device)
{
	//setup characters on keypad
	rn_keyConfig (device, 19,'1',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 18,'2',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 17,'3',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 16,'+',0, 0, 0,  0, 0 );

	rn_keyConfig (device, 11,'4',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 10,'5',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  9,'6',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  8,'-',0, 0, 0,  0, 0 );

	rn_keyConfig (device,  3,'7',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  2,'8',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  1,'9',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  0,'0',0, 0, 0,  0, 0 );

}


/***************************************************************************
***************************************************************************/
void main (	void	)
{
	auto int wKey, ballspeed, stop;
 	auto int device0;
   auto rn_search newdev;
   auto unsigned int i;

	auto int	px,py;                        // Current Position
	auto int dx,dy;                        // Current Direction
	auto int nx,ny;                        // New Position
	auto int	xl, xh,	yl, yh;

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------
	brdInit();			// Initialize the controller

   rn_init(RN_PORTS, 1);      // Initialize controller RN ports

   // Verify that the Rabbitnet display board is connected
	newdev.flags = MATCHFLAG;
	newdev.productid = MATCHPID;
   if ((device0 = rn_find(&newdev)) == -1)
   {
   	printf("\n no device found\n");
      exit(0);
   }

   // Initialize Display and Keypad low-level drivers
   // Note: Functions brdInit() and rn_init() must executed before initializing
   //       display and keypad drivers.
   rn_keyInit(device0, KEYSTROBLINES, 10);   //set key press buzzer for 10ms
	configKeypad3x4(device0);	// Set keys to the default driver configuration
	rn_dispInit(device0, DISPROWS, DISPCOLS);

   rn_dispPrintf(device0, 0, "Start   Pong");
   rn_keyBuzzerAct(device0, 100, 0);
	for (i=0; i<40000; i++);	//small delay
   rn_keyBuzzerAct(device0, 100, 0);
   rn_dispClear(device0, 0);
	rn_dispCursor(device0, RNDISP_CURBLINKON, 0);

	xl =  0;							// box coordinates, start column
	xh =  DISPCOLS;				// box coordinates, end column
	yl =  0;                   // box coordinates, start row
	yh =  DISPROWS; 				// lines 0 .. yh-1
   px = xl; py = yl;        	// Position Ball
   dx = 1; dy = 1;            // Give Direction

   ballspeed = 200;           // start ball speed at 200 ms delay
   stop = 0;

	while(1)
	{
   	costate
      {
      	if (!stop)
         {
		      rn_dispGoto (device0, px, py, 0);
				waitfor(DelayMs(ballspeed));

	   	   nx = px + dx;               	// Try New Position
   	   	ny = py + dy;

	      	if (nx <= xl || nx >= xh)     // Avoid Collision
   	      	dx = -dx;
	   	   if (ny <= yl || ny >= yh)
   	   	   dy = -dy;

	      	nx = px + dx;               	// Next Position
		      ny = py + dy;

	   	   rn_dispGoto (device0, px, py, 0);
				waitfor(DelayMs(ballspeed));

	   	   px = nx; py = ny;           	// Move Ball
         }
      }

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
      	if ((wKey == '-') && (ballspeed<500))
         {
         	ballspeed+=10;		//increase delay
            stop = 0;
         }
		}

		costate
		{
      	if ((wKey == '+') && (ballspeed>10))
         {
         	ballspeed-=10;		//decrease delay
            stop = 0;
         }
		}

		costate
		{
      	if (wKey == '0')
         {
         	stop = 1;			//stop ball movement
         }
		}
   }
}
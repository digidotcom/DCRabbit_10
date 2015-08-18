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
	alphanum.c

	This sample program is intended for RN1600 RabbitNet Keypad/Display
   Interface card.

   Description
   ============
   This program demonstrates use of the 2x6 keypad and 4x20 display
   found in the development kit.	It demonstrate how you can create
   messages with the keypad and then display them on the LCD.

   Note: Backlight function will work on displays that have
   backlight capability.

   Keypad character assignment for this example:

		[  <  ] [  >  ] [  ^  ] [  A  ] [  B  ] [     ]
		[  -  ] [  +  ] [  v  ] [  C  ] [  D  ] [  E  ]

	Instructions
   ============
   0. Install the 2x6 keypad on J6 and 4x20 display onto J5.
      To ensure keypad driver compatibility, the keypad
      must be installed so that a strobe line or data line
      starts on J6 pin 1.
	1. Compile and run this program.
	2. Follow the instructions on the LCD display.

		'<' will move cursor left in the character set
		'>' will move cursor right in the character set
		'^' will display previous character set
		'v' will display next character set
      ' ' will insert a blank character
      'E' Enter key

***************************************************************************/
#class auto
#memmap xmem  // Required to reduce root memory usage
/////
//local macros
/////
#define ON 1
#define OFF 0
#define DISPROWS 4     //number of lines in display
#define DISPCOLS 20    //number of columns in display
#define KEYSTROBLINES 0x00C0		//strobe lines for 2x6 keypad


//////
// Search criteria
//////
#define MATCHFLAG RN_MATCH_PRDID	//set flag to search for product ID
#define MATCHPID  RN1600			//RN1600 KDIF card


// Character set used to create a message
char * const s[] = { "ABCDEFGHIJKLM",
					     "NOPQRSTUVWXYZ",
						  "0123456789",
						  "!@#\\\"/-+&$%^",
						  "&*()_="};

// Instructions for the message menu
char * const help[] = {"Use Scrolling keys to select characters ",
							 "Use PLUS key to add a characters        ",
							 "Use MINUS key for a backspace           ",
							 "Use last character in string for SPACE  ",
							 "Use ENTER key to exit message menu      "};


// General use buffer
char buffer[1024];
static int device0;


/**********************************************
   2x6 keypad index:
   [ 13  ] [ 12  ] [ 11  ] [ 10  ] [  9  ] [  8  ]
	[  5  ] [  4  ] [  3  ] [  2  ] [  1  ] [  0  ]

   Associated character assignments:
	[  <  ] [  >  ] [  ^  ] [  A  ] [  B  ] [     ]
	[  -  ] [  +  ] [  v  ] [  C  ] [  D  ] [  E  ]
    |  |  |  |  |  |  |  |  |  |
    |  |  |  |  |  |  |  |  |  |
    9  8  7  6  5  4  3  2  1  0

 	Connector Pins 7, 6 are output strobes.
**********************************************/
void configKeypad2x6(int device)
{
	//setup characters on keypad
	rn_keyConfig (device, 13,'<',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 12,'>',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 11,'^',0, 0, 0,  0, 0 );
	rn_keyConfig (device, 10,'A',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  9,'B',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  8,' ',0, 0, 0,  0, 0 );

	rn_keyConfig (device,  5,'-',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  4,'+',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  3,'v',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  2,'C',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  1,'D',0, 0, 0,  0, 0 );
	rn_keyConfig (device,  0,'E',0, 0, 0,  0, 0 );

}

void fast_configKeypad2x6(int device)
{
	// Setup for FAST key repeat after holding down key for 12 ticks
	rn_keyConfig (device, 13,'<',0, 12, 1,  1, 1 );
	rn_keyConfig (device, 12,'>',0, 12, 1,  1, 1 );
	rn_keyConfig (device, 11,'^',0, 12, 1,  1, 1 );
	rn_keyConfig (device, 10,'A',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  9,'B',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  8,' ',0, 12, 1,  1, 1 );

	rn_keyConfig (device,  5,'-',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  4,'+',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  3,'v',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  2,'C',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  1,'D',0, 12, 1,  1, 1 );
	rn_keyConfig (device,  0,'E',0, 12, 1,  1, 1 );
}

//***************************************************************************
//	General use MS delay function
//***************************************************************************
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


//***************************************************************************
//	Clear a single text line on the LCD display.
//***************************************************************************
void clrline(int device, int line, int len)
{
	char spaces[256];
	int i;

	for(i=0; i < len; i++)
	{
		spaces[i] = ' ';
	}
	spaces[i] = '\0';

	rn_dispGoto(device, 0, line, 0);
	rn_dispPrintf(device, 0, "%s", spaces);
}


//***************************************************************************
//	Function to display the user instruction for creating a message from
// the keypad.
//***************************************************************************
void display_help(int device)
{
	auto int i, loop, row;
	auto int wKey, helpMenuDone;

	// Copy help messages to a single array
	buffer[0] = '\0';
	for(i = 0; i<(sizeof(help)/2); i++)
	{
		strcat(buffer, help[i]);
	}

	// Display help message until the user presses a key
	helpMenuDone = FALSE;
	i=0;
	while(!helpMenuDone)
	{
   	costate
      {
		   rn_dispGoto(device, 0, 2, 0);
   	   rn_dispPrintf (device, 0, "%s", help[i++]);
			waitfor(DelayMs(1000));
	      if (i>4)
   	   	i = 0;
      }

      costate
      {
	      rn_keyProcess (device, 0);
			if(rn_keyGet(device, 0) != 0)
				helpMenuDone = TRUE;
		}
	}
}


//***************************************************************************
//	Message Menu used to create ASCII strings from the keypad
//***************************************************************************
void enter_chars(int device, char *p)
{
	int i, column, len, disp_col, disp_row;
	int wKey;
	char *orig_ptr;

	// Initialize function parameters
	orig_ptr = p;
	i = 0;
	column = disp_col = 0;
	len = 0;
   disp_row = 2;

	// Display the initial charater set for the user to choose from
   rn_dispClear(device, 0);
	rn_dispGoto(device, 0, 1, 0);
	rn_dispPrintf(device, 0, "-");
	rn_dispGoto(device, 0, 0, 0);
	rn_dispPrintf(device, 0, "%s", s[i]);

	// Display user instructions
	display_help(device);

	fast_configKeypad2x6(device);

	// Clear only the bottom half of the display
	rn_dispGoto(device, 0, 2, 0);
	rn_dispPrintf(device, 0, "                    ");
	rn_dispGoto(device, 0, 3, 0);
	rn_dispPrintf(device, 0, "                    ");

	// Set row 1 to start at column 0
	rn_dispGoto(device, 0, 1, 0);
	rn_dispPrintf(device, 0, "-");
	rn_dispGoto(device, 0, 2, 0);

	do
	{

		// Wait for a key to be pressed
		do
		{
			rn_keyProcess(device, 0);
			msDelay(50);
			wKey = rn_keyGet(device, 0);
		} while(wKey == 0);

		switch(wKey)
		{
			// Scroll-Down to select new character group
			case 'v':
				i = i < (sizeof(s)/2)-1 ? ++i : 0;
				clrline(device, 1, len);
				rn_dispGoto(device, 0, 1, 0);
  				rn_dispCursor(device, RNDISP_CURBLINKOFF, 0);
				rn_dispPrintf(device, 0, "-");
				column = 0;
				clrline(device, 0, strlen(*s));
				rn_dispGoto(device, 0, 0, 0);
				rn_dispPrintf(device, 0, "%s", s[i]);
				break;

			// Scroll-Up to select new character group
			case '^':
				i = i > 0 ? --i : 0;
				clrline(device, 1, len);
				rn_dispGoto(device, 0, 1, 0);
  				rn_dispCursor(device, RNDISP_CURBLINKOFF, 0);
				rn_dispPrintf(device, 0, "-");
				column = 0;
				clrline(device, 0, strlen(*s));
				rn_dispGoto(device, 0, 0, 0);
				rn_dispPrintf(device, 0, "%s", s[i]);
				break;

			// Scroll-Right for character set
			case '>':
				column = column < strlen(*(&s[i]))-1 ? ++column : 0;
				clrline(device, 1, len = strlen(*(&s[i])));
				rn_dispGoto(device, column, 1, 0);
  				rn_dispCursor(device, RNDISP_CURBLINKOFF, 0);
				rn_dispPrintf(device, 0, "-");
				break;

			// Scroll-Left for character set
			case '<':
				column = column > 0  ? --column : 0;
				clrline(device, 1, len = strlen(*(&s[i])));
				rn_dispGoto(device, column, 1, 0);
  				rn_dispCursor(device, RNDISP_CURBLINKOFF, 0);
				rn_dispPrintf(device, 0, "-");
				break;

			// Add the selected character to the message
			case '+':			// select char
				*p = s[i][column];
				rn_dispGoto(device, disp_col, disp_row, 0);
			  	rn_dispCursor(device, RNDISP_CURBLINKON, 0);
				rn_dispPrintf(device, 0, "%c", *p++);
				rn_dispGoto(device, ++disp_col, disp_row, 0);
				break;

         // Add space character
         case ' ':
				*p = ' ';
				rn_dispGoto(device, disp_col, disp_row, 0);
			  	rn_dispCursor(device, RNDISP_CURBLINKON, 0);
				rn_dispPrintf(device, 0, "%c", *p++);
				rn_dispGoto(device, ++disp_col, disp_row, 0);
				break;

			// Do a Backspace in the message
			case '-':
				rn_dispGoto(device, disp_col, disp_row, 0);
			  	rn_dispCursor(device, RNDISP_CURBLINKON, 0);
				if (disp_col > 0)
				{
					p--;
				}

				rn_dispGoto(device, --disp_col, disp_row, 0);
				rn_dispPrintf(device, 0, " ");
				rn_dispGoto(device, disp_col, disp_row, 0);
				if(disp_col < 0)
				{
					disp_col = 0;
					p = orig_ptr;
				}
				break;
		}
	} while(wKey != 'E');

	// NULL the terminate the user message
	*p = '\0';
  	rn_dispCursor(device, RNDISP_CURBLINKOFF, 0);

	// Set the keypad back to the default driver configuration
	configKeypad2x6(device);
}

//***************************************************************************
//	Mainline
//***************************************************************************
void main (	void	)
{
	auto char entry[256];

	auto int wKey, i, loop;
	auto int helpMenuDone;
 	auto int device0;
   auto rn_search newdev;

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
   rn_keyInit(device0, KEYSTROBLINES, 10);	//key press buzzer for 10ms
	configKeypad2x6(device0);	// Set keys to the default driver configuration
	rn_dispInit(device0, DISPROWS, DISPCOLS);
	rn_dispBacklight(device0, ON, 0);		//turn on backlight

	while(1)
	{
		// Display user prompt for the message menu
		rn_dispClear(device0, 0);
		rn_dispGoto(device0, 0, 0, 0);
		rn_dispPrintf(device0, 0, "Press + to create a message...");

		// Wait for ENTER key to be pressed
		do
		{
			rn_keyProcess(device0, 0);
			wKey = rn_keyGet(device0, 0);
		} while(wKey != '+');

		// Go to the message menu
		rn_dispClear(device0, 0);
		memset(entry,0,sizeof(entry));
		enter_chars(device0, entry);

		// Display the message the user typed
		rn_dispClear(device0, 0);
		rn_dispGoto(device0, 0, 0, 0);
		rn_dispPrintf(device0, 0, "Typed...%s", entry);

		// Wait for user to press any key to startover
		rn_dispGoto(device0, 0, 3, 0);
		rn_dispPrintf(device0, 0, "ENTER to Continue");
		do
		{
			rn_keyProcess(device0, 0);
			wKey = rn_keyGet(device0, 0);
		} while(wKey == 0);
	}
}
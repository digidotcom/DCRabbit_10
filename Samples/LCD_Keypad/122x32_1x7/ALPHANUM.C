/***************************************************************************
	alphanum.c

	Rabbit Semiconductor, 2001
	Sample program to demonstrate how you can create messages with the
	keypad and then display them on the LCD display.

   NOTE: Not currently supported on RCM4xxx modules.

	Instructions:
	1. Compile and run this program.
	2. Follow the instructions on the LCD display.

***************************************************************************/
#class auto

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

#memmap xmem  // Required to reduce root memory usage

// Character set used to create a message
const char * const s[] = { "ABCDEFGHIJKLM_",
					     "NOPQRSTUVWXYZ_",
						  "0123456789_",
						  "!@#\\\"/-+&$%^_",
						  "&*()_=_"};

// Instructions for the message menu
const char * const help[] = {"Use Scrolling keys for character selection... ",
							 "Use PLUS key to add a character to your message... ",
							 "Use MINUS key for a backspace... ",
							 "Use last character in string for a SPACE... ",
							 "Use ENTER key to exit message menu... "};

// 6x8 character font set structure
fontInfo fi6x8;

// Text windowframe structures
windowFrame textWindow1, textWindow2;

// General use buffer
char buffer[1024];


//***************************************************************************
//	General use MS delay function
//***************************************************************************
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + (unsigned long)delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


//***************************************************************************
//	Clear a single text line on the LCD display.
//***************************************************************************
void clrline(int line, int len)
{
	char spaces[256];
	int i;

	for(i=0; i < len; i++)
	{
		spaces[i] = ' ';
	}
	spaces[i] = '\0';
	TextGotoXY(&textWindow1, 0, line);
	TextPrintf(&textWindow1, "%s", spaces);
}


//***************************************************************************
//	Function to display the user instruction for creating a message from
// the keypad.
//***************************************************************************
void display_help(void)
{
	auto int i, loop;
	auto int wKey, helpMenuDone;

	// Copy help messages to a single array
	buffer[0] = '\0';
	for(i = 0; i<(int)(sizeof(help)/2); i++)
	{
		strcat(buffer, help[i]);
	}

	// Display help message until the user presses a key
	helpMenuDone = FALSE;
	while(!helpMenuDone)
	{
		i=0;
		while(buffer[i] != '\0' && !helpMenuDone)
		{
			glHScroll(0, 16, LCD_XS, 16, -6);
			glPrintf(116,  20,   &fi6x8, "%c", buffer[i++]);

			// 250ms delay loop, checking for key press every msec
			for(loop=0; loop < 250; loop++)
			{
				msDelay(1);
				keyProcess ();
				if(keyGet() != 0)
				{
					helpMenuDone = TRUE;
				}
			}
		}
	}
}


//***************************************************************************
//	Message Menu used to create ASCII strings from the keypad
//***************************************************************************
void enter_chars(char *p)
{
	int i, column, len, disp_col, disp_row;
	int wKey;
	char *orig_ptr;

	// Initialize function parameters
	orig_ptr = p;
	i = 0;
	column = 0;
	len = 0;

	// Display the initial charater set for the user to choose from
	glBlankScreen();
	TextGotoXY(&textWindow1, 0, 1);
	TextPrintf(&textWindow1, "-");
	TextGotoXY(&textWindow1, 0, 0);
	TextPrintf(&textWindow1, "%s", s[i]);

	// Display user instructions
	display_help();

	// Setup for FAST key repeat after holding down key for 12 ticks
	keyConfig (  6,'E',0, 12, 1,  1, 1 );
	keyConfig (  2,'D',0, 12, 1,  1, 1 );
	keyConfig (  5,'+',0, 12, 1,  1, 1 );
	keyConfig (  1,'U',0, 12, 1,  1, 1 );
	keyConfig (  4,'-',0, 12, 1,  1, 1 );

	keyConfig (  0,'L',0, 12, 1,  1, 1 );
	keyConfig (  3,'R',0, 12, 1,  1, 1 );


	// Clear only the bottom half of the display
	glSetBrushType(PIXWHITE);
	glBlock(0,16,122,16);
	glSetBrushType(PIXBLACK);

	// Set window2 to start at column 0
	TextGotoXY(&textWindow2, 0, 0);
	TextPrintf(&textWindow2, "_");
	TextGotoXY(&textWindow2, 0, 0);

	do
	{

		// Wait for a key to be pressed
		do
		{
			keyProcess();
			msDelay(50);
			wKey = keyGet();
		} while(wKey == 0);

		switch(wKey)
		{
			// Scroll-Down to select new character group
			case 'D':
				i = i < (int)(sizeof(s)/2)-1 ? ++i : 0;
				clrline(1, len);
				TextGotoXY(&textWindow1, 0, 1);
				TextPrintf(&textWindow1, "-");
				column = 0;
				clrline(0, strlen(*s));
				TextGotoXY(&textWindow1, 0, 0);
				TextPrintf(&textWindow1, "%s", s[i]);
				break;

			// Scroll-Up to select new character group
			case 'U':
				i = i > 0 ? --i : 0;
				clrline(1, len);
				TextGotoXY(&textWindow1, 0, 1);
				TextPrintf(&textWindow1, "-");
				column = 0;
				clrline(0, strlen(*s));
				TextGotoXY(&textWindow1, 0, 0);
				TextPrintf(&textWindow1, "%s", s[i]);
				break;

			// Scroll-Right for character set
			case 'R':
				column = column < strlen(*(&s[i]))-1 ? ++column : 0;
				clrline(1, len = strlen(*(&s[i])));
				TextGotoXY(&textWindow1, column, 1);
				TextPrintf(&textWindow1, "-");
				break;

			// Scroll-Left for character set
			case 'L':
				column = column > 0  ? --column : 0;
				clrline(1, len = strlen(*(&s[i])));
				TextGotoXY(&textWindow1, column, 1);
				TextPrintf(&textWindow1, "-");
				break;

			// Add the selected character to the message
			case '+':			// select char
				*p = s[i][column];
				if(*p == '_' && column == strlen(*(&s[i]))-1 )
				{	// Change to a space
					*p = ' ';
				}
				TextCursorLocation(&textWindow2, &disp_col, &disp_row);
				TextPrintf(&textWindow2, "%c", *p++);
				TextPrintf(&textWindow2, "_");
				TextGotoXY(&textWindow2, disp_col+1, 0);
				break;

			// Do a Backspace in the message
			case '-':
				TextGotoXY(&textWindow2, disp_col, disp_row);
				TextPrintf(&textWindow2, "  ");
				TextGotoXY(&textWindow2, disp_col, disp_row);
				TextPrintf(&textWindow2, "_ ");
				if(disp_col > 0)
				{
					p--;
				}
				TextGotoXY(&textWindow2, disp_col--, disp_row);
				if(disp_col < 0)
				{
					disp_col = 0;
					p = orig_ptr;
				}
				break;
		}
	}while(wKey != 'E');

	// NULL the terminate the user message
	*p = '\0';

	// Set the keypad back to the default driver configuration
	keypadDef();
}


//***************************************************************************
//	Mainline
//***************************************************************************
void main (	void	)
{
	auto char entry[256];

	auto int wKey, i, loop;
	auto int helpMenuDone;

	//------------------------------------------------------------------------
	// Board and drivers initialization
	//------------------------------------------------------------------------
	brdInit();		// Required for all controllers
	dispInit();		// Graphic driver initialization, Start-up the keypad driver
	keypadDef();	// Set keys to the default driver configuration

	//------------------------------------------------------------------------
	// Font initialization
	//------------------------------------------------------------------------
	// Initialize structures with FONT bitmap information
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);				//	initialize basic font

	//------------------------------------------------------------------------
	// Text window initialization
	//------------------------------------------------------------------------
	// Setup the widow frame to be the entire LCD to display information
	TextWindowFrame(&textWindow1, &fi6x8, 0, 0, 122, 32);
	TextWindowFrame(&textWindow2, &fi6x8, 0, 24, 122, 8);

	//------------------------------------------------------------------------
	// Main loop for the user to create messages from the keypad
	//------------------------------------------------------------------------
	while(1)
	{
		// Display user prompt for the message menu
		glBlankScreen();
		TextGotoXY(&textWindow1, 0, 0);
		TextPrintf(&textWindow1, "Press + to create a message...\n");

		// Wait for ENTER key to be pressed
		do
		{
			keyProcess();
			wKey = keyGet();
		} while(wKey != '+');

		// Go to the message menu
		glBlankScreen();
		memset(entry,0,sizeof(entry));
		enter_chars(entry);

		// Display the message the user typed
		glBlankScreen();
		TextGotoXY(&textWindow1, 0, 0);
		TextPrintf(&textWindow1, "Typed...%s", entry);

		// Wait for user to press any key to startover
		TextGotoXY(&textWindow1, 0, 3);
		TextPrintf(&textWindow1, "ENTER to Continue");
		do
		{
			keyProcess();
			wKey = keyGet();
		} while(wKey == 0);
	}
}
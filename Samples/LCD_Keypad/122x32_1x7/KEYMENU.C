/***************************************************************************
	keymenu.c
	Rabbit Semiconductor, 2001

	This sample program is for the LCD MSCG12232 Display Module.

   NOTE: Not currently supported on RCM4xxx modules.

  	This program demonstrates how to implement a menu system with using a
  	highlight bar on a graphic LCD display. The menu options for this sample
  	are as follows:

  	1.	Set Date & Time
  	2.	Display Date/Time
  	3. Backlight menu
  	4. Toggle LEDS
  	5. Increment LEDS
  	6. Disable LEDS

  	To select a option use the scroll keys(Scroll and/or Page UP/DOWN keys)
  	to highlight the option that you want to select and then press the ENTER
  	key.

	Once the option is selected the operation will be completed or you
	will be prompted to do additional steps to complete the option
	selected.


	Development Notes:
	------------------
	1. Menu options can be added/deleted and the highlight bar will automatically
	adjust to the new menu list. This will also require that you add/delete case
	statements in main() to match your menu list.

	2. Re-designed the display_menu function to allow for sub_menus.

**************************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

//----------------------------------------------------------
// Macro's for MENU system....to be set for your application
//----------------------------------------------------------
#define NUM_MENUS			2  // Used to set the size of a structure
                           // array to hold menu information, set
                           // to actual number of menus that you
                           // are going to be using.

// Define the unique menu number...is used for an index into the menu
// structure array.
#define LVL_MAINMENU		0  // Top level menu
#define LVL_BACKLIGHT   1  // Sub-level #1

//----------------------------------------------------------
// Menu options........set as needed for your application
//----------------------------------------------------------
// Can insert/delete menu options. The highlight bar is setup
// to start with the first MENU option and stop at the last
// menu option in the MENU.
//
// When adding/deleting menu options you must match up the
// case statements to the menu option number.
//

const char *main_menu [] =
{		" <<<<Main Menu>>>>",
		"1.Set Date & Time",
		"2.Display Date/Time",
		"3.Backlight menu",
		"4.Toggle LED's",
		"5.Increment LED's",
		"6.Disable LED's",
		""
};

const char *backlight [] =
{		"<<<Backlight Menu>>>",
		"1.Turn Backlight OFF",
		"2.Turn Backlight ON",
		"3.Exit Menu",
		""
};

// Do auto calculation of the number of menu options for
// a given MENU.
#define NUM_MAINMENU_OPTS sizeof(main_menu)/sizeof(int)
#define NUM_BACKLIGHT_OPTS sizeof(backlight)/sizeof(int)

//----------------------------------------------------------
// START.....Macros and Structure for menu system
// !!! The following section is not intended to be changed.
//----------------------------------------------------------
#define MENU_INIT 				0
#define MENU_NO_CHANGE	 		1
#define MENU_REFRESH     		2

struct menu_infor
{
	int current_offset;
	int new_offset;
	int lasthighlight;
	int highlight;
};

struct menu_infor menu[NUM_MENUS];
//----------------------------------------------------------
// END.....Macros and Structure for menu system
//----------------------------------------------------------



//---------------------------------------------------------
// Bitmaps
//---------------------------------------------------------
// Bitmap : Zwbw5_bmp
// Buffer Size : 203
// Monochrome  : White Foreground, Black Background
// Mode   : Landscape
// Height : 29 pixels.
// Width  : 53 pixels.
// Init   : glXPutBitmap (leftedge,topedge,53,29,Zwbw5_bmp);

xdata Zwbw5_bmp {
'\x00','\x00','\x1F','\xF8','\x00','\x00','\x07',
'\x00','\x00','\xFF','\xFF','\x80','\x00','\x07',
'\x00','\x03','\xF3','\xF0','\xE0','\x00','\x07',
'\x00','\x0F','\xFF','\xE0','\x38','\x00','\x07',
'\x00','\x3F','\x0F','\xFC','\x0E','\x00','\x07',
'\x00','\x7E','\x1E','\x07','\x83','\x00','\x07',
'\x00','\xFC','\x38','\x01','\xE1','\x80','\x07',
'\x01','\xB8','\x30','\x00','\x38','\xC0','\x07',
'\x03','\x30','\x70','\x00','\x0C','\x60','\x07',
'\x06','\x60','\xE0','\x00','\x03','\x70','\x07',
'\x0E','\x60','\xC0','\x00','\x01','\xF0','\x07',
'\x0C','\xC1','\x80','\x00','\x00','\xE8','\x07',
'\x1F','\xFF','\xE0','\x00','\x00','\x2C','\x37',
'\x3F','\x83','\x7F','\x00','\x00','\x34','\x1F',
'\x31','\x03','\x01','\xE0','\x00','\x2A','\x3F',
'\x31','\x02','\x00','\x1C','\x00','\x26','\x67',
'\x62','\x06','\x00','\x07','\x00','\x27','\xC7',
'\x62','\x04','\x00','\x01','\x80','\x27','\x07',
'\x00','\x00','\x00','\x00','\x00','\x70','\x07',
'\x00','\x00','\x00','\x00','\x03','\x80','\x07',
'\x7C','\x21','\x0D','\xF9','\xF8','\x7C','\x07',
'\x08','\x31','\x8B','\x0F','\x84','\x63','\x07',
'\x18','\x11','\x8F','\xE1','\x14','\x61','\x07',
'\x11','\x93','\xF0','\x05','\x34','\x61','\x07',
'\x31','\xC0','\x54','\x07','\xE4','\x61','\x07',
'\x20','\x0A','\x56','\x05','\x44','\x61','\x07',
'\x40','\x0C','\x62','\x05','\x24','\x63','\x07',
'\x40','\x04','\x21','\x99','\x34','\x6E','\x07',
'\xFC','\x04','\x20','\xF1','\x17','\xF8','\x07'
};

//---------------------------------------------------------
// Macro's
//---------------------------------------------------------
#define MAXDISPLAYROWS	4
#define LEDOFF				0
#define TOGGLE				1
#define INCREMENT			2
#define OPERATE			3

#define ASCII				0
#define NUMBER				1

//----------------------------------------------------------
// Structures, arrays, variables
//----------------------------------------------------------
fontInfo fi6x8, fi8x10, fi12x16;
windowFrame textWindow;

typedef struct  {
	int data;
	char *ptr;
} fieldupdate;

struct tm CurTime;

char szTime[40];
char szString[20];


int ledCntrl;
int beeperTick, timerTick ;
int max_menu_options;
int max_cmds_options;
unsigned long ulTime;
char *keybuffer;


//------------------------------------------------------------------------
// Milli-sec delay function
//------------------------------------------------------------------------
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


//------------------------------------------------------------------------
// Process key to do number and ASCII field changes
//------------------------------------------------------------------------
int ProcessKeyField(int mode, fieldupdate *field)
{
	static int wKey;

	keyProcess();
	msDelay(100);
	if((wKey = keyGet()) != 0)
	{
		switch(wKey)
		{
			// Decrement number by 10 or pointer by 3
			case '-':
				if(mode == NUMBER)
					field->data -= 10;
				else
					field->ptr  -= 3;
				break;

			// Increment number by 10 or pointer by 3
			case '+':
				if(mode == NUMBER)
					field->data += 10;
				else
					field->ptr  += 3;
				break;

			// Increment number or pointer by 1
			case 'U':
				if(mode == NUMBER)
					field->data++;
				else
					field->ptr++;
				break;

			// Decrement number or pointer by 1
			case 'D':	// Decrement X1
				if(mode == NUMBER)
					field->data--;
				else
					field->ptr--;
				break;

			// Done Editing field
			case 'E':
				wKey = 'E';
				break;

			default:
				wKey = -1;
		  		break;
		}
	}
	return(wKey);
}

//------------------------------------------------------------------------
// Get and process the users MENU option
//------------------------------------------------------------------------
int GetKeypadOption(int *offset, int *highlight, int num_options )
{
	static int wKey;

	if((wKey = keyGet()) != 0)
	{
		switch(wKey)
		{
			case '-':	// Page down
				if(*offset < (num_options - 1))
				{
					if((*offset + MAXDISPLAYROWS) < (num_options - 1))
						*offset += 4;
				}
				if(*offset == 0)
					*highlight = 1;
				else
					*highlight = 0;
				wKey = -1;
				break;

			case '+':	// Page up
				if(*offset > 3)
					*offset -=4;
				else
					*offset = 0;
				if(*offset == 0)
					*highlight = 1;
				else
					*highlight = 0;
				wKey = -1;
				break;

			case 'U':	// Scroll-up by one line
				*highlight -= 1;
				if(*highlight < 0)
				{
					*offset -= 1;
					*highlight = 0;
				}
				if(*offset == 0 && *highlight == 0)
					*highlight = 1;
				wKey = -1;
				break;

			case 'D':	// Scroll-down by one line
				if((*offset + (*highlight) + 1) < (num_options - 1))
				{
					*highlight += 1;
					if(*highlight > 3)
					{
						*offset += 1;
						*highlight = 3;
					}
				}
				wKey = -1;
				break;

			case 'E':	// Select option
				wKey = *offset + *highlight;
				break;

			default:
				wKey = -1;
		  		break;
		}
	}
	return(wKey);
}

/* --------------------------------------------------------------------------------------

SYNTAX:	      int display_menu(char **line, int *state, int menu_num, int num_options)

DESCRIPTION:   Display a MENU on the LCD display and get the menu option from the user


PARAMETER1:    Pointer to list of menu options
PARAMETER2:    Pointer to Menu control parameter, control parameters macro's are as follows:
	               MENU_INIT..........Initialize and Display Menu
	               MENU_NO_CHANGE.....Select option, No menu/highlight bar changes occur.
                  MENU_REFRESH.......Display last image of menu of where it was.
PARAMETER3:    Unique menu number, starting with 0 and on up.
PARAMETER4:    The number of options for a given menu.

RETURN VALUE:	0 if no option is selected, otherwise it be the option selected

-----------------------------------------------------------------------------------------*/
int display_menu ( char **line, int *state, int level, int num_options)
{
	auto int menu_option;
	auto int i;
	auto struct menu_infor *ptr;

	ptr = &menu[level];
	menu_option = 0;			// Initially set to no option selected
	switch(*state)
	{
		case 0: // Set menu parameters in menu structure array
			ptr->current_offset = 0;		// Initialize menu line index
			ptr->highlight = 1;				// Assumes all menus have a heading
			ptr->new_offset = !ptr->current_offset;
			keyProcess ();

			// Make sure no key is being pressed initially
			if(keyGet() == 0)
			{
				*state = 4;
				break;
			}
			break;

		case 1: 	// Get the user's option
			menu_option = GetKeypadOption(&ptr->current_offset, &ptr->highlight, num_options);
			if(menu_option == -1)
			{
				// Check if user selected the scrolling option
				glSetBrushType(PIXXOR);
				glBlock (0, ptr->lasthighlight*8, 122, 8);
				glSetBrushType(PIXBLACK);

				// Set menu option to zero due to scrolling operation
				menu_option = 0;
				*state = 4;
			}
			break;

		case 2: // Refresh menu options
			glBuffLock();
			glBlankScreen();
			for(i=0; i < 4; i++)
			{	// Display up to 4 lines of menu options
				TextGotoXY(&textWindow, 0, i);
				TextPrintf(&textWindow, "%s", line[ptr->current_offset]);
				if(*line[ptr->current_offset + 1] == '\0')
				{
					break;
				}
				ptr->current_offset++;
			}
			// Reset the offset back to the first option displayed
			ptr->current_offset = ptr->current_offset-i;
			ptr->new_offset = ptr->current_offset;
			glSetBrushType(PIXXOR);
			glBlock (0, ptr->highlight*8, 122, 8);
			glSetBrushType(PIXBLACK);
			glBuffUnlock();
			ptr->lasthighlight = ptr->highlight;
			*state = 1;
			break;


		case 4: // Display menu option
			if(ptr->current_offset != ptr->new_offset)
			{
				glBuffLock();
				glBlankScreen();
				for(i=0; i < 4; i++)
				{	// Display up to 4 lines of menu options
					TextGotoXY(&textWindow, 0, i);
					TextPrintf(&textWindow, "%s", line[ptr->current_offset]);
					if(*line[ptr->current_offset + 1] == '\0')
					{
						break;
					}
					ptr->current_offset++;
				}
				glBuffUnlock();
				// Reset the offset back to the first option displayed
				ptr->current_offset = ptr->current_offset-i;
				ptr->new_offset     = ptr->current_offset;
			}
			glSetBrushType(PIXXOR);
			glBlock (0, ptr->highlight*8, 122, 8);
			glSetBrushType(PIXBLACK);
			ptr->lasthighlight = ptr->highlight;
			*state = 1;
			break;

		default:
			*state = 0;
			break;
	}
	return(menu_option);
}

//------------------------------------------------------------------------
// Format the Date and Time for the LCD display
//------------------------------------------------------------------------
void FormatDateTime ( void )
{
	ulTime = read_rtc ();			// get the RTC value
	mktm( &CurTime, ulTime );		// convert seconds to date values

	strftime( szTime, sizeof szTime, "%a %b %e, %Y\n%H:%M:%S", &CurTime);
}

//------------------------------------------------------------------------
// Display the Date and Time on the LCD display
//------------------------------------------------------------------------
int dispDate( void )
{
	static int status;
	auto int wKey;

	costate
	{
		// Get current Date/Time
		status = 0;
		ulTime = read_rtc ();			// get the RTC value
		mktm( &CurTime, ulTime );		// convert seconds to date values
		FormatDateTime();					// convert to text
		waitfor(DelayMs(5));

		// Display Date and Time
		glBuffLock();
		TextGotoXY(&textWindow, 0, 0);
		TextPrintf(&textWindow, "%s\n", szTime);
		waitfor(DelayMs(5));

		// Display user exit message
		TextGotoXY(&textWindow, 0, 3);
		TextPrintf(&textWindow, "Press Key to EXIT");
		waitfor(DelayMs(5));
		glBuffUnlock();

		// Wait for key to be pressed to exit
		waitfor(((wKey = keyGet()) != 0) || DelayMs(100));
		if(wKey != 0)
		{
			glBlankScreen();
			status = 1;
		}
	}
	return(status);
}


//------------------------------------------------------------------------
// LED control function
//------------------------------------------------------------------------
void leds( int mode )
{
	static int toggle, increment;
	auto int led, mask;

	#GLOBAL_INIT {toggle=0;}
	#GLOBAL_INIT {increment=0;}

	if(mode != OPERATE)
	{
		ledCntrl = mode;
		toggle = 0;
		increment = 0;
		return;
	}

	if(ledCntrl == TOGGLE)
	{
		toggle = (~toggle) & 0x01;
		for(led = 0; led <= 6; led++)
		{
			//Toggle the LED's
			dispLedOut(led, toggle);
		}
	}
	else if(ledCntrl == INCREMENT)
	{
		mask = 0x01;
		increment++;
		for(led = 0; led <= 6; led++)
		{

			if(increment & mask)
				dispLedOut(led, 1);
			else
				dispLedOut(led, 0);
			mask = mask << 1;
		}
	}
	else
	{	//Turn all LED'S OFF
		for(led = 0; led <= 6; led++)
			dispLedOut(led, 0);
	}
}


//------------------------------------------------------------------------
// Date and Time prompt message routine
//------------------------------------------------------------------------
void date_prompt(char *ptr, int *col, int *row)
{

	glBlankScreen();
	TextGotoXY(&textWindow, 0, 0);
	TextPrintf(&textWindow, "%s", ptr);
	TextCursorLocation(&textWindow, col, row);

	TextGotoXY(&textWindow, 0, 3);
	TextPrintf(&textWindow, "ENTER to Continue...");
}


//------------------------------------------------------------------------
// Set Date and Time
//------------------------------------------------------------------------
void SetDateTime( void )
{
	int wKey;
	int col, row;
	char buffer[256];
	fieldupdate dateTime;

	// Setup for FAST key repeat after holding down key for 12 ticks
	keyConfig (  6,'E',0, 12, 1,  1, 1 );
	keyConfig (  2,'D',0, 12, 1,  1, 1 );
	keyConfig (  5,'+',0, 12, 1,  1, 1 );
	keyConfig (  1,'U',0, 12, 1,  1, 1 );
	keyConfig (  4,'-',0, 12, 1,  1, 1 );

	date_prompt("Select \n4 digit year: ", &col, &row);
	dateTime.data = 2001;
	while(1)
	{
		sprintf(buffer, "%04d", dateTime.data);
		TextGotoXY(&textWindow, col, row);
		TextPrintf(&textWindow, "%s", buffer);
		while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
		if(dateTime.data < 1900 || dateTime.data > 2047)
		{
			dateTime.data = 2001;
		}
		if(wKey == 'E')
		{
			if( dateTime.data  >= 1900 && dateTime.data < 2048) {
				CurTime.tm_year = dateTime.data - 1900;	// offset from 1900
				break;
			}
		}
	}

	date_prompt("Enter month: ", &col, &row);
	dateTime.data = 1;
	while(1)
	{
		sprintf(buffer, "%02d", dateTime.data);
		TextGotoXY(&textWindow, col, row);
		TextPrintf(&textWindow, "%s", buffer);
		while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
		if(wKey == 'E')
		{
			if( dateTime.data >= 1 && dateTime.data < 13 )
			{
				CurTime.tm_mon = month2tm_mon(dateTime.data);
				break;
			}
		}
		if(dateTime.data < 1 || dateTime.data > 12)
		{
			dateTime.data  = (dateTime.data < 1) ? 12 : 1;
		}
	}

	date_prompt("Enter \nday of month: ", &col, &row);
	dateTime.data = 1;
	while(1)
	{
		sprintf(buffer, "%02d", dateTime.data);
		TextGotoXY(&textWindow, col, row);
		TextPrintf(&textWindow, "%s", buffer);
		while((wKey = ProcessKeyField(NUMBER, &dateTime))== 0);
		if(wKey == 'E')
		{
			if( dateTime.data  >= 1 && dateTime.data < 32) {
				CurTime.tm_mday = dateTime.data;
				break;
			}
		}
		if(dateTime.data < 1 || dateTime.data > 31)
		{
			dateTime.data  = (dateTime.data < 1) ? 31 : 1;
		}
	}


	date_prompt("Enter \nhour (24hr): ", &col, &row);
	dateTime.data = 0;
	while(1)
	{
		sprintf(buffer, "%02d", dateTime.data);
		TextGotoXY(&textWindow, col, row);
		TextPrintf(&textWindow, "%s", buffer);
		while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
		if(wKey == 'E')
		{
			if(dateTime.data >= 0 && dateTime.data < 24) {
				CurTime.tm_hour = dateTime.data;
				break;
			}
		}
		if(dateTime.data < 0 || dateTime.data > 23)
		{
			dateTime.data  = (dateTime.data < 0) ? 23 : 0;
		}
	}

	date_prompt("Enter minute: ", &col, &row);
	dateTime.data = 0;
	while(1)
	{
		sprintf(buffer, "%02d", dateTime.data);
		TextGotoXY(&textWindow, col, row);
		TextPrintf(&textWindow, "%s", buffer);
		while((wKey = ProcessKeyField(NUMBER, &dateTime)) == 0);
		if(wKey == 'E')
		{
			if( dateTime.data >= 0 && dateTime.data < 60) {
				CurTime.tm_min = dateTime.data;
				break;
			}
			if(wKey == 'E')
			{
				break;
			}
		}
		if(dateTime.data < 0 || dateTime.data > 59)
		{
			dateTime.data  = (dateTime.data < 0) ? 59 : 0;
		}
	}

	CurTime.tm_sec = 0;
	ulTime = mktime ( &CurTime );		// get seconds from 1/1/1980
	write_rtc ( ulTime );				// set the real time clock
	keypadDef();

	glBlankScreen();
	while(1)
	{
		// Get current Date/Time
		FormatDateTime();					// convert to text

		// Display Date and Time
		glBuffLock();
		TextGotoXY(&textWindow, 0, 0);
		TextPrintf(&textWindow, "%s\n", szTime);

		// Display user exit message
		TextGotoXY(&textWindow, 0, 3);
		TextPrintf(&textWindow, "Press Key to EXIT    ");
		glBuffUnlock();

		keyProcess ();
		if((wKey = keyGet()) != 0)
		{
			glBlankScreen();
			break;
		}
	}
}

//------------------------------------------------------------------------
// Display Sign-on message
//------------------------------------------------------------------------
void SignOnMessage(void)
{
	auto int signMesgDone, i, loop;
	auto char buffer[256];

	// Display Sign-on Message then wait for any key to continue
	glXPutBitmap (0,0,53,29,Zwbw5_bmp);
	msDelay(500);

	signMesgDone = FALSE;
	while(!signMesgDone)
	{
		i=0;
		sprintf(buffer, "Hello from Zworld!!!");
		strcat(buffer, "     Press any KEY to Continue...      ");
		while(buffer[i] != '\0' && !signMesgDone)
		{
			glHScroll(50, 0, LCD_XS, 16, -6);
			glPrintf (116,  4,   &fi6x8, "%c", buffer[i++]);
			for(loop=0; loop < 165; loop++)
			{
				msDelay(1);
				keyProcess ();
				if(keyGet() != 0)
				{
					signMesgDone = TRUE;
				}
			}
		}
	}
}

//------------------------------------------------------------------------
// Routine for sub_menu backlight options
//------------------------------------------------------------------------
int backlight_menu( void )
{
	static int done;
	static int state;
	auto int option;

	costate
	{
		done = FALSE;
		state = MENU_INIT;
		while(!done)
		{
			// Display the MAIN MENU
			waitfor((option = display_menu(backlight, &state, LVL_BACKLIGHT, NUM_BACKLIGHT_OPTS)) > 0);

			// Get menu option from the user
			switch(option)
			{

				// Turn Backlight OFF
				case 1:	glBackLight(0);
							state = MENU_NO_CHANGE;
							break;

				// Turn Backlight ON
				case 2:	glBackLight(1);
							state = MENU_NO_CHANGE;
							break;

				case 3:
					  	 	done = TRUE;
			       		break;

				// User made invalid selection
				default:
					break;
			}
		}
	}
	return(done);
}

//------------------------------------------------------------------------
// Sample program to demonstrate the LCD and keypad
//------------------------------------------------------------------------
void main (	void	)
{
	auto int option;
	static int state;

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------
	brdInit();			// Initialize the controller
	dispInit();			// Start-up the keypad driver, Initialize the graphic driver
	keypadDef();		// Use the default keypad ASCII return values


	glBackLight(1);	// Turn-on the backlight
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);			//	Initialize 6x8 font
	glXFontInit(&fi8x10, 8, 10, 32, 127, Font8x10);		//	Initialize 10x16 font
	glXFontInit(&fi12x16, 12, 16, 32, 127, Font12x16);	//	Initialize 12x16 font

	// Setup and center text window to be the entire display
	TextWindowFrame(&textWindow, &fi6x8, 1, 0, 121, 32);

	// Set variables to known states
	ledCntrl = LEDOFF;	// Initially disable the LED's
	state = MENU_INIT;
	//------------------------------------------------------------------------
	// Display Sign-on message and wait for keypress
	//------------------------------------------------------------------------
	SignOnMessage();

	//------------------------------------------------------------------------
	// Main program loop for the MENU system
	//------------------------------------------------------------------------
	for (;;)
	{
		costate
		{
			keyProcess ();
			waitfor(DelayMs(10));
		}

		costate
		{
			leds(OPERATE);
			waitfor(DelayMs(50));
		}
		costate
		{
			// Display the MAIN MENU
			waitfor((option = display_menu(main_menu, &state, LVL_MAINMENU, NUM_MAINMENU_OPTS)) > 0);


			// Get menu option from the user
			switch(option)
			{
				// Change Date/Time
				case 1:	glBlankScreen();
							SetDateTime();
							state = MENU_INIT;
							break;

				// Display current Date/Time
				case 2:	glBlankScreen();
							waitfor(dispDate());
							state = MENU_INIT;
							break;

				// Display backlight memu options
				case 3:	waitfor(backlight_menu());
							state = MENU_REFRESH;
							break;

				// Enable Toggle leds option
				case 4:	leds(TOGGLE);
							state = MENU_NO_CHANGE;
							break;

				// Enable Increment leds option
				case 5:	leds(INCREMENT);
							state = MENU_NO_CHANGE;
							break;

				// Disable LED's
				case 6:	leds(LEDOFF);
							state = MENU_NO_CHANGE;
							break;

				// User made invalid selection
				default:
					break;
			}
		}
	}
}
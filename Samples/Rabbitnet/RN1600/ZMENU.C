/***************************************************************************

	ZMENU.C
	Digi International, Copyright © 2005-2008.  All rights reserved.

	This sample program is intended for RN1600 RabbitNet Keypad/Display
   Interface card.

	DESCRIPTION
   ===========
   This sample demonstrates a menu system using an RN1600
	KDIF board.  The programmer can list a set of action
	options for the operator to choose from.  Keypads and
   character displays found in the development kit are used in
   this program.

	This sample incorporates ethernet connectivity only
	for the use of changing IP addresses, gateways, and
	netmasks.  No web browsing capabilities have been added.

   Note: Backlight function will work only on displays that have
   backlight capability.

   Menu Features
   =============
   This sample has 3 menus, a main menu, a data entry menu, and
	a TCPIP menu.  The main menu simply allows selection for the
	other two, as well as a selection for erasing the stdio window
   and turning backlight on and off.
	The data entry menu demonstrates data entry capability of
	the following:
	longs, floats, strings, passwords, and a time date stamp.
	The TCPIP menu demonstrates how to change ip addresses via
	the keypad.

	As selections are made, the current menu number and the
	selection made will display in the stdio window.  When
	a data entry or a tcp menu selection is made the appropriate
	values entered will also be displayed in the STDIO.

   Once downloaded, this sample can display all of the above
   information in Hyperterminal (Tera Term, or other serial port
   emulators), by moving the programming cable connector on the
   controller from PROG, to DIAG and recycling power.

   The setup for Hyperterminal is as follows:

               Baudrate:	57600
               DataBits:	8
               Parity:		None
               StopBits:	1
               Flow Control:	None


	Instructions
   ============
   1. Install the 2x6 keypad on J6. To ensure keypad driver
   	compatibility, the keypad must be installed so that a
      strobe line or data line starts on J6 pin 1.

	   2x6 keypad character assignment for this example:

		[  U  ] [  S  ] [  L  ] [  R  ] [     ] [     ]
		[  D  ] [  P  ] [  -  ] [  +  ] [     ] [  E  ]

		'U' scroll up up 1 menu option.
	   'D' scroll down 1 menu option.
   	'S' page up to the next set of menu items.
	   'P' page down to the next set of menu items.
   	'E' select the highlighted item.

	   'L' cursor left, Used in the data entry section for moving
   		 the cursor to the next character for selection.
	 	'R' cursor right, Used in the data entry section, for moving
   	    the cursor to the next character for selection.
	   '+' add item, Used in the data entry section, for selecting
   	 	 the character highlighted.
	   '-' delete item,	Used in the data entry section, for deleting
   	  	the last character selected.

   2. Install the 4x20 display onto J5.
	3. Compile and run this program.
   4. Press the scroll and paging keys to view items.  Press
   	enter key to select item.
	5. The LCD will display Menu Title at the top, followed by
		options selectable by the user.
   6. 3x4 and 4x6 (not in development kit) keypads can be used with
   	this example. Uncomment the appropriate ZMENU_KEYSTROBELINES
      macro in the configuration section below. Keys can be assigned
      by the function Zmenu_KeyConfig() to match a desired layout.
   7.	The macros ZMENU_COLUMNS and ZMENU_ROWS are used to define
		the size of the display and can be changed in the
      configuration section below.


END DESCRIPTION **********************************************************/

#class auto
/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

#memmap xmem
#use "dcrtcp.lib"

////////////////////////////
// Configuration section
////////////////////////////
#define 	ZMENU_KEYSTROBELINES 	0x00C0 //strobe lines for 2x6 keypad
//#define 	ZMENU_KEYSTROBELINES 	0x0070 //strobe lines for 3x4 keypad
//#define	ZMENU_KEYSTROBELINES    0x03C0 //strobe for 4x6 keypad
#define	ZM_MAIN_MENU				0		// Main Menu Number
#define	ZM_DATA_MENU				1     // Data Menu Number
#define	ZM_TCP_MENU					2     // TCP Menu Number
#define	ZMENU_COLUMNS				20 	// default columns for 20 x 4 LCD
#define	ZMENU_ROWS					4 		// default rows for 20 x 4 LCD
#define	ZMENU_SCROLL_UP			'U'	// keyvalue for scroll up
#define	ZMENU_SCROLL_DN			'D'	// keyvalue for scroll  down
#define	ZMENU_PAGE_UP				'S'	// keyvalue for page up
#define	ZMENU_PAGE_DN				'P'	// keyvalue for page down
#define	ZMENU_CURSOR_LEFT			'L'	// keyvalue for cursor left
#define	ZMENU_CURSOR_RIGHT		'R'	// keyvalue for cursor right
#define	ZMENU_ADD_ITEM				'+'	// keyvalue for add item
#define	ZMENU_DEL_ITEM				'-'  	// keyvalue for delete item
#define	ZMENU_SELECT				'E' 	// keyvalue for select

////////////////////////////
// End configuration section
////////////////////////////


#define STDIO_DEBUG_SERIAL  SADR		// displays results in hyperterminal
                                    // when in run mode.
#define STDIO_DEBUG_BAUD    57600   // baudrate for connection with
                                    // hyperterminal.

#define 	ZM_MATCHFLAG 				RN_MATCH_PRDID	//set flag to search for product ID
#define 	ZM_MATCHPID  				RN1600			//RN1600 KDIF card

// Some Global vars used in this sample
shared long 	lVal,ipVal,rVal,nVal;
shared struct	tm tVal;
shared float 	fVal;
shared char 	sVal[50];
shared char 	pVal[50];
shared int		ipState;


// Definitions and globals used within the zmenu functions.
// These should not be altered.
// Configuration Flags
#define	ZMENU_TITLE					0x00	// Setup Title Screen
#define	ZMENU_OPTION				0x01  // Setup for option Menu
#define	ZMENU_END					0x02  // End Menu Config
// Menu Setup Flags
#define	ZMENU_BORDER				0x01  // Use bordering flag
#define	ZMENU_SHADOW				0x02  // Use shadowing flag
#define	ZMENU_KEYPAD				0x04  // Use keypad flag
#define	ZMENU_TOUCHSCREEN			0x10  // reserved for future use
#define	ZMENU_USE_LOC_OFFSET		0x08	// reserved for future use
// Menu option Run Flags
#define	ZMENU_NULL					0x0000  	// no entry flag
#define	ZMENU_FUNCTION				0x0001  	// Function call flag
#define	ZMENU_SUBMENU				0x0002  	// Submenu call flag
#define	ZMENU_LASTMENU				0x0004	// last menu call flag
#define	ZMENU_SET_FLAG				0x0008	// Set an int Value flag
#define	ZMENU_LONG					0x0010  	// DataEntry flags
#define	ZMENU_FLOAT					0x0020   // DataEntry flags
#define	ZMENU_STRING				0x0040   // DataEntry flags
#define	ZMENU_ADDRESS				0x0080   // DataEntry flags
#define	ZMENU_DATETIME				0x0100   // DataEntry flags
#define	ZMENU_PASSWORD				0x0200   // DataEntry flags
#define	ZMENU_REFRESH_TIME		0x0400	// DataEntry flags
// Menu State Flags
#define	ZMENU_INIT					0x00 		// Initialize Menu
#define	ZMENU_REFRESH				0x02 		// Refresh Menu Display
#define	ZMENU_CLEAR					0x04 		// Clear Menu Display

#define	ZMENU_MAX_DATA_LEN 100		// Maximum string length in
													// data entry

typedef struct
{
	void	(*zmKeyConfig)();		// keypad configuration
   void	(*zmKeyProcess)();   // keypad proccess
   char	(*zmKeyGet)();       // keypad aquire
   void	(*zmPrintf)();       // Display text function
   void	(*zmPrintTitle)();   // Display Title function
   void  (*zmPrintOption)();  // Display menu options function
   void  (*zmClearMenu)();    // Clear current menu function
   void	(*zmBlankScreen)();  // Blank entire screen function
   void	(*zmHighlightEnable)(); // Enable highlighting function
   void	(*zmHighlight)();       // Move higlight function
   void	(*zmBorder)();          // Display border function
   void  (*zmLock)();            // Lock display function
   void  (*zmUnlock)();          // Unlock display function
   int	(*zmDataEntry)();       // Perform data entry function
} _zm_fxns; // 28 Bytes of root

typedef struct
{
  	int	xCoord;           // Starting xCoordinate
  	int	yCoord;           // Starting yCoordinate
   int 	border;           // Flag for the menu border option
  	int 	winWidth;			// Width of window frame
  	int 	winHeight;        // Height of window frame
  	int	curXmax;				//
  	int 	current_offset; 	// Pointer to current menu list
  	int 	new_offset;			// Pointer to new menu list location
  	int 	lasthighlight;  	// Last location of Highlight Bar
  	int 	highlight;        // Current location of Hightlight Bar
  	int	lastoption;       // Last option in list
  	int	menuState;        // current menu state
  	int	lastmenuState;    // Last menu state
  	int	maxDispOptions;		// Maximum Display Options.
} _zm_params;	// 28 bytes

typedef struct
{
 	char				*option;		// option heading
 	int				param;      // parameters used with option
 	int				menuflag;   //
 	void				*dval;      // dataentry pointer
 	int				digits;     // number of digits for dataentry
 	unsigned long	naddr;      // location of next option
   union
   {
   	int	menu;             // Menu number used for ZMENU_SUBMENU/LASTMENU
   	int	(*fxn)();         // Function pointer used with ZMENU_FUNCTION
      int	flagval;          // Flag value used with ZMENU_SET_FLAG
      int	dtype;            // Data Entry type used with data entry options
   } 	zm_type;
}	_zm_option;  // 16 bytes

typedef struct
{
 	char		*heading;			// Menu Heading
 	int		param;            // menu parameters
}	_zm_title;	// 4 bytes

typedef struct
{
	int				enabled;		// Menu has been enabled
	unsigned long	foption_addr; // location of first option
   _zm_title 		title;        // Title structure
   _zm_params		params;       // Menu internal parameters
} _zm_menu;	// 48 bytes.

unsigned long	zm_xPtr;			 // Location of the menu structures in xmem

unsigned int   zm_startmenu, zm_lastmenu, zm_endmenu, zm_runmenu;
unsigned int   zm_state;     	// Current menu system state (0 or 1)
char	zm_DataString[ZMENU_MAX_DATA_LEN]; // data entry holder
_zm_menu	zm_menu;
_zm_fxns	zm_fxns;
_zm_option zm_option;
char * const zm_deString[] =
{
	"0123456789-.:",
	"ABCDEFGHIJKLM",
	"NOPQRSTUVWXYZ",
	"abcdefghijklm",
	"nopqrstuvwxyz",
	"@ !#$^&*()_=|",
	"{}[]<>,/?~;'+",
	""
};
rn_devstruct *zm_devaddr;
int	zm_device;
rn_search zm_newdev;
int zm_oldX, zm_oldY, zm_newX, zm_newY;


// User Function Prototypes to control the menu system.

/*********************************************
Zmenu_Init

SYNTAX:				void	Zmenu_Init (int Menus, int StartMenu);

DESCRIPTION:		Initializes the Zmenu library.  Must be called once
						at start of program.

PARAMETER1:			The Maximum number of Menus needed for the application
PARAMETER2:			The first menu number to run.
RETURN VALUE:     none.
************************************************/
void	Zmenu_Init (int Menus, int StartMenu);
/*********************************************
Zmenu_Config

SYNTAX:           int	Zmenu_Config(int MenuNumber, ...);

DESCRIPTION:   	Zmenu_Config, is the function for setting up a menu.
						The function uses identifiers for determining course
						of action.  Each Zmenu_Config must incorporate 1 of the 2
						identifiers ZMENU_TITLE, or ZMENU_OPTION, and must end with
						the identifier ZMENU_END.

sample usage;

	// Menu Config Function.
	Zmenu_Config(0,
					 ZMENU_TITLE,"MAIN MENU", ZMENU_KEYPAD | ZMENU_BORDER, NULL,
					   20,4,0,0,3,
					 ZMENU_OPTION,"Toggle Backlight", ZMENU_FUNCTION,zbacklight,
                ZMENU_OPTION,"Increment LEDS", ZMENU_SET_INT,&ledState,1,
                ZMENU_OPTION,"Turn Off LEDS", ZMENU_SET_INT,&ledState,0,
                ZMENU_OPTION,"GOTO DATA MENU",ZMENU_SUBMENU,1,
					 ZMENU_OPTION,"GOTO LOG MENU",ZMENU_SUBMENU,2,
                ZMENU_END );

PARAMETER1:			The MenuNumber to configure.

PARAMETERX:			identifiers and parameters used for the menu.

IDENTIFIERS and There usage
============================================================================
ZMENU_TITLE 		identifies the next set of settings to be associated with
						the Menu parameters.  Parameter 1 following the identifier
						is the Title of the Menu., Parameter 2 is the
						characteristics of the Menu.  Characteristics allowed
						are as follows;

ZMENU_BORDER		- Will place a border around the menu.
ZMENU_KEYPAD  		- Will use the keypad for menu control.

						These parameters can be OR'ed together as needed.
						Parameter 3 is a pointer to the font that will be
						used for the Menu and its items associated.
						NULL can be used if the Menu is being used on a
						character style LCD that has no fonts.

ZMENU_OPTION, 		identifies the next set of parameters to be associated
						with a particular option within the menu.  Parameter 1
						following the identifier is always the title of the option.
						Parameter 2 is the item action that will be taken if the
						item is selected.  The parameter following the action
						parameter is dependent upon the action parameter itself.
						Action parameters allowed are as follows;

ZMENU_FUNCTION		- The next parameter would be a pointer to a user-defined
						  function that will be called when the item is selected.
						  The User function must return a non-zero when completed,
						  as well as being non-blocking.
ZMENU_SUBMENU		- The next parameter would be the menu number to be
					  	  displayed when the item is selected.
ZMENU_LASTMENU		- no parameter is entered.  The item selected will display
						  the previous menu displayed.
ZMENU_SET_FLAG		- Two parameters are required.  The first would be a
						  pointer to an int that this item is associated with;
						  the next would be the value to place in that int.
ZMENU_LONG			- This is a data entry function.  Two parameters are required.
						  The first is a long pointer to a long value that will be
						  used for data entry.  The second will be the maximum
						  number of digits that the long value will have
						  (in decimal).  When this item is selected.
						  A data entry window will be displayed allowing the
						  operator to enter a numeric value.
ZMENU_FLOAT			- This is a data entry function.  Two parameters are required.
						  The first is a float pointer to a float value that will be
						  used for data entry.  The second will be the maximum
						  number of digits that the float value will have
						  (in decimal).  When this item is selected.
						  A data entry window will be displayed allowing the
						  operator to enter a numeric value.
ZMENU_STRING		- This is a data entry function.  Two parameters are required.
						  The first is a char pointer to a char array value that
						  will be used for data entry.  The second will be the
						  maximum number of digits that the char array value
						  will have (in decimal).  When this item is selected.
						  A data entry window will be displayed allowing the
						  operator to enter an alphanumeric value.
ZMENU_TIMEDATE		- This is a data entry function.  1 parameter is required.
						  The only parameter required is a pointer to the time
						  structure that will be used for data entry.
						  When this item is selected, a data entry window will
						  be displayed allowing the operator to enter a time
						  date value.
ZMENU_PASSWORD		- Can be or'ed with the above data entry functions
						  to enable password entry.

The Zmenu_Config function can also be used multiple times for the same menu.
This is necessary do to function limitations concerning the maximum amount of
characters related to a function (which is currently 512 bytes).
For example if you were to take the above sample again, you could
setup the menu as follows;

Zmenu_Config(0,
	ZMENU_TITLE,"MAIN MENU",ZMENU_BORDER | ZMENU_KEYPAD,NULL,
		20,4,0,0,3,
	ZMENU_OPTION,"Toggle Leds",ZMENU_SET_INT, &ledState, 1,
	ZMENU_END);

Zmenu_Config(0,
	ZMENU_OPTION,"Blink Leds",ZMENU_SET_INT, &ledState,2,
	ZMENU_END);

****************************************************/
int	Zmenu_Config (int Menu, ...);
/*********************************************
Zmenu_State

SYNTAX:				void Zmenu_State(int OnOff);

DESCRIPTION:		Disables/Enables the Zmenu_Menuhandler function.  This
						is used to disable the Menu handler to give manual control
						of the screen and keypad.  Once the menu is Enabled
						again, the last menu that was used will be displayed.
PARAMETER1:			1 to Enable, 0 to disable.
RETURN VALUE:     none.
********************************************/
void	Zmenu_State(int OnOff);
/*********************************************
Zmenu_Handler
SYNTAX:				int Zmenu_Handler(int *Menu, int *Option);
DESCRIPTION:		Runs the ZMENU system. Must be called periodically
						from the application program. It handles all
						menu display and keypad readings, as well as all
						function calling that is setup in the Zmenu_Config
						options.  *Menu will have the menu number of the
						last menu that was selected, *Option will have the
						option number that was last selected.  Once an option
						is selected, and its task completed the return value
						will be 1.

PARAMETER1:			The integer pointer to store the last menu selected.
PARAMETER2:			The integer pointer to store the last option selected.
RETURN VALUE:     Returns 1 when on option is selected.  0 if no options
						were selected.
*******************************************/
int 	Zmenu_Handler(int *Menu, int *Option);
/*********************************************
Zmenu_KeyConfig

SYNTAX:				void Zmenu_KeyConfig();

DESCRIPTION:		User defineable function for configuring the
						individual keys on a Rabbit Net keypad.  This must
						be defined in the source program. Use the variable
						zm_device as the first parameter

RETURN VALUE:     none.

sample function for use with the RN16000 kdif and 2x6 keypad:

void Zmenu_KeyConfig()
{
	rn_keyConfig (zm_device, 16,ZMENU_SELECT,0,0,0,0,0);
	rn_keyConfig (zm_device, 11,ZMENU_SCROLL_DN,0,50,10,10,10);
  	rn_keyConfig (zm_device, 10,ZMENU_PAGE_DN,0,50,10,10,10);
	rn_keyConfig (zm_device,  9,ZMENU_DEL_ITEM,0,50,10,10,10);
	rn_keyConfig (zm_device,  8,ZMENU_ADD_ITEM,0,50,10,10,10);
	rn_keyConfig (zm_device,  3,ZMENU_SCROLL_UP,0,50,10,10,10);
	rn_keyConfig (zm_device,  2,ZMENU_PAGE_UP,0,50,10,10,10);
	rn_keyConfig (zm_device,  1,ZMENU_CURSOR_LEFT,0,50,10,10,10);
	rn_keyConfig (zm_device,  0,ZMENU_CURSOR_RIGHT,0,50,10,10,10);
}
**********************************************************/
void 	Zmenu_KeyConfig(); 			// Configures the Keypad.


// Other user functions called.
int 	zmToggleBacklight();       // Toggles the backlight
int	zmMainMenu(int Option);    // For use with the Main Menu
int	zmDataMenu(int Option);    // For use with the Data Menu
int	zmTCPMenu(int Option);    // For use with the TCP Menu
int 	zmClearStdio();

// 	other zmenu prototypes called internally.
int	_zmenu_init();
void 	zm_BlankFxn();
void 	_zm_keyProcess();
char 	_zm_keyGet();
void  _zm_highlightenable (int OnOff);
void  _zm_highlight (int OptNum);
void 	_zmPrintTitle();
void 	_zmPrintOption(int OptNum, int OptLoc );
void 	_zmClearMenu ();
int 	_zmDataKeypad();
int	_zmEnterData();
void	_zm_write (int	Menu);
void	_zm_read( int Menu );
void	_zmenu_write_option(unsigned long addr);
void	_zmenu_read_option(unsigned long addr);
int 	Zmenu_Keypad(int MenuNumber);
int 	Zmenu_DisplayMenu(int MenuNumber, int *state );
int	Zmenu_TaskOption(int OptionNumber);

// Clears the STDIO window
nodebug
int zmClearStdio()
{
 	printf ( "\x1Bt" );            	// Space Opens Window
   printf ( "\x1b[2J" );            	// Space Opens Window
   printf("ZMENU SAMPLE PROGRAM\r\n");
   return 1;
}

// handles the result of the menu item selected in the TCP Menu section.
// PARAMETER1:  The option selected by the user.
nodebug
int	zmTCPMenu(int Option)
{
   auto long ipval;
   auto char ipbuff[20];
   switch (Option)
   {
    	case 1:	// Option 1 of the Tcp Menu
         ifconfig(IF_DEFAULT, IFS_DOWN,
         						   IFS_IPADDR, ipVal,
    								   IFS_ROUTER_SET, rVal,
    								   IFS_NETMASK, nVal,
    								   IFS_UP,
    								   IFS_END);
         inet_ntoa(ipbuff,ipVal);
    		printf("IP Address Entered = %s\r\n",ipbuff);
    		break;
      case 2:	// Option 2 of the Tcp Menu
         ifconfig(IF_DEFAULT, IFS_DOWN,
         						   IFS_IPADDR, ipVal,
    								   IFS_ROUTER_SET, rVal,
    								   IFS_NETMASK, nVal,
    								   IFS_UP,
    								   IFS_END);
         inet_ntoa(ipbuff,rVal);
    		printf("Router Address Entered = %s\r\n",ipbuff);
    		break;
      case 3:	// Option 3 of the Tcp Menu
         ifconfig(IF_DEFAULT, IFS_DOWN,
         						   IFS_IPADDR, ipVal,
    								   IFS_ROUTER_SET, rVal,
    								   IFS_NETMASK, nVal,
    								   IFS_UP,
    								   IFS_END);
         inet_ntoa(ipbuff,nVal);
    		printf("Netmask Entered = %s\r\n",ipbuff);
    		break;
      case 4:	// Option 4 of the Tcp Menu
         Zmenu_State(0); 		// Turn off the menu system, to have manual
   							// control of display and keypad
   		// wait here until all pressed keys are processed.
   		while (rn_keyGet(zm_device,0) != 0)rn_keyProcess(zm_device,0);
   		// clear the screen
   		rn_dispClear(zm_device,0);
         ifconfig(IF_DEFAULT, IFG_IPADDR, &ipval, IFS_END);
         // move cursor and display heading
    		rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - \
    						(strlen("-----IP ADDRESS-----")/2),
    					0,0);
    		rn_dispPrintf(zm_device,0,"-----IP ADDRESS-----");
         // get the ip value and put it into string format for display purposes
   		inet_ntoa(ipbuff,ipval);
   		// move cursor to bottom of display and center
   		rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - (strlen(ipbuff)/2),
    					ZMENU_ROWS - 1,0);
   		// display the ip value
   		rn_dispPrintf(zm_device,0,ipbuff);
   		// wait for the user to press a key, then exit.
   		while (rn_keyGet(zm_device,0) == 0)
  			{
   			tcp_tick(NULL);
   			rn_keyProcess(zm_device,0);
   		}
   		// have the menu system resume control again.
   		Zmenu_State(1);
    		break;

      case 5:	// Option 5 of the Tcp Menu
         Zmenu_State(0); 		// Turn off the menu system, to have manual
   							// control of display and keypad
   		// wait here until all pressed keys are processed.
   		while (rn_keyGet(zm_device,0) != 0)rn_keyProcess(zm_device,0);
   		// clear the screen
   		rn_dispClear(zm_device,0);
         ifconfig(IF_DEFAULT, IFG_ROUTER_DEFAULT, &ipval, IFS_END);
         // move cursor and display heading
      	rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - \
      						(strlen("-----ROUTER IP-----")/2),
    					0,0);
      	rn_dispPrintf(zm_device,0,"-----ROUTER IP-----");
         inet_ntoa(ipbuff,ipval);
   		// move cursor to bottom of display and center
   		rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - (strlen(ipbuff)/2),
    					ZMENU_ROWS - 1,0);
   		// display the ip value
   		rn_dispPrintf(zm_device,0,ipbuff);
   		// wait for the user to press a key, then exit.
   		while (rn_keyGet(zm_device,0) == 0)
  			{
   			tcp_tick(NULL);
   			rn_keyProcess(zm_device,0);
   		}
   		// have the menu system resume control again.
   		Zmenu_State(1);
    		break;

    	case 6:	// Option 6 of the Tcp Menu
         Zmenu_State(0); 		// Turn off the menu system, to have manual
   							// control of display and keypad
   		// wait here until all pressed keys are processed.
   		while (rn_keyGet(zm_device,0) != 0)rn_keyProcess(zm_device,0);
   		// clear the screen
   		rn_dispClear(zm_device,0);
         ifconfig(IF_DEFAULT, IFG_NETMASK, &ipval, IFS_END);
         // move cursor and display heading
      	rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - \
      						(strlen("-----NETMASK IS----")/2),
    					0,0);
    		rn_dispPrintf(zm_device,0,"-----NETMASK IS----");
         inet_ntoa(ipbuff,ipval);
   		// move cursor to bottom of display and center
   		rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - (strlen(ipbuff)/2),
    					ZMENU_ROWS - 1,0);
   		// display the ip value
   		rn_dispPrintf(zm_device,0,ipbuff);
   		// wait for the user to press a key, then exit.
   		while (rn_keyGet(zm_device,0) == 0)
  			{
   			tcp_tick(NULL);
   			rn_keyProcess(zm_device,0);
   		}
   		// have the menu system resume control again.
   		Zmenu_State(1);
    		break;
    }
    return 1;
}

// handles the result of the menu item selected in the Data Menu section.
// PARAMETER1:  The option selected by the user.
nodebug
int	zmDataMenu(int Option)
{
   switch (Option)
   {
    	case 1:	// Option 1 of the Data Menu
    		printf("Long Value Entered = %d\r\n",lVal);
    		break;
      case 2:	// Option 2 of the Data Menu
      	printf("Float Value Entered = %f\r\n",fVal);
    		break;
      case 3:	// Option 3 of the Data Menu
         printf("Time Date Entered = %02d/%02d/%04d %02d:%02d:%02d\r\n",
         tVal.tm_mday, tm_mon2month(tVal.tm_mon), tVal.tm_year + 1900,
         tVal.tm_hour, tVal.tm_min, tVal.tm_sec);
         tm_wr(&tVal);
         SEC_TIMER = mktime(&tVal);
    		break;
      case 4:	// Option 4 of the Data Menu
      	printf("String Value Entered = %s\r\n",sVal);
    		break;

      case 5:	// Option 5 of the Data Menu
      	printf("Password Entered = %s\r\n",pVal);
    		break;
    }

	return 1;
}
// handles the result of the menu item selected in the Menu Menu section.
// PARAMETER1:  The option selected by the user.
int	zmMainMenu(int Option)
{
 	return 1;
}
// Toggles the backlight
nodebug
int	zmToggleBacklight ()
{
 	static int state;
 	#GLOBAL_INIT {state = 1;}
 	if (state) state = 0;
 	else state = 1;
 	rn_dispBacklight(zm_device,state,0);
 	return 1;

}

// Configures the keypad based on the ZMENU_KEYSTROBLINES value.
nodebug
void Zmenu_KeyConfig()
{
	if (ZMENU_KEYSTROBELINES == 0x0070 )  //3X4 keypad
   {
	   rn_keyConfig (zm_device, 19,ZMENU_SCROLL_UP,0 ,50, 10,  10, 10 );
		rn_keyConfig (zm_device, 11,ZMENU_SCROLL_DN,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device, 18,ZMENU_PAGE_UP,0 ,50, 10,  10, 10 );
   	rn_keyConfig (zm_device, 10,ZMENU_PAGE_DN,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device, 17,ZMENU_CURSOR_LEFT,0 ,50, 10,  10, 10 );
		rn_keyConfig (zm_device,  9,ZMENU_DEL_ITEM,0 ,50, 10,  10, 10 );
   	rn_keyConfig (zm_device, 16,ZMENU_CURSOR_RIGHT,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  8,ZMENU_ADD_ITEM,	0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  0,ZMENU_SELECT,	0, 0, 0,  0, 0);
   }
   else if (ZMENU_KEYSTROBELINES == 0x00C0) //2x6 keypad
   {
	   rn_keyConfig (zm_device, 13,ZMENU_SCROLL_UP,0 ,50, 10,  10, 10 );
		rn_keyConfig (zm_device,  5,ZMENU_SCROLL_DN,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device, 12,ZMENU_PAGE_UP,	0 ,50, 10,  10, 10 );
   	rn_keyConfig (zm_device,  4,ZMENU_PAGE_DN,	0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device, 11,ZMENU_CURSOR_LEFT,0 ,50, 10,  10, 10 );
		rn_keyConfig (zm_device,  3,ZMENU_DEL_ITEM,	0 ,50, 10,  10, 10 );
   	rn_keyConfig (zm_device, 10,ZMENU_CURSOR_RIGHT,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  2,ZMENU_ADD_ITEM,	0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  0,ZMENU_SELECT,	0, 0, 0,  0, 0);
   }
   else if (ZMENU_KEYSTROBELINES == 0x03C0) //4x6 keypad
   {
	   rn_keyConfig (zm_device,  5,ZMENU_SCROLL_UP,0 ,50, 10,  10, 10 );
		rn_keyConfig (zm_device, 13,ZMENU_SCROLL_DN,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  4,ZMENU_PAGE_UP,0 ,50, 10,  10, 10 );
   	rn_keyConfig (zm_device, 12,ZMENU_PAGE_DN,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  3,ZMENU_CURSOR_LEFT,0 ,50, 10,  10, 10 );
		rn_keyConfig (zm_device, 11,ZMENU_DEL_ITEM,0 ,50, 10,  10, 10 );
   	rn_keyConfig (zm_device,  2,ZMENU_CURSOR_RIGHT,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device, 10,ZMENU_ADD_ITEM,0 ,50, 10,  10, 10 );
	   rn_keyConfig (zm_device,  8,ZMENU_SELECT,0, 0, 0,  0, 0);
   }
}

// Main Control
main ()
{
   static int MenuRan, OptionSelect,status;

 	brdInit();
 	Zmenu_Init(10,ZM_MAIN_MENU);	// Initialize 10 menus starting at Menu 0
	sock_init(); // initialize TCP.
	rn_dispBacklight(zm_device,1,0); // Turn on Backlight
	// Setup the individual menus.
   Zmenu_Config(ZM_MAIN_MENU,
   		  	ZMENU_TITLE, "----ZMENU SAMPLE----",ZMENU_KEYPAD,ZMENU_COLUMNS,
   		  	ZMENU_ROWS,0,0,ZMENU_ROWS -1,
   		  	ZMENU_OPTION,"Toggle Backlight",ZMENU_FUNCTION,zmToggleBacklight,
   		  	ZMENU_OPTION,"Data Entry Menu",ZMENU_SUBMENU,ZM_DATA_MENU,
           	ZMENU_OPTION,"TCP View Menu",ZMENU_SUBMENU,ZM_TCP_MENU,
				ZMENU_OPTION,"Clear STDIO",ZMENU_FUNCTION,zmClearStdio,
     		  	ZMENU_END);

	Zmenu_Config(ZM_DATA_MENU,
   		  	ZMENU_TITLE, "---- DATA ENTRY ----",ZMENU_KEYPAD,ZMENU_COLUMNS,
   		  	ZMENU_ROWS,0,0,ZMENU_ROWS -1,
   		  	ZMENU_OPTION,"Enter A Long Value",ZMENU_LONG,&lVal,10,
   		  	ZMENU_OPTION,"Enter A Float Value",ZMENU_FLOAT,&fVal,10,
   		  	ZMENU_OPTION,"Set The Time/Date",ZMENU_DATETIME | ZMENU_REFRESH_TIME,&tVal,20,
   		  	ZMENU_OPTION,"Enter A String",ZMENU_STRING,&sVal,20,
   		  	ZMENU_OPTION,"Enter A Password",ZMENU_STRING | ZMENU_PASSWORD,&pVal,20,
				ZMENU_OPTION,"Return To Previous",ZMENU_LASTMENU,
     		  	ZMENU_END);
   Zmenu_Config(ZM_TCP_MENU,
   		  	ZMENU_TITLE, "------TCP MENU------",ZMENU_KEYPAD,ZMENU_COLUMNS,
   		  	ZMENU_ROWS,0,0,ZMENU_ROWS -1,
   		  	ZMENU_OPTION,"Enter IP Address",ZMENU_ADDRESS,&ipVal,20,
            ZMENU_OPTION,"Enter Router Addr.",ZMENU_ADDRESS,&rVal,20,
            ZMENU_OPTION,"Enter Netmask",ZMENU_ADDRESS,&nVal,20,
            ZMENU_OPTION,"View IP",ZMENU_SET_FLAG,&ipState,1,
            ZMENU_OPTION,"View ROUTER",ZMENU_SET_FLAG,&ipState,2,
            ZMENU_OPTION,"View NETMASK",ZMENU_SET_FLAG,&ipState,3,
            ZMENU_OPTION,"Previous Menu",ZMENU_LASTMENU,
   		  	ZMENU_END);
   // Initialize variables
	tm_rd(&tVal);	// Get the Time.
   lVal = 0;
   fVal = 0;
   sprintf(sVal,"HELLO THERE");
   ipState = 0;
   Zmenu_State(1); // Turn on the Menu System.
   zmClearStdio(); // Clear the STDIO window
   // Setup the TCP connection
   ifconfig(IF_DEFAULT, IFG_IPADDR, &ipVal,
   							IFG_ROUTER_DEFAULT, &rVal,
                        IFG_NETMASK, &nVal,
   							IFS_END);

   for (;;)  // loop forever
   {
   	costate
   	{
   		// Wait for a  menu/option to be selected
       	waitfor(Zmenu_Handler(&MenuRan,&OptionSelect));
       	// display the menu/option that was selected
         printf("Last Menu = %d\tLast Option = %d\r\n",MenuRan,OptionSelect);
         switch (MenuRan)
         {
          	case ZM_MAIN_MENU:
          		waitfor (zmMainMenu(OptionSelect));
          		break;
          	case ZM_DATA_MENU:
	              waitfor (zmDataMenu(OptionSelect));
          		break;

          	case ZM_TCP_MENU:
          			waitfor (zmTCPMenu(OptionSelect ));
          		break;
         }
   	}
	tcp_tick(NULL);
   }
}


// The Rest of these functions pertain to the menu control and should not
// be modified if possible.


// low level initialize and setup of the various structures.
nodebug
int	_zmenu_init()
{
   zm_newdev.flags = ZM_MATCHFLAG;    	// Setup Search criteria
	zm_newdev.productid = ZM_MATCHPID; 	// Setup Search criteria
	rn_init(RN_PORTS, 1);      			//initialize controller RN ports
	zm_device = rn_find(&zm_newdev); 	// Find the RN1600
 	rn_keyInit(zm_device, ZMENU_KEYSTROBELINES, 10);
   rn_dispInit(zm_device, ZMENU_ROWS, ZMENU_COLUMNS);
   // Setup the Individual function pointers used by Zmenu.lib
	zm_fxns.zmKeyConfig 			= 	Zmenu_KeyConfig;
	zm_fxns.zmKeyProcess 		= _zm_keyProcess;
	zm_fxns.zmKeyGet 				= _zm_keyGet;
	zm_fxns.zmPrintf 				= 	rn_dispPrintf;
	zm_fxns.zmPrintTitle			= _zmPrintTitle;
	zm_fxns.zmPrintOption		= _zmPrintOption;
	zm_fxns.zmBlankScreen 		= 	zm_BlankFxn;
	zm_fxns.zmClearMenu			= _zmClearMenu;
	zm_fxns.zmHighlightEnable 	= _zm_highlightenable;
	zm_fxns.zmHighlight 			= _zm_highlight;
	zm_fxns.zmBorder 				= 	zm_BlankFxn;
	zm_fxns.zmLock					= 	zm_BlankFxn;
	zm_fxns.zmUnlock				= 	zm_BlankFxn;
	zm_fxns.zmDataEntry			= _zmEnterData;
	// Call the user defined keyconfig function.
	zm_fxns.zmKeyConfig();
}

// blank function holder
nodebug
void zm_BlankFxn()
{
}

// process keypresses
nodebug
void _zm_keyProcess()
{
 	rn_keyProcess (zm_device, 0);
}

// get keypresses
nodebug
char _zm_keyGet()
{
 	return rn_keyGet (zm_device, 0);
}

// turn on highlighting by blinking the cursor.
nodebug
void  _zm_highlightenable (int OnOff)
{
	rn_dispCursor(zm_device, RNDISP_CURBLINKON, 0);
}

// Move the cursor to the appropriate line.
nodebug
void  _zm_highlight (int OptNum)
{
  rn_dispGoto (zm_device, 0, OptNum + 1, 0);
}

// Display the Menu Title.
nodebug
void _zmPrintTitle()
{
   rn_dispGoto(	zm_device,((ZMENU_COLUMNS/2) - \
   					(strlen(zm_menu.title.heading)/2)),0,0);
   rn_dispPrintf(zm_device,0,zm_menu.title.heading);
}

// Display the appropriate options for the menu
// PAR. 1:	The Option Number to display.
// PAR. 2:  Line number to display the option on.
nodebug
void _zmPrintOption(int OptNum, int OptLoc )
{
   auto int index;
  	index = 0;
   if (zm_menu.foption_addr)
   {
   	_zmenu_read_option(zm_menu.foption_addr);
   	if (OptNum != 0);
   	{
     		for (index = 1; index <= OptNum ; index++)
     		{
     		  	_zmenu_read_option(zm_option.naddr);
     		  	if (zm_option.naddr == 0) break;
   		}
   	}
   	rn_dispGoto(zm_device,1,OptLoc + 1,0);
   	rn_dispPrintf(zm_device,0,zm_option.option);
	}
}

// Clear the lcd screen
nodebug
void _zmClearMenu ()
{
 	rn_dispClear(zm_device,0);
}


// Keypad handler for data entry functions.

nodebug
int	_zmDataKeypad()
{
 	auto int keypress,pindex;
 	static int len, startX, dPos,keyindex;
 	static char zmPass[20];
 	#GLOBAL_INIT {startX = keyindex = 0; }
 	// waitfor a keypress
 	if ( (keypress = rn_keyGet(zm_device,0)) > 0)
 	{
 		// calc the current length of the data entry selection string
 		len = strlen(zm_deString[keyindex]);
 		// calc. the current length of the data entered string
 		dPos = strlen(zm_DataString);
    	switch (keypress)
    	{
       	case ZMENU_CURSOR_RIGHT: // move the highlighted entry.
            if ( startX < ( len - 1 ) )
            {
             	startX++;
             	zm_newX ++;
            }
            else
            {
            	startX = 0;
            	zm_newX=  (ZMENU_COLUMNS/2)-(strlen(zm_deString[keyindex])/2);
   			}

            rn_dispGoto(zm_device,zm_newX,zm_newY,0);
       		break;
       	case ZMENU_CURSOR_LEFT:	// move the highlighted entry.
            if (startX > 0)
            {
             	startX--;
             	zm_newX --;
            }
            else
            {
            	startX = len - 1;
               zm_newX=  (ZMENU_COLUMNS/2)-(strlen(zm_deString[keyindex])/2)+ \
               			 (strlen(zm_deString[keyindex])-1);
            }
            rn_dispGoto(zm_device,zm_newX,zm_newY,0);
       		break;
       	case ZMENU_ADD_ITEM:	// Add a char to the datastring
       		// Make sure string isn't already maximum length
       		if (strlen(zm_DataString) < zm_option.digits)
       		{
               // add the new char to the string
       			zm_DataString[dPos] = zm_deString[keyindex][startX];
       			dPos++;
       			// terminate the new string
       			zm_DataString[dPos] = '\0';
       			// move the cursor
       			if (ZMENU_ROWS > 2)
               	rn_dispGoto(zm_device,0,1,0);
               else
                  rn_dispGoto(zm_device,0,0,0);
               // Check for a password entry. If so display asterisks instead
               if (zm_option.param & ZMENU_PASSWORD)
               {
               	memset(zmPass,'*',dPos);
                  zmPass[dPos] = '\0';
                  rn_dispPrintf(zm_device,0,zmPass);
               }
               else
       			{
       				// Display the new string
       				rn_dispPrintf(zm_device,0,zm_DataString);

            	}
            	// Move the cursor back to the data selection area.
            	rn_dispGoto(zm_device,zm_newX,zm_newY,0);
            }
       		break;
       	case ZMENU_DEL_ITEM:	// Add a char to the datastring
       		// Make sure the length isn't 0.
       		if (dPos > 0)
       		{
       			dPos--;
       			// remove the last char by replacing it with a null term.
       			zm_DataString[dPos] = '\0';
       			// Do the same as adding a char.
               if (ZMENU_ROWS > 2)
               	rn_dispGoto(zm_device,0,1,0);
               else
                  rn_dispGoto(zm_device,0,0,0);
            	rn_dispPrintf(zm_device,0,"                    ");
               if (ZMENU_ROWS > 2)
               	rn_dispGoto(zm_device,0,1,0);
               else
                  rn_dispGoto(zm_device,0,0,0);
               if (zm_option.param & ZMENU_PASSWORD)
               {
                  memset(zmPass,'*',dPos);
                  zmPass[dPos] = '\0';
                  rn_dispPrintf(zm_device,0,zmPass);
               }
               else
               {
            		rn_dispPrintf(zm_device,0,zm_DataString);
            	}
            	rn_dispGoto(zm_device,zm_newX,zm_newY,0);
            }
         	break;
         case ZMENU_SCROLL_DN: // Scroll down to next set of data selections
         	if (keyindex < 6) keyindex++;
	         else	keyindex = 0;
	         // display the next set.
            rn_dispGoto(zm_device,(ZMENU_COLUMNS/2)-(strlen(zm_deString[keyindex])/2),
            				ZMENU_ROWS - 1,0);
            rn_dispPrintf(zm_device,0,zm_deString[keyindex]);
            rn_dispGoto(zm_device,zm_newX,zm_newY,0);
         	break;
         case ZMENU_SCROLL_UP:	// Scroll up to next set of data selections
            if (keyindex > 0)keyindex--;
            else	keyindex = 6;
            // display the next set.
            rn_dispGoto(zm_device,(ZMENU_COLUMNS/2)-(strlen(zm_deString[keyindex])/2),
            				ZMENU_ROWS - 1,0);
            rn_dispPrintf(zm_device,0,zm_deString[keyindex]);
            rn_dispGoto(zm_device,zm_newX,zm_newY,0);
         	break;
         case ZMENU_SELECT: // The enter key as been pressed.
         	// reset variables and exit out of function.
         	keyindex = 0;
         	startX = 0;
         	return 1;
         	break;
         default:
         	break;
    	}

 	}
 	return 0;
}

// Data Entry Function Control
nodebug
int	_zmEnterData()
{
	static int retval, state, lastCurPos, CurPos;
	auto int keypress;
	static struct tm TimeDate;

	#GLOBAL_INIT {state = 0;}
	switch (state)
	{
    	case 0:
    		retval = 0;
    		// set the initial location of the cursor
         zm_oldX = zm_oldY = -1;
         // Check to see what type of data entry was selected, and
         // setup the display string appropriately.
         if (zm_option.param & ZMENU_LONG)
          	sprintf(zm_DataString,"%ld",*(long *)zm_option.dval);
         else if(zm_option.param & ZMENU_FLOAT)
         	sprintf(zm_DataString,"%f",*(float *)zm_option.dval);
         else if(zm_option.param & ZMENU_STRING)
             memcpy(zm_DataString,zm_option.dval,strlen(zm_option.dval)+1);
         else if(zm_option.param & ZMENU_ADDRESS)
         {
         	#ifdef IP_H
         		inet_ntoa(zm_DataString,*(longword *)zm_option.dval);
            #endif
         }
         else if(zm_option.param & ZMENU_DATETIME)
         {
            if (zm_option.param & ZMENU_REFRESH_TIME)
            	tm_rd(&TimeDate);
            else
            	memcpy(&TimeDate, *(struct tm **)&zm_option.dval,sizeof(TimeDate));
            strftime( zm_DataString, sizeof zm_DataString, "%m-%d-%Y %H:%M:%S",
            	&TimeDate);
         }
         // Check the length against the max value entered.
         if (strlen(zm_DataString) > zm_option.digits)
           	zm_DataString[zm_option.digits] = '\0';
         // Zero out the string if it is a password.
         if (zm_option.param & ZMENU_PASSWORD)
         	memset(zm_DataString,0x00,sizeof(zm_DataString));
         // Setup the data selection cursor location.
         zm_newX = (ZMENU_COLUMNS/2) - ((strlen(zm_deString[0]))/2);
         zm_newY = ZMENU_ROWS - 1;
         rn_dispClear(zm_device,0);
         // Display the heading if there is enough room to do so
         if (ZMENU_ROWS > 2)
         {
          	rn_dispGoto(zm_device,(ZMENU_COLUMNS/2) - ((strlen(zm_option.option))/2),0,0);
           	rn_dispPrintf(zm_device,0,zm_option.option);
           	rn_dispGoto(zm_device,0,1,0);
         }
         else
         {
         	rn_dispGoto(zm_device,0,0,0);
         }
         // Display the data, and data selection strings.
         rn_dispPrintf(zm_device,0,zm_DataString);
         rn_dispGoto(zm_device,zm_newX,zm_newY,0);
         rn_dispPrintf(zm_device,0,zm_deString[0]);
         rn_dispGoto(zm_device,zm_newX,zm_newY,0);
         state++;
    		break;
      case 1: // Wait for the ZMENU_SELECT key to be pressed.
      	if (_zmDataKeypad())
      	{
      		state = 0;
         	retval = 1;
         }
      	break;
    	default:	// Should (hopefully) never get here.
    		retval = 0;
    		state = 0;
    		break;
	}
 	return retval;
}


// Stores a Menu Structure to xmem
nodebug
void	_zm_write (int	Menu)
{
   root2xmem(zm_xPtr + (sizeof(zm_menu) * Menu),&zm_menu,sizeof(zm_menu));
}
// retrieves a menu structure from xmem
nodebug
void	_zm_read( int Menu )
{
 	xmem2root (&zm_menu,zm_xPtr + (sizeof(zm_menu) * Menu),sizeof(zm_menu));
}
// stores an option structure to xmem
nodebug
void	_zmenu_write_option(unsigned long addr)
{
	if (addr)root2xmem(addr,&zm_option,sizeof(zm_option));
}
// retrieves an option structure from xmem
nodebug
void	_zmenu_read_option(unsigned long addr)
{
 	if (addr)xmem2root(&zm_option,addr,sizeof(zm_option));
}

// user function for initializing.
// PAR 1:	The Maximum number of menus needed.
// PAR 2:   The Starting Menu number to use.
nodebug
void	Zmenu_Init (int NumMenus, int StartMenu)
{
   auto int index;
   // reset the zm_menu structure.
   memset(&zm_menu,0x00,sizeof(zm_menu));
   // xalloc the space required to hold all of the menus
   zm_xPtr = xalloc((long)sizeof(zm_menu) * (long)NumMenus);
   // reset the xmem area to 0 where the menu structures will be stored.
   for(index = 0 ; index < NumMenus ; index++)_zm_write(index);
   // Set the initial menu to be started with
   zm_startmenu = StartMenu;
   zm_lastmenu != zm_startmenu;
   zm_endmenu = NumMenus;
   // set the menu system to be disabled until it is enabled by Zmenu_State()
   zm_state = 0;
   // Call the low level initialization function specific to the device
   // (ZMENU_RN1600, ZMENU_KDU, or ZMENU_EDISP libraries).
   _zmenu_init();
}

// User function for configuring the menus.  See the sample description
// for more detail.

nodebug
int	Zmenu_Config (int Menu, ...)
{
   auto char *p;
	auto int index, ident, done, retval;
	auto long laddr;
	retval = -1; 	// set initial return value to failure condition
	done = index = 0;
	// set the pointer to the first elypsis parameter.
   p = (char *)(&Menu + 1);
   // check to see that a valid menu number is entered.
   if (Menu <= zm_endmenu)
   {
   	while (!done)
   	{
      	ident = *(int *)p;  	// ident should = Either ZMENU_TITLE,
      								//	ZMENU_INIT, or ZMENU_END
    		p 	+= sizeof(int);   // Move pointer to the next param
      	switch (ident)
      	{
      		case ZMENU_TITLE:
      			// clear out the menu structure.
      			memset (&zm_menu, 0x00, sizeof(zm_menu));
      			// store the menu title pointer
      			zm_menu.title.heading 	= *(char **)p;
       			p += sizeof(char *); // index pointer
            	zm_menu.title.param 	= *(int *)p; // store the menu params
       			p += sizeof(int);    // index pointer
            	zm_menu.params.winWidth = *(int *)p; // store width
            	p += sizeof(int);
            	zm_menu.params.winHeight = *(int *)p;// store height
            	p += sizeof(int);
            	zm_menu.params.xCoord  = *(int *)p; // store start xCoord
            	p += sizeof(int);
               zm_menu.params.yCoord  = *(int *)p; // store start yCoord
            	p += sizeof(int);
            	zm_menu.params.maxDispOptions = *(int *)p; // max DispOptions
            	p += sizeof(int);
            	// init rest of the params
            	zm_menu.params.current_offset = 0;
            	zm_menu.params.new_offset = 0;
            	zm_menu.params.highlight = 0;
            	zm_menu.params.lasthighlight = 0;
            	zm_menu.params.lastoption = -1; // indicates no menu options
            	zm_menu.params.menuState = ZMENU_INIT;
            	zm_menu.params.lastmenuState = ZMENU_INIT;
            	zm_menu.enabled = 1;
            	zm_menu.foption_addr = 0;
            	_zm_write(Menu); // store the Menu Data.
      			break;

      		case ZMENU_OPTION:
      			_zm_read(Menu);  // Read in the Menu Data
      			// Make sure ZMENU_TITLE was done, and that the option number
      			// is valid.
      			if (zm_menu.enabled)
      			{
                  if (!zm_menu.foption_addr)
                  {
                   	zm_menu.foption_addr = xalloc((long)sizeof(zm_option));
                   	laddr = zm_menu.foption_addr;
                  }
                  else
                  {
                      laddr =  zm_menu.foption_addr;
                      _zmenu_read_option(zm_menu.foption_addr);
                      while (zm_option.naddr != 0)
                      {
                      	laddr = zm_option.naddr;
                      	_zmenu_read_option(zm_option.naddr);
                      }
                      zm_option.naddr = xalloc((long)sizeof(zm_option));
                      _zmenu_write_option(laddr);
                      laddr = zm_option.naddr;
                  }
                  memset(&zm_option,0x00,sizeof(zm_option));
      				// index the item number to the next avail one.
                  zm_option.option =  *(char **)p;
                  p += sizeof(char *); // index pointer
            		// store the item action parameter data.
            		zm_option.param 	= *(int *)p;
            		p += sizeof(int); // index pointer
            		// switch on the appropriate item action parameter
            		switch (zm_option.param)
         			{
         		 		case ZMENU_FUNCTION:
         		 			// store the function pointer.
                 		 	zm_option.zm_type.fxn 	= (int(*)())*(int *)p;
               			p += sizeof( int *);// index pointer
         					break;

         				case ZMENU_SUBMENU:
         					// store the submenu number to use
                  		zm_option.zm_type.menu 	= *(int *)p;
                  		p += sizeof(int); // index pointer
	        					break;

         				case ZMENU_LASTMENU:
         					// set the last menu parameter value
                  		zm_option.zm_type.menu 	-1;
                  		p += sizeof(int); // index pointer
         					break;

                     case	ZMENU_SET_FLAG:
                     	// store the pointer value
                     	zm_option.dval = *(int **)p;
                     	p += sizeof(int *);  // index pointer
                     	// store the value to use
                        zm_option.zm_type.flagval = *(int *)p;
                        p += sizeof(int ); // index pointer
                     	break;

         				default: // Must be a Data Entry option
                        if (zm_option.param & ZMENU_LONG)
                        	zm_option.dval = *(long **)p;
                        else if (zm_option.param & ZMENU_FLOAT)
                        	zm_option.dval = *(float **)p;
                        else if (zm_option.param & ZMENU_STRING)
                        	zm_option.dval = *(char **)p;
                        #ifdef IP_H
                        else if (zm_option.param & ZMENU_ADDRESS)
                         	zm_option.dval = *(longword **)p;
                        #endif
                        else if (zm_option.param & ZMENU_DATETIME)
                        	zm_option.dval = *(struct tm **)p;
                        else
                        {
                         	done = 1;
                         	break;
                        }
                        // store the data entry type to use.
                        zm_option.zm_type.dtype = zm_option.param;
                        // store the pointer to use
                  		p += sizeof( char *); // index pointer
                  		// store the number of digits to use
                  		zm_option.digits = *(int *)p;
                  		p += sizeof(int);  // index pointer
         					break;
            		}
            		if (!done)
            		{
                     // Store the lastoption value
            			zm_menu.params.lastoption++;
            			// Store the new data to extended memory
              			_zmenu_write_option( laddr );
            			_zm_write(Menu);
            		}
            	}
            	// an invalid item number was entered, exit function and
            	// return a -1.
            	else done = 1;
      			break;

      		case ZMENU_END:
      			// a proper ending to the function, no errors were detected.
      			// return a 1.
       			retval = 1;
      			done = 1;
      			break;
         	default:
               // an invalid Menu option was entered, exit function and
               // return a -1.
           		done = 1;
           		break;
      	}
   	}
   }
   return retval;
}

// User Function.  The enables/disables the Zmenu_Handler function.
// 1 = Enable, 2 = Disable.
nodebug
void Zmenu_State(int OnOff)
{
 	if (!OnOff)
 	{
      // Set the last state of the current menu.
 		zm_menu.params.menuState = ZMENU_REFRESH;
 		// store this so that it can run the next time the menu is enabled.
 		_zm_write(zm_runmenu);
 		// disable the Zmenu_Handler function
 		zm_state = 0;
 	}
 	else
 	{
      // Enable the Zmenu_Handler function
 		zm_state = 1;
 		// Turn on the Highlight option
		zm_fxns.zmHighlightEnable();
	}
}

// keypad handler
// PAR 1:  the menu number to handle.

nodebug
int Zmenu_Keypad(int MenuNumber)
{
   auto int *offset;
	auto int *highlight;
   auto int maxMenuOptions;
	auto int wKey;
	// setup current highligh, offset, and max menu options for the
	// current menu values.
	offset = &(zm_menu.params.current_offset);
	highlight = &(zm_menu.params.highlight);
   maxMenuOptions = zm_menu.params.lastoption + 1;
	if((wKey = zm_fxns.zmKeyGet()) != 0) // check for a key press
	{
		switch(wKey)
		{
			case ZMENU_PAGE_DN:	// Page down
				if(*offset < (maxMenuOptions))
				{
					if((*offset + zm_menu.params.maxDispOptions) < (maxMenuOptions))
					{
						*offset += zm_menu.params.maxDispOptions;
						if(*offset + zm_menu.params.maxDispOptions > maxMenuOptions-1)
						 	*offset = (maxMenuOptions)- zm_menu.params.maxDispOptions;
					}
				}
				*highlight = zm_menu.params.maxDispOptions-1;
				wKey = -1;
				break;

			case ZMENU_PAGE_UP:	// Page up
				if(*offset > (zm_menu.params.maxDispOptions))
					*offset -=zm_menu.params.maxDispOptions;
				else
					*offset = 0;
				if(*offset == 0)
					*highlight = 0;
				else
					*highlight = 0;

				wKey = -1;
				break;

			case ZMENU_SCROLL_UP:	// Scroll-up by one line
				*highlight -= 1;
				if(*highlight < 0)
				{
					*offset -= 1;
					if(*offset < 0)
						*offset = 0;
					*highlight = 0;
				}
				if(*offset == 0 && *highlight == 0)
				{
					*highlight = 0;
				}
				wKey = -1;
				break;

			case ZMENU_SCROLL_DN:	// Scroll-down by one line
				if((*offset + (*highlight)) < (maxMenuOptions))
				{
					*highlight += 1;
					if(*highlight > (zm_menu.params.maxDispOptions - 1))
					{
						*offset += 1;
						if((*offset + *highlight) > maxMenuOptions)
							*offset -= 1;
						*highlight = zm_menu.params.maxDispOptions-1;
					}
					if(*highlight >= maxMenuOptions-1)
					{
						*highlight = maxMenuOptions-1;
					}
				}
				wKey = -1;
				break;

			case ZMENU_SELECT:	// Select option
				wKey = (*offset+1) + *highlight;
				break;

			default: // a key was pressed, but not one that does anything
						// here.
				wKey = -1;
		  		break;
		}
	}
   return(wKey);
}

// Displays the current menu and appropriate options.
// PAR 1:  The menu number to display.
// PAR 2:  pointer to the state in which it is displayed.
nodebug
int Zmenu_DisplayMenu(int MenuNumber, int *state )
{
   auto int menu_option;
	auto int i, status;
	auto int max_char_lines;
	menu_option = 0;
	switch(*state)
	{
		case 0: // Set menu parameters in menu structure array

         _zm_read(MenuNumber); // read in the menu data.
			zm_menu.params.current_offset = 0; // 0 the current offset
			zm_menu.params.highlight = 0; // move highlight to the first item
			zm_menu.params.new_offset = !zm_menu.params.current_offset;
         *state = 3;  // continue at case 3.
         _zm_write(MenuNumber); // store the new menu data.
			break;

		case 1: 	// Get the user's option
			_zm_read(MenuNumber); // read in the menu data
         // check to see if the keypad option was enabled.
			if (zm_menu.title.param & ZMENU_KEYPAD)
				// get the current option selected (if any)
				menu_option = Zmenu_Keypad(MenuNumber);
			if(menu_option == -1)
			{

  				// Set menu option to zero due to scrolling operation
				menu_option = 0;
				*state = 3;
			}
			// store the new settings
         _zm_write(MenuNumber);
			break;

		case 2: // Refresh menu options
			_zm_read(MenuNumber); // read in the menu data
			zm_fxns.zmLock();           // lock the display (if needed)
         zm_fxns.zmClearMenu();      // clear the current display
         if (zm_menu.title.param & ZMENU_BORDER)
         	zm_fxns.zmBorder();         // place border around menu area (if active)
         zm_fxns.zmPrintTitle(); // display menu title
         // display the menu items
         for(i=0; i < zm_menu.params.maxDispOptions; i++)
			{
            zm_fxns.zmPrintOption(zm_menu.params.current_offset,i);
				if(zm_menu.params.current_offset + 1 > zm_menu.params.lastoption)
				{
					break;
				}
				zm_menu.params.current_offset++;
			}
			// Reset the offset back to the first option displayed
			zm_menu.params.current_offset = zm_menu.params.current_offset-i;
			zm_menu.params.new_offset = zm_menu.params.current_offset;
			// highlight the option current.
			zm_fxns.zmHighlight(zm_menu.params.highlight);
			zm_menu.params.lasthighlight =zm_menu.params.highlight;
			// go back to case 1
			*state = 1;
			// unlock the display (if needed)
			zm_fxns.zmUnlock();
			// store the new menu data.
         _zm_write(MenuNumber);
			break;


		case 3: // Display menu option
			_zm_read(MenuNumber);  // read in the menu data
			zm_fxns.zmLock();            // lock the display (if needed)
			// see if a a different option is now active.
			if(zm_menu.params.current_offset != zm_menu.params.new_offset)
			{
            zm_fxns.zmClearMenu();      // clear the current display
            if (zm_menu.title.param & ZMENU_BORDER)
         		zm_fxns.zmBorder();         // place border around menu area (if active)
         	zm_fxns.zmPrintTitle(); // display menu title
         	// display the item list.
				for(i=0; i < zm_menu.params.maxDispOptions; i++)
				{
               zm_fxns.zmPrintOption( zm_menu.params.current_offset,i);
               if(zm_menu.params.current_offset + 1 > zm_menu.params.lastoption)
					{
						break;
					}
					zm_menu.params.current_offset++;
				}
				// Reset the offset back to the first option displayed
				zm_menu.params.current_offset = zm_menu.params.current_offset-i;
				zm_menu.params.new_offset     = zm_menu.params.current_offset;
				// highlight the active item
				zm_fxns.zmHighlight(zm_menu.params.highlight);
			}
			else
			{
				zm_fxns.zmLock();  // Lock the display ( if needed)
            // unhighlight the old item
         	zm_fxns.zmHighlight(zm_menu.params.lasthighlight);
            // highlight the active item
         	zm_fxns.zmHighlight(zm_menu.params.highlight);
         	zm_fxns.zmUnlock(); // unLock the display ( if needed)
			}
			zm_menu.params.lasthighlight = zm_menu.params.highlight;
			*state = 1;
			zm_fxns.zmUnlock(); // unLock the display ( if needed)
			_zm_write(MenuNumber);  // store new settings.
			break;

		default:
			_zm_read(MenuNumber);// read in the menu data
			*state = 0; // reset the state.
			_zm_write(MenuNumber); // store new settings.
			break;
	}
	return(menu_option);

}

// Controlled by Zmenu_Handler when an option is selected.
// PAR 1.	The option that was just selected.
nodebug
int	Zmenu_TaskOption(int OptionNumber)
{
   static int retval;
   auto int index;
   auto int last;
   static struct tm TimeDate;
   retval = 0;
   // switch on the action item ass. with the option selected.
   _zmenu_read_option(zm_menu.foption_addr);
   if (OptionNumber > 1)
   // find, and get the option selected parameters from xmem.
   for (index = 1; index < OptionNumber ; index++)
   	_zmenu_read_option(zm_option.naddr);
   // Test for what type of option
   if (zm_option.param & ZMENU_FUNCTION)
   {
   	// Run user defined option
   	if ( zm_option.zm_type.fxn() )retval = 1;
   }
   else if (zm_option.param & ZMENU_SUBMENU)
   {

    		// set the menu state to refresh the next time
         zm_menu.params.menuState = ZMENU_REFRESH;
      	_zm_write( zm_runmenu ); // store current menu
      	zm_lastmenu = zm_runmenu; // set the last menu value
      	// set the run menu to be the option selected menu.
      	zm_runmenu = zm_option.zm_type.menu;
      	retval = 1;
   }
   else if (zm_option.param & ZMENU_LASTMENU)
   {
      zm_menu.params.menuState = ZMENU_REFRESH;
      _zm_write( zm_runmenu ); // store current menu
      last =  zm_runmenu; // store the current menu
      zm_runmenu = zm_lastmenu; // get the last menu value
      zm_lastmenu = last; // restore the current menu back.
      retval = 1;
   }
   else if (zm_option.param & ZMENU_SET_FLAG)
   {
      *(int *)zm_option.dval = zm_option.zm_type.flagval;
    		retval = 1;
   }
   else	// Must be a data entry function.  check for which type,
   // waitfor the function to finish, then store the value accordingly.
   {
      	zm_menu.params.menuState = ZMENU_REFRESH;
      	_zm_write( zm_runmenu );

    		if (zm_fxns.zmDataEntry())
    		{
					if (zm_option.param & ZMENU_LONG)
					{
                 	*(long *)zm_option.dval = atol(zm_DataString);
               }
               else if (zm_option.param & ZMENU_FLOAT)
               {
               	*(float *)zm_option.dval = atof(zm_DataString);

               }
               else if (zm_option.param & ZMENU_STRING)
               {

                  memcpy(	zm_option.dval,
                         	zm_DataString,
                           strlen(zm_DataString)+1);
               }
               else if (zm_option.param & ZMENU_ADDRESS)
               {
               #ifdef IP_H
               	*(longword *)zm_option.dval = resolve(zm_DataString);
               	inet_ntoa (zm_DataString,*(longword *)zm_option.dval);
         		#endif
               }
               else if (zm_option.param & ZMENU_DATETIME)
               {
               	TimeDate.tm_mday = atoi(zm_DataString);
               	TimeDate.tm_mon  = atoi(zm_DataString + 3);
               	TimeDate.tm_year = atoi(zm_DataString + 6) - 1900;
               	TimeDate.tm_hour = atoi(zm_DataString + 11);
               	TimeDate.tm_min  = atoi(zm_DataString + 14);
               	TimeDate.tm_sec  = atoi(zm_DataString + 17);
               	memcpy(*(struct tm **)&zm_option.dval,&TimeDate,sizeof(TimeDate));
               }
    		 		retval = 1;
   		}
   }

   return retval;
}

// User function for handling the Zmenu control.
// PAR 1:	Pointer to store The last menu selected.
// PAR 2:   Pointer to The last option selected.
// RET Val. 1 if an option has just been selected. 0 if not.
nodebug
int Zmenu_Handler(int *Menu, int *Option)
{
	static int state,menuOpt,firstTry, retval;
	#GLOBAL_INIT {state = 0;firstTry = 1;}
	if (firstTry)
	{
	 // setup the Startmenu, at power up or first time through
    zm_runmenu 		= zm_startmenu;
	 zm_lastmenu		= zm_startmenu;
	 *Menu = *Option = 0;
	 firstTry = 0;
	 retval = 0;
	}
		// check that the menuhandler is enabled.
	if (zm_state)
	{
		// check that the keypad is enabled.
      if (zm_menu.title.param & ZMENU_KEYPAD)
      {
      	costate
      	{
        		waitfor(DelayMs(10));
        		zm_fxns.zmKeyProcess();
      	}
      }
      //
		switch (state)
		{
      	case 0: // display the menu.
      		retval = 0;
           	if ((menuOpt = Zmenu_DisplayMenu(zm_runmenu,
            										  &zm_menu.params.menuState) ) != 0)

           	{
               *Menu 	= zm_runmenu;
               *Option 	= menuOpt;
   				state++;
            }
	      	break;
	      case 1: // do the action that was setup.
	      	if (Zmenu_TaskOption(menuOpt))
	      	{
               retval 	= 1;
	      		state 	= 0;
            }
	      	break;
		}
	}
	return retval;
}
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
	fun.c

	This sample program is for the LCD MSCG12232 Display Module.

   NOTE: Not currently supported on RCM4xxx modules.

  	This program demonstrates the drawing primitive features of the
  	graphic library (i.e. lines, circles, polygons). Also demo's the
  	keypad with the key release option.

  	Instructions:
  	-------------
  	1. Run and compile this program.
  	2. Watch the LCD display as it goes through the various graphic demo's.
  	3. At any given time press a key on the controller keypad to see the
  	   corresponding LED light-up.

	Note: When this sample program is ran on a OP6800 controller, press
	      SW1-SW4 for a corresponding LED to light-up. The Switch to LED
	      mapping is as follows:

	      OP6800 Demo Board
	      -----------------
	      SW1 -> DS4
	      SW2 -> DS3
	      SW3 -> DS2
	      SW4 -> DS1

**************************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

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

// Init   : glXFontInit ( &chinese,9,13,0x20,0x22,Welcome );
xdata Welcome {
// Char = ' ' 0x20
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',

// ying_bmp	0x21
'\x20','\x00',
'\x10','\xC0',
'\x1B','\x3E',
'\x12','\x22',
'\x02','\x22',
'\x02','\x22',
'\x72','\x22',
'\x12','\x22',
'\x12','\xA2',
'\x13','\x2A',
'\x12','\x24',
'\x10','\x20',
'\x10','\x20',
'\x28','\x00',
'\x47','\xFF',
'\x00','\x00',

// Huan_bmp 0x22
'\x00','\x80',
'\x00','\x80',
'\xFC','\x80',
'\x05','\xFE',
'\x85','\x04',
'\x4A','\x48',
'\x28','\x40',
'\x10','\x40',
'\x18','\x40',
'\x18','\x60',
'\x24','\xA0',
'\x24','\x90',
'\x41','\x18',
'\x86','\x0E',
'\x38','\x04',
'\x00','\x00',

// Char = '.' 0x23
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\x00',
'\x00','\xC0',
'\x00','\x00'
};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Bitmap : rs1_bmp
// Buffer Size : 75
// Monochrome  : White Foreground, Black Background
// Mode   : Landscape
// Height : 15 pixels.
// Width  : 39 pixels.
// Init   : glXPutBitmap (leftedge,topedge,39,15,rs1_bmp);

xdata rs1_bmp {
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x18','\x00','\xFC','\x01',
'\x00','\x3E','\x01','\xFF','\x01',
'\x1E','\x7F','\xE0','\x1F','\xE1',
'\x3F','\xFF','\xFF','\xFF','\xF1',
'\x3F','\xFF','\xFF','\xFF','\xF1',
'\x3F','\xFF','\xFF','\xFF','\xE1',
'\x00','\x1F','\xFF','\xFC','\x01',
'\x00','\x00','\x0F','\xFF','\xF1',
'\x00','\x00','\x00','\x3F','\xF1',
'\x00','\x00','\x00','\x07','\xE1',
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01'
};
// Bitmap : rs2_bmp
// Buffer Size : 75
// Monochrome  : White Foreground, Black Background
// Mode   : Landscape
// Height : 15 pixels.
// Width  : 39 pixels.
// Init   : glXPutBitmap (leftedge,topedge,39,15,rs2_bmp);

xdata rs2_bmp {
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x7E','\x00','\x30','\x01',
'\x01','\xFF','\x00','\xF8','\x01',
'\x0F','\xF0','\x0F','\xFC','\xF1',
'\x1F','\xFF','\xFF','\xFF','\xF9',
'\x1F','\xFF','\xFF','\xFF','\xF9',
'\x0F','\xFF','\xFF','\xFF','\xF9',
'\x00','\x7F','\xFF','\xF0','\x01',
'\x1F','\xFF','\xE0','\x00','\x01',
'\x1F','\xF8','\x00','\x00','\x01',
'\x0F','\xC0','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01',
'\x00','\x00','\x00','\x00','\x01'
};


int DemoBd_switch;
int Switches_Active;
int Keypad_Active;

// Structure to hold font information
fontInfo fi6x8, fi8x10, fi11x16, fichinese;

//------------------------------------------------------------------------
// Milli-sec delay function
//------------------------------------------------------------------------
// General use msDelay function
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + (unsigned long)delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

//------------------------------------------------------------------------
// Milli-sec delay function that also monitors the keypad
//------------------------------------------------------------------------
nodebug
int key_msDelay(int delay)
{
	auto int loop;

	for(loop=0; loop < delay; loop++)
	{
		msDelay(1);
		keyProcess ();
		if(keyGet() != 0)
		{
			return(TRUE);
		}
	}
}

//------------------------------------------------------------------------
// Scroll the Welcome Message
//------------------------------------------------------------------------
nodebug
int scroll_welcome(int x, int y, fontInfo *pInfo, char *ptr, int numBitScroll)
{
	static int i, loop, scroll, status;

	status = 0;
	costate
	{
		scroll = LCD_XS - (strlen(ptr)*abs(numBitScroll));
		for(i=0; i < strlen(ptr); i++)
		{
			glHScroll(0, 16, LCD_XS, 16, numBitScroll);
			waitfor(DelayMs(30));
			glPrintf (LCD_XS+numBitScroll,  y, pInfo, "%c", ptr[i]);
			waitfor(DelayMs(30));
		}
		glHScroll(0, 16, LCD_XS, 16, -scroll);
		waitfor(DelayMs(2500));
		status = 1;
	}
	return(status);
}

//------------------------------------------------------------------------
// Display Welcome Message
//------------------------------------------------------------------------
nodebug
int WelcomeMessage(void)
{
	static int i, status;

	status = 0;
	costate
	{
		glBlankScreen();
		glPrintf(0,0, &fi11x16 , "Welcome...");
		for(i=0; i < 4; i++)
		{
			switch(i)
			{
				case 0:
					waitfor(scroll_welcome(0, 16, &fi11x16, "Willkommen", -11));
					break;
				case 1:
					waitfor(scroll_welcome(0, 16, &fi11x16, "Bienvenidos", -11));
					break;
				case 2:
					waitfor(scroll_welcome(0, 16, &fichinese, "\x22\x21", -16));
					break;
				case 3:
					waitfor(scroll_welcome(0, 16, &fi11x16, "to Z-World!", -11));
					break;
			}
		}

		for(i=0; i<32;i++)
		{
			glVScroll(0, 0, LCD_XS, 32, -1);
			waitfor(DelayMs(30));
		}

		// Delay for 100ms for visual effects
		waitfor(DelayMs(100));

		// Check for keypress to exit welcome message
		glBlankScreen();
		status = 1;
	}
	return(status);
}

//------------------------------------------------------------------------
// Function to read the DEMO board switches
//------------------------------------------------------------------------
nodebug
void monitor_switches(void)
{
	if(!digIn(0) ||!digIn(1) || !digIn(2) || !digIn(3))
	{
		Switches_Active = TRUE;
		if(!digIn(0))
		{
			DemoBd_switch = 0;
		}
		if(!digIn(1))
		{
			DemoBd_switch = 1;
		}
		if(!digIn(2))
		{
			DemoBd_switch = 2;
		}
		if(!digIn(3))
		{
			DemoBd_switch = 3;
		}
	}
	else
	{
		Switches_Active = FALSE;
	}
}

//------------------------------------------------------------------------
// Function to demonstrate DEMO board switches and LED's
//------------------------------------------------------------------------
nodebug
void DemoBd_Demo(void)
{
	static int DemoBd_increment, DemoBd_mask, DemoBd_led, channel;
	#GLOBAL_INIT{Switches_Active = FALSE;}

	costate
	{
		monitor_switches();
		if(Switches_Active)
		{
			switch(DemoBd_switch)
			{
				case 0:
					channel = 10;
					digOut(channel, 0);
					break;

				case 1:
					channel = 9;
					digOut(channel, 0);
					break;

				case 2:
					channel = 8;
					digOut(channel, 0);
					break;

				case 3:
					channel = 7;
					digOut(channel, 0);
					break;
			}
			for(DemoBd_led = 0; DemoBd_led <= 3; DemoBd_led++)
			{
				if(DemoBd_led+7 != channel)
					digOut(DemoBd_led+7, 1);
			}
		}
		else
		{
			DemoBd_mask = 0x01;
			DemoBd_increment++;
			for(DemoBd_led = 0; DemoBd_led <= 3; DemoBd_led++)
			{
				if(DemoBd_increment & DemoBd_mask)
					digOut(DemoBd_led+7, 0);
				else
					digOut(DemoBd_led+7, 1);
				DemoBd_mask = DemoBd_mask << 1;
			}
			waitfor(DelayMs(40));
		}
	}
}

//------------------------------------------------------------------------
// Function to demonstrate controller keypad and LED's
//------------------------------------------------------------------------
nodebug
void Keypad_Demo(void)
{
	static int led, mask, channel, wKey, keypad_active, update_led, release_value;
	#GLOBAL_INIT {keypad_active= FALSE;}
	#GLOBAL_INIT {update_led = TRUE;}


	costate
	{
		keyProcess ();
		waitfor ( DelayMs(10) );
	}

	costate
	{
		// Wait for any key to be pressed
		waitfor((wKey = keyGet()));
		release_value = -1;
		switch(wKey)
		{
				case 'L': release_value = '7'; break;
				case '-': release_value = '6'; break;
				case 'U': release_value = '5'; break;
				case '+': release_value = '4'; break;
				case 'D': release_value = '3'; break;
				case 'E': release_value = '2'; break;
				case 'R': release_value = '1'; break;
		}
		if(release_value != -1)
		{
			// Set flag to indicate a key is being pressed
			keypad_active = TRUE;

			// Wait for the key to be released
			waitfor(keyGet() == release_value);
			keypad_active = FALSE;
		}
	}

	costate
	{
		if(!keypad_active)
		{
			for(channel = 0; channel <= 6; channel++)
			{
				for(led = 0; led <=6; led++)
				{
					if(led != channel)
						dispLedOut(led, 0);
				}
				dispLedOut(channel, 1);
				waitfor(DelayMs(100));
				if(keypad_active)
				{
					break;
				}
			}
			update_led = TRUE;
		}
	}
	costate
	{
		if(keypad_active)
		{
			if(update_led)
			{
				switch(wKey)
				{
					case 'L':
						dispLedOut(0, 1);
						channel = 0;
						break;
					case '-':
						dispLedOut(1, 1);
						channel = 1;
						break;

					case 'U':
						dispLedOut(2, 1);
						channel = 2;
						break;

					case '+':
						dispLedOut(3, 1);
						channel = 3;
						break;

					case 'D':
						dispLedOut(4, 1);
						channel = 4;
						break;

					case 'E':
						dispLedOut(5, 1);
						channel = 5;
						break;

					case 'R':
						dispLedOut(6, 1);
						channel = 6;
						break;
				}
			}
			for(led=0; led <=6; led++)
			{
				if(led != channel)
				{
					dispLedOut(led, 0);
				}
			}
			update_led = FALSE;
		}
	}
}


//------------------------------------------------------------------------
// Main()
// Demonstrate drawing features of the graphic library (i.e. lines,
// circles, polygons).
//------------------------------------------------------------------------
main()
{
	static int loop, i, j, y, wKey;
	static const char str1[] = {"Rabbit Powered"};
	static const char str2[] = {"  Displays"};
	static const char ptr [] = {"Sample Program....FUN.C!!!           "};

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------
	brdInit();

	// Start-up the keypad driver
	// Initialize the LCD display
	dispInit();

	// Use default key values along a key release code
	keyConfig (  3,'R', '1', 0, 0,  0, 0 );
	keyConfig (  6,'E', '2', 0, 0,  0, 0 );
	keyConfig (  2,'D', '3', 0, 0,  0, 0 );
	keyConfig (  5,'+', '4', 0, 0,  0, 0 );
	keyConfig (  1,'U', '5', 0, 0,  0, 0 );
	keyConfig (  4,'-', '6', 0, 0,  0, 0 );
	keyConfig (  0,'L', '7', 0, 0,  0, 0 );


	// Initial the font information structure
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);				//	Initialize 6x8 font
	glXFontInit(&fi8x10, 8, 10, 32, 127, Font8x10);			//	Initialize 8x10 font
	glXFontInit(&fi11x16,11,16,0x20,0xFF, Courier12);		// Initialize 11x16 Courier font
	glXFontInit(&fichinese, 16, 16, 0x20, 0x23, Welcome);	//	Initialize with chinese font

	//------------------------------------------------------------------------
	// Continuous loop to demonstrate graphic functions
	//------------------------------------------------------------------------
	for(;;)
	{

		//------------------------------------------------------------------------
		// Costate to demonstrate display keypad and LED's
		//------------------------------------------------------------------------
		costate
		{
			Keypad_Demo();
		}

		//------------------------------------------------------------------------
		// Costate to demonstrate LED's and switches on the OP6800 Demo board
		//------------------------------------------------------------------------
		costate
		{
			if(_BOARD_TYPE_ == OP6800)
			{
				DemoBd_Demo();
			}
		}
		//------------------------------------------------------------------------
		// Costate to demonstrate the graphic primitives
		//------------------------------------------------------------------------
		costate
		{
			// Display the welcome message and wait for keypress
			waitfor(WelcomeMessage());

			// Display Z-World logo
			glBlankScreen();
			glXPutBitmap (0,0,53,29,Zwbw5_bmp);
			for(i=0; ptr[i] != '\0'; i++)
			{
				glHScroll(50, 0, LCD_XS, 16, -6);
				glPrintf (116,  4,   &fi6x8, "%c", ptr[i]);
				waitfor(DelayMs(100));
			}
			// With using the line algorithm draw a circular pattern, with
			// using the buffer LOCK/UNLOCK speed feature.
			glBlankScreen();
			glBuffLock();
			for(j = 0; j < LCD_YS; j++)
			{
				glPlotLine(0, j, LCD_XS - 1, LCD_YS - 1 - j);

				if((j % 6) == 0)
				{
					glBuffUnlock();
					glBuffLock();
					waitfor(DelayMs(5));
				}
			}
			for(i = LCD_XS - 1; i > 0; --i)
			{
				glPlotLine(i, 0, LCD_XS - i, LCD_YS - 1);
				if((i % 10) == 0)
				{
					glBuffUnlock();
					glBuffLock();
					waitfor(DelayMs(5));
				}
			}
			glBuffUnlock();
			waitfor(DelayMs(300));

			// Plot circle
			glBlankScreen();
			glPlotCircle (60, 16, 14);
			waitfor(DelayMs(750));

			// Fill the circle that was just plotted
			glFillCircle (60, 16, 14);
			waitfor(DelayMs(500));


			// Draw a series of circles (going from small to large)
			glBlankScreen();
			for (y = 1; y < 60-4; y+=4)
			{
				glPlotCircle(60,16,y);
				waitfor(DelayMs(200));
			}

			// Draw a series of circles (going from large to small)
			glBlankScreen();
			for (y = (60-4);  y >= 0; y-=4)
			{
				glPlotCircle(60,16,y);
				waitfor(DelayMs(20));
			}

			// Draw a series of polygons (going from large to small)
			glBlankScreen();
			for (y = 1; y < 16; y+=2)
			{
				glPlotPolygon(4, 122-y,y,  y,y,  y,32-y,  122-y,32-y);
				waitfor(DelayMs(300));
			}

			// Plot 4 polygons using the buffer LOCK/UNLOCK feature to speed-up
			// the display process.
			glBlankScreen();
			glBuffLock();
			glPlotPolygon(4, 0, 0,   16,0,    16,15,   0, 15);
			glPlotPolygon(4, 0, 16,  16,16,   16,31,   0, 31);
			glPlotPolygon(4, 105, 0,   121,0,    121,15,   105, 15);
			glPlotPolygon(4, 105, 16,  121,16,   121,31,   105, 31);
			glBuffUnlock();
			waitfor(DelayMs(700));

			// Fill the polygons with using the buffer LOCK/UNLOCK speed feature
			glBuffLock();
			glFillPolygon(4, 0, 0,   16,0,    16,15,   0, 15);
			glFillPolygon(4, 0, 16,  16,16,   16,31,   0, 31);
			glFillPolygon(4, 105, 0,   121,0,    121,15,   105, 15);
			glFillPolygon(4, 105, 16,  121,16,   121,31,   105, 31);
			glBuffUnlock();
			waitfor(DelayMs(700));

			// Demonstrate scrolling bitmap
			glBlankScreen();
			glXPutBitmap (83,0,39,15,rs2_bmp);
			glHScroll(0, 0, 122, 16, -5);
			for (i=0; i<strlen(str1); i++)
			{
				glHScroll(0, 0, 122, 16, -5);
				glPrintf (114, 4,  &fi6x8, "%c", str1[i]);
				waitfor(DelayMs(5));
			}
			waitfor(DelayMs(1000));

			glXPutBitmap (0,16,39,15,rs1_bmp);
			glHScroll(0, 16, 122, 16, 8);
			for (i=strlen(str2)-1; i>=0; i--)
			{
				glHScroll(0, 16, 122, 16, 6);
				glPrintf (0, 20, &fi6x8, "%c", str2[i]);
				waitfor(DelayMs(5));
			}
			waitfor(DelayMs(2500));
			glBlankScreen();
		}
	}
}
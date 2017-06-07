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
/*******************************************************************************
	lcdkeyfun.c

	This program is used with RCM6xxx series MiniCore modules and the RCM3000
	prototyping board wired to the MiniCore Development Board.
	
	Bridge the following pins on the two boards:
	PA0 to PA7, PB2 to PB5, PE3*, PE6, /IORD, /RESET and GND

	* Simply grounding PE3 on the RCM3000 prototyping board will also work.
	
	The brdInit() function in RCM66xxW.lib enables the external I/O bus for
	LCD/Keypad operations.

	Description
	===========
	This program demonstrates the drawing primitive features of the graphics
	library (i.e. lines, circles, polygons).  Also demonstrated is the keypad
	with the key release option and prototyping board switches and LEDs control.

	Instructions
	============
	1. Compile and run this program.
	2. Watch the LCD display as it goes through the various graphics primitives
	   demonstrations.
	3. Press a key on the LCD/Keypad Module to see the corresponding LCD/Keypad
	   Module LED light up.
*******************************************************************************/

#memmap xmem
#class auto
#define USE_DISPLAY_KEYPAD
#define PORTA_AUX_IO		//required to run LCD/Keypad for this demo
#use "RCM66xxW.LIB"	// sample library to use with this sample program

// Bitmap : digi_1c_bmp
// Buffer Size : 160
// Monochrome  : Black Foreground, White Background
// Mode   : Landscape
// Height : 32 pixels.
// Width  : 35 pixels.
// Init   : glXPutBitmap (leftedge,topedge,35,32,digi_1c_bmp);

xdata digi_1c_bmp {
'\x00','\x00','\x00','\xC0','\x00',
'\x00','\x00','\x03','\xC0','\x00',
'\x00','\x00','\x0F','\xE0','\x00',
'\x00','\x00','\x3F','\xF0','\x00',
'\x00','\x00','\xFF','\xF0','\x00',
'\x00','\x03','\xFF','\xF0','\x00',
'\x00','\x0F','\xFF','\xF8','\x00',
'\x00','\x3F','\xFF','\xF8','\x00',
'\x01','\xFF','\xFF','\xFC','\x00',
'\x07','\xFF','\xFF','\xFC','\x00',
'\x10','\x3F','\xFF','\xFE','\x00',
'\x70','\x07','\xFF','\xFE','\x00',
'\xF0','\x03','\xFF','\xFF','\x60',
'\xF0','\xE1','\x1C','\x08','\x40',
'\x70','\xE1','\x18','\x08','\x80',
'\x70','\xE1','\x11','\x88','\x80',
'\x30','\xE1','\x11','\x88','\xC0',
'\x30','\xE1','\x11','\x88','\xC0',
'\x10','\xE1','\x11','\x88','\xE0',
'\x10','\x03','\x10','\x08','\xE0',
'\x10','\x07','\x18','\x08','\xC0',
'\x00','\x0F','\x1F','\xCE','\x00',
'\x0F','\xFF','\xF0','\x88','\x00',
'\x07','\xFF','\xF8','\x10','\x00',
'\x07','\xFF','\xFF','\x80','\x00',
'\x03','\xFF','\xFE','\x00','\x00',
'\x03','\xFF','\xF8','\x00','\x00',
'\x01','\xFF','\xE0','\x00','\x00',
'\x01','\xFF','\x80','\x00','\x00',
'\x00','\xFE','\x00','\x00','\x00',
'\x00','\xF8','\x00','\x00','\x00',
'\x00','\x60','\x00','\x00','\x00'
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

int ProtoBd_switch, swstate1, swstate2;
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

	done_time = MS_TIMER + delay;
	while ((long) (MS_TIMER - done_time) < 0);
}

//------------------------------------------------------------------------
// Milli-sec delay function that also monitors the keypad
//------------------------------------------------------------------------
nodebug
int key_msDelay(int delay)
{
	auto int loop;

	for (loop = 0; loop < delay; ++loop) {
		msDelay(1);
		keyProcess();
		if (keyGet() != 0) {
			return TRUE;
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
	costate {
		scroll = LCD_XS - (strlen(ptr) * abs(numBitScroll));
		for (i = 0; i < strlen(ptr); ++i) {
			glHScroll(0, LCD_YS >> 1, LCD_XS, LCD_YS >> 1, numBitScroll);
			waitfor(DelayMs(30));
			glPrintf(LCD_XS + numBitScroll, y, pInfo, "%c", ptr[i]);
			waitfor(DelayMs(30));
		}
		glHScroll(0, LCD_YS >> 1, LCD_XS, LCD_YS >> 1, -scroll);
		status = 1;
	}
	return status;
}

//------------------------------------------------------------------------
// Display Welcome Message
//------------------------------------------------------------------------
nodebug
int WelcomeMessage(void)
{
	static int i;
	auto int status;

	status = 0;
	costate {
		glBlankScreen();
		glPrintf(0, 0, &fi11x16 , "Welcome...");
		for (i = 0; i < 4; ++i) {
			waitfor(scroll_welcome(0, LCD_YS >> 1, &fi11x16, "           ", -11));
			switch (i) {
			case 0:
				waitfor(scroll_welcome(0, LCD_YS >> 1, &fi11x16, "Willkommen ", -11));
				break;
			case 1:
				waitfor(scroll_welcome(0, LCD_YS >> 1, &fi11x16, "Bienvenidos", -11));
				break;
			case 2:
				waitfor(scroll_welcome(0, LCD_YS >> 1, &fichinese, "\x22\x21     ", -16));
				break;
			case 3:
				waitfor(scroll_welcome(0, LCD_YS >> 1, &fi11x16, " to Rabbit!", -11));
				break;
			}
			waitfor(DelayMs(2500));
		}

		for (i = 0; i < LCD_YS; ++i) {
			glVScroll(0, 0, LCD_XS, LCD_YS, -1);
			waitfor(DelayMs(30));
		}

		// Delay for 100ms for visual effects
		waitfor(DelayMs(100));

		// Check for keypress to exit welcome message
		glBlankScreen();
		status = 1;
	}
	return status;
}

//------------------------------------------------------------------------
// Function to demonstrate LCD/Keypad Module keypad and LEDs
//------------------------------------------------------------------------
nodebug
void Keypad_Demo(void)
{
	static int led, mask, channel, wKey, keypad_active, update_led, release_value;

#GLOBAL_INIT { keypad_active = FALSE; }
#GLOBAL_INIT { update_led = TRUE; }

	costate {
		keyProcess();
		waitfor(DelayMs(10));
	}

	costate {
		// Wait for any key to be pressed
		waitfor((wKey = keyGet()));
		release_value = -1;
		switch (wKey) {
			case 'L': release_value = '7'; break;
			case '-': release_value = '6'; break;
			case 'U': release_value = '5'; break;
			case '+': release_value = '4'; break;
			case 'D': release_value = '3'; break;
			case 'E': release_value = '2'; break;
			case 'R': release_value = '1'; break;
		}
		if (release_value != -1) {
			printf("Keypress '%c', waiting for release...", wKey);
			// Set flag to indicate a key is being pressed
			keypad_active = TRUE;
			// Wait for the key to be released
			waitfor(keyGet() == release_value);
			printf("done\n");
			keypad_active = FALSE;
		}
	}

	costate {
		if (!keypad_active) {
			for (channel = 0; channel <= 6; ++channel) {
				for (led = 0; led <= 6; ++led) {
					if (led != channel) dispLedOut(led, 0);
				}
				dispLedOut(channel, 1);
				waitfor(DelayMs(100));
				if (keypad_active) {
					break;
				}
			}
			update_led = TRUE;
		}
	}

	costate {
		if (keypad_active) {
			if (update_led) {
				switch (wKey) {
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
			for (led = 0; led <= 6; ++led) {
				if (led != channel) dispLedOut(led, 0);
			}
			update_led = FALSE;
		}
	}
}

//------------------------------------------------------------------------
// void main(void)
// Demonstrate drawing features of the graphic library (i.e. lines,
// circles, polygons).
//------------------------------------------------------------------------
void main(void)
{
	static int buflock_toggle, loop, i, j, y, wKey;
	static const char str1[] = {"Rabbit Powered "};
	static const char str2[] = {"   Displays"};
	static const char ptr[] = {"Sample program... lcdkeyfun.c!                "};

#GLOBAL_INIT { buflock_toggle = 0; }

	//------------------------------------------------------------------------
	// initialize the controller
	//------------------------------------------------------------------------

	// it's just good practice to initialize Rabbit's board-specific I/O
	brdInit();

	// initialize the LCD display
	dispInit();
	glBackLight(1);	// backlight on

	// start-up the keypad driver
	// use default key values along with a key release code
	keyConfig (  3,'R', '1', 0, 0,  0, 0 );
	keyConfig (  6,'E', '2', 0, 0,  0, 0 );
	keyConfig (  2,'D', '3', 0, 0,  0, 0 );
	keyConfig (  5,'+', '4', 0, 0,  0, 0 );
	keyConfig (  1,'U', '5', 0, 0,  0, 0 );
	keyConfig (  4,'-', '6', 0, 0,  0, 0 );
	keyConfig (  0,'L', '7', 0, 0,  0, 0 );

	// initialize the font information structures
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);			//	Initialize 6x8 font
	glXFontInit(&fi8x10, 8, 10, 32, 127, Font8x10);		//	Initialize 8x10 font
	glXFontInit(&fi11x16, 11, 16, 32, 255, Courier12);	// Initialize 11x16 Courier font
	glXFontInit(&fichinese, 16, 16, 32, 35, Welcome);	//	Initialize with chinese font

	//------------------------------------------------------------------------
	// continuous loop to demonstrate graphic functions
	//------------------------------------------------------------------------
	for (;;) {

		//------------------------------------------------------------------------
		// costate to demonstrate LCD/Keypad Module Keypad and LEDs
		//------------------------------------------------------------------------
		costate {
			Keypad_Demo();
		}

		//------------------------------------------------------------------------
		// costate to demonstrate the graphic primitives
		//------------------------------------------------------------------------
		costate {
			// display the welcome message
			waitfor(WelcomeMessage());

			// display Digi logo
			glBlankScreen();
			glXPutBitmap (0, 0, 35, 32, digi_1c_bmp);
			for (i = 0; ptr[i] != '\0'; ++i) {
				glHScroll(48, 0, LCD_XS, LCD_YS, -6);
				glPrintf (116,  12,   &fi6x8, "%c", ptr[i]);
				waitfor(DelayMs(100));
			}
			waitfor(DelayMs(100));

			// toggle the LCD buffer lock enable status
			//  (graphics display speed will alternate between slower and faster)
			glBlankScreen();
			buflock_toggle = !buflock_toggle;
			glPrintf(1, 1, &fi6x8, "Graphics buffer lock");
			glPrintf(1, 12, &fi6x8, "and unlock feature");
			glPrintf(1, 23, &fi6x8, "is now %sabled.",
			         buflock_toggle ? "en" : "dis");
			waitfor(DelayMs(1500));

			// using the line algorithm draw a circular pattern, also
			// demonstrating the buffer LOCK/UNLOCK speed feature
			glBlankScreen();
			if (buflock_toggle) glBuffLock();
			for (j = 0; j < LCD_YS; ++j) {
				glPlotLine(0, j, LCD_XS - 1, LCD_YS - 1 - j);
				if (j % 6 == 0) {
					if (buflock_toggle) {
						glBuffUnlock();
						glBuffLock();
					}
				}
			}
			for (i = LCD_XS - 1; i > 0; ++j, --i) {
				glPlotLine(i, 0, LCD_XS - i, LCD_YS - 1);
				if (j % 6 == 0) {
					if (buflock_toggle) {
						glBuffUnlock();
						glBuffLock();
					}
				}
			}
			if (buflock_toggle) glBuffUnlock();
			waitfor(DelayMs(300));

			// plot circle
			glBlankScreen();
			glPlotCircle (LCD_XS >> 1, LCD_YS >> 1, 14);
			waitfor(DelayMs(500));

			// fill the circle that was just plotted
			glFillCircle (LCD_XS >> 1, LCD_YS >> 1, 14);
			waitfor(DelayMs(500));


			// draw a series of circles (going from small to large)
			glBlankScreen();
			if (buflock_toggle) glBuffLock();
			for (y = 1; y < LCD_XS >> 1 - 4; ++y) {
				glPlotCircle(LCD_XS >> 1, LCD_YS >> 1, y);
				if (y % 10 == 0) {
					if (buflock_toggle) {
						glBuffUnlock();
						glBuffLock();
					}
				}
			}
			if (buflock_toggle) glBuffUnlock();
			waitfor(DelayMs(500));

			// draw a series of circles (going from large to small)
			glBlankScreen();
			if (buflock_toggle) glBuffLock();
			for (y = LCD_XS >> 1 - 4;  y > 0; --y) {
				glPlotCircle(LCD_XS >> 1, LCD_YS >> 1, y);
				if (y % 6 == 0) {
					if (buflock_toggle) {
						glBuffUnlock();
						glBuffLock();
					}
				}
			}
			if (buflock_toggle) glBuffUnlock();
			waitfor(DelayMs(500));

			// draw a series of polygons (going from large to small)
			glBlankScreen();
			for (y = 1; y < LCD_YS >> 1; ++y) {
				glPlotPolygon(4, LCD_XS - y, y, y, y, y, LCD_YS - y, LCD_XS - y, LCD_YS - y);
				if (y % 6 == 0) {
					if (buflock_toggle) {
						glBuffUnlock();
						glBuffLock();
					}
				}
			}
			waitfor(DelayMs(500));

			// plot some polygons
			glBlankScreen();
			if (buflock_toggle) glBuffLock();
			for (j = 0, i = 0; i < LCD_YS - LCD_YS / 6; i += LCD_YS / 6) {
				for (y = 0; y < LCD_XS - (LCD_YS >> 2); ++j, y += LCD_YS >> 2) {
					glPlotPolygon(4,
					              y, i,
					              y + (LCD_YS >> 2) - 2, i,
					              y + (LCD_YS >> 2) - 2, i + LCD_YS / 6 - 2,
					              y, i + LCD_YS / 6 - 2);
					if (0 == j % 6) {
						if (buflock_toggle) {
							glBuffUnlock();
							glBuffLock();
						}
					}
				}
			}
			if (buflock_toggle) glBuffUnlock();
			waitfor(DelayMs(500));

			// fill the polygons
			if (buflock_toggle) glBuffLock();
			for (j = 0, i = 0; i < LCD_YS - LCD_YS / 6; i += LCD_YS / 6) {
				for (y = 0; y < LCD_XS - (LCD_YS >> 2); ++j, y += LCD_YS >> 2) {
					glFillPolygon(4,
					              y, i,
					              y + (LCD_YS >> 2) - 2, i,
					              y + (LCD_YS >> 2) - 2, i + LCD_YS / 6 - 2,
					              y, i + LCD_YS / 6 - 2);
					if (0 == j % 10) {
						if (buflock_toggle) {
							glBuffUnlock();
							glBuffLock();
						}
					}
				}
			}
			if (buflock_toggle) glBuffUnlock();
			waitfor(DelayMs(500));

			// demonstrate left-scrolling bitmap
			glBlankScreen();
			glXPutBitmap(83, 0, 39, 15, rs2_bmp);
			glHScroll(0, 0, LCD_XS, LCD_YS >> 1, -5);
			for (i = 0; i < strlen(str1); ++i) {
				glHScroll(0, 0, LCD_XS, LCD_YS >> 1, -5);
				glPrintf (114, 4, &fi6x8, "%c", str1[i]);
				waitfor(DelayMs(5));
			}
			waitfor(DelayMs(500));

			// demonstrate right-scrolling bitmap
			glXPutBitmap (0, LCD_YS >> 1, 39, 15, rs1_bmp);
			glHScroll(0, LCD_YS >> 1, LCD_XS, LCD_YS >> 1, 8);
			for (i = strlen(str2) - 1; i >= 0; --i) {
				glHScroll(0, LCD_YS >> 1, LCD_XS, LCD_YS >> 1, 6);
				glPrintf (0, 20, &fi6x8, "%c", str2[i]);
				waitfor(DelayMs(5));
			}
			waitfor(DelayMs(2500));
			glBlankScreen();
		}
	}
}
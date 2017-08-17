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
	keypadtoled.c

	This program is used with RCM6xxx series MiniCore modules and the RCM3000
	prototyping board wired to the MiniCore Development Board.
	
	Bridge the following pins on the two boards:
	PA0 to PA7, PB2 to PB5, PE3*, PE6, /IORD, /RESET and GND

	* Simply grounding PE3 on the RCM3000 prototyping board will also work.
	
	The brdInit() function in RCM66xxW.lib enables the external I/O bus for
	LCD/Keypad operations.

	Description
	===========
	This sample program demonstrates the use of the external I/O bus with the
	controller using an LCD/Keypad Module.  When an LCD/Keypad Module keypress is
	detected, the program lights up an LED on the LCD/Keypad Module and displays
	a message on the LCD.

	Instructions
	============
	1. Compile and run this program.
	2. Press and hold a key on the LCD/Keypad Module to see a message displayed
	   on the LCD and light up an LED.  Here's the list of what key
	   controls which LED(s):

	   Keypad (L to R)   LCD/Keypad LED
	   ---------------   --------------
	   Scroll-Left          DS1
	   Page-Down            DS2
	   Scroll-Up            DS3
	   Page-Up              DS4
	   Scroll-Down          DS5
	   Enter                DS6
	   Scroll-Right         DS7
*******************************************************************************/

#class auto
#memmap xmem
#define USE_DISPLAY_KEYPAD
#define PORTA_AUX_IO		//required to run LCD/Keypad for this demo
#use "RCM66xxW.lib"	// sample library to use with this sample program

#define ON	1
#define OFF 0

// Structure to hold font information
fontInfo fi6x8, fi8x10;

void main(void)
{
	static int channel, i, keypad_active, led, new_keypress_message;
	static int prompt_displayed, release_value, wKey;

	//------------------------------------------------------------------------
	// Initialize the controller
	//------------------------------------------------------------------------

	// it's just good practice to initialize Rabbit's board-specific I/O
	brdInit();

	// start up the keypad driver and initialize the graphic driver
	dispInit();

	// use default key values along with a key release code
	keyConfig(3, 'R', '1', 0, 0,  0, 0);
	keyConfig(6, 'E', '2', 0, 0,  0, 0);
	keyConfig(2, 'D', '3', 0, 0,  0, 0);
	keyConfig(5, '+', '4', 0, 0,  0, 0);
	keyConfig(1, 'U', '5', 0, 0,  0, 0);
	keyConfig(4, '-', '6', 0, 0,  0, 0);
	keyConfig(0, 'L', '7', 0, 0,  0, 0);

	// initialize 6x8 font
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);		// initialize 6x8 font
	glXFontInit(&fi8x10, 8, 10, 32, 127, Font8x10);	// initialize 8x10 font
	glBlankScreen();

	// initialize control flags
	keypad_active = FALSE;
	prompt_displayed = FALSE;
	new_keypress_message = FALSE;

	for (;;) {
		costate {
			keyProcess();
			waitfor(DelayMs(10));
		}

		costate {
			// wait for any key to be pressed
			waitfor((wKey = keyGet()) != 0);
			release_value = -1;
			switch (wKey) {
				case 'L': release_value = '7'; break;
				case '-': release_value = '6'; break;
				case 'U': release_value = '5'; break;
				case '+': release_value = '4'; break;
				case 'D': release_value = '3'; break;
				case 'E': release_value = '2'; break;
				case 'R': release_value = '1'; break;
				default:
			}
			if (release_value != -1) {
				keypad_active = TRUE;
				// wait for the key to be released
				waitfor(keyGet() == release_value);
				keypad_active = FALSE;
			}
		}

		costate {
			if (!keypad_active) {
				if (!prompt_displayed) {
					glBlankScreen();
					glPrintf(0, 0, &fi6x8, "Waiting for a key to");
					glPrintf(0, 8, &fi6x8, "be pressed on the");
					glPrintf(0, 16, &fi6x8, "LCD/Keypad Module.");
					glFillPolygon(4, 115, 26, 121, 26, 121, 31, 115, 31);
					prompt_displayed = TRUE;
					new_keypress_message = FALSE;
				}

				// perform LCD/Keypad Module LEDs chase
				for (channel = 0; channel <= 6; ++channel) {
					for (led = 0; led <= 6; ++led) {
						if (led != channel) {
							dispLedOut(led, 0);
						} else {
							dispLedOut(channel, 1);
						}
					}
					waitfor(DelayMs(100));
					if (keypad_active) {
						break;
					}
				}
			}
		}

		costate {
			if (keypad_active && !new_keypress_message) {
				glBlankScreen();
				glFillPolygon(4, 113, 26, 121, 26, 121, 31, 113, 31);
				switch (wKey) {
				case 'L':
					glPrintf(0, 0, &fi8x10, "Scroll-Left key");
					glPrintf(0, 16, &fi8x10, "is active.");
					dispLedOut(0, 1);
					channel = 0;
					break;
				case '-':
					glPrintf(0, 0, &fi8x10, "Page-Down key");
					glPrintf(0, 16, &fi8x10, "is active.");
					dispLedOut(1, 1);
					channel = 1;
					break;
				case 'U':
					glPrintf(0, 0, &fi8x10, "Scroll-Up key");
					glPrintf(0, 16, &fi8x10, "is active.");
					dispLedOut(2, 1);
					channel = 2;
					break;
				case '+':
					glPrintf(0, 0, &fi8x10, "Page-Up key");
					glPrintf(0, 16, &fi8x10, "is active.");
					dispLedOut(3, 1);
					channel = 3;
					break;
				case 'D':
					glPrintf(0, 0, &fi8x10, "Scroll-Down key");
					glPrintf(0, 16, &fi8x10, "is active.");
					dispLedOut(4, 1);
					channel = 4;
					break;
				case 'E':
					glPrintf(0, 0, &fi8x10, "Enter key");
					glPrintf(0, 16, &fi8x10, "is active.");
					dispLedOut(5, 1);
					channel = 5;
					break;
				case 'R':
					glPrintf(0, 0, &fi8x10, "Scroll-Right");
					glPrintf(0, 16, &fi8x10, "key is active.");
					dispLedOut(6, 1);
					channel = 6;
					break;
				}
				// turn off all non-selected LCD/Keypad module LEDs
				for (led = 0; led <= 6; ++led) {
					if (led != channel) {
						dispLedOut(led, 0);
					}
				}
				prompt_displayed = FALSE;
				new_keypress_message = TRUE;
			}
		}

		costate {
			if (keypad_active) {
				// perform LCD/Keypad Module LCD cursor block ping-pong
				for (i = 0; i < LCD_XS - 8; i += 4) {
					glHScroll(0, 26, LCD_XS, 6, -4);
					waitfor(DelayMs(5));
					if (!keypad_active) {
						abort;
					}
				}
				for (i = 0; i < LCD_XS - 8; i += 4) {
					glHScroll(0, 26, LCD_XS, 6, 4);
					waitfor(DelayMs(5));
					if (!keypad_active) {
						abort;
					}
				}
			}
		}
	}
}


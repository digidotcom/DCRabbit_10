/***************************************************************************
	text.c

	Rabbit Semiconductor, 2001
	This sample program is for the LCD MSCG12232 Display Module.

   NOTE: Not currently supported on RCM4xxx modules.

  	This program demonstrates how the Text functions from the graphic
  	library can be used. Here's a list of what will be demonstrated:

  	1. Font initialization.
  	2. Text window initialization.
  	3. Text window, end of line wrap-around, end of text window clipping,
	   linefeed and carriage return.
	4. Creating 2 different TEXT windows for display.
	5. Displaying different FONT sizes.

	Instructions:
	1. Compile and run this program.
	2. View the display for each of the above features.
	3. Following the instructions when prompted.

**************************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

fontInfo fi6x8, fi12x16, fi5x6, fi8x10;
windowFrame textWindow1, textWindow2, textWindow3;

void main()
{
	auto int row1, row2, row3;
	auto int col1, col2, col3;
	auto int counter1, counter2, counter3;
	auto int wKey;

	//------------------------------------------------------------------------
	// Board and drivers initialization
	//------------------------------------------------------------------------
	brdInit();		// Required for all controllers
	dispInit();		// Graphic driver initialization, Start-up the keypad driver
	keypadDef();	// Use the default keypad ASCII return values


	//------------------------------------------------------------------------
	// Font initialization
	//------------------------------------------------------------------------
	// Initialize structures with FONT bitmap information
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);				//	initialize basic font
	glXFontInit(&fi8x10, 8, 10, 32, 127, Font8x10);			//	initialize basic font
	glXFontInit(&fi12x16, 12, 16, 32, 127, Font12x16);		//	initialize basic font


	//------------------------------------------------------------------------
	// Text window initialization
	//------------------------------------------------------------------------
	// Setup the widow frame1 to be the entire LCD to display information
	TextWindowFrame(&textWindow1, &fi6x8, 0, 0, 122, 32);

	// Setup the widow frame2 to be the upper half of the LCD to display information
	TextWindowFrame(&textWindow2, &fi6x8, 0, 0, 122, 16);

	// Setup the widow frame3 to be the lower half of the LCD to display information
	TextWindowFrame(&textWindow3, &fi6x8, 0, 16, 122, 16);


	//------------------------------------------------------------------------
	// Text window, end of line wrap-around, end of text window clipping,
	// and linefeed.
	//------------------------------------------------------------------------
	// Demonstrate text window, auto end of line wrap-around and text window clipping
	TextPrintf(&textWindow1, "Win1..Text end-of-line Wrapping and end-of-display Clipping\nENTER to Continue...>>this should be clipped");

	do
	{
		keyProcess();
		wKey = keyGet();
	} while(wKey != 'E');

	//------------------------------------------------------------------------
	// Text window carriage return demonstration
	//------------------------------------------------------------------------
	counter1 = 0;
	glBlankScreen();
	TextGotoXY(&textWindow1, 0, 3);
	TextPrintf(&textWindow1, "ENTER to Continue...");

	// Set the cursor back to the beginning of the display line
	TextGotoXY(&textWindow1, 0, 0);
	do
	{
		keyProcess();
		wKey = keyGet();

		// The string length must not exceed the maximum columns on a given
		// line, otherwise end-of-line auto-wrap will occur and "\r" will
		// take effect on the newline.
		TextPrintf(&textWindow1, "Win1 Counter = %03d\r",  counter1++);

		// When using the number of digit specifier (ie "%3d") you'll need to keep
		// the value within the range that you specify otherwise you'll get asterisks
		// indicating an out of range error.
		if(counter1 == 999)
		{
			counter1 = 0;
		}
	} while(wKey != 'E');

	//------------------------------------------------------------------------
	// Creating 2 different TEXT windows for display.
	//------------------------------------------------------------------------

	// Display Text within window2
	glBlankScreen();
	TextGotoXY(&textWindow2, 0, 0);
	TextPrintf(&textWindow2, "Win2 Counter = ");

	// Get current cursor location for window2
	TextCursorLocation(&textWindow2, &col2, &row2);


	// Display text within window3
	TextGotoXY(&textWindow3, 0, 0);
	TextPrintf(&textWindow3, "Win3 Counter = ");

	// Get current location of cursor for window3
	TextCursorLocation(&textWindow3, &col3, &row3);

	TextGotoXY(&textWindow3, 0, 1);
	TextPrintf(&textWindow3, "ENTER to Continue...");

	counter2 = 0;
	counter3 = 0;

	do
	{
		TextGotoXY(&textWindow2, col2, row2);
		TextPrintf(&textWindow2, "%02d", counter2++);

		// When using the number of digit specifier (ie "%2d") you'll need to keep the value
		// within the range that you specify otherwise you'll get asterisks indicating an
		// out of range error.
		if(counter2 == 99)
		{
			counter2 = 0;
		}

		TextGotoXY(&textWindow3, col3, row3);
		TextPrintf(&textWindow3, "%d", counter3++);
		keyProcess();
		wKey = keyGet();
	} while(wKey != 'E');

	//------------------------------------------------------------------------
	// Demonstrate displaying different FONT sizes
	//------------------------------------------------------------------------

	glBlankScreen();
	// Setup the widow frame1 to be the entire LCD to display information
	TextWindowFrame(&textWindow1, &fi6x8, 0, 24, 122, 8);

	// Setup the widow frame1 to be the entire LCD to display information
	TextWindowFrame(&textWindow2, &fi8x10, 0, 0, 122, 16);

	// Setup the widow frame1 to be the entire LCD to display information
	TextWindowFrame(&textWindow3, &fi12x16, 0, 2, 122, 16);

	TextGotoXY(&textWindow1, 0, 0);
	TextPrintf(&textWindow1, "ENTER to Continue...");

	TextPrintf(&textWindow2, "Font=8x10");
	do
	{
		keyProcess();
		wKey = keyGet();
	} while(wKey != 'E');

	// Clear only the area in window2
	glSetBrushType(PIXWHITE);
	glBlock(0, 0, 122, 16);
	glSetBrushType(PIXBLACK);

	TextPrintf(&textWindow3, "Font=12x16");
	do
	{
		keyProcess();
		wKey = keyGet();
	} while(wKey != 'E');

	//------------------------------------------------------------------------
	// Program completed message
	//------------------------------------------------------------------------

	glBlankScreen();
	// Setup the widow frame1 to be the entire LCD to display information
	TextWindowFrame(&textWindow3, &fi12x16, 0, 0, 122, 32);

	TextGotoXY(&textWindow3, 0, 0);
	TextPrintf(&textWindow3, "EndOfDemo,\nExiting!!!");
}
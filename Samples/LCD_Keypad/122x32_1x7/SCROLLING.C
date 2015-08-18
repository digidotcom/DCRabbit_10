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
	scrolling.c

	This sample program is for the LCD MSCG12232 Display Module.

   NOTE: Not currently supported on RCM4xxx modules.

  	This program demonstrates the scrolling features of the graphic
  	library.

  	Instructions:
  	-------------
  	1. Run and compile this program.
  	2. Watch the LCD display as it goes through the various scrolling
  	   demo's.

**************************************************************************/
#class auto		// Change default: local vars now stored on stack.
					// (Demo runs slower with "#class auto")
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

// Define font width for scrolling demo
#define FONTWIDTH 6

// Create structure for 6x8 font character set
fontInfo fi6x8;

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


nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

void main()
{
	auto int i,x,y,z;
	auto char s[256];

	//------------------------------------------------------------------------
	// Initialize controller
	//------------------------------------------------------------------------
	brdInit();			// Initialize Controller...Required for controllers!!!
	dispInit();			// Initialize the graphic driver
	glBackLight(1);	// Turn-on the backlight
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);		//	Initialize basic font

	while(1)
	{
		//------------------------------------------------------------------------
		// Text Scrolling up example
		//------------------------------------------------------------------------
		glPrintf(0, 0,&fi6x8,"Scroll-up Demo");
		msDelay(2500);
		glBlankScreen();

		for (y = 0, z = 32; y < 32; y += 8)
		{
			for (x = 0; x < 121-6; x+=6)
			{
				glPrintf(x, y,&fi6x8,"%c",z++);
				if (z > 64) z = 32;
			}
		}

		for(y=0; y<3; y++)
		{
			glVScroll(0, 0, LCD_XS, LCD_YS, -8);
			for (x = 0; x < 121-6; x+=6)
			{
				glPrintf(x, 24,&fi6x8,"%c",z++);
				if (z > 128) z = 32;
					msDelay(5);
			}
			msDelay(600);
		}

		//------------------------------------------------------------------------
		// Text Scrolling down example
		//------------------------------------------------------------------------
		glBlankScreen();
		glPrintf(0, 0,&fi6x8,"Scroll-Down Demo");
		msDelay(2500);

		glBlankScreen();
		for (y = 0, z=32; y < 32; y += 8)
		{
			for (x = 0; x < 121-6; x+=6)
			{
				glPrintf(x, y,&fi6x8,"%c",z++);
				if (z > 64) z = 32;
					msDelay(5);
			}
		}

		for(y=0; y<3; y++)
		{
			glVScroll(0, 0, LCD_XS, LCD_YS, 8);
			for (x = 0; x < 121-6; x+=6)
			{
				glPrintf(x, 0,&fi6x8,"%c",z++);
				if (z > 128) z = 32;
					msDelay(5);
			}
			msDelay(600);
		}

		//------------------------------------------------------------------------
		// Text Scrolling left example
		//------------------------------------------------------------------------
		glBlankScreen();
		glPrintf(0, 0,&fi6x8,"Scroll-Left Demo");
		msDelay(2500);
		glBlankScreen();
		for(y = 0; y < 1; y++)
		{
			sprintf(s, "Hello from Z-World!  ");
			i =0;
			while(s[i] != '\0')
			{
				glHScroll(0, 0, LCD_XS, 16, -8);
				glPrintf (116,  4,   &fi6x8, "%c", s[i++]);
				msDelay(250);
			}
		}

		//------------------------------------------------------------------------
		// Text Scrolling right and left example
		//------------------------------------------------------------------------
		glBlankScreen();

		glPrintf(0, 0,&fi6x8,"Scroll-Right & Left");
		glPrintf(0, 8,&fi6x8,"Demo...");
		msDelay(2500);

		glBlankScreen();

		sprintf(s, "Scroll Right/Left");
		glPrintf (0,  0,   &fi6x8, "%s", s);
		msDelay(1500);
		for(y = 0; y < 2; y++)
		{
			for(i=0; i<(LCD_XS -(strlen(s)*FONTWIDTH)); i++)
			{
				glHScroll(0, 0, LCD_XS, 8, 1);
			}
			msDelay(1500);
			for(i=0; i<(LCD_XS-(strlen(s)*FONTWIDTH)); i++)
			{
				glHScroll(0, 0, LCD_XS, 8, -1);
			}
			msDelay(1500);
		}
		glBlankScreen();

		//------------------------------------------------------------------------
		// Bitmap scrolling example
		//------------------------------------------------------------------------
		glPrintf(0, 0,&fi6x8,"Bitmap Scroll Right");
		glPrintf(0, 8,&fi6x8,"& Left Demo...");
		msDelay(2500);
		glBlankScreen();
		glXPutBitmap (0,0,53,29,Zwbw5_bmp);
		msDelay(2000);
		for(y = 0; y < 2; y++)
		{
			for(i=0; i<(LCD_XS-53); i++)
			{
				glHScroll(0, 0, LCD_XS, LCD_YS, 1);
			}
			msDelay(1500);
			for(i=0; i<(LCD_XS-53); i++)
			{
				glHScroll(0, 0, LCD_XS, LCD_YS, -1);
			}
			msDelay(1500);
		}
		glBlankScreen();
	}
}
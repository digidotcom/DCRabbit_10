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
	dkademo1.c

   NOTE: Not currently supported on RCM4xxx modules.

	This program demonstrates some of the features of the 122x32
	display and 1x7 keypad assembly with the graphic library. Features
	such as font and bitmap manipulation with horizontal and vertical
	scrolling.

	Font sets and bitmaps were created specially for this
	demonstration.

********************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

fontInfo fi1,fi2,fi6x8;

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

/////////////////////////////////////////////////////////////////////
void flashled(int led, int length)
{
	switch (led)
	{
		case 0: case 1: case 2: case 3:
			led *= 2;
			break;
		case 4:
			led = 1;
			break;
		case 5:
			led = 3;
			break;
		case 6:
			led = 5;
			break;
	}
	dispLedOut(led,1);
	msDelay(length);
	dispLedOut(led,0);
}
/////////////////////////////////////////////////////////////////////
// displayClock - displays clock faces of each hour by using a
// font range from wingdings character set.
/////////////////////////////////////////////////////////////////////
// Font   : Wingdings (24pt,Bold)
// Monochrome  : Black Foreground, White Background
// Mode   : Landscape
// Width  : 27 Pixels
// Height : 32 Pixels
// Range  : 0xB7..0xC2
// Init   : glXFontInit ( &fi,27,32,0xB7,0xC2,clock );

xdata clock {
// Char = 0xB7
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0F','\x1E','\x00',
'\x30','\x0F','\x03','\x00',
'\x30','\x0E','\x03','\x00',
'\x30','\x0E','\x03','\x00',
'\x3C','\x0C','\x0F','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x1C','\x03','\x00',
'\x30','\x00','\x03','\x00',
'\x18','\x00','\x06','\x00',
'\x1E','\x00','\x1E','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xB8
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x30','\x0C','\x63','\x00',
'\x30','\x0D','\xC3','\x00',
'\x30','\x0F','\x03','\x00',
'\x3C','\x0E','\x0F','\x00',
'\x30','\x1C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x00','\x03','\x00',
'\x18','\x00','\x06','\x00',
'\x1E','\x00','\x1E','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xB9
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x3C','\x1F','\xEF','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x00','\x03','\x00',
'\x30','\x00','\x03','\x00',
'\x1E','\x00','\x1E','\x00',
'\x18','\x00','\x06','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xBA
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x1C','\x03','\x00',
'\x3C','\x1C','\x0F','\x00',
'\x30','\x0E','\x03','\x00',
'\x30','\x0F','\xC3','\x00',
'\x30','\x01','\xC3','\x00',
'\x18','\x00','\x06','\x00',
'\x1E','\x00','\x1E','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xBB
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x1C','\x03','\x00',
'\x3C','\x0C','\x0F','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0E','\x03','\x00',
'\x30','\x06','\x03','\x00',
'\x18','\x03','\x06','\x00',
'\x1E','\x00','\x1E','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xBC
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x3C','\x0C','\x0F','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x18','\x0C','\x06','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xBD
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\xE0','\x70','\x00',
'\x01','\x86','\x18','\x00',
'\x03','\x66','\x6C','\x00',
'\x06','\x06','\x06','\x00',
'\x0C','\x06','\x03','\x00',
'\x0F','\x06','\x0F','\x00',
'\x18','\x06','\x01','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x07','\x01','\x80',
'\x1E','\x06','\x07','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x0E','\x01','\x80',
'\x18','\x0C','\x01','\x80',
'\x0C','\x18','\x03','\x00',
'\x0F','\x00','\x0F','\x00',
'\x06','\x00','\x06','\x00',
'\x03','\x60','\x6C','\x00',
'\x01','\x86','\x18','\x00',
'\x00','\xE0','\x70','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xBE
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\xE0','\x70','\x00',
'\x01','\x86','\x18','\x00',
'\x03','\x66','\x6C','\x00',
'\x06','\x06','\x06','\x00',
'\x0C','\x06','\x03','\x00',
'\x0F','\x06','\x0F','\x00',
'\x18','\x06','\x01','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x07','\x01','\x80',
'\x1E','\x07','\x07','\x80',
'\x18','\x0E','\x01','\x80',
'\x18','\x7E','\x01','\x80',
'\x18','\x70','\x01','\x80',
'\x0C','\x00','\x03','\x00',
'\x0F','\x00','\x0F','\x00',
'\x06','\x00','\x06','\x00',
'\x03','\x60','\x6C','\x00',
'\x01','\x86','\x18','\x00',
'\x00','\xE0','\x70','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xBF
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\xE0','\x70','\x00',
'\x01','\x86','\x18','\x00',
'\x03','\x66','\x6C','\x00',
'\x06','\x06','\x06','\x00',
'\x0C','\x06','\x03','\x00',
'\x0F','\x06','\x0F','\x00',
'\x18','\x06','\x01','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x06','\x01','\x80',
'\x1E','\xFF','\x07','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x00','\x01','\x80',
'\x18','\x00','\x01','\x80',
'\x0F','\x00','\x0F','\x00',
'\x0C','\x00','\x03','\x00',
'\x06','\x00','\x06','\x00',
'\x03','\x60','\x6C','\x00',
'\x01','\x86','\x18','\x00',
'\x00','\xE0','\x70','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xC0
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\xE0','\x70','\x00',
'\x01','\x86','\x18','\x00',
'\x03','\x66','\x6C','\x00',
'\x06','\x06','\x06','\x00',
'\x0C','\x06','\x03','\x00',
'\x0F','\x06','\x0F','\x00',
'\x18','\xC6','\x01','\x80',
'\x18','\x76','\x01','\x80',
'\x18','\x1E','\x01','\x80',
'\x1E','\x0E','\x07','\x80',
'\x18','\x07','\x01','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x00','\x01','\x80',
'\x0C','\x00','\x03','\x00',
'\x0F','\x00','\x0F','\x00',
'\x06','\x00','\x06','\x00',
'\x03','\x60','\x6C','\x00',
'\x01','\x86','\x18','\x00',
'\x00','\xE0','\x70','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xC1
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\xE0','\x70','\x00',
'\x01','\x86','\x18','\x00',
'\x03','\x66','\x6C','\x00',
'\x06','\x06','\x06','\x00',
'\x0C','\x06','\x03','\x00',
'\x0F','\x1E','\x0F','\x00',
'\x18','\x1E','\x01','\x80',
'\x18','\x0E','\x01','\x80',
'\x18','\x0E','\x01','\x80',
'\x1E','\x06','\x07','\x80',
'\x18','\x06','\x01','\x80',
'\x18','\x07','\x01','\x80',
'\x18','\x00','\x01','\x80',
'\x0C','\x00','\x03','\x00',
'\x0F','\x00','\x0F','\x00',
'\x06','\x00','\x06','\x00',
'\x03','\x60','\x6C','\x00',
'\x01','\x86','\x18','\x00',
'\x00','\xE0','\x70','\x00',
'\x00','\x3F','\xC0','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
// Char = 0xC2
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x7F','\x80','\x00',
'\x01','\xC0','\xE0','\x00',
'\x03','\x0C','\x30','\x00',
'\x06','\xCC','\xD8','\x00',
'\x0C','\x0C','\x0C','\x00',
'\x18','\x0C','\x06','\x00',
'\x1E','\x0C','\x1E','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x3C','\x0C','\x0F','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x0C','\x03','\x00',
'\x30','\x00','\x03','\x00',
'\x18','\x00','\x06','\x00',
'\x1E','\x00','\x1E','\x00',
'\x0C','\x00','\x0C','\x00',
'\x06','\xC0','\xD8','\x00',
'\x03','\x0C','\x30','\x00',
'\x01','\xC0','\xE0','\x00',
'\x00','\x7F','\x80','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00',
'\x00','\x00','\x00','\x00'
};

/////////////////////////////////////////////////////////////////////
void displayClock(void)
{
	auto char i,j;

	for (i=0xb7,j=0; i<=0xc2; i++,j++)
	{
		glPrintf (47, 0, &fi1, "%c", i);
		if	(j==7)
			j=0;
		flashled(j, 200);
	}
	msDelay(500);
	glBlankScreen();
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////
void vertscroll(void)
{
	auto int numtimes;

	glXPutBitmap (0,0,53,29,Zwbw5_bmp);

	glPrintf(56, 24, &fi6x8, "Solutions");
	msDelay(800);
	glVScroll(56, 0, LCD_XS, LCD_YS, -8);
	glPrintf(56, 24, &fi6x8, "at your");
	msDelay(800);
	glVScroll(56, 0, LCD_XS,  LCD_YS, -8);
	glPrintf(56, 24, &fi6x8, "fingertips");
	msDelay(2000);

	for (numtimes=0; numtimes<4; numtimes++)
	{
		glVScroll(56, 0, LCD_XS, LCD_YS, -8);
		msDelay(800);
	}
	glBlankScreen();
}

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

/////////////////////////////////////////////////////////////////////
void horizscroll(void)
{
	auto int i;
	static const char str1[122] = "Rabbit Powered";
	static const char str2[122] = "  Displays";


	glXPutBitmap (83,0,39,15,rs2_bmp);
	glHScroll(0, 0, 122, 16, -5);
	for (i=0; i<strlen(str1); i++)
	{
		glHScroll(0, 0, 122, 16, -5);
		glPrintf (114, 4,  &fi6x8, "%c", str1[i]);
		msDelay(200);
	}

	glXPutBitmap (0,16,39,15,rs1_bmp);
	glHScroll(0, 16, 122, 16, 8);
	for (i=strlen(str2)-1; i>=0; i--)
	{
		glHScroll(0, 16, 122, 16, 6);
		glPrintf (0, 20, &fi6x8, "%c", str2[i]);
		msDelay(200);
	}

	msDelay(1000);
	glBlankScreen();
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Font   : Math B (8pt)
// Monochrome  : Black Foreground, White Background
// Mode   : Landscape
// Width  : 8 Pixels
// Height : 10 Pixels
// Range  : 0x00..0x6
// Init   : glXFontInit ( &fi,8,10,0x00,0x06,keys7 );

xdata keys7 {
// arrow left
'\x00',
'\x00',
'\x00',
'\x10',
'\x30',
'\xF0',
'\x30',
'\x10',
'\x00',
'\x00',
// arrow up
'\x00',
'\x00',
'\x00',
'\x00',
'\x20',
'\x20',
'\x70',
'\xF8',
'\x00',
'\x00',
// arrow down
'\x00',
'\x00',
'\x00',
'\x00',
'\xF8',
'\x70',
'\x20',
'\x20',
'\x00',
'\x00',
// arrow right
'\x00',
'\x00',
'\x00',
'\x80',
'\xC0',
'\xF0',
'\xC0',
'\x80',
'\x00',
'\x00',
// minus symbol
'\x00',
'\x00',
'\x00',
'\x00',
'\x00',
'\xE0',
'\x00',
'\x00',
'\x00',
'\x00',
// plus symbol
'\x00',
'\x00',
'\x00',
'\x20',
'\x20',
'\xF8',
'\x20',
'\x20',
'\x00',
'\x00',
// enter symbol
'\x00',
'\x00',
'\x00',
'\x24',
'\x64',
'\xFC',
'\x60',
'\x20',
'\x00',
'\x00'
};
/////////////////////////////////////////////////////////////////////

void movebox(void)
{
	auto int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
	auto int maxboxw, maxboxh, minboxw, minboxh, curboxw, curboxh;
	auto char wkey, escflag;
	auto int steph, stepv;
	static windowFrame window1, window2;

	steph = stepv = 5;
	toplx = botlx = 46;
	toply = topry = 10;
	toprx = botrx = toplx+20;
	botly = botry = toply+10;

	maxboxw = LCD_XS;
	minboxw = curboxw = toprx-toplx+1;
	maxboxh = LCD_YS;
	minboxh = curboxh = botly-toply+1;

 	glPlotPolygon(4, toplx, toply, toprx, topry, botrx, botry, botlx, botly );

	TextWindowFrame(&window1, &fi6x8, 0, 0, 122, 32);
	TextGotoXY(&window1,0,0);
	TextPrintf(&window1,"Move box: press keys");
	TextGotoXY(&window1,2,3);
	TextPrintf(&window1, "To exit: press");
	glPrintf (100, 22, &fi2, "%c", 0x06);

 	escflag = 1;
	while (escflag)
	{
		costate
		{								//	Process Keypad Press/Hold/Release
			keyProcess ();
			waitfor ( DelayMs(10) );
		}

		costate
		{								//	Process Keypad Press/Hold/Release
			waitfor ( wkey = keyGet() );	//	Wait for Keypress
			if (wkey != 0)
			{
				wkey = wkey-0x30;
				flashled(wkey, 50);
				glSetBrushType(PIXWHITE);
				glPlotPolygon(4, toplx, toply, toprx, topry, botrx, botry, botlx, botly );
				switch (wkey)
				{
					case 0:
						//move left
						toplx-=steph;
						toprx-=steph;
						botlx-=steph;
						botrx-=steph;
						break;
					case 1:
						//move up
						toply-=stepv;
						topry-=steph;
						botly-=steph;
						botry-=steph;
						break;
					case 2:
						//move down
						toply+=stepv;
						topry+=stepv;
						botly+=stepv;
						botry+=stepv;
						break;
					case 3:
						//move right
						toplx+=steph;
						toprx+=steph;
						botlx+=steph;
						botrx+=steph;
						break;
					case 4:
						//decrease by one bottom and rightside
						if (curboxw > minboxw)
						{
							curboxw--;
							toprx--;
							botrx--;
						}
						if (curboxh > minboxh)
						{
							curboxh--;
							botly--;
							botry--;
						}
						break;
					case 5:
						//increase by one bottom and rightside
						if (curboxw < maxboxw)
						{
							curboxw++;
							toprx++;
							botrx++;
						}
						if (curboxh < maxboxh)
						{
							curboxh++;
							botly++;
							botry++;
						}
						break;
					case 6:
						escflag=0;
						break;
				} //end switch

			// Re-draw user instructions
			glSetBrushType(PIXBLACK);
			TextGotoXY(&window1,0,0);
			TextPrintf(&window1,"Move box: press keys");
			TextGotoXY(&window1,2,3);
			TextPrintf(&window1, "To exit: press");
			glPrintf (100, 22, &fi2, "%c", 0x06);

			// Plot new polygon
			glSetBrushType(PIXXOR);
			glPlotPolygon(4, toplx, toply, toprx, topry, botrx, botry, botlx, botly );
			} //end if
		} // end costate
	} //end while loop
	glSetBrushType(PIXBLACK);
	msDelay(200);
	glBlankScreen();
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void initsystem(void)
{
	auto int status;

	brdInit();
	dispInit();
	glBlankScreen();
	glBackLight(1);
//	keypadDef();
	keyConfig (  0,'0',0, 25, 1,  1, 1 );
	keyConfig (  1,'1',0, 25, 1,  1, 1 );
	keyConfig (  2,'2',0, 25, 1,  1, 1 );
	keyConfig (  3,'3',0, 25, 1,  1, 1 );
	keyConfig (  4,'4',0, 25, 1,  1, 1 );
	keyConfig (  5,'5',0, 25, 1,  1, 1 );
	keyConfig (  6,'6',0, 0, 0,  0, 0 );

	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);
	glXFontInit ( &fi1,27,32,0xB7,0xC2,clock );
	glXFontInit ( &fi2,8,10,0x00,0x06,keys7 );

}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void main()
{
	initsystem();

	for (;;)
	{
		displayClock();
		vertscroll();
		horizscroll();
		movebox();
	}
}
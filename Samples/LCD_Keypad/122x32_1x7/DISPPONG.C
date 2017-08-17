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
/***********************************************************

      disppong.c
      (Adapted from pong.c)

      Demonstration of output to graphical display. Run this
      program on controllers supporting graphic displays

      Three 16x12 pixel bitmaps are used as the "pong" ball.

		Change the macros below that best fits your display.

************************************************************/
#class auto
#memmap xmem  // Required to reduce root memory usage

#define USE_DISPLAY_KEYPAD
#define PORTA_AUX_IO
#if RCM6700_SERIES
	#use "RCM67xx.LIB"
#elif RCM6600W_SERIES
	#use "RCM66xxW.LIB"
#else
	#fatal "Include platform library with necessary macros."
#endif

#define BOXTOPLEFTX 0   //box upper left x-coordinate
#define BOXTOPLEFTY 0	//box upper left y-coordinate
#define BOXWIDTH 122-1		//box width
#define BOXHEIGHT 32-1		//box height

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + (unsigned long)delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

//////////////////////////////////////////////////////////
// clear display and turn on backlight
//////////////////////////////////////////////////////////
void displayInit()
{
	dispInit();
	glBlankScreen();
	glBackLight(1);
}

//////////////////////////////////////////////////////////
// draws box outline
//////////////////////////////////////////////////////////
void drawBox(int x, int y, int w, int h)
{
 	glPlotLine(x, y, w, y);		//north line
 	glPlotLine(w, y, w, h);		//east line
 	glPlotLine(w, h, x, h);		//south line
 	glPlotLine(x, h, x, y);		//west line
}

//////////////////////////////////////////////////////////
// 16x12 pixel bitmaps with function call information
//////////////////////////////////////////////////////////

//	glXPutBitmap (topleftx,toplefty,16,12,smileyball);
xdata smileyball {
'\x07','\xC0',
'\x18','\x30',
'\x30','\x18',
'\x26','\xC8',
'\x46','\xC4',
'\x40','\x04',
'\x48','\x24',
'\x48','\x24',
'\x24','\x48',
'\x23','\x98',
'\x18','\x30',
'\x07','\xC0'
};

//	glXPutBitmap (topleftx,toplefty,16,12,wallball);
xdata wallball {
'\x07','\xC0',
'\x18','\x30',
'\x30','\x18',
'\x26','\xC8',
'\x46','\xC4',
'\x40','\x04',
'\x40','\x04',
'\x4F','\xE4',
'\x20','\x08',
'\x30','\x18',
'\x18','\x30',
'\x07','\xC0'
};

//	glXPutBitmap (topleftx,toplefty,16,12,blank);
xdata blank {
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
'\x00','\x00'
};

//////////////////////////////////////////////////////////
// forever bounce smiley ball
//////////////////////////////////////////////////////////
nodebug
void pong()
{
	auto int	px,py;      // Current Position
	auto int dx,dy;      // Current Direction
	auto int nx,ny;		// New Position

	// Position Ball
   px = BOXTOPLEFTX + 1;   py = BOXTOPLEFTY + 1;
   // Give Direction
   dx = 2;   dy = 2;

   while (1)
   {
		glXPutBitmap (px,py,16,12,smileyball);

		// Try New Position
      nx = px + dx;
      ny = py + dy;

		// Avoid Collision
      if (nx <= BOXTOPLEFTX || ((nx+16) >= BOXWIDTH))
      {
			glXPutBitmap (px,py,16,12,wallball);
			msDelay(50);
         dx = -dx;
		}
      if (ny <= BOXTOPLEFTY || ((ny+12) >= BOXHEIGHT))
      {
			glXPutBitmap (px,py,16,12,wallball);
			msDelay(50);
			dy = -dy;
		}

		// Next Position
      nx = px + dx;
      ny = py + dy;

		msDelay(50);
		glXPutBitmap (px,py,16,12,blank);

		// Move Ball
      px = nx; py = ny;
   }
}

//////////////////////////////////////////////////////////

void main ()
{
	brdInit();
	displayInit();

	drawBox(BOXTOPLEFTX, BOXTOPLEFTY,
			  BOXWIDTH-BOXTOPLEFTX, BOXHEIGHT-BOXTOPLEFTY);
	pong();
}


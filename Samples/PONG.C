/***********************************************************

	Samples\PONG.C
	Digi International, Copyright ©2009.  All rights reserved.

   Demonstration of output to STDIO. Uses ANSI escape sequences.
   Appearance will vary with different character sets.
   For best results the font for the STDIO window should be set
   to "Terminal".

************************************************************/

#class auto

int	xl, xh,	yl, yh;

/********* Position Cursor **********/

void gotoxy (int x, int y)
{
	printf( "\x1B[%d;%dH", y, x);
}

/********* Clear Screen ******/
void cls ()
{
	printf( "\x1B[2J");		// erase screen and move to home position
}

void box(int x,int y, int w, int h)
{
	int i;
	char hor_line[100];

	//define horizontal border
	memset( hor_line, 0xC4, w - 1);
	hor_line[w - 1] = 0;

	//print top, with upper-left (0xDA) and upper-right (0xBF) corners
	gotoxy(x, y);
 	printf ( "\xDA%s\xBF", hor_line);

	//print sides
	for( i = 1 ; i < h ; i++)
	{
		gotoxy(x, y + i);
	   putchar ( '\xB3' );
		gotoxy(x + w, y + i);
	   putchar ( '\xB3' );
	}

	//print bottom, with lower-left (0xC0) and lower-right (0xD9) corners
	gotoxy(x, y + h);
 	printf ( "\xC0%s\xD9", hor_line );
}

void pong()
{
	int   px,py;                        // Current Position
	int   dx,dy;                        // Current Direction
	int   nx,ny;                        // New Position

   px = xl; py = yl;        				// Position Ball
   dx = 1; dy = 1;                 		// Give Direction

   while (1)
   {
 		costate
      {
   	   px += dx; py += dy;           // Move Ball to new position

      	// draw ball and move cursor back to home position
	      gotoxy ( px, py );
   	   putchar ( '*' );
   	   gotoxy ( 1, 1);

			waitfor(DelayMs(75));

	      nx = px + dx;               	// Try New Position
   	   ny = py + dy;
      	if (nx <= xl || nx >= xh)     // Reverse direction on collision
         {
         	dx = -dx;
         }
	      if (ny <= yl || ny >= yh)
	      {
   	      dy = -dy;
   	   }

			// erase old ball
      	gotoxy ( px, py );
	      putchar ( ' ' );
      }
   }
}

///////////////////////////////////////////////////////////////////////

void main ()
{
   cls ();                         	// Clear Screen
	xl =  1;									// box coordinates
	xh = 26;
	yl =  4;
	yh = 12;

	/*
		Calculation to center title:
		xh-xl = width of box
		subtract 4 (width of title) = amount of whitespace on either side of title
		divide by 2 (after adding 1 to round up) = offset from left side of box
		add offset to xl = screen position of title
	*/
   gotoxy ( (xh-xl-4+1)/2+xl, yl-1 );  // Position Cursor
   printf ( "Pong" );              	// Title
	box(xl, yl, xh - xl, yh - yl);
	pong();
}
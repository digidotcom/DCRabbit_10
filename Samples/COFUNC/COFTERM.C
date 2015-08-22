/***************************************************************

   cofterm.c
   Zworld, 1999

   Example program demonstrating cofunctions, the cofunction
   serial library, and using a serial ANSI terminal to modify
   parameters on the Rabbit.

   This program is set up to run from the stdio window of
   Dynamic C or connected to a serial terminal on port B. This
   is controled by the TOSTDIO define below.

   If you want to use the serial port, comment out the TOSTDIO
   define below, and change the baud rate to your selected
   value.  The serial port will default to port B.  If you want
   to change the port see the comment in the #ifndef TOSTDIO
   section.

****************************************************************/
#class auto

#define TOSTDIO
#define BAUD		115200

/***************************************************************

   TOSTDIO - changes the default device and terminal type to
   match the stdio window for Dynamic C.

   NOTE:  The ESCAPE key does not work in the stdio window.
   The functionality of this key can only be used in a terminal
   program that supports it.

***************************************************************/

#ifndef TOSTDIO

//
// To change the serial port change the B to A, C, or D:
//
// i.e. to use port A:
//
//   BINBUFSIZE	-> AINBUFSIZE
//   BOUTBUFSIZE	-> AOUTBUFSIZE
//   cof_serBgetc -> cof_serAgetc
//   cof_serBputc -> cof_serAputc
//   cof_serBputs -> cof_serAputs
//   serBopen	   -> cof_serAputs
//

#define BINBUFSIZE 	31
#define BOUTBUFSIZE	31

#define scof_getch cof_serBgetc
#define scof_putch cof_serBputc
#define scof_puts  cof_serBputs
#define port_open	 serBopen

#else

scofunc scof_getch()
{
	while(!kbhit()) yield;
	return getchar();
}

scofunc scof_putch(char ch)
{
	putchar(ch);
}

scofunc scof_puts(char *str)
{
	int x;
	printf("%s",str);
}

//
// force open the DynamicC STDIO window
//

#define port_open(BAUD) printf("")

#endif


/***************************************************************
   basic single user cofunction input/output routines
***************************************************************/


/***************************************************************

	The TerminalSettings structure holds function pointers
	for calculating the escape sequences to clear the screen
	clear to the end of line, and moving the cursor position.

	Two terminals are supported by this example program:  the
	Dynamic C Stdio window and the DOS ANSI escape sequences.

	By following the example here, you should be able to extend
	this to most terminal types by adding the functions for
	ClrScr, ClrEol, and GotoXY that are used by your terminal.
	Then set the terminal_settings variable to point to this
	structure.

	The ESCSEQSIZE define needs to be large enough to
	temporarily	hold the	largest escape sequence for ClrScr,
	ClrEol or GotoXY because it may be use to hold the escape
	sequence for these functions.

***************************************************************/

#define ESCSEQSIZE 16

typedef char* (*TermFPTR)();

typedef struct
{
	TermFPTR clrscr,clreol,gotoxy;
} TerminalSettings;


//
// ANSI Terminal Emulation
//

char* ANSIClrScr(char* buffer) { return "\x1B[2J"; }
char* ANSIClrEol(char* buffer) { return "\x1B[K"; }
char* ANSIGotoXY(int x, int y, char* buffer)
{
	sprintf(buffer,"\x1B[%d;%df",y,x);
	return buffer;
}

const TerminalSettings ANSI = {
	ANSIClrScr,
	ANSIClrEol,
	ANSIGotoXY
};

//
// Dynamic C Stdio Terminal Emulation
//

char* DCWTClrScr(char* buffer) {	return "\x1Bt"; }
char* DCWTClrEol(char* buffer) { return "\x1BT"; }
char* DCWTGotoXY(int x, int y,char* buffer)
{
	sprintf(buffer,"\x1B=%c%c",x+' ',y+' ');
	return buffer;
}

const TerminalSettings DCWT = {
	DCWTClrScr,
	DCWTClrEol,
	DCWTGotoXY
};

//
// Select the appropriate terimal type
//

#ifdef TOSTDIO
const TerminalSettings* terminal_settings = &DCWT;
#else
const TerminalSettings* terminal_settings = &ANSI;
#endif

/***************************************************************

	Terminal Control Single User Cofunctions

	echoon, echooff, clrscr, clreol, gotoxy, putat, getat, and
	getatusing

	These functions provide the "user" interface to the lower
	level i/o functions.  With the exception of echoon and
	echooff they are all single user cofunctions.  They single
	user cofunctions could be normal cofunctions if your design
	keeps the same cofunction from being called concurrently
	from two	locations.

***************************************************************/

/***************************************************************

   __echoon==1 - echo character
   __echoon==0 - echo '*'

   echoon() is a firsttime function which turns the echo on.
   echooff() is a firsttime function which turns the echo off.

   These were made firsttime functions so they could be called
   inside the waitfordone/wfd statement.

***************************************************************/

int __echoon;
firsttime void echoon()  { __echoon=1; }
firsttime void echooff() { __echoon=0; }

/***************************************************************
	clrscr - clear the terminal's screen using the
	terminal_settings.
***************************************************************/

scofunc clrscr()
{
	char buffer[ESCSEQSIZE];
	wfd scof_puts(terminal_settings->clrscr(buffer));
}

/***************************************************************
	clreol - clear the terminal's screen using the
	terminal_settings.
***************************************************************/

scofunc clreol()
{
	char buffer[ESCSEQSIZE];
	wfd scof_puts(terminal_settings->clreol(buffer));
}

/***************************************************************
	gotoxy(int xpos, int ypos) - locate the cursor at screen
	coordinates (xpos,ypos).
***************************************************************/

scofunc gotoxy(int x, int y)
{
	char buffer[ESCSEQSIZE];
	wfd scof_puts(terminal_settings->gotoxy(x,y,buffer));
}

/***************************************************************
	putat(int x, int y, char* str) - print the str at location
	(x,y).
***************************************************************/


scofunc putat(int x, int y, char* str)
{
	char buffer[ESCSEQSIZE];
	wfd scof_puts(terminal_settings->gotoxy(x,y,buffer));
	wfd scof_puts(str);
}

/***************************************************************

	getat(int x, int y, char* input, length) - get a string with
	the cursor starting at (x,y) into the variable input for a
	maximum number of characters length.  This function will
	print the string starting at the location (x,y) and allow
	editing at that point.

	As the user types characters they are echoed to the screen
	if the __echoon variable is set.  Otherwise, a "*" is echoed
	in the characters place.

	If there are any character in the string BS will back over
	the last character (removing it from the string).

	RET, TAB, or ESC terminate input and return to the caller.
	The ESC causes the input to terminate with an aborted code
	which stops the enclosing waitfordone statement.

	x=wfd {
		getat(...);
		getat(...);
		getat(...);
	}
	if(x<0) abort;

	The above waitfordone statement of treating the gets as one
	form where the escape will terminate the form.  By detecting
	the abort resonable action could be taken to abort or to back
	up a sequence of input screens.

***************************************************************/

scofunc getat(int xpos, int ypos, char* input, int length)
{
	int x,offset;
	char buffer[ESCSEQSIZE];

	#GLOBAL_INIT { __echoon=1; }	// global echo flag starts on

	offset=strlen(input);

	wfd gotoxy(xpos,ypos);			// locate cursor at (xpos,ypos)

	if(__echoon)
		wfd scof_puts(input);		// print the input string
	else {
		for(x=0;x<offset;x++)
			wfd scof_putch('*');		// if echo is off print '*' instead of input
	}

	for(x=0;x<length;x++)
		wfd scof_putch(' ');			// blank out rest of string

	wfd gotoxy(xpos+offset,ypos);	// locate cursor at end of input

	for(;;) {
		wfd x=scof_getch();			// get a character

		switch(x) {
			case '\b':
				if(offset>0) {			// if backspace remove previous character
					wfd scof_puts("\b \b");
					offset--;
				} else
					wfd scof_putch('\x07');
				break;

			case '\t':					// user done if they TAB or RET
			case '\r':
				input[offset]=0;
				return 0;

			case '\x1B':				// user canceling form with ESC
				abort;

			default:
				if(offset<length && isprint(x)) {
					if(__echoon)		// print the character or '*'
						wfd scof_putch((char)x);
					else
						wfd scof_putch('*');

					input[offset++]=x; // place character in input
				}
				break;
		}
	}
}

/***************************************************************

	getatusing(int xpos, int ypos, char* format, char* input)
	This function acts in a similiar way to the getat above,
	except the length parameter is replaced by a format string.

	This string consists of the letters:
		A = upper/lower case alphabetical characters
		N = upper/lower case alphabetical characters and numbers
		# = numbers
		- = -
		/ = /
		: = :

		The last three are delimiters that are automatically
		added and removed as the user enters characters into
		the fields.

	The length of the format string limits the length of the
	input string.

***************************************************************/

scofunc getatusing(int xpos, int ypos, char* format, char* input)
{
	int x,offset,length;
	char buffer[ESCSEQSIZE];

	offset=strlen(input);
	length=strlen(format);

	wfd gotoxy(xpos,ypos);			// locate cursor at (xpos,ypos)

	if(__echoon)
		wfd scof_puts(input);		// print the input string
	else {
		for(x=0;x<offset;x++)
			wfd scof_putch('*');		// if echo is off print '*' instead of input
	}

	for(x=offset;x<length;x++)		// blank out rest of string
		wfd scof_putch(' ');

	wfd gotoxy(xpos+offset,ypos);	// locate cursor at end of input

	for(;;) {
		switch(format[offset]) {	// print any delimiters
			case '-':
			case '/':
			case ':':
				wfd scof_putch(format[offset++]);
				continue;

			default:
				break;
		}

		wfd x=scof_getch();			// get a character

		switch(x) {
			case '\b':
				if(offset>0) {			// remove character
					wfd scof_puts("\b \b");
					offset--;

					switch(format[offset]) {
						case '-':		// remove delimiter
						case '/':
						case ':':
							wfd scof_puts("\b \b");
							offset--;
							break;

						default:
							break;
					}

				} else
					wfd scof_putch('\x07');
				break;

			case '\t':					// user done if they TAB or RET
			case '\r':
				input[offset]=0;
				return 0;

			case '\x1B':				// user canceling form with ESC
				abort;

			default:
				if(offset<length) {
					switch(format[offset]) { // validate input
						case 'A':
							if(!isalpha(x)) goto err;
							break;

						case '#':
							if(!isdigit(x)) goto err;
							break;

						case 'N':
							if(!isalnum(x)) goto err;
							break;

						default:
err:						wfd scof_putch('\x07');
							continue;
					}

					if(__echoon)	// print the character or '*'
						wfd scof_putch((char)x);
					else
						wfd scof_putch('*');

					input[offset++]=x; // place character in input
				}
				break;
		}
	}
}


/***************************************************************

	login()
	This function is an example of using putat and getat to
	build a form and demonstrates how echoon and echooff can
	be used.  The first waitfordone statement initialises the
	labels on the screen and the second gets input from the
	user.

***************************************************************/

scofunc login()
{
	int x;
	char name[16],pass[16],buffer[80];

	name[0]=pass[0]=0;

	wfd {
		clrscr();
		putat(5,5,"name:");
		putat(5,6,"password:");
		echoon();
	}

	x=wfd {
		getat(15,5,name,15);
		echooff();
		getat(15,6,pass,15);
		echoon();
	}

	if(x>0) {
		sprintf(buffer,"name %s, password %s accepted.",name,pass);
		wfd putat(5,10,buffer);
		waitfor(DelaySec(3));
	} else
		abort;
}

/***************************************************************

	chgparm()
	This function is an example of using putat and getatusing to
	build a form and demonstrates how you can limit a input to
	all letters and numbers in the case of the first parameter
	and numbers followed by a letter using a '-' as a delimiter
	in the second case.

***************************************************************/

scofunc chgparm()
{
	int x;
	char parm1[6],parm2[6];

	parm1[0]=parm2[0]=0;

	wfd {
		clrscr();
		putat(5,5,"parameter #1:");
		putat(5,6,"parameter #2:");
		echoon();
	}

	x=wfd {
		getatusing(20,5,"NNNNN",parm1);
		getatusing(20,6,"####-A",parm2);
	}

	if(x>0) {
		wfd putat(5,10,"Parameters changed.");
		waitfor(DelaySec(3));
	} else
		abort;
}


/***************************************************************

	main()
	Open the serial port/stdio window.
	Continuously call the login and chgparm forms.

***************************************************************/

main()
{
	int x;

	port_open(BAUD);

	for(;;) {
		costate {
			x=wfd login();
			if(x<0) abort;
			x=wfd chgparm();
			if(x<0) abort;
		}
	}
}
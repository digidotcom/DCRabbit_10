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
/**************************************************************************

	slave.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
   This sample requires two units which each have an available
   RS485 serial port, one to use as slave, the other as master.

	This program demonstrates a simple RS485 transmission
	of lower case letters to the slave controller.  The slave
	will send converted upper case letters back to the master
	controller and these will be displayed in the STDIO window.

	Run this program on the slave controller first, then disconnect
   the programming cable and run the 'master.c' program on the
   other controller.  The RS485 ports must be connected together
   as shown below.

	Connections:
	============
		Master to  Slave
		485+ <---> 485+
		485- <---> 485-
		GND  <---> GND

	On BLxS2xx series boards:
	   485+ pin 9 on J11
	   485- pin 4 on J11
	   GND  pin 5 on J11

	Instructions:
	=============
	1. Connect the two boards as shown under connections.
   2. Compile and run this program on one board.
   2. Disconnect programming cable and move it to the other board.
   2. Compile and run the 'master.c' program on the other board.
 	2. View STDIO window for sample program results.
**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

#if _BOARD_TYPE_ == RCM4010    // BL4S210
  #define SER_NO_FLOWCONTROL
#endif

// serial buffer size
#define CINBUFSIZE  255
#define COUTBUFSIZE 255
#define ser485open 		serCopen
#define ser485wrFlush	serCwrFlush
#define ser485rdFlush 	serCrdFlush
#define ser485putc 		serCputc
#define ser485getc 		serCgetc

///////////////////////////////////////////////////////////////////////////
void main()
{
	auto int i, nIn1;

   // Initialize the controller
	brdInit();

	ser485open(57600);		// Set baud rate first
	ser485wrFlush();			// Clear Rx and Tx data buffers
	ser485rdFlush();

   // Initializes serial mode and disables RS-485 transmitter
	serMode(0);

	while (1)
	{
		while ((nIn1 = ser485getc()) == -1);	//	Wait for lowercase ascii byte
      for (i = ((freq_divider*3) >> 1); --i; ); // Delay before transmit
		ser485Tx();										//	Enable transmitter
		ser485putc ( toupper(nIn1) );				//	Echo uppercase byte
		while (ser485getc() == -1);				//	Wait for echo
		ser485Rx();										//	Disable transmitter
	}
}
///////////////////////////////////////////////////////////////////////////
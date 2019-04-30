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

	master.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
   This sample requires two units which each have an available
   RS485 serial port, one to use as slave, the other as master.

	This program demonstrates a simple RS485 transmission
	of lower case letters to the slave controller.  The slave
	will send converted upper case letters back to the master
	controller and these will be displayed in the STDIO window.

   Stopping and re-starting this program once the controllers are
   communicating may cause an error on the read back of the
   character which was interrupted.

	Use the 'slave.c' sample to program the slave controller first,
   then disconnect the programming cable and run this program on
   the master controller.  The RS485 ports must be connected
   together as shown below.

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
   2. Compile and run the 'slave.c' sample program on one board.
   2. Disconnect programming cable and move to the other board.
   2. Compile and run this program.
 	2. View STDIO window for sample program results.
**************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

#if _BOARD_TYPE_ == RCM4010   // BL4S210
  #define SER_NO_FLOWCONTROL
#endif

// serial setup
#define CINBUFSIZE  255
#define COUTBUFSIZE 255
#define ser485open 		serCopen
#define ser485wrFlush	serCwrFlush
#define ser485rdFlush 	serCrdFlush
#define ser485putc 		serCputc
#define ser485getc 		serCgetc

nodebug
void msDelay(unsigned int delay)
{
	unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

///////////////////////////////////////////////////////////////////////////

void main()
{
	int nIn1;
	char cOut;

   // Initialize the controller
	brdInit();

	ser485open(57600);	// Set baud rate first

	ser485wrFlush();		// Clear Rx and Tx data buffers
	ser485rdFlush();

   // Initializes serial mode and disables RS-485 transmitter
	serMode(0);

	// Sync up to the slave controller
	printf ("Starting sync up with the other controller.\r");
	cOut = 0x55;
	do
	{
		ser485Tx();							            // enable transmitter
		ser485putc ( cOut );				            // send lowercase byte
		while ((nIn1 = ser485getc()) == -1);      // wait for echo
		ser485Rx();							            // disable transmitter
		msDelay(5);
		if ((nIn1 = ser485getc ()) == -1)	      // check for reply
		{
			printf ("Waiting to sync up to the other controller.\r");
		}
	} while (nIn1 != cOut);

	while (1)
	{
		for (cOut='a';cOut<='z';++cOut)
		{
			ser485Tx();										//	enable transmitter
			ser485putc ( cOut );							//	send lowercase byte
			while (ser485getc() == -1);				//	wait for echo
			ser485Rx();										// disable transmitter

			while ((nIn1 = ser485getc ()) == -1);	//	wait for reply
			if (nIn1 == (toupper(cOut)))
         {
				printf ("\n\rUpper case %c is %c\n", cOut, nIn1 );
         }
         else
         {
				printf ("\n\rERROR: Sent %c and received %c\n", cOut, nIn1 );
         }
		}
	}
}
///////////////////////////////////////////////////////////////////////////
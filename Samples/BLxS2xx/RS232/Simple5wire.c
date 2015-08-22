/********************************************************************
	Simple5wire.c

	Digi International, Copyright © 2008.  All rights reserved.

	This program is used with BLxS2xx series controllers, except
   the BL4S210 (see below).

	Description:
	============
	This program demonstrates basic initialization for a
	simple RS232 5-wire loopback displayed in STDIO window.

   The BL4S210 board does not have full flow control and is not
   supported by this sample. This sample applies to all other
   BLxS2xx series controllers.

	Here's the typical connections for a 5 wire interface that
   would be made between controllers.

		 TX <---> RX
       RX <---> TX
		RTS <---> CTS
      CTS <---> RTS
		Gnd <---> Gnd


	Connections:
	============
	1. Connect TXE (pin 1) to RXE (pin 6) located on J11.
   2. Connect TXF (pin 2) to RXF (pin 7) located on J11.

   Instructions:
	=============
	1.  Compile and run this program.
	2.  TxF and RxF become the flow control RTS and CTS.
	3.  To test flow control, disconnect RTS from CTS while
		 running	this program.  Characters should stop printing
		 in STDIO window and continue when RTS and CTS are
		 connected again.

********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

#if _BOARD_TYPE_ == RCM4010
   #error "The BL4S210 board does not have full flow control and is not "
   #error "supported by this sample. This sample applies to all other"
   #fatal "BLxS2xx series controllers."
#endif

// serial buffer size
#define EINBUFSIZE  15
#define EOUTBUFSIZE 15

// serial baud rate
#ifndef _232BAUD
#define _232BAUD 115200
#endif

// BLxS2xx boards have no pull-up on serial Rx lines, and we assume in this
// sample the possibility of disconnected or non-driven Rx line.  This sample
// has no need of asynchronous line break recognition.  By defining the
// following macro we choose the default of disabled character assembly during
// line break condition.  This prevents possible spurious line break interrupts.
#define RS232_NOCHARASSYINBRK

main()
{
   int input, output;

   // Initialize controller
   brdInit();

   // Open serial port
	serEopen(_232BAUD);

   // Must use serial mode 1 for 5 wire RS232 operation and
   // serMode must be executed after the serXopen function(s).
   serMode(1);

   // Enable flow control
	serEflowcontrolOn();

   // Clear serial buffers
	serEwrFlush();
	serErdFlush();

   // infinite loop sending and receiving on serial port
   while (1)
	{
		for (output='a'; output<='z'; ++output)
		{
			printf("Sending char: %c\t", (char) output);
			serEputc (output);								//	Send Byte
			while ((input=serEgetc()) == -1);			// Wait to receive
			printf("Received char: %c\n", input);
		}
	}
}



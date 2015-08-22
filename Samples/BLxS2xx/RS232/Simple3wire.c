/********************************************************************
	Simple3wire.c

	Digi International, Copyright © 2008.  All rights reserved.

	This program is used with BLxS2xx series controllers, except
   the BL4S210 (see below).

	Description:
	============
	This program demonstrates basic initialization for a
	simple RS232 3-wire loopback displayed in STDIO window.

   The BL4S210 board does not have two 3-wire serial ports and
   is not supported by this sample. This sample applies to all
   other BLxS2xx series controllers.

   Here's the typical connections for a 3 wire interface that
   would be made between controllers.

       TX <---> RX
       RX <---> TX
		Gnd <---> Gnd

	Connections:
	============
	1. Connect TXE (pin 1) to RXF (pin 7) located on J11.
   2. Connect TXF (pin 2) to RXE (pin 6) located on J11.

	Instructions:
	=============
	1. Compile and run this program.
 	2. View STDIO window for sample program results.

********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

#if _BOARD_TYPE_ == RCM4010 // BL4S210
   #error "The BL4S210 board does not have two 3-wire serial ports and is "
   #error "not supported by this sample. This sample applies to all other"
   #fatal "BLxS2xx series controllers."
#endif

// serial buffer size
#define EINBUFSIZE  15
#define EOUTBUFSIZE 15
#define FINBUFSIZE  15
#define FOUTBUFSIZE 15

// serial baud rate
#ifndef _232BAUD
#define _232BAUD 115200
#endif

main()
{
   int input, output, output2;

   // initialize board (including serial ports)
   brdInit();

    // open serial ports
   serEopen(_232BAUD);
   serFopen(_232BAUD);

   // Must use serial mode 0 for 3 wire RS232 operation and
   // serMode must be executed after the serXopen function(s).
	serMode(0);

   // Clear serial data buffers
   serEwrFlush();
	serErdFlush();
   serFwrFlush();
	serFrdFlush();

   // infinite loop sending and receiving on serial port
   while (1)
	{
		for (output='a'; output<='z'; ++output)
		{
			// TXE -> RXF
			printf("TXE: %c\t", (char) output);
			serEputc (output);								//	Send Byte
			while ((input=serFgetc()) == -1);			// Wait to receive
			printf("RXF: %c\t", (char) input);
         // TXF->RXE
			output2 = toupper(input);
			printf("TXF: %c\t", (char) output2);
			serFputc (output2);								//	Send Byte
			while ((input=serEgetc()) == -1);			// Wait to receive
			printf("RXE: %c\n", (char) input);
		}
	}
}




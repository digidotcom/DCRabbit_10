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

	parity.c

	This program is used with BLxS2xx series controllers, except
   the BL4S210 (see below).

   Description:
	============
	This demonstration will repeatedly send byte values 0-127 from serial
	port F to serial port E. The program will switch between generating
   parity or not on port F. Port E will always be checking parity, so
   parity errors should occur during every other sequence.

   Note: For the sequence that does get parity errors, the errors won't
	occur for each byte received. This is because certain byte patterns
   along with the stop bit will appear to generate the proper parity for
   the UART.

   The BL4S210 board has only one serial port available and is not
   supported in this sample.  This sample applies to all other BLxS2xx
   series controllers.

   Connections:
	============
	1. Connect TXF (pin 2) to RXE (pin 6) located on J11.

   Instructions:
	=============
   1. Power-on the controller.
	2. Compile and run this program.
   3. Stop program.
	4. View STDIO window to see the test results of the sample program.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

#if _BOARD_TYPE_ == RCM4010 // BL4S210
   #error "The BL4S210 board does not have two 3-wire serial ports and is not "
   #error "supported by this sample. This sample applies to all other BLxS2xx "
   #fatal "series controllers."
#endif

// serial buffer size
#define FINBUFSIZE 255
#define FOUTBUFSIZE 255

#define EINBUFSIZE 255
#define EOUTBUFSIZE 255

// serial baud rate
#ifndef _232BAUD
#define _232BAUD 9600L
#endif

void main()
{
	auto int received, receive_count;
	auto int i;
	auto int txconfig;

   // Initialize the controller
	brdInit();

	serEopen(_232BAUD);
   serFopen(_232BAUD);

   // Serial mode must be done after opening the serial ports
   serMode(0);

   // Clear serial data buffers
   serErdFlush();
   serFrdFlush();
   serEwrFlush();
   serFwrFlush();

	//printf("Start with the parity option set properly\n");
	serEparity(PARAM_OPARITY);
   serFparity(PARAM_OPARITY);

	txconfig = PARAM_OPARITY;

	while (1)
	{
		costate
		{
      	receive_count = 0;
			// Send data value 0 - 127
			for (i = 0; i < 128; ++i)
			{
         	yield; // Yield so data can be read from serial port C
				serFputc(i);
			}
         // Wait for data buffer, internal data and shift registers to
         // become empty
   		waitfor(serFwrFree() == FOUTBUFSIZE);
   		waitfor(!((RdPortI(SFSR)&0x08) || (RdPortI(SFSR)&0x04)));

         // Wait for entire 128 bytes to be processed or a timeout
         waitfor(receive_count == 128 || DelayMs(500));

			// Toggle between parity options
			if (txconfig != PARAM_NOPARITY)
			{
				txconfig = PARAM_NOPARITY;
				printf("\n\nImproperly set parity option\n");
			}
			else
			{
				txconfig = PARAM_OPARITY;
				printf("\n\nProperly set parity option\n");
			}
			serFparity(txconfig);
		}
		costate
		{
      	// Receive characters one at a time.
 			if ((received = serEgetc()) != -1)
         {
         	++receive_count;
				printf("received 0x%x", received);
				if (serEgetError() & SER_PARITY_ERROR)
				{
					printf("\tPARITY ERROR\n");
				}
            else
            {
            	printf("\n");
            }
	   	}
      }
	}
}
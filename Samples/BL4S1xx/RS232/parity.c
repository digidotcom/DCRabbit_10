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

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
	This demonstration will repeatedly send byte values 0-127 from serial
	port D to serial port F. The program will cycle through parity types
   on serial port D. Serial port F will always be checking for odd parity,
   so parity errors should occur during most sequences.

   Note: For the sequence that does get parity errors, the errors won't
	occur for each byte received. This is because certain byte patterns
   along with the stop bit will appear to generate the proper parity for
   the UART.

   Connections:
	============
	1. Connect TX/1-W to CTS (CTS is RXF) located on J5.

   Instructions:
	=============
   1. Power-on the controller.
	2. Compile and run this program.
   3. Stop program.
	4. View STDIO window to see the test results of the sample program.

***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include BL4S1xx series library
#use "BL4S1xx.lib"

// serial buffer size
#define DINBUFSIZE  255
#define DOUTBUFSIZE 255

#define FINBUFSIZE  255
#define FOUTBUFSIZE 255

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

    // open serial ports
   serDopen(_232BAUD);
   serFopen(_232BAUD);

	// disable flow control, initialize 2 3-wire serial ports.
   serMode(0);

   // Clear serial data buffers
   serDwrFlush();
	serDrdFlush();
   serFwrFlush();
	serFrdFlush();

	// initialize parity
	serFparity(PARAM_OPARITY);
	serDparity(PARAM_OPARITY);
	txconfig = PARAM_OPARITY;

	printf("Starting...\n");

	while (1)
	{
		costate
		{
	      //send as fast as we can
	      for (i = 0; i < 128; i++)
	      {
	         waitfor(DelayMs(10));   //necessary if we are not using
	                                 //flow control
	         waitfordone{ cof_serDputc(i); }
	      }
	      // wait for data buffer, internal data and shift registers to become
	      // empty
	      waitfor(serDwrFree() == DOUTBUFSIZE);
	      waitfor(!((RdPortI(SDSR)&0x08) || (RdPortI(SDSR)&0x04)));
         waitfor(DelayMs(10));   //now wait for last Rx character to "arrive"
	      yield;

	      //toggle between sending parity bits, and not
	      if (txconfig == PARAM_SPARITY)
	      {
	         txconfig = PARAM_NOPARITY;
	         printf("\nParity option set to no parity.\n");
	         printf("Parity errors are expected on some received characters.\n");
	      }
	      else if(txconfig == PARAM_NOPARITY)
	      {
	         txconfig = PARAM_OPARITY;
	         printf("\nParity option set to odd parity.\n");
	         printf("No parity error should occur on any received character.\n");
	      }
	      else if(txconfig == PARAM_OPARITY)
	      {
	         txconfig = PARAM_EPARITY;
	         printf("\nParity option set to even parity.\n");
	         printf("Parity errors are expected on all received characters.\n");
	      }
	      else if(txconfig == PARAM_EPARITY)
	      {
	         txconfig = PARAM_MPARITY;
	         printf("\nParity option set to mark parity.\n");
	         printf("Parity errors are expected on some received characters.\n");
	      }
	      else if(txconfig == PARAM_MPARITY)
	      {
	         txconfig = PARAM_SPARITY;
	         printf("\nParity option set to space parity.\n");
	         printf("Parity errors are expected on some received characters.\n");
	      }
	      serDparity(txconfig);
		}

		costate
		{
			//receive characters one at a time and check for parity error
			waitfordone
			{
				received = cof_serFgetc();
			}
			printf("received 0x%x", received);
			if (serFgetError() & SER_PARITY_ERROR)
			{
				printf("\tPARITY ERROR\n");
			} else
         {
         	putchar('\n');
         }
	   }
	}
}
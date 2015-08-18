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
/*******************************************************************

	parity.c

	This program is used with RCM4500W series controllers
	and prototyping boards.

	Description
	===========
	This program demonstrates use of parity modes by repeatedly
	sending byte values 0-127 from TXC (serial port C) to RXD
   (serial port D).
	The program will cycle through parity types on serial port
	C. Serial port D will always be checking for odd parity, so
	parity errors should occur during most sequences.

	Prototyping Board Connections
	=============================

		On the RS232 connector
		-----------------------------

		 TXC <-------> RXD


	Instructions
	============
	1.  Run this program and observe error sequence in STDIO.

*******************************************************************/
#class auto

#define CINBUFSIZE 31
#define COUTBUFSIZE 31
#define DINBUFSIZE 31
#define DOUTBUFSIZE 31

// RCM45xxW boards have no pull-up on serial Rx lines, and we assume in this
// sample the possibility of disconnected or non-driven Rx line.  This sample
// has no need of asynchronous line break recognition.  By defining the
// following macro we choose the default of disabled character assembly during
// line break condition.  This prevents possible spurious line break interrupts.
#define RS232_NOCHARASSYINBRK

const long baud_rate = 9600L;

main()
{
	auto int received;
	auto int i;
	auto int txconfig;

	serCopen(baud_rate);
	serDopen(baud_rate);

	serCparity(PARAM_OPARITY);
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
	         waitfordone{ cof_serCputc(i); }
	      }
	      // wait for data buffer, internal data and shift registers to become
	      // empty
	      waitfor(serCwrFree() == COUTBUFSIZE);
	      waitfor(!((RdPortI(SCSR)&0x08) || (RdPortI(SCSR)&0x04)));
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
	      serCparity(txconfig);
		}

		costate
		{
			//receive characters in a leisurely fashion
			waitfordone
			{
				received = cof_serDgetc();
			}
			printf("received 0x%x\n", received);
			if (serDgetError() & SER_PARITY_ERROR)
			{
				printf("PARITY ERROR\n");
			}
	   }
	}
}


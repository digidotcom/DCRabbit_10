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
/**********************************************************

	computer3wire.c

	This sample program is for the BL4S210 SBC.

   Description:
	============
	This program demonstrates basic initialization for a
	simple RS232 3-wire connection.  Character typed in
   either the stdio window or in a serial terminal
   program are echoed in both displays.

	Connections:
	============
     	 Connect BL4S210 Serial port to computer.
         TxB - Pin 1 on J11 to PC RXD (Pin 2 if DB9, Pin 3 if DB25)
         RxB - Pin 6 on J11 to PC TXD (Pin 3 if DB9, Pin 2 if DB25)
         GND - Pin 5 on J11 to PC GND (Pin 5 if DB9, Pin 7 if DB25)

	Instructions:
	=============
   1.	 Start serial terminal program (Moni, Terra-Term, or
   	 HyperTerminal for example).
   2.  Configure terminal program to Baud defined below with
   	 8 data bits, 1 stop bit, and no parity.
   3.  Compile and run this program.
	4.  Type in either the stdio window or the terminal program.
   	 All characters typed should appear in both displays.
       There may be some differences for special characters like
       newlines (enter key), delete, backspace, and others.

**********************************************************/
#class auto	 // Change local var storage default to "auto"

// define for use of BL4S210 (single 3-wire RS-232 port)
#define SER_NO_FLOWCONTROL

// include BLxS2xx series library
#use "BLxS2xx.lib"

// serial buffer size
#define BINBUFSIZE  63
#define BOUTBUFSIZE 15

// serial baud rate
#ifndef _232BAUD
#define _232BAUD 57600
#endif

main()
{
	auto int i, input;
   auto char input_char, output_char;

   // initialize board (including serial ports)
   brdInit();

    // open serial port
   serBopen(_232BAUD);

	// disable flow control
	serMode(0);

   // Clear serial data buffers
   serBwrFlush();
	serBrdFlush();

   // print numbers to serial and stdio window
   for (i = 0; i < 10; ++i)
   {
     	while(!serBputc ('0' + i));
      putchar('0' + i);
   }

   printf("\nStart 3-wire serial computer display\n");
   serBputs("\nStart 3-wire serial computer display\n");

	while (1)
	{
   	costate
      {
       	waitfor((input=serBgetc()) != -1);	// wait for character from serial
			input_char = (char) input;				// convert int to char
         putchar(input_char);						// send character over stdio
         waitfor(serBputc(input_char));		// echo character back to serial
			if (input_char == '\r')
         {
         	putchar('\n');                   // add newline character
            waitfor(serBputc('\n'));
         }
      }

      costate
      {
       	waitfor(kbhit());							// wait for character from stdio
         output_char = getchar();				// get stdio character
         putchar(output_char);					// echo character back to stdio
         waitfor(serBputc(output_char));		// send character to serial
			if (output_char == '\r')
         {
         	putchar('\n');                   // add newline character
            waitfor(serBputc('\n'));
         }
      }
	}
}


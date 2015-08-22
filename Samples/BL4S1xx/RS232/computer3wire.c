/**********************************************************

	computer3wire.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
	This program demonstrates basic initialization for a
	simple RS232 3-wire connection.  Character typed in
   either the stdio window or in a serial terminal
   program are echoed in both displays.

	Instructions:
	=============
   1.	 Connect BL4S1xx Serial port to computer.
   2.	 Start serial terminal program (Moni, Terra-Term, and
   	 HyperTerminal for example).
   3.  Configure terminal program to Baud defined below with
   	 8 data bits, 1 stop bit, and no parity.
   4.  Compile and run this program.
	5.  Type in either the stdio window or the terminal program.
   	 All characters typed should appear in both displays.
       There may be some differences for special characters like
       newlines (enter key), delete, backspace, and others.

**********************************************************/
#class auto	 // Change local var storage default to "auto"

// include BL4S1xx series library
#use "BL4S1xx.lib"

// serial buffer size
#define DINBUFSIZE  63
#define DOUTBUFSIZE 15

// serial baud rate
#ifndef _232BAUD
#define _232BAUD 115200
#endif

main()
{
	auto int i, input;
   auto char input_char, output_char;

   // initialize board (including serial ports)
   brdInit();

    // open serial port
   serDopen(_232BAUD);

	// disable flow control
	serMode(0);

   // Clear serial data buffers
   serDwrFlush();
	serDrdFlush();

   // print numbers to serial and stdio window
   for (i = 0; i < 10; ++i)
   {
     	while(!serDputc ('0' + i));
      putchar('0' + i);
   }

   printf("\nStart 3-wire serial computer display\n");
   serDputs("\nStart 3-wire serial computer display\n");

	while (1)
	{
   	costate
      {
       	waitfor((input=serDgetc()) != -1);	// wait for character from serial
			input_char = (char) input;				// convert int to char
         putchar(input_char);						// send character over stdio
         waitfor(serDputc(input_char));		// echo character back to serial
			if (input_char == '\r')
         {
         	putchar('\n');                   // add newline character
            waitfor(serDputc('\n'));
         }
      }

      costate
      {
       	waitfor(kbhit());							// wait for character from stdio
         output_char = getchar();				// get stdio character
         putchar(output_char);					// echo character back to stdio
         waitfor(serDputc(output_char));		// send character to serial
			if (output_char == '\r')
         {
         	putchar('\n');                   // add newline character
            waitfor(serDputc('\n'));
         }
      }
	}
}


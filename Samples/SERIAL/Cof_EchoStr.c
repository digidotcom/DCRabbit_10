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
/*****************************************************

     Cof EchoStr.c

     This program echos a block of characters over serial port D.
     It must be run with a serial utility such as Hyperterminal.

     Connect RS232 cable from PC to Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXD
       Connect PC's RS232 RD  to Rabbit TXD

     Configure the serial utility for port connected to RS232, 19200 8N1.

     Run this program.
     Run Hyperterminal.
     Type characters into the serial utility window.

     This program uses the single user cofunction form of the serial read
     and write routines, which allows it to cooperatively multi-task with
     other costates when it is not actively reading or writing data. It
     will echo a Carriage Return terminated character strings sent from
     the serial utility back to the serial utility where they will appear
     in its send/receive window.

******************************************************/
#class auto


/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#define DINBUFSIZE  15
#define DOUTBUFSIZE 15
#define MAXS 5
#define TIMEOUT 3000UL // will time out 3 seconds after receiving
                       // a character unless cof_serDread completes

void main()
{
	int getOk, done;
	char s[MAXS + 1]; // plus 1 for null terminator

	done = 0;
   serDopen(19200);
   while (!done) {
		loophead();
   	costate {
   		wfd getOk = cof_serDgets(s, MAXS, TIMEOUT); // yields until getting a null terminated string
   		if (getOk) {
      		wfd cof_serDputs(s);                     // then yields until the string is written
      	}
      	else {
      		if (!strcmp(s, "q")) {
      			wfd cof_serDputs("Done");
      			done = 1;
      		}
      		else
      			wfd cof_serDputs("Timed out!");
      	}
      }
   }
   while (serDwrFree() != DOUTBUFSIZE) ;      // allow transmission to complete before closing
   serDclose();
}
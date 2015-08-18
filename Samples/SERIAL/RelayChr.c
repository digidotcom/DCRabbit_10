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

     RelayChr.c

     This program receives characters in port B and relays them
     out port C.
     It must be run with a serial utility such as Hyperterminal.

     Connect RS232 cable from PC to Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXB
       Connect PC's RS232 RD  to Rabbit TXC
	  With a second serial cable from the PC to the Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXC
       Connect PC's RS232 RD  to Rabbit TXC

     Configure the serial utility on both serial ports to RS232, 19200 8N1.

     Run this program.
     Run Hyperterminal.
     Type characters into the serial utility window.

     This program will echo characters sent from the serial utility back
     to the serial utility where they will appear in its send/receive window.

******************************************************/
#class auto


/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#define BINBUFSIZE  15
#define BOUTBUFSIZE 15
#define CINBUFSIZE  15
#define COUTBUFSIZE 15

void main()
{
   int chr;

	chr = 0;
   serBopen(19200);
   serCopen(19200);
   while (chr != 27) {  // Exit on Esc
      if ((chr = serBgetc()) != -1 && chr != 27) {
         	serCputc(chr);
      }
   }
   serCputs("Done");
   while (serCwrFree() != COUTBUFSIZE) ; // allow tx to complete before closing
   serCclose();
   serBclose();
}
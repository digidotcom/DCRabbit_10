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

     EchoBlk.c

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

     This program will echo a block of characters sent from the serial
     utility back to the serial utility where they will appear in its
     send/receive window. It reads bytes over serial port D into a data
     structure and writes back what it read as soon as the number of bytes
     defined by the length parameter is read or when it times out. The
     length parameter in this example is set to the size of the data
     structure into which the stream of serial data will be read. Each time
     a complete block is read the timeout clock stops. Once a new block
     begins, it will timeout if the byte to byte period elapses prior to
     completing the block.

******************************************************/

/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#class auto

#define DINBUFSIZE  7
#define DOUTBUFSIZE 7
#define Esc 27

// This timeout period determines when an active input data stream is
//   considered to have ended and is implemented within serDread.
#define MSSG_TMOUT 3000UL // will discontinue collecting data 3 seconds after
                          // receiving any character or when maxSize are read.

// This timeout period determines when to give up waiting for any data to
//   arrive and must be implemented within the user's program, as it is below.
#define IDLE_TMOUT 5000UL // will timeout after 5 seconds if no data is read.

/**
 * 	Using serXread(), the program can accept binary data, not just text.
 * 	This structure represents a 16-bit integer followed by twenty 32-bit
 * 	floating point values.  Of course, them floats better be IEEE-754
 * 	format values!
 */
typedef struct {
	short d;
	float f[20];
} DATA;    // serDread and serDwrite read and write bytes of any data type


void main()
{
	int n, maxSize;
	unsigned long t;
	DATA data;
	char s[32]; 		// For status messages.

	n = 0;
   maxSize = sizeof(DATA);   // 82 bytes
   serDopen(19200);
   t = MS_TIMER;
   while ((n != 1) || ((char)data.d != Esc)) {  // until 'Esc'
   	// 82 bytes max, up to 3 sec between chars before timing out
   	if ((n = serDread(&data, maxSize, MSSG_TMOUT)) > 0) {
   		if (n != maxSize) {
   			sprintf (s, " %d characters read ", n);
      		serDputs(s);
      	}
      	serDwrite(&data, n);
      	t = MS_TIMER;                    // start anew for next message
      }
      else
      {
      	if (MS_TIMER - t > IDLE_TMOUT)
      	{
      		serDputs(" Timed out! ");
      		t = MS_TIMER;                 // start anew for next message
      	}
      }
   }
   serDputs(" Done");
   while (serDwrFree() != DOUTBUFSIZE) ;  // Complete tx before closing
   serDclose();
}
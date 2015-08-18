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
/*---------------------------------------------------------------------------

   SERDMA.C


   This sample demonstrates serial DMA features of the RCM4500W series
   controllers.  The library API are now generalized to allow less
   hardcoding within a user program.  This allows for easier portability
   of applications.  To use the generalized API, instead of calling
   serCgetc(), for example, call serXgetc(SER_PORT_C).  There is a generic
   version of each function except the three most I/O intensive functions:
   open, close, and flowcontrolOn. See the code below for an example.

   The RS232 library can take advantage of DMA (direct memory access) by
   calling serXdmaOn.  This will use the DMA to transfer data from the
   circular buffer to the serial port and vice versa.  There are advantages
   and disadvantages to using DMA for serial transfers.

   The Advantages: 1. DMA will only fire an interrupt after a full transfer
   instead of after sending or receiving each byte.  This allows more time
   for the processor to do other things.  2. DMA is ideal for sending large
   amounts of data quickly.  If transfering files over serial, DMA is the
   faster and less processor intensive solution.

   The Disadvantages: 1. DMA receive doesn't support parity checking.  This
   is because there is no interrupt after each byte and therefore no way to
   check whether the parity error flag was set.  2. DMA flowcontrol only
   works on 6 designated pins.  These pins are the DMA external request
   pins PD2, PD3, PE2, PE3, PE6, and PE7.  Note that extra circuitry is
   required on the RCM4xxx prototyping board in order to connect the DMA
   flow control pins, and that this sample does not demonstrate the DMA
   flow control feature.

   Instructions: For the RCM4500W series protoboard, no modifications are
   necessary.  To use another serial port, however, simply change the
   USE_PORT macro and the two BUFSIZE macros.

   1. Plug in a serial cable from your PC to J4 so that the following pins
      are connected:

         (PC) TX  <----> RXD
         (PC) RX  <----> TXD
         (PC) GND <----> GND

   2. Compile and run this program.

   3. In Tera Term or a similar serial terminal program, connect to the
      serial port and select the correct baud rate (115200).  Type into
      the terminal or send a file.  Observe the output.

   4. Use the STDIO menu options to view or clear the buffer.  This
      program stores the most recent 8K of data in a buffer stored in FAR.

---------------------------------------------------------------------------*/

// USER DEFINED VARIABLES /////////////////////////////////////////////////////
#define USE_PORT		D
// NB: This sample is fairly slow about emptying the Rx buffer and does not use
//     DMA flow control, so we size the Rx and Tx buffers to be about 16 times
//     our maximum individual read / write length of 64 characters.  Without
//     DMA flow control and at a high baud rate, on sustained Rx even this
//     generous buffer size may be overflowed.
#define DINBUFSIZE	1023		// Serial receive buffer (stored in root)
#define DOUTBUFSIZE	1023		// Serial transmit buffer (stored in root)

#define MAX_SIZE		8192		// Buffer size (stored in xmem)
#define BAUD_RATE		115200L
#define LINE_LENGTH	16			// For printing the buffer
#define PAGE_LENGTH	1024		// For printing the buffer
// END USER DEFINED VARIABLES /////////////////////////////////////////////////

// CONCAT is a macro to combine other macros. If USE_PORT == C, then this will
// create serXopen to be ser + C + open, or serCopen.  The generalized RS232
// library defines all serX functions except the following two and the
// flowcontrolOn function, due to the large amount of port specific I/O and the
// allocation of root data.
#define serXopen					CONCAT(ser, CONCAT(USE_PORT, open))
#define serXclose					CONCAT(ser, CONCAT(USE_PORT, close))
// myport will be used as the parameter to the other serX functions.
#define myport						CONCAT(SER_PORT_, USE_PORT)

// RCM45xxW boards have no pull-up on serial Rx lines, and we assume in this
// sample the possibility of disconnected or non-driven Rx line.  This sample
// has no need of asynchronous line break recognition.  By defining the
// following macro we choose the default of disabled character assembly during
// line break condition.  This prevents possible spurious line break interrupts.
#define RS232_NOCHARASSYINBRK

int main()
{
	// This data will be stored in xmem.
	static far char data[MAX_SIZE];

	auto int len, offset;
	auto int i, j;
	auto char str[80];
	auto char input, *p;

	serXopen(BAUD_RATE);
	serXdmaOn(myport, DMA_CHANNEL_ANY, DMA_CHANNEL_ANY);

	printf("[P]rint Buffer, [C]lear Buffer, [Q]uit.\n\n");
	offset = 0;
	while(1) {
	   printf("> ");
	   while(!kbhit()) {
	      len = serXread(myport, str, 64, 10);
	      if(offset + len < MAX_SIZE) {
	         // Use the FAR version of the string function strncpy.
	         _f_strncpy(data + offset, str, len);
	         offset += len;
	      } else {
	         _f_strncpy(data + offset, str, MAX_SIZE - offset);
	         offset = len - (MAX_SIZE - offset);
	         _f_strncpy(data, str + len - offset, offset);
	      }
	      if(len > 0) {
	         serXwrite(myport, str, len);
	      }
	   }

	   // Get the input and print it out so the user can see what was typed.
	   input = getchar();
	   printf("%c\n", input);
	   switch(toupper(input)) {
	      case 'P':
	         printf("Printing buffer:\n");
	         for(i = 0; i < MAX_SIZE; i += LINE_LENGTH) {
	            if(i % PAGE_LENGTH == 0) {
	               printf("\tPage %X:\n", i / PAGE_LENGTH);
	            }
	            p = str;
	            for(j = 0; j < LINE_LENGTH && i + j < MAX_SIZE; j++) {
	               p += sprintf(p, "%02X ", data[i+j]);
	            }
	            p += sprintf(p, "   ");
	            for(j = 0; j < LINE_LENGTH && i + j < MAX_SIZE; j++) {
	               p += sprintf(p, "%c", isprint(data[i+j]) ? data[i+j] : ' ');
	            }
	            printf("\t%s\n", str);
	         }
	         printf("\n");
	         break;
	      case 'C':
	         printf("Clearing buffer...\n");
	         _f_memset(data, 0, MAX_SIZE);
	         printf("done.\n");
	         break;
	      case 'Q':
	         serXdmaOff(myport);
	         serXclose();
	         exit(0);
	      default:
	         printf("Unknown command\n");
	   }
	}
}


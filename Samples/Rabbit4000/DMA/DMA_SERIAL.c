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
/*****************************************************************************\
	dma_serial.c

	This program is used with Rabbit 4000 series microprocessors and protoboards.

	This sample demonstrates the use of the DMA, Timer A and Serial Port C to
	create a signal on Port PC2.

	The purpose of this sample is to create arbitrary frequencies with the DMA.
	We can preload the bit patterns we want to transmit in an array and set the
	DMA on its way with no furthur work required.  This would be ideal for many
	applications, such as motor control preset forward and backward arrays.

	Instructions:
		To see the output, hook up an oscilloscope to PC2 and run the program.

\*****************************************************************************/

// Uncomment the following line to debug DMA API functions.
//#define DMA_DEBUG

// A large size means less overhead since we are repeating the signal.
#define SIZE 3000

int main()
{
	int i, j, err;
	dma_chan_t handle;
	dma_addr_t src;
   char data[SIZE];

   src = xalloc(SIZE);

   // To create 1/3rd the frequency, we need 3 bits low, 3 bits high ect.
   // 00011100 01010001 11000111 - This pattern, in order it must be written,
   // is 0xC7, 0x51, 0x1C.
	for(i = 0; i < SIZE; i++)
   {
   	if(i % 3 == 0)
      	data[i] = 0xC7;
      if(i % 3 == 1)
      	data[i] = 0x51;
      if(i % 3 == 2)
      	data[i] = 0x1C;
   }

   root2xmem(src, data, SIZE);

   // setup parallel port C to output TXC to PC2
	WrPortI(PCFR, &PCFRShadow, PCFRShadow | 0x04);

	// setup timer for serial port C
	WrPortI(SCDLR, NULL, 0x01);
	WrPortI(SCDHR, NULL, 0x80);

   // setup serial port C
   WrPortI(SCCR, &SCCRShadow, 0x0C);
	WrPortI(SCER, &SCERShadow, 0x00);

   // Maximize the time spent on the DMA and its priorities.
   // Set the burst size to 1.
	err = DMAsetParameters(0, 0, DMA_IDP_FIXED, 1, 0);
   if(err){
   	printf("Error: DMAsetParameters.\n");
      exit(err);
   }

   handle = DMAalloc(DMA_CHANNEL_ANY, 1);
   if(handle == DMA_CHANNEL_NONE){
   	printf("Error: DMAalloc.\n");
      exit(EINVAL);
   }

   // begin DMA transfer over serial port C
   err = DMAmem2ioi(handle, SCAR, src, SIZE, DMA_F_REPEAT);
   if(err){
   	printf("Error: DMAmem2ioi.\n");
      exit(err);
   }

   printf("Press any key to end program.\n");
   while(!kbhit())
		;

   err = DMAstop(handle);
   if(err){
   	printf("Error: DMAstop.\n");
      exit(err);
   }

   err = DMAunalloc(handle);
   if(err){
   	printf("Error: DMAunalloc.\n");
      exit(err);
   }
}
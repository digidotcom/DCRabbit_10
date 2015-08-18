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

	dma_pwm.c

	This program is used with Rabbit 4000 series microprocessors and protoboards.

	This sample demonstrates a PWM output using DMA instead of interrupts.
	A wav file is loaded with the DMA directly to the PWM to vary the width of
	the pulses at a user defined speed.

   Hook up a speaker in series with a resistor to PC4 to hear the output. The
   stronger the resistor, the lower the volume.

\*****************************************************************************/

// Uncomment the following line to debug DMA API functions.
//#define DMA_DEBUG

#ximport "data\cat.dat"		xmem_src
unsigned long wav_size;
dma_addr_t src;

int main()
{
	int err, count;
   unsigned int dma_div, flags;
   dma_chan_t handle;

   // The dma divisor determines how fast the data is sent to PWM0R.  The
   // larger this number is, the slower the wav file will play.
	dma_div = freq_divider * 80;
	src = xmem_src + sizeof(long);
	xmem2root((void *)&wav_size, xmem_src, 4);

   WrPortI(PWL0R, NULL, 0x01);   // enable spread
   WrPortI(PWM0R, NULL, 0x80);   // 50% duty cycle
   WrPortI(TAT9R, &TAT9RShadow, 0);

   // setup pwm output to be PC4
	WrPortI(PCDDR, &PCDDRShadow, PCDDRShadow | 0x10);
	WrPortI(PCFR,  &PCFRShadow,  PCFRShadow  | 0x10);
	WrPortI(PCAHR, &PCAHRShadow, PCAHRShadow | 0x02);

   // Set the DMA to use 85% of the CPU resources.
   err = DMAsetParameters(0, 0, DMA_IDP_FIXED, 4, 15);
   if(err){
   	printf("Error: DMAsetParameters.\n");
      exit(err);
   }

   printf("Playing wav file...\n");
	for(count = 0; count < 5; count++){			// play the wav 5 times.
		printf(" Count = %d.\n", count + 1);

      handle = DMAalloc(DMA_CHANNEL_ANY, 0);
      if(handle == DMA_CHANNEL_NONE){
      	printf("Error: DMAalloc.\n");
         exit(EINVAL);
      }

      // If the wav_size were larger than 32765 bytes, more than one transfer
      // would be necessary.
      DMAtimerSetup(dma_div);
      flags = DMA_F_TIMER | DMA_F_TIMER_1BPR;
	   err = DMAmem2ioi(handle, PWM0R, src, (int)wav_size, flags);
	   if(err){
			printf("Error: DMAmem2ioi.\n");
         exit(err);
	   }

	   while(!DMAcompleted(handle, NULL))
	      ;

      // reset PWM to 50% duty cycle
	   WrPortI(PWM0R, NULL, 0x80);

 		err = DMAunalloc(handle);
      if(err){
      	printf("Error: DMAunalloc.\n");
         exit(err);
      }
   }
}
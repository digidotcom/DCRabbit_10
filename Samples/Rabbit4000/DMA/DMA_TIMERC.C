/*****************************************************************************\
	dma_timerc.c
 	Rabbit Semiconductor, 2006

	This program is used with Rabbit 4000 series microprocessors and protoboards.

	This sample uses timer c and DMA to test the functionality of all the DMA
	API functions.  A wav file is loaded with the DMA directly into Timer C to
	vary the width of the pulses at a user defined speed.

	Instructions:
		Hook up a speaker to PE0 and run this program.

\*****************************************************************************/

// Uncomment the following line to debug DMA API functions.
//#define DMA_DEBUG

#ximport "data\cat.dat"		xmem_src
unsigned long wav_size;
dma_addr_t src;

// Setup timer c and parallel port D alternate outputs.
void timerc_setup()
{
	unsigned int tc_div;
   // The Timer C divisor affects how far apart the pulses are.  This
   // effectively controls the volume by changing the average voltage
   // between a certain range.  Too high and the pulses get very far apart
   // and start to produce a high pitched noise.  Too low and the pulses
   // get too close together and the wav becomes inaudible.
	tc_div  = 0x0100;

   // setup PE0 for alternate output TMRC[0]
	WrPortI(PEDDR, &PEDDRShadow, PEDDRShadow | 0x01);
   WrPortI(PEALR, &PEALRShadow, (PEALRShadow & 0xFC) | 0x02);
   WrPortI(PEFR,  &PEFRShadow, PEFRShadow | 0x01);

   // setup timer c
   WrPortI(TCS0HR, NULL, 0x00);	// SET value MSB
   WrPortI(TCS0LR, NULL, 0x00);	// SET value LSB
   WrPortI(TCR0HR, NULL, 0x00);	// RESET value MSB (LSB set by DMA later)

	// Timer C Divider
   WrPortI(TCDLR, NULL, (char)tc_div);
   WrPortI(TCDHR, NULL, (char)(tc_div >> 8));

	// Timer C Control registers
   WrPortI(TCCR,  &TCCRShadow,  0x00);	  // interrupts disabled for timer c
   WrPortI(TCCSR, &TCCSRShadow, 0x01);   // enable main clock for timer c
}

int main()
{
	int count, err;
   unsigned int dma_div, flags;
   dma_chan_t handle;

	// The dma divisor determines how fast the data sent to TCR0LR changes.
	// The larger this number is, the slower the wav file will play.
	dma_div = 0x0F00;

	src = xmem_src + sizeof(long);
	xmem2root((void *)&wav_size, xmem_src, 4);
	timerc_setup();

   // Set the DMA to use 50% of the CPU resources.
	err = DMAsetParameters(0, 0, DMA_IDP_FIXED, 2, 50);
	if(err){
   	printf("Error: DMAsetParameters.\n");
		exit(err);
   }

   handle = DMAalloc(DMA_CHANNEL_ANY, 0);
   if(handle == DMA_CHANNEL_NONE){
      printf("Error: DMAalloc.\n");
      exit(EINVAL);
   }

   printf("Playing wav file...\n");
	for(count = 0; count < 5 && !kbhit(); count++){			// play the wav 5 times.
		printf(" Count = %d.\n", count + 1);

		// This function sets up the timer.  To use the timer, the flag values
      // must be set correctly as well.
      DMAtimerSetup(dma_div);

      // if the wav_size were larger than 32765 bytes, more than one transfer
      // would be necessary.
      flags = DMA_F_TIMER | DMA_F_TIMER_1BPR;
	   err = DMAmem2ioi(handle, TCR0LR, src, (int)wav_size, flags);
	   if(err){
			printf("Error: DMAmem2ioi.\n");
         exit(err);
	   }

	   while(!DMAcompleted(handle, NULL))
	      ;

   }

   WrPortI(TCCSR, NULL, 0x00);   // disable the main clock for timer c

   err = DMAunalloc(handle);
   if(err){
      printf("Error: DMAunalloc.\n");
      exit(err);
   }
}
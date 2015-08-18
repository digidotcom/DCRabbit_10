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
	dma_notes.c

	This program is used with Rabbit 4000 series microprocessors and protoboards.

	This sample uses timer c and DMA to test the functionality of the DMA API.
	Specifically, this sample uses many low-level API and the DMA interrupt.
	The DMA writes to the timer C divider via the TCBAR register.

	Instructions:
		Hook up a speaker to PE0 and choose a song with the SONG define below.
      To increase the volume, change the volume define below.

\*****************************************************************************/

// Uncomment the following line to debug DMA API functions.
//#define DMA_DEBUG

// Choose a song to output.
// Available songs are 'tuning_note' 'axelf' 'bumblebee' and 'encounters'
#define SONG bumblebee
// VOLUME must be greater than or equal to 1 (higher is louder).
#define VOLUME	50

#define SONG_SIZE sizeof(SONG)

// Note:
// ------------------------------------------------------------------------- //
//   The values below won't necessarily correspond to the frequencies depending
// on the speed of your board. However, the relative frequencies are correct
// and the songs should sound correct.  To see the frequency of 'A', change the
// SONG to tuning_note and use an oscilloscope.
// ------------------------------------------------------------------------- //

// Values based on a 29MHz clock (divided by 2 for pclk/2).
// TC CLOCK = 29.4912MHz/2
// y = (29.4912 * 1000000/2) / x Hz

#define _A		0x82E8 // 440.00 Hz
#define _Bb		0x7B90 // 466.16 Hz
#define _B		0x749E // 493.92 Hz
#define _C		0x6E13 // 523.28 Hz
#define _Db		0x67E5 // 554.40 Hz
#define _D		0x6210 // 587.36 Hz
#define _Eb		0x5C91 // 622.24 Hz
#define _E		0x575E // 659.28 Hz
#define _F		0x5276 // 698.48 Hz
#define _Gb		0x4DD6 // 740.00 Hz
#define _G		0x4978 // 784.00 Hz
#define _Ab 	0x4558 // 830.64 Hz
#define REST	0xFFFF // Not really a rest, but a low A to create an illusion

// Varify that A = 440Hz with an oscilloscope
const unsigned int tuning_note[] = {
	_A
};

// Theme from Close Encounters of the Third Kind.  Multiply by 2 or 4 for
// each octave to lower the note.  For higher notes, divide by 2 or 4.
// repeat notes to lengthen them, or change dma_div to slow entire piece.
const unsigned int encounters[] = {
	_G,   _G,   _G,   _G,
   _A/2, _A/2, _A/2, _A/2,
   _F,   _F,   _F,   _F,
   _F*2, _F*2, _F*2, _F*2,
   _C,   _C,   _C,   _C,   _C,   _C,   _C,   _C };

// Flight of the Bumble Bee.
const unsigned int bumblebee[] = {
	_A,    _Ab*2, _G*2,  _Gb*2, _F*2,  _Bb,   _A,    _Ab*2,
	_A,    _Ab*2, _G*2,  _Gb*2, _F*2,  _Gb*2, _G*2,  _Ab*2,
   _A,    _Ab*2, _G*2,  _Gb*2, _F*2,  _Bb,   _A,    _Ab*2,
   _A,    _Ab*2, _G*2,  _Gb*2, _F*2,  _Gb*2, _G*2,  _Ab*2,
   _A,    _Ab*2, _G*2,  _Gb*2, _G*2,  _Gb*2, _F*2,  _E*2,
   _F*2,  _Gb*2, _G*2,  _Ab*2, _A,    _Bb,   _A,    _Ab*2,
   _A,    _Ab*2, _G*2,  _Gb*2, _G*2,  _Gb*2, _F*2,  _E*2,
   _F*2,  _Gb*2, _G*2,  _Ab*2, _A,    _Bb,   _C,    _Db,
   _D,    _Db,   _C,    _B,    _Bb,   _Eb,   _D,    _Db,
   _D,    _Db,   _C,    _B,    _Bb,   _B,    _C,    _Db,
   _D,    _Db,   _C,    _B,    _Bb,   _Eb,   _D,    _Db,
   _D,    _Db,   _C,    _B,    _Bb,   _B,    _C,    _Db,
	_D,	 _Db,   _C,    _B,    _C,    _B,    _Bb,   _A,
   _Bb,   _B,    _C,    _Db,   _D,    _Eb,   _D,    _Db,
	_D,	 _Db,   _C,    _B,    _C,    _B,    _Bb,   _A,
   _Bb,   _B,    _C,    _Db,   _D,    _Eb,   _D,    _Db,
   _D,    REST,  REST,  REST,  REST,  REST,  REST,  REST,
   REST,  _Bb*2, _B*2,  _C*2,  _Db*2, _D*2,  _Eb*2, _E*2,
   _F*2,  _Gb*2, _G*2,  _Ab*2, _A,    _Bb,   _C,    _Db,
   _D,    REST,  REST,  REST,  _A,    REST,  REST,  REST,
   _D*2,  REST,  REST,  REST,  REST,  REST,  REST,  REST };

// Theme from Beverly Hills Cop
const unsigned int axelf[] = {
	_A,   _A,   REST, REST, _C,   REST, REST, _A,
   REST, _A,   _D,   _D,   _A,   _A,   _G*2, _G*2,
   _A,   _A,   REST, REST, _E,   REST, REST, _A,
   REST, _A,   _F,   _F,   _E,   _E,   _C,   _C,
   _A,   _A,   _E,   _E,   _A/2, REST, _A,   _G*2,
   REST, _G*2, _E*2, _E*2, _B,  _B,    _A,   _A,
   REST, REST, REST, REST, REST, REST, REST, REST,
   REST, REST, REST, REST, _E*2, REST, _G*2, REST };

// Global variables
unsigned int isr_count;
unsigned long wav_size;
dma_addr_t src;
int channel;

// The DMA ISR reloads the length registers and the TCBPR register.  TCBPR
// points to the TC divider registers at 0x0502 and 0x0503.  These are loaded
// with the values in the array to create different sounds.
nodebug interrupt void dma_isr()
{
   #asm
   ld		hl, (channel)
	xor	a
	rl		hl
	rl		hl
	rl		hl
	rl		hl
   ex		de, hl
   ld		hl, D0L0R
   add	hl, de
   ld    a, 0x02
   ioi   ld (hl), a
   ioi	ld (TCBPR), a
	ld		hl, D0L1R
   add	hl, de
   xor	a
   ioi	ld (hl), a
   #endasm

	if(isr_count >= (wav_size >> 1)) {
	   WrPortI(TCCSR, &TCCSRShadow, 0x00);   // disable main clock for timer c
		return;
   }

   isr_count++;
   DMAstartDirect(channel);
}

// This function makes each note 16 times longer.
void song_setup()
{
	static unsigned int arr[SONG_SIZE*16];
	int i, j;

	for(i = 0; i < wav_size/32; i++){
	   for(j = 0; j < 16; j++){
	      arr[i*16+j] = SONG[i];
	   }
	}
  	root2xmem(src, arr, (int)wav_size);
}

// This function sets up the timer C registers.
void timerc_setup()
{
	// SET value defaults to 0, RESET value = VOLUME
   WrPortI(TCR0HR, NULL, (char)(VOLUME >> 8));
   WrPortI(TCR0LR, NULL, (char)VOLUME);

	// TCBPR value of 0x02 points the Block Access Register to TCDLR (0x502)
  	WrPortI(TCBPR,  NULL, 0x02);

	// interrupts disabled for timer c
   WrPortI(TCCR,  &TCCRShadow, 0x00);
}

int main()
{
	int err;
	char control;
   unsigned int dma_div, flags;
   dma_chan_t handle;

	// dma_div is the tempo .. higher = slower
   dma_div = 0xFFFF;

	wav_size = SONG_SIZE*16;
	src = xalloc(wav_size);
   song_setup();

   // setup PE0 for alternate output TMRC[0]
	WrPortI(PEDDR, &PEDDRShadow, PEDDRShadow | 0x01);
   WrPortI(PEALR, &PEALRShadow, (PEALRShadow & 0xFC) | 0x02);
   WrPortI(PEFR, &PEFRShadow, PEFRShadow | 0x01);
	timerc_setup();

   handle = DMAalloc(DMA_CHANNEL_ANY, 8);
   if(handle == DMA_CHANNEL_NONE) {
      printf("Error: DMAalloc.\n");
      exit(EINVAL);
   }

	// ISR Setup
   channel = DMAhandle2chan(handle);
	SetVectExtern4000(0x08+channel, dma_isr);

	// Parameter Setup
	err = DMAsetParameters(3, 3, DMA_IDP_FIXED, 1, 80);
   if(err) {
   	printf("Error: DMAsetParameters.\n");
      exit(err);
   }

	// Timer Setup:  This function sets the DMA divider registers, but to use
	// the timer the DMA_F_TIMER flag must be sent to the transfer function.
   DMAtimerSetup(dma_div);
   flags = DMA_F_TIMER | DMA_F_TIMER_1BPR | DMA_F_INTERRUPT;

   printf("press any key to quit.\n");

	while(!kbhit()) {
	   isr_count = 0;
      DMAmem2ioi(handle, TCBAR, src, 2, flags);
	   WrPortI(TCCSR,  NULL, 0x01);   // enable main clock for timer c
	   while(isr_count < (wav_size >> 1) && !kbhit())
			;
	}

   WrPortI(TCCSR, NULL, 0x00);   // disable main clock for timer c

   err = DMAunalloc(handle);
   if(err) {
      printf("Error: DMAunalloc.\n");
      exit(err);
   }
}
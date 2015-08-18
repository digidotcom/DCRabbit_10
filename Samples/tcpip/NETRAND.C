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
/*******************************************************************************
        Samples\TCPIP\netrand.c

        Initialize the network interface(s).  Every time a packet arrives for
        this host, the inter-packet timings will be used to add randomness
        to the entropy table (see RAND.LIB documentation).

        Each bit of randomness is printed out as soon as it is available.

        Send packets to this host using, e.g., "ping" from another host.
        From Unix, become root user then issue "ping -f <IP address>" to
        really stress test the program (flood ping).
*******************************************************************************/
#class auto


/***********************************
 * Configuration                   *
 ***********************************/


/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG		1			// 1 = static single ethernet

#define NET_ADD_ENTROPY			// Define this to get DCRTCP to record inter-packet timings.

#define BITS_2_COLLECT	500	// Number of bits to accumulate before printing stats.

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use dcrtcp.lib

#define MAXSEQ	4
word singles[2];
word pairs[4];
word triples[8];
word quads[16];

word oneseqs[32];
word zeroseqs[32];

word * const seqs[MAXSEQ] = { singles, pairs, triples, quads };


void main()
{
	unsigned long seq, mask;
	word accum;
	word i, j;
	int k;

	accum = 0;
	memset(singles, 0, sizeof(singles));
	memset(pairs, 0, sizeof(pairs));
	memset(triples, 0, sizeof(triples));
	memset(quads, 0, sizeof(quads));
	memset(oneseqs, 0, sizeof(oneseqs));
	memset(zeroseqs, 0, sizeof(zeroseqs));
	seed_init(NULL);

	// Set timer B running at maximum rate to increase clock resolution.
	// This gives more random bits per observation.
	WrPortI(TBCR, &TBCRShadow, 0x00);	// clock timer B with (perclk/2) and
													//     set interrupt level to 0 (off)
	WrPortI(TBCSR, &TBCSRShadow, 0x01);	// enable timer B

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
	ip_print_ifs();

   for (;;) {
      tcp_tick(NULL);
      while (seed_bits()) {
      	// The following call to tcp_tick() is required since, if the network
      	// interface is very busy and interrupt driven, then there might be
      	// so much new entropy being generated that this loop is never exited.
      	// (This is an artificial case since a normal application would never
      	// spend so much time printing stuff out).
	      tcp_tick(NULL);
      	accum++;
      	seq = seq << 1 | seed_getbits(1);
      	printf("%lu", seq & 1);

      	for (i = 0; i < MAXSEQ && i < accum; i++)
      		(*(seqs[i] + (word)(seq & (2<<i)-1)))++;
      	for (i = 0; i < 32 && i < accum; i++) {
      		mask = (2uL << i) - 1;
      		if ((seq & mask) == mask)
      			oneseqs[i]++;
      		else if (!(seq & mask))
      			zeroseqs[i]++;
      	}

      	if (accum % BITS_2_COLLECT == 0) {
      		printf("\nAccumulated %u bits...\n", accum);
      		for (i = 0; i < MAXSEQ; i++) {
      			for (j = 0; j < (2<<i); j++) {
      				for (k = i; k >= 0; k--)
      					printf("%u", (j>>k) & 1);
      				printf(": %u\n", seqs[i][j]);
      			}
      		}
      		for (i = 0; i < 32; i++)
      			printf("%2u x 0's: %5u  %2u x 1's: %5u\n", i+1, zeroseqs[i], i+1, oneseqs[i]);
      	}
      }
   }
}
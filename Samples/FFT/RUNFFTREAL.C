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
/***********************************************************

	RUNFFTREAL.C

	Sample test program for fftreal()

	First construct a floating-point array comprising a dc
	value and up to three sinusoids with configurable
	amplitude, frequency, and phase. Round the floating values
	to integers, scaling if necessary to fit in a 16-bit word,
	store them in the integer array x[], and print x[].
	Then transform x[] via fftreal() and print the results.
	Finally inverse transform x[] via fftrealinv() and print
	it a third time.

************************************************************/
#class auto

#use fft.lib

int k, fsize, scale, blockexp, x[2048];
unsigned long int timer;

float fxmax, fscale, fx[2048];
float DC,
      A1, F1, P1,
      A2, F2, P2,
      A3, F3, P3;

#define     max(a,b)    ( a > b ? a : b )
#define     twoPI       (2*PI)

void dumpx( char *title );
void printsigparams( void );

void main ( void ) {

    blockexp = 0;

//  First construct a real-valued floating-point array comprising a dc value and
//  up to three sinusoids with configurable amplitude, frequency, and phase.
//  Round the floating values to integers, scaling if necessary to fit in a
//  16-bit word, store them in the integer array x[], and print x[].  Then
//  transform x[] via fftreal() and print the results.  Finally inverse
//  transform x[] via fftrealinv() and print it a third time.

	fsize = 16;

    DC =  2000.0;
    A1 =  4000.0;       F1 = 1.0/16;        P1 =  0.0;
    A2 =  8000.0;       F2 = 1.0/8;         P2 = PI/4;
    A3 = 16000.0;       F3 = 1.0/2;         P3 = PI/2;
    // Amplitude        // Frequency        // Phase

    // first calculate floating point values and note maximum value

    fxmax = 0;
    for (k = 0; k < 2*fsize; k++) {
        fx[k] =  DC
               + A1 * sin(twoPI * F1 * k + P1)
               + A2 * sin(twoPI * F2 * k + P2)
               + A3 * sin(twoPI * F3 * k + P3);
        fxmax = max( fxmax, fabs( fx[k] ) );
    }
    printsigparams();

    // scale floating point values to be less than 32767 in
    // absolute value and load integer x[k] array.

    fscale = max( 1.0, fxmax/32767.0 );
    for (k = 0; k < 2*fsize; k++)
        x[k] = (int) ( fx[k] / fscale + 0.5);  // 0.5 rounds
    if (fscale > 1.0 ) printf("input divided by %7.5f\n"  \
                              "to fit in 16-bit integer\n"\
                              "------------------------\n", fscale);
    dumpx( "before fftreal()" );

	// compute the FFT

    timer = MS_TIMER;
    fftreal(x, fsize, &blockexp);
    timer = MS_TIMER-timer;
    printf("%4d-point fftreal() took %ld ms\n", fsize, timer);
    dumpx( "after fftreal()" );

	// compute the inverse FFT

    fftrealinv(x, fsize, &blockexp);
	dumpx("after fftrealinv()");

} // end of main

//-------------------- Utility Functions ------------------------

void dumpx( char *title ) {
    auto int k;
    printf("x[n] %s\n", title);
    for (k = 0; k < fsize; k++)
        printf("%4d  %6d %6d\n", k, x[2*k], x[2*k + 1]);
    printf("blockexp is %d\n\n", blockexp);
}

void printsigparams( void ) {
    printf("---------- Signal Parameters ----------\n");
    printf("%7.f (dc)\n", DC);
    printf("%7.f sin(%6.4f * twoPI * k + %4.2f)\n", A1, F1, P1);
    printf("%7.f sin(%6.4f * twoPI * k + %4.2f)\n", A2, F2, P2);
    printf("%7.f sin(%6.4f * twoPI * k + %4.2f)\n", A3, F3, P3);
    printf("---------------------------------------\n");
}

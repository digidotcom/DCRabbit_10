/***********************************************************

	RUNPOWERSPECTRUM.C 
   Z-World, 2000

	 Sample test program for powerspectrum()

  First construct a real-valued floating-point array comprising
  a dc value and up to three sinusoids with configurable
  amplitude, frequency, and phase. Round the floating values
  to integers, scaling if necessary to fit in a 16-bit word,
  store them in the integer array x[], and print x[].  Then
  transform x[] via fftreal() and print the results.  Finally
  run powerspectrum() and print the results both as long
  integers and in db referenced to the largest component.

************************************************************/
#class auto

#use fft.lib

int x[1024+2];   // the extra 2 elements are used in the call to powerspectrum()
int k, fsize, scale, blockexp; 

unsigned long int timer, fmaxpower;

float fxmax, fscale, maxdb, ftmp, fx[1024];
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

    DC =  2000.0;
    A1 =  4000.0;       F1 = 1.0/16;        P1 =  0.0;
    A2 =  8000.0;       F2 = 1.0/8;         P2 = PI/4;
    A3 = 16000.0;       F3 = 1.0/2;         P3 = PI/2;
    // Amplitude        // Frequency        // Phase

	fsize = 16;

    // first calculate floating point values and note maximum value

    fxmax = 0;
    for (k = 0; k < 2*fsize; k++) {
        fx[  k  ] =  DC
                   + A1 * sin(twoPI * F1 * k + P1)
                   + A2 * sin(twoPI * F2 * k + P2)
                   + A3 * sin(twoPI * F3 * k + P3);
        fxmax = max( fxmax, fabs( fx[k] ) );
    }
    printsigparams();

    // Scale floating point values to be less than 32767 in
    // absolute value and load integer x[k] array.

    fscale = max( 1.0, fxmax/32767.0 );
    for (k = 0; k < 2*fsize; k++)
        x[k] = (int) ( fx[k] / fscale + 0.5);  // 0.5 rounds
    if (fscale > 1.0 ) printf("input divided by %7.5f\n"\
                              "to fit in 16-bit integer\n"\
                              "------------------------\n", fscale);
    dumpx( "before fftreal()" );

	// Compute the FFT and its execution time.

    timer = MS_TIMER;
    fftreal(x, fsize, &blockexp);
    timer = MS_TIMER-timer;
    printf("%3d-point fftreal() took %ld ms\n\n", fsize, timer);

    // realfft() returns the fmax term in the imaginary position of the dc
    // term, i.e. in x[1].  The values of the dc term in x[0] and fmax term in
    // x[1] equal the full amplitude of their respective components.  The
    // values of all the terms other than dc term and the fmax term equal one
    // half the amplitude of their respective components.  (The other half is
    // contained in the missing, negative-frequency components.)  Halve the
    // fmax term so that it can be treated the same as the other non-dc terms.

    x[1] /= 2;
    dumpx( "after fftreal() and fmax/2" );

	// Remove the fmax power value from x[1] and set x[1] to zero in
    // preparation for calling powerspectrum().  Although we could square the
    // fmax term and report it separately, we can instead put the fmax term in
    // the convenient position just beyond the last FFT entry in x[] and let
    // powerspectrum() process it just like any other entry. 

	x[  2*fsize  ] = x[1];
	x[2*fsize + 1] = x[1] = 0;

    powerspectrum(x, fsize + 1, &blockexp); // + 1 PIcks up fmax term

    // After powerspectrum() the values of all the terms other than the dc term
    // now equal 50% of their corresponding sinusoidal power, but the dc term
    // has 100% of the dc power.  Correct the imbalance by first halving the dc
    // term to 50% and then increasing blockexp by 1 to put everything at 100%.

	*(long *)&x[0] /= 2;    // dc power term
	blockexp++;

    printf("x[] after powerspectrum()\n");
    for (k = 0; k < fsize + 1; k++)
        printf("%4d  %12ld\n", k, *(long *)&x[2*k]);
    printf("blockexp is %d\n\n", blockexp);

    // Compute and print db values, using the largest value as the 0 db
    // reference. Because the decibel values are relative to the largest
    // component, we can ignore blockexp.

    maxdb = -99.0;
    for (k = 0; k < fsize + 1; k++) {
        ftmp  = (float) *(long *)&x[2*k];
        fx[k] = ( ftmp > 0.0 ) ? 10*log10( ftmp ) : -99;
        if ( fx[k] > maxdb ) maxdb = fx[k];
    }
    printf("fx[] after powerspectrum(), db(max)\n");
    for (k = 0; k < fsize + 1; k++) {
        if ( fx[k] > -99 )
            printf("%4d  %7.3f\n", k, fx[k] - maxdb);
        else
            printf("%4d     -\n", k);
    }

} // end of main

//-------------------- Utility Functions ------------------------

void dumpx( char *title ) {
    int k;
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

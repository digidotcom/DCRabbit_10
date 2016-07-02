/*
 *		makecold.cpp
 *		Copyright (C) Z-World (jjb)
 *		This program builds a "tripletized" coldloader file from
 *		a *.bin file.
 *
 *
 *		To make this work you have to change a couple of the compiler
 *		options.  You should turn pointer checking off, Restrict watch
 *		expressions, and exclude the bios.
 *
 *		Your program should use the following format:
 *
 *		#rcodorg rootcode2 0x0 0x0 0x6000 apply
 *
 *		#asm
 *		main::
 *		premain::
 *				ld		a,0x84
 *		ioi	ld		(SPCR),a
 *				ld		a,0xff
 *
 *				ld		a,0xa5
 *		ioi	ld		(PADR),a
 *
 *		loop:
 *				jr		loop
 *		#endasm
 *
 *		Notice both the main and premain.  This is necessary to cause
 *		the compiler to not bring in the functions called from premain.
 *
 *		Select the desired combination of RAM control lines and spectrum
 *		spreader options, below.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

// RAM control lines:  0 is /OE0/WE0/CS0, 1 is /OE1/WE1/CS1, 2 is /OE1/WE1/CS2,
//                     3 is /OE1/WE1/CS0, 4 is /OE0/WE0/CS1, 5 is /OE0/WE0/CS2

// 16-bit coldloader built with
// RAM_CONTROL_OPTION   =	6
// BOOTSTRAP_FROM_16BIT =	1
// COLDLOAD_COMPILE =		1
// Normal 8-bit coldloader built with
// RAM_CONTROL_OPTION   =	1
// BOOTSTRAP_FROM_16BIT =	0
// COLDLOAD_COMPILE =		1
// For Precoldload build with
// RAM_CONTROL_OPTION   =	1
// BOOTSTRAP_FROM_16BIT =	0
// COLDLOAD_COMPILE =		0

#define _RAM_CONTROL_OPTION 1
#define BOOTSTRAP_FROM_16BIT 0

// spectrum spreader:  0 to disable, 1 for normal, 2 for strong
#define ENABLE_SPREADER_OPTION 0

#define DISABLE_WATCHDOG 1
#define COLDLOAD_COMPILE 0

#if 0 == _RAM_CONTROL_OPTION
	#define RC0_SEQUENCE "\x80\x14\x40"	// MB0CR: 2 ws, /OE0, /WE0, /CS0 active
	#define RC1_SEQUENCE "\x80\x15\x40"	// MB1CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC2_SEQUENCE "\x80\x16\x40" // MB2CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC3_SEQUENCE "\x80\x17\x40" // MB3CR: 2 ws, /OE0, /WE0, /CS0 active
#elif 1 == _RAM_CONTROL_OPTION
	#define RC0_SEQUENCE "\x80\x14\x45"	// MB0CR: 2 ws, /OE1, /WE1, /CS1 active
	#define RC1_SEQUENCE "\x80\x15\x45"	// MB1CR: 2 ws, /OE1, /WE1, /CS1 active
   #define RC2_SEQUENCE "\x80\x16\x40" // MB2CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC3_SEQUENCE "\x80\x17\x40" // MB3CR: 2 ws, /OE0, /WE0, /CS0 active
#elif 2 == _RAM_CONTROL_OPTION
	#define RC0_SEQUENCE "\x80\x14\x46"	// MB0CR: 2 ws, /OE1, /WE1, /CS2 active
	#define RC1_SEQUENCE "\x80\x15\x46"	// MB1CR: 2 ws, /OE1, /WE1, /CS2 active
   #define RC2_SEQUENCE "\x80\x16\x40" // MB2CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC3_SEQUENCE "\x80\x17\x40" // MB3CR: 2 ws, /OE0, /WE0, /CS0 active
#elif 3 == _RAM_CONTROL_OPTION
	#define RC0_SEQUENCE "\x80\x14\x44"	// MB0CR: 2 ws, /OE1, /WE1, /CS0 active
	#define RC1_SEQUENCE "\x80\x15\x44"	// MB1CR: 2 ws, /OE1, /WE1, /CS0 active
   #define RC2_SEQUENCE "\x80\x16\x40" // MB2CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC3_SEQUENCE "\x80\x17\x40" // MB3CR: 2 ws, /OE0, /WE0, /CS0 active
#elif 4 == _RAM_CONTROL_OPTION
	#define RC0_SEQUENCE "\x80\x14\x41"	// MB0CR: 2 ws, /OE0, /WE0, /CS1 active
	#define RC1_SEQUENCE "\x80\x15\x41"	// MB1CR: 2 ws, /OE0, /WE0, /CS1 active
   #define RC2_SEQUENCE "\x80\x16\x40" // MB2CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC3_SEQUENCE "\x80\x17\x40" // MB3CR: 2 ws, /OE0, /WE0, /CS0 active
#elif 5 == _RAM_CONTROL_OPTION
	#define RC0_SEQUENCE "\x80\x14\x42"	// MB0CR: 2 ws, /OE0, /WE0, /CS2 active
	#define RC1_SEQUENCE "\x80\x15\x42"	// MB1CR: 2 ws, /OE0, /WE0, /CS2 active
   #define RC2_SEQUENCE "\x80\x16\x40" // MB2CR: 2 ws, /OE0, /WE0, /CS0 active
   #define RC3_SEQUENCE "\x80\x17\x40" // MB3CR: 2 ws, /OE0, /WE0, /CS0 active
#elif 6 == _RAM_CONTROL_OPTION
   #define RC0_SEQUENCE
   #define RC1_SEQUENCE "\x80\x15\x45"
   #define RC2_SEQUENCE "\x80\x16\x45"
   #define RC3_SEQUENCE "\x80\x17\x45"
#else
	#error "Unsupported RAM control lines option!"
#endif

#if 0 == ENABLE_SPREADER_OPTION
	// OK, but spectrum spreader is disabled.
#elif 1 == ENABLE_SPREADER_OPTION
	#define ES_SEQUENCE "\x80\x0A\x00"	// GCM0R: normal spreading
#elif 2 == ENABLE_SPREADER_OPTION
	#define ES_SEQUENCE "\x80\x0A\x80"	// GCM0R: strong spreading
#else
	#error "Unsupported spectrum spreader option!"
#endif

#if 0 == BOOTSTRAP_FROM_16BIT
   #define SEGSIZE_SEQUENCE  "\x80\x13\xc6"  // SEGSIZE
   #define STACKSEG_SEQUENCE "\x80\x11\x74"  // STACKSEG
   #define DATASEG_SEQUENCE  "\x80\x12\x3a"  // DATASEG
#else
   #define SEGSIZE_SEQUENCE  "\x80\x13\xc1"  // SEGSIZE
   #define STACKSEG_SEQUENCE "\x80\x11\x80"  // STACKSEG
   #define DATASEG_SEQUENCE  "\x80\x12\x7f"  // DATASEG
#endif
/*
 *	This is the first few bytes of the coldload sequence.  It is
 *	used to set up some of the configuration registers.
 */

const char start_sequence[] =
#if 0 < ENABLE_SPREADER_OPTION
   "\x80\x0E\xA0"  // GOCR: CLK and STATUS low, /WDTOUB and /BUFEN active low
   "\x80\x19\x0C"  // MTCR: /OE0, /OE1 to toggle 1/2 clock early
   ES_SEQUENCE
   "\x80\x0B\x80"  // GCM1R: spreader on
#endif
	"\x80\x00\x08"  // GCSR: no periodic int, main osc no div
	"\x80\x10\x00"  // MMIDR: normal operation
	RC0_SEQUENCE
	RC1_SEQUENCE
	RC2_SEQUENCE
	RC3_SEQUENCE
   SEGSIZE_SEQUENCE
   STACKSEG_SEQUENCE
   DATASEG_SEQUENCE
#if 0 < BOOTSTRAP_FROM_16BIT
   "\x80\x73\x01"  // enable PE4 alt function as /A0
   "\x80\x75\x10"  // enable PE4 as alt output
   "\x80\x1D\x20"  // enable basic 16-bit mode for /CS1
#endif
#if 0 < DISABLE_WATCHDOG
   "\x80\x09\x51"
   "\x80\x09\x54"  // disable watchdog
#endif
#if 1 == COLDLOAD_COMPILE
   "\x80\xc4\x00"  // set serial port A async, 8 bit, pport C input
   "\x80\xa0\x01"  // enable timer A with cpuclk/2
#endif
	;
   const char end_sequence[] =


#if 0 < BOOTSTRAP_FROM_16BIT
   "\x80\x1D\x00"  // 8-bit memories
   "\x80\x14\x4D"  // MB0CR (reads only; a spurious write will occur during
                   // the switchover to 16-bit mode)
   "\x80\x13\xc6"  // SEGSIZE
   "\x80\x11\x74"  // STACKSEG
   "\x80\x12\x3a"  // DATASEG
#endif
   "\x80\x24\x80";

int main(int argc, char* argv[])
{
	int h_in, h_out;
   unsigned int x;
   char ch;
   char seq[3];
   char *ptr2me;

   if (argc!=3) {
      // the following '\\' assumes Windows style pathlist separator!
   	if ((ptr2me = strrchr(argv[0], '\\')) == NULL) {
      	ptr2me = argv[0];	// just use the whole string
      } else {
      	ptr2me++;			// skip the pathlist separator
      }
   	printf("usage:  %s <source file> <destination file>\n", ptr2me);
      return 1;
   }

   printf("source=%s, destination=%s\n",argv[1],argv[2]);

   if((h_in=open(argv[1],O_BINARY|O_RDONLY))<0) {
   	perror("error opening source file\n");
      return 1;
   }

   if((h_out=open(argv[2],O_BINARY|O_CREAT|O_EXCL|O_RDWR,
   	S_IREAD|S_IWRITE))<0) {
   	perror("error creating destination file\n");
      return 1;
   }

   if(write(h_out,start_sequence,sizeof(start_sequence)-1)<0) {
   	printf("error writing start sequence\n");
   }

   for(x=0;x<32768&&!eof(h_in);x++) {
   	if(read(h_in,&ch,1)<0) {
      	printf("error reading input file\n");
         return 1;
      }
#if 0 < BOOTSTRAP_FROM_16BIT
      seq[0]=0x10 + (x>>8)&0xff;
#else
      seq[0]=(x>>8)&0xff;
#endif
      seq[1]=x&0xff;
      seq[2]=ch;

      if(write(h_out,seq,3)<0) {
      	printf("error writing output file\n");
         return 1;
      }
   }

   if(write(h_out,end_sequence,sizeof(end_sequence)-1)<0) {
   	printf("error writing end sequence\n");
   }

   if(x>=32768)
   	printf("warning:  input file may have been too long\n");

   printf("output file successfully created (%d bytes)\n",
   	x*3+sizeof(start_sequence)-1);
   return 0;
}
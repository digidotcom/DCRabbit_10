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

     	Samples\SERIAL_STDIO.C

      Demonstration of redirection of STDIO output from
      Dynamic C's STDIO window to a serial port (this
      feature was added in Dynamic C 7.25).

      NOTE: these macros are designed for debugging use only!
		They allow STDIO functions to be redirected to the
		serial ports for viewing in run mode -- these macros
		are not designed to be a standard part of a Dynamic C
		program (character input is not buffered, for example).

		When in debug mode, STDIO will go to the STDIO window
		as usual.  When in run mode, STDIO will go to the
		designated serial port.  This is especially useful since
		the programming port (serial port A) can be used with
		the PROG header for debug mode and the DIAG header
		in run mode without recompiling the program, keeping the
		other serial ports free for application use.

		The desired serial port is selected by adding ONE of the
		following lines to your program:

				#define	STDIO_DEBUG_SERIAL	SADR
				#define	STDIO_DEBUG_SERIAL	SBDR
				#define	STDIO_DEBUG_SERIAL	SCDR
				#define	STDIO_DEBUG_SERIAL	SDDR
				#define	STDIO_DEBUG_SERIAL	SEDR
				#define	STDIO_DEBUG_SERIAL	SFDR

		If you are using a BLxS2xx SBC, then your only options are:

				#define	STDIO_DEBUG_SERIAL	SEDR
				#define	STDIO_DEBUG_SERIAL	SFDR

		Then specify the desired baud rate with a define like
		the following:

				#define	STDIO_DEBUG_BAUD	57600

		Many PC-based terminal programs expect a carriage return
		as well as a newline character at the end of each line.
		A carriage return ('\r') can be added to each linefeed ('\n')
		in a printed string by adding the following line:

				#define	STDIO_DEBUG_ADDCR

		If serial debugging is desired even when in programming mode,
		it can be forced by adding the following line:

				#define	STDIO_DEBUG_FORCEDSERIAL

************************************************************/
#class auto


// Add these lines to redirect run-mode printf output at 57600 baud
// to serial port B (serial port C on RCM43x0, serial port D on
// BL4S100 family and serial port E on BLxS200 family (except BL4S210)).

#if (BLXS200_SERIES && _BOARD_TYPE_ != RCM4010)
   #define  STDIO_DEBUG_SERIAL   SEDR
#elif BL4S100_SERIES
   #define  STDIO_DEBUG_SERIAL   SDDR
#elif RCM4300_SERIES
	#define  STDIO_DEBUG_SERIAL   SCDR
#else
   #define  STDIO_DEBUG_SERIAL   SBDR
#endif

#define	STDIO_DEBUG_BAUD		57600

// Add this line to add carriage returns ('\r') to each newline
// char ('\n') when sending printf output to a serial port.

#define	STDIO_DEBUG_ADDCR

// Add this line to force both run-mode and program-mode printf
// output to a serial port.

//#define	STDIO_DEBUG_FORCEDSERIAL


void main()
{
	char	s[256];

	printf("This is a test of printf redirect.\n");
	printf("this is the second line of the test.\n");

	printf("Please enter a string: ");
	gets(s);
	printf("You entered '%s'.\n", s);

	while (1)
		;	// avoid watchdog timeout and restart in run mode
}
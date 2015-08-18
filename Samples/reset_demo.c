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

   Samples\reset_demo.c

   Demonstration of using Rabbit reset functions and a protected variable.

   This program will print out to the selected serial port the reaon why the
   last reset occurred, then cause either a watchdog timeout or a soft reset
   according to a protected static counter.  On serial boot flash devices
   (ie. RCM43xx core modules), the watchdog timeout causes a hardware reset,
   so the watchdog reset message will be not be shown and will be replaced
   by the hardware reset message.

   To properly run this demo, compile it to flash, disconnect the programming
   cable, connect the selected serial port to your computer at 19200 baud, and
   hit the reset button (or cycle the board's power).  The controller will then
   start running the program.  A hard (power fail) reset can be forced by
   pressing the reset button or cycling the power.

   NOTE 1:  It is necessary to disconnect the target controller's programming
            cable in order for the target to run the loaded program after reset.
            If not disconnected from the programming cable, after reset the
            target controller executes its bootstrap loader loop instead of the
            previously loaded code.

   NOTE 2:  Loss of target communication will result when a target controller is
            power-cycled or reset while it is communicating with Dynamic C when
            running in debug mode.

*******************************************************************************/
#class auto

#define  BAUD_RATE	19200L

// MY_SERIAL_PORT_CHOICE must be one of 1 (port A) through 6 (port F).
#define MY_SERIAL_PORT_CHOICE 4

#if 1 == MY_SERIAL_PORT_CHOICE
   #define AINBUFSIZE 31
   #define AOUTBUFSIZE 31
   #define MY_SER_OPEN serAopen
   #define MY_SER_PUTS serAputs
#elif 2 == MY_SERIAL_PORT_CHOICE
   #define BINBUFSIZE 31
   #define BOUTBUFSIZE 31
   #define MY_SER_OPEN serBopen
   #define MY_SER_PUTS serBputs
#elif 3 == MY_SERIAL_PORT_CHOICE
   #define CINBUFSIZE 31
   #define COUTBUFSIZE 31
   #define MY_SER_OPEN serCopen
   #define MY_SER_PUTS serCputs
#elif 4 == MY_SERIAL_PORT_CHOICE
   #define DINBUFSIZE 31
   #define DOUTBUFSIZE 31
   #define MY_SER_OPEN serDopen
   #define MY_SER_PUTS serDputs
#elif 5 == MY_SERIAL_PORT_CHOICE
   #define EINBUFSIZE 31
   #define EOUTBUFSIZE 31
   #define MY_SER_OPEN serEopen
   #define MY_SER_PUTS serEputs
#elif 6 == MY_SERIAL_PORT_CHOICE
   #define FINBUFSIZE 31
   #define FOUTBUFSIZE 31
   #define MY_SER_OPEN serFopen
   #define MY_SER_PUTS serFputs
#else
   #error "MY_SERIAL_PORT_CHOICE must be one of 1 (port A) through 6 (port F)."
#endif

void delayMs(unsigned long);

void main()
{
   static protected long counter;
   static char s[256];

   _sysIsSoftReset();	// recover any protected variables

#if (BLXS200_SERIES && _BOARD_TYPE_ == RCM4010)
   // Switch jumperless port routing for BL4S210 SBC serial port
   WrPortI(SPCR,  &SPCRShadow,  0x8C);
   WrPortI(PEFR,  &PEFRShadow,  (PEFRShadow | 1));
   WrPortI(IB0CR, &IB0CRShadow, 0x48);
   WrPortE(0x30, NULL, 0x4C);
#endif

   MY_SER_OPEN(BAUD_RATE);		// open selected serial port at 19200 baud

   // check why the controller was last reset
   if (chkHardReset()) {
      sprintf(s, "Hard reset (power fail) occurred.\r\n");
      MY_SER_PUTS(s);
   }

   if (chkWDTO()) {
      sprintf(s, "Watchdog timeout occurred.\r\n");
      MY_SER_PUTS(s);
   }

   if (chkSoftReset()) {
      sprintf(s, "'Soft' reset (by software) occurred.\r\n");
      MY_SER_PUTS(s);
   }

   delayMs(1000);
   counter++;				// increment counter

   if (counter & 0x01) {		// force a watchdog timeout
      // allocate one virtual watchdog, timing = 1/16 sec
      VdGetFreeWd(1);
      while (1);
   } else {
      forceSoftReset();
   }
}

void delayMs(unsigned long delay)
{
	auto unsigned long time0;

	for (time0 = MS_TIMER; MS_TIMER - time0 < delay; ) ;
}


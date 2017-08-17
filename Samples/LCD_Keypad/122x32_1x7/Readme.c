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
/*=========================================================
     Information for using the Graphic Sample Programs
===========================================================

Graphic Sample Program Information
----------------------------------
The graphic sample programs are intended for Rabbit's
development boards or controllers that have a graphic
LCD display module (or optional connectors).

See Samples/RCM6700/LCD_Keypad and Samples/RCM6600W/LCD_Keypad for
additional samples targeting those boards.

If you compile a graphic sample program on a product
that wasn't designed with a Graphic LCD, then you will
get compiler errors because target controller has not been
setup with #use'ing the graphic libraries.

If you have custom wired a Rabbit 122x32 display with a
1x7 keypad, then you will need to #use the LCD122KEY7.lib
library and specify several define values to tell the library
how the display and keypad are connected.

DEFINITIONS NEEDED FOR CUSTOM WIRING OF LCD122KEY7 module:

#define LCDCSREGISTER  // I/O strobe usually port E pin
#define LCDCSSHADOW	  // Shadow register for LCDCSREGISTER
#define LCDCSCONFIG	  // Usually 7 waits, I/O Rd & Wr data strobe, allow writes
#define LCDSTROBE		  // Bit mask of I/O strobe pin (ie. PE4 = 0x10)
#define LCDBASEADDR	  // A15, A14, A13 = I/O Address from IBxCR
#define KEYCSREGISTER  // I/O strobe usually same as LCDCSREGISTER
#define KEYCSSHADOW	  // Shadow register for KEYPADCSREGISTER
#define KEYCSCONFIG	  // Usually same as LCDCSCONFIG
#define KEYSTROBE		  // Bit mask of I/O strobe pin, usually same as LCDSTROBE
#define KEYBASEADDR 	  // Usually same as LCDBASEADDR

Note that RCM67xx.LIB and RCM66xxW.LIB include definitions appropriate for
connecting to LCD/Keypad interface circuitry on the RCM3000 prototyping board.

If you're trying to use these sample programs with your
own custom Graphic LCD application, instructions for
creating a graphic LCD low-level driver is located in
the following drivers:

DC\LIB\Displays\Graphic\122x32\MSCG12232.LIB
	" OR "
DC\LIB\Displays\Graphic\320x240\SED1335F.LIB

Once you design your low-level driver you will need to #use
the graphic.lib and font libraries for your application.
Here's an example for the MSCG12232 LCD module that is used
on the OP6800 product:

// for a 122x32 LCD display
#use "mscg12232.lib"    // Low-level driver......LCD specific
#use "graphic.lib"      // High-level functions..Required
#use "6x8l.lib"		   // Font Lib..............Optional
#use "8x10l.lib"        // "  	" 	"
#use "12x16l.lib"       // "  	"	"
#use "courier12.lib"    // "  	" 	"

The above is located in default.h in the OP6800 ID area, however for
custom applications you can #use these libraries at the top of your
application program.


=======================================================================*/
EOF





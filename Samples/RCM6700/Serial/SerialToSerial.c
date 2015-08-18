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
   SerialToSerial.c

   This program is used with RCM57xx or RCM67xx series controllers, an interface
   board, a digital I/O accessory board, and optionally, a serial I/O accessory
   board.

   Description
   ===========
   This program monitors switches S1, S2, S3 and S4 on the digital I/O accessory
   board and lights LEDs DS1-DS4 while a corresponding button switch is pressed.
   It also sends messages indicating that a switch is pressed from serial port D
   to serial port C. Messages received by serial port C are displayed in Dynamic
   C's stdio window.

   A serial port accessory board can be used, but the two serial ports can
   communicate at TTL levels with direct connections on the digital I/O
   accessory board also by simply connecting:
      J2 pin 19 (PC0/TXD) to J2 pin 22 (PC3/RXC).

   If using the Serial Port Accessory board, you should instead connect:
      J3 pin 3 (TXD) to J4 pin 5 (RXC).

   Use the following jumper placements on the digital I/O accessory board:
      I/O control       On Digital I/O board
      ---------------   ---------------------
      Port A bits 4-7   LEDs DS1-DS4
      Port B bits 4-7   Button switches S1-S4

      Jumper settings (digital I/O board only, except as noted)
      ---------------------------------------------------------

      JP5   1-2, 3-4, 5-6, 7-8 (and serial I/O board, if used)
      JP8   1-2, 3-4, 5-6, 7-8

         2    4    6   8
         o    o    o   o
         |    |    |   |
         o    o    o   o
         1    3    5   7

      JP7   2-4, 3-5 (connect switch pins to GND and 3.3V)

         1 o    o 2
                |
         3 o    o 4
           |
         5 o    o 6

         7 o    o 8

   Instructions
   ============
   1. Make sure jumpers are connected as shown above.
   2. Compile and run this program.
   3. Press switches S1-S4 on the Digital I/O board to see corresponding LEDs
      DS1-DS4 activity and serial messages in the sdtdio window.
*******************************************************************************/

/*******************************************************************************
   Set the serial ports' communication rate.
*******************************************************************************/
#define BAUDRATE    9600

/*******************************************************************************
  The serial ports' input and output buffers sizes are defined here. If these
  are not defined to be (2^n)-1, where n = 1...15, or they are not defined at
  all, they will default to 31 and a compiler warning will be displayed.
*******************************************************************************/
#define CINBUFSIZE  63
#define COUTBUFSIZE 15
#define DINBUFSIZE  15
#define DOUTBUFSIZE 63

/*******************************************************************************
   LEDs' (DSx) and switches' (Sx) bit definitions.
*******************************************************************************/
#define DS1         4
#define DS2         5
#define DS3         6
#define DS4         7
#define S1          4
#define S2          5
#define S3          6
#define S4          7

/*******************************************************************************
   Local communication (string) buffers' size.
*******************************************************************************/
#define STRLEN      32

void InitIO(void)
{
	// Make Port B pins for switch inputs
	BitWrPortI(PBDDR, &PBDDRShadow, 0, S1);
	BitWrPortI(PBDDR, &PBDDRShadow, 0, S2);
	BitWrPortI(PBDDR, &PBDDRShadow, 0, S3);
	BitWrPortI(PBDDR, &PBDDRShadow, 0, S4);

	// Set Port A pins for LEDs low
	BitWrPortI(PADR, &PADRShadow, 1, DS1);
	BitWrPortI(PADR, &PADRShadow, 1, DS2);
	BitWrPortI(PADR, &PADRShadow, 1, DS3);
	BitWrPortI(PADR, &PADRShadow, 1, DS4);

	// Make Port A bit-wide output
	BitWrPortI(SPCR, &SPCRShadow, 1, 2);
	BitWrPortI(SPCR, &SPCRShadow, 0, 3);
}

void main(void)
{
	char outString[STRLEN], inString[STRLEN];
	int i, j, ch;

	serDopen(BAUDRATE);
	serCopen(BAUDRATE);

	InitIO();

	j = 0;

	while (1)
	{
		costate
		{
			// Bits for switches and LEDs correspond
			for (i = S1; i <= S4; ++i)
			{
				if (!BitRdPortI(PBDR, i))
				{
					// Delay so we don't output too many times
					waitfor (DelayMs(300));

					// Light corresponding LED
					BitWrPortI(PADR, &PADRShadow, 0, i);

					sprintf(outString, "Switch %d is on\n", i-3);

					// Make sure there's room in ouput buffer
					waitfor (serDwrFree() > strlen(outString));
					// Send string
					serDputs(outString);
				}
				else
				{
					BitWrPortI(PADR, &PADRShadow, 1, i);  // LED off
				}
			}
		}

		// Wait for any ASCII input
		if (!(0xFF00 & (ch = serCgetc())))
		{
			inString[j] = ch;
			++j;
			if (j >= STRLEN)					// Make sure we don't overflow
			{										//  array bounds in case 0x0A
				j = 0;							//  gets dropped.
			}
			if (ch == 0x0A)					// Check for new line as delimiter
			{
				inString[j] = 0;				// NULL terminate
				printf("%s",inString);
				j = 0;
			}
		}
	}
}
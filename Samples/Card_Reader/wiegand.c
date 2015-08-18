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
/*
 * wiegand.c
 *
 * Sample to demonstrate use of Wiegand card reader.
 * This sample assumes that the zero bit line is connected to PE0
 * and the one bit line is connected to PE4.
 *
 * Just for kicks, if you initially #define BOX_N_DICE then it
 * is assumed that
 *   PA5 = active low RED LED
 *   PA6 = active low GREEN LED
 *   PA7 = active low BEEPER
 * This macro is only used in this demo program (not in wiegand.lib).
 */

//#define WIEGAND_VERBOSE	// Get the library to print messages for us.
//#define WIEGAND_DEBUG	// Allow Dynamic C debugging of the

#define BOX_N_DICE		// Assume Motorola Indala reader or equivalent
								// (This makes no difference to the basic functionality,
                        // it just assumes that some LEDs and beeper are hooked up).

#define MY_CARD_HI	0x620		// MSBs of 'my card'
#define MY_CARD_LO	0x3AF		// LSBs of 'my card'

#use "wiegand.lib"

int main()
{
	int rc;
   unsigned long res[2], raw_res[2];
   word tt;

#ifdef BOX_N_DICE
	WrPortI(PADR, &PADRShadow, 0xFF);	// All high (inactive)
	WrPortI(SPCR, &SPCRShadow, 0x84);	// Port A outputs
#endif

	rc = wiegand_init(0, 0, 0, 0, 0, 26);

   if (rc) {
   	printf("Failed to initialize.  Return code = %d\n", rc);
      exit(1);
   }

   for (;;) {
   	if (wiegand_ready()) {
			rc = wiegand_result(0, raw_res, res);
#ifdef BOX_N_DICE
         if (rc == WIEGAND_OK) {
         	// If the card is 'my card', flash green LED.
            // Otherwise, flash red LED and beep.
            if (res[0] == MY_CARD_LO && res[1] == MY_CARD_HI) {
            	printf("Welcome!  Please enter...\n\n");
            	WrPortI(PADR, &PADRShadow, 0xBF);
            }
            else {
            	printf("Hello, stranger!  Please find a friendly security officer...\n\n");
            	WrPortI(PADR, &PADRShadow, 0x5F);
            }
            // Hang around for 500ms
				tt = (word)MS_TIMER + 500;
            while ((int)((word)MS_TIMER - tt) < 0);
            WrPortI(PADR, &PADRShadow, 0xFF);
         }
#endif
      }
   }
}
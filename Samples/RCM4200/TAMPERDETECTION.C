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
/*****************************************************************************

	TamperDetection.C

	This sample is designed for the RCM4200 series and prototyping boards.

   Tamper Detection detects any attempt to enter bootstrap mode.  When this
   is detected, the VBAT RAM is erased.  This can be useful for storing data
   from a remote location such as an AES encryption key.

   This sample shows how to load and read the VBAT RAM as well as enabling a
   visual indicator for detection.

	INSTRUCTIONS:

	1. Compile and run this sample.
   2. Remove the programming cable and reset the board.
   3. Press switch 2 to load the VBAT RAM with the key.  This changes the LED's
      to solid state.  Notice that reseting the board does not change this.
   4. Plug in the programming cable briefly and unplug it again. Now the LED's
      are flashing because the VBAT RAM has been erased. Notice that reseting
      the board does not change this.
   5. Feel free to repeat steps 3 and 4.

   NOTE:
   -----
   Actual recommended use of this feature would not store the 'key' in the
   flash, but retrieve it from a remote connection during runtime.

*****************************************************************************/

#use RCM42xx.LIB

#define DS2_BIT 2
#define DS3_BIT 3
#define S2_BIT  4

#define SECURE 1

const char key[] = "J8BPN392401CZ1LQSSF2O3LMC8W6AK3X";
char buffer[32];

int main()
{
	int status, led2, led3, sw2;

	// Initialize I/O pins
	brdInit();

   led2 = 0;
   led3 = 1;
   sw2 = 0;
   status = !SECURE;

   while(1) {
      costate {
         waitfor(DelayMs(200));
         if(status == SECURE) {
            // LED's on
	         BitWrPortI(PBDR, &PBDRShadow, 0, DS2_BIT);
	         BitWrPortI(PBDR, &PBDRShadow, 0, DS3_BIT);
         }
         else {
            // Flash LED's
	         led2 = !led2;
	         led3 = !led2;
	         BitWrPortI(PBDR, &PBDRShadow, led2, DS2_BIT);
	         BitWrPortI(PBDR, &PBDRShadow, led3, DS3_BIT);
         }
      }
      costate {
         // Wait for switch S2 press
			if(BitRdPortI(PBDR, S2_BIT))
				abort;
			waitfor(DelayMs(50));				// switch press detected
			if(BitRdPortI(PBDR, S2_BIT))	// wait for switch release
			{
				sw2 = !sw2;							// set valid switch
				abort;
			}
      }
      costate {
         // Periodically check the VBAT RAM
         memset(buffer, 0, 32);
         vram2root(buffer, 0, 32);

         if(memcmp(buffer, key, 32) == 0)
            status = SECURE;
         else
            status = !SECURE;

         waitfor(DelayMs(1000));
      }
      costate {
         // If the switch is pressed, reset the VBAT RAM
         if(sw2) {
            root2vram(key, 0, 32);
            sw2 = !sw2;
         }
      }
   }
}
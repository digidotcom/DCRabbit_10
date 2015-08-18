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
/**************************************************************************

	switchchar.c

	This program is used with RCM5400W series controllers
	and prototyping boards.

	Description
	===========
  	This program transmits and then receives an ASCII string on serial
	ports C and D. It also displays the serial data received from both
	ports in STDIO window.

	Proto-Board Connections
  	=======================
   Place wire jumpers on RS232 connector:

     	   TXD <---> RXC
         RXD <---> TXC

  	Instructions
   ============
   1. Compile and Run this program.
   2. Press and release S2 and S3 on the proto-board.
	3. View data sent between serial ports in the STDIO window.

**************************************************************************/
#class auto

#define ON	1
#define OFF 0

// The input and output buffers sizes are defined here. If these
// are not defined to be (2^n)-1, where n = 1...15, or they are
// not defined at all, they will default to 31 and a compiler
// warning will be displayed.

#define CINBUFSIZE 	255
#define COUTBUFSIZE 	255

#define DINBUFSIZE 	255
#define DOUTBUFSIZE  255

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long time0;

	for (time0 = MS_TIMER; MS_TIMER - time0 < delay; ) ;
}


///////////////////////////////////////////////////////////////////////////
// S2 uses PB4
#define S2_PORT		PBDR
#define S2_SHADOW		PBDRShadow
#define S2_BIT 		4

// S3 uses PB5
#define S3_PORT		PBDR
#define S3_SHADOW		PBDRShadow
#define S3_BIT 		5

main()
{
	auto int i, ch;
	auto char buffer[64];	//buffer used for serial data
	auto int sw1, sw2;

	static const char string1[] = {"This message has been Rcv'd from serial port C !!!\n\n\r"};
	static const char string2[] = {"This message has been Rcv'd from serial port D !!!\n\n\r"};

	sw1 = sw2 = 0;							//initialize switch to false value

   // Initialize serial port C, set baud rate to 19200
 	serCopen(19200);
	serCwrFlush();
 	serCrdFlush();

  	// Initialize serial port D, set baud rate to 19200
   serDopen(19200);
   serDwrFlush();
   serDrdFlush();

	// Clear data buffer
   memset(buffer, 0x00, sizeof(buffer));

   printf("\nStart of Sample Program!!!\n\n\n\r");
   //---------------------------------------------------------------------
   // Do continuous loop transmitting data between serial ports C and D
   //---------------------------------------------------------------------
	while(1) {
		costate {
			if (BitRdPortI(S2_PORT, S2_BIT))		//wait for switch press
				abort;
			waitfor(DelayMs(50));
			if (BitRdPortI(S2_PORT, S2_BIT)) {	//wait for switch release
				sw1 = !sw1;
				abort;
			}
		}

		costate {
			if (BitRdPortI(S3_PORT, S3_BIT))		//wait for switch press
				abort;
			waitfor(DelayMs(50));
			if (BitRdPortI(S3_PORT, S3_BIT)) {	//wait for switch release
				sw2 = !sw2;
				abort;
			}
		}

		costate {
      	// toggle led upon valid switch press/release
			if (sw1) {
				sw1 = !sw1;

            // The switch is attached the the serial port, so we need to read
            // the junk it sends
            serCrdFlush();
   			// Transmit an ascii string from serial port D to serial port C
				memcpy(buffer, string2, strlen(string2));
   			serDputs(buffer);
				memset(buffer, 0x00, sizeof(buffer));

   			// Get the data string that was transmitted by port D
		    	i = 0;
		     	while((ch = serCgetc()) != '\r')	{
		     		// Copy only valid RCV'd characters to the buffer
					if(ch != -1) {
						buffer[i++] = ch;
					}
				}
				buffer[i++] = ch;			 //copy '\r' to the data buffer
     			buffer[i]   = '\0';      //terminate the ascii string

		     	// Display ascii string received from serial port D
     			printf("%s", buffer);

		  		// Clear buffer
				memset(buffer, 0x00, sizeof(buffer));
			}
		}

		costate {
      	// toggle led upon valid switch press/release
			if (sw2) {
				sw2=!sw2;

            // The switch is attached the the serial port, so we need to read
            // the junk it sends
            serDrdFlush();
		   	// Transmit an ascii string from serial port C to serial port D
				memcpy(buffer, string1, strlen(string1));
     			serCputs(buffer);
		     	memset(buffer, 0x00, sizeof(buffer));

				// Get the data string that was transmitted by serial port C
     			i = 0;
		     	while((ch = serDgetc()) != '\r') {
					// Copy only valid RCV'd characters to the buffer
					if(ch != -1) {
						buffer[i++] = ch;
					}
				}
		     	buffer[i++] = ch; 		//copy '\r' to the data buffer
		     	buffer[i]   = '\0';     //terminate the ascii string

	     		// Display ascii string received from serial port C
		   	printf("%s", buffer);
			} //endif
		} //endcostate
	} //endwhile
}
///////////////////////////////////////////////////////////////////////////


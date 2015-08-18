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

	computer_parity.c

	This sample program is for the BL4S1xx series SBCs.

   Description:
	============
	This program demonstrates using parity over a
	simple RS232 3-wire connection.  Parity is selected
   for the Rabbit and for the serial terminal program.
   Characters typed in either the stdio window or in a
   serial terminal program are echoed in both displays.
   Parity errors are counted and displayed by the Rabbit.

	Instructions:
	=============
   1.	 Connect BL4S1xx Serial port to computer.
   2.	 Start serial terminal program (Moni, Terra-Term, and
   	 HyperTerminal for Windows work well).
   3.  Configure terminal program to Baud defined below with
   	 8 data bits, 1 stop bit, and either no, odd, or even parity.
   4.  Compile and run this program.
	5.	 Configure the serial port on the Rabbit by using the menu options
   	 in the STDIO window.
       Menu:
	      q - Quit
	      s - Send "Sample Text"
	      r - Reset Counters
	      n - Set No Parity
	      e - Set Even Parity
	      o - Set Odd Parity
	6.  Type in the terminal program to send characters to the program.
   	 The characters typed will be echoed in the terminal program and
       will be displayed on the top of the STDIO window with a message
       displaying whether there was an error or not.  There may be some
       differences for special characters like newlines (enter key),
       delete, backspace, and others.  Each character sent will also
       increment either the successful or error counter depending on the
       parity of both the Rabbit and the terminal program.


***************************************************************************/
#class auto	 // Change local var storage default to "auto"

// include BL4S1xx series library
#use "BL4S1xx.lib"

// serial buffer size
#define DINBUFSIZE  63
#define DOUTBUFSIZE 15

// serial baud rate
#ifndef _232BAUD
#define _232BAUD 115200
#endif

// Different parity types
char* const parity_strings[] = {"No", "Even", "Odd"};

// global constants
int tx_parity, success_count, error_count;

// updates the display in Dynamic C with the current parity and the number
// of errors
void update_display(void)
{
   // display the data
   printf("Current Mode: %s Parity  \n" \
   		 "Serial Success Count: %d\n" \
   		 "Parity Error Count: %d\n", parity_strings[tx_parity],
          success_count, error_count);

   printf("Menu: \n" \
          "q - Quit\n" \
          "s - Send \"Sample Text\"\n" \
          "r - Reset Counters\n" \
          "n - Set No Parity\n" \
          "e - Set Even Parity\n" \
          "o - Set Odd Parity\n");
   printf("\nInput Command: ");
}

int parse_menu(char input)
{
   switch(input)
   {
      case 'q':
      case 'Q':
      {
         // quit the program
			printf("quiting\n\n");
         exit(0);
      }
      case 's':
      case 'S':
      {
         // send a sample text
         printf("Sending sample text\n\n");
         serDputs("Sample Text\r\n");
         break;
      }
      case 'r':
      case 'R':
      {
         // reset counters
         printf("Reseting counters\n\n");
         success_count = 0;
         error_count = 0;
         break;
      }
      case 'n':
      case 'N':
      {
         // set no parity
         printf("Setting no parity\n\n");
         tx_parity = PARAM_NOPARITY;
         serDparity(tx_parity);
         break;
      }
      case 'e':
      case 'E':
      {
         // set even parity
         printf("Setting even parity\n\n");
         tx_parity = PARAM_EPARITY;
         serDparity(tx_parity);
         break;
      }
      case 'o':
      case 'O':
      {
         // set odd parity
         printf("Setting odd parity\n\n");
         tx_parity = PARAM_OPARITY;
         serDparity(tx_parity);
         break;
      }
      default:
      {
         // bad value
         printf("Bad input\n\n");
         return -1;
      }
   } // switch
   return 0;
}


void main()
{
   auto int rs232_received, menu_received;
   auto char error_code;

   // Initialize the controller
	brdInit();

   // open serial port
   serDopen(_232BAUD);

	// disable flow control
	serMode(0);

   // Clear serial data buffers
	serDwrFlush();
	serDrdFlush();

	// start with no parity
	tx_parity = PARAM_NOPARITY;
	serDparity(tx_parity);

   // Initialize the counters
   success_count = 0;
   error_count = 0;

   // print header
   printf("Starting Parity Sample\n\n\n");

   // update the display
   update_display();

	while (1)
	{
      // parse menu
      if (kbhit())
      {
         menu_received = getchar();

         // clear the stdio window
         printf ( " \x1Bt" );

         // reprint the input line
         printf("Input Command: %c\n", menu_received);

         // parse the key hit (and print it)
         parse_menu(menu_received);

         // update the display
         update_display();
      }

		costate
		{
      	// Receive characters one at a time
 			if((rs232_received = serDgetc()) != -1)
         {
         	// convert carriage return to a newline
	         if (rs232_received == '\r')
	         {
	            rs232_received = '\n';
	         }

            // send back on serial port
            waitfor(serDputc(rs232_received));

            // clear the stdio window
            printf ( " \x1Bt" );

            error_code = serDgetError();

				if (error_code & SER_PARITY_ERROR)
				{
					++error_count;
               printf ("Received character with parity error: %c\n\n\n",
               			rs232_received);
				}
            else if (error_code & SER_OVERRUN_ERROR)
            {
            	++error_count;
               printf ("Received character with overrun error: %c\n\n\n",
               			rs232_received);
				}
            else if (error_code & SER_OVERFLOW_ERROR)
            {
            	++error_count;
               printf ("Received character with overflow error: %c\n\n\n",
               			rs232_received);
            }
            else
            {
            	++success_count;
               printf("Received character with no error: %c\n\n\n",
               		 rs232_received);
            }

            // update the display
            update_display();
	   	}
      }
	}
}
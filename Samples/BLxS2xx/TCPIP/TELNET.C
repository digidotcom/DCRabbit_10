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
   telnet.c

	This sample program is for the BLxS2xx series controllers.

   Description:
	============
   This program takes anything that comes in on a TELNET connection and sends
   it out serial port E (serial port B on a BL4S210) and to the STDIO window.
   It will also send any data that is entered on the serial port and send it
   through the TELNET connection.  It uses a digital input to indicate that
   the TCP/IP connection should be closed and a digital output to toggle an
   LED to indicate that there's an active connection.

	Connections:
	============

	****WARNING****: When using the J7 connector, be sure to insulate or cut
      the exposed wire from the wire leads you are not using.  The only
      connection required from J7 for any of the sample programs is +5v.

	1. DEMO board jumper settings:
			- Set switches to active low (ACT_L) by setting JP15 2-4 and 3-5.
			- Set LEDs to sinking by
            * removing all jumpers in JP3 and JP4
         	* setting all jumpers in JP1 and JP2

	2. Connect a wire from the controller J10 pin 5 GND to the DEMO board GND.

	3. Connect a wire from the controller J7 pin 6 (+5V) to the DEMO board +V.

   4. Connect the following wires from the controller J10 to the DEMO
	   board screw terminal:

         From J10 pin 9 DIO0 to LED1
	   	From J10 pin 4 DIO1 to SW1

	5. Use the following pins for the serial connection:

		For BL4S210:
		Use TXB (pin 1) and RXB (pin 6) located on J11.

		For all other BLxS2xx series boards:
		Use TXE (pin 1) and RXE (pin 6) located on J11.

	Instructions:
	=============
   1. Get help (ctrl-H) on TCPCONFIG for instructions on setting up
      IP addressing for this server sample.
	2. Compile and run this program.
   3. Start a Telnet session to this board.
   4. View LED1, when toggling it indicates that there's an active telnet
      connection.
   5. Press SW1 to close the telent connection, LED1 should stop toggling to
      indicate that the telnet connection has been closed.


*******************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

// Set default memory mapping to xmem.
#memmap xmem

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

// User macro for TCP port to use for processing
//  23 = telnet.
#define PORT 23

#use "dcrtcp.lib"

/*
 *		NOACT_TIMEOUT is the number of seconds of no activity before closing
 *		the connection.
 *
 */

#define NOACT_TIMEOUT		60


// Led control MACRO's
#define LED_FREQ				10

/*
 *		Serial Port Configuration
 *
 */

#define BAUD_RATE		19200L

#if _BOARD_TYPE_ == RCM4010
   #define BINBUFSIZE  127
   #define BOUTBUFSIZE 127
   #define serxopen    serBopen
   #define serxrdFlush serBrdFlush
   #define serxwrFlush serBwrFlush
   #define serxwrFree  serBwrFree
   #define serxwrite   serBwrite
   #define serxread    serBread
#else
   #define EINBUFSIZE  127
   #define EOUTBUFSIZE 127
   #define serxopen    serEopen
   #define serxrdFlush serErdFlush
   #define serxwrFlush serEwrFlush
   #define serxwrFree  serEwrFree
   #define serxwrite   serEwrite
   #define serxread    serEread
#endif

// enum of states used to parse commands
enum
{
   kStartCMD, 	// hit an 0xFF which represents a telnet command
	kNormal,    // normal characters at this point
   kRemoveByte,// we need to remove the next byte, as it is part of a telnet cmd
};

///////////////////////////////////////////////////////////////////////

/*
 *
 * Function to strip out telnet commands
 * Losses if there are NULL bytes inside of telnet string.
 */

int strip_telnet_cmds(char *buffer, int bytes_read)
{
   static int state;
   auto char *ptr1, *ptr2;
   auto char *end;

   // initialize static state
   #GLOBAL_INIT {
      state = kNormal;
   }

   // check for null buffer, or negative array size
   if (!buffer || bytes_read < 0)
   {
   	// buffer is null, reset the state variable and return -1
      state = kNormal;
      return -1;
   }

   // initialize the pointer to the end of the array
   end = buffer + bytes_read;

   // ptr2 traverses the buffer and copies all of the valid data (bytes that
   // are not part of the telnet commands) to ptr1, which also starts at the
   // beginning of the buffer.  bytes_read is updated everytime a byte from
   // a telnet command is encountered by ptr2.

   for (ptr1 = ptr2 = buffer; ptr2 < end; ++ptr2)
   {
		if (state == kNormal)
      {
      	// we are looking for telnet commands
         if (*ptr2 == 0xFF)
         {
         	// we hit a telnet command
            state = kStartCMD;
            --bytes_read;
         } else {
         	// transfer the valid data byte
            *ptr1 = *ptr2;
            ++ptr1;
         }
      } else if (state == kStartCMD)
      {
      	// we found a 0xFF last time though, lets parse the command
         switch(*ptr2)
         {
            case 0xFB:
            case 0xFC:
            case 0xFD:
            case 0xFE:
               // found three byte telnet command, remove next byte
               state = kRemoveByte;
               --bytes_read;
               break;

            case 0xFF:
               // two 0xFF in a row should only remove one of the 0xFFs
               state = kNormal;
               *ptr1 = *ptr2;
               ++ptr1;
               break;

            default:
               // this is a two byte telnet command, ignore this byte
               state = kNormal;
               --bytes_read;
               break;
         }
      } else if (state == kRemoveByte)
      {
      	// we are at the end of a three byte telnet command, ignore this byte
         state = kNormal;
         --bytes_read;
      } else {
      	// we should never be here
         return -1;
      }
	} // for

   return bytes_read;
}

// close TCP socket connection
void connection_close(tcp_Socket* socketPtr, CoData* TCP_CoData)
{
	static const char msg[] = "\n\rConnection closed\n\r";
   // close the socket on an error
   sock_close(socketPtr);
   // flush the rx serial buffer
   serxrdFlush();
   serxwrite(msg, sizeof(msg) - 1);
	// turn off the connection LED
   pulseDisable(0, 1);
	// restart the TCP_CoData costate from the start
   CoBegin(TCP_CoData);
}

void main()
{
   static CoData TCP_CoData;
   auto long timeout;
	auto int bytes_read;
   auto char buffer[64];
	auto tcp_Socket socket;
	auto unsigned long ip;

	brdInit();

   setPWM(0, LED_FREQ, 50, 0, 0);// Configure DIO0 as PWM output
   pulseDisable(0, 1);				// turn off led
   setDigIn(1);						// Configure DIO1 as a general digital input

	serxopen(BAUD_RATE);				// set up 3-wire serial port
   serMode(0);

	sock_init_or_exit(1);			// initialize DCRTCP
	tcp_reserveport(PORT);			// set up PORT for SYN Queueing
											// which will hold a pending connection
											// until we are ready to process it.


   ifconfig (IF_DEFAULT, IFG_IPADDR, &ip, IFS_END);
	printf ("Telnet to %s to be connected to serial port at %lu bps.\n\n",
   	inet_ntoa (buffer, ip), BAUD_RATE);

	while(1)
   {
      // call tcp-tick to process tcp data
      // Make sure that the connection hasn't closed on us.
      if(tcp_tick(&socket) == 0) {
         connection_close(&socket, &TCP_CoData);
      }

      costate
      {
         // watch for button press
			waitfor(!digIn(1)); 				// wait for switch 1 to be pressed
			waitfor(DelayMs(50));			// debounce
			if (!digIn(1)) {
         	connection_close(&socket, &TCP_CoData);
				waitfor(digIn(1));         // wait for switch 1 to be released
         }
      }

		costate TCP_CoData always_on
      {
			// initialize connection
			tcp_listen(&socket,PORT,0,0,NULL,0);

         // listen for new connection
         waitfor(sock_waiting(&socket) == 0);

         // make sure socket established
         if (sock_established(&socket))
         {
	         // connection now open
	         timeout = SEC_TIMER + NOACT_TIMEOUT; // set timeout
	         serxrdFlush();                   // flush serial buffer
	         pulseEnable(0);               // start the LED blinking
	         strip_telnet_cmds(NULL, 0);   // initialize the state variable
	         while(1)
	         {
	            // check for TCP traffic and button press
	            yield;

	            /*
	             *    read as many bytes from the socket as we have
	             *    room in the serial buffer. Also strip out the
	             *    telnet commands.
	             */

	            bytes_read = 0;
	            if(sock_bytesready(&socket) != -1)
	            {
	               bytes_read = sock_fastread(&socket, buffer,
                                            i_min(sizeof buffer, serxwrFree()));
	               bytes_read = strip_telnet_cmds(buffer, bytes_read);

	               if(bytes_read < 0)
	               {
	                  // close connection
	                  break;
	               } else if (bytes_read > 0)
	               {
	                  // copy any bytes that we read to the serial port
	                  timeout = SEC_TIMER + NOACT_TIMEOUT;
	                  serxwrite(buffer, bytes_read);
	                  // null terminate string and echo to stdio window
	                  buffer[bytes_read] = 0;
	                  printf("%s", buffer);
	               }
	            }

	            /*
	             *    read as many bytes from the serial port as we
	             *    have room in the socket buffer.
	             *
	             */

	            bytes_read = serxread(buffer,
	                            i_min(sizeof buffer, sock_tbleft(&socket)), 100);
	            if(bytes_read > 0) {
	               timeout = SEC_TIMER + NOACT_TIMEOUT;
						//write input back to serial port
	               serxwrite(buffer, bytes_read);
	               if(sock_fastwrite(&socket,buffer,bytes_read) < 0) {
	                  // close connection
	                  break;
	               }
	            }

	            // close the socket on a timeout
	            if((long) (SEC_TIMER-timeout) >= 0) {
	               // close connection
	               break;
	            }
	         } // while (1) when connection open
			}

	      // close connection
	      connection_close(&socket, &TCP_CoData);

		} // costate
	} // while(1)
}
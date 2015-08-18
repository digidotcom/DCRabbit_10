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
/********************************************************************
	tcp_respond.c
 	(Adapted from Intellicom's tcp_respond.c)

 	This sample program is used with controller boards
 	equipped with ethernet, LCD and keypad.  A 122x32 pixel
 	display and 1x7 keypad module with LED's are assumed.

   NOTE: Not currently supported on RCM4xxx modules.

	This program and tcp_send.c are to be executed on two different
	controller boards so that the two boards communicate with each other
	using a crossover cable.
	In the absence of a second board, pcsend.exe can be used
	on the PC console side at the command prompt.  This executable
	and source code is located in \samples\lcd_keypad\windows directory.

	This program waits for a message from another machine.  The message
	received is displayed on the LCD, and the user is allowed to respond
	by pressing a key on the keypad.  The response is then sent to the
	remote machine.

	Note: Currently only LEFT, UP and DOWN scroll keys are setup to
	      cause a response message.
********************************************************************/
#class auto		// Change default: local vars now stored on stack.
#memmap xmem  // Required to reduce root memory usage

#if CPU_ID_MASK(_CPU_ID_) >= R4000
#fatal "This sample is not currently supported by Rabbit 4000 based products."
#endif

fontInfo fi6x8;
windowFrame wholewindow;

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


// Port to listen on
#define PORT				4040

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

#define IGNORE 0x20

/*
 * General use Milli-sec delay function (don't use inside a co-statement).
 */
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + (unsigned long)delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

/*
 * Receive a message.  The connected socket and is sent back, and the need
 * for a response is indicated.
 */
void RecvMessage(tcp_Socket* tcpSock)
{
	auto char buffer[500];
	auto int numBytes;

	tcp_listen(tcpSock, PORT, 0, 0, NULL, 0);

	/* Wait for connection. */
	while( ! tcp_established( tcpSock ) )
	{
		if( ! tcp_tick( (sock_type *) tcpSock ) )
		{
			TextGotoXY(&wholewindow, 0, 0 );
			TextPrintf(&wholewindow, "Error: listen");
			return;
		}
	}

	/*  Wait for some data to appear, or an error... */
	while((numBytes = sock_fastread(tcpSock, buffer, sizeof(buffer))) == 0 )
	{
		if( ! tcp_tick( tcpSock ) || numBytes == -1 )
		{
			TextGotoXY(&wholewindow, 0, 0 );
			TextPrintf(&wholewindow, "Error: read");
			return;
		}
	}
	// Display received from other controller
	glBlankScreen();
	TextGotoXY(&wholewindow, 0, 0 );
	TextPrintf(&wholewindow, "Message received:");
	buffer[numBytes] = '\0';
	TextGotoXY(&wholewindow, 0, 1 );
	TextPrintf(&wholewindow, "%s", buffer);

}   /* end RecvMessage() */


/*
 * Send the response to the remote machine
 */
void SendMessage(tcp_Socket *tcpSock, unsigned key, char* messages[])
{

	sock_write(tcpSock, messages[key], strlen(messages[key])+1);
	sock_close(tcpSock);
	while (tcp_tick(tcpSock) != 0);
	return;
}

void flashled(int led)
{
	switch (led)
	{
		case 0:  led = 0; break;
		case 1:  led = 1; break;
		case 2:  led = 2; break;
		case 3:  led = 3; break;
		default: led = 6;
	}
	dispLedOut(led,1);
	msDelay(400);
	dispLedOut(led,0);
	msDelay(400);
}

void initsystem()
{

	brdInit();
	dispInit();
	keypadDef();
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);
	TextWindowFrame(&wholewindow, &fi6x8, 0, 0, LCD_XS, LCD_YS);
	glBackLight(1);
}


void main(void)
{
	tcp_Socket tcpSock;
	char *messages[12];
	unsigned int key;
	int i, send_response, test;

	// Initialize flag to indicate not ready to send response
	send_response = FALSE;

	// Initialize the message array with NULL pointers
	for (i = 0; i < (sizeof(messages)/sizeof(char*)); i++)
	{
		messages[i] = NULL;
	}

	// Define messages here--note that you only need define the messages
	// you will actually use.
	messages[0] = "I hear ya...";
	messages[1] = "Hello, there...";
	messages[2] = "It's a \"Rabbit\" Kind of place...";

	initsystem();
	sock_init();

	// Configure the upper row of keys on the keypad, in order
	// from left to right.
	for (i = 0; i < 7; i++)
	{
		// Only enable a key if there is a corresponding message
		if (messages[i] != NULL)
		{
			keyConfig ( i, ('0'+ i), 0, 0, 0, 0, 0 );
		}
		else
		{
			keyConfig ( i, IGNORE, 0, 0, 0, 0, 0 );
		}
	}

	while (1)
	{
		//	Process Keypad Press/Hold/Release
		costate
		{
			keyProcess ();
			waitfor ( DelayMs(10) );
		}

		costate
		{
			// Wait for a message from another device
			if (send_response == FALSE)
			{
				glBlankScreen();
				TextGotoXY(&wholewindow, 0, 0 );
				TextPrintf(&wholewindow, "Waiting for a \nmessage to be sent!");

				// Function is blocking until a message is received
				RecvMessage(&tcpSock);

				// Received message, prompt user to continue
				TextGotoXY(&wholewindow, 0, 3 );
				TextPrintf(&wholewindow, "Press key to proceed");

				// Allow the keyProcess function(above costate) to execute before
				// checking for keypress.
				waitfor(DelayMs(10));

				// Check if a key has been pressed
				waitfor ( key = keyGet() );	//	Wait for Keypress

				// Set flag to send respond back to controller that
				// sent the message.
				send_response = TRUE;
			}
		}

		costate
		{
			if(send_response)
			{
				glBlankScreen();
				TextGotoXY(&wholewindow, 0, 0 );
				TextPrintf(&wholewindow, "Press the key that \nis setup to send \nyour response...");
				waitfor ( key = keyGet() );	//	Wait for Keypress

				// Flash Leds to indicate that a key has been pressed
				flashled(key - '0');

				// Only handle the keypress if it corresponds to a message and if
				// a response is currently needed
				if (key != IGNORE)
				{
					SendMessage(&tcpSock, (key - '0'), messages);
					send_response = FALSE;
				}
			}
		}
	}

}
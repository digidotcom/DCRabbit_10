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
	tcp_send.c
 	(Adapted from Intellicom's tcp_send.c)

 	This sample program is used with controller boards
 	equipped with ethernet, LCD and keypad.  A 122x32 pixel
 	display and 1x7 keypad module with LED's are assumed.

   NOTE: Not currently supported on RCM4xxx modules.

	This program and tcp_respond.c are to be executed on two different
	controller boards so that the two boards communicate with each
	other using a crossover cable.
	In the absence of a second board, pcrespond.exe can be
	used on the PC console side at the command prompt. This executable
	and source code is located in \samples\lcd_keypad\windows directory.

	When a key on the keypad is pressed, a message associated with
	that key	is sent to a specified destination address and port.
	The destination then responds to that message.  The response is
	displayed to the LCD.

	Note: Currently only LEFT and UP scroll keys are setup to
	      cause a message to be sent.

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


/*
 * 	Have TCP install text error messages when trouble occurs.  If not
 * 	defined, then TcpSocket.err_msg is not used at all.
 * 	TcpSocket.err_msg == NULL if none.
 */
#define _SOCKET_MESSAGES

/*
 * 	How many seconds to wait for active connection.
 */
#define  CONNECT_TIMEOUT	10

/*
 * Also note that you will need to set up the messages in the main()
 * function.
 */

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

// The structure that holds a message and the destination
typedef struct {
	char *text;
	char *addr;
	word port;
} Message;

#define IGNORE 0x20


/*
 * General use Milli-sec delay function
 */
nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + (unsigned long)delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}

/*
 * Send the message specified by the key value.  Note that the message
 * number is actually one less than the key number (since key number 0
 * cannot be used).
 * 	RETURN: 	0 = good, -1 = error.
 */
int SendMessage(unsigned key, Message messageArray[])
{
	auto tcp_Socket	tcpSock;
	auto longword		remIP;
	auto int 			numBytes;
	auto char 			buffer[500];
	auto longword 		timeout;
	auto char *			p;
	auto int 			result;

	/*  glBlankScreen() doesn't change cursor position, so must do both. */
	glBlankScreen();
	TextGotoXY(&wholewindow, 0, 0 );

	remIP = inet_addr(messageArray[key].addr);
	tcp_open(&tcpSock, 0, remIP, messageArray[key].port, NULL);

	/* Wait for connection -- 10 seconds. */
	timeout = SEC_TIMER + CONNECT_TIMEOUT;
	while( !tcp_established(&tcpSock) )
	{
		if( (long)(SEC_TIMER - timeout) >= 0 )
		{
			TextPrintf(&wholewindow, "ERROR: connect: timeout");
			return -1;
		}
		if( ! tcp_tick( (sock_type *) & tcpSock ) )
		{
			TextPrintf(&wholewindow, "ERROR: connect\n");
			return -1;
		}
	}

	/*  Write the message text, including the trailing NULL byte. */
	p = messageArray[key].text;
	for( numBytes = strlen(p)+1 ; numBytes > 0 ; )
	{
		result = sock_fastwrite( &tcpSock, p, numBytes );
		if( result < 0 || ! tcp_tick( (sock_type *) &tcpSock ) )
		{
			TextPrintf(&wholewindow,  "ERROR: write\n");
			return -1;
		}
		numBytes -= result;
		p += result;
	}


	TextPrintf(&wholewindow, "Message sent.......\nwaiting for response");

	/*  Wait for some data to appear, or an error... */
	while((numBytes = sock_fastread(&tcpSock, buffer, sizeof(buffer))) == 0)
	{
		if( ! tcp_tick( &tcpSock ) || numBytes == -1)
		{
			TextGotoXY(&wholewindow, 0, 0 );
			TextPrintf(&wholewindow, "ERROR: read\n");
			return -1;
		}
	}

	// Display response message
	glBlankScreen();
	buffer[numBytes] = '\0';
	TextGotoXY(&wholewindow, 0, 0 );
	TextPrintf(&wholewindow, "%s", buffer);
	sock_close(&tcpSock);

	// Delay so that the response message can be read
	msDelay(2000);

	// Wait until the socket is closed
	while (tcp_tick(&tcpSock) != 0 );
	return 0;
}

void flashled(int led)
{
	switch (led)
	{
		case 0:  led = 0; break;
		case 1:  led = 1; break;
		case 2:  led = 2; break;
		case 3:  led = 3; break;
		default: led = 6; break;
	}
	dispLedOut(led,1);
	msDelay(400);
	dispLedOut(led,0);
	msDelay(400);
}

void initsystem(void)
{

	brdInit();
	dispInit();
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);
	TextWindowFrame(&wholewindow, &fi6x8, 0, 0, LCD_XS, LCD_YS);
	glBackLight(1);
}

void main(void)
{
	Message messageArray[12];
	unsigned int key;
	int i;

	// Initialize the message array
	for (i = 0; i < (sizeof(messageArray)/sizeof(Message)); i++)
	{
		messageArray[i].text = NULL;
	}

	// Define messages here--note that you only need define the messages you
	// will actually use.  The IP addresses should NOT be MY_IP_ADDRESS !
	messageArray[0].text = "This is test 1.";
	messageArray[0].addr = "10.10.6.176";			/* IP or hostname of destination */
	messageArray[0].port = 4040;

	messageArray[1].text = "This is test 2.";
	messageArray[1].addr = "10.10.6.176";			/* IP or hostname of destination */
	messageArray[1].port = 4040;

	initsystem();
	sock_init();

	// Configure keys on the keypad, in order from left to right.
	for (i = 0; i < 7; i++)
	{
		// Only enable a key if there is a corresponding message
		if (messageArray[i].text != NULL)
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
			keyProcess();
			waitfor (DelayMs(10));
		}

		costate
		{
			TextGotoXY(&wholewindow, 0, 0 );
			TextPrintf(&wholewindow, "Press the KEY that\ncorresponds to the\nmessage that you\nwant to send...");
			waitfor ( key = keyGet() );	//	Wait for Keypress

			// Flash LEDs to indicate that a key was pressed
			flashled(key-'0');

			if (key != IGNORE)
			{
				if( SendMessage((key-'0'), messageArray) < 0 )
				{
					/* If error message, display for a moment, then clear. */
					waitfor( DelayMs(3000) );
					glBlankScreen();
				}
			}
		}
	}

}
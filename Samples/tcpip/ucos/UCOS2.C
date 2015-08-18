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
        Samples\tcpip\UCOS\ucos2.c

        A sample data-gathering app, that combines a task reading from the
        serial port with another task to report this data to the user over the
        network, with associated local messaging and processing.

*******************************************************************************/
#class auto

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

/* Serial port configuration */

/*
 * The Speed at which to run the serial port
 */
#define SERIAL_PORT_SPEED 19200L

/*
 *		Serial port Settings
 *		Serial PORT 1=A, 2=B, 3=C, 4=D
 * 	The size of the read/write buffers in the serial port driver.
 * 	You probably don't have to change this
 *
 */

#define SERIAL_PORT	2
#define INBUFSIZE		1023
#define OUTBUFSIZE 	31

/* MCOS configuration */
#define OS_MAX_TASKS           2  		// Maximum number of tasks system can create (less stat and idle tasks)
#define OS_SEM_EN					 1			// Enable semaphores
#define OS_SEM_POST_EN			 1       // Enable old style post to semaphore
#define OS_MAX_EVENTS			 7			// MAX_TCP_SOCKET_BUFFERS + 2 + 1 (1 semaphore is used in this app)
#define STACK_CNT_256			 2			// LED update task + main()
#define STACK_CNT_2K         	 1			// TCP/IP needs a 2K stack


/********************************
 * End of configuration section *
 ********************************/

/*
 * 	Select serial port to use.
 */

#if (SERIAL_PORT==1)

	#define serZopen  	serAopen
	#define serZread  	serAread
	#define serZgetc  	serAgetc
	#define serZrdUsed  	serArdUsed
	#define serZwrite 	serAwrite
	#define serZclose 	serAclose
	#define serZwrFlush	serAwrFlush
	#define serZrdFlush	serArdFlush
	#define serZwrFree	serAwrFree

	#define AINBUFSIZE	INBUFSIZE
	#define AOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==2)

	#define serZopen  	serBopen
	#define serZread  	serBread
	#define serZgetc  	serBgetc
	#define serZrdUsed  	serBrdUsed
	#define serZwrite 	serBwrite
	#define serZclose 	serBclose
	#define serZwrFlush	serBwrFlush
	#define serZrdFlush	serBrdFlush
	#define serZwrFree	serBwrFree

	#define BINBUFSIZE	INBUFSIZE
	#define BOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==3)

	#define serZopen  	serCopen
	#define serZread  	serCread
	#define serZgetc  	serCgetc
	#define serZrdUsed  	serCrdUsed
	#define serZwrite 	serCwrite
	#define serZclose 	serCclose
	#define serZwrFlush	serCwrFlush
	#define serZrdFlush	serCrdFlush
	#define serZwrFree	serCwrFree

	#define CINBUFSIZE	INBUFSIZE
	#define COUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==4)

	#define serZopen  	serDopen
	#define serZread  	serDread
	#define serZgetc  	serDgetc
	#define serZrdUsed  	serDrdUsed
	#define serZwrite 	serDwrite
	#define serZclose 	serDclose
	#define serZwrFlush	serDwrFlush
	#define serZrdFlush	serDrdFlush
	#define serZwrFree	serDwrFree

	#define DINBUFSIZE	INBUFSIZE
	#define DOUTBUFSIZE	OUTBUFSIZE

#else
	#error "Unknown SERIAL_PORT value!"
	/* R3000 supports serial ports E and F ... */
#endif


///////////////////////////////////////////////////////////////////////
/*
 * IMPORTANT NOTE!
 *
 * It is important to note that the TCP/IP task REQUIRES a 2K stack!
 * This is because the function resolve() (a DNS lookup) creates a
 * udp socket on the stack. If DNS lookups are NOT needed, and
 * resolve() is never called, it is possible to run the TCP/IP stack
 * in only a 1K stack.
 *
 * Also note, all TCP/IP stack functions should be call from ONE (1)
 * thread only, or data corruption and/or system instability may result!
 */

#memmap xmem
#use "ucos2.lib"
#use "dcrtcp.lib"

/* buffer for passing messages between the two tasks */
#define MSG_BUF_SIZE		256
char	message_buffer[MSG_BUF_SIZE];
int	base, boundary;

/* a semaphore for locking the above buffer */
OS_EVENT	*Semaphore;

/* buffer and count used by the transmitter */
char	send_buf[INBUFSIZE + 1], temp_buf[INBUFSIZE + 1];
int	offset;

/* our outgoing socket */
tcp_Socket s;

/*
 * The task that reads from the serial port, and stores the data in the
 * message_buffer. Ideally, most of this (reading from the serial port) should
 * be in the serial ISR, as we delay an OS level tick each time we poll for
 * new data. This is a rather serious ammount of latency between polls, and
 * could easily lead to data loss.
 */
void reader_task(void *ptr)
{
	static INT8U	error;
	auto char *	p;
	auto int 	count, temp;

	/* initilization part */
	serZopen(SERIAL_PORT_SPEED);

	for(;;) {
		count = serZread(temp_buf, INBUFSIZE, 0);
		if(count) {
			OSSemPend(Semaphore, 0, &error);

			p = temp_buf;
			while(count) {
				count--;

				temp = base + 1;
				if(temp == MSG_BUF_SIZE)
					temp = 0;
				if(temp == boundary) {
					/* no room; drop the character on the floor */
					continue;
				}

				message_buffer[base] = *p;
				p++;
				base = temp;
			}

			OSSemPost(Semaphore);

		} else {
			/* no data read - yield */
			OSTimeDly(1);
		}
	}
}

void process_data(char c)
{
	/* do extra processing on c here - nothing is done for now */

	/* queue the data for transmit */
	send_buf[offset] = c;
	offset++;
}

void send_data(void)
{
	auto int cnt;

#GLOBAL_INIT
	{
		offset = 0;
	}

	/* send the buffer */
	cnt = 0;
	while(cnt < offset) {
		cnt += sock_fastwrite(&s, send_buf + cnt, offset - cnt);
		tcp_tick((sock_type *)&s);
	}

	/* reset */
	offset = 0;
}

void writer_task(void *ptr)
{
	static INT8U	error;
	auto int 	temp;

	/*
	 * Listen on a socket for a connection to be made.
	 *
	 * NOTE - this should really be making a connection to a
	 * custom client program in a more active manner, but for
	 * purposes of this sample program, listening should be
	 * sufficient, and does not require a sample program to
	 * be compiled on another machine, as it only requires
	 * 'telnet'.
	 */
	tcp_listen(&s, 23, 0, 0, NULL, 0);

	/* wait for a connection to be made */
	while(!sock_established((sock_type *)&s))
		tcp_tick((sock_type *)&s);

	for(;;) {
		tcp_tick((sock_type *)&s);
		OSSemPend(Semaphore, 0, &error);
		while(base != boundary) {
			/* while unprocessed data exists... */

			temp = boundary + 1;
			if(temp == MSG_BUF_SIZE)
				temp = 0;

			/* as we are holding the semaphore at the moment, this should be fast */
			process_data(message_buffer[boundary]);

			boundary = temp;
		}
		OSSemPost(Semaphore);

		send_data();
	}
}

void main()
{
	auto INT8U error;

	OSInit();

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	/* init our buffer */
	memset(message_buffer, 0, MSG_BUF_SIZE);
	base = boundary = 0;

	/* create our semaphore */
	Semaphore = OSSemCreate(1);

	/* Build the two tasks */
	error = OSTaskCreate(reader_task, 	NULL, 256, 0);
	error = OSTaskCreate(writer_task,	NULL, 2048, 1); // does TCP/IP, and therefore needs the 2k stack

	/* start it all running */
	OSStart();
}
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

        Samples\tcpip\serialexa.c

        This program directs all of the data from a serial port to a
        TCP port and vice versa.

        This is configurable at compile time.  Use "tcp_config.lib" to
        change the IP address, netmask, gateway and name server IP.
        In this file, set TIME_ZONE, and vs_info structures to match
        your application.

        If you use TELNET to connect here, it will try to negotiate its
        options.  This program doesn't handle those.

        This program was initially built from the state.c example program.

        JJB 04/27/2001: Created
        JJB 06/06/2001: Added a tcp_tick(NULL) to main to insure arp
                        requests are handled while we are waiting for
                        a connection.
        JJB 06/08/2001: vs_state->open_timeout was not initialized.
                        this caused the program not to respond the
                        first time it was compiled.  open_timeout
                        does not apply when dealing with a passive
                        socket.
        JJB 07/29/2002: Optimized the handling of characters when there
        						is no character gap.
        bjw 08/23/2002: Updated for "tcp_config.lib". Use SERIAL_PORT to
        					   select which one to use.

*******************************************************************************/

#class auto

//#define USE_STDIO
#define VERBOSE


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
 * 	Tweak the ethernet settings...
 */
#define TCP_BUF_SIZE 14000
#define MAX_TCP_SOCKET_BUFFERS 1 // We only need one socket for this example
#define VS_MAXOFFSET 512


/*
 *  The TIMEZONE compiler setting gives the number of hours from
 *  local time to Greenwich Mean Time (GMT).  For pacific standard
 *  time this is -8.  Note:  for the time to be correct it must be set
 *  with tm_rd which is documented in the Dynamic C user manual.
 *
 */

#define TIMEZONE        -8

/*
 *		Serial port Settings
 *		Serial PORT can be 1=A, 2=B, 3=C, 4=D, 5=E, 6=F
 *
 */

#define SERIAL_PORT	4
#define BAUD_RATE		19200L
#define INBUFSIZE		1023
#define OUTBUFSIZE 	1023



/********************************
 * End of configuration section *
 ********************************/

/*
 *  memmap forces the code into xmem.  Since the typical stack is larger
 *  than the root memory, this is commonly a desirable setting.  Another
 *  option is to do #memmap anymem 8192 which will force code to xmem when
 *  the compiler notices that it is generating withing 8192 bytes of the
 *  end.
 *
 *  #use the Dynamic C TCP/IP stack library and the HTTP application library
 *
 */

#memmap xmem
#use "dcrtcp.lib"

/*
 * 	Select serial port to use.
 */

#if (SERIAL_PORT==1)

	#define serYopen  	serAopen
	#define serYread  	serAread
	#define serYgetc  	serAgetc
	#define serYrdUsed  	serArdUsed
	#define serYwrite 	serAwrite
	#define serYclose 	serAclose
	#define serYwrFlush	serAwrFlush
	#define serYrdFlush	serArdFlush
	#define serYwrFree	serAwrFree

	#define AINBUFSIZE	INBUFSIZE
	#define AOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==2)

	#define serYopen  	serBopen
	#define serYread  	serBread
	#define serYgetc  	serBgetc
	#define serYrdUsed  	serBrdUsed
	#define serYwrite 	serBwrite
	#define serYclose 	serBclose
	#define serYwrFlush	serBwrFlush
	#define serYrdFlush	serBrdFlush
	#define serYwrFree	serBwrFree

	#define BINBUFSIZE	INBUFSIZE
	#define BOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==3)

	#define serYopen  	serCopen
	#define serYread  	serCread
	#define serYgetc  	serCgetc
	#define serYrdUsed  	serCrdUsed
	#define serYwrite 	serCwrite
	#define serYclose 	serCclose
	#define serYwrFlush	serCwrFlush
	#define serYrdFlush	serCrdFlush
	#define serYwrFree	serCwrFree

	#define CINBUFSIZE	INBUFSIZE
	#define COUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==4)

	#define serYopen  	serDopen
	#define serYread  	serDread
	#define serYgetc  	serDgetc
	#define serYrdUsed  	serDrdUsed
	#define serYwrite 	serDwrite
	#define serYclose 	serDclose
	#define serYwrFlush	serDwrFlush
	#define serYrdFlush	serDrdFlush
	#define serYwrFree	serDwrFree

	#define DINBUFSIZE	INBUFSIZE
	#define DOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==5)

	#define serYopen  	serEopen
	#define serYread  	serEread
	#define serYgetc  	serEgetc
	#define serYrdUsed  	serErdUsed
	#define serYwrite 	serEwrite
	#define serYclose 	serEclose
	#define serYwrFlush	serEwrFlush
	#define serYrdFlush	serErdFlush
	#define serYwrFree	serEwrFree

	#define EINBUFSIZE	INBUFSIZE
	#define EOUTBUFSIZE	OUTBUFSIZE

#elif (SERIAL_PORT==6)

	#define serYopen  	serFopen
	#define serYread  	serFread
	#define serYgetc  	serFgetc
	#define serYrdUsed  	serFrdUsed
	#define serYwrite 	serFwrite
	#define serYclose 	serFclose
	#define serYwrFlush	serFwrFlush
	#define serYrdFlush	serFrdFlush
	#define serYwrFree	serFwrFree

	#define FINBUFSIZE	INBUFSIZE
	#define FOUTBUFSIZE	OUTBUFSIZE

#else
	#error "Unknown SERIAL_PORT value!"
#endif

/*
 *		Connection States
 *
 */

#define VS_INIT			0		/* closed or never opened */
#define VS_LISTEN			1		/* waiting for a connection */
#define VS_OPENING		2		/* wait while we open a connection */
#define VS_OPEN			3		/* we have a connection */
#define VS_WAITCLOSE		4		/* wait for the connection to be torn down */

/*
 *		VsState contains all of the information necessary
 *		to manage the connection.  The state is one of the above
 *		connection states.
 *
 */

#define VS_MODEOFF			0
#define VS_MODEACTIVE		1
#define VS_MODEPASSIVE		2

typedef struct
{
	int state;
	tcp_Socket socket;

	int	offset;
	long	last_character;
	char	buffer[VS_MAXOFFSET];

	int	open_timeout;
} VsState;

/*
 *		VsState holds the configuration information.  The
 *		factory_defaults file is copied to vs_info when
 *		the RESET command is issued.
 *
 */

typedef struct
{
	int	port;					// port to listen on when in VS_MODEPASSIVE
	int	timeout;				// intercharacter delay before flushing characters (ms)
	long	baud;					// serial port baud rates

	int	mode;					// VS_MODEACTIVE, VS_MODEPASSIVE, VS_MODEOFF
	long	dest_ip;				// ip address to call when in VS_MODEACTIVE
	int	dest_port;			// port to call when in VS_MODEACTIVE

	int	binary;				// TCP_MODE_BINARY or TCP_MODE_ASCII

	int	open_timeout;		// interopen delay when unsuccessful open socket (ms)

} VsInfo;

VsInfo vs_info;
VsState vs_state;

const VsInfo factory_defaults =
{
	23,							// port to listen on when in VS_MODEPASSIVE
	0,								// intercharacter delay before flushing characters (ms)
	BAUD_RATE,					// serial port baud rates

	VS_MODEPASSIVE,			// VS_MODEACTIVE, VS_MODEPASSIVE, VS_MODEOFF
	0,								// ip address to call when in VS_MODEACTIVE
	0,								// port to call when in VS_MODEACTIVE

	TCP_MODE_BINARY,			// TCP_MODE_BINARY or TCP_MODE_ASCII

	1000							// interopen delay when unsuccessful open socket (ms)
};

///////////////////////////////////////////////////////////////////////

/*
 *		This function sets the VsState.state to the initial
 *		state so it will be initialized on the first pass
 *		through the handler.
 *
 */

void vs_init(VsState* vs_state)
{
	vs_state->state=VS_INIT;
	vs_state->offset=0;
	vs_state->last_character=MS_TIMER;
	vs_state->open_timeout=vs_info.open_timeout;

	memcpy(&vs_info,&factory_defaults,sizeof(vs_info));
}

/*
 *		This function drives the state machine.
 *
 *		It can be in 3 main states.  VS_INIT is the initial
 *		state. It will wait for the interopen time if it is
 *		an active connection and then open the connection.
 *		If it is a passive connection, VS_INIT will listen
 *		for a new connection.
 *
 *		The VS_LISTEN and VS_OPENING state wait for a
 *		connection to be established and change the
 *		socket mode when it is.
 *
 *		When the state is VS_OPEN the state machine passes
 *		anything from the serial port to the tcp port and
 *		from the tcp port to the serial port.  Notice the
 *		code monitoring the vs_state.timeout.  When a timeout
 *		is specified in the vs_info structure the VS_OPEN
 *		state will wait until it either sees an inter character
 *		gap in vs_info.timeout ms or until the internal buffer
 *		is full.  This has two effects.  First it is requires
 *		less overhead on the network.  Second it may increase
 *		the burstiness of the data.  Set vs_info.timeout to
 *		zero if you don't want that behavior.
 *
 *		When the state machine notices the socket has been
 *		closed for some reason it returns the state back to
 *		VS_INIT.
 *
 */

void vs_handler(VsState* state)
{
	auto tcp_Socket* socket;
	auto int ch, bytes_written;
	auto int bytes_to_write;

	if(vs_info.mode==VS_MODEOFF)
		return;

	socket=&state->socket;

	/*
	 *		was the connection reset?
	 */
	if(state->state!=VS_INIT && tcp_tick(socket)==0) {
#ifdef VERBOSE
		printf("Connection closed\n");
#endif
		state->state=VS_INIT;
		state->open_timeout=vs_info.open_timeout;
	}

	switch(state->state)
	{
		case VS_INIT:
			/*
			 *		passive open on the socket port
			 */

			if(state->open_timeout && vs_info.mode == VS_MODEACTIVE) {
				costate {
					waitfor(DelayMs(state->open_timeout));
					state->open_timeout=0;
				}

				if(state->open_timeout)
					break;
			}

			serYopen(vs_info.baud);

			if(vs_info.mode == VS_MODEPASSIVE) {
				if (tcp_listen(socket,vs_info.port,0,0,NULL,0) != 0) {
					state->state=VS_LISTEN;
#ifdef VERBOSE
					printf("\nListening on socket\n");
#endif
				}
				else {
					printf("\nError listening on socket!\n");
				}
			} else if(vs_info.mode == VS_MODEACTIVE) {
				if (tcp_open(socket,0,vs_info.dest_ip,vs_info.dest_port,NULL) != 0) {
					state->state=VS_OPENING;
#ifdef VERBOSE
					printf("\nOpening socket\n");
#endif
				}
				else {
					printf("\nError opening socket!\n");
				}
			}
			break;

		case VS_LISTEN:
		case VS_OPENING:
			/*
			 *		wait for a connection
			 */

			if(sock_established(socket) || sock_bytesready(socket) >= 0) {
				state->state=VS_OPEN;
				sock_mode(socket,vs_info.binary);

#ifdef VERBOSE
				printf("New Connection\n");
#endif
			}
			break;

		case VS_OPEN:
			if(vs_info.timeout!=0 &&
				state->offset &&
				(long) ((state->last_character+vs_info.timeout) - MS_TIMER) < 0) {
				bytes_written=sock_fastwrite(socket,state->buffer,state->offset);
				if (bytes_written < 0) {
					state->state = VS_WAITCLOSE;
					sock_close(socket);
#ifdef VERBOSE
					printf("Connection closed\n");
#endif
					break;
				}

				/*
				 *		Hmmm... We weren't able to write all the bytes out.
				 *		Since we don't want to loose any characters, we will
				 *		shift everything over and hopefully write it soon.
				 *
				 */

				if(bytes_written!=state->offset) {
					memcpy(state->buffer,state->buffer+bytes_written,state->offset-bytes_written);
					state->offset = bytes_written;
					break;
				} else
					state->offset = 0;
			}

			/*
			 *		process any characters.
			 */

			bytes_to_write=sock_bytesready(socket);
			if(bytes_to_write>serYwrFree())
				bytes_to_write=serYwrFree();

			if(bytes_to_write>(int)sizeof(state->buffer))
				bytes_to_write=sizeof(state->buffer);

			if(bytes_to_write>0) {
				sock_read(socket,state->buffer,bytes_to_write);
				serYwrite(state->buffer,bytes_to_write);
			}

			/*
			 *		If we aren't worried about interpacket delay
			 *		just send the characters if there is room in
			 *		the buffer.
			 *
			 */

			if(vs_info.timeout==0) {
				bytes_to_write=serYrdUsed();

				if(bytes_to_write>sock_tbleft(socket))
					bytes_to_write=sock_tbleft(socket);

				if(bytes_to_write>(int)sizeof(state->buffer))
					bytes_to_write=sizeof(state->buffer);

				if(bytes_to_write>0) {
					serYread(state->buffer,bytes_to_write,0);
					sock_write(socket,state->buffer,bytes_to_write);
				}
			} else {
				while(state->offset<VS_MAXOFFSET && (ch=serYgetc())!=-1) {
#ifdef USE_STDIO
					printf("%c",ch);
#endif
					state->buffer[state->offset++]=ch;
					state->last_character=MS_TIMER;
				}
			}

			/*
			 *		We should immediately flush characters if the buffer
			 *		is full.
			 *
			 */

			if(state->offset==VS_MAXOFFSET) {
				bytes_written=sock_fastwrite(socket,state->buffer,state->offset);
				if (bytes_written < 0) {
					state->state = VS_WAITCLOSE;
					sock_close(socket);
#ifdef VERBOSE
					printf("Connection closed\n");
#endif
					break;
				}

				/*
				 *		Hmmm... We weren't able to write all the bytes out.
				 *		Since we don't want to loose any characters, we will
				 *		shift everything over and hopefully write it soon.
				 *
				 */

				if(bytes_written!=state->offset) {
					memcpy(state->buffer,state->buffer+bytes_written,state->offset-bytes_written);
					state->offset = bytes_written;
				} else
					state->offset = 0;
			}

			break;

		case VS_WAITCLOSE:
			break;

		default:
			/*
			 *		how did we get here?  programming error?
			 */

			state->state=VS_INIT;
			break;
	}
}

/*
 *		main() initializes DCRTCP by calling sock_init and the
 *		VsState by vs_init().  It then calls vs_handler() repeatitively
 *		passing the VsState.  If you wanted to extend this to have
 *		multiple connections, add more VsStates with their own calls
 *		to the vs_handler.
 *
 */

void main()
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	vs_init(&vs_state);
	serYopen(vs_info.baud);

   while (1) {
   	tcp_tick(NULL);
		vs_handler(&vs_state);
	}
}
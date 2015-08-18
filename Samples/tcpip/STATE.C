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
        Samples\tcpip\state.c

        A basic state machine style server. This is a demonstration
        of a common way to implement servers, by implementing a
        basic web server.

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


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use dcrtcp.lib

#define MY_PORT 			80

/*
 *		Connection States
 *
 */

#define MY_INIT			0		/* closed or never opened */
#define MY_LISTEN			1		/* waiting for a connection */
#define MY_GETREQ			2		/* get the http request line */
#define MY_GETHEAD		3		/* get any http headers */
#define MY_SENDRSP		4		/* send the canned page */
#define MY_WAITCLOSE		5		/* wait for the connection to be torn down */

/*
 *		MyState contains all of the information necessary
 *		to manage the connection.  The state is one of the above
 *		connection states.  The offset and length are used to
 *		send the my_response.  The offset is the offset within
 *		the "page" and the length is the offset of the end of the
 *		"page."  The socket structure is used internally by DCRTCP
 *		to manage the state and buffers.  The buffer is used to
 *		temporarily hold the request or header lines.
 *
 */

typedef struct
{
	int state;
	int offset;
	int length;
	tcp_Socket socket;
	char buffer[256];
} MyState;

/*
 *		This function sets the MyState.state to the initial
 *		state so it will be initialized on the first pass
 *		through the handler.
 *
 */

void my_init(MyState* my_state)
{
	my_state->state=MY_INIT;
}

/*
 *		my_response is the "page" that is sent back to any
 *		query.
 *
 */

const char my_response[] =
	"HTTP/1.0 200 OK\r\n" \
	"Content-Type: text/html\r\n\r\n" \
	"<HTML>" \
	"<HEAD><TITLE>test</TITLE></HEAD>" \
	"<BODY><H1>test</H1></BODY>" \
	"</HTML>";

/*
 *		my_handler is a function that is passed a MyState
 *		structure and manages the connection.  The first time
 *		the function is called on a MyState the MyState->state
 *		should be MY_INIT.  The hander does a passive open on
 *		the socket and waits for a connection.  When a
 *		connection is established it reads the http request
 *		and header lines waiting for \r\n\r\n.  At this point
 *		the handler sends the my_response and closes the
 *		connection.  After the connection has been confirmed
 *		closed the MyState.state is set back to MY_INIT.
 *
 *		It is important to notice how sock_stringready works.
 *		The sock_stringready function will only return
 *		when there is a \r\n terminated string in the buffer or
 *		the buffer has been completely filled.  This allows us
 *		to call sock_gets which would normally block without
 *		worrying that it will actually block.
 *
 *		The second thing to notice is the use of sock_fastwrite
 *		in MY_SENDRSP.  This function only writes as much data
 *		as there is buffer space at the moment it is called.
 *		This method using the offset moving towards the length
 *		can be used to make sure all of the data is sent.
 *
 */

void my_handler(MyState* state)
{
	auto tcp_Socket* socket;
	auto int bytes_written;
	auto word delims;
	auto int rc;

	socket=&state->socket;

	/*
	 *		was the connection reset?
	 */
	if(state->state!=MY_INIT && tcp_tick(socket)==0)
		state->state=MY_INIT;

	switch(state->state)
	{
		case MY_INIT:
			/*
			 *		passive open on the socket port:  MY_PORT
			 */
			tcp_listen(socket,MY_PORT,0,0,NULL,0);
			state->state=MY_LISTEN;
			printf("\nInitializing socket\n");
			break;

		case MY_LISTEN:
			/*
			 *		wait for a connection
			 */
			if(sock_established(socket)) {
				state->state=MY_GETREQ;
				printf("New Connection\n");
			}
			break;

		case MY_GETREQ:
			/*
			 *		process the request line.
			 */
			delims = DELIM_CRLF;
			rc = sock_stringready(socket,&delims,sizeof(state->buffer));
			if (rc > 0) {
				sock_gets(socket,state->buffer,sizeof(state->buffer));
				state->state=MY_GETHEAD;
				printf("%s\nHeaders\n",state->buffer);
			}
			if (rc < 0) {
				printf("Unexpected error processing request\n");
				sock_close(socket);
				state->state=MY_WAITCLOSE;
			}
			break;

		case MY_GETHEAD:
			/*
			 *		process any headers
			 */
			delims = DELIM_CRLF;
			rc = sock_stringready(socket,&delims,sizeof(state->buffer));
			if(rc > 0) {
				sock_gets(socket,state->buffer,sizeof(state->buffer));
				printf("%s\n",state->buffer);

				/*
				 *		When there is an empty line we know that
				 *		the browser is done sending headers.
				 */
				if(state->buffer[0]==0) {
					state->state=MY_SENDRSP;
					state->offset=0;
					state->length=strlen(my_response);
					printf("Sending Response:\n%s\n",my_response);
				}
			}
			if (rc < 0) {
				printf("Unexpected error processing headers\n");
				sock_close(socket);
				state->state=MY_WAITCLOSE;
			}
			break;

		case MY_SENDRSP:
			bytes_written=sock_fastwrite(socket,my_response+state->offset,
				state->length-state->offset);

			state->offset+=bytes_written;

			/*
			 *		if there is an error or we have written out the
			 *		entire my_response.
			 *
			 */
			if(bytes_written<0 ||
				state->offset>=state->length) {
				sock_close(socket);
				state->state=MY_WAITCLOSE;
				printf("Closing Socket\n");
			}
			break;

		case MY_WAITCLOSE:
			break;

	}
}

/*
 *		main() initializes DCRTCP by calling sock_init and the
 *		MyState by my_init().  It then calls my_handler() repeatitively
 *		passing the MyState.  If you wanted to extend this to have
 *		multiple connections, add more MyStates with their own calls
 *		to the my_handler().
 *
 */

void main()
{
	MyState my_state;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	/*
	 *		The reserve port function enables SYN queuing.  This
	 *		allows connections to be held pending if there is no
	 *		socket to handle the port.
	 *
	 */

	tcp_reserveport(MY_PORT);

	my_init(&my_state);

	for(;;) {
		my_handler(&my_state);
	}
}
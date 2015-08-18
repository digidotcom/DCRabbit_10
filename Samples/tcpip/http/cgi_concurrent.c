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
        cgi_concurrent.c

        This program demonstrates some more complex CGI programming.  In
        particular, it demonstrates how to use the user data area within each
        HTTP state structure, as well as how to share access to a resource.

        To use this program, browse to the Rabbit's web server in your web
        browser.  You will be prompted to enter two numbers that will be added
        by a trained parrot.  If a trained parrot is not available, then a
        software engineer will do.  After submitting the numbers, a prompt is
        generated to Dynamic C's stdio window that asks for the sum of the
        numbers.  Enter the sum at the prompt, and the sum will then be
        displayed in the web browser.

        One useful thing to try with this sample is submitting from multiple
        pages at once.  The code is written such that only one prompt is
        displayed at the stdio window at once--after all, we would not want to
        confuse the poor parrot!  Essentially, only one CGI instance is allowed
        access to the stdio window at a time.

        Also note that the CGI will properly abort if the HTTP connection times
        out (which is set to 60 seconds in this sample--see below).  The stdio
        window is released at this point to be used by another CGI instance.

*******************************************************************************/

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
 * Define a number of concurrent web servers, along with the socket buffers
 * needed to support them.
 */
#define HTTP_MAXSERVERS 4
#define MAX_TCP_SOCKET_BUFFERS 4

/*
 * Increases the timeout for an HTTP connection to 60 seconds.  This will
 * give the trained parrot more time to enter the correct sum.
 */
#define HTTP_TIMEOUT 60

/********************************
 * End of configuration section *
 ********************************/

// Leave this uncommented to debug the parrot.  Note that this refers to
// verifying the parrot's calculation, not to removing lice from the parrot.
// Delousing is done by another process entirely.
#define PARROT_DEBUG

typedef struct {
	char buffer[10];		// Stores the received numbers in string format
   int buf_offset;		// Offset into the buffer
   int number1;			// Value of the first number in the sum
   int number2;			// Value of the second number in the sum
   int calculated_sum;	// The sum as calculated by this program
   int parrot_sum;		// The sum entered into the stdio window
} ParrotState;

// Defining this macro determines the size of the user data area that is
// allocated within each HTTP state structure.  This area is typically used
// in CGI programming, and can be used to store any information that is
// needed across calls of a CGI state machine.
#define HTTP_USERDATA_SIZE sizeof(ParrotState)

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

#ximport "samples/tcpip/http/pages/cgi_concurrent.html"     index_html

// This is the prototype for the CGI function that is used to process the
// numbers to be summed, to prompt on the stdio window for the sum, and to
// display the sum back to the web browser.
int add(HttpState *state);

// This associates file extensions with file types.  The default for / must
// be first.
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".cgi", MIMETYPE_HTML)
SSPEC_MIMETABLE_END

// This structure associates resource names with their locations in memory.
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/index.html", index_html),
	SSPEC_RESOURCE_FUNCTION("/add.cgi", add)
SSPEC_RESOURCETABLE_END

// This is essentially a semaphore for the use of the trained parrot and the
// stdio window.  The CGI function first tries to reserve this resource before
// prompting for the sum.  In this way, only one HTTP server is allowed access
// to the stdio window at a time.
int parrot_used;

// States for the CGI state machine
enum {
	ADD_GET_PARROT,	// Acquires the parrot/stdio semaphore
   ADD_GET_NUMBERS,	// Get the numbers in the POST submission, as well as
                     // prompt in the stdio window for the sum
   ADD_GET_SUM,		// Read the sum from the stdio window
	ADD_SEND_HEADER,	// Send the header info of the response to the browser
   ADD_SEND_CONTENT,	// Send the sum to the web browser
   ADD_FINISH			// Wait for all information to be send to the browser
};

// This is the main CGI function.  It is called whenever the browser requests
// "add.cgi"--in this case, the Rabbit web server calls this function repeatedly
// until it indicates that has completed.  A CGI function must return 0 when
// it needs to be called again, and 1 when it has completed.
//
// In particular, this CGI processes two numbers that are presented as part of
// a POST request from the browser.  These two numbers are then presented to the
// stdio window.  It will then read the sum from the stdio window and present
// it to the user via the web browser.
//
// Note that this CGI makes use of a user data area within the HTTP server's
// state structure.  This allows multiple CGI functions to be running at one
// time.  However, the stdio window is a shared resource, so this sample also
// shows how to properly share access to that resource.  Essentially, it
// serializes access to the stdio window.  To do this, it uses a variable as
// a semaphore, along with the abort_notify and cancel members of the HTTP
// state structure.
int add(HttpState *state)
{
	auto int retval;		// Used to store the return value from functions
   ParrotState *parrot;	// Points to the user state information within the HTTP
   							// state structure
   auto char ch;			// Stores a character for reading from the stdio window

	// Check if this CGI function is being called in a "cancel" condition.  This
   // means that the connection is being aborted for some reason.  The most
   // likely reason is that the HTTP connection has timed out.  The semaphore
   // for the shared resource must be freed in this case.
   if (state->cancel) {
   	printf("\nABORTING!  You're too slow, parrot!  No crackers for you!\n");
      parrot_used = 0;
      return 1;
   }

	// Get a pointer to the user data within the HTTP state structure.  This
   // information is unique to each HTTP server instance, and allows the user
   // to keep information on multiple concurrent CGI functions.
   parrot = http_getUserState(state);

	// This code is a catch-all that writes out any data to the HTTP server
   // socket that has not yet been written.
   if (state->length) {
		// buffer to write out
		if (state->offset < state->length) {
			state->offset += sock_fastwrite(&state->s,
					state->buffer + (int)state->offset,
					(int)state->length - (int)state->offset);
		}
      else {
			state->offset = 0;
			state->length = 0;
		}
      // Indicate that this CGI function should be called again.
      return 0;
   }

   // This is the state machine for this CGI function.  It drives the process
   // of reading the numbers to add, prompting for the sum via the stdio
   // window, reading the sum from the stdio window, and sending the result
   // back to the browser.  Note that the substate member of the HTTP state
   // structure is used to store the state for this state machine.  Both
   // substate and subsubstate are available for user code.
   switch (state->substate) {

	// This state is used to acquire the semaphore for the parrot/stdio window.
   // We also set up the abort_notify flag which tells the web server to
   // notify us if it is about to abort the connection (probably because of a
   // timeout).  This way, we can release the semaphore.
   case ADD_GET_PARROT:
      if (!parrot_used) {
         parrot_used = 1;
         state->abort_notify = 1;
         state->substate = ADD_GET_NUMBERS;
      }
      break;

   // This state reads the numbers to add from the web browser.  These numbers
   // have been submitted as part of a POST request.  After reading them,
   // then a prompt is displayed on the stdio window that requests the sum of
   // the numbers.
   case ADD_GET_NUMBERS:
      // state->s is the socket structure, and state->p is pointer
      // into the HTTP state buffer (initially pointing to the beginning
      // of the buffer).  Note that state->p was set up in the submit
      // CGI function.  Also note that we read up to the content_length,
      // or HTTP_MAXBUFFER, whichever is smaller.  Larger POSTs will be
      // truncated.
      retval = sock_aread(&state->s, state->p,
                          (state->content_length < HTTP_MAXBUFFER-1)?
                           (int)state->content_length:HTTP_MAXBUFFER-1);
      if (retval < 0) {
         // Error--just bail out
         printf("Socket error!\n");
         return 1;
      }
      else if (retval == 0) {
         // Have not received the entire POST yet--call again
         return 0;
      }

      // Using the subsubstate to keep track of how much data we have received
      state->subsubstate += retval;

      if (state->subsubstate >= state->content_length) {
         // NULL-terminate the content buffer
         state->buffer[(int)state->content_length] = '\0';

         // Scan the received POST information for the number1 and number2
         // values
         http_scanpost("number1", state->buffer, parrot->buffer, 10);
         parrot->number1 = atoi(parrot->buffer);
         http_scanpost("number2", state->buffer, parrot->buffer, 10);
         parrot->number2 = atoi(parrot->buffer);
         parrot->calculated_sum = parrot->number1 + parrot->number2;
      }
      // Present the prompt on the stdio window.
      printf("\nAttention Parrot! Add these two numbers...\n");
      printf("%d + %d = ", parrot->number1, parrot->number2);
      state->substate = ADD_GET_SUM;
      break;

	// This state monitors the stdio window for input.  This input is converted
   // into the sum, or an error message is displayed if invalid input is
   // entered.
   case ADD_GET_SUM:
      // Check if the trained parrot has pressed a key
      if (kbhit()) {
         ch = getchar();
         if ((ch == '\r') || (ch == '\n')) {
            // The parrot has pressed enter
            parrot->parrot_sum = atoi(parrot->buffer);
            // The parrot is now free for another sum.  We also no longer
            // need to be notified on abort to free the parrot.
            parrot_used = 0;
            state->abort_notify = 0;
            state->substate = ADD_SEND_HEADER;
         }
         else if ((ch >= '0') && (ch <= '9')) {
            // The parrot entered the next digit in the sum
            parrot->buffer[parrot->buf_offset] = ch;
            parrot->buf_offset++;
            printf("%c", ch);
         }
         else if ((ch == 'c') || (ch == 'C')) {
            // The parrot might be requesting a cracker
            printf("\nDon't even think about asking for a cracker until ");
            printf("you've completed this problem...\n");
            printf("%d + %d = ", parrot->number1, parrot->number2);
            parrot->buf_offset = 0;
         }
         else {
            // Who knows what the parrot is doing...
            printf("\nThat's not a digit!  0, 1, 2, 3, 4, 5, 6, 7, 8, or 9!\n");
            printf("%d + %d = ", parrot->number1, parrot->number2);
            parrot->buf_offset = 0;
         }
      }
      break;

	// Send the header information and the beginning of the web page to the web
   // browser.
   case ADD_SEND_HEADER:
      _f_strcpy(state->buffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
      _f_strcat(state->buffer, "<HTML><HEAD><TITLE>The parrot says...</TITLE></HEAD>");
      _f_strcat(state->buffer, "<BODY><H1>The parrot says...</H1>");
      state->length = strlen(state->buffer);
      state->offset = 0;
      state->substate = ADD_SEND_CONTENT;
      break;

	// Send the remainder of the web page to the web browser.  This includes the
   // sum that was entered on the stdio window.  If debugging mode is
   // enabled, then the correctness of the sum is indicated as well.
   case ADD_SEND_CONTENT:
      sprintf(state->buffer, "%d + %d = %d", parrot->number1,
              parrot->number2, parrot->parrot_sum);
#ifdef PARROT_DEBUG
      if (parrot->parrot_sum != parrot->calculated_sum) {
         _f_strcat(state->buffer, "<P>The parrot is WRONG!  Silly parrot...");
      }
      else {
         _f_strcat(state->buffer, "<P>The parrot is CORRECT!");
      }
#endif
      _f_strcat(state->buffer, "<P>Click <A HREF=\"/index.html\">here</A> to go back to the sums page");
      _f_strcat(state->buffer, "</BODY></HTML>");
      state->length = strlen(state->buffer);
      state->offset = 0;
      state->substate = ADD_FINISH;
      break;

	// This state finishes up the state machine.
   case ADD_FINISH:
      // Finished processing--returning 1 indicates that we are done
      return 1;
   }

   // By default, indicate that this CGI function needs to be called again.
	return 0;
}

void main(void)
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
	tcp_reserveport(80);

	// Initialize the semaphore for the shared resource
   parrot_used = 0;

   while (1) {
   	http_handler();
   }
}


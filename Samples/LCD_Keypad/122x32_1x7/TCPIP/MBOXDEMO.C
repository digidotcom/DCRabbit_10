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
   Samples\LCD_Keypad\122x32_1x7\TCPIP\mboxdemo.c
 	(Adapted from Intellicom's mboxdemo.c)

 	This sample program is used with controller boards
 	equipped with ethernet, LCD and keypad.  A 122x32 pixel
 	display and 1x7 keypad module with LED's are assumed.

   NOTE: Not currently supported on RCM4xxx modules.

   This program implements a web server that allows email messages to
   be entered that are then shown on the LCD display.  The keypad
   allows the user to scroll within messages, flip to other emails,
   mark messages as read, and delete emails.  When a new email
   arrives, an LED turns on, and turns back off once the message
   has been marked as read.  A log of all email actions is kept,
   which can be displayed in the web browser.  All current emails
   can also be read with the web browser.

	Instructions
	------------
	1. Make changes in the configuration section below to match
		your requirements.
	2. Compile and run this program.
	3. Through the created web page you can read, send or view
		messages.
	4. The controller display will indicate if you have mail and
		you can do the following by using the keypad to:

			View Next Message, press Left Arrow key
			Scroll Up, press Up Arrow key
			Scroll Down, press Down Arrow key
			View Previous Message, press Right Arrow key
			Read Message, press Minus key
			Delete Message, press Plus key
			Ignore Message, press Enter key

********************************************************************/
#class auto
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
 * TCP/IP modification - reduce TCP socket buffer
 * size, to allow more connections. This can be increased,
 * with increased performance, if the number of sockets
 * are reduced.  Note that this buffer size is split in
 * two for TCP sockets--1024 bytes for send and 1024 bytes
 * for receive.
 */
#define TCP_BUF_SIZE 2048

/*
 * Web server configuration
 */

/*
 * Define the number of HTTP servers and socket buffers.  Note
 * that we need one TCP socket for SMTP support. With
 * tcp_reserveport(), fewer HTTP servers are needed.
 */
#define HTTP_MAXSERVERS 2
#define MAX_TCP_SOCKET_BUFFERS 3

/*
 * Our web server as seen from the clients.
 * This should be the address that the clients (netscape/IE)
 * use to access your server. Usually, this is your IP address.
 * If you are behind a firewall, though, it might be a port on
 * the proxy, that will be forwarded to the controller board. The
 * commented out line is an example of such a situation.
 *
 *	//#define REDIRECTHOST	"myproxy.mydomain.com:8080"
 */
#define REDIRECTHOST		_PRIMARY_STATIC_IP

/*
 * The following values can customize the maximum number of emails,
 * the maximum sizes of buffers, etc.
 */
#define MAX_EMAILS 25				// Maximum number of emails
#define MAX_FROM_LEN 41				// Maximum length of From line
#define MAX_SUBJECT_LEN 41			// Maximum length of Subject line
#define MAX_BODY_LEN 1000			// Maximum length of Body
#define DISP_BUFFER_LINES 100		// Maximum number of lines in display buffer
#define DISP_ROWS 4					// Number of rows on the LCD
#define DISP_COLS 20					// Number of cols on the LCD
#define LOG_BUFFER_SIZE 1000		// Size of the log buffer


/********************************
 * End of configuration section *
 ********************************/

#define REDIRECTTO      "http://" REDIRECTHOST ""

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

#ximport "samples/icom/pages/main.html"		index_html
#ximport "samples/icom/pages/enter.html"		enter_html

/* the default mime type for files without an extension must be first */
SSPEC_MIMETABLE_START
   SSPEC_MIME_FUNC(".shtml", "text/html", shtml_handler),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif")
SSPEC_MIMETABLE_END

/*
 * The following structure is used to parse data returned from a FORM
 */
typedef struct {
	char *name;
	char *value;
	int len;
} FORMType;
FORMType FORMSpec[3];

/*
 * This structure is a node in a doubly linked list of emails
 */
struct EmailIndex {
	int prev;
	int next;
	int used;
	int read;
};
struct EmailIndex emailIndex[MAX_EMAILS];

/*
 * This structure keeps track of the head, tail, and current email in the
 * email list
 */
struct EmailList {
	int head;
	int tail;
	int ptr;
};
struct EmailList emailList;

int emailDispLine;	// Current line of email being displayed
int emailLastLine;	// Last line of email being displayed
int flag;

/*
 * This structure holds an email
 */
typedef struct {
	char from[MAX_FROM_LEN];
	char subject[MAX_SUBJECT_LEN];
	char body[MAX_BODY_LEN];
} Email;
Email email;
Email emailTemp;

// Points to the email buffer in xmem
long emailBuffer;

// Points to the display buffer in xmem
long dispBuffer;

// Points to the log buffer in xmem
long logBuffer;
// Points to the current location in the log buffer
int logPtr;

#define FLAG_DOWN 0
#define FLAG_UP   1

/*
 * Initialize the mailbox flag to down position
 */
void FlagInit(void)
{
	flag = FLAG_DOWN;
	dispLedOut(0,0);   //red led off
}

/*
 * Put the mailbox flag up, if it isn't already
 */
void FlagChange(int flagState)
{
	if (flagState == FLAG_UP) {
		if (flag == FLAG_DOWN) {
			flag = FLAG_UP;
			dispLedOut(0,1);   //red led oon
		}
	} else {
		if (flag == FLAG_UP) {
			flag = FLAG_DOWN;
			dispLedOut(0,0);   //red led off
		}
	}
}

/*
 * Go through the email list and set the read flag appropriately
 */
void FlagCheck(void)
{
	int ptr;
	int status;

	status = FLAG_DOWN;

	if (emailList.head == -1) {
		FlagChange(FLAG_DOWN);
	}
	for (ptr = emailList.head; (ptr != -1) && (status == FLAG_DOWN); ptr = emailIndex[ptr].next) {
		if (emailIndex[ptr].read == 0) {
			status = FLAG_UP;
		}
	}
	FlagChange(status);
}

/*
 * Create an empty email list
 */
void CreateEmailList(void)
{
	int i;
	for (i = 0; i < MAX_EMAILS; i++) {
		emailIndex[i].used = 0;
		emailIndex[i].read = 0;
		emailIndex[i].prev = -1;
		emailIndex[i].next = -1;
	}

	emailList.head = -1;
	emailList.tail = -1;
	emailList.ptr = -1;

	emailBuffer = xalloc(sizeof(Email) * MAX_EMAILS);
}

/*
 * Initialize the display buffer
 */
void DispBufferInit(void)
{
	dispBuffer = xalloc((DISP_COLS + 1) * DISP_BUFFER_LINES);
}

/*
 * Display the current window within the display buffer
 */
void DispUpdateWindow(void)
{
	int i;
	char line[DISP_COLS + 1];

	glBlankScreen();
	for (i = 0; i < DISP_ROWS; i++) {
		if ((emailDispLine + i) < emailLastLine) {
			TextGotoXY(&wholewindow, 0, i);
			xmem2root(line, dispBuffer + ((DISP_COLS+1) * (emailDispLine+i)), DISP_COLS+1);
			TextPrintf(&wholewindow, "%s", line);
		} else {
			TextGotoXY(&wholewindow, 0, i);
			TextPrintf(&wholewindow, "");
		}
	}
}

/*
 * Write out part of an email message in the display buffer (e.g., the
 * sender, the subject, the body...)
 */
void DispPart(char *src, int *lineNum, char *string) {
	char line[DISP_COLS + 1];
	int i;
	int j;

	strcpy(line, string);
	i = strlen(line);
	j = 0;
	while (src[j] != '\0') {
		while ((i < DISP_COLS) && (src[j] != '\n') && (src[j] != '\0')) {
			if (src[j] == '\r') {
				j++;
			} else {
				line[i] = src[j];
				i++;
				j++;
			}
		}
		line[i] = '\0';
		root2xmem(dispBuffer + ((DISP_COLS+1) * *lineNum), line, DISP_COLS+1);
		(*lineNum)++;

		if (src[j] == '\n') {
			j++;
			i = 0;
		} else if ((src[j] == '\r') && (src[j+1] == '\n')) {
			j += 2;
			i = 0;
		} else if (src[j] != '\0') {
			i = 0;
		}
	}
}

/*
 * Write out a "No email" message to the display buffer
 */
void DispNoEmail(void)
{
	int i;
	char line[DISP_COLS+1];

	strcpy(line, "No email");
	root2xmem(dispBuffer, line, strlen(line)+1);
	emailDispLine = 0;
	emailLastLine = 1;

	DispUpdateWindow();
}

/*
 * Write out an email message to the display buffer
 */
void DispEmail(void)
{
	int i;
	int j;
	char line[DISP_COLS + 1];
	int row;
	int col;
	int lineNum;

	lineNum = 0;
	emailDispLine = 0;

	if (emailList.ptr == -1) {
		DispNoEmail();
	} else {
		xmem2root(&email, emailBuffer + sizeof(Email)*emailList.ptr, sizeof(Email));
		DispPart(email.from, &lineNum, "FROM: ");
		DispPart(email.subject, &lineNum, "SUBJECT: ");
		strcpy(line, "BODY");
		root2xmem(dispBuffer + ((DISP_COLS+1) * lineNum), line, DISP_COLS+1);
		lineNum++;
		DispPart(email.body, &lineNum, "");

		line[0] = '\0';
		emailLastLine = lineNum;

		DispUpdateWindow();
	}
}

/*
 * Initialize the log buffer
 */
void LogInit(void)
{
	logBuffer = xalloc(LOG_BUFFER_SIZE);
	root2xmem(logBuffer, "\0", 1);
	logPtr = 0;
}

/*
 * Add an entry to the log
 */
void LogAddEntry(char *entry)
{
	int entrySize;
	entrySize = strlen(entry);
	if ((logPtr + entrySize) < (LOG_BUFFER_SIZE - 1)) {
		root2xmem(logBuffer + logPtr, entry, entrySize);
		root2xmem(logBuffer + logPtr + entrySize, "\n\0", 2);
		logPtr = logPtr + entrySize + 1;
	}
}

/*
 * Add an entry about email activity to the log
 */
void LogAddEmailEntry(char *message, char *from, char *subject)
{
	char line[100];
	strcpy(line, "Email ");
	strncat(line, message, 100 - strlen(line));
	strncat(line, " -- FROM: ", 100 - strlen(line));
	strncat(line, from, 100 - strlen(line));
	strncat(line, " -- SUBJECT: ", 100 - strlen(line));
	strncat(line, subject, 100 - strlen(line));
	LogAddEntry(line);
}

/*
 * Strip the encoding from the POST info
 */
void StripEncoding(char *dest, char *src, int len)
{
	int i;
	int j;
	char upper;
	char lower;
	char value;

	i = 0;
	j = 0;
	while( (i < len) && (src[i] != '\0') ) {
		if (src[i] == '+') {
			dest[j] = ' ';
		} else if (src[i] == '%') {
			if ((i + 2) >= len) {
				break;
			}
			upper = toupper(src[++i]);
			lower = toupper(src[++i]);

			if ((upper >= '0') && (upper <= '9')) {
				upper = upper - '0';
			} else if ((upper >= 'A') && (upper <= 'F')) {
				upper = upper - 'A' + 10;
			}
			if ((lower >= '0') && (lower <= '9')) {
				lower = lower - '0';
			} else if ((lower >= 'A') && (lower <= 'F')) {
				lower = lower - 'A' + 10;
			}

			value = (upper * 16) + lower;
			dest[j] = value;
		} else {
			dest[j] = src[i];
		}
		i++;
		j++;
	}
	dest[j] = '\0';
}

/*
 * Add an email to the email list
 */
int AddEmail(char *from, char *subject, char *body)
{
	int i;
	char line[100];

	if (emailList.head == -1) {
		emailList.head = 0;
		emailList.tail = 0;
		emailList.ptr = 0;
		emailIndex[emailList.head].prev = -1;
	} else {
		// Find an available slot
		for (i = 0; i < MAX_EMAILS; i++) {
			if (emailIndex[i].used == 0) {
				emailIndex[emailList.tail].next = i;
				emailIndex[i].prev = emailList.tail;
				emailList.tail = i;
				break;
			}
		}
		if (i == MAX_EMAILS) {
			// Failure--too full
			return -1;
		}
	}
	StripEncoding(email.from, from, MAX_FROM_LEN);
	StripEncoding(email.subject, subject, MAX_SUBJECT_LEN);
	StripEncoding(email.body, body, MAX_BODY_LEN);

	root2xmem(emailBuffer + sizeof(Email)*emailList.tail, &email, sizeof(Email));
	emailIndex[emailList.tail].next = -1;
	emailIndex[emailList.tail].used = 1;
	emailIndex[emailList.tail].read = 0;
	FlagChange(FLAG_UP);
	emailList.ptr = emailList.tail;
	DispEmail();
	LogAddEmailEntry("sent", email.from, email.subject);

	return 0;
}

/*
 * Delete an email from the email list
 */
int DeleteEmail(void)
{
	int prev;
	int i;

	if ((emailList.ptr < 0) || (emailList.ptr >= MAX_EMAILS)) {
		// Index out of range
		return -1;
	}

	emailIndex[emailList.ptr].used = 0;
	if ((emailList.ptr == emailList.head) && (emailList.ptr == emailList.tail)) {
		// Removing only remaining entry
		emailList.head = -1;
		emailList.tail = -1;
		emailList.ptr = -1;
	} else if (emailList.ptr == emailList.head) {
		// Removing first entry
		emailList.head = emailIndex[emailList.ptr].next;
		emailIndex[emailList.head].prev = -1;
		emailList.ptr = emailList.head;
	} else if (emailList.ptr == emailList.tail) {
		// Removing last entry
		emailList.tail = emailIndex[emailList.ptr].prev;
		emailIndex[emailList.tail].next = -1;
		emailList.ptr = emailList.tail;
	} else {
		// Removing middle entry
		emailIndex[emailIndex[emailList.ptr].prev].next = emailIndex[emailList.ptr].next;
		emailIndex[emailIndex[emailList.ptr].next].prev = emailIndex[emailList.ptr].prev;
		emailList.ptr = emailIndex[emailList.ptr].next;
	}
 	FlagCheck();
	return 0;
}

/*
 * Parse the value of the variable, and store it in the appropriate
 * FORMSpec structure.  Return -1 on error.
 */
char *parsePtr;
int ParseToken(HttpState* state, int *bytesRead)
{
	int i;
	int retval;
	int len;
	for (i = 0; i < (sizeof(FORMSpec)/sizeof(FORMType)); i++) {
		if (!strcmp(FORMSpec[i].name, state->buffer)) {
			parsePtr = FORMSpec[i].value;
			len = 0;
			retval = sock_fastread(&state->s, parsePtr, 1);
			while ((*bytesRead < (state->content_length - 2)) &&
			       (tcp_tick(&state->s) != 0) &&
			       (*parsePtr != '&')) {
				if (retval != 0) {
					(*bytesRead)++;
					if (len < (FORMSpec[i].len - 1)) {
						parsePtr++;
						len++;
					}
				}
				retval = sock_fastread(&state->s, parsePtr, 1);
			}
			*parsePtr = '\0';
		}
		if (i < (sizeof(FORMSpec)/sizeof(FORMType) - 1) && (tcp_tick(&state->s) == 0)) {
			return -1;
		}
	}
	return 1;
}

/*
 * Parse the url-encoded POST data into the FORMSpec struct
 * (ie: parse 'foo=bar&baz=qux' into the struct).  Return -1
 * on error.
 */
int ParsePost(HttpState* state)
{
	int retval;
	int bytesRead;

	bytesRead = 0;

	while ((bytesRead < (state->content_length - 2)) && (tcp_tick(&state->s) != 0)) {
		retval = sock_fastread(&state->s, state->p, 1);

		if (retval != 0) {
			bytesRead++;
			if (*state->p == '=') {
				*state->p = '\0';
				state->p = state->buffer;
				if (ParseToken(state, &bytesRead) == -1) {
					return -1;
				}
			} else {
				state->p++;
			}
		}
	}
	if (bytesRead == (state->content_length - 2)) {
		return 1;
	} else {
		return -1;
	}
}

/*
 * Accept the new email and display it to the LCD.
 */
int Submit(HttpState* state)
{
	if (state->length) {
		/* buffer to write out */
		if (state->offset < state->length) {
			state->offset += sock_fastwrite(&state->s,
					state->buffer + (int)state->offset,
					(int)state->length - (int)state->offset);
		} else {
			state->offset = 0;
			state->length = 0;
		}
	} else {
		switch (state->substate) {
		case 0:
			/* init the FORMSpec data */
			FORMSpec[0].value[0] = '\0';
			FORMSpec[1].value[0] = '\0';
			FORMSpec[2].value[0] = '\0';

			state->p = state->buffer;
			state->substate++;
			break;

		case 1:
			/* parse the POST information */
			if (ParsePost(state) != -1) {
				// Add the email to the email list
				if (AddEmail(emailTemp.from, emailTemp.subject, emailTemp.body) != -1) {
					state->substate++;
				} else {
					// Failed to add the email
					state->substate = 10;
				}
			} else {
				// Failed to parse the new email
				state->substate = 10;
			}
			break;

		case 2:
			state->substate = 0;
			cgi_redirectto(state,REDIRECTTO);
			break;

		// This case only occurs when there has been an error
		case 10:
			strcpy(state->buffer, "HTTP/1.0 200 OK\r\n\r\n");
			state->length = strlen(state->buffer);
			state->offset = 0;
			state->substate++;
			break;

		case 11:
			strcpy(state->buffer, "<HTML><HEAD><TITLE>Email Failed!</TITLE></HEAD><BODY><H1>Email failed!</H1>\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		case 12:
			strcpy(state->buffer, "<P><A HREF=\"/\">Back to main page</A></BODY></HTML>\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		default:
			state->substate = 0;
			return 1;
		}
	}

	return 0;
}

/*
 * Process a read email request from the web browser--all emails
 * are displayed.
 */
int emailCurr;
int emailNum;
int bodyPosition;
int Read(HttpState* state)
{
	int i;
	char line[10];

	if (state->length) {
		/* buffer to write out */
		if (state->offset < state->length) {
			state->offset += sock_fastwrite(&state->s,
					state->buffer + (int)state->offset,
					(int)state->length - (int)state->offset);
		} else {
			state->offset = 0;
			state->length = 0;
		}
	} else {
		switch (state->substate) {
		case 0:
			strcpy(state->buffer, "HTTP/1.0 200 OK\r\n\r\n");
			state->length = strlen(state->buffer);
			state->offset = 0;
			state->substate++;
			break;

		case 1:
			strcpy(state->buffer, "<html><head><title>Read Email</title></head><body>\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		case 2:
			emailCurr = emailList.head;
			emailNum = 1;
			strcpy(state->buffer, "<H1>Read Email</H1>\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		case 3:
			bodyPosition = 0;
			if (emailCurr != -1) {
				strcpy(state->buffer, "<B>Email #");
				itoa(emailNum, line);
				strcat(state->buffer, line);
				strcat(state->buffer, "</B><P><PRE>");
				state->length = strlen(state->buffer);
				state->substate++;
			} else if (emailNum == 1) {
				strcpy(state->buffer, "No email");
				state->length = strlen(state->buffer);
				state->substate = 9;
			} else {
				state->substate = 9;
			}
			break;

		case 4:
			xmem2root(&email, emailBuffer	+ sizeof(Email)*emailCurr, sizeof(Email));
			strcpy(state->buffer, "FROM: ");
			strcat(state->buffer, email.from);
			strcat(state->buffer, "\r\n");
			state->length = strlen(state->buffer);
			emailNum++;
			emailCurr = emailIndex[emailCurr].next;
			state->substate++;
			break;

		case 5:
			strcpy(state->buffer, "SUBJECT: ");
			strcat(state->buffer, email.subject);
			strcat(state->buffer, "\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		case 6:
			strcpy(state->buffer, "BODY\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		case 7:
			strncpy(state->buffer, email.body + bodyPosition, HTTP_MAXBUFFER);
			if (state->buffer[HTTP_MAXBUFFER - 1] != '\0') {
				state->buffer[HTTP_MAXBUFFER - 1] = '\0';
				if (strlen(state->buffer) == (HTTP_MAXBUFFER - 1)) {
					// More data to get
					bodyPosition += HTTP_MAXBUFFER - 1;
				} else {
					state->substate++;
				}
			} else {
				state->substate++;
			}
			state->length = strlen(state->buffer);
			break;

		case 8:
			strcpy(state->buffer, "\r\n</PRE>\r\n");
			state->length = strlen(state->buffer);
			state->substate = 3;
			break;

		case 9:
			strcpy(state->buffer, "<p><a href=\"/\">Back to main page</a></body></html>\r\n");
			state->length = strlen(state->buffer);
			state->substate++;
			break;

		default:
			state->substate = 0;
			return 1;
		}
	}

	return 0;
}

/*
 * Process a view log request--the log of email actions is displayed.
 */
int logPosition;
int ViewLog(HttpState *state)
{
	if (state->length) {
		/* buffer to write out */
		if (state->offset < state->length) {
			state->offset += sock_fastwrite(&state->s,
					state->buffer + (int)state->offset,
					(int)state->length - (int)state->offset);
		} else {
			state->offset = 0;
			state->length = 0;
		}
	} else {
		switch (state->substate) {
			case 0:
				logPosition = 0;
				strcpy(state->buffer, "HTTP/1.0 200 OK\r\n\r\n");
				state->length = strlen(state->buffer);
				state->offset = 0;
				state->substate++;
				break;

			case 1:
				strcpy(state->buffer, "<html><head><title>Display Log</title></head><body>\r\n");
				state->length = strlen(state->buffer);
				state->substate++;
				break;

			case 2:
				strcpy(state->buffer, "<H1>Display Log</H1>\r\n<PRE>\r\n");
				state->length = strlen(state->buffer);
				state->substate++;
				break;

			case 3:
				xmem2root(state->buffer, logBuffer + logPosition, HTTP_MAXBUFFER);
				if (state->buffer[HTTP_MAXBUFFER - 1] != '\0') {
					state->buffer[HTTP_MAXBUFFER - 1] = '\0';
					if (strlen(state->buffer) == (HTTP_MAXBUFFER - 1)) {
						// More data to get
						logPosition += HTTP_MAXBUFFER - 1;
					} else {
						state->substate++;
					}
				} else {
					state->substate++;
				}
				state->length = strlen(state->buffer);
				break;

			case 4:
				strcpy(state->buffer, "\r\n</PRE>\r\n");
				state->length = strlen(state->buffer);
				state->substate++;
				break;

			case 5:
				strcpy(state->buffer, "<p><a href=\"/\">Back to main page</a></body></html>\r\n");
				state->length = strlen(state->buffer);
				state->substate++;
				break;

			default:
				state->substate = 0;
				return 1;
		}
	}
	return 0;
}

/*
 *  The resource table associates ximported files with URLs on the webserver.
 */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/enter.html", enter_html),
	
	SSPEC_RESOURCE_FUNCTION("/enteremail.cgi", Submit),
	SSPEC_RESOURCE_FUNCTION("/read.html", Read),
	SSPEC_RESOURCE_FUNCTION("/display.html", ViewLog)
SSPEC_RESOURCETABLE_END

/*
 * These constants define keypad actions
 */
#define DELETE 		'1'
#define READ 			'2'
#define EMAIL_UP 		'3'
#define EMAIL_DOWN 	'4'
#define SCROLL_UP 	'5'
#define SCROLL_DOWN 	'6'
#define IGNORE 		'7'

void initsystem()
{
	auto int status;

	brdInit();
	dispInit();
	glXFontInit(&fi6x8, 6, 8, 32, 127, Font6x8);
	status = TextWindowFrame(&wholewindow, &fi6x8,0, 0, LCD_XS, LCD_YS);

	keyConfig ( 0, EMAIL_DOWN,  0, 0, 0,  0, 0);
	keyConfig ( 1, SCROLL_UP,   0, 0, 0,  0, 0);
	keyConfig ( 2, SCROLL_DOWN, 0, 0, 0,  0, 0);
	keyConfig ( 3, EMAIL_UP,    0, 0, 0,  0, 0);
	keyConfig ( 4, READ,        0, 0, 0,  0, 0);
	keyConfig ( 5, DELETE,      0, 0, 0,  0, 0);
	keyConfig ( 6, IGNORE,      0, 0, 0,  0, 0);
}

main()
{
	auto unsigned wKey;
	auto int i;

	/* init FORM searchable names - must init ALL FORMSpec structs! */
	FORMSpec[0].name = "email_from";
	FORMSpec[0].value = emailTemp.from;
	FORMSpec[0].len = MAX_FROM_LEN;
	FORMSpec[1].name = "email_subject";
	FORMSpec[1].value = emailTemp.subject;
	FORMSpec[1].len = MAX_SUBJECT_LEN;
	FORMSpec[2].name = "email_body";
	FORMSpec[2].value = emailTemp.body;
	FORMSpec[2].len = MAX_BODY_LEN;

	initsystem();

	sock_init();
	http_init();
	tcp_reserveport(80);

	FlagInit();
	CreateEmailList();
	LogInit();
	DispBufferInit();
	DispNoEmail();

	while (1) {
		http_handler();

		costate {
			keyProcess();
			waitfor (DelayMs(10));
		}

		costate {
			waitfor (wKey = keyGet());
			switch (wKey) {
				case DELETE:
					if (emailList.ptr != -1) {
						LogAddEmailEntry("deleted", email.from, email.subject);
						DeleteEmail();
					}
					DispEmail();
					break;
				case READ:
					if ((emailList.ptr != -1) && (emailIndex[emailList.ptr].read != 1)) {
						LogAddEmailEntry("read", email.from, email.subject);
						emailIndex[emailList.ptr].read = 1;
						FlagCheck();
					}
					break;
				case EMAIL_UP:
				if (emailList.ptr != -1) {
					if (emailIndex[emailList.ptr].prev != -1) {
						emailList.ptr = emailIndex[emailList.ptr].prev;
					}
					DispEmail();
				}
				break;
				case EMAIL_DOWN:
					if (emailList.ptr != -1) {
						if (emailIndex[emailList.ptr].next != -1) {
							emailList.ptr = emailIndex[emailList.ptr].next;
						}
						DispEmail();
					}
					break;
				case SCROLL_UP:
					if ((emailList.ptr != -1) && (emailDispLine > 0)) {
						emailDispLine--;
						DispUpdateWindow();
					}
					break;
				case SCROLL_DOWN:
					if ((emailList.ptr != -1) && ((emailDispLine + 4) < emailLastLine)) {
						emailDispLine++;
						DispUpdateWindow();
					}
					break;

				default: // Do nothing
					break;

			}
		}
	}
}
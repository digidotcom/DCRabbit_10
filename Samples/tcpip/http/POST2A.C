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
        Samples\TCPIP\HTTP\post2a.c

        This is an alternate version of post2.c that uses the ZSERVER.LIB
        functionality to build the server table.  Comparison with post2.c
        shows how a program can be converted from using http_flashspec
        to ZSERVER.LIB functionality.

        This uses the post.c style form submission, and the ssi.c style
        dynamic pages to build a fully functional and audited contoller.
        When users first access the page, they enter their name in a form,
        that is then stored in a HTTP cookie. This is used to later build
        an audit trail of what changes each user makes.
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
 * only one socket and server are needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1


/*
 * Increase the number of spec entries
 */
#define SSPEC_MAXSPEC 19

/*
 * Don't use the FlashSpec structure
 */
#define HTTP_NO_FLASHSPEC



/********************************
 * End of configuration section *
 ********************************/

/*
 *  REDIRECTTO is used by each ledxtoggle cgi's to tell the
 *  browser which page to hit next.
 *
 */

#define REDIRECTTO      "/"
#define REGISTERFORM    "/register.html"

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

#ximport "samples/tcpip/http/pages/form.html"      reg_form
#ximport "samples/tcpip/http/pages/ssi2.shtml"     index_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"    rabbit1_gif
#ximport "samples/tcpip/http/pages/ledon.gif"      ledon_gif
#ximport "samples/tcpip/http/pages/ledoff.gif"     ledoff_gif
#ximport "samples/tcpip/http/pages/button.gif"     button_gif
#ximport "samples/tcpip/http/pages/showsrc.shtml"  showsrc_shtml
#ximport "samples/tcpip/http/post2a.c"             ssi_c

/* the default for / must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".shtml", MIMETYPE_HTML, shtml_handler),
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END

#define MAX_FORMSIZE	64
typedef struct {
	char *name;
	char value[MAX_FORMSIZE];
} FORMType;
FORMType FORMSpec[2];

char led1[15];
char led2[15];
char led3[15];
char led4[15];

/*
 * The audit list - stores the last NUM_ENTRIES changes to the site
 */
#define NUM_ENTRIES	16
typedef struct {
	char who[HTTP_MAXNAME];
	char which;		/* which LED changed */
	char led1;		/* updated status of all LEDs */
	char led2;
	char led3;
	char led4;
} AUDITLine;
AUDITLine audit_list[NUM_ENTRIES];
int current_line;

char get_led_status(char *led)
{
   	if (strcmp(led,"ledon.gif")==0)
   		return 1;
   	else
   		return 0;
}

void AuditInitLine(AUDITLine* line)
{
	line->who[0] = '\0';
	line->led1 = get_led_status(led1);
	line->led2 = get_led_status(led2);
	line->led3 = get_led_status(led3);
	line->led4 = get_led_status(led4);
}

void AuditInit(void)
{
	int i;
	for(i=0;i<NUM_ENTRIES;i++) {
		AuditInitLine(&audit_list[i]);
	}
	current_line = 0;
}

/**
 * 	print the audit list.  Call by SSI exec command in "SSI2.shtml"
 * 	(ie, it holds the static areas of the HTML page).
 */
int audit_list_print(HttpState* state)
{
	int printline;
	AUDITLine* p;

	if(state->substate >= NUM_ENTRIES)
		return 1;

	printline = current_line - state->substate;
	if(printline < 0)
		printline += NUM_ENTRIES;


	p = &audit_list[printline];

	if('\0' == p->who[0]) {
		state->substate++;
		return 0;
	}

	sprintf(state->buffer, "<tr><td>%s</td><td>led%d</td><td>%c%c%c%c</td></tr>\r\n",
		p->who,
		p->which,
		1 == p->led1 ? 'O' : 'X',
		1 == p->led2 ? 'O' : 'X',
		1 == p->led3 ? 'O' : 'X',
		1 == p->led4 ? 'O' : 'X'
		);
	state->headerlen = strlen(state->buffer);
	state->headeroff = 0;
	state->substate++;

	return 0;
}

void add_audit(HttpState* state, char which)
{
	struct sockaddr sock_addr;

	current_line++;
	if(current_line >= NUM_ENTRIES)
		current_line = 0;

	AuditInitLine(&audit_list[current_line]);

	audit_list[current_line].which = which;

	strcpy(audit_list[current_line].who, state->cookie);
}

/*
 *  Instead of sending other text back from the cgi's
 *  we have decided to redirect them to the original page.
 *  the cgi_redirectto forms a header which will redirect
 *  the browser back to the main page.
 *
 */

int led1toggle(HttpState* state)
{
   if('\0' == state->cookie[0]) {
   	cgi_redirectto(state,REGISTERFORM);
   	return 0;
   }

   if (strcmp(led1,"ledon.gif")==0)
      strcpy(led1,"ledoff.gif");
   else
      strcpy(led1,"ledon.gif");

   add_audit(state, 1);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led2toggle(HttpState* state)
{
   if('\0' == state->cookie[0]) {
   	cgi_redirectto(state,REGISTERFORM);
   	return 0;
   }

   if (strcmp(led2,"ledon.gif")==0)
      strcpy(led2,"ledoff.gif");
   else
      strcpy(led2,"ledon.gif");

   add_audit(state, 2);


   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led3toggle(HttpState* state)
{
   if('\0' == state->cookie[0]) {
   	cgi_redirectto(state,REGISTERFORM);
   	return 0;
   }

   if (strcmp(led3,"ledon.gif")==0)
      strcpy(led3,"ledoff.gif");
   else
      strcpy(led3,"ledon.gif");

   add_audit(state, 3);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led4toggle(HttpState* state)
{
   if('\0' == state->cookie[0]) {
   	cgi_redirectto(state,REGISTERFORM);
   	return 0;
   }

   if (strcmp(led4,"ledon.gif")==0)
      strcpy(led4,"ledoff.gif");
   else
      strcpy(led4,"ledon.gif");

   add_audit(state, 4);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

/*
 * parse the url-encoded POST data into the FORMSpec struct
 * (ie: parse 'foo=bar&baz=qux' into the struct
 */
int parse_post(HttpState* state)
{
	auto int retval;
	auto int i;

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
		return 1;
	}

	// Using the subsubstate to keep track of how much data we have received
	state->subsubstate += retval;

	if (state->subsubstate >= state->content_length) {
		// NULL-terminate the content buffer
		state->buffer[(int)state->content_length] = '\0';

		// Scan the received POST information into the FORMSpec structure
		for(i=0; i<(sizeof(FORMSpec)/sizeof(FORMType)); i++) {
			http_scanpost(FORMSpec[i].name, state->buffer, FORMSpec[i].value,
			              MAX_FORMSIZE);
		}

		// Finished processing--returning 1 indicates that we are done
		return 1;
	}
	// Processing not finished--return 0 so that we can be called again
	return 0;
}

/*
 * Sample submit.cgi function
 */
int submit(HttpState* state)
{
	int i;

	if(state->length) {
		/* buffer to write out */
		if(state->offset < state->length) {
			state->offset += sock_fastwrite(&state->s,
					state->buffer + (int)state->offset,
					(int)state->length - (int)state->offset);
		} else {
			state->offset = 0;
			state->length = 0;
		}
	} else {
		switch(state->substate) {
		case 0:
			_f_strcpy(state->buffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n");
			break;

		case 1:
			/* init the FORMSpec data */
			FORMSpec[0].value[0] = '\0';
			FORMSpec[1].value[0] = '\0';
			state->p = state->buffer;

			parse_post(state);

			state->substate++;
			return 0;

		case 2:
			http_setcookie(state->buffer, FORMSpec[0].value);
			break;

		case 3:
			_f_strcpy(state->buffer, "\r\n\r\n<html><head><title>Results</title></head><body>\r\n");
			break;

		case 4:
			sprintf(state->buffer, "<p>Username: %s<p>\r\n<p>Email: %s<p>\r\n",
				FORMSpec[0].value, FORMSpec[1].value);
			break;

		case 5:
			_f_strcpy(state->buffer, "<p>Go <a href=\"/\">home</a></body></html>\r\n");
			break;

		default:
			state->substate = 0;
			return 1;
		}

		state->length = strlen(state->buffer);
		state->offset = 0;
		state->substate++;
	}

	return 0;
}

void main()
{
	/* FORM stuff */
	sspec_addxmemfile("register.html", reg_form, SERVER_HTTP);
	sspec_addfunction("submit.cgi", submit, SERVER_HTTP);

	/* normal SSI button stuff */
	sspec_addxmemfile("/", index_html, SERVER_HTTP);
	sspec_addxmemfile("showsrc.shtml", showsrc_shtml, SERVER_HTTP);
	sspec_addxmemfile("rabbit1.gif", rabbit1_gif, SERVER_HTTP);
	sspec_addxmemfile("ledon.gif", ledon_gif, SERVER_HTTP);
	sspec_addxmemfile("ledoff.gif", ledoff_gif, SERVER_HTTP);
	sspec_addxmemfile("button.gif", button_gif, SERVER_HTTP);

	sspec_addxmemfile("ssi.c", ssi_c, SERVER_HTTP);

	sspec_addvariable("led1", led1, PTR16, "%s", SERVER_HTTP);
	sspec_addvariable("led2", led2, PTR16, "%s", SERVER_HTTP);
	sspec_addvariable("led3", led3, PTR16, "%s", SERVER_HTTP);
	sspec_addvariable("led4", led4, PTR16, "%s", SERVER_HTTP);

	sspec_addfunction("led1tog.cgi", led1toggle, SERVER_HTTP);
	sspec_addfunction("led2tog.cgi", led2toggle, SERVER_HTTP);
	sspec_addfunction("led3tog.cgi", led3toggle, SERVER_HTTP);
	sspec_addfunction("led4tog.cgi", led4toggle, SERVER_HTTP);
	sspec_addfunction("audit", audit_list_print, SERVER_HTTP);

	strcpy(led1,"ledon.gif");
	strcpy(led2,"ledon.gif");
	strcpy(led3,"ledoff.gif");
	strcpy(led4,"ledon.gif");

	/* Init the audit history */
   	AuditInit();

	/* init FORM searchable names - must init ALL FORMSpec structs! */
	FORMSpec[0].name = "user_name";
	FORMSpec[1].name = "user_email";

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
	http_init();
	tcp_reserveport(80);

	while (1) {
		http_handler();
	}
}


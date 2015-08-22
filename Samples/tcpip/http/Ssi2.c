/*******************************************************************************
        Samples\TcpIp\HTTP\ssi2.c
        Rabbit Semiconductor, 2000

        The same lights and buttons from ssi.c, with an audit log
        of who made what changes, recoded as a hash of the remote
        user's IP address.
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

//#define DCRTCP_DEBUG
//#define TCP_VERBOSE

/*
 * TCP/IP modification - increase TCP socket buffer
 * size, for increased performance.  Note that this buffer size is split in
 * two for TCP sockets-- half for send and half for receive.
 */
#define TCP_BUF_SIZE (1460*4)		// 1460 is the default max segment size for TCP socket, allow 2 for Tx and Rx.

/*
 * Web server configuration
 */

/*
 * Only one server is needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

/*
 *  REDIRECTTO is used by each ledxtoggle cgi's to tell the
 *  browser which page to hit next.  The default REDIRECTTO
 *  assumes that you are serving a page that does not have
 *  any address translation applied to it.
 *
 */

#define REDIRECTTO 		"/index.shtml"

/*
 *  Notice that we have ximported in the source code for
 *  this program.  This allows us to <!--#include file="ssi.c"-->
 *  in the pages/showsrc.shtml (even though this is "SSI2.C").
 *
 */

#ximport "samples/tcpip/http/pages/ssi2.shtml"      index_html
#ximport "samples/tcpip/http/pages/rabbit1.gif"    rabbit1_gif
#ximport "samples/tcpip/http/pages/ledon.gif"      ledon_gif
#ximport "samples/tcpip/http/pages/ledoff.gif"     ledoff_gif
#ximport "samples/tcpip/http/pages/button.gif"     button_gif
#ximport "samples/tcpip/http/pages/showsrc.shtml"  showsrc_shtml
#ximport "samples/tcpip/http/ssi2.c"                ssi_c

/*
 *  In this case the .html is not the first type in the
 *  type table.  This causes the default (no extension)
 *  to assume the shtml_handler.
 *
 */

/* the default for / must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".shtml", MIMETYPE_HTML, shtml_handler),
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END

/*
 *  Each ledx contains a text string that is either
 *  "ledon.gif" or "ledoff.gif"  This string is toggled
 *  each time the ledxtoggle.cgi is requested from the
 *  browser.
 *
 */

char led1[15];
char led2[15];
char led3[15];
char led4[15];

/*
 * The audit list - stores the last NUM_ENTRIES changes to the site
 */
#define NUM_ENTRIES	16
typedef struct {
	longword address;	/* who made the change */
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
	line->address = 0;
	line->led1 = get_led_status(led1);
	line->led2 = get_led_status(led2);
	line->led3 = get_led_status(led3);
	line->led4 = get_led_status(led4);
}

void AuditInit(void)
{
	auto int i;
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
	auto int printline;
	auto AUDITLine* p;

	if(state->substate >= NUM_ENTRIES)
		return 1;

	printline = current_line - state->substate;
	if(printline < 0)
		printline += NUM_ENTRIES;


	p = &audit_list[printline];

	if(0 == p->address) {
		state->substate++;
		return 0;
	}

	sprintf(state->buffer, "<tr><td>%02X</td><td>led%d</td><td>%c%c%c%c</td></tr>\r\n",
		((byte)(p->address)) ^ ((byte)(p->address >> 8)),
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

	if(0 == getpeername((sock_type*)&state->s, &sock_addr, NULL)) {
		audit_list[current_line].address = sock_addr.s_ip;
	} else {
		audit_list[current_line].address = 1;
	}

	audit_list[current_line].which = which;
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
   if (strcmp(led4,"ledon.gif")==0)
      strcpy(led4,"ledoff.gif");
   else
      strcpy(led4,"ledon.gif");

   add_audit(state, 4);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_XMEMFILE("/index.shtml", index_html),
	SSPEC_RESOURCE_XMEMFILE("/showsrc.shtml", showsrc_shtml),
	SSPEC_RESOURCE_XMEMFILE("/rabbit1.gif", rabbit1_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledon.gif", ledon_gif),
	SSPEC_RESOURCE_XMEMFILE("/ledoff.gif", ledoff_gif),
	SSPEC_RESOURCE_XMEMFILE("/button.gif", button_gif),
	SSPEC_RESOURCE_XMEMFILE("/ssi.c", ssi_c),
	SSPEC_RESOURCE_ROOTVAR("led1", led1, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led2", led2, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led3", led3, PTR16, "%s"),
	SSPEC_RESOURCE_ROOTVAR("led4", led4, PTR16, "%s"),
	SSPEC_RESOURCE_FUNCTION("/led1tog.cgi", led1toggle),
	SSPEC_RESOURCE_FUNCTION("/led2tog.cgi", led2toggle),
	SSPEC_RESOURCE_FUNCTION("/led3tog.cgi", led3toggle),
	SSPEC_RESOURCE_FUNCTION("/led4tog.cgi", led4toggle),
	SSPEC_RESOURCE_FUNCTION("/audit", audit_list_print)
SSPEC_RESOURCETABLE_END


void main()
{
   strcpy(led1,"ledon.gif");
   strcpy(led2,"ledon.gif");
   strcpy(led3,"ledoff.gif");
   strcpy(led4,"ledon.gif");

   /* Init the audit history */
   AuditInit();

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
	tcp_reserveport(80);

   while (1) {
      http_handler();
   }
}
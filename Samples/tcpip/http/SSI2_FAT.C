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
        Samples\TcpIp\HTTP\ssi2_fat.c

        This is the same as ssi2.c, except that the FAT filesystem is used
        to contain the web pages and images.

        NOTE:
        Before running this sample, you must run samples\tcpip\zserver\fat_setup.c
        That sample copies the necessary files to the FAT filesystem (you only
        have to run it once).

*******************************************************************************/
#class auto

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/


// Necessary for zserver.
#define FAT_USE_FORWARDSLASH

/*
 *  This value is required to enable FAT blocking mode for ZServer.lib
 */
#define FAT_BLOCK


#use "fat16.lib"


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
 *  browser which page to hit next.
 *
 */

#define REDIRECTTO 		"/A/ssi2.ssi"


// Import the source code for this sample.
#ximport "samples/tcpip/http/ssi2_fat.c"          ssi_c

/*
 *  In this case the .html is not the first type in the
 *  type table.  This causes the default (no extension)
 *  to assume the shtml_handler.
 *
 */

// This table maps file extensions to the appropriate "MIME" type.  This is
// needed for the HTTP server.
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".ssi", MIMETYPE_HTML, shtml_handler),
	SSPEC_MIME(".htm", MIMETYPE_HTML),
	SSPEC_MIME(".cgi", ""),
	SSPEC_MIME(".gif", MIMETYPE_GIF),
SSPEC_MIMETABLE_END

/*
 *  Each ledx contains a text string that is either
 *  "/A/ledon.gif" or "/A/ledoff.gif"  This string is toggled
 *  each time the ledxtoggle.cgi is requested from the
 *  browser.
 *
 */

char led1[15];
char led2[15];
char led3[15];
char led4[15];

#define LEDON "/A/ledon.gif"
#define LEDOFF "/A/ledoff.gif"

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
  	return !strcmp(led,LEDON);
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
   if (strcmp(led1,LEDON)==0)
      strcpy(led1,LEDOFF);
   else
      strcpy(led1,LEDON);

   add_audit(state, 1);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led2toggle(HttpState* state)
{
   if (strcmp(led2,LEDON)==0)
      strcpy(led2,LEDOFF);
   else
      strcpy(led2,LEDON);

   add_audit(state, 2);


   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led3toggle(HttpState* state)
{
   if (strcmp(led3,LEDON)==0)
      strcpy(led3,LEDOFF);
   else
      strcpy(led3,LEDON);

   add_audit(state, 3);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

int led4toggle(HttpState* state)
{
   if (strcmp(led4,LEDON)==0)
      strcpy(led4,LEDOFF);
   else
      strcpy(led4,LEDON);

   add_audit(state, 4);

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

SSPEC_RESOURCETABLE_START
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
	int rc, i;
   char buf[20];

	printf("Initializing filesystem...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
	// Verify that PARTITION_A (i.e partition 0 on device 0) is mounted.
	if (NULL == PARTITION_A)
	{
		// This sample program cannot succeed without access to PARTITION_A.
		printf("The required PARTITION_A was not mounted; exiting now.\n");
		exit(rc);
	}
   if (rc)
   	printf("Failed to initialize, rc=%d\nProceeding anyway...\n", rc);

   strcpy(led1,LEDON);
   strcpy(led2,LEDON);
   strcpy(led3,LEDOFF);
   strcpy(led4,LEDON);

   /* Init the audit history */
   AuditInit();

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
	tcp_reserveport(80);
   http_set_path("/", "A/ssi2.ssi");	// Set default resource name

   printf("Now try connecting via your web browser.\n");
   printf("Try a URL of http://%s/\n", inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
   printf("\nPress any key to bring down the server cleanly.\n");

   while (1) {
      http_handler();
      if (kbhit())
      {
		   // Cycle through all partitions and unmount as needed
		   for(i = 0; i < SSPEC_MAX_PARTITIONS; i++) {
		   	// See if the partition is registered
    			if (sspec_fatregistered(i)) {
		      	// The partition was registered, lets unmount it
					fat_UnmountDevice(sspec_fatregistered(i)->dev);
		      }
		   }
         exit(0);
      }
   }
}
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
        Samples\tcpip\POP3\parse_extra.c

        A simple program that will connect to a POP3 server and download
        e-mail form it, optionally deleting the messages after they have
        been read. This version will parse the incoming mail more, to provide
        an easier to use format.
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
 * POP3 settings
 */

/*
 *	Enter the name of your POP3 server here.
 */
#define POP_HOST	"mail.domain.com"

/*
 * This is the username and password for the account on the
 * pop3 server.  These names must not change until complete.
 */
#define POP_USER	"myname"
#define POP_PASS	"secret"

/* Uncomment this to show extra debug output */
//#define POP_DEBUG

/* comment this out to delete the messages off the server after they are read */
#define POP_NODELETE

/*
 * When this is defined, the POP3 library will do extra parsing of the
 * incoming e-mails, seperating the 'to:', 'from:', 'subject:' and body
 * fields from the rest of the header, and provide this data in a nicer
 * manner.
 * NOTE: Changes the parameters passed to storemsg() .
 */
#define POP_PARSE_EXTRA

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"
#use "pop3.lib"

/*
 * 	This is the POP_PARSE_EXTRA calling style.
 */
int n;
int storemsg(int num, char *to, char *from, char *subject, char *body, int len)
{
#GLOBAL_INIT
	{
		n = -1;
	}

	if(n != num) {
		n = num;
		printf("RECEIVING MESSAGE <%d>\n", n);
		printf("\tFrom: %s\n", from);
		printf("\tTo: %s\n", to);
		printf("\tSubject: %s\n", subject);
	}

	printf("MSG_DATA> '%s'\n", body);

	return 0;
}

void main()
{
	static long address;
	static int retval;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	pop3_init(storemsg);

	printf("Resolving name...\n");
	address = resolve(POP_HOST);
	printf("Calling pop3_getmail()...\n");
	pop3_getmail(POP_USER, POP_PASS, address);

	printf("Entering pop3_tick()...\n");
	while((retval = pop3_tick()) == POP_PENDING)
		continue;

	if(retval == POP_SUCCESS)
		printf("POP was successful!\n");
	if(retval == POP_TIME)
		printf("POP timed out!\n");
	if(retval == POP_ERROR)
		printf("POP returned a general error!\n");

	printf("All done!\n");
}
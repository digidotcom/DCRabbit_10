/*******************************************************************************
        pop.c
        Rabbit Semiconductor, 2000

        A simple program that will connect to a POP3 server and download
        e-mail form it, optionally deleting the messages after they have
        been read.
*******************************************************************************/

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

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

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"
#use "pop3.lib"

/*
 * 	This is the no POP_PARSE_EXTRA calling style.
 */
int n;
int storemsg(int num, char *buf, int len)
{
#GLOBAL_INIT
	{
		n = -1;
	}

	if(n == -1) {
		n = num;
		printf("RECEIVING MESSAGE <%d>\n", n);
	}

	if(n != num) {
		n = num;
		printf("RECEIVING MESSAGE <%d>\n", n);
	}

	printf("MSG_DATA> '%s'\n", buf);

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
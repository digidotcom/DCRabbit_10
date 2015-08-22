/*******************************************************************************
        Samples\TCPIP\telnet\rxsample.c
        Rabbit Semiconductor, 2000

        An example that will listen on a telnet port for incoming data, and
        pass this received data to a user handler, optionally striping the
        telnet controll codes out.

        Sample telnet daemon; receives a connection, optionally cooks the
        input, and stores it into a buffer for you. (nothing is done with
        the buffer for now; the data will be overwritten).  See receive_data()
        for processing code.

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
 * Cook the input? If this is defined, telnet control
 * codes will be removed from the incoming data stresm.
 */
#define INPUT_COOKED
//#undef INPUT_COOKED

/* Also, the function receive_data() below should be looked at */

/********************************
 * End of configuration section *
 ********************************/

/*
 * Force everything into xmem, and import the TCP/IP lib.
 */
#memmap xmem
#use "dcrtcp.lib"

/********************************************************
 * User interface part
 ********************************************************/

void receive_data(char *buf, int len)
{
	/*
	 * this is a user-function that will recieve the data
	 * as it comes in. (buf points to the buffer, len is the
	 * length of the data that was received.)
	 *
	 * Note - When this function returns, the buffer will be
	 * overwritten, so if you want the data, it should be
	 * coppied out of the buffer before the function returns.
	 *
	 * Nothing is done with it for now; it is just dropped on
	 * the floor.
	 */

	 auto int i;
	 for(i=0;i<len;i++)
	 	printf("%c",buf[i]);
}

int init_recv(int port);
void recv_tick(void);

void main()
{

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	/*
	 * Listen on the port, and otherwise prepare the daemon
	 */
	if(init_recv(23)) {
		printf("Error in init_recv - couldn't listen on port\n");
		exit(0);
	}

	/*
	 * call tick to run it
	 */
	while(1) {
		recv_tick();
	}
}


/*************************************************************
 * End of user interface part - main network function
 *************************************************************/

#define MAX_BUFLEN	256

/* vars for telnet recieve daemon */
typedef struct {
	tcp_Socket sock;
	tcp_Socket *s;

	int lport;
	int state;

	char buf[MAX_BUFLEN];
	char cmdbuf[10];
} telnet_recv;

telnet_recv tr_state;
telnet_recv * const state = &tr_state;

/* states */
#define STATE_INIT	0
#define STATE_STEADY	1

/* init the socket */
int init_recv(int port)
{
	state->s = &state->sock;
	state->lport = port;
	tcp_listen(state->s, port, 0, 0, NULL, 0);
	state->state = STATE_INIT;
	return 0;
}

#ifdef INPUT_COOKED
void cook_cmd(void)
{
	auto int len;
	len = 3;

	while(len > 0) {
		len -= sock_fastwrite(state->s, state->cmdbuf + 3 - len, len);
	}
}

int cook_input(int len)
{
	auto int newlen, i;

	newlen = len;

	for(i=0; i<(len-2); i++) {
		if(state->buf[i] == 255) {
			/* controll code */
			switch(state->buf[i + 1]) {
			case 251:
			case 252: {
				sprintf(state->cmdbuf, "%c%c%c",
					255, 254, state->buf[i + 2]);
				cook_cmd();
				memcpy(state->buf + i, state->buf + i + 3, len - (i + 3));
				newlen -= 3;
				break;
			}
			case 253:
			case 254:
				sprintf(state->cmdbuf, "%c%c%c",
					255, 252, state->buf[i + 2]);
				cook_cmd();
				memcpy(state->buf + i, state->buf + i + 3, len - (i + 3));
				newlen -= 3;
				break;

			case 255:
				break;

			default:
				/* unknown command; kill it */
				memcpy(state->buf + i, state->buf + i + 2, len - (i + 2));
				newlen -= 2;
				break;
			}
		}
	}

	return newlen;
}
#endif

void recv_tick(void)
{
	auto int retval;

	tcp_tick(state->s);

	switch(state->state) {
	case STATE_INIT:
		if(sock_established(state->s)) {
			printf("Connection Established.\n");
			state->state = STATE_STEADY;
		}
		break;

	case STATE_STEADY:
		if(!sock_established(state->s)) {
			/* connection died; reset */
			printf("Connection lost.\n\n");
			init_recv(state->lport);
			break;
		}

		retval = sock_fastread(state->s, state->buf, MAX_BUFLEN);
		if(retval) {
			/* we got data */
#ifdef INPUT_COOKED
			retval = cook_input(retval);
#endif
			receive_data(state->buf, retval); /* give it to the user */
		}
		break;

	default:
		/* shouldn't ever get here */
		/* reset the conroller */
		exit(-1);
	}
}
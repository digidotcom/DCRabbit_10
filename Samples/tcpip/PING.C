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
        Samples\tcpip\ping.c

        ICMP demonstration, by pinging a remote host.
        Prints a message when the ping response arrives here.
        If PING_WHO is not defined, then it pings the default
        gateway.

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

/** Remote interface to send PING to (passed to resolve()): **/
/*  Undefine to retrieve default gateway and ping that. */
// #define PING_WHO			"10.10.6.1"


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"

int main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);


	/*
	 *		Get the binary ip address for the target of our
	 *		pinging.
	 */

#ifdef PING_WHO
	/* Ping a specific IP addr: */
	ping_who=resolve(PING_WHO);
	if(ping_who==0) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		return 1;
	}
#else
	/* Examine our configuration, and ping the default router: */
	tmp_seq = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, & ping_who, IFS_END );
	if( tmp_seq != 0 ) {
		printf( "ERROR: ifconfig() failed --> %d\n", (int) tmp_seq );
		return 1;
	}
	if(ping_who==0) {
		printf("ERROR: unable to resolve IF_ROUTER_DEFAULT\n");
		return 1;
	}
#endif

	seq=0;
	for(;;) {
		/*
		 *		It is important to call tcp_tick here because
		 *		ping packets will not get processed otherwise.
		 *
		 */

		tcp_tick(NULL);

		/*
		 *		Send one ping per second.
		 */

		costate {
			waitfor(DelaySec(1));
			_ping(ping_who,seq++);
		}

		/*
		 *		Has a ping come in?  time_out!=0xfffffff->yes.
		 */

		time_out=_chk_ping(ping_who,&tmp_seq);
		if(time_out!=0xffffffff)
			printf("received ping:  %ld\n", tmp_seq);
	}
}
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
        Samples\TcpIp\virtualeth.c

        Demonstrates the use of virtual Ethernet interfaces.  These are
        multiple logical interfaces on a single physical Ethernet interface.
        This could be used to, for instance, replace a number of network
        devices with a single network device.  The advantage of this method
        is that the system that communicates with these devices would not need
        to have its configuration or code changed.

        This program will create 3 virtual Ethernet interfaces in addition
        to 1 physical Ethernet interface on 4 consecutive IP addresses.
        Each interface will have a TCP socket listening on port 9000.  Each
        socket will report when data is received.

	  ** NOTE **

	  This sample will only work on a board with 10 or 10/100 base T Ethernet.
	  The ability to host more than one IP address on the same network port
	  is only supported with Ethernet.

*******************************************************************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 *
 * NOTE:  Since this sample uses a band of sequential IP addresses, we
 * are intentionally NOT using the TCP_CONFIG.LIB setup.  This means that
 * you will need to set your network configuration in the LOCAL_* macros
 * below.
 */
#define TCPCONFIG 		0

/*
 * If VIRTUAL_ETH is defined, then USE_ETHERNET must be defined.  This
 * definition simply means that we are using the first (and probably only)
 * physical Ethernet interface.
 */
#define USE_ETHERNET    0x01

/*
 * Defining VIRTUAL_ETH enables the virtual Ethernet capability.  In this
 * case, we are creating three virtual Ethernet interfaces, in addition to
 * the one physical Ethernet interface.  Each of the virtual interfaces can
 * have their own IP addresses.
 */
#define VIRTUAL_ETH		3

/*
 * The following macros define the network setup of the physical Ethernet
 * interface.  The virtual Ethernet interfaces will be the subsequent IP
 * addresses (e.g., "10.10.6.113", "10.10.6.114", "10.10.6.115").
 */
#define LOCAL_IP			"10.10.6.112"
#define LOCAL_NETMASK	"255.255.255.0"
#define LOCAL_GATEWAY	"10.10.6.1"

/*
 * Each of the TCP sockets will listen on the following port number.
 */
#define LOCAL_PORT  		9000	// listening for incoming connections on this port

/*
 * Maximum Number of seconds to stay in any one state.  It is generally a good
 * idea to have a "catch-all" timeout in case of state machine programming
 * errors.  This timeout will cause a reopen of the socket.
 */
#define TIME2WAIT       1000	// maximum time to wait in any one state (secs.)

/*
 * States for the program state machine.
 */
#define LSTN_STATE		0     // listen for incoming connections
#define ESTB_STATE      1     // check if a connection is established
#define RECV_STATE		2     // waiting for data
#define SEND_STATE		3     // send data
#define CLSE_STATE		4     // close the socket
#define CLWT_STATE		5		// wait for the socket to close
#define ABRT_STATE      6     // error closing so abort

#memmap xmem
#use "dcrtcp.lib"

/*
 * This is the state structure for the program.  This is an array of
 * these structures, one for each Ethernet interface (physical and virtual).
 */
struct{
   int state;					// current state of the state machine
   int iface;					// interface number for this interface
   tcp_Socket s;				// TCP socket structure
   char buff[128];			// buffer for reading and writing data
   int bytes;					// count of number of bytes in the buffer
   unsigned long timer;		// timer to check for timeouts
} socks[VIRTUAL_ETH+1];

/*
 * Prototype for the timeout-checking function.
 */
int timed_out(int);

void main(void)
{
	// index is used to loop through the interfaces
	int index;

	// Initialize the TCP/IP stack
  	sock_init();

   // Initialize the state machine structure
   for (index = 0; index <= VIRTUAL_ETH; index++) {
     	socks[index].state = LSTN_STATE;
      socks[index].iface = 0;
      memset(socks[index].buff, 0, sizeof(socks[index].buff));
      socks[index].bytes = 0;
   }

	// Perform network configuration on the main (physical) Ethernet ineterface
   printf("Bringing up Main Interface %2d:\n", socks[0].iface);
   ifconfig(IF_ETH0,
   			IFS_IPADDR, 	aton(LOCAL_IP),
   			IFS_NETMASK,	aton(LOCAL_NETMASK),
           	IFS_ROUTER_SET,aton(LOCAL_GATEWAY),
            IFS_UP,
          	IFS_END);
   // Wait for the interface to come up
   while (ifpending(IF_ETH0) == IF_COMING_UP) {
		tcp_tick(NULL);
	}
   printf("Main Interface %2d: is up!!\n", socks[0].iface);

	// Configure each of the virtual Ethernet interfaces
   for (index = 1; index <= VIRTUAL_ETH; index++) {
		// virtual_eth() creates a new virtual Ethernet interface and returns
		// the new interface number
   	socks[index].iface = virtual_eth(IF_ETH0, aton(LOCAL_IP) + index,
   	                                 aton(LOCAL_NETMASK), NULL);
      if (socks[index].iface != -1) {
      	printf("Created Virtual Interface %2d:\n", socks[index].iface);
      }
      else {
      	exit(0);
      }
		// Wait for the virtual Ethernet interface to come up
      while (ifpending(socks[index].iface) == IF_COMING_UP) {
			tcp_tick(NULL);
		}
      printf("Virtual Interface %2d: is up!!\n", socks[index].iface);
   }

	// Print out information on the interfaces
	ip_print_ifs();

	// Begin the main program loop
   while (1) {
   	// Iterate over the Ethernet interfaces
   	for (index = 0; index <= VIRTUAL_ETH; index++) {
      	switch (socks[index].state) {
      		// Listen on the socket
      		case LSTN_STATE:
      			// Note that the iface number is passed to tcp_extlisten()
               if (tcp_extlisten(&socks[index].s, socks[index].iface,
                                 LOCAL_PORT, 0, 0, NULL, 0, 0, 0)) {
                  socks[index].state = ESTB_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
                  printf("Interface %2d: listening on port: %5d\n",
                         socks[index].iface, LOCAL_PORT);
               }
               else {
               	// tcp_extlisten() failed--let the user know
                  printf("Interface %2d: tcp_extlisten failed\n",
                         socks[index].iface);
               	socks[index].state = CLSE_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
               }
               break;
           	// Check if a connection has been established
            case ESTB_STATE:
              	if (sock_established(&socks[index].s) ||
              	    sock_bytesready(&socks[index].s) >= 0) {
               	socks[index].state = RECV_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
                  printf("Interface %2d: socket established.\n",
                         socks[index].iface);
               }
               break;
				// Check if data has been received.  If so, read it out.
            case RECV_STATE:
            	// Read any incoming data
            	socks[index].bytes = sock_fastread(&socks[index].s,
            	                                   socks[index].buff,
            	                                   sizeof(socks[index].buff));
               if (socks[index].bytes == -1) {
               	// sock_fastread() returned an error--means that the socket is
               	// likely closed
               	printf("Interface %2d: sock_fastread failed\n",
               	       socks[index].iface);
               	socks[index].state = CLSE_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
               }
               // Check if we received any data
               if (socks[index].bytes > 0) {
               	printf("Interface %2d: revd: %2d bytes\n", socks[index].iface,
               	       socks[index].bytes);
						socks[index].state = SEND_STATE;			// send the data back
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
            	}
            	break;
            // Echo back any received data
         	case SEND_STATE:
            	socks[index].bytes = sock_fastwrite(&socks[index].s,
            	                                    socks[index].buff,
            	                                    socks[index].bytes);
               if (socks[index].bytes == -1) {
               	// sock_fastwrite() returned an error--means that the socket
               	// is likely closed
               	printf("Interface %2d: sock_fastwrite failed\n",
               	       socks[index].iface);
               	socks[index].state = CLSE_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
               }
               // Check how much data was written.  Note that in this program,
               // if not all the data was written, the remaining data will be
               // dropped.  A more realistic program would try sending the rest
               // of the data later, or using sock_awrite() until the data can
               // be sent.
               if (socks[index].bytes > 0) {
               	printf("Interface %2d: sent: %2d bytes\n",
               	       socks[index].iface, socks[index].bytes);
						socks[index].state = RECV_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
            	}
            	break;
				// Close the socket
        		case CLSE_STATE:
               sock_close(&socks[index].s);
               socks[index].state = CLWT_STATE;
               socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
               break;
				// Wait for the socket to completely close
            case CLWT_STATE:
					if (!sock_alive(&socks[index].s)) {
                  printf("Interface %2d: socket closed.\n",
                         socks[index].iface);
                  socks[index].state = LSTN_STATE;
                  socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
               }
               break;
				// Abort the socket--used only if a socket has timed out in one of
				// the closing states
            case ABRT_STATE:
               sock_abort(&socks[index].s);     // abort the socket
               socks[index].state = LSTN_STATE; // try to listen again
               socks[index].timer = SEC_TIMER + TIME2WAIT; // reset the timer
               break;
         }

         // Drive the TCP/IP stack
    		tcp_tick(NULL);

			// Check the timeout on this socket, and close or abort the socket
			// if necessary
         timed_out(index);
    	}
 	}
}

// This function checks if we have been in a state for too long.  If so, then
// it either changes to the close state or aborts the socket (if we were
// already in a close state).
int timed_out(int index)
{
	if ((long)(SEC_TIMER - (socks[index].timer)) > 0) {
      printf("Interface %2d: state: %2d  timed out!!\n", socks[index].iface,
             socks[index].state);
      if((socks[index].state == CLSE_STATE) ||
         (socks[index].state == CLWT_STATE)) {
      	socks[index].state = ABRT_STATE;
      }
      else {
      	socks[index].state = CLSE_STATE;
      }
   	return 1;
   }
}


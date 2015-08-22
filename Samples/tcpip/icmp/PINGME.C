/*******************************************************************************
        pingme.c
        Rabbit Semiconductor, 2000

        A very basic TCP/IP program, that will initilize the TCP/IP
        interface, and allow the device to be 'pinged' from another
        computer on the network.
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


/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use dcrtcp.lib

void main()
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   for (;;) {
      tcp_tick(NULL);
   }
}
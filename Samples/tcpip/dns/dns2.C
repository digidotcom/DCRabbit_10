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
        dns2.c

        Demonstration of how to look up an IP address through a DNS
        server using the nonblocking API.  This function looks up
        multiple hostnames simultaneously.  It is organized around
        a state machine, with separate state information maintained
        for each request.
*******************************************************************************/
#class auto


/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1


// MY_DOMAIN specifies the domain to add to a hostname in the resolving
// process. If the hostname is terminated by a '.', resolve_name_start() does
// not attempt to append MY_DOMAIN. Otherwise the following rules apply:
// If the hostname contains a '.' resolve_name_start first searches this name.
// Upon failure, resolve_name_start() then searches the hostname with '.' and
// MY_DOMAIN appended. Otherwise, if the hostname does not contain a '.'
// resolve_name_start() will _first_ search for the hostname with '.' and
// MY_DOMAIN appended. Upon failure, the function will search for the original
// hostname.
#define MY_DOMAIN			"digi.com"

// This lists the hostnames that will be looked up by this program.  This
// can be customized.
char* const hostnames[] =
	{
	  "www.digi.com",
	  "google.com",
	  "www.frobozz.xyzzy.",					// This host does not exist, so it will
	  												// fail.
	  "www",										// This will have ".digi.com" appended
	  												// to it.
	  "www.yahoo.com"							// This name will be looked up later than
	  												// the others, since only 4 concurrent
	  												// resolves are supported by default.
	};

#memmap xmem
#use "dcrtcp.lib"

// This structure holds the state information for the DNS requests.
typedef struct {
	int state;
	longword ipaddress;
	int handle;
} request_info;

// These are the states of the state machine.
#define DNS_START			0
#define DNS_CHECKING		1
#define DNS_FINISHED		2

void main(void)
{
	request_info dns_requests[sizeof(hostnames) / sizeof(char *)];
	int i;
	int retval;
	char buffer[20];
	int done;

	// Initialize the dns_requests structure
	for (i = 0; i < (sizeof(dns_requests) / sizeof(request_info)); i++) {
		dns_requests[i].state = DNS_START;
	}

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);


	// Loop until we are done with all DNS requests.
	done = 0;
	while (done == 0) {
		tcp_tick(NULL);

		// Drive the state machine
		for (i = 0; i < (sizeof(dns_requests) / sizeof(request_info)); i++) {

			switch (dns_requests[i].state) {
			case DNS_START:
				// Start the process of looking up the hostname.
				// resolve_name_start() returns a handle that we use in
				// subsequent calls to resolve_name_check().
				dns_requests[i].handle = resolve_name_start(hostnames[i]);
				if (dns_requests[i].handle >= 0) {
					printf("Starting the lookup for %s  ...\n", hostnames[i]);
					dns_requests[i].state = DNS_CHECKING;
				} else if (dns_requests[i].handle == RESOLVE_LONGHOSTNAME) {
					printf("The name %s is too large!\n", hostnames[i]);
					dns_requests[i].state = DNS_FINISHED;
				}
				break;

			case DNS_CHECKING:
				// Check if the given DNS request has finished
				retval = resolve_name_check(dns_requests[i].handle,
				                            &dns_requests[i].ipaddress);
				if (retval == RESOLVE_SUCCESS) {
					// The request finished successfully
					printf("%s is %s\n", hostnames[i],
					       inet_ntoa(buffer, dns_requests[i].ipaddress));
					dns_requests[i].state = DNS_FINISHED;
				} else if (retval == RESOLVE_FAILED) {
					// The given hostname does not exist
					printf("%s does not exist\n", hostnames[i]);
					dns_requests[i].state = DNS_FINISHED;
				} else if (retval == RESOLVE_TIMEDOUT) {
					// Our request has timed out, and can not be completed.
					printf("Lookup for %s timed out\n", hostnames[i]);
					dns_requests[i].state = DNS_FINISHED;
				} else if (retval == RESOLVE_AGAIN) {
					// Call resolve_name_check again next time through the state machine
				} else if (retval == RESOLVE_HANDLENOTVALID) {
					// Should never happen in this program
					printf("Invalid handle for %s (should never happen)\n",
					       hostnames[i]);
					dns_requests[i].state = DNS_FINISHED;
				} else {
					// Should never happen
					printf("Unknown return type for %s\n", hostnames[i]);
					dns_requests[i].state = DNS_FINISHED;
				}
				break;

			case DNS_FINISHED:
				// Do nothing
				break;
			}
		}

		// Check if all requests are finished
		done = 1;
		for (i = 0; i < (sizeof(dns_requests) / sizeof(request_info)); i++) {
			if (dns_requests[i].state != DNS_FINISHED) {
				done = 0;
			}
		}
	}

	printf("\nDone!\n");
}
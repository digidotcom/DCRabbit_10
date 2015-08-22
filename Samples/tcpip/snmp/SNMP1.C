/*
 * SNMP1.C  Simple demonstration of SNMP.
 * Copyright 2007, Rabbit Semiconductor
 *
 * Instructions:
 *   . Tell your SNMP management agent about this demo.  This depends
 *     on the particular software, but will typically involve compiling
 *     the SMI/MIB definition files.  The files are located in the
 *     "mibs" subdirectory of the snmp samples.
 *        RABBITSEMI-SMI.txt             - top level Rabbit Semiconductor
 *        RABBITSEMI-PRODUCTS-MIB.txt    - listing of products (boards)
 *        RABBITSEMI-DEMO-SNMP1.txt      - describes this demo.
 *     The management agent can use this information to allow browsing
 *     of the available managed objects.
 *
 *   . Change the initial #define's as desired.  In particular, set
 *     the correct network configuration set (TCPCONFIG).
 *
 *   . Change the password (community name) parameters in the initial
 *     call to snmp_set_dflt_communities().
 *
 *   . Compile and run this program.
 *
 *   . Use your management agent to examine the objects.  You can
 *     modify some of the objects (the ones under the demoRWObjects
 *     subtree).  If you modify rw_int to be greater than 3000, then
 *     trap messages will be sent to the agent.
 */

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 	1

#memmap xmem
#define USE_SNMP		1		// This is necessary for all SNMP applications
#define SNMP_TRAPS			// This must be defined to support trap sending
#define SNMP_INTERFACE	IF_ANY	// Support all incoming interfaces


#define DISABLE_TCP			// Do not require TCP (SNMP uses only UDP)



// Set the IP address of your management agent.
#define MANAGER_IP		"10.10.6.178"

// For this demo only, send trap every 5 sec (optional)
//#define SEND_TRAPS


// Optional definitions to enable Dynamic C debugging and/or extra messages.
//#define DCRTCP_DEBUG
//#define DCRTCP_VERBOSE
//#define MIB_DEBUG
//#define MIB_VERBOSE

#define SNMP_ENTERPRISE			12807			// Rabbit Semiconductor (do not change)


#use "dcrtcp.lib"

/*
 * Managed variables.  Read/write.
 */
int rw_int;
long rw_long;
char rw_fixed[20];
char rw_str[20];
char rw_oct[22];
snmp_oid rw_oid;
longword trapdest_ip;
longword rw_tt;

/*
 * Managed variables.  Read-only.
 */
int r_int;
long r_long;
char r_fixed[20];
char r_str[20];
char r_oct[22];
snmp_oid r_oid;


/*
 * This function will be used as a callback function for one of the
 * read/write variables (rw_long).  It demonstrates how to "scale" a variable
 * from internal units into the units expected by the management
 * agent.  In this case, the variable appears as 1/10th of its internal
 * value.  Note that the transformation needs to work both ways if the
 * variable is writable by the agent.
 */
int scale(snmp_parms * p, int wr, int commit, long * v, word * len, word maxlen)
{
	printf("Callback: wr=%d commit=%d v(in)=%ld ", wr, commit, *v);

	if (wr) {
		// On write by agent, we ensure that the variable is within bounds.
		if (*v > 200000000)
			return SNMP_ERR_badValue;
		if (*v < -200000000)
			return SNMP_ERR_badValue;

		// OK, scale it up to internal representation.
		*v *= 10;
	}
	else
		// Read by the agent: scale it down.
		*v /= 10;

	printf("v(out)=%ld\n", *v);
	return 0;
}


int main()
{
	auto snmp_parms _p;
	auto snmp_parms * p;
	auto word tt;
	auto word trapindices[2];
	auto word monindex;

	// Set the community passwords
	snmp_set_dflt_communities("public", "private", "trap");

	// Set p to be a pointer to _p, for calling convenience.
	p = &_p;

	// Set parameter structure to default initial state (required).
	snmp_init_parms(p);

	// Create the MIB tree.  The following functions all operate on the parameter structure.
	// "p" is passed to all functions, and also set to the return value.  This is the recommended
	// way of doing the MIB tree setup, since if any step fails it will return NULL.  Passing the
	// NULL on to subsequent functions is harmless, and avoids the need to do error checking
	// after each call.  Only at the end of sequence sould "p" be tested for NULL.

	// Set the "root" of the MIB tree for following calls.  Note that the entire MIB
	// tree can be rooted at a different point simply by changing this one call.
	p = snmp_append_parse_stem(p, "3.1.1");	// Set to SNMP_ENTERPRISE.oemExperiments.demos

	// Read/write access - the following is set by default so no need to call.
	p = snmp_set_access(p, SNMP_PUBLIC_MASK|SNMP_PRIVATE_MASK, SNMP_PRIVATE_MASK);

	p = snmp_add_int(p, "1.1.0", &rw_int);
	monindex = snmp_last_index(p);		// Save index for later monitor call

	p = snmp_set_callback(p, scale);
	p = snmp_add_long(p, "1.2.0", &rw_long);	// This variable has a callback function, scale().
	p = snmp_set_callback(p, NULL);

	p = snmp_add_foct(p, "1.3.0", rw_fixed, 20);

	p = snmp_add_str(p, "1.4.0", rw_str, 20);

	p = snmp_add_oct(p, "1.5.0", rw_oct, 22);

	p = snmp_add_objectID(p, "1.6.0", &rw_oid);

	p = snmp_add_ipaddr(p, "1.7.0", &trapdest_ip);

	p = snmp_add_timeticks(p, "1.8.0", &rw_tt);

	// Read-only access for following additions
	p = snmp_set_access(p, SNMP_PUBLIC_MASK|SNMP_PRIVATE_MASK, 0);

	p = snmp_add_int(p, "2.1.0", &r_int);
	trapindices[0] = snmp_last_index(p);

	p = snmp_add_long(p, "2.2.0", &r_long);

	p = snmp_add_foct(p, "2.3.0", r_fixed, 20);

	p = snmp_add_str(p, "2.4.0", r_str, 20);

	p = snmp_add_oct(p, "2.5.0", r_oct, 22);
	trapindices[1] = snmp_last_index(p);

	p = snmp_add_objectID(p, "2.6.0", &r_oid);

	// Initialize the variables.
	rw_int = 1001;
	rw_long = 1000002;
	memcpy(rw_fixed, "rw_fixed abcdefghijk", 20);
	strcpy(rw_str, "rw_str");
	memcpy(rw_oct, "\x06\x00rw_oct", 8);
	memcpy(&rw_oid, &_p, sizeof(snmp_oid));
	trapdest_ip = aton(MANAGER_IP);
	rw_tt = snmp_timeticks();	// Set base epoch

	r_int = 2001;
	r_long = 2000002;
	memcpy(r_fixed, "r_fixed abcdefghijkl", 20);
	strcpy(r_str, "r_str");
	memcpy(r_oct, "\x05\x00r_oct", 7);
	memcpy(&r_oid, &_p, sizeof(snmp_oid));

	// Finally, we check that the MIB tree was constructed without error.
	// If there was any error, p will be set to NULL.
	if (!p) {
		printf("There was an error constructing the MIB.\n");
		exit(1);
	}

	// Monitor the rw_int variable (whose MIB tree index was saved in monindex).
	// trapindices was set up with the indices for r_int and r_oct.
	snmp_monitor(monindex, 0, 3000, 1, 16, 6, &trapdest_ip, SNMP_TRAPDEST, 30, 2, trapindices);

	// See what we've got.
	snmp_print_tree();
	printf("MIB tree: used %ld out of %ld bytes\n", snmp_used(), (long)SNMP_MIB_SIZE);

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	// Print interfaces
	ip_print_ifs();

	// Print routers
	router_printall();


	tt = _SET_SHORT_TIMEOUT(5000);

	for (;;) {
		if (_CHK_SHORT_TIMEOUT(tt)) {
#ifdef SEND_TRAPS
			snmp_trap(trapdest_ip, SNMP_TRAPDEST, 20, 2, trapindices);
#endif
			tt = _SET_SHORT_TIMEOUT(5000);

		}
		tcp_tick(NULL);
	}
}
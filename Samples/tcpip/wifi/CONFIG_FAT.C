/*******************************************************************************
        Samples\TCPIP\WIFI\CONFIG_FAT.C
        Digi International, Copyright (c) 2008.  All rights reserved.

 ******************************************************************************
 ******************************************************************************
 *** Currently, the only core module which will be able to run this sample  ***
 *** is the RCM5450W, and it should be compiled to flash                    ***
 ******************************************************************************
 ******************************************************************************

 This sample shows how to configure the WiFi security-related settings using
 a web-based interface, and using the FAT filesystem to store the relevant
 information.

 Initial web page access userid/password are "admin"/"upload".

 This shows very basic (even boring) web pages, but the aim is to show the
 general method by which the multitude of WiFi configuration options may be
 loaded onto the target board, using the WiFi medium itself.

 Initially, the WiFi interface is brought up in default mode as specified
 by TCPCONFIG 1 in conjunction with any macro settings you may have such
 as
   _PRIMARY_STATIC_IP   = "10.10.6.100"
   MY_GATEWAY           = "10.10.6.1";
   MY_NAMESERVER        = "10.10.6.1";
   IFC_WIFI_SSID        = "mySSID";
   IFC_WIFI_MODE        = IFPARAM_WIFI_INFRASTRUCTURE;
   IFC_WIFI_REGION      = IFPARAM_WIFI_REGION_AMERICAS;
   IFC_WIFI_ENCRYPTION  = IFPARAM_WIFI_ENCR_NONE;
   USE_EAP
   ROOT_SIZE_4K         = 8U

 [Note: USE_EAP is optional.  Include if you want to include EAP authentication.
 USE_EAP is not a library control macro, but is used by this sample in order to
 include or exclude the appropriate code sections.  The ROOT_SIZE_4K macro is
 required, since the default is not enough for this sample.]

 Typically, the default configuration would have no encryption, in order to
 avoid the "chicken and egg" problem of getting the interface going in the
 first place when deployed to some end-user.

 The above options are "compiled in" as initial defaults, which would be the
 "factory defaults" from the end-user's point of view.  This sample takes this
 conventional approach
 of compiling in some known good options.  The Dynamic C stdio window shows
 some useful results, such as the IP address at which the web server may be
 accessed for further configuration.  A real application would need some other
 method of informing the end-user of these details, such as an LCD front panel
 or a separate "discovery" program run on a PC.

 Once the initial application-dependent method of establishing an initial
 network connection, and then being able to connect a web browser to the web
 server running on the target board, are achieved then this sample shows how
 various interface configuration items may be changed at run-time, and stored
 in persistent storage for next time.

 In this sample, the persistent storage is provided by the FAT filesystem which
 is hosted by the serial flash memory on the board.

   NOTE: in the following, an absolute file name like "/A/foo" is taken to
   be a Zserver resource name i.e. the initial /A identifies a mounted
   FAT partition.  If you are directly using the FAT library, then do not
   use the /A prefix, but specify the appropriate device and
   FAT partition where required.  On the RCM5450W, this will be dev 0,
   partition 0.

 For this sample, we assume that there is a base directory like /A/wiconf
 in which the configuration items are stored.  (You can change this directory
 using some macros below).  Within this directory, there are a number of
 sub-directories, one for each wireless network SSID.  Usually, there
 will only be one SSID, however there are many real-world situations where
 several different SSIDs may need to be accessed by the target board.  For
 this reason, you can specify as many SSIDs as desired.

   NOTE: FAT filesystem only allows 8 case-insensitive characters in the
   directory name, however SSIDs may be up to 32 arbitrary characters.
   This sample takes the reasonable approach that most SSIDs are unique
   within the first 8 characters, and do not make much use of non-alphanumeric
   characters, thus the directory names are derived from the first 8 chars
   of the SSID, forced to uppercase, and with non-alphanumeric chars converted
   to underscores.  E.g. My%ssID would translate to MY_SSID.

 Since each SSID is given its own directory, files specific to that SSID
 are stored within that directory.  For example, client certificate files may
 need to be different for each SSID, and thus the client certificates are
 stored in the appropriate SSID subdirectory.

 All other configuration items are stored in a "registry" file.  The contents
 of the current registry file may be dumped by
 pressing the 'r' key when running this sample.  Pressing the 'l' key lists
 recursively the complete directory hierarchy of the 'A' partition.

 Any item which is required but not found in the specific SSID directory
 will be obtained from the base configuration directory.  Thus, the base
 directory may be used for "default" settings, or settings which are the
 same for most SSIDs.  A good example is the trusted certificate authority
 certificate, since this is likely to be universal for any given organization.

 WPA (TKIP and/or CCMP) with pre-shared key is always supported in this
 sample.  WEP is not supported, since it is largely obsolete and considered
 insecure.

 WPA Enterprise is supported if you #define USE_EAP (either in the project
 options "defines" tab, or at the top of the code below).  Adding WPA
 Enterprise support adds a lot of code, so for initial testing you probably
 should disable it.  Similarly, you can #define INSECURE_HTTP (see below)
 to eliminate the secure web server code, but of course doing so would
 compromise a "real" application's security.


*******************************************************************************/
#class auto

#if _BOARD_TYPE_ != RCM5450W && \
    !RCM6700_SERIES && \
    !R6000TEST_SERIES
	#warns "This sample requires an RCM5450W or RCM6700"
	#warns "although it may run without USE_EAP on other boards."
	#warns "Comment out these warnings if desired."
	#warns _BOARD_TYPE_
	#fatal "<--- see previous warning"
#endif

#if SUPPRESS_FAST_RAM_COPY
	#fatal "This program must be compiled to flash."
#endif

#if __SEPARATE_INST_DATA__ && ROOT_SIZE_4K < 4U || \
    !__SEPARATE_INST_DATA__ && ROOT_SIZE_4K < 8U
	#warns "This sample requires an increase in the ROOT_SIZE_4K macro from"
	#warns "the normal default.  In the project options defines tab, put"
	#warns "ROOT_SIZE_4K = 8U"
	#warns "if using non-separate I&D option, otherwise set the value to 4U."
	#fatal "<--- see previous warning"
#endif

#if 0
	// Change the above to "#if 1" to include all the following debug and
	// diagnostic definitions.  This will probably require increases to
	// both XMEMCODE_SIZE and ROOT_SIZE_4K in the project defines box.
	// e.g. XMEMCODE_SIZE=0xA0000; ROOT_SIZE_4K=8U;
	#define HTTP_DEBUG
	#define DCRTCP_DEBUG
	//#define HTTP_VERBOSE
	//#define TLS_VERBOSE
	//#define TLS_DEBUG
	//#define SSL_CERT_VERBOSE
	//#define SSL_SOCK_VERBOSE
	#define WIFIG_DEBUG
	//#define WIFIG_VERBOSE 1
	//#define WPA_VERBOSE 1
	#define ZSERVER_DEBUG
	#define REGISTRY_DEBUG
	#define FAT_DEBUG
	#define WPA_EAP_BLOCKING
	#define WIFI_SHA1_DEBUG
#endif

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

// Define this to print extra messages
#define VERBOSE

// Define this to use ordinary HTTP (otherwise, use HTTPS)
//#define INSECURE_HTTP

// Define web server userid and password for configuration updates.  These
// are sent encrypted (and hence safely) unless INSECURE_HTTP is defined, in
// which case anyone could eavesdrop and easily find out the access code!
#define CONF_USER			"admin"
#define CONF_PASSWORD	"upload"


/*
 *  This value is required to enable FAT blocking mode for ZServer.lib
 */
#define FAT_BLOCK

// Necessary for zserver.
#define FAT_USE_FORWARDSLASH

// Define a directory in which to place certificates and related information
#define CONF_PART  "A"          // FAT partition "drive letter"
#define CONF_DIR   "wiconf"     // Configuration directory name.
										  // Don't put leading or trailing slash!
#define CONF_REG	 "reg"		  // Base name of registry file in CONF_DIR.
											// This contains settings for all SSIDs.
											// See function help for registry_prep_write()
											// for details on the registry library
											// API.

// Full Zserver resource name of configuration directory.  This will be
// "/A/wiconf" if the above defaults are used.
#define CONF_BASE      "/" CONF_PART "/" CONF_DIR

// Full pathname for registry files (except without extension, which is
// automatically maintained by the library for robustness).  Will be
// "/A/wiconf/reg" if the above defaults are used.
#define CONF_REGFILE   CONF_BASE "/" CONF_REG



/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

#ifndef INSECURE_HTTP
	// These are required for HTTPS
	#define USE_HTTP_SSL
	#define HTTP_SSL_SOCKETS   1

	// Specify format and file location of HTTPS server certificates.  This is
	// one of those "chicken and egg" things: an initial certificate is required
	// so the browser can connect securely to the HTTP server on this board,
	// however the browser will not necessarily trust this certificate.  The user
	// will typically need to tell the browser to accept this certificate in
	// spite of it not being trusted.  This is probably OK in a controlled
	// environment.  Of course, in a real application each board would
	// ideally have a unique server certificate, otherwise an attacker could
	// get hold of one board and thereby compromise all boards with the
	// same certificate.
	#ximport "samples/tcpip/ssl/https/cert/server.der" server_pub_cert
	#ximport "samples/tcpip/ssl/https/cert/serverkey.der" server_priv_key
#endif

/*
 * Web server configuration
 */

#define SSPEC_MAXNAME	64


// Need a large buffer when using HTTP uploads with SSL.  This is because SSL
// requires a complete ciphertext "record" to be accumulated in the TCP socket
// buffer, and browsers may use a record size slightly over 16k (the largest
// record allowed by the TLS specification).
// There's no getting around the fact that SSL is memory-intensive.
// The value here should be sized to allow 2MSS (2x1460) for
// transmit, and 16k+36 for receive.  Note that the Rx side is automatically
// set to the required value, and the Tx side gets the remainder.
#define HTTP_SOCK_BUF_SIZE (16384+36+2*1460)

// Make a reasonably large HTTP CGI buffer, for efficiency
#define HTTP_MAXBUFFER 512


/*
 * Only one socket and server are needed for a reserved port.  We would
 * normally specify more, but this demo uses static variables to hold some
 * state when processing HTTPS requests, thus we must serialize access by
 * allowing only one connection at a time.
 */
#define HTTP_MAXSERVERS 1				// Don't change this from 1!
#define MAX_TCP_SOCKET_BUFFERS 1
#define MAX_UDP_SOCKET_BUFFERS 1

// Assume no RTC available for checking sertificates.  Undefine this if your
// app always sets the RTC to the correct date/time, which will allow additional
// validity check of certificates.
#define X509_NO_RTC_AVAILABLE

// Save code by assuming no nameserver lookups required
#define DISABLE_DNS


/********************************
 * End of configuration section *
 ********************************/

#define USE_DHCP
#define RAND_USE_MD5
#define WIFI_USE_WPA								// Bring in WPA support
#define WIFI_AES_ENABLED                 // Enable AES specific code

#ifdef USE_EAP
	#define WPA_USE_EAP     (WPA_USE_EAP_TLS|WPA_USE_EAP_PEAP)
	#define IFC_WIFI_EAP_METHODS 0				// Initially, no EAP
#else
	// Define to 0L:        use new WPA library (more code) - must be longword!
	// Don't define at all: use old WPA library (saves code space)
	//#define WPA_USE_EAP		0L
#endif

#define TIMEZONE        0

#define USE_HTTP_UPLOAD		// Required for this demo, to include upload code.
#define HTTP_ZSSI				// Use compressed server-side processing


// Define this because we are using a static rule table.
#define SSPEC_FLASHRULES

#memmap xmem


#use "fat16.lib"		// Must use FAT before http/zserver
#use "dcrtcp.lib"
#use "http.lib"
#use "registry.lib"	// File registry organization



#ifdef USE_EAP
	#ximport "samples/tcpip/wifi/pages/cert_upload.html"    index_html
#else
	// Cut-down page for EAP-less operation (i.e. only support WPA Personal
	// with pre-shared key, not WPA Enterprise).
	#ximport "samples/tcpip/wifi/pages/cert_upload_noeap.html"    index_html
#endif


// Forward declaration of our security and network settings upload handler.
int security_CGI(HttpState * s);

// Forward dec for compressed server-side includes handler.  Need this
// because we dynamically modify the web page.
int config_ssi_handler(HttpState* state);


// Some debug print routines
void lsall(void);		// List /A recursively
int dumpfile(char * filename);

// Static var for the "current SSID" which is important for the
// web page displays.
char u_ssid[33];		// Store SSID field

// Global vars for reconfiguration signal
int config_restart;
word config_restart_timer;



// Define a group bit for updating resources...
#define ADMIN_GROUP	0x0002

// This table maps file extensions to the appropriate "MIME" type.  This is
// needed for the HTTP server.
SSPEC_MIMETABLE_START
   SSPEC_MIME_FUNC(".html", "text/html", shtml_handler),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END


// This is the access permissions "rule" table.  It associates userid
// information with files in the FAT filesystem.  This is necessary because
// FAT filesystems do not support the concept of owner userids.  Basically,
// this is a simple prefix-matching algorithm.
SSPEC_RULETABLE_START
	// You need to be in user group ADMIN_GROUP to read and write resources
   // starting with CONF_BASE.  The 2nd parameter ("newCerts") is used as
   // the resource realm, which your browser will use to prompt you for a
   // userid and password.  Only the HTTPS server can access these, and
   // basic (i.e. plaintext) user/password authentication is specified.  This
   // is adequate, since HTTPS is protecting the password from eavesdropping.
	SSPEC_MM_RULE(CONF_BASE, "newCerts", ADMIN_GROUP, ADMIN_GROUP,
	                 SERVER_HTTP|SERVER_HTTPS, SERVER_AUTH_BASIC, NULL)
SSPEC_RULETABLE_END


// The flash resource table is now initialized with these macros...
SSPEC_RESOURCETABLE_START
// "zssi" required resource name for compressed server-side includes
SSPEC_RESOURCE_P_FUNCTION("zssi", config_ssi_handler,	"newCerts", ADMIN_GROUP,
				 0x0000, SERVER_HTTP|SERVER_HTTPS, SERVER_AUTH_BASIC),
// "/index.html" - this is the web page which is initially presented.
//   It contains the form to be filled out.
SSPEC_RESOURCE_XMEMFILE("/index.html", index_html),
// "upload.cgi" - resource name as specified in HTML for handling the POST data
// security_CGI - name of the upload CGI handler
// "newPages" - the realm associated with upload.cgi
// ADMIN_GROUP - read permission must be limited to this group.  The CGI itself
//          only needs read permission, but it should be the same group bits as
//          specified for the write permission for the target resource files
//          (see ruletable entry above).
// 0x0000 - no write permission.  Writing to a CGI does not make sense,
//           only writing to files.
// SERVER_HTTPS - only the HTTPS server accesses CGIs.
// SERVER_AUTH_BASIC - should use the same authentication technique as files.
SSPEC_RESOURCE_P_CGI("upload.cgi", security_CGI, "newCerts", ADMIN_GROUP,
                       0x0000, SERVER_HTTP|SERVER_HTTPS, SERVER_AUTH_BASIC)
SSPEC_RESOURCETABLE_END

void reconfig(int iface);

#ifndef INSECURE_HTTP
// This is global, since we use it in main() and reconfig()
SSL_Cert_t my_cert;	// HTTPS server certificate
#endif

void main()
{
	int rc, i;
	char buf[20], c;
   int uid;
   word partno;

	// Unknown (default) SSID
   u_ssid[0] = 0;
   config_restart = 0;

   partno = CONF_PART[0] - 'A';	// Convert letter to a number (usually 0)

	printf("Initializing network...\n");
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	printf("Initializing filesystem...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
	// However, for this demo we also allow for the partition to be formatted
	// if not already done so.  Thus, we use the lower level fat_AutoMount()
	// before calling sspec_automount().  This allows the conditional format
	// flags to be specified, as well as only the specific partition which we
	// are interested in to be mounted.
	rc = fat_AutoMount(FDDF_MOUNT_PART_0<<partno | FDDF_MOUNT_DEV_0<<(partno>>2)
	                    | FDDF_COND_DEV_FORMAT | FDDF_COND_PART_FORMAT);
   if (rc) {
   	printf("Failed to mount FAT partition '%s', rc=%d\n", CONF_PART, rc);
   	exit(rc);
   }

   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
   if (rc) {
   	printf("Failed to mount FAT partition '%s', rc=%d\n", CONF_PART, rc);
   	exit(rc);
   }

   printf("Setting up configuration userid '%s'...\n\n", CONF_USER);
   // Create a user ID
   uid = sauth_adduser(CONF_USER, CONF_PASSWORD, SERVER_HTTP|SERVER_HTTPS);
   if (uid < 0) {
   	printf("Failed to create userid, rc=%d\n", rc);
   	exit(uid);
   }
	else {
	   // Ensure that that user has a group mask of ADMIN_GROUP i.e. this user
      // is a member of "ADMIN".
	   sauth_setusermask(uid, ADMIN_GROUP, NULL);
      // Also need to assign individual write access.
      sauth_setwriteaccess(uid, SERVER_HTTP|SERVER_HTTPS);
      printf("Userid created successfully: use '%s' with password '%s'\n\n",
                 CONF_USER, CONF_PASSWORD);
   }

#ifdef INSECURE_HTTP
   printf("Initializing HTTP (INSECURE mode)...\n\n");
   http_init();
   // Reserve the HTTP port
   tcp_reserveport(80);
   printf("Ready: point your browser to http://%s/\n\n",
               inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
#else
   printf("Initializing HTTPS...\n\n");
   http_init();
   // Reserve the HTTPS port
   tcp_reserveport(443);

   printf("Loading HTTPS server certificate...\n\n");
	if (SSL_new_cert(&my_cert, server_pub_cert, SSL_DCERT_XIM, 0) ||
	    SSL_set_private_key(&my_cert, server_priv_key, SSL_DCERT_XIM))
		exit(7);

	https_set_cert(&my_cert);
   printf("Ready: point your browser to https://%s/\n\n",
               inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
#endif

   printf("\nPress 'x' to bring down the server cleanly.\n");
   printf("      'l' to list filesystem.\n");
   printf("      'r' to dump registry.\n");

   while (1) {
      http_handler();
      if (kbhit())
      {
      	c = tolower(getchar());
      	if (c == 'x') {
	         // Cycle through all partitions and unmount as needed
	         for(i = 0; i < SSPEC_MAX_PARTITIONS; i++) {
	            // See if the partition is registered
	            if (sspec_fatregistered(i)) {
	               printf("Unmounting partition # %d\n", i);
	               // The partition was registered, lets unmount it
	               fat_UnmountDevice(sspec_fatregistered(i)->dev);
	            }
	         }
	         exit(0);
         }
         else if (c == 'l') {
         	lsall();
         }
         else if (c == 'r') {
         	// There are 3 possible files.  Usually, only one should exist.
	         dumpfile("/" CONF_DIR "/" CONF_REG ".1");
	         dumpfile("/" CONF_DIR "/" CONF_REG ".2");
	         dumpfile("/" CONF_DIR "/" CONF_REG ".3");
         }
      }
      if (config_restart && _CHK_SHORT_TIMEOUT(config_restart_timer)) {
      	config_restart = 0;
      	// OK, time to bounce interface
      	reconfig(IF_WIFI0);
      }
      if (ifpending(IF_WIFI0) == IF_DOWN) {
      	printf("Connection lost, attempting reconnect...\n");
      	ifup(IF_WIFI0);
	      while (ifpending(IF_DOWN) == IF_COMING_UP)
	         tcp_tick(NULL);
			if (ifpending(IF_WIFI0) == IF_DOWN) {
				// In a real app, we might try to fall back to the last known good
				// settings, but for now just give up.
				printf("Reconnect failed, press F9 to start from scratch.\n");
				exit(1);
			}
      }
   }
}




// Values for http_getState()
#define HTTP_DFLTUPLD_INIT		0	// Initial state, set by server (must be 0)
#define HTTP_DFLTUPLD_NORM		1	// Normal state (reading client data)
#define HTTP_DFLTUPLD_WRITING	2	// Writing uploaded data to the resource, but
                                 // resource could not accept all the data on
                                 // the last call.
#define HTTP_DFLTUPLD_PPING	3	// Computing hex key from passphrase

// Here we take advantage of the fact that we single-thread the web server.
// This allows us to use static variables to maintain current CGI state.
// If more than one server was available, then we would need to put everything
// in an array and index the array using the server number.

int spec;		// Handle to the resource being loaded (>=0) or error code (<0)
int wrote;		// Amount already written in partial write to client
int filecount;	// Counter for number of files uploaded successfully.
int errors;		// Non-zero if got any errors during processing

char fullname[256];	// Full resource name being loaded
int  file_stored;		// Whether file was written to storage

enum {
   VAR_NONE,
	VAR_SSID,
	VAR_SUB,
	VAR_NEW_SSID,
	VAR_IP,
	VAR_NETMASK,
	VAR_ROUTER,
	VAR_NAMESERVER,
	VAR_ENCR,
	VAR_METH,
	VAR_DHCP,
	VAR_DEL,
	VAR_PHRASE,
	VAR_HEXKEY,
	VAR_PEAP_USER,
	VAR_PEAP_PWD
}  which_var;			// Which variable (field) being loaded (when not a file).

char * u_var;			// pointer to variable being loaded
word   u_remlen;		// remaining length of that variable

char u_sub[16];		// Store submit button string values
char u_new_ssid[33];
char u_meth[16];		// Store methods and other radio/checkbox string values
char u_phrase[32];	// Store PSK passphrase field
char u_hexkey[65];	// Store PSK hex key field
char u_ssid_dir[9];	// SSID translated to a valid FAT directory name
char u_peap_user[64];	// Store user field
char u_peap_pwd[64];		// Store password field
char u_cert_ca[64];
char u_cert_client[64];
char u_cert_client_k[64];
char u_ip[16];
char u_netmask[16];
char u_router[16];
char u_nameserver[16];
char u_dhcp[16];				// true if DHCP checked

int query;				// true for query
int create;				// true for new SSID entry
int submit;				// true for update existing SSID entry
int del;					// true for delete existing SSID entry
int restart;			// true for restart network interface
int encr;				// Preferred encryption
int methods;			// Preferred method(s)
int dhcp;
int deletions;			// Cert file deletion requests
int got_phrase;		// 0 no, 1 yes
int got_hexkey;		// 0 no, 1 yes, 2 yes but error
char key_from_phrase[32];	// Binary key computed from passphrase
wpa_passphrase_to_psk_state pps;

int certs_loaded;		// Count of non-empty certs loaded
int certs_valid;		// Of those, how many passed basic validity check

#define MAX_SSIDS 20
int num_ssids;			// Number of SSIDs in registry
char ssid_table[MAX_SSIDS][33];


void init_data(void)
{
      // Init static vars
      spec = -1;		// No open resource yet
      filecount = 0;	// No files as yet
      errors = 0;		// Assume all OK.
      which_var = VAR_NONE;
		fullname[0] = 0;
		u_new_ssid[0] = 0;
		u_ip[0] = 0;
		u_netmask[0] = 0;
		u_router[0] = 0;
		u_nameserver[0] = 0;
		u_dhcp[0] = 0;
		u_phrase[0] = 0;
		u_hexkey[0] = 0;
		u_peap_user[0] = 0;
		u_peap_pwd[0] = 0;
		u_cert_ca[0] = 0;
		u_cert_client[0] = 0;
		u_cert_client_k[0] = 0;

		query = create = submit = del = restart = 0;
		encr = 0;
		methods = 0;
		dhcp = 0;
		deletions = 0;
		got_phrase = 0;
		got_hexkey = 0;

		certs_loaded = 0;
		certs_valid = 0;
}


// Delete SSID entry u_ssid (and directory u_ssid_dir and its contents)
int security_delete(HttpState * s)
{
	char fname[128];
	int fname_dirlen;


#ifdef VERBOSE
	printf("Deleting %s settings in %s\n", u_ssid, CONF_REGFILE);
#endif

	registry_update(CONF_REGFILE, u_ssid, NULL, http_getContext(s));


	// Now delete directory...
	strcpy(fname, u_ssid_dir);
	if (!*u_ssid_dir || u_ssid_dir[strlen(u_ssid_dir)-1] != '/')
		strcat(fname, "/");
	fname_dirlen = strlen(fname);

	// Delete all possible relevant files (doesn't matter if not found)
	strcpy(fname+fname_dirlen, "ca.pdc");
	sspec_delete(fname, http_getContext(s));
	strcpy(fname+fname_dirlen, "client.pdc");
	sspec_delete(fname, http_getContext(s));
	strcpy(fname+fname_dirlen, "client_k.pdc");
	sspec_delete(fname, http_getContext(s));
	// Finally, delete the directory itself.  May fail if there were other files
	// in that dir, but that's OK since maybe app stored them there.
	sspec_rmdir(u_ssid_dir, http_getContext(s));

	return 0;
}


// Functions to manage registry
int security_write_registry(HttpState * s)
{
	RegistryEntry rtab[20];
	int rc;

	rc = 0;
	if (u_ip[0]) {
	   rtab[rc].key = "ip";
	   rtab[rc].value = u_ip;
	   rtab[rc++].options = REGOPTION_STRING(sizeof(u_ip));
	}
	if (u_netmask[0]) {
	   rtab[rc].key = "netmask";
	   rtab[rc].value = u_netmask;
	   rtab[rc++].options = REGOPTION_STRING(sizeof(u_netmask));
	}
	if (u_router[0]) {
	   rtab[rc].key = "router";
	   rtab[rc].value = u_router;
	   rtab[rc++].options = REGOPTION_STRING(sizeof(u_router));
	}
	if (u_nameserver[0]) {
	   rtab[rc].key = "nameserver";
	   rtab[rc].value = u_nameserver;
	   rtab[rc++].options = REGOPTION_STRING(sizeof(u_nameserver));
	}
	if (got_phrase || got_hexkey) {
		rtab[rc].key = "hexkey";
		rtab[rc].value = u_hexkey;
		rtab[rc++].options = REGOPTION_BIN(32);	// Binary length
	}
	if (u_peap_user[0]) {
	   rtab[rc].key = "peap_user";
	   rtab[rc].value = u_peap_user;
	   rtab[rc++].options = REGOPTION_STRING(sizeof(u_peap_user));
	}
	if (u_peap_pwd[0]) {
	   rtab[rc].key = "peap_pwd";
	   rtab[rc].value = u_peap_pwd;
	   rtab[rc++].options = REGOPTION_STRING(sizeof(u_peap_pwd));
	}
	if (u_cert_ca[0] || deletions & 1) {
	   rtab[rc].key = "ca";
	   rtab[rc].value = u_cert_ca;
	   rtab[rc++].options = u_cert_ca[0] ? REGOPTION_STRING(sizeof(u_cert_ca)) :
                                          REGOPTION_DELETE;
	}
	if (u_cert_client[0] || deletions & 2) {
	   rtab[rc].key = "client";
	   rtab[rc].value = u_cert_client;
	   rtab[rc++].options = u_cert_client[0] ?
                  REGOPTION_STRING(sizeof(u_cert_client)) : REGOPTION_DELETE;
	}
	if (u_cert_client_k[0] || deletions & 4) {
	   rtab[rc].key = "client_k";
	   rtab[rc].value = u_cert_client_k;
	   rtab[rc++].options = u_cert_client_k[0] ?
                  REGOPTION_STRING(sizeof(u_cert_client_k)) : REGOPTION_DELETE;
	}
	rtab[rc].key = "encr";
	rtab[rc].value = &encr;
	rtab[rc++].options = REGOPTION_SHORT;
	rtab[rc].key = "methods";
	rtab[rc].value = &methods;
	rtab[rc++].options = REGOPTION_SHORT;

	rtab[rc].key = "dhcp";
	rtab[rc].value = &dhcp;
	rtab[rc++].options = REGOPTION_SHORT;


	rtab[rc++].options = REGOPTION_EOL;

#ifdef VERBOSE
	printf("Saving new settings to %s\n", CONF_REGFILE);
#endif
	return registry_update(CONF_REGFILE, u_ssid, rtab, http_getContext(s));

}

int reg_enum_callback(void far * ptr, int new_sect, char * sect, char far * key,
                       char far * value)
{
	if (new_sect) {
		if (num_ssids >= MAX_SSIDS)
			return 1;
		strncpy(ssid_table[num_ssids], sect, 32);
		++num_ssids;
	}
	return 0;
}

int security_read_registry(HttpState * s)
{
	RegistryEntry rtab[20];
	int rc;

	init_data();

	rc = 0;

   // read as an ascii hex string
	rtab[rc].key = "ip";
	rtab[rc].value = u_ip;
	rtab[rc++].options = REGOPTION_STRING(sizeof(u_ip));

   // read as an ascii hex string
	rtab[rc].key = "netmask";
	rtab[rc].value = u_netmask;
	rtab[rc++].options = REGOPTION_STRING(sizeof(u_netmask));

   // read as an ascii hex string
	rtab[rc].key = "router";
	rtab[rc].value = u_router;
	rtab[rc++].options = REGOPTION_STRING(sizeof(u_router));

   // read as an ascii hex string
	rtab[rc].key = "nameserver";
	rtab[rc].value = u_nameserver;
	rtab[rc++].options = REGOPTION_STRING(sizeof(u_nameserver));

   // read as an ascii hex string
	rtab[rc].key = "hexkey";
	rtab[rc].value = u_hexkey;
	rtab[rc++].options = REGOPTION_STRING(sizeof(u_hexkey));

   // read as an ascii hex string
	rtab[rc].key = "hexkey";
	rtab[rc].value = u_hexkey;
	rtab[rc++].options = REGOPTION_STRING(sizeof(u_hexkey));

   rtab[rc].key = "peap_user";
   rtab[rc].value = u_peap_user;
   rtab[rc++].options = REGOPTION_STRING(sizeof(u_peap_user));

   rtab[rc].key = "peap_pwd";
   rtab[rc].value = u_peap_pwd;
   rtab[rc++].options = REGOPTION_STRING(sizeof(u_peap_pwd));

   rtab[rc].key = "ca";
   rtab[rc].value = u_cert_ca;
   rtab[rc++].options = REGOPTION_STRING(sizeof(u_cert_ca));

   rtab[rc].key = "client";
   rtab[rc].value = u_cert_client;
   rtab[rc++].options = REGOPTION_STRING(sizeof(u_cert_client));

   rtab[rc].key = "client_k";
   rtab[rc].value = u_cert_client_k;
   rtab[rc++].options = REGOPTION_STRING(sizeof(u_cert_client_k));

	rtab[rc].key = "encr";
	rtab[rc].value = &encr;
	rtab[rc++].options = REGOPTION_SHORT;

	rtab[rc].key = "methods";
	rtab[rc].value = &methods;
	rtab[rc++].options = REGOPTION_SHORT;

	rtab[rc].key = "dhcp";
	rtab[rc].value = &dhcp;
	rtab[rc++].options = REGOPTION_SHORT;

	rtab[rc++].options = REGOPTION_EOL;

	num_ssids = 0;
	return registry_get(CONF_REGFILE, u_ssid, rtab,
							http_getContext(s),
							reg_enum_callback, 0, NULL);

}



int security_CGI(HttpState * s)
{
#ifndef INSECURE_HTTP
	auto SSL_Cert_t test_cert;
#endif
	auto SSpecStat stat;
	auto char * fname;		// Pointer to field (i.e. resource to upload) name
	auto char * p;
   auto int rc, len, mode, wlen, i, j;
	static long ms;			// In verbose mode, keep track of throughput.

	mode = http_getState(s);

	if (!mode) {
   	// Initial state (HTTP_DFLTUPLD_INIT).  Do things we want to do only
      // once at the start.
#ifdef VERBOSE
      printf("CGI: init\n");
#endif
   	// First time through.  Generate HTTP header
      http_setState(s, HTTP_DFLTUPLD_NORM);		// Next state is "normal"
      // Note: it is safe to use the getData buffer, since the first call to
      // the CGI will not have any incoming data in that buffer.
	   http_genHeader(s, http_getData(s), HTTP_MAXBUFFER, 200, NULL, 0,
           "<html><head><title>Update/Create Status</title></head><body>\r\n");

		u_ssid[0] = 0;

		// Zero all static data
		init_data();

		// Make sure there is a directory called CONF_BASE (this is harmless
		// if it already exists).
		rc = sspec_mkdir(CONF_BASE, http_getContext(s));
#ifdef VERBOSE
		printf("Initial mkdir %s return code %d\n", (CONF_BASE), rc);
#endif

      return CGI_SEND;	// Write string to client.
   }

   // Default return code.
   rc = 0;

   // Main switch depending on the current CGI action code
	switch (http_getAction(s)) {

   	case CGI_CONTINUE:
      	// Continuation because we returned CGI_MORE on the last call.
      	if (mode == HTTP_DFLTUPLD_WRITING) {
            // We were writing buffer to resource
            http_setState(s, HTTP_DFLTUPLD_NORM);  // Set back to normal mode
            len = wrote; // Get length already written
            // Write the bit we couldn't write before.
            wlen = sspec_write(spec, http_getData(s)+len,
                                 http_getDataLength(s)-len);
            if (wlen < 0)
               goto _closeError;
            len += wlen;
            if (len < http_getDataLength(s))
            	// Still not complete.
	            goto _partialwrite;
         }
         else if (mode == HTTP_DFLTUPLD_PPING) {
         	// Computing the hexkey from the passphrase, bit by bit.
				if (wpa_passphrase_to_psk_run(&pps, 10))
         		return CGI_MORE;
            http_setState(s, HTTP_DFLTUPLD_NORM);  // Set back to normal mode
            got_phrase = 1;
	#ifdef VERBOSE
			   printf("Done generating key from passphrase\n");
	#endif
            if (got_hexkey && memcmp(u_hexkey, key_from_phrase, 32)) {
               sprintf(http_getData(s),
                 "<h2><font color=red>WARNING: Hex key did not match " \
                 "passphrase result</font></h2>" \
                 "<p>Will use the passphrase result for this SSID.</p>"
                 );
               memcpy(u_hexkey, key_from_phrase, 32);
   			}
            else {
            	if (!got_hexkey)
               	memcpy(u_hexkey, key_from_phrase, 32);
               sprintf(http_getData(s),
                 "<h1>Passphrase OK</h1>"
                 );
            }
            return CGI_SEND;

         }
         	// any other mode we ignore (should never happen)
      	break;

   	case CGI_START:
      	// Start of a new part of the multi-part data.
      	fname = http_getField(s);
#ifdef VERBOSE
      	printf("CGI_START field name=%s\n", fname);
         ms = MS_TIMER;
#endif
			if (query || del || restart) {
	         http_skipCGI(s);  // Skip all parts for query/del/restart
	         break;
			}
			if (!strcmp(fname, "ca") && (u_var = u_cert_ca) ||
			    !strcmp(fname, "client") && (u_var = u_cert_client) ||
			    !strcmp(fname, "client_k") && (u_var = u_cert_client_k)) {
				// OK, is one of the files we want.
				file_stored = 0;
				which_var = VAR_NONE;
				strcpy(fullname, CONF_BASE);
				strcat(fullname, "/");
				if (u_ssid_dir[0]) {
#ifdef VERBOSE
					printf("Checking subdirectory %s...\n", u_ssid_dir);
#endif
					strcat(fullname, u_ssid_dir);
					// Create subdirectory if not exists, or check that it's a
					// directory if it does.
					rc = sspec_stat(fullname, http_getContext(s), &stat);
#ifdef VERBOSE
					printf("stat %s returns %d\n", fullname, rc);
#endif
					if (rc == -ENOENT) {
						// Create directory
						rc = sspec_mkdir(fullname, http_getContext(s));
#ifdef VERBOSE
						printf("mkdir %s returns %d\n", fullname, rc);
#endif
						if (rc < 0) {
	                  sprintf(http_getData(s),
	                    "<h2><font color=red>%s failed (for %s): could not " \
                       "create directory.</font></h2>" \
	                    "<h3>Error code %d</h3>\r\n"
	                    , fullname
	                    , fname
	                 	  , -rc
	                    );
	                  http_skipCGI(s);  // Skip to next part
	                  ++errors;
	                  return CGI_SEND;     // ...after sending this buffer
						}
					}
					else if (rc < 0) {
	               sprintf(http_getData(s),
	                 "<h2><font color=red>%s failed (for %s): could not stat " \
                    "directory.</font></h2>" \
               	  "<h3>Error code %d</h3>\r\n"
	                 , fullname
	                 , fname
	                 , -rc
	                 );
	               http_skipCGI(s);  // Skip to next part
                  ++errors;
	               return CGI_SEND;     // ...after sending this buffer
					}
					else if (!sspec_is_directory(fullname)) {
#ifdef VERBOSE
						printf("%s is not a directory\n", fullname);
#endif
	               sprintf(http_getData(s),
	                 "<h2><font color=red>%s failed (for %s): is not a " \
                    "directory.</font></h2>\r\n"
	                 , fullname
	                 , fname
	                 );
	               http_skipCGI(s);  // Skip to next part
	               ++errors;
	               return CGI_SEND;     // ...after sending this buffer
					}
				}

				// OK, directory exists so construct full resource name
				if (fullname[strlen(fullname)-1] != '/')
					strcat(fullname, "/");
				strcat(fullname, fname);
				// Add an extension so as to always distinguish files from
				// directories .pdc stands for "PEM or DER Certificate".
				strcat(fullname, ".pdc");

				// If client filename was blank, do not touch any existing file.
				// Unfortunately, browsers seem not to bother to compute the
            // individual file content lengths.  It would be better if we could
            // test the content length for zero, but we are reduced to this.
            // An alternative would be to write the content to a temporary file,
            // and only move it if valid at the end.
				if (!http_getFileName(s)[0]) {
#ifdef VERBOSE
		 			printf("%s skipped, no client file name\n", fullname);
#endif
					http_skipCGI(s);
					break;
				}

				spec = sspec_open(fullname, http_getContext(s),
                                O_WRITE|O_CREAT|O_TRUNC, 0);
#ifdef VERBOSE
		 		printf("open %s returns %d\n", fullname, spec);
#endif
            if (spec < 0) {
      			sprintf(http_getData(s),
                 "<h2><font color=red>%s failed: could not open.</font></h2>" \
                 "<h3>Error code %d</h3>\r\n"
                 , fullname
                 , -spec
                 );
               http_skipCGI(s);	// Skip to next part
               ++errors;
               return CGI_SEND;		// ...after sending this buffer
      		}
            else {
            	++certs_loaded;
            	// Try writing a string to the client.  Most browsers will be
               // able to display this straight away.  This will give some
               // confirmation that something is happening.
	            sprintf(http_getData(s),
	              "<h2>Loading %s...</h2>\r\n"
	              , fullname
	              );
	            return CGI_SEND;
            }
         }
         else if (!strcmp(fname, "ssid")) {
         	which_var = VAR_SSID;
            u_var = u_ssid;
            u_remlen = sizeof(u_ssid)-1;
         }
         else if (!strcmp(fname, "new_ssid")) {
         	which_var = VAR_NEW_SSID;
            u_var = u_new_ssid;
            u_remlen = sizeof(u_new_ssid)-1;
         }
         else if (!strcmp(fname, "sub")) {
         	which_var = VAR_SUB;
            u_var = u_sub;
            u_remlen = sizeof(u_sub)-1;
         }
         else if (!strcmp(fname, "encr")) {
         	which_var = VAR_ENCR;
            u_var = u_meth;
            u_remlen = sizeof(u_meth)-1;
         }
         else if (!strcmp(fname, "method")) {
         	which_var = VAR_METH;
            u_var = u_meth;
            u_remlen = sizeof(u_meth)-1;
         }
         else if (!strcmp(fname, "dhcp")) {
         	which_var = VAR_DHCP;
            u_var = u_dhcp;
            u_remlen = sizeof(u_dhcp)-1;
         }
         else if (!strcmp(fname, "delete")) {
         	which_var = VAR_DEL;
            u_var = u_meth;
            u_remlen = sizeof(u_meth)-1;
         }
         else if (!strcmp(fname, "passphrase")) {
         	which_var = VAR_PHRASE;
            u_var = u_phrase;
            u_remlen = sizeof(u_phrase)-1;
         }
         else if (!strcmp(fname, "hexkey")) {
         	which_var = VAR_HEXKEY;
            u_var = u_hexkey;
            u_remlen = sizeof(u_hexkey)-1;
         }
         else if (!strcmp(fname, "peap_user")) {
         	which_var = VAR_PEAP_USER;
            u_var = u_peap_user;
            u_remlen = sizeof(u_peap_user)-1;
         }
         else if (!strcmp(fname, "peap_pwd")) {
         	which_var = VAR_PEAP_PWD;
            u_var = u_peap_pwd;
            u_remlen = sizeof(u_peap_pwd)-1;
         }
         else if (!strcmp(fname, "ip")) {
         	which_var = VAR_IP;
            u_var = u_ip;
            u_remlen = sizeof(u_ip)-1;
         }
         else if (!strcmp(fname, "netmask")) {
         	which_var = VAR_NETMASK;
            u_var = u_netmask;
            u_remlen = sizeof(u_netmask)-1;
         }
         else if (!strcmp(fname, "router")) {
         	which_var = VAR_ROUTER;
            u_var = u_router;
            u_remlen = sizeof(u_router)-1;
         }
         else if (!strcmp(fname, "nameserver")) {
         	which_var = VAR_NAMESERVER;
            u_var = u_nameserver;
            u_remlen = sizeof(u_nameserver)-1;
         }
         else
         	http_skipCGI(s);	// Not a file: skip to next part.
         break;

   	case CGI_DATA:
   		if (which_var) {
   			// Getting data for one of the variables we know about (not a file)
				rc = http_getDataLength(s);
				if (rc > u_remlen)
					rc = u_remlen;
				_f_memcpy(u_var, http_getData(s), rc);
				u_remlen -= rc;
				u_var += rc;
				return 0;
   		}
      	// This is data to write to the resource.
         if (spec < 0)
         	// This should never happen (since we always skip data if there is
            // an error opening the resource), but it doesn't hurt to check!
         	return 0;

      	len = sspec_write(spec, http_getData(s), http_getDataLength(s));
         if (len < 0) {
         	_closeError:
         		sspec_close(spec);
               spec = len;	// Save error code
      			sprintf(http_getData(s),
                 "<h2><font color=red>%s failed: could not write.</font></h2>" \
                 "<h3>Error code %d</h3>\r\n"
                 , http_getField(s)
                 , -len
                 );
               http_skipCGI(s);	// Skip to next part
               ++errors;
               return CGI_SEND;	// ...after sending this message
         }
         else {
         	if (len < http_getDataLength(s)) {
            _partialwrite:
            	// Partial write.  Set state to HTTP_DFLTUPLD_WRITING and return
               // CGI_MORE. Next call will have action code set to CGI_CONTINUE.
               http_setState(s, HTTP_DFLTUPLD_WRITING);
               wrote = len;	// Amount already written
               rc = CGI_MORE;
            }
         }
         break;

   	case CGI_END:
   		if (query || del || restart)
   			break;
   		if (which_var) {
   			// End of data for one of the variables we know about (not a file)
   			*u_var = 0;
   			if (which_var == VAR_SSID) {
   				// Need to condense SSID to valid 8-char directory name
   			_setdirname:
   				memcpy(u_ssid_dir, u_ssid, 8);
   				u_ssid_dir[8] = 0;
   				u_var = u_ssid_dir;
   				while (*u_var) {
   					if (!isalnum(*u_var)) *u_var = '_';
   					++u_var;
   				}
   			}
   			else if (which_var == VAR_NEW_SSID) {
   				// This field only used if creating, in which case none of the
   				// above flags will be set because of the form order.
   				// If creating new SSID entry, copy it to u_ssid and use as such.
   				if (!submit) {
   					strcpy(u_ssid, u_new_ssid);
   					goto _setdirname;
   				}
   			}
   			else if (which_var == VAR_SUB) {
   				if (u_sub[0] == 'Q')
   					query = 1;
   				else if (u_sub[0] == 'C')
   					create = 1;
   				else if (u_sub[0] == 'U')
   					submit = 1;
   				else if (u_sub[0] == 'D')
   					del = 1;
   				else if (u_sub[0] == 'R')
   					restart = 1;
   			}
   			else if (which_var == VAR_ENCR) {
   				encr |= atoi(u_meth);
   			}
   			else if (which_var == VAR_METH) {
   				methods |= atoi(u_meth);
   			}
   			else if (which_var == VAR_DHCP) {
   				dhcp |= atoi(u_dhcp);
   			}
   			else if (which_var == VAR_DEL) {
   				deletions |= atoi(u_meth);
   				// This depends on getting the checkbox in the correct order i.e.
   				// immediately after the file concerned.
   				if (!file_stored && atoi(u_meth))
   					goto _deletefile;
   			}
   			else if (which_var == VAR_HEXKEY && u_hexkey[0]) {
   				// First convert hex to binary and store in-place.
   				got_hexkey = 1;
					if (strlen(u_hexkey) != 64) {
	               sprintf(http_getData(s),
	               "<h2><font color=red>Hex key was not 64 characters</font></h2>"
	                 );
	               got_hexkey = 2;
                  ++errors;
	               return CGI_SEND;
					}
         		u_var = p = u_hexkey;
	            for (i = 0; i < 32; i++) {
	               j = hexstrtobyte (u_var);
	               if (j == -1) {
	                  sprintf(http_getData(s), "<h2><font color=red>" \
	                    "Hex key contained invalid character</font></h2>"
	                    );
	               	got_hexkey = 2;
	                  ++errors;
	                  return CGI_SEND;
	               }
	               *p++ = j;
	               u_var += 2;
	            }
   			}
   			else if (which_var == VAR_PHRASE && u_phrase[0] && u_ssid[0]
                       && got_hexkey != 2)
            {
               sprintf(http_getData(s),
                 "<p>Calculating key from passphrase.  Please wait...\r\n</p>"
                 );
            	wpa_passphrase_to_psk_init(&pps, u_phrase,
                                   u_ssid, strlen(u_ssid), key_from_phrase);
            	http_setState(s, HTTP_DFLTUPLD_PPING);
            	return CGI_SEND_MORE;
   			}
   			break;
   		}
      	// End of this file.  Close it.
#ifdef VERBOSE
			ms = MS_TIMER - ms;
         if (ms < 1) ms = 1;
      	printf("CGI: END --- actual received length=%ld, %f sec, %f bytes/sec\n",
         	http_getContentLength(s), 0.001*ms,
            (float)http_getContentLength(s)*1000.0/ms);
#endif
     		sspec_close(spec);
      	spec = -1;	// No open resource now
			if (http_getContentLength(s)) {
	         // Check certificate for validity.  fullname is a static, so it is
	         // still a valid resource name of the file we just loaded.
	         // [We only do this if not INSECURE mode, otherwise, we don't have
            // these APIs available]
#ifndef INSECURE_HTTP
	#ifdef VERBOSE
            printf("checking validity of %s\n", fullname);
	#endif
				if (!strcmp(http_getField(s), "client_k")) {
					// This should be a key file; the others are certificates
	#ifdef VERBOSE
            	printf("...is a key\n");
	#endif
					rc = SSL_set_private_key(NULL, (long)(char far *)fullname,
                                         SSL_DCERT_Z);
				}
				else {
	#ifdef VERBOSE
            	printf("...is a certificate\n");
	#endif
	            memset(&test_cert, 0, sizeof(test_cert));
	            rc = SSL_new_cert(&test_cert, (long)(char far *)fullname,
                                    SSL_DCERT_Z, 0);
	            SSL_free_cert(&test_cert);
				}
				if (rc) {
      			sprintf(http_getData(s),
                 "<h2><font color=red>%s: failed basic validity test.</font>" \
                 "</h2><h3>Error code %d</h3>\r\n"
                 , fullname
                 , rc
                 );
               ++errors;
               return CGI_SEND;
				}
				else {
					++certs_valid;
		     		file_stored = 1;
		     	}

#else
				// Just assume valid
				++certs_valid;
     			file_stored = 1;
#endif
			}
			else
				// Turned out to be a deletion
				--certs_loaded;

			if (file_stored)
				// Do this so the resource name of the cert gets stored in registry
				strcpy(u_var, fullname);

         // Increment successful file count.
         ++filecount;
         if (http_getContentLength(s)) {
	         sprintf(http_getData(s),
	           "<h2><font color=#009900>%s uploaded successfully.</font></h2>" \
	           "<h3>Total bytes: %ld.</h3>\r\n"
	           , http_getField(s)
	           , http_getContentLength(s)
	           );
         }
         else {
         _deletefile:
            // No content length provided.  Is either an empty file or no file
            // was specified in the form.  For this demo, we delete the
            // corresponding file in this case.
            rc = sspec_delete(fullname, http_getContext(s));
#ifdef VERBOSE
            printf("delete %s returns %d\n", fullname, spec);
#endif
            sprintf(http_getData(s),
              "<h2><font color=purple>%s deleted (rc=%d).</font></h2>\r\n"
              , fullname
              , -rc
              );
         }

         return CGI_SEND;

      case CGI_EOF:
      	// Normal end of entire set of parts.
#ifdef VERBOSE
      	printf("CGI: EOF (unused content=%ld) for SSID=%s\n",
                       s->content_length, u_ssid);
#endif
			if (query) {
				sprintf(http_getData(s),
					"<h2>Query SSID %s successful</h2>" \
					"<h3><a href=\"/index.html\">Continue...</a></h3>"
					, u_ssid);
				return CGI_SEND_DONE;
			}
			if (del) {
				rc = security_delete(s);
				sprintf(http_getData(s),
					"<h2>Delete SSID %s successful</h2>" \
					"<h3><a href=\"/index.html\">Continue...</a></h3>"
					, u_ssid);
				return CGI_SEND_DONE;
			}
			if (restart) {
				if (!u_ssid[0])
					// Cannot restart with default ssid selected
      			sprintf(http_getData(s),
               "<h2><font color=red>Cannot restart if default SSID selected</" \
					"font></h2><h3><a href=\"/index.html\">Go back...</a></h3>\r\n");
				else {
	            sprintf(http_getData(s),
	               "<h2>Restart WiFi interface with SSID %s will commence in 2" \
	               " seconds</h2><h3><a href=\"/index.html\">Continue...</a>" \
	               "</h3>\r\n<p>Note that the above link may not respond if " \
                  "the URL changes.</p>" , u_ssid);
	            // Set a flag for the main loop to process this
	            config_restart = 1;
	            config_restart_timer = _SET_SHORT_TIMEOUT(2000);
	            security_read_registry(s);
	         }
				return CGI_SEND_DONE;
			}

			len = 0;
			if (!errors) {
				// No errors, save changed settings in "registry".
				rc = security_write_registry(s);
				if (rc < 0) {
					++errors;
      			sprintf(http_getData(s),
                 "<h2><font color=red>Failed to update registry</font></h2>" \
                 "<h3>Error code %d</h3>\r\n"
                 , rc
                 );
               len = strlen(http_getData(s));
				}
			}

         sprintf(http_getData(s) + len,
           "<h2>%d file(s) out of %d OK, %d errors encountered</h2>" \
           "%s" \
           "<h3><a href=\"/index.html\">Continue...</a></h3>" \
           "</body></html>\r\n"
           , certs_valid
           , certs_loaded
           , errors
           , errors ?
           		"<p><font color=red>Since at least one error was encountered, " \
               "press your browser's BACK button to correct the errors, then " \
               "resubmit.</font></p>" :
           		""
           );

         // Send this last message, and we are done!
         return CGI_SEND_DONE;

      case CGI_ABORT:
      	// Server had to abort the connection.
#ifdef VERBOSE
      	printf("HTTPU: ABORT CGI\n");
#endif
			// Clean up resource if one was open.
			if (spec >= 0)
         	sspec_close(spec);
         break;
   }

   // Return one of
   //  0 : normal continuation
   //  CGI_MORE : call back with CGI_CONTINUE since we hadn't finished something
   //  CGI_DONE : terminate the connection normally.
   //  CGI_SEND : send what's in the buffer
   //  CGI_SEND_DONE : send what's in the buffer, and we're done.
   return rc;
}






char pwd[257];
int active_part;
FATfile  file[5];


// Print out recursive directory listing of static var 'pwd'
int lslr(int level)
{
	int rc;
   fat_dirent dent;
   char fname[13];
   int del, lent, pwdlen, ilevel;

	ilevel = level<<2;	// indent level
   pwdlen = strlen(pwd);

   // Open directory to get a file handle
	rc = fat_Open(fat_part_mounted[active_part], pwd, FAT_DIR, 0,
	              file+level, NULL);
   if (rc) {
   	printf("Open Directory '%s' Error: %ls\n",
                           pwd, strerror(rc));
      return rc;
   }
   if (!level)
   	printf("Recursive listing from %s:\n", pwd);

   del = lent = 0;
	for (;;) {
   	// Use fat_ReadDir to read directory entries into structure 'dent'
   	rc = fat_ReadDir(file+level, &dent, FAT_INC_DEF + FAT_INC_DEL );
      if (rc)
      	break;
		if (!dent.name[0])
      	break;
      else if (dent.name[0] == 0xE5)
      	++del;
      else if ((dent.attr & FATATTR_LONG_NAME) == FATATTR_LONG_NAME)
      	++lent;
      else {
			// Looks OK
         fat_GetName(&dent, fname, FAT_LOWERCASE);
         if (fname[0] != '.') {
	         printf("%*.*s%-12.12s %c%c%c%c%c%c len=%lu\n",
	            ilevel, ilevel, "",fname,
	            dent.attr & FATATTR_READ_ONLY ? 'R' : ' ',
	            dent.attr & FATATTR_HIDDEN ? 'H' : ' ',
	            dent.attr & FATATTR_SYSTEM ? 'S' : ' ',
	            dent.attr & FATATTR_VOLUME_ID ? 'V' : ' ',
	            dent.attr & FATATTR_DIRECTORY ? 'D' : ' ',
	            dent.attr & FATATTR_ARCHIVE ? 'A' : ' ',
	            dent.fileSize);
	         if (dent.attr & FATATTR_DIRECTORY && level < 4) {
	            // Now recurse to sub-directory
	            strcat(pwd, fname);
	            strcat(pwd, "/");
	            lslr(level+1);
	            pwd[pwdlen] = 0;
	         }
	      }
      }
   }
   fat_Close(file+level);
   return rc;
}


void lsall(void)
{
	active_part = 0;
	strcpy(pwd, "/");
	lslr(0);
}


int dumpfile(char * filename)
{
	auto char b[81];
	int rc;
   int len;
   long red;
   int ltr;

   printf("\n------ %s --------\n", filename);

   // Open file to get a file handle
   rc = fat_Open(fat_part_mounted[0], filename, FAT_FILE,0, file, NULL);
   if (rc) {
   	printf("Open '%s' failed rc=%d\n", filename, rc);
      return rc;
   }
   red = 0;
   rc = 0;
   b[80] = 0;
   // Read in 80 characters at a time until EOF.
   for (;;) {
     	ltr = sizeof(b) - 1;
      rc = fat_Read(file, b, ltr);
      if (rc < 0)
      	break;
      b[rc] = 0;
  	   printf("%s", b);
     	red += rc;
   }
  	if (rc == -EEOF) {
      if (!red)
	     	printf("'%s' has no data.\n", filename);
   }
   else
	   if (rc < 0)
   		printf("Read Error: %ls\n", strerror(rc));
   printf("\nRead %ld bytes\n", red);
   printf("------ %s --------\n\n", filename);
	return fat_Close(file);
}



int config_ssi_handler(HttpState* state)
{
	auto long L;

	state->headeroff = 0;
	state->headerlen = 0;
	state->buffer[0] = 0;

	switch (state->substate) {
   case 0:
   	switch (state->subsubstate) {
   		case 0:
   			// This is a dummy state just for initializing all the other stuff
   			// The <<0,0>> symbol should appear near the start of each page.
   			security_read_registry(state);
   			state->cond[0] = 0;	// Initialize iterator
   			break;
      	case 1:	// new SSID...
				_f_strcpy(state->buffer, u_ssid);
            break;
         case 2: // list of SSIDs
         	// This requires multiple iterations to complete
         	if (!state->cond[0])
         		sprintf(state->buffer,
                  "<option value=\"\"%s>[defaults]</option>\r\n",
         			u_ssid[0] ? "" : " selected=\"selected\"");
         	else if (state->cond[0] <= num_ssids)
					sprintf(state->buffer, "<option value=\"%s\"%s>%s</option>\r\n",
						ssid_table[state->cond[0]-1],
						strcmp(ssid_table[state->cond[0]-1], u_ssid) ? "" :
                      " selected=\"selected\"", ssid_table[state->cond[0]-1]);
				else
					break;
				++state->cond[0];
				state->headerlen = strlen(state->buffer);
         	return 0;	// send and come back
      }
   	break;

   case 1:
   	// State 1 is checkbox value for each of the method flags.  We put either
   	// nothing or checked="checked".
   	if (methods & state->subsubstate)
   		_f_strcpy(state->buffer, "checked=\"checked\"");
      break;

   case 3:
   	// State 3 is radio button value for the selected encryption method.
   	// We put either nothing or checked="checked".
   	if (!encr)
   		encr = 1;	// Force at least one button to be checked
                     //  ('1' means "no encryption")
   	if (encr & state->subsubstate)
   		_f_strcpy(state->buffer, "checked=\"checked\"");
      break;

   case 2:
   	// State 2 for other security related fields...
   	switch (state->subsubstate) {
   		case 0:	// hexkey
   			_f_strcpy(state->buffer, u_hexkey);
   			break;
      	case 1:	// peap_user
				_f_strcpy(state->buffer, u_peap_user);
            break;
      	case 2:	// peap_pwd
				_f_strcpy(state->buffer, u_peap_pwd);
            break;
      	case 3:	// CA cert.
      		// The cert entries should be in the form
      		// <li>Remote name: /A/MYSSID/ca</li>
      		if (u_cert_ca[0])
					sprintf(state->buffer, "<li>Remote name: %s</li>\r\n", u_cert_ca);
            break;
      	case 4:	// Client cert.
      		if (u_cert_client[0])
					sprintf(state->buffer, "<li>Remote name: %s</li>\r\n",
                         u_cert_client);
            break;
      	case 5:	// Client key.
      		if (u_cert_client_k[0])
					sprintf(state->buffer, "<li>Remote name: %s</li>\r\n",
                         u_cert_client_k);
            break;
         case 10:	// IP addr
				_f_strcpy(state->buffer, u_ip);
         	break;
         case 11:	// netmask
				_f_strcpy(state->buffer, u_netmask);
         	break;
         case 12:	// router
				_f_strcpy(state->buffer, u_router);
         	break;
         case 13:	// nameserver
				_f_strcpy(state->buffer, u_nameserver);
         	break;
         case 14:	// dhcp
   			if (dhcp)
   				_f_strcpy(state->buffer, "checked=\"checked\"");
         	break;
      }
   	break;
   }

	state->headerlen = strlen(state->buffer);
	return 1;
}



void reconfig(int iface)
{
	char buf[20];
	word tmo;

	printf("Restarting interface using SSID %s\n", u_ssid);
	printf("IP=%s, netmask=%s, router=%s, nameserver=%s, dhcp=%d\n",
		u_ip, u_netmask, u_router, u_nameserver, dhcp);
	printf("Encr=%d, methods=%d\n",
		encr, methods);

	http_shutdown(0);

	ifdown(iface);
	while (ifpending(iface) == IF_COMING_DOWN)
		tcp_tick(NULL);


	ifconfig(iface,
		IFS_DHCP, dhcp,
		IFS_IPADDR, inet_addr(u_ip),
		IFS_NETMASK, inet_addr(u_netmask),
		IFS_ROUTER_SET, inet_addr(u_router),
		IFS_NAMESERVER_SET, inet_addr(u_nameserver),
		IFS_WIFI_SSID, strlen(u_ssid), u_ssid,
		IFS_WIFI_ENCRYPTION,
			encr & 4 ? IFPARAM_WIFI_ENCR_CCMP :
			encr & 2 ? IFPARAM_WIFI_ENCR_TKIP :
			           IFPARAM_WIFI_ENCR_NONE,
		IFS_WIFI_AUTHENTICATION,
#ifdef USE_EAP
			methods & 6 ? IFPARAM_WIFI_AUTH_WPA_8021X :
#endif
			encr & 6 ? IFPARAM_WIFI_AUTH_WPA_PSK :
			           IFPARAM_WIFI_AUTH_OPEN,
		IFS_WIFI_WPA_PSK_HEXSTR, u_hexkey,
		IFS_END);

#ifdef USE_EAP
	ifconfig(iface,
		IFS_WIFI_EAP_METHODS,
			(methods & 4 ? WPA_USE_EAP_TLS : 0) |
			(methods & 2 ? WPA_USE_EAP_PEAP : 0),
		IFS_WIFI_IDENTITY, u_peap_user,
		IFS_WIFI_PASSWORD, u_peap_pwd,
		IFS_WIFI_WPA_PROTOCOL,
			encr & 4 ? IFPARAM_WIFI_WPA_PROTOCOL_RSN :
			           IFPARAM_WIFI_WPA_PROTOCOL_ALL,
		IFS_WIFI_CA_CERT_PATH,
			u_cert_ca[0] ? u_cert_ca : NULL,
		IFS_WIFI_CLIENT_CERT_PATH,
			u_cert_client[0] ? u_cert_client : NULL,
			u_cert_client_k[0] ? u_cert_client_k :
				u_cert_client[0] ? u_cert_client : NULL,
		IFS_END);

#endif


	ifup(iface);
	printf("Waiting for interface to come back up...\n");
	tmo = _SET_SHORT_TIMEOUT(10000);
	while (ifpending(iface) == IF_COMING_UP) {
		tcp_tick(NULL);
		if (_CHK_SHORT_TIMEOUT(tmo)) {
			printf("Still waiting...\n");
			tmo = _SET_SHORT_TIMEOUT(10000);
		}
	}


	if (ifpending(iface) == IF_DOWN) {
		printf("Interface failed to come back up.\n");
		exit(2);
	}

	http_init();
#ifndef INSECURE_HTTP
	https_set_cert(&my_cert);
   printf("Ready: point your browser to https://%s/\n\n",
              inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
#else
   printf("Ready: point your browser to http://%s/\n\n",
              inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
#endif

}








/*******************************************************************************
        Samples\TcpIP\TFTP\tftpclnt.c
        Rabbit Semiconductor, 2001

        Demonstrate how to use TFTP (Trivial File Transfer Protocol) stand-alone
        i.e. without also using BOOTP/DHCP.

        To run this demo, you need to fill in the address of a TFTP server
        machine, as well as the name of a file to download.  The server will
        need to be set up to allow access to the specified file names.
        TFTP servers are finicky about permissions, since there is no
        security with TFTP.  Make sure all files exist and are given world-
        readable/writable permissions.

        If you have purchased two network capable Rabbit boards and connect
        to the same network you may use tftp.c and tftpclnt.c to
        work together.  Be sure to change the TFTP_SERVER macro to point
        it at the server's address.

*******************************************************************************/
#class auto

#define MAX_UDP_SOCKET_BUFFERS 2
//#define TFTP_ALLOW_BUG	// work-around RH7.0 bug

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
 * Define TFTP server, download and upload file names, and max download length.
 */
#define TFTP_SERVER		"10.10.6.111"
#define TFTP_DL_FILENAME	"foo"
#define TFTP_UL_FILENAME	"bar"
#define TFTP_DL_SIZE		3000

#memmap xmem
#use "dcrtcp.lib"
#use "tftp.lib"


udp_Socket tsock;		// Define UDP socket for TFTP use

int main()
{
	struct tftp_state ts;
	int status;
	word bflen;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);


   /*########## DOWNLOAD ##########*/

  	ts.state = 0;								// 0 = read
  	ts.buf_len = TFTP_DL_SIZE;				// max length to download
  	ts.buf_addr = xalloc(TFTP_DL_SIZE);	// allocate a buffer
  	ts.my_tid = 0;								// zero to use default TFTP UDP port number
  	ts.sock = &tsock;							// point to socket to use
  	ts.rem_ip = resolve(TFTP_SERVER);	// resolve server IP address
  	ts.mode = TFTP_MODE_OCTET;				// send/receive binary data
  	strcpy(ts.file, TFTP_DL_FILENAME);	// set file name on server

  	printf("Downloading %s...\n", ts.file);

  	// This uses the non-blocking TFTP functions, but in a blocking
  	// manner.  It would be easier to use tftp_exec(), but this
  	// doesn't return the server error message.
  	tftp_init(&ts);
	while ((status = tftp_tick(&ts)) > 0);	// Loop until complete
	if (!status)
		printf("Download completed\n");
	else if (status == -3)
		printf("ERROR: Download timed out.\n");
	else if (status == -5)
		printf("Download completed, but truncated\n");
	else {
		printf("Download failed: code %d\n", status);
		if (status == -1)
			printf("  Message from server: %s\n", ts.file);
	}


   /*########## UPLOAD ##########*/

  	ts.state = 1;								// 0 = write
  	ts.buf_len = ts.buf_used;				// length to upload (use same buffer as downloaded)
  	ts.my_tid = 0;								// zero to use default TFTP UDP port number
  	ts.sock = &tsock;							// point to socket to use
  	ts.rem_ip = resolve(TFTP_SERVER);	// resolve server IP address
  	ts.mode = TFTP_MODE_OCTET;				// send/receive binary data
  	strcpy(ts.file, TFTP_UL_FILENAME);	// set file name on server

  	printf("Uploading as %s...\n", ts.file);

  	tftp_init(&ts);
	while ((status = tftp_tick(&ts)) > 0);	// Loop until complete
	if (!status)
		printf("Upload completed\n");
	else {
		printf("Upload failed: code %d\n", status);
		if (status == -1)
			printf("  Message from server: %s\n", ts.file);
		else if (status == -3)
			printf("ERROR: Download timed out.\n");
		else if (status == -2) {
			printf("  Server did not ack last packet...\n");
			printf("    Acked %u\n", ts.buf_used);
			if (ts.buf_len - ts.buf_used <= 512)
				printf("    Some bug-ridden servers don't ack the last packet sent :-(\n");
		}
	}

	printf("All done.\n");
	return 0;
}
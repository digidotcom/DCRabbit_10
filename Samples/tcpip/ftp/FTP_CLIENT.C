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
        ftp_client.c

        Demonstration of the use of the ftp_client library to download and
        upload files.  Uses both "standard" mode and "data handler" mode.
        Optionally, also includes FTP server library making it do the same
        things as the FTP_SERVER.C sample program.
*******************************************************************************/
#class auto


// Define to see more of what's going on
//#define FTP_DEBUG
//#define FTP_VERBOSE
//#define DCRTCP_DEBUG
//#define DCRTCP_VERBOSE

// Include FTP server code
//#define SERVER_TOO

// Use passive mode for client.  This works better if you are behind a
// firewall, FTPing to a server outside the firewall.
#define USE_PASSIVE

/*
 * We are not defining any static resources.  Defining the following macro
 * tells ZSERVER.LIB (used by the FTP server) not to look for these static
 * resources.  Note that if you are using on the FTP client, and not the server,
 * then the following macro is unnecessary.
 */
#define SSPEC_NO_STATIC

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
 * FTP Client configuration
 *
 * The FTP client takes several parameters, to determine
 * where the remote ftp server is, and what file to get/put.
 * This can all be placed directly in the ftp_client_setup()
 * call below, but it is defined here for easy-access.
 */

/* Address of remote FTP server */
#define REMOTE_HOST		"10.10.6.178"

/* Port on the FTP server to connect to. 0 == default FTP port */
#define REMOTE_PORT		0

/* username on the server */
#define REMOTE_USERNAME "foo"

/* password on the server */
#define REMOTE_PASSWORD "bar"

/* the files to get/put, and initial working directory */
#define REMOTE_FILE		"testfile1"			// Get this file first
#define REMOTE_FILE_UP	"testfile2"			// Then put it back as this file name
#define REMOTE_FILE_UP2	"testfile3"			// Then create and upload "quick brown fox" data to this file
#define REMOTE_FILE2		REMOTE_FILE_UP2	// And finally download the same data.
#define REMOTE_DIR		""

/********************************
 * End of configuration section *
 ********************************/

#ifdef USE_PASSIVE
	#define PASSIVE_FLAG  FTP_MODE_PASSIVE
#else
	#define PASSIVE_FLAG  0
#endif

#ifdef SERVER_TOO
	#define server_tick  ftp_tick()
	#ximport "samples/tcpip/http/pages/rabbit1.gif" rabbit1_gif

#else
	#define server_tick
#endif

#memmap xmem
#use "dcrtcp.lib"
#use "ftp_client.lib"
#ifdef SERVER_TOO
#use "ftp_server.lib"
#endif

char buf[2048];
int retval;

/* Forward declarations: */
int download_normal(void);
int upload_normal(void);
int download_datahandler(void);
int upload_datahandler(void);

int main()
{
#ifdef SERVER_TOO
	int file;
	int user;

	// Set up the first file and user
	file = sspec_addxmemfile("rabbitA.gif", rabbit1_gif, SERVER_FTP);
	user = sauth_adduser("anonymous", "", SERVER_FTP);
	ftp_set_anonymous(user);
	sspec_setuser(file, user);
	sspec_setuser(sspec_addxmemfile("test1", rabbit1_gif, SERVER_FTP), user);
	sspec_setuser(sspec_addxmemfile("test2", rabbit1_gif, SERVER_FTP), user);

	// Set up the second file and user
	file = sspec_addxmemfile("rabbitF.gif", rabbit1_gif, SERVER_FTP);
	user = sauth_adduser("foo", "bar", SERVER_FTP);
	sspec_setuser(file, user);
	sspec_setuser(sspec_addxmemfile("test3", rabbit1_gif, SERVER_FTP), user);
	sspec_setuser(sspec_addxmemfile("test4", rabbit1_gif, SERVER_FTP), user);
#endif

	printf("Calling sock_init()...\n");
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);


#ifdef SERVER_TOO
	ftp_init(NULL); /* use default handlers */
	tcp_reserveport(FTP_CMDPORT);	// Port 21
#endif

	if (download_normal())
		exit(1);

	if (upload_normal())
		exit(1);

	if (upload_datahandler())
		exit(1);

	if (download_datahandler())
		exit(1);

	printf("FTP client tests completed.\n");
#ifdef SERVER_TOO
	printf("Now running just server...\n");
	for (;;) ftp_tick();
#endif

	return 0;
}




int download_normal(void)
{
	printf("Calling ftp_client_setup() to download %s...\n", REMOTE_FILE);
	if(ftp_client_setup(resolve(REMOTE_HOST),REMOTE_PORT,REMOTE_USERNAME,
			REMOTE_PASSWORD,FTP_MODE_DOWNLOAD|PASSIVE_FLAG,REMOTE_FILE,
			REMOTE_DIR,buf,sizeof(buf))) {
		printf("FTP setup failed.\n");
		exit(0);
	}

	printf("Looping on ftp_client_tick()...\n");
	while( 0 == (retval = ftp_client_tick()) )
		server_tick;

	if( 1 == retval ) {
		printf("FTP download completed successfully.  %d bytes.\n", ftp_client_filesize());
		return 0;
	} else {
		printf("FTP download failed: status = %d, last code = %d\n", retval, ftp_last_code());
		return 1;
	}
}

int upload_normal(void)
{
	printf("Calling ftp_client_setup() to upload %s...\n", REMOTE_FILE_UP);
	if(ftp_client_setup(resolve(REMOTE_HOST),REMOTE_PORT,REMOTE_USERNAME,
			REMOTE_PASSWORD,FTP_MODE_UPLOAD|PASSIVE_FLAG,REMOTE_FILE_UP,
			REMOTE_DIR,buf,ftp_client_filesize())) {
		printf("FTP setup failed.\n");
		exit(2);
	}

	printf("Looping on ftp_client_tick()...\n");
	while( 0 == (retval = ftp_client_tick()) )
		server_tick;

	if( 1 == retval ) {
		printf("FTP upload completed successfully.  %d bytes.\n", ftp_client_filesize());
		return 0;
	} else {
		printf("FTP upload failed: status = %d, last code = %d\n", retval, ftp_last_code());
		return 1;
	}
}


/**
 * 	The return value from this function depends on the in/out
 * 	flag.  For FTPDH_IN, the function should return 'len'
 * 	if the data was processed successfully and download should
 * 	continue; -1 if an error has occurred and the transfer
 * 	should be aborted.  For FTPDH_OUT, the function should
 * 	return the actual number of bytes placed in the data
 * 	buffer, or -1 to abort.  If 0 is returned, then the
 * 	upload is terminated normally.  For FTPDH_END, the
 * 	return code should be 0 for success or -1 for error.  If
 * 	an error is flagged, then this is used as the return code
 * 	for ftp_client_tick().  For FTPDH_ABORT, the return code
 * 	is ignored.
 */
int my_datahandler(char * data, int len, longword offset,
                   int flags, void * dhnd_data)
{
#define DATA "The quick brown fox jumps over the lazy dog.\r\n"
#define DATALEN 46
#define REPS 1000L
	auto int mod;

	switch (flags) {
		case FTPDH_IN:
			printf("DH: got %d bytes at offset %ld\n", len, offset);
			return len;
		case FTPDH_OUT:
			if (offset >= REPS * DATALEN)
				return 0;
			mod = (int)(offset % DATALEN);
			if (len > DATALEN - mod)
				len = DATALEN - mod;
			strncpy(data, DATA + mod, len);
			return len;

		case FTPDH_END:
			printf("DH: END OK\n");
			return 0;
		case FTPDH_ABORT:
			printf("DH: ABORTED\n");
			return 0;
	}
	return -1;
}

int download_datahandler(void)
{
	printf("Calling ftp_client_setup() to download %s...\n", REMOTE_FILE2);
	if(ftp_client_setup(resolve(REMOTE_HOST),REMOTE_PORT,REMOTE_USERNAME,
			REMOTE_PASSWORD,FTP_MODE_DOWNLOAD|PASSIVE_FLAG,REMOTE_FILE2,
			REMOTE_DIR,NULL,0)) {
		printf("FTP setup failed.\n");
		exit(0);
	}
	ftp_data_handler(my_datahandler, NULL, 0);

	printf("Looping on ftp_client_tick()...\n");
	while( 0 == (retval = ftp_client_tick()) )
		server_tick;

	if( 1 == retval ) {
		printf("FTP download completed successfully.  %ld bytes.\n", ftp_client_xfer());
		return 0;
	} else {
		printf("FTP download failed: status = %d, last code = %d\n", retval, ftp_last_code());
		return 1;
	}
}

int upload_datahandler(void)
{
	printf("Calling ftp_client_setup() to upload %s...\n", REMOTE_FILE_UP2);
	if(ftp_client_setup(resolve(REMOTE_HOST),REMOTE_PORT,REMOTE_USERNAME,
			REMOTE_PASSWORD,FTP_MODE_UPLOAD|PASSIVE_FLAG,REMOTE_FILE_UP2,
			REMOTE_DIR,NULL,0)) {
		printf("FTP setup failed.\n");
		exit(0);
	}
	ftp_data_handler(my_datahandler, NULL, 0);

	printf("Looping on ftp_client_tick()...\n");
	while( 0 == (retval = ftp_client_tick()) )
		server_tick;

	if( 1 == retval ) {
		printf("FTP upload completed successfully.  %ld bytes.\n", ftp_client_xfer());
		return 0;
	} else {
		printf("FTP upload failed: status = %d, last code = %d\n", retval, ftp_last_code());
		return 1;
	}
}


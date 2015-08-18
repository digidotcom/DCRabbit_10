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
        samples\tcpip\tftp\tftp.c

        An example using UDP, that implements a tftp server
        that can send and receive files.  If you have purchased two
        network capable Rabbit boards and connect them to the
        same network you may use tftp.c and tftpclnt.c to work
        together.

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
 * This sample does not need any TCP sockets, and only 1 UDP socket.
 * Override default settings.
 */
#define MAX_TCP_SOCKET_BUFFERS 0
#define MAX_UDP_SOCKET_BUFFERS 1

/*
 * TFTP configuration
 *
 * These define the test file that we will serve up
 */

/*
 * Name of sample file
 */
#define SAMPLEFILENAME	"foo"

/* size of the file (in bytes) */
#define SAMPLEFILELEN	1024

/********************************
 * End of configuration section *
 ********************************/

/*********************************************
 * Notes:
 *  - transfer type 'octet' (binary) is always assumed (for now)
 *
 *********************************************/

#memmap xmem
#use "dcrtcp.lib"

/* The sample file we can send */
char samplefile[SAMPLEFILELEN];

udp_Socket tftpsock;

longword remoteip;
word remoteport;

#define TIMEOUT_LENGTH	10

#define PBUF_LEN	518
char pbuf[PBUF_LEN];

/* Error message types */
#define ERR_OTHER		0
#define ERR_NOTFOUND	1
#define ERR_ACCESS	2
#define ERR_NOSPACE	3
#define ERR_ILLEGAL	4
#define ERR_PORT		5
#define ERR_EXISTS	6
#define ERR_NOUSER	7

int open_udp_sock(longword addr)
{
	/*printf("Opening UDP socket\n");*/

	if(!udp_open(&tftpsock, 69, addr, remoteport, NULL)) {
		printf("udp_open failed!\n");
		return 1;
	}
	return 0;
}

void senderror(char type, char *msg)
{
	pbuf[0] = 0;	/* set the error opcode */
	pbuf[1] = 5;

	pbuf[2] = 0;     /* set the error type */
	pbuf[3] = type;

	strcpy(pbuf + 4, msg);

	udp_send(&tftpsock, pbuf, strlen(msg) + 5);
	printf("Error message <%d> '%s' was sent\n",type,msg);
}

void sendack(int block)
{
	pbuf[0] = 0;	/* set the error opcode */
	pbuf[1] = 4;

	pbuf[2] = ((char*)&block)[1];     /* set the error type */
	pbuf[3] = ((char*)&block)[0];

	udp_send(&tftpsock, pbuf, 4);
	printf("ACK <%d> was sent\n", block);
}

/*
 * waits for an ACK to be read; returns 0 on ACK == block, 1 on error, 2 on timeout
 */
int waitforack(int block)
{
	auto int temp, retval;
	auto unsigned long timestamp;

	timestamp = SEC_TIMER;

	while(1) {
		if(SEC_TIMER > (timestamp + TIMEOUT_LENGTH)) {
			printf("TIMEOUT on waiting for ACK; resending ACK.\n");
			return 2;
		}

		tcp_tick(&tftpsock);
		retval = udp_recv(&tftpsock, pbuf, PBUF_LEN);
		if(retval > 0) {
			switch(pbuf[1]) {
			case 4:
				/* ACK received */
				temp = pbuf[3] + (256 * pbuf[2]);
				printf("Got ACK <%d>\n", temp);
				if( temp == block )
					return 0;

				/* wrong ACK? */
				printf("Received incorrect ACK (%d)\n",temp);
				break; /* do nothing; wait for the correct ACK */

			case 5:
				/* ERROR was received. Bail out */
				printf("ERROR was received. Bailing...\n");
				return 1;

			default:
				/* unknown opcode; bailing */
				senderror(ERR_ILLEGAL, "Expected ACK");
				return 1;
			}
		}
	}
}

int doread(void)
{
	auto long length, count;
	auto unsigned long file;
	auto int block, retval;
	auto char *f;

	/* handle the RRQ; put the file to the client */
	if(0 == strcmp(SAMPLEFILENAME, pbuf + 2) ) {
		/* the file requested was good; send it */

		/*file = samplefile;
		 *xmem2root(&length, file, 4);
		 *file += 4; */
		f = samplefile;
		length = sizeof(samplefile);

		printf("File had length %ld\n",length);

		count = 0; block = 0;
		while(length >= 512) {
			block++;

			/* write out the full length data packets */

			do {
				pbuf[0] = 0;		/* set the opcode */
				pbuf[1] = 3;

				pbuf[2] = ((char*)&block)[1];     /* set the block # */
				pbuf[3] = ((char*)&block)[0];

				/*xmem2root(pbuf + 4, file + count, 512);*/ /* fill the packet with data */
				memcpy(pbuf + 4, f + (int)count, 512);


				udp_send(&tftpsock, pbuf, 516);
				printf("Wrote data block <%d> with size 512\n",block);
				tcp_tick(&tftpsock);

				retval = waitforack(block);
				if(1 == retval) return 1;
			} while(2 == retval);

			count += 512;
			length -= 512;
		}

		/* finish up the remaining bytes */
		block++;

		do {
			pbuf[0] = 0;		/* set the opcode */
			pbuf[1] = 3;

			pbuf[2] = ((char*)&block)[1];     /* set the block # */
			pbuf[3] = ((char*)&block)[0];


			/*xmem2root(pbuf + 4, file + count, (int)length);*/
			memcpy(pbuf + 4, f + (int)count, (int)length);

			udp_send(&tftpsock, pbuf, (int)length + 4);
			printf("Wrote data block <%d> with size %d\n", block, (int)length);

			retval = waitforack(block);
			if(1 == retval) {
				return 1;
			}
		} while(2 == retval);

		printf("All done.\n\n");
		return 0;
	} else {
		/* unknown file; error out */
		senderror(ERR_NOTFOUND, "File was not found");
		return 1;
	}
}

int getdatapkt(int block)
{
	auto int temp, retval;

	auto unsigned long timestamp;

	timestamp = SEC_TIMER;

	while(1) {
		if(SEC_TIMER > (timestamp + TIMEOUT_LENGTH)) {
			printf("TIMEOUT on reading data packet; resending ACK.\n");
			sendack(block - 1);
			timestamp = read_rtc();
		}

		tcp_tick(&tftpsock);
		retval = udp_recv(&tftpsock, pbuf, PBUF_LEN);
		if(retval > 0) {
			switch(pbuf[1]) {
			case 3:
				/* DATA received */
				temp = pbuf[3] + (256 * pbuf[2]);
				printf("Got data pkt <%d>\n", temp);
				if( temp == block ) {
					/* correct data file was read */
					sendack(block);	/* ACK the block */
					return retval;			/* return the length read */
				}

				/* wrong DATA? */
				break; /* do nothing; wait for the correct data pkt */

			case 5:
				/* ERROR was received. Bail out */
				return -1;

			default:
				/* unknown opcode; bailing */
				senderror(ERR_ILLEGAL, "Expected DATA");
				return -1;
			}
		}
	}
}

/* temporary storage for the received file */
char buf[2048];

int dowrite(void)
{
	auto char *file;
	auto int buflen, block, retval;

	/* handle the WRQ; get the file from the user */

	/* get a place to put the file */
	file = buf;
	buflen = 2048;

	printf("Receiving '%s'...\n",pbuf+2);

	sendack(0); /* ACK the WRQ, wait for the first data packet */

	block = 0;
	while(1) {
		block++;

		/* get a data packet */
		retval = getdatapkt(block);
		if(-1 == retval) return 1;

		retval -= 4;	/* strip off header */

		if(retval > buflen) { /* check for storage space */
			senderror(ERR_NOSPACE, "Out of room to store file");
			return 1;
		}

		memcpy(file, pbuf+4, retval); /* copy the file to dest */
		file += retval;
		buflen -= retval;

		if(retval < 512) {
			/* all done */
			printf("RECEIVED FILE-->\n");
			for(retval = 0; retval < (2048-buflen); retval++)
				putchar(buf[retval]);
			printf("<--END OF RECEIVED FILE\n\n");
			return 0;
		}
	}
}

int do_tftp(void)
{
	auto int retval;

	remoteip = remoteport = 0;
	if(open_udp_sock(0)) /* open listen port */
		return 0;

	while(tcp_tick(&tftpsock)) {

		retval = udp_recvfrom(&tftpsock, pbuf, PBUF_LEN, &remoteip, &remoteport);
		// Must have at least two bytes
		if(retval >= 2) {
			printf("REMOTEIP == 0x%lx, REMOTEPORT == 0x%x\n", remoteip, remoteport);

			/* possible request was received */
			switch(pbuf[1]) {
			case 1: {
				/* RRQ */
				printf("Read request received for file '%s'\n",pbuf+2);
				doread();
				break;
			}
			case 2: {
				/* WRQ */
				printf("Write request received for file '%s'\n",pbuf+2);
				dowrite();
				break;
			}
			case 4: {
				printf("ACK <%d> was received (error)\n",pbuf[3] + (256 * pbuf[2]));
				break;
			}
			default:
				/* error? */
				printf("Bad opcode was received: %d\n", pbuf[1]);
				senderror(ERR_ILLEGAL, "opcode was not RRQ or WRQ");
			}

			/* reset the udp socket */
			remoteip = remoteport = 0;
			udp_close(&tftpsock);
			if(open_udp_sock(0))
				return 0;
		}
	}

	sock_close(&tftpsock);
	return 0;
}

void main()
{
	auto int i;

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

	for(i = 0; i < SAMPLEFILELEN; i++) {
		samplefile[i] = 'a';
	}

	while(1) {
		do_tftp();
	}
}
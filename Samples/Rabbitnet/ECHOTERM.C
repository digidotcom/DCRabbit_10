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
/*******************************************************************
   echoterm.c

	This sample program is for any RabbitNet board.

	Description
	===========
	This program demonstrates a simple character echo to any
	RabbitNet board through a serial terminal on the master controller.

   This program will first look for a device directly connected to
   each controller port using rn_device().  The last device found
	will echo characters sent by the controller.

	Connections
	===========
   1. Connect RabbitNet and power supply cables to the controller
   	and RN boards as described in the RabbitNet Manual.
	2. Connect PC (tx) to the controller on an available tx port
   	such as TXE.
  	3. Connect PC (rx) to the controller on an available rx port
   	such as RXE.
	4. Connect PC GND to GND.
   5. Configure the controller serial port below to match your
   	connections.

	Tera Term setup
	===============
	1. Startup Tera Term on your PC.
	2. Configure the serial parameters for the following:
	   a) Baud Rate of 19200, 8 bits, no parity and 1 stop bit.
	   b) Enable the "Local Echo" option.
	   c) Set line feed options to:  Receive = CR     Transmit = CR+LF

	Instructions
	============
   1. Configure the search criteria in the section below.
 	2. Set PRINTSTATS below to display status byte descriptions.
	3. Compile and run this program.

*******************************************************************/
#class auto
/////
// configure your serial port connection here
//	presently configured serial port E
///
#define seropen	serEopen
#define serclose	serEclose
#define serputs	serEputs
#define serputc	serEputc
#define serwrite	serEwrite
#define sergetc	serEgetc
#define serrdFlush serErdFlush
#define serwrFlush serEwrFlush
#define serrdFree	serErdFree
#define serwrFree	serEwrFree
#define SXSR SESR
#define EINBUFSIZE 255
#define EOUTBUFSIZE 255

#define INBUFSIZE EINBUFSIZE
#define OUTBUFSIZE EOUTBUFSIZE
#define BAUDRATE 19200l

//////
// Define as 1 or 0 to display or not display status bit descriptions
//////
#define PRINTSTATS 1

const char statstr[9][128] = {
	"bit 0 Reset occured, check control register",
	"bit 1 Command rejected, try again",
	"bit 2 Reserved",
	"bit 3 Reserved",
	"bit 4 Communication error, check comm status register",
	"bit 5 Reserved",
	"bit 6 Device Ready",
	"bit 7 Device Busy, try again",
   "No Connection"
   };

void printstat(int statusbyte)
{
	auto int i;
	auto char sbuf[128];

	if (statusbyte == -1)
   {
	   sprintf(sbuf, "Status %d: %s\n\r\x0", statusbyte, statstr[8]);
		serwrite(sbuf, strlen(sbuf));
   }
   else
   {
	   sprintf(sbuf, "Status 0x%02x description:\n\r\x0", statusbyte);
		serwrite(sbuf, strlen(sbuf));
		for (i=0; i<8; i++)
	   {
   		if ((statusbyte>>i)&1)
      	{
	      	sprintf(sbuf, " %s\n\r\x0", statstr[i]);
				serwrite(sbuf, strlen(sbuf));
   	   }
   	}
   }
	serputs("\n\r\x0");
}


nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


main()
{
	auto int device0, tmpdev, portnum, i, statusbyte;
	auto char sendbyte, recbyte;
	auto char sendbuf[128];
   auto rn_devstruct *devaddr;

	brdInit();                 //initialize controller
	seropen(BAUDRATE);
	serrdFlush();
	serwrFlush();

	memset(sendbuf, '\x0', sizeof(sendbuf));
	while(serrdFree() != INBUFSIZE) sergetc();
	serputs("Starting . . .\n\r\x0");

   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   device0 = -1;
   //search on ports using physical node address
   for (i=0, portnum=0000; i<RN_MAX_PORT; i++, portnum+=0100)
   {
	   if ((tmpdev = rn_device(portnum)) == -1)
		{
			sprintf(sendbuf, "\n\rNo board connection at port %d\n\r\x0", i);
			serwrite(sendbuf, strlen(sendbuf));
      }
      else
      {
   		devaddr = (rn_devstruct *)tmpdev;
   		sprintf(sendbuf, "\n\rDevice found on port %d\n\r\x0", i);
			serwrite(sendbuf, strlen(sendbuf));
         //rn_devtable is a global table
   		sprintf(sendbuf, "Serial number 0x%02x%02x%02x%02x\n\r\x0",
         	devaddr->signature[0], devaddr->signature[1],
            devaddr->signature[2], devaddr->signature[3]);
			serwrite(sendbuf, strlen(sendbuf));
         device0 = tmpdev;
      }
   }

   if (device0 == -1)
   {
		serputs("\n\rNo board connections!\n\r\x0");
		while (serwrFree() != OUTBUFSIZE);
	 	while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
		serclose();
   	exit(0);
   }

	serputs("\n\rEcho characters on last device found ...\n\n\r\x0");
	while (1)
	{
		for (sendbyte='A'; sendbyte<='Z'; sendbyte++)
		{
			statusbyte = rn_echo(device0, sendbyte, &recbyte);
			if( sendbyte == recbyte )
			{
				ledOut(0,1);
				for (i=0; i<4000; i++);
				ledOut(0,0);
				for (i=0; i<6000; i++);
			}
			else
			{
				ledOut(1,1);
				for (i=0; i<4000; i++);
				ledOut(1,0);
				for (i=0; i<6000; i++);
			}
			sprintf(sendbuf,"Send %c, receive %c\n\r\x0", sendbyte, recbyte);
         if (PRINTSTATS) printstat(statusbyte);
			serwrite(sendbuf, strlen(sendbuf));
         msDelay(1000);
		}
		while (serwrFree() != OUTBUFSIZE);
  	 	while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
	}
	serclose();
}


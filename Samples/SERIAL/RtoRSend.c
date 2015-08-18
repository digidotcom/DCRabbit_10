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
/*****************************************************

     RtoRSend.c

     This program sends a message over serial port B to another Rabbit device,
     waits for a reply, then echos the reply to the stdio window.

     Connect RS232 cable from one Rabbit to the other:
     	 Connect GND on one Rabbit to GND on the other
       Connect RXB on J5 from one Rabbit to TXB on J5 on the other
       Connect TXB on J5 from one Rabbit to RXB on J5 on the other

     Connect the programming cable to the other Rabbit
     Configure Dynamic C 'Default BIOS file type' in Options/Compiler menu
       selection to the radio button 'Code and BIOS in Flash'
     Recompile BIOS on the other Rabbit (Ctrl-Y)
     Run Samples\RS232\RtoRReply.c on the other Rabbit (F9)
     Disconnect power from the other Rabbit
     Click Ok on the 'Target not responding' dialog box, this is normal
     Disconnect the programming cable from the other Rabbit
     The other Rabbit now has RtoRReply.c ready to run in flash
     Connect the programming cable to the primary Rabbit
     Connect power to the primary Rabbit (if not already powered)
     Connect power to the other Rabbit
     The other Rabbit is now waiting for a message to which it will reply
     Recompile BIOS on the primary Rabbit (Ctrl-Y)
     Run this program on the primary Rabbit (F9)
     Type a message from the PC keyboard followed by the 'Enter' key

******************************************************/
#class auto


/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#define BINBUFSIZE  63
#define BOUTBUFSIZE 63

#define TIMEOUT 20UL   // will time out 20 milliseconds after receiving any
                       // character unless MAXSIZE characters are received

#define MAXSIZE  128
#define MAXSIZE2 160

char mssg[MAXSIZE];
char msg2[MAXSIZE2];

void main()
{
	int n;

	n = 0;
   serBopen(19200);
   printf("Enter message, or just 'Enter' to quit\n\n");
   gets(mssg);
	while (strlen(mssg) > 0)
	{
   	serBputs(mssg);
   	serBrdFlush();
   	while ((n = serBread(msg2, MAXSIZE2-1, TIMEOUT)) == 0) ;

   	msg2[n] = 0;
   	printf("%s\n\n", msg2);

   	printf("Next message, or just 'Enter' to quit\n\n");
   	gets(mssg);
   }
   while (serBwrFree() != BOUTBUFSIZE) ;
   serBclose();
}
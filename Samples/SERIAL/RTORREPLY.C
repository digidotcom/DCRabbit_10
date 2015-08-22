/*****************************************************

     RtoRReply.c
     Z-World, 1999

     This program is to be run on a second Rabbit device, with the first
     Rabbit running Samples\RS232\RtoRSend.c. This program waits for
     a message to arrive over serial B port from the first Rabbit, after
     which it sends a reply back over serial port B.

     Connect RS232 cable from one Rabbit to the other:
     	 Connect GND on one Rabbit to GND on the other
       Connect RXB on J5 from one Rabbit to TXB on J5 on the other Rabbit
       Connect TXB on J5 from one Rabbit to RXB on J5 on the other Rabbit

     Connect the programming cable to the Rabbit on which this will be run
     Configure Dynamic C 'Default BIOS file type' in Options/Compiler menu
       selection to the radio button 'Code and BIOS in Flash'
     Recompile BIOS (Ctrl-Y)
     Run Samples\RS232\RtoRReply.c (F9)
     Disconnect power
     Click Ok on the 'Target not responding' dialog box, this is normal
     Disconnect the programming cable
     The Rabbit now has RtoRReply.c ready to run in flash

******************************************************/

/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#class auto

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

   serBopen(19200);
   while (1)
   {
   	serBrdFlush(); 		// Remove any waiting chars.
  		while ((n = serBread(mssg, MAXSIZE-1, TIMEOUT)) == 0) ;
   	mssg[n] = 0;

   	sprintf(msg2, "This is Rabbit 2's reply to \"%s\"\r\n\r\n", mssg);

   	serBputs(msg2);
   }
}
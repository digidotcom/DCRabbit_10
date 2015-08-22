/*****************************************************

     EchoChr.c
     Digi International, Copyright ©2010.  All rights reserved.

     This program echos characters over serial port D.
     It must be run with a serial utility such as Hyperterminal.

     Connect RS232 cable from PC to Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXD
       Connect PC's RS232 RD  to Rabbit TXD

     Configure the serial utility for port connected to RS232, 19200 8N1.

     Run this program.
     Run Hyperterminal.
     Type characters into the serial utility window.

     This program will echo characters sent from the serial utility back to
     the serial utility where they will appear in its send/receive window.
     It will terminate after reading a single <Esc>.

******************************************************/
#class auto

/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#define DINBUFSIZE  15
#define DOUTBUFSIZE 15

#define  CH_ESCAPE	27

#define  D_BAUDRATE	19200L

xmem void main()
{
   int c;

	c = 0;
   serDopen(D_BAUDRATE);
   while (c != 27) {  // Exit on Esc
      if ((c = serDgetc()) != -1 && c != CH_ESCAPE) {
         serDputc(c);
      	if( c == '\r' ) { 		/* Cook ENTER into CR_LF */
      		serDputc('\n');
      	}
      }
   }
   serDputs("Done\r\n");
   while (serDwrFree() != DOUTBUFSIZE) ;      // allow transmission to complete before closing
   serDclose();
}
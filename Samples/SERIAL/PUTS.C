/*****************************************************

     Puts.c
     Digi International, Copyright ©2010.  All rights reserved.

     This program writes a null terminated string over serial port D.
     It must be run with a serial utility such as Hyperterminal.
     This program will ensure all chars are sent, then exit

     Connect RS232 cable from PC to Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXD
       Connect PC's RS232 RD  to Rabbit TXD

     Configure the serial utility for port connected to RS232, 19200 8N1.

     Run Hyperterminal.
     Run this program.
     See the message appear.

******************************************************/
#class auto

/*****************************************************
     The input and output buffers sizes are defined here. If these
     are not defined to be (2^n)-1, where n = 1...15, or they are
     not defined at all, they will default to 31 and a compiler
     warning will be displayed.
******************************************************/
#define DINBUFSIZE 15
#define DOUTBUFSIZE 15

void main()
{
	static const char s[] = "Hello Rabbit\r\n";

	serDopen(19200);
	serDputs(s);

	// Wait until the port is done sending
	while (serXsending(SER_PORT_D));

	// now we can close the serial port without cutting off Tx data
	serDclose();
}
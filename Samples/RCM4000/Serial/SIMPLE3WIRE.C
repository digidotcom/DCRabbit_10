/**********************************************************
	simple3wire.c
   Rabbit Semiconductor, 2006

	This program is used with RCM4000 series controllers
	and prototyping boards.

	Description
	===========
	This program demonstrates basic initialization for a
	simple RS232 3-wire loopback displayed in STDIO window.

	Normally we would connect to another controller,

		Tx  <---> Rx
		Rx  <---> Tx
		Gnd <---> Gnd

	However, for this simple demonstration, make the
	following connections:

	Proto-board Connections
	=======================

		TxC <---> RxD
		RxC <---> TxD

	Instructions
	============
	1.  Compile and run this program.
	2.  Lower case characters are sent by TxC. RxD receives
	    the character.  TxD sends the converted uppercase
	    character to RxC and displays in STDIO.

**********************************************************/
#class auto

///////
// change serial buffer name and size here
///////
#define CINBUFSIZE  15
#define COUTBUFSIZE 15

#define DINBUFSIZE  15
#define DOUTBUFSIZE 15

///////
// change serial baud rate here
///////
#ifndef _232BAUD
#define _232BAUD 115200
#endif

// RCM40xx boards have no pull-up on serial Rx lines, and we assume in this
// sample the possibility of disconnected or non-driven Rx line.  This sample
// has no need of asynchronous line break recognition.  By defining the
// following macro we choose the default of disabled character assembly during
// line break condition.  This prevents possible spurious line break interrupts.
#define RS232_NOCHARASSYINBRK

main()
{
	auto int nIn1, nIn2;
	auto char cOut;

   serCopen(_232BAUD);
	serDopen(_232BAUD);
	serCwrFlush();
	serCrdFlush();
	serDwrFlush();
	serDrdFlush();

	while (1)
	{
		for (cOut='a';cOut<='z';++cOut)
		{
			serCputc (cOut);								//	Send lowercase byte
			while ((nIn1=serDgetc()) == -1);			// Wait for echo
			serDputc (toupper(nIn1));					//	Send the converted upper case byte
			while ((nIn2=serCgetc()) == -1);			// Wait for echo
			printf ("Serial C sent %c, serial D sent %c, serial C received %c\n",
						cOut, toupper(nIn1), nIn2);
		}
	}
}


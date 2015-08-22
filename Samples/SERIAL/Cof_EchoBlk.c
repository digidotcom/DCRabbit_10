/*****************************************************

     Cof EchoBlk.c
     Digi International, Copyright ©2010.  All rights reserved.

     This program echos a block of characters over serial port D.
     It must be run with a serial utility such as Hyperterminal.

     Connect RS232 cable from PC to Rabbit:
     	 Connect PC's RS232 GND to Rabbit GND
       Connect PC's RS232 TD  to Rabbit RXD
       Connect PC's RS232 RD  to Rabbit TXD

     Configure the serial utility for port connected to RS232, 19200 8N1.

     This program uses the single user cofunction form of the serial read
     and write routines, which allows it to cooperatively multi-task with
     other costates when it is not actively reading or writing data. It
     reads bytes over serial port D into a data structure and writes back
     what it read as soon as the number of bytes defined by the length
     parameter is read or when it times out. The length parameter in this
     example is set to the size of the data structure into which the stream
     of serial data will be read. Each time a complete block is read the
     timeout clock stops. Once a new block begins, it will timeout if the
     byte to byte period elapses prior to completing the block. It will
     terminate if a timeout occurs after reading a single <Esc>.

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

// This timeout period determines when an active input data stream is
//   considered to have ended and is implemented within serDread.
#define MSSG_TMOUT 3000UL  // will discontinue collecting data 3 seconds after
                           // receiving any character or when maxSize are read.

// This timeout period determines when to give up waiting for any data to
//   arrive and must be implemented within the user's program, as it is below.
#define IDLE_TMOUT 20000UL // will timeout after 20 seconds if no data is read.

typedef struct {
	short d;
	float f[20];
} DATA;    // cof_serDread and cof_serDwrite read and write bytes of any data type

void main()
{
	unsigned long t;
	static const char Esc = 27;
	int n, maxs;
	DATA data;
	char s[32];

   maxs = sizeof(DATA);   // 82 bytes
	n = 0;
   serDopen(19200);
   for (;;) {
		loophead();
   	costate {
   		t = MS_TIMER;
   		wfd n = cof_serDread(&data, maxs, MSSG_TMOUT); // yields until ends
   		if (n == 1 && (char) data.d == Esc) {          // quit if just <Esc>
	      	wfd cof_serDputs("\r\nEscaped!\r\n");
	      	break;
   		}
   		else {
	      	wfd cof_serDwrite(&data, n);
	   		if (n != maxs) {
	   			sprintf (s, "\r\n%d characters read.\r\n", n);
	     			wfd cof_serDputs(s);
     			}
      	}
      }
      costate {
    		if (MS_TIMER - t > IDLE_TMOUT)
    		{
    			t = MS_TIMER;
    			wfd cof_serDputs("\r\nTimed out!\r\n");
    		}
      }
   }

   while (serDwrFree() != DOUTBUFSIZE);	// wait for Tx buffer empty
   serDclose();									// disables D serial port r/w
}
/*******************************************************************

	SerialToSerial.c

 	Digi International, Copyright (C) 2008.  All rights reserved.

   This program is used with a RCM56xxW or RCM66xxW series controller with
   interface board, and optionally a digital I/O accessory board,
   and optionally, a serial port accessory board.

   See "Add Additional Boards" in the User's Manual for instructions on
   how to attach the accessory boards.

   Description
   ===========
   This program monitors switch S1 and light LED DS1 on the baseboard, or if
   DIGITAL_IO_ACCESSORY is defined, it monitors switches S1, S2, S3 and S4 on
   the Digital I/O accessory board and lights LEDs DS1-DS4 while
   a corresponding button switch is pressed. It also sends messages
   indicating that a switch is pressed from serial port D to serial port C.
   Messages received by serial port C are displayed in Dynamic C's
   stdio window.

   A serial port accessory board can be used, but the two serial ports can
   also communicate at TTL levels with direct connections on the interface
   board or the digital I/O accessory board by simply connecting:

       J2 pin 19 (PC0/TXD)   to    J2 pin 22 (PC3/RXC).

   If using the Serial Port Accessory board, you can connect:
       J3 pin 3 (TXD) to  J4 pin 5 (RXC) instead.

   Use the following jumper placements on the interface board:

	I/O control       On Interface Board
	--------------    ------------------
	Port D bit 0		DS1, LED

   Jumper settings (Interface Board)
   ---------------------------------
   JP1   5-6, 7-8

         2    4    6   8
         o    o    o   o
         |         |   |
         o    o    o   o
         1    3    5   7

   If used, set the following jumper placements on the digital I/O
   accessory board:

	I/O control       On Digital I/O board
	--------------    ----------------------
	Port A bits 4-7  	LEDs DS1-DS4
	Port B bits 4-7	Button switches S1-S4

     Jumper settings (Digital I/O board AND Serial Port board if using)
     -----------------------------------
      JP5   1-2, 3-4, 5-6, 7-8
      JP8   1-2, 3-4, 5-6, 7-8

           2    4    6   8
           o    o    o   o
           |    |    |   |
           o    o    o   o
           1    3    5   7

       These connect switch pins to GND and 3.3V

       JP7   2-4, 3-5

        1 o    o 2
               |
        3 o    o 4
          |
        5 o    o 6

        7 o    o 8

   Instructions
   ============
   1.  Make sure jumpers are connected as shown above.
   2   Uncomment DIGITAL_IO_ACCESSORY definition below if using
       digital I/O accessory board.
   3.  Compile and run this program.
   4.  Press S1 on the interface board or S1-S4 on the digital I/O
       accessory board to see serial messages in the sdtdio window.
******************************************************/

// Comment the following define to use only interface board with its one
// button and corresponding LED, uncomment to use the digital I/O accessory
// board with its four buttons and corresponding LEDs.
//#define DIGITAL_IO_ACCESSORY

/******************************************************
  The input and output buffers sizes are defined here. If these
  are not defined to be (2^n)-1, where n = 1...15 as required by the
  serial drivers, or they are not defined at all, they will default
  to 31 and a compiler warning will be displayed. Define OUTBUFSIZE
  large enough to hold the entire null terminated output string.
******************************************************/
#define DINBUFSIZE  15
#define DOUTBUFSIZE 63
#define CINBUFSIZE  63
#define COUTBUFSIZE 15

#ifdef DIGITAL_IO_ACCESSORY
   #define DS1 4
   #define DS2 5
   #define DS3 6
   #define DS4 7
   #define S1  4
   #define S2  5
   #define S3  6
   #define S4  7
#else
   #define DS1 0
   #define DS4 0
   #define S1  1
   #define S4  1
#endif

#define BAUDRATE 9600
#define MAX_TX_LEN   32

#define LEDON  0
#define LEDOFF 1

#ifdef DIGITAL_IO_ACCESSORY
InitIO()
{
   // Make Port B pins for switch inputs
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S1);
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S2);
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S3);
   BitWrPortI(PBDDR, &PBDDRShadow, 0, S4);

   // Set Port A pins for LEDs low
   BitWrPortI(PADR, &PADRShadow, 1, DS1);
   BitWrPortI(PADR, &PADRShadow, 1, DS2);
   BitWrPortI(PADR, &PADRShadow, 1, DS3);
   BitWrPortI(PADR, &PADRShadow, 1, DS4);
}
#endif

#if RCM6600W_SERIES
	#use "rcm66xxw.lib"
#else
	#use "rcm56xxw.lib"
#endif

void main()
{
	char outString[MAX_TX_LEN], inString[MAX_TX_LEN];
   int i, j, ch, out_string_len;

	brdInit();
   // The brdInit for this board sets pins that are not initially assigned to
   // hardware as outputs, such as the serial port Rx pins. The Rx pin for
   // serial port D is not set to an input because it is not used in this
   // sample.
   BitWrPortI(PCDDR, &PCDDRShadow, 0, 3);  // set serial port C Rx as input

	serDopen(BAUDRATE);
	serCopen(BAUDRATE);

#ifdef DIGITAL_IO_ACCESSORY
	InitIO();
#endif

   // Initialize DS1 LED (PDO) to output
 	BitWrPortI(PDDDR, &PDDDRShadow, 0, 1);
   // Initialize DS1 LED (PDO) to output
 	BitWrPortI(PDDDR, &PDDDRShadow, 1, 0);
   // Make sure PD0 not set to alternate function
 	BitWrPortI(PDFR,  &PDFRShadow, 0, 1);
   // Make sure PD0 not set to alternate function
 	BitWrPortI(PDFR,  &PDFRShadow, 0, 0);

   j = 0;

   while(1){

		costate
      {
     		// Bits for switches and LEDs correspond
 			for (i = S1; i <= S4; i++)
  			{
#ifdef DIGITAL_IO_ACCESSORY
            if (!BitRdPortI(PBDR, i))
#else
            if (!BitRdPortI(PDDR, i))
#endif
      		{
               // Delay so we don't output too many times
            	waitfor(DelayMs(300));

               // Light corresponding LED
#ifdef DIGITAL_IO_ACCESSORY
					BitWrPortI(PADR, &PADRShadow, LEDON, i);
#else
					BitWrPortI(PDDR, &PDDRShadow, LEDON, i-1);
#endif

					sprintf(outString, "Switch %d is on\n", i-S1+1);
               out_string_len = strlen(outString);

               // Make sure there's room in ouput buffer
					waitfor(serDwrFree() > out_string_len);
               // Write string out
               serDwrite(outString, out_string_len);
     			}
      		else
            {
#ifdef DIGITAL_IO_ACCESSORY
		   		BitWrPortI(PADR, &PADRShadow, LEDOFF, i);  // LED off
#else
               BitWrPortI(PDDR, &PDDRShadow, LEDOFF, i-1);
#endif
     			}
	   	}
      }

      // Wait for any ASCII input
      //  serCgetc() returns -1 (0xFFFF) if no chars available
     	if ( (ch = serCgetc()) != -1 )
      {
  	      inString[j] = ch;
     	   j++;
         if (j >= MAX_TX_LEN)         // Make sure we don't overflow
         {                            //  array bounds in case 0x0A
            j = 0;                    //  gets dropped.
         }
         else if (ch == '\n')         // Check for new line as delimiter
  	      {
     	      inString[j] = 0;          // NULL terminate
     			printf("%s",inString);
           	j = 0;
  	      }
      }
   }
}



/*******************************************************************************
	hdlc_test_4000.c
 	Rabbit, 2007

	This program is used with Rabbit 4000 based controllers on an RCM4xxx
	prototyping board.

	Description
	===========
	This program demonstrates a simple HDLC mode loopback test on one serial
	port, either E or F.  Please note that not all RCM4xxx boards make both of
	these serial ports externally available.  Consult your hardware manual for
	information relevant to your particular board.

	Instructions
	============
	1.  Uncomment exactly one of the HDLC_x_USEPORT macros below to choose either
	    serial port E or serial port F.
	2a. (Mutually exclusive with 2b.)  If using serial port E, connect J2 PE7 and
	    PE7 together on the RCM4xxx prototyping board.  Note that this sample
	    relies on parallel port E's alternate function capability to reconfigure
	    PE7 and PE6 for use with serial port E. By default, PC7 and PC6 are
	    associated with serial port E.
	2b. (Mutually exclusive with 2a.)  If using serial port F, connect J4 RXC and
	    TXC together on the RCM4xxx prototyping board.  Note that this sample
	    relies on parallel port C's alternate function capability to reconfigure
	    PC2 and PC3 for use with serial port F.  By default, PC2 and PC3 are
	    associated with serial port C.
	3.  Compile and run this program.
	4.  Packets are sent out and received via either serial port E or F.  Observe
	    the packets' Tx / Rx status, presented on the STDIO window.
*******************************************************************************/
#class auto

#define	STDIO_DEBUG_SERIAL	SADR
#define	STDIO_DEBUG_BAUD	57600

// uncomment exactly one of the two following macro definition lines and edit
//  it if necessary to select the appropriate one of parallel ports C, D or E
#define HDLC_E_USEPORT E
//#define HDLC_F_USEPORT E

// uncomment the following macro definition line to allow successful receipt of
//  packets while single-stepping in debug mode (the default HDLC_PACKET.LIB
//  interrupt priority of 1 competes unsuccessfully with debug communication)
//#define HDLC_X_INTLEVEL 0x02

// uncomment the following line to enable HDLC_PACKET.LIB debugging
//#define HDLC_DEBUG

// uncomment the following line to enable HDLC_PACKET.LIB printf information
#define HDLC_VERBOSE

#ifdef HDLC_E_USEPORT
 #ifdef HDLC_F_USEPORT
	#fatal "Must define only one of HDLC_E_USEPORT or HDLC_F_USEPORT."
 #endif
	#define HDLCopenX HDLCopenE
	#define HDLCsendingX HDLCsendingE
	#define HDLCsendX HDLCsendE
	#define HDLCerrorX HDLCerrorE
	#define HDLCpeekX HDLCpeekE
	#define HDLCdropX HDLCdropE
  #ifdef HDLC_X_INTLEVEL
	#define HDLC_E_INTLEVEL HDLC_X_INTLEVEL
  #endif
#else
 #ifdef HDLC_F_USEPORT
	#define HDLCopenX HDLCopenF
	#define HDLCsendingX HDLCsendingF
	#define HDLCsendX HDLCsendF
	#define HDLCerrorX HDLCerrorF
	#define HDLCpeekX HDLCpeekF
	#define HDLCdropX HDLCdropF
  #ifdef HDLC_X_INTLEVEL
	#define HDLC_F_INTLEVEL HDLC_X_INTLEVEL
  #endif
 #else
	#fatal "Must define exactly one of HDLC_E_USEPORT or HDLC_F_USEPORT."
 #endif
#endif

#use hdlc_packet.lib

#define BUFFER_COUNT 4
#define BUFFER_SIZE 100
#define BAUD_RATE 100000L

void main(void)
{
	static char tx_packet[100];
	static char rx_packet[100];
	auto int errors;
	auto int packet_size;
	auto int peeklen;
	auto unsigned long peekaddr;
	auto unsigned long xbuffers;

	xbuffers = xalloc(BUFFER_COUNT * (BUFFER_SIZE + 4));
	HDLCopenX(BAUD_RATE, HDLC_NRZ, xbuffers, BUFFER_COUNT, BUFFER_SIZE);
	strcpy(tx_packet, "CatMouse987654321");

	while (1) {
		if (HDLCsendingX() == 0) {
			if (HDLCsendX(tx_packet, strlen(tx_packet)) == 1) {
				printf("Sent packet '%s' (%d bytes).\n", tx_packet,
				       strlen(tx_packet));
			} else {
				printf("Packet not sent.\n");
			}
		}

		errors = HDLCerrorX();
		// NB:  If seeing errors reported while single stepping in debug mode,
		//      uncomment the HDLC_X_INTLEVEL macro definition override, above.
		if (errors) {
			printf("HDLC error code 0x%04X:\n", errors);
			if (HDLC_NOBUFFER & errors) printf("  HDLC_NOBUFFER\n");
			if (HDLC_OVERRUN & errors) printf("  HDLC_OVERRUN\n");
			if (HDLC_OVERFLOW & errors) printf("  HDLC_OVERFLOW\n");
			if (HDLC_ABORTED & errors) printf("  HDLC_ABORTED\n");
			if (HDLC_BADCRC & errors) printf("  HDLC_BADCRC\n");
		}

		//use the more efficient peek method
		if (HDLCpeekX(&peekaddr, &peeklen)) {
			xmem2root(rx_packet, peekaddr, peeklen);
			packet_size = peeklen;
			HDLCdropX();
		} else {
			packet_size = 0;
		}

		if (packet_size > 0) {
			rx_packet[packet_size] = 0;
			printf("Read packet '%s' (%d bytes).\n", rx_packet, packet_size);
		}
	}
}
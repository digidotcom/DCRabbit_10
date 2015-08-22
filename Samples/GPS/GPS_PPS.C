/*
	samples\gps\gps_pps.c

	Demonstrate synchronization of local clocks with GPS time pulses.

	This assumes the following hardware:
	. GPS receiver attached to serial port C with 4800bps 8N1
	  NMEA-0183 messages
	. a PPS (Pulse Per Second) signal is connected to PE1.  low->high
	  transition is trigger.
	. PB4 is an active low reset to the GPS receiver

*/

#define GPS_PPS_IPSET 2

//#define GPS_VERBOSE
#define GPS_DEBUG

//////////////////////// GPS STUFF /////////////////////////
#define NMEA_BAUD			4800

#define SERC_TXPORT		PCDR
#define SERC_RXPORT		PCDR
#define CINBUFSIZE		63
#define COUTBUFSIZE		63
/////////////////////// END GPS STUFF //////////////////////

#use "gps.lib"

/********* Position Cursor **********/

void gotoxy (int x, int y)
{
	printf( "\x1B[%d;%dH", y, x);
}


void main(void)
{
	long sec_timer;
	struct tm  st;
	char ppschar = ' ';
	int status = -1;
	int rc;

	BitWrPortI(PBDDR, &PBDDRShadow, 1, 4); // GPSReset to Output
	BitWrPortI(PCDDR, &PCDDRShadow, 1, 2); // Serial C on PortC
	BitWrPortI(PCFR, &PCFRShadow, 1, 2); // Set PC2 to serial C out
	WrPortI(PCALR, &PCALRShadow, 0x00); // Make sure set to ALT0;
	// Serial Port C
	WrPortI(SCCR, &SCCRShadow, 0x00); // Setup Port C for GPS communication
	// GPS Reset SET GPS ON
	BitWrPortI(PBDR, &PBDRShadow, 1, 4); // Release reset
	serCopen(NMEA_BAUD);

	nmea_pps_init(1);		// PE1 as interrupt

	sec_timer = 0;

	printf("SEC_TIMER:\n"
			 "PPS:\n"
			 "NMEA:\n"
			 "Status:\n\n");

	printf("Note: SEC_TIMER is local rabbit clock, printed at point of update.\n");
	printf("        It should visibly converge to exact sync with PPS.\n");
	printf("      PPS is a character that toggles when each PPS pulse is detected\n");
	printf("        which is the exact instant of UTC second increment.\n");
	printf("        Also shows RTC LSBs (32768Hz clock) value at PPS.\n");
	printf("      NMEA is updated on receipt of a timestamp via the NMEA mesage.\n");
	printf("        It may not update every second, and will usually lag.\n");
	printf("      Status should progress INIT, CONNECT, UTC OK, LOCKED.\n");
	printf("        LOCKED means the local SEC_TIMER is in sync with the PPS.\n");

	for (;;) {
		rc = nmea_tick(SER_PORT_C);

		if (sec_timer != SEC_TIMER) {
			sec_timer = SEC_TIMER;
			mktm(&st, sec_timer);
			gotoxy(12,1);
			printf("%02u:%02u:%02u", st.tm_hour, st.tm_min, st.tm_sec);
		}
		if (rc & GPS_UPD_PPS) {
			// MSB is even or odd second
			ppschar = nmea_rtc_lsbs() & 0x8000 ? '*' : ' ';
			gotoxy(12,2);
			printf("%c %5u", ppschar, nmea_rtc_lsbs() & 0x7FFF);
		}
		if (rc & GPS_UPD_UTC) {
			gotoxy(12,3);
			printf("%02u:%02u:%02u",
				nmea_utc()->tm_hour,
				nmea_utc()->tm_min,
				nmea_utc()->tm_sec);

		}
		if (nmea_status() != status) {
			status = nmea_status();
			gotoxy(12,4);
			switch (status) {
			default: printf("0: INIT   "); break;
			case 1:  printf("1: CONNECT"); break;
			case 2:  printf("2: UTC OK "); break;
			case 3:  printf("3: LOCKED "); break;
			}
		}
	}

}





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
/*
	samples\gps\gps.c

	Demonstrate basic location/time data provided by GPS receiver.

	This assumes the following hardware:
	. GPS receiver attached to serial port C with 4800bps 8N1
	  NMEA-0183 messages
	. PB4 is an active low reset to the GPS receiver

*/

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

void main(void)
{
	int rc;

	BitWrPortI(PBDDR, &PBDDRShadow, 1, 4); // GPSReset to Output
	BitWrPortI(PCDDR, &PCDDRShadow, 1, 2); // Serial C on PortC
	BitWrPortI(PCFR, &PCFRShadow, 1, 2); // Set PC2 to serial C out
	WrPortI(PCALR, &PCALRShadow, 0x00); // Make sure set to ALT0;
	 // Setup Port C for async serial communication
	WrPortI(SCCR, &SCCRShadow, 0x00);
	// GPS Reset SET GPS ON
	BitWrPortI(PBDR, &PBDRShadow, 1, 4);
	serCopen(NMEA_BAUD);

	printf("Waiting for GPS...\n\n");

	for (;;) {
		rc = nmea_tick(SER_PORT_C);

		if (rc > 0) {
			if (rc & GPS_UPD_UTC)
	         printf("UTC %04u/%02u/%02u,%02u:%02u:%02u\r\n"
	            , nmea_utc()->tm_year + 1900
	            , tm_mon2month(nmea_utc()->tm_mon)
	            , nmea_utc()->tm_mday
	            , nmea_utc()->tm_hour
	            , nmea_utc()->tm_min
	            , nmea_utc()->tm_sec
	            );
			if (rc & GPS_UPD_LOCATION)
	         printf("Lat %10.6f, Lon %10.5f, Alt %5.0fm, Satellites %u\r\n"
	            , nmea_lat() * (180/PI)
	            , nmea_lon() * (180/PI)
	            , nmea_alt()
	            , nmea_numsat()
	            );
		}
	}

}



void gps(void)
{
	static int setmsgs = 1;
	static int setrtc = 1;
	char buf[80];
	int len = serCrdUsed();
	if (len) {
		if (len > sizeof(buf))
			len = sizeof(buf);
		// Won't block...
		serCread(buf, len, 1);
		nmea_process(buf, len);
	}

	if (setmsgs) {
		if (nmea_status()) {
	      // Set Trimble Copernicus GPS to send GGA and ZDA messages every 5 sec
	      //serCputs("$PTNLSNM,0021,05*50\r\n");
	      setmsgs = 0;
	   }
	}
	if (setrtc) {
		if (nmea_status() > 1) {
			nmea_set_rtc(1, 1);
			setrtc = 0;
		}
	}
}


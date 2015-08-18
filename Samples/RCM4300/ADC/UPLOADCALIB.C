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
/***************************************************************************

	uploadcalib.c

	This sample program is for the RCM4300 series controllers with
	prototyping boards.

	Description
	===========
	This program demonstrates the reading of calibrations constants from
	a controller's user block in Flash and transmitting the file using a
	serial port utility such as Tera Term.

	Note: To download calibrations created by this program, use
			dnloadcalib.c

	This sample is not intended for "Code and BIOS in RAM" compile mode.

	The Tera Term serial utility can be downloaded from their WEB page
	located at:
	http://hp.vector.co.jp/authors/VA002416/teraterm.html

  	Hardware setup
  	==============
	1. Connect PC (tx) to the controller RXD on J4.
  	2. Connect PC (rx) to the controller TXD on J4.
	3. Connect PC GND to GND on J4.

	Tera Term setup
	===============
	1. Startup Tera Term on your PC.
	2. Configure the serial parameters for the following:
	   a) Baud Rate of 19200, 8 bits, no parity and 1 stop bit.
	   b) Enable the "Local Echo" option.
	   c) Set line feed options to:  Receive = CR     Transmit = CR+LF
 	3. Select File-->Log-->Path and Filename
		The next steps are important, if not done if will overwrite previously
		saved calibration data when using same the filename.
   	a) Enable the File APPEND option located at the bottom of dialog box.
   	b) Select the OPEN option at the right-hand side of dialog box.
   	c) Tera Term is now ready to log all data received on the serial
   		port to your specified file.

	Program Instructions
	====================
   1. Compile and run this program. (At this point you should see a
      message displayed in the Tera Term display window)

   2. In the Tera Term display window enter the serial number of the controller,
      then press the ENTER key. (At this point you should see the calibration
      data being displayed in the Tera Term display window)

   3. Once the program is done executing, choose CLOSE from the
      within the Tera Term LOG window (A separate pop-up window that would
      probably be minimized at the bottom of your PC screen)
      This will finish logging and close the file.

   4. Open your data file and verify that the calibration data has been
      written properly. For example:


	Serial port transmission
	========================
	Uploading calibration table . . .
	Enter the serial number of your controller = 9MN234

	::
	SN9MN234
	ADSE
	0
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	1
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
		|
		|

	ADDF
	0
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	2
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
	float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,float_gain,float_offset,
		|
		|

	ADMA
	3
	float_gain,float_offset,
	4
	float_gain,float_offset,
		|
		|

	END
	::
	End of table upload

***************************************************************************/
#define PRINTDEBUG 0		//define as 1 to show debug
#define TRANSMITFILE 1  //define as 1 to transmit over serial line

/////
// define your start and channels here
//	presently configured to prototyping boards defaults
///
#define STARTSE	0		//start single-ended lines
#define ENDSE		6		//end single-ended lines
#define STARTDIFF	0		//start differential lines
#define ENDDIFF	4		//end differental lines
#define STARTMA	3		//start milli-amp lines
#define ENDMA		6		//end milli-amp lines

/////
// configure your serial port connection here
//	presently configured serial port D
///
#define SERPORT		D
#define DINBUFSIZE	255
#define DOUTBUFSIZE	255

#define serXopen		CONCAT(ser, CONCAT(SERPORT, open))
#define serXclose		CONCAT(ser, CONCAT(SERPORT, close))
#define myport			CONCAT(SER_PORT_, SERPORT)
#define SXSR			CONCAT(S, CONCAT(SERPORT, SR))
#define INBUFSIZE		CONCAT(SERPORT, INBUFSIZE)
#define OUTBUFSIZE	CONCAT(SERPORT, OUTBUFSIZE)
#define BAUDRATE		19200l

// RCM43xx boards have no pull-up on serial Rx lines, and we assume in this
// sample the possibility of disconnected or non-driven Rx line.  This sample
// has no need of asynchronous line break recognition.  By defining the
// following macro we choose the default of disabled character assembly during
// line break condition.  This prevents possible spurious line break interrupts.
#define RS232_NOCHARASSYINBRK

#use RCM43xx.LIB
#ifndef ADC_ONBOARD
   #error "This core module does not have ADC support.  ADC programs will not "
   #fatal "   compile on boards without ADC support.  Exiting compilation."
#endif

main()
{
	auto int channel, gaincode;
	char tempbuf[64];
	char sendbuf[128];
	int ch,i;

	brdInit();
	serXopen(BAUDRATE);
	serXrdFlush(myport);
	serXwrFlush(myport);

	if (PRINTDEBUG)
	{
		printf("ADC Block size = 0x%x\n", 4096*GetIDBlockSize());

		printf("ADC single-ended address 0x%x\n", ADC_CALIB_ADDRS);		//single-ended address start
		printf("ADC differential address 0x%x\n", ADC_CALIB_ADDRD);		//differential address start
		printf("ADC milli-Amp address 0x%x\n", ADC_CALIB_ADDRM);			//milli-amp address start

		printf("\nRead constants from user block\n");
	}
	anaInEERd(ALLCHAN, SINGLE, gaincode);			//read all single-ended
	anaInEERd(ALLCHAN, DIFF, gaincode);				//read all differential
	anaInEERd(ALLCHAN, mAMP, gaincode);				//read all milli-amp

	memset(sendbuf, '\x0', sizeof(sendbuf));
	while(serXrdFree(myport) != INBUFSIZE) serXgetc(myport);

	// Send data out the serial port to the PC
	serXputs(myport, "Uploading calibration table . . .\n\r\x0");
	serXputs(myport, "Enter the serial number of your controller = \x0");

	ch = 0;
	i=0;
	while (ch != '\r')
	{
		// make sure you have local echo enabled in Tera Term, so that the
	  	// serial number will written to the calibration file.
		while ((ch = serXgetc(myport)) == -1);
		// Check for a BACKSPACE...allow editing of the serial number
		if (ch == '\b' && i > 0)
		{
			--i;
		}
		else
			tempbuf[i++] = ch;
	}

	tempbuf[i] = '\x0';
	sprintf(sendbuf, "\n\r::\n\rSN");
	strcat(sendbuf, tempbuf);
	strcat(sendbuf, "\n\r\x0");
	serXputs(myport, sendbuf);

	////
	if (PRINTDEBUG) {
		printf("\n\nFormatting single-ended channels to transmit\n");
	}

	sprintf(sendbuf, "ADSE\n\r\x0");
	serXputs(myport, sendbuf);

	for(channel = STARTSE; channel <= ENDSE; channel++)
	{
		memset(sendbuf, '\x0', sizeof(sendbuf));
		sprintf(sendbuf, "%d\n\r\x0", channel);
		serXwrite(myport, sendbuf, sizeof(sendbuf));

		for(gaincode = 0; gaincode < 8; gaincode++)
		{
			memset(sendbuf, '\x0', sizeof(sendbuf));
			sprintf(sendbuf, "%9.6f,%9.6f,", _adcCalibS[channel][gaincode].kconst, _adcCalibS[channel][gaincode].offset);
			if (gaincode == 3 || gaincode == 7)
				strcat(sendbuf, "\n\r\x0");
			serXwrite(myport, sendbuf, sizeof(sendbuf));
		}
	}

	////
	if(PRINTDEBUG) {
		printf("\n\nFormatting differential channels to transmit\n");
	}

	sprintf(sendbuf, "ADDF\n\r\x0");
	serXputs(myport, sendbuf);

	for(channel = STARTDIFF; channel <= ENDDIFF; channel+=2)
	{
		memset(sendbuf, '\x0', sizeof(sendbuf));
		sprintf(sendbuf, "%d\n\r\x0", channel);
		serXwrite(myport, sendbuf, sizeof(sendbuf));

		for(gaincode = 0; gaincode < 8; gaincode++)
		{
			memset(sendbuf, '\x0', sizeof(sendbuf));
			sprintf(sendbuf, "%9.6f,%9.6f,", _adcCalibD[channel][gaincode].kconst, _adcCalibD[channel][gaincode].offset);
			if (gaincode == 3 || gaincode == 7) {
				strcat(sendbuf, "\n\r\x0");
			}
			serXwrite(myport, sendbuf, sizeof(sendbuf));
		}
	}

	////
	if (PRINTDEBUG) {
		printf("\n\nFormatting milli-Amp channels to transmit\n");
	}

	sprintf(sendbuf, "ADMA\n\r\x0");
	serXputs(myport, sendbuf);

	for(channel = STARTMA; channel <= ENDMA; channel++)
	{
		memset(sendbuf, '\x0', sizeof(sendbuf));
		sprintf(sendbuf, "%d\n\r\x0", channel);
		serXwrite(myport, sendbuf, sizeof(sendbuf));

		sprintf(sendbuf, "%9.6f,%9.6f,\n\r\x0", _adcCalibM[channel].kconst, _adcCalibM[channel].offset);
		serXwrite(myport, sendbuf, sizeof(sendbuf));
	}

	sprintf(sendbuf, "END\n\r::\n\r\x0");
	serXputs(myport, sendbuf);

	serXputs(myport, "End of table upload\n\n\r\x0");

	// Make sure all data gets transmitted before exiting the program
	while (serXwrFree(myport) != OUTBUFSIZE);
   while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
	serXclose();
}


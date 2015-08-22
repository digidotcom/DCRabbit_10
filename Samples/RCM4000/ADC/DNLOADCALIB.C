/**************************************************************************

	dnloadcalib.c
	Rabbit Semiconductor, 2006

	This sample program is for the RCM4000 series controllers with
	prototyping boards.

	This program demonstrates how to retrieve your analog calibration data
 	to rewrite it back to simulated eeprom in flash with using a serial
 	utility such as Tera Term.

	Description
	===========
	This program demonstrates the sending of a data file to download
	calibrations constants to a controller's user block in Flash and
	by transmitting the file using a	serial port a PC serial utility
	such as Tera Term.

	Note: To upload calibrations to be used by this program, use
			uploadcalib.c

	!!!This program must be compiled to Flash.

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

	Program Instructions
   ====================
   1. Compile and run this program. Verify that the message "Waiting,
      Please Send Data file" message is being display in Tera Term
      display window before proceeding.

   2. From within Tera Term do the following:
   	- Select...File-->Send File-->Path and filename
      a) Select the OPEN option within dialog box.

	3. Once the data file has been downloaded it will indicate if the
		calibration data was successfully written.

	Example dat file
	================

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

**************************************************************************/
#class auto

#define ADC_SCLKBRATE 115200ul
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

// RCM40xx boards have no pull-up on serial Rx lines, and we assume in this
// sample the possibility of disconnected or non-driven Rx line.  This sample
// has no need of asynchronous line break recognition.  By defining the
// following macro we choose the default of disabled character assembly during
// line break condition.  This prevents possible spurious line break interrupts.
#define RS232_NOCHARASSYINBRK

#define FILEBUFSIZE	4096	//4K max file size
#define TERMINATOR	'\n'

#use RCM40xx.LIB
#ifndef ADC_ONBOARD
   #error "This core module does not have ADC support.  ADC programs will not "
   #fatal "   compile on boards without ADC support.  Exiting compilation."
#endif

char string[128];
char buffer[128];

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long time0;

	for (time0 = MS_TIMER; MS_TIMER - time0 < delay; ) ;
}

/////////////////////////////////////////////////////////////////////
// Calibration data error handler
/////////////////////////////////////////////////////////////////////
void caldata_error(char *ptr, char *msg)
{
	memset(ptr, 0x20, 80);
	ptr[0]  = '\r';
	ptr[80] = '\0';
	serXwrite(myport, ptr, strlen(ptr));
	sprintf(ptr, msg);
	serXwrite(myport, ptr, strlen(ptr));

	// Make sure all data gets transmitted before exiting the program
	while (serXwrFree(myport) != OUTBUFSIZE);
   while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
}


/////////////////////////////////////////////////////////////////////
// Locate the calibration data within the file using known
// identifier TAGS.
/////////////////////////////////////////////////////////////////////
unsigned long find_tag(unsigned long fileptr, long len)
{
	auto char data[2047];
	auto long index,i;
	auto char *begptr, *firstline, *secondline, *saveptr;
	auto int eofile, eoline, nextcase, dnstate, channel, gaincode;

	index = 0;
	xmem2root(data, fileptr, (int)len);
	begptr = strtok(data, "\n\r");		//begin data file
	while (strncmp(begptr, "::", 2))		//look for start
	{
		begptr = strtok(NULL, "\n\r");
	}
	begptr = strtok(NULL, "\n\r");
	serXputs(myport, "\n\rData file serial number is \x0");
	serXwrite(myport, begptr, strlen(begptr));

	eofile = FALSE;
	saveptr = begptr+strlen(begptr)+1;
	while (!eofile)
	{
		eoline = FALSE;
		nextcase = 0;
		begptr = strtok(saveptr, "\n\r");
		saveptr = begptr+strlen(begptr)+1;
		if (!strncmp(begptr, "ADSE", 4))
		{
			dnstate = 2;
		}
		else
			if (!strncmp(begptr, "ADDF", 4))
			{
				dnstate = 3;
			}
			else
				if (!strncmp(begptr, "ADMA", 4))
				{
					dnstate = 4;
				}
				else
					if (!strncmp(begptr, "END", 3))
					{
						eofile = TRUE;
						eoline = TRUE;
					}
					else
						nextcase = 1;

		while (!eoline)
		{
			switch (nextcase)
			{
				case 2:		//single ended
					firstline = strtok(NULL, "\n\r");
					secondline = strtok(NULL, "\n\r");
					for (gaincode = 0; gaincode <= 3; gaincode++)
					{
						begptr = strtok(firstline, ",");
						_adcCalibS[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibS[channel][gaincode].offset = atof(begptr);
						firstline = begptr+strlen(begptr)+1;
					}

					for (gaincode = 4; gaincode <= 7; gaincode++)
					{
						begptr = strtok(secondline, ",");
						_adcCalibS[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibS[channel][gaincode].offset = atof(begptr);
						secondline = begptr+strlen(begptr)+1;
					}
					saveptr = secondline+1;
					eoline = TRUE;
					break;
				case 3:		//differential
					firstline = strtok(NULL, "\n\r");
					secondline = strtok(NULL, "\n\r");
					for (gaincode = 0; gaincode <= 3; gaincode++)
					{
						begptr = strtok(firstline, ",");
						_adcCalibD[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibD[channel][gaincode].offset = atof(begptr);
						firstline = begptr+strlen(begptr)+1;
					}
					for (gaincode = 4; gaincode <= 7; gaincode++)
					{
						begptr = strtok(secondline, ",");
						_adcCalibD[channel][gaincode].kconst = atof(begptr);
						begptr = strtok(NULL, ",");
						_adcCalibD[channel][gaincode].offset = atof(begptr);
						secondline = begptr+strlen(begptr)+1;
					}
					saveptr = secondline+1;
					eoline = TRUE;
					break;
				case 4:		//milli-amp
					firstline = strtok(NULL, "\n\r");
					begptr = strtok(firstline, ",");
					_adcCalibM[channel].kconst = atof(begptr);
					begptr = strtok(NULL, ",");
					_adcCalibM[channel].offset = atof(begptr);
					saveptr = begptr+strlen(begptr)+2;
					eoline = TRUE;
					break;
				case 1:
					channel = atoi(begptr);
					nextcase = dnstate;
					eoline = FALSE;
					break;
				case 0:
					eoline = TRUE;
					break;
			} //switch

		} //while not eoline
	} //while not eofile

	anaInEEWr(ALLCHAN, SINGLE, gaincode);			//read all single-ended
	anaInEEWr(ALLCHAN, DIFF, gaincode);				//read all differential
	anaInEEWr(ALLCHAN, mAMP, gaincode);				//read all milli-amp

}


/////////////////////////////////////////////////////////////////////
//	Read the file from the serial port
/////////////////////////////////////////////////////////////////////
unsigned long getfile( unsigned long xmem_ptr )
{
	auto char buffer[256];
	auto unsigned int len;
	auto unsigned int total_len, file_done;

	serXrdFlush(myport);
	while (serXrdFree(myport) == INBUFSIZE);
	total_len = 0;
	file_done = FALSE;
	while(!file_done)
	{
		// Use the serial timeout to determine that the dowload is completed
		if((len = serXread(myport, buffer, 256, 100)) < 256)
		{
			file_done = TRUE;
		}
		if(!file_done)	msDelay(100);

		// Move data to large XMEM buffer
		root2xmem((xmem_ptr+total_len), buffer, len);
		total_len += len;
	}
	return(total_len);
}


/////////////////////////////////////////////////////////////////////
//	Retrieve analog calibration data and rewrite to the flash
/////////////////////////////////////////////////////////////////////
void main()
{
	auto unsigned long fileptr, tempPtr, xmemPtr, index;
	auto unsigned long len;
	auto int i;
	auto char serialNumber[64];

	//------------------------------------------------------------------------
	//		Initialize the Controller
	//------------------------------------------------------------------------
	brdInit();
	serXopen(BAUDRATE);	//set baud rates for the serial ports to be used
	serXwrFlush(myport);		//clear Rx and Tx data buffers
	serXrdFlush(myport);

	//------------------------------------------------------------------------
	//		Allocate and Clear XMEM
	//------------------------------------------------------------------------

	// Allocate XMEM memory for the file that will be read in from the PC
	xmemPtr = xalloc(FILEBUFSIZE);

	// Clear the buffer in XMEM
	for(index =0; index < FILEBUFSIZE; index++)
	{
		root2xmem(xmemPtr + index, "\x00", 1);
	}

	//------------------------------------------------------------------------
	//		Download the Data File from the PC
	//------------------------------------------------------------------------
	sprintf(string, "\r\nWaiting...Please Send Data file\n\r");
	serXwrite(myport, string, strlen(string));

	// Get the calibration data file from the PC and put it into XMEM
	if(!(len = getfile(xmemPtr)))
	{
		caldata_error(string, "\r\n\nEncounter an error while reading calibration file");
		exit(1);
	}
	fileptr = xmemPtr;
	sprintf(string, "\r\n\nDownload Complete\n\n\r");
	serXwrite(myport, string, strlen(string));

	//------------------------------------------------------------------------
	//	 Parse data file and write to calibrations to flash
	//------------------------------------------------------------------------
	sprintf(string, "\r\nParsing data file\n\r");
	serXwrite(myport, string, strlen(string));

	tempPtr = find_tag(fileptr, len);

	sprintf(string, "\r\n\nExiting....Calibration data successfully written\n\n\r");
	serXwrite(myport, string, strlen(string));
	while (serXwrFree(myport) != OUTBUFSIZE);
   while((RdPortI(SXSR)&0x08) || (RdPortI(SXSR)&0x04));
	serXclose();
}
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
 	dac_rd_caldata.c

	This sample program is for the BLxS2xx series controllers.

	Description:
	============
	This program dumps the calibration data for all the DAC channels
   and the modes of operation. The program will display the calibration
   gain factor and offset value via the STDIO window for each channel
   and mode of operation.

	Instructions:
	=============
	1. Compile and run this program.
	2. View STDIO window for calibration data values.

***************************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series lbrary
#use "BLxS2xx.lib"

main ()
{
   auto int i;
   auto calib_t cal_data[2];

   // Initialize controller
	brdInit();

   printf("Calibration data, DAC unipolar voltage operation\n");
   printf("------------------------------------------------\n");

   // Clear memory
   memset((char*)&cal_data, 0x00, sizeof(cal_data));

   // Read calibration data for DAC channels 0 & 1,
   // unipolar output configuration.
  	_anaOutEERd(0, DAC_VOLT0_INDEX, &cal_data[0]);
  	_anaOutEERd(1, DAC_VOLT0_INDEX, &cal_data[1]);
  	printf("CH = 0  Gain = %f  Offset = %d\n",
             cal_data[0].gain, cal_data[0].offset);
  	printf("CH = 1  Gain = %f  Offset = %d\n\n\n",
             cal_data[1].gain, cal_data[1].offset);

   printf("Calibration data, DAC bipolar voltage operation\n");
   printf("-----------------------------------------------\n");

   // Clear memory
	memset((char*)&cal_data, 0x00, sizeof(cal_data));

   // Read calibration data for DAC channels 0 & 1,
   // bipolar output configuration.
  	_anaOutEERd(0, DAC_VOLT1_INDEX, &cal_data[0]);
  	_anaOutEERd(1, DAC_VOLT1_INDEX, &cal_data[1]);
  	printf("CH = 0  Gain = %f  Offset = %d\n",
             cal_data[0].gain, cal_data[0].offset);
  	printf("CH = 1  Gain = %f  Offset = %d\n\n\n",
             cal_data[1].gain, cal_data[1].offset);

   printf("Calibration data, DAC unipolar 4-20ma operation\n");
   printf("-----------------------------------------------\n");

   // Clear memory
	memset((char*)&cal_data, 0x00, sizeof(cal_data));

   // Read calibration data for DAC channels 0 & 1,
   // bipolar output configuration.
  	_anaOutEERd(0, DAC_mAMPS_INDEX, &cal_data[0]);
  	_anaOutEERd(1, DAC_mAMPS_INDEX, &cal_data[1]);
  	printf("CH = 0  Gain = %f  Offset = %d\n",
             cal_data[0].gain, cal_data[0].offset);
  	printf("CH = 1  Gain = %f  Offset = %d\n\n\n",
             cal_data[1].gain, cal_data[1].offset);

   // Wait for any key from keyboard to exit program
   while(!kbhit());
   while(kbhit()) { getchar(); }
}
///////////////////////////////////////////////////////////////////////////


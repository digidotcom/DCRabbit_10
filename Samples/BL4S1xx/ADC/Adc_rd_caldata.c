/***************************************************************************
 	adc_rd_caldata.c
	Digi International, Copyright © 2008.  All rights reserved.

	This sample program is for the BL4S1xx series SBCs.

	Description:
	============
	This program dumps the calibration data for all the ADC channels
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

// include series BL4S1xx library
#use "BL4S1xx.LIB"

main ()
{
   auto int gaincode, channel, opmode;
   calib_t cal_data;

  	// Initialize the controller
	brdInit();
   printf("\n\n");
   printf("NOTE: If Scale = ********* then the channel is\n");
   printf("      not calibrated for this particular gain.\n\n\n");

   printf("Calibration data for ADC set for unipolar operation\n");
   printf("---------------------------------------------------\n");
   memset((char*)&cal_data, 0x00, sizeof(cal_data));
  	for (channel = 0; channel < BL_ANALOG_IN; ++channel)
   {
      printf("Channel %d:\n", channel);
    	for (gaincode = 0; gaincode < BL_MAX_GAINS; ++gaincode)
   	{
   		anaInRdCalib(channel, SE0_MODE, gaincode, &cal_data);
   		printf("    GAIN = %d  Scale = %9.6f  Offset = %d\n",
                    gaincode, cal_data.gain, cal_data.offset);
     	}
   }

   printf("\n\n");
   printf("Calibration data for ADC set for differential operation\n");
   printf("-------------------------------------------------------\n");
   memset((char*)&cal_data, 0x00, sizeof(cal_data));
  	for (channel = 0; channel < BL_ANALOG_IN; channel += 2)
   {
      printf("Channel Pair %d-%d:\n", channel, channel + 1);
      for (gaincode = 0; gaincode < BL_MAX_GAINS; ++gaincode)
   	{
   		anaInRdCalib(channel, DIFF_MODE, gaincode, &cal_data);
   		printf("    GAIN = %d  Scale = %9.6f  Offset = %d\n",
                    gaincode, cal_data.gain, cal_data.offset);
   	}
   }

   printf("\n\n");
   printf("Calibration data for ADC set for 4-20ma operation\n");
   printf("-------------------------------------------------\n");
   memset((char*)&cal_data, 0x00, sizeof(cal_data));
   for (channel = 0; channel < BL_ANALOG_4TO20; ++channel)
   {
   	anaInRdCalib(channel, mAMP_MODE, mAMP_GAINCODE, &cal_data);
   	printf("CH = %d  GAIN = %d  Scale = %9.6f  Offset = %d\n",
             channel, gaincode, cal_data.gain, cal_data.offset);
   }
}
///////////////////////////////////////////////////////////////////////////


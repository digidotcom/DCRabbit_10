/*
   Copyright (c) 2019, Digi International Inc.

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
	Default calibration constants for BL4S200 hardware.  Written April/May 2019.
	
	Calibration constants in this file obtained using programs from
	Samples/BLxS2xx and manually calibrating a single BL4S200 board.

	This file provides "sane" defaults for situations where it isn't possible
	to calibrate hardware.  For accurate ADC inputs and DAC outputs, Digi
	International recommends calibrating all modes intended for use on a
	given piece of hardware.
*/

#use "BLxS2xx.LIB"

typedef calib_t calib_adc_se_unipolar_t[BL_MAX_GAINS][BL_ANALOG_IN];
// se_bipolar only uses gain 0-5 (but allocates UserBlock space for all 8)
typedef calib_t calib_adc_se_bipolar_t[6][BL_ANALOG_IN];
typedef calib_t calib_adc_differential[BL_MAX_GAINS][BL_ANALOG_IN / 2];
typedef calib_t calib_adc_current_t[BL_ANALOG_4TO20];
// Although BL4S200 defines BL_MAX_DAC as 8, the hardware only has 2 DACs.
typedef calib_t calib_dac_unipolar_t[2];
typedef calib_t calib_dac_bipolar_t[2];
typedef calib_t calib_dac_current_t[2];

const far calib_adc_se_unipolar_t default_adc_se_unipolar = {
	// single-ended unipolar, channel 0-8 for 8 gain codes
	// These values from Tom Collins 2019-04-24
	{  // gain 0                                                                    
	   { -1.24409934e-02, 0 }, { -1.25187495e-02, 8 },                              
	   { -1.25187495e-02, 6 }, { -1.21762910e-02, 0 },                              
	   { -1.23261539e-02, 0 }, { -1.22883440e-02, 2 },                              
	   { -1.22507652e-02, -2 }, { -1.21762910e-02, -3 },                            
	},                                                                              
	{  // gain 1                                                                    
	   { -6.18711710e-03, 0 }, { -6.11212150e-03, 2 },                              
	   { -6.13069860e-03, 3 }, { -6.14939020e-03, 7 },                              
	   { -6.13069860e-03, 0 }, { -6.11212150e-03, -1 },                             
	   { -6.13069860e-03, 0 }, { -6.14939020e-03, 2 },                              
	},                                                                              
	{  // gain 2                                                                    
	   { -3.08132520e-03, 2 }, { -3.07207158e-03, 9 },                              
	   { -3.07207158e-03, 10 }, { -3.03560798e-03, 10 },                            
	   { -3.03560798e-03, -2 }, { -3.08132520e-03, 4 },                             
	   { -3.06287384e-03, 2 }, { -3.04464274e-03, 1 },                              
	},                                                                              
	{  // gain 3                                                                    
	   { -2.47005955e-03, 3 }, { -2.44807149e-03, 13 },                             
	   { -2.44807149e-03, 12 }, { -2.44082813e-03, 17 },                            
	   { -2.44807149e-03, 1 }, { -2.43362854e-03, 0 },                              
	   { -2.46268627e-03, 5 }, { -2.44807149e-03, 2 },                              
	},                                                                              
	{  // gain 4                                                                    
	   { -1.54838745e-03, 9 }, { -1.53488398e-03, 22 },                             
	   { -1.53935875e-03, 24 }, { -1.54385960e-03, 35 },                            
	   { -1.53488398e-03, 5 }, { -1.52161403e-03, 2 },                              
	   { -1.53488398e-03, 7 }, { -1.53488398e-03, 8 },                              
	},                                                                              
	{  // gain 5                                                                    
	   { -1.23631127e-03, 10 }, { -1.22571445e-03, 30 },                            
	   { -1.22571445e-03, 28 }, { -1.21875002e-03, 35 },                            
	   { -1.22922647e-03, 8 }, { -1.22222246e-03, 7 },                              
	   { -1.22571445e-03, 12 }, { -1.21875002e-03, 8 },                             
	},                                                                              
	{  // gain 6                                                                    
	   { -7.71043760e-04, 15 }, { -7.63333340e-04, 45 },                            
	   { -7.65886330e-04, 45 }, { -7.63333340e-04, 61 },                            
	   { -7.63333340e-04, 12 }, { -7.60797460e-04, 14 },                            
	   { -7.65886330e-04, 20 }, { -7.63333340e-04, 19 },                            
	},                                                                              
	{  // gain 7                                                                    
	   { -6.18918970e-04, 24 }, { -6.15591470e-04, 62 },                            
	   { -6.10666750e-04, 55 }, { -6.09042530e-04, 77 },                            
	   { -6.12299480e-04, 19 }, { -6.13941050e-04, 20 },                            
	   { -6.12299480e-04, 27 }, { -6.12299480e-04, 26 },                            
	},                                                                              
};

const far calib_adc_se_bipolar_t default_adc_se_bipolar = {
	// single-ended bipolar, channel 0-8 for 6 gain codes
	// These values from Tom Collins 2019-04-24
	{  // gain 0
	   { -1.23148141e-02, 866 }, { -1.22392652e-02, 868 },
	   { -1.22769223e-02, 868 }, { -1.22580630e-02, 869 },
	   { -1.22580630e-02, 868 }, { -1.22205214e-02, 865 },
	   { -1.22580630e-02, 867 }, { -1.22958393e-02, 868 },
	},
	{  // gain 1
	   { -6.15763520e-03, 852 }, { -6.11995020e-03, 857 },
	   { -6.13496960e-03, 857 }, { -6.12745100e-03, 857 },
	   { -6.13496960e-03, 853 }, { -6.11246980e-03, 855 },
	   { -6.13496960e-03, 853 }, { -6.13496960e-03, 852 },
	},
	{  // gain 2
	   { -3.07774380e-03, 821 }, { -3.05909081e-03, 834 },
	   { -3.07305878e-03, 831 }, { -3.05909081e-03, 833 },
	   { -3.06838867e-03, 824 }, { -3.04984883e-03, 824 },
	   { -3.06373299e-03, 822 }, { -3.07774380e-03, 823 },
	},
	{  // gain 3
	   { -2.45978753e-03, 809 }, { -2.44494737e-03, 818 },
	   { -2.45234486e-03, 816 }, { -2.44864053e-03, 821 },
	   { -2.44864053e-03, 809 }, { -2.43759411e-03, 808 },
	   { -2.45234486e-03, 809 }, { -2.45234486e-03, 809 },
	},
	{  // gain 4
	   { -1.54216878e-03, 768 }, { -1.53523252e-03, 784 },
	   { -1.53064297e-03, 780 }, { -1.53523252e-03, 792 },
	   { -1.52835820e-03, 765 }, { -1.52835820e-03, 765 },
	   { -1.53523252e-03, 767 }, { -1.53523252e-03, 770 },
	},
	{  // gain 5
	   { -1.23802397e-03, 741 }, { -1.22700317e-03, 763 },
	   { -1.23065488e-03, 759 }, { -1.22700317e-03, 773 },
	   { -1.22518523e-03, 740 }, { -1.22518523e-03, 740 },
	   { -1.22518523e-03, 744 }, { -1.22700317e-03, 745 },
	},
};

const far calib_adc_differential default_adc_differential = {
	// single-ended bipolar, channel 0-8 for 6 gain codes
	// These values from Tom Collins 2019-04-27
	{  // gain 0
	   { -1.23200835e-02, -4 }, { -1.22863994e-02, -4 },
	   { -1.22528960e-02, -3 }, { -1.23116439e-02, -1 },
	},
	{  // gain 1
	   { -6.15279600e-03, -4 }, { -6.14860230e-03, -2 },
	   { -6.13188230e-03, 0 }, { -6.14023060e-03, 0 },
	},
	{  // gain 2
	   { -3.07845720e-03, -8 }, { -3.07436869e-03, -3 },
	   { -3.06216883e-03, -1 }, { -3.07641155e-03, -5 },
	},
	{  // gain 3
	   { -2.46603251e-03, -9 }, { -2.45768413e-03, -5 },
	   { -2.45270249e-03, 0 }, { -2.45768413e-03, -1 },
	},
	{  // gain 4
	   { -1.54175295e-03, -18 }, { -1.53750868e-03, -11 },
	   { -1.53223599e-03, -2 }, { -1.53539527e-03, -4 },
	},
	{  // gain 5
	   { -1.23205909e-03, -26 }, { -1.23040855e-03, -14 },
	   { -1.22876244e-03, -2 }, { -1.22958503e-03, -2 },
	},
	{  // gain 6
	   { -7.69174130e-04, -48 }, { -7.65223810e-04, -23 },
	   { -7.65785630e-04, -5 }, { -7.72592660e-04, -3 },
	},
	{  // gain 7
	   { -6.14712300e-04, -51 }, { -6.13818230e-04, -28 },
	   { -6.12926720e-04, 3 }, { -6.15609050e-04, 0 },
	},
};

const far calib_adc_current_t default_adc_current = {
	// channel 0-3, using a single gain code
	// These values from Tom Collins 2019-04-25
   { -6.15853630e-02, 9 }, { -6.08433740e-02, 22 },
   { -6.07228980e-02, 21 }, { -6.07228980e-02, 29 },
};

const far calib_dac_unipolar_t default_dac_unipolar = {
	// DAC unipolar voltage, 2 channels
	// These values from Tom Collins 2019-04-30
	{ 2.66250013e-03, 3935 }, { 2.66666687e-03, 3938 },
};

const far calib_dac_bipolar_t default_dac_bipolar = {
	// DAC bipolar voltage, 2 channels
	// These values from Tom Collins 2019-04-30
	{ 5.32326240e-03, 2040 }, { 5.30871700e-03, 2043 },
};

const far calib_dac_current_t default_dac_current = {
	// DAC unipolar current (4-20mA), 2 channels
	// These values from Tom Collins 2019-04-30
	{ 4.17065380e-03, 4949 }, { 4.17802710e-03, 4942 },
};


// Helper function for read_calibration().
void dump_calibration(unsigned offset, unsigned gains, unsigned channels)
{
	calib_t readings[64];		// max of 8 gains and 8 channels
	calib_t *entry;
	int gain, channel;
	
	if (gains * channels > 64) {
		printf("!!! TOO BIG !!!\n");
	}
	
	entry = &readings[0];
	readUserBlock(entry, offset, gains * channels * sizeof(calib_t));
	for (gain = 0; gain < gains; ++gain) {
		if (gains > 1) printf("{  // gain %u", gain);
		for (channel = 0; channel < channels; ++channel, ++entry) {
			if (channel % 2 == 0) {
				printf("\n   ");
			}
			printf("{ %.8e, %d }, ", entry->gain, entry->offset);
		}
		if (gains > 1) printf("\n},");
		printf("\n");
	}
}


// Read current BL4S200 ADC/DAC calibration data and print out in a format
// matching the constants at the top of this program.
void read_calibration(void)
{
	printf("\n adc_se_unipolar:\n");
	dump_calibration(CAL_ADC_SE0, 8, 8);

	printf("\n default_adc_se_bipolar:\n");
	dump_calibration(CAL_ADC_SE1, 6, 8);

	printf("\n default_adc_differential:\n");
	dump_calibration(CAL_ADC_DIFF, 8, 4);

	printf("\n default_adc_current:\n");
	dump_calibration(CAL_ADC_MA, 1, BL_ANALOG_4TO20);

	printf("\n default_dac_unipolar:\n");
	dump_calibration(CAL_DACV_UNIPOLAR, 1, 2);

	printf("\n default_dac_bipolar:\n");
	dump_calibration(CAL_DACV_BIPOLAR, 1, 2);
	
	printf("\n default_dac_current:\n");
	dump_calibration(CAL_DACI_UNIPOLAR, 1, 2);
}


#define writeCalibration(offset, var) \
	writeUserBlock(offset, &var, sizeof(var));

// replace all BL4S200 ADC/DAC calibration data with reasonable defaults
void write_default_calibration(void)
{
	// use any of these calls to fill in missing sections of constants
	writeCalibration(CAL_ADC_SE0, default_adc_se_unipolar);
	writeCalibration(CAL_ADC_SE1, default_adc_se_bipolar);
	writeCalibration(CAL_ADC_DIFF, default_adc_differential);
	writeCalibration(CAL_ADC_MA, default_adc_current);
	writeCalibration(CAL_DACV_UNIPOLAR, default_dac_unipolar);
	writeCalibration(CAL_DACV_BIPOLAR, default_dac_bipolar);
	writeCalibration(CAL_DACI_UNIPOLAR, default_dac_current);
}


// overwrite all BL4S200 ADC/DAC calibration data with 0xFF
void erase_calibration_data(void)
{
	// Block of 0xFF the size of the calibration constants
	unsigned char all_FF[3 * CAL_ANALOG_SIZE * BL_ANALOG_IN * BL_MAX_GAINS
	                     + CAL_ANALOG_SIZE * BL_ANALOG_4TO20
	                     + 3 * CAL_ANALOG_SIZE * BL_MAX_DAC];
	memset(all_FF, 0xFF, sizeof all_FF);
	writeUserBlock(CAL_ADC_SE0, all_FF, sizeof all_FF);
}


void main()
{
	// Show current calibration data for this hardware.
	read_calibration();
	
	// Call erase_calibration_data() to erase existing calibration constants,
	// write_default_calibration() to replace existing constants with defaults,
	// or any of the writeCalibration() calls inside write_default_calibration()
	// to replace just those constants.
}


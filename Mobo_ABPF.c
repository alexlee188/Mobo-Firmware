//*********************************************************************************
//**
//** Project.........: USB controller firmware for the Softrock 6.3 SDR,
//**                   enhanced with the 9V1AL Motherboard, F6ITU LPF bank
//**                   and other essentials to create an all singing and
//**                   all dancing HF SDR amateur radio transceiver
//**
//**                   Initial Core project team: 9V1AL, F6ITU, KF4BQ, KY1K
//**                   TF3LJ & many more 
//**
//** Platform........: AT90USB162 @ 16MHz
//**
//** Licence.........: This software is freely available for non-commercial 
//**                   use - i.e. for research and experimentation only!
//**
//** History &
//** Acknowledgements: The Mobo code is a further development of the firmware
//**                   initially developed for the ATtiny45 by Thomas Baier, DG8SAQ,
//**                   and then much refined by F.W. Krom, PE0FKO.
//**                   The code, although bloated up to three times its original
//**                   size to cater for the functions needed for a "full featured"
//**                   SDR transceiver, still retains the original command
//**                   structure and is fully compatible with V 15.10 of the
//**                   of the PE0FKO code.
//**                   Thanks Fred!!!
//**
//**
//**
//** Initial version.: 2009-08-23, Loftur Jonasson, TF3LJ
//** Last update.....: 2010-05-23, Loftur Jonasson, TF3LJ
//**                   Check the Mobo.c file
//**
//*********************************************************************************


#include "Mobo.h"


//
//-----------------------------------------------------------------------------
//			Set Band Pass and Low Pass filters
//-----------------------------------------------------------------------------
//
#if CALC_BAND_MUL_ADD							// Band dependent Frequency Subtract and Multiply
uint8_t SetFilter(uint32_t freq)
#else
void SetFilter(uint32_t freq)
#endif
{
	#if I2C_08_FILTER_IO						// TX filter controls over I2C
	sint16_t band_sel;
	#endif

	#if I2C_16_FILTER_IO						// TX filter controls over I2C
	sint16_t band_sel;
	#endif

	#if CALC_BAND_MUL_ADD						// Band dependent Frequency Subtract and Multiply
	uint8_t freqBand, i;
	#else
	uint8_t i;
	#endif

	#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
	uint8_t data;
	#endif

	sint32_t Freq;

	Freq.dw = freq;								// Freq.w1 is 11.5bits
		
	//-------------------------------------------	
	// Set RX Band Pass filters
	//-------------------------------------------
	for (i = 0; i < 7; i++)
	{
		if (Freq.w1.w < R.FilterCrossOver[i].w) break;
	}
	#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
	data = R.FilterNumber[i] & 0x07;			// We only want 3 bits
	pcf_data_out = pcf_data_out & 0xf8;			// clear and leave upper 5 bits untouched
	MoboPCF_set(data);							// Combine the two and write out
	selectedFilters[0] = R.FilterNumber[i];		// Used for LCD Print indication
	#else
	//i = i & 0x07;								// We only want 3 bits
	pcf_data_out = pcf_data_out & 0xf8;			// clear and leave upper 5 bits untouched
	MoboPCF_set(i);								// Combine the two and write out
	selectedFilters[0] = i;						// Used for LCD Print indication
	#endif

	#if CALC_BAND_MUL_ADD						// Band dependent Frequency Subtract and Multiply
	freqBand = i;								// Band info used for Freq Subtract/Multiply feature
	#endif

	//-------------------------------------------
	// Set TX Low Pass filters
	//-------------------------------------------
	for (i = 0; i < TXF - 1; i++)
	{
		if (Freq.w1.w < R.TXFilterCrossOver[i].w) break;
	}
	
	#if DDRD_TX_FILTER_IO						// TX filter controls on PortD
	IO_DDR_LPF |= (0x0f << IO_TX_LPF);
	IO_PORT_LPF = (IO_PORT_LPF & ~(0x0f << IO_TX_LPF)) | (i<<IO_TX_LPF);
	#endif// DDRD_TX_FILTER_IO
	
	#if I2C_08_FILTER_IO						// TX filter controls over I2C
	#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
	band_sel.w = 1<<R.TXFilterNumber[i];		// Set bit in a 16 bit register
	pcf8574_byte(R.PCF_I2C_lpf1_addr, band_sel.b0);
	selectedFilters[1] = R.TXFilterNumber[i];	// Used for LCD Print indication
	#else
	band_sel.w = 1<<i;							// Set bit in a 16 bit register
	pcf8574_byte(R.PCF_I2C_lpf1_addr, band_sel.b0);
	selectedFilters[1] = i;						// Used for LCD Print indication
	#endif// SCRAMBLED_FILTERS
	#endif// I2C_08_FILTER_IO

	#if I2C_16_FILTER_IO						// TX filter controls over I2C
	#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
	band_sel.w = 1<<R.TXFilterNumber[i];		// Set bit in a 16 bit register
	pcf8574_byte(R.PCF_I2C_lpf1_addr, band_sel.b0);
	pcf8574_byte(R.PCF_I2C_lpf2_addr, band_sel.b1);
	selectedFilters[1] = R.TXFilterNumber[i];	// Used for LCD Print indication
	#else
	band_sel.w = 1<<i;							// Set bit in a 16 bit register
	pcf8574_byte(R.PCF_I2C_lpf1_addr, band_sel.b0);
	pcf8574_byte(R.PCF_I2C_lpf2_addr, band_sel.b1);
	selectedFilters[1] = i;						// Used for LCD Print indication
	#endif// SCRAMBLED_FILTERS
	#endif// I2C_16_FILTER_IO

	#if PCF_FILTER_IO							// 8x BCD control for LPF switching, switches P1 pins 4-6
	uint8_t	j;
	j = i<<3;									// leftshift x 3 for bits 3 - 5
	pcf_data_out = pcf_data_out & 0b11000111;	// Clear out old data before adding new
	MoboPCF_set(j);								// Combine the two and write out
	selectedFilters[1] = i;						// Used for LCD Print indication
	#endif

	#if M0RZF_FILTER_IO							// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
	uint8_t	j;									// filter value

	//if      (i == 0) j = 0b00001000;			// First filter, as defined in data struct in Mobo.c
	//else if (i == 1) j = 0b00010000;			// Second filter
	//else if (i == 2) j = 0b00100000; 			// Third filter
	//else             j = 0b00000000; 			// Default filter

	j = 0b00001000<<i;							// Left shift i times
	pcf_data_out = pcf_data_out & 0b11000111;	// Clear out old data before adding new
	MoboPCF_set(j);								// Combine the two and write out
	selectedFilters[1] = i;						// Used for LCD Print indication
	#endif

	#if CALC_BAND_MUL_ADD						// Band dependent Frequency Subtract and Multiply
	return freqBand;	
	#endif
}



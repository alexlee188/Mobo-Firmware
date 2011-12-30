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
//** Several alternate functions included in this file:
//**
//**                   Quiet Mode (LCD_PAR_DISPLAY2):
//**                   20x4 and 40x2
//**
//**                   Old style, constant LCD write (LCD_PAR_DISPLAY):
//**                   16x2, 20x2 non bargraph versions
//**                   16x2, 20x2, 20x4 and 40x2 bargraph versions
//**
//**                   Experimental (not tested) I2C alternatives of "Old style"
//**
//** Initial version.: 2009-09-09, Loftur Jonasson, TF3LJ
//**
//** Last update to this file: 2010-05-24, Loftur Jonasson, TF3LJ
//**
//**                   Check the Mobo.c file
//**
//**
//*********************************************************************************

#include	"Mobo.h"
//#include	<rprintf.h>								// AVRLIB (not AVRLibc) functions


// Identical routines for I2C and parallel connected displays
#if (LCD_PAR_DISPLAY2 || LCD_PAR_DISPLAY || LCD_I2C_DISPLAY)
//
//-----------------------------------------------------------------------------
// This little routine is used to handle LCD print from program memory,  
// rather than using SRAM.
// A small trick necessary for not to overrun what little SRAM we have left
// (similar function in lcd.c, but it compiles 6 bytes leaner if placed here)
//-----------------------------------------------------------------------------
//
void lcd_putchar_P(const char *addr)
{
	uint8_t c;

	while ((c = pgm_read_byte(addr++)))
		lcd_data(c);
}

#endif


//*******************************************************************************************************************

#if LCD_PAR_DISPLAY2	// New and improved, low traffic (low noise) Mobo_LCD_Display.c routines


//*******************************************************************************************************************

#if LCD_DISPLAY_20x4_b	// 20x4 LCD display variant.
//
//-----------------------------------------------------------------------------
//			Display stuff on a 20x4 LCD 
//-----------------------------------------------------------------------------
//

#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
static uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer
#endif

//
//-------------------------------------------------------
// Display TX/RX Status on change
//-------------------------------------------------------
//
void lcd_display_TRX_status_on_change(void)
{
	static uint8_t tx_status = 1;					// Force run first time. Keep track of TRX status
	
	if (tx_status != (Status1 & TX_FLAG))			// Has there been a TX/RX change?
	{
		tx_status = Status1 & TX_FLAG;

		lcd_display_TRX_stuff();
	}
}


//
//-------------------------------------------------------
// Display TX/RX Stuff
//-------------------------------------------------------
//
void lcd_display_TRX_stuff(void)
{
	lcd_clrscr();								// Clear Screen is a cheaper operation than
												// writing blanks, though it causes flicker

	lcd_display_freq_and_filters();				// Update frequency display
	lcd_display_P_SWR_V_C_T();					// Update 2nd line, Temp, V, I...

	lcd_command(LCD_DISP_ON);					// Blinking block cursor OFF

	lcd_gotoxy(0,0);							// First char, first line
	if(Status1 & TX_FLAG)						// TX or RX ?
	{
		lcd_data('T');
	}
	else
	{
		lcd_data('R');

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif

		//
		//-------------------------------------------
		// Display static stuff when in Receive ("RX") mode
		//-------------------------------------------
		//
		#if DISP_TERSE								// Option for 20x4 and 40x2 displays
		lcd_gotoxy(12,3);							// Line 4 position 13
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VERBOSE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,2);							// Line 3 position 0
		lcd_putchar_P(PSTR("Softrock 6.3  &"));		// Print string from flash rom
		lcd_gotoxy(12,3);							// Line 4 position 13
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VER_SIMPLE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,3);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		lcd_gotoxy(16,3);
		rprintf("%s",VERSION);						// Print version info
		#endif
		#if DISP_VER_SHORT							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,3);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3  "));			// Print string from flash rom
		rprintf("%s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
		#if DISP_VERSION							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,2);							// Line 3 position 0
		lcd_putchar_P(PSTR("SR 6.3 <--> Mobo 4.3"));// Print string from flash rom
		lcd_gotoxy(5,3);							// Line 4 position 5
		rprintf("Ver: %s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
	}
}


//
//-------------------------------------------------------
// Display Frequency and Filters
//-------------------------------------------------------
//
void lcd_display_freq_and_filters(void)
{
	uint32_t dispFrq;
	uint16_t fMHz;
	uint32_t fHz;

	lcd_gotoxy(1,0);								// Second char, first line

	//
	//-------------------------------------------------------
	// Display the Si570 frequency (divided by 4) on the LCD:
	//-------------------------------------------------------
	//

	#if PSDR_IQ_OFFSET36							// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		//frq = frq + R.LCD_RX_Offset;
		R.Freq[0] = R.Freq[0] + R.LCD_RX_Offset;
	}
	#endif

	dispFrq = (double) R.Freq[0] * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

	#if PSDR_IQ_OFFSET68							// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		dispFrq = dispFrq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
	}
	#endif

	fMHz = dispFrq / 1000000;
	fHz = dispFrq % 1000000;

	rprintf("X%3u.%06luMHz F%x-%x", fMHz,fHz,selectedFilters[0],selectedFilters[1]);
}

//
//-------------------------------------------------------
// Display Power, SWR, Voltage, Current and Temperature
//-------------------------------------------------------
//
void lcd_display_P_SWR_V_C_T(void)
{
	#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure
	#endif

	lcd_gotoxy(0,1);								// First char, second line

	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	uint16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.w>>7) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Display "  Voltage and temperature" in second line
	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
													// (threshold still set in deg C)
	rprintf("%3uF", tmp_F);
	#else
	rprintf("%3d", tmp_C);
	lcd_data(0xdf); lcd_data('C');
	#endif
	
	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	uint16_t vdd_tenths = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	uint16_t vdd = vdd_tenths / 10;
	vdd_tenths = vdd_tenths % 10;
	
	lcd_gotoxy(7,1);
	rprintf("%2u.%1uV", vdd, vdd_tenths);

	//
	//-------------------------------------------------------
	// During TX, Display PA current in second line of LCD
	//-------------------------------------------------------
	//
	if (Status1 & (PA_CAL | TX_FLAG))
	{
		// Fetch and normalize PA current
		uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
		uint16_t idd = idd_ca/100; idd_ca = idd_ca%100;
	
		lcd_gotoxy(13,1);							// Second line, 13th position
		rprintf("%u.%02uA", idd, idd_ca);			// Display current while in transmit
	}

	//
	//-------------------------------------------------------
	// Display alternate text during RX in second line of LCD
	//-------------------------------------------------------
	//
	else
	{
		lcd_putchar_P(PSTR("  Bias:"));				// Print string from flash rom
	}

	//
	//-------------------------------------------------------
	// Display Bias setting in second line of LCD
	//-------------------------------------------------------
	//
	const char bias[]= {'R','L','H'};
	lcd_gotoxy(19,1);								// Second line, 19th position
	lcd_data(bias[biasInit]);						// Print Bias status 'R'educed
													// 'Low' or 'H'igh

	//-------------------------------------------------------
	// Display Stuff in third and fourth line of the LCD		
	//-------------------------------------------------------
	lcd_gotoxy(0,3);								// Fourth line, first position	
	//--------------------------------------------
	// Display a PA thermal runaway situation		
	//--------------------------------------------
	if(Status1 & TMP_ALARM)							// Thermal runaway, TX shutdown
	{
		lcd_putchar_P(PSTR("Temperature Shutdown"));// A trick to print string from flash rom
													// rather than from SRAM
	}

	//
	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	//
	else if(Status1 & PA_CAL)						// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA Bias Calibrate"));	// Print string from flash rom rather than from SRAM
	}

	//
	//-------------------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------------------
	//
	else if(Status1 & TX_FLAG)
	{
		//
		//-------------------------------------------------------
		// Power Output Bargraph in third line
		//-------------------------------------------------------
		//
		lcd_gotoxy(0,2);							// Third line
		uint16_t pow_tot, pow, pow_mw;

		// Prepare Power readout
		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		static uint8_t i = 0;

		#if	PWR_PEP_ADJUST							// Option to adjust the number of samples in PEP measurement
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= R.PEP_samples) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < R.PEP_samples; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#else
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= PEP_PERIOD) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < PEP_PERIOD; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#endif
		#endif

		// progress, maxprogress, len
		lcdProgressBar(pow_tot/100, R.PWR_fullscale*10, 12);

		pow = pow_tot / 1000; 						// Watts
		pow_mw = pow_tot % 1000;					// milliWatts

		rprintf("P%2u.%03uW", pow, pow_mw);

		//
		//--------------------------------------------
		// Display SWR Bargraph in fourth line
		//--------------------------------------------
		//
		lcd_gotoxy(0,3);							// Fourth line

		// Prepare SWR readout
		uint16_t swr, swr_hundredths, swr_tenths;
		swr = measured_SWR / 100;					// SWR
		swr_hundredths = measured_SWR % 100;		// sub decimal point, 1/100 accuracy
		swr_tenths = swr_hundredths / 10;			// sub decimal point, 1/10 accuracy

		// progress, maxprogress, len
		#if	BARGRAPH_SWR_SCALE						// Add option to adjust the Fullscale value for the SWR bargraph
		// possible bug fix for SWR lcdProgressBar exceeding range
		uint16_t display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? R.SWR_fullscale*100 : (measured_SWR-100);
		lcdProgressBar(display_SWR, R.SWR_fullscale*100, 12);
		#else
		display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? SWR_FULL_SCALE : (measured_SWR-100);
		lcdProgressBar(display_SWR - 100, SWR_FULL_SCALE, 12);
		#endif	

		lcd_putchar_P(PSTR("SWR"));					// Print string from flash rom

		rprintf("%2u.", swr);						// Print the super decimal portion of the SWR

		#if SWR_ALARM_FUNC							// SWR alarm function, activates a secondary PTT
		//
		//--------------------------------------------
		// Display a SWR Alarm situation
		//--------------------------------------------
		//
		if(Status1 & SWR_ALARM)						// SWR Alarm flag set
		{
			if(swr >= 100)							// SWR more than 99
				lcd_putchar_P(PSTR("  --"));
			else
				lcd_data(swr_tenths+0x30);			// Print the sub-decimal value of the SWR, single digit precision
													// (a lean byte saving way to print a single decimal)

			lcd_data('A');							// A in SWRA indication
			lcd_command(LCD_DISP_ON_BLINK);			// Blinking block cursor ON
		}
		else
		#endif//SWR_ALARM_FUNC
			rprintf("%02u", swr_hundredths);		// Print the sub-decimal value of the SWR, double digit precision

		// In case of a blinking "SWR Alarm" cursor, put it on "A" in "SWR Alarm"
		lcd_gotoxy(19,3);
	}
}	


//
//-------------------------------------------------------
// Display if new Frequency is stored by Shaft Encoder func
//-------------------------------------------------------
//
#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE			// Shaft Encoder VFO function
void lcd_display_Memory_Stored(void)				// Display Memory Stored for a certain amount of time
{
	static int count = 0;							// Display for ENC_STORE_DISP * 100ms

	lcd_gotoxy(0,3);								// Fourth line
	lcd_putchar_P(PSTR("VFO Memory Stored   "));	// Print string from flash rom rather than from SRAM

	count++;
	if(count>=ENC_STORE_DISP)
	{
		Status2 &= ~ENC_STORED;
		count = 0;
		lcd_display_TRX_stuff();
	}
}
#endif
#endif//LCD_DISPLAY_20x4_b// 20x4 LCD display.


//*******************************************************************************************************************


#if LCD_DISPLAY_40x2_b	// Alternate 40x2 LCD display variant.
//
//-----------------------------------------------------------------------------
//			Display stuff on a 40x2 LCD 
//-----------------------------------------------------------------------------
//

#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
static uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer
#endif

//
//-------------------------------------------------------
// Display TX/RX Status on change
//-------------------------------------------------------
//
void lcd_display_TRX_status_on_change(void)
{
	static uint8_t tx_status = 1;					// Force run first time. Keep track of TRX status
	
	if (tx_status != (Status1 & TX_FLAG))			// Has there been a TX/RX change?
	{
		tx_status = Status1 & TX_FLAG;

		lcd_display_TRX_stuff();
	}
}


//
//-------------------------------------------------------
// Display TX/RX Stuff
//-------------------------------------------------------
//
void lcd_display_TRX_stuff(void)
{
	lcd_clrscr();								// Clear Screen is a cheaper operation than
												// writing blanks, though it causes flicker

	lcd_display_freq_and_filters();				// Update frequency display
	lcd_display_P_SWR_V_C_T();					// Update 2nd line, Temp, V, I...

	lcd_command(LCD_DISP_ON);					// Blinking block cursor OFF

	lcd_gotoxy(0,0);							// First char, first line
	if(Status1 & TX_FLAG)						// TX or RX ?
	{
		lcd_data('T');
	}
	else
	{
		lcd_data('R');

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif

		//
		//-------------------------------------------
		// Display static stuff when in Receive ("RX") mode
		//-------------------------------------------
		//
		#if DISP_TERSE								// Option for 20x4 and 40x2 displays
		lcd_gotoxy(32,1);							// Line 2 position 32
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VERBOSE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,0);							// Line 1 position 21
		lcd_putchar_P(PSTR("Softrock 6.3  &"));		// Print string from flash rom
		lcd_gotoxy(32,1);							// Line 2 position 32
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VER_SIMPLE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,1);							// Line 2 position 21
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		lcd_gotoxy(36,1);
		rprintf("%s",VERSION);						// Print version info
		#endif
		#if DISP_VER_SHORT							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,1);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3  "));			// Print string from flash rom
		rprintf("%s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
		#if DISP_VERSION							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,0);							// Line 1 position 20
		lcd_putchar_P(PSTR("SR 6.3 <--> Mobo 4.3"));// Print string from flash rom
		lcd_gotoxy(25,1);							// Line 2 position 25
		rprintf("Ver: %s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
	}
}


//
//-------------------------------------------------------
// Display Frequency and Filters
//-------------------------------------------------------
//
void lcd_display_freq_and_filters(void)
{
	uint32_t dispFrq;
	uint16_t fMHz;
	uint32_t fHz;

	lcd_gotoxy(1,0);								// Second char, first line

	//
	//-------------------------------------------------------
	// Display the Si570 frequency (divided by 4) on the LCD:
	//-------------------------------------------------------
	//

	#if PSDR_IQ_OFFSET36							// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		//frq = frq + R.LCD_RX_Offset;
		R.Freq[0] = R.Freq[0] + R.LCD_RX_Offset;
	}
	#endif

	dispFrq = (double) R.Freq[0] * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

	#if PSDR_IQ_OFFSET68							// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		dispFrq = dispFrq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
	}
	#endif

	fMHz = dispFrq / 1000000;
	fHz = dispFrq % 1000000;

	rprintf("X%3u.%06luMHz F%x-%x", fMHz,fHz,selectedFilters[0],selectedFilters[1]);
}

//
//-------------------------------------------------------
// Display Power, SWR, Voltage, Current and Temperature
//-------------------------------------------------------
//
void lcd_display_P_SWR_V_C_T(void)
{
	#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure
	#endif

	lcd_gotoxy(0,1);								// First char, second line

	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	uint16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.w>>7) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Display "  Voltage and temperature" in second line
	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
													// (threshold still set in deg C)
	rprintf("%3uF", tmp_F);
	#else
	rprintf("%3d", tmp_C);
	lcd_data(0xdf); lcd_data('C');
	#endif
	
	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	uint16_t vdd_tenths = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	uint16_t vdd = vdd_tenths / 10;
	vdd_tenths = vdd_tenths % 10;
	
	lcd_gotoxy(7,1);
	rprintf("%2u.%1uV", vdd, vdd_tenths);

	//
	//-------------------------------------------------------
	// During TX, Display PA current in second line of LCD
	//-------------------------------------------------------
	//
	if (Status1 & (PA_CAL | TX_FLAG))
	{
		// Fetch and normalize PA current
		uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
		uint16_t idd = idd_ca/100; idd_ca = idd_ca%100;
	
		lcd_gotoxy(13,1);							// Second line, 13th position
		rprintf("%u.%02uA", idd, idd_ca);			// Display current while in transmit
	}

	//
	//-------------------------------------------------------
	// Display alternate text during RX in second line of LCD
	//-------------------------------------------------------
	//
	else
	{
		lcd_putchar_P(PSTR("  Bias:"));				// Print string from flash rom
	}

	//
	//-------------------------------------------------------
	// Display Bias setting in second line of LCD
	//-------------------------------------------------------
	//
	const char bias[]= {'R','L','H'};
	lcd_gotoxy(19,1);								// Second line, 19th position
	lcd_data(bias[biasInit]);						// Print Bias status 'R'educed
													// 'Low' or 'H'igh

	//-------------------------------------------------------
	// Display Stuff in third and fourth line of the LCD		
	//-------------------------------------------------------
	lcd_gotoxy(20,1);								// Second half, second line	
	//--------------------------------------------
	// Display a PA thermal runaway situation		
	//--------------------------------------------
	if(Status1 & TMP_ALARM)							// Thermal runaway, TX shutdown
	{
		lcd_putchar_P(PSTR("Temperature Shutdown"));// A trick to print string from flash rom
													// rather than from SRAM
	}

	//
	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	//
	else if(Status1 & PA_CAL)						// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA Bias Calibrate"));	// Print string from flash rom rather than from SRAM
	}

	//
	//-------------------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------------------
	//
	else if(Status1 & TX_FLAG)
	{
		//
		//-------------------------------------------------------
		// Power Output Bargraph in third line
		//-------------------------------------------------------
		//
		lcd_gotoxy(20,0);							// Second half, first line
		uint16_t pow_tot, pow, pow_mw;

		// Prepare Power readout
		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		static uint8_t i = 0;

		#if	PWR_PEP_ADJUST							// Option to adjust the number of samples in PEP measurement
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= R.PEP_samples) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < R.PEP_samples; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#else
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= PEP_PERIOD) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < PEP_PERIOD; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#endif
		#endif

		// progress, maxprogress, len
		lcdProgressBar(pow_tot/100, R.PWR_fullscale*10, 12);

		pow = pow_tot / 1000; 						// Watts
		pow_mw = pow_tot % 1000;					// milliWatts

		rprintf("P%2u.%03uW", pow, pow_mw);

		//
		//--------------------------------------------
		// Display SWR Bargraph in fourth line
		//--------------------------------------------
		//
		lcd_gotoxy(20,1);							// Second half, second line

		// Prepare SWR readout
		uint16_t swr, swr_hundredths, swr_tenths;
		swr = measured_SWR / 100;					// SWR
		swr_hundredths = measured_SWR % 100;		// sub decimal point, 1/100 accuracy
		swr_tenths = swr_hundredths / 10;			// sub decimal point, 1/10 accuracy

		// progress, maxprogress, len
		#if	BARGRAPH_SWR_SCALE						// Add option to adjust the Fullscale value for the SWR bargraph
		// possible bug fix for SWR exceeding lcdProgressBar range
		uint16_t display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? R.SWR_fullscale*100 : (measured_SWR-100);
		lcdProgressBar(display_SWR, R.SWR_fullscale*100, 12);
		#else
		display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? SWR_FULL_SCALE : (measured_SWR-100);
		lcdProgressBar(display_SWR - 100, SWR_FULL_SCALE, 12);
		#endif	

		lcd_putchar_P(PSTR("SWR"));					// Print string from flash rom

		rprintf("%2u.", swr);						// Print the super decimal portion of the SWR

		#if SWR_ALARM_FUNC							// SWR alarm function, activates a secondary PTT
		//
		//--------------------------------------------
		// Display a SWR Alarm situation
		//--------------------------------------------
		//
		if(Status1 & SWR_ALARM)						// SWR Alarm flag set
		{
			if(swr >= 100)							// SWR more than 99
				lcd_putchar_P(PSTR("  --"));
			else
				lcd_data(swr_tenths+0x30);			// Print the sub-decimal value of the SWR, single digit precision
													// (a lean byte saving way to print a single decimal)

			lcd_data('A');							// A in SWRA indication
			lcd_command(LCD_DISP_ON_BLINK);			// Blinking block cursor ON
		}
		else
		#endif//SWR_ALARM_FUNC
			rprintf("%02u", swr_hundredths);		// Print the sub-decimal value of the SWR, double digit precision

		// In case of a blinking "SWR Alarm" cursor, put it on "A" in "SWR Alarm"
		lcd_gotoxy(39,1);
	}
}	


//
//-------------------------------------------------------
// Display if new Frequency is stored by Shaft Encoder func
//-------------------------------------------------------
//
#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE			// Shaft Encoder VFO function
void lcd_display_Memory_Stored(void)				// Display Memory Stored for a certain amount of time
{
	static int count = 0;							// Display for ENC_STORE_DISP * 100ms

	lcd_gotoxy(20,1);								// Second half, second line	
	lcd_putchar_P(PSTR(" VFO Memory Stored  "));	// Print string from flash rom rather than from SRAM

	count++;
	if(count>=ENC_STORE_DISP)
	{
		Status2 &= ~ENC_STORED;
		count = 0;
		lcd_display_TRX_stuff();
	}
}
#endif

#endif//LCD_DISPLAY_40x2_b// Alternate 40x2 LCD display.

#endif	//LCD_PAR_DISPLAY2	// Alternate low traffic (low noise) Mobo_LCD_Display.c routines



//*******************************************************************************************************************



#if (LCD_PAR_DISPLAY || LCD_I2C_DISPLAY)// Identical routines for I2C and parallel connected displays



//*******************************************************************************************************************



#if LCD_DISPLAY_16x2// Original Default Display Routines for 16x2 LCD
//
//-----------------------------------------------------------------------------
//			Display stuff on a 16x2 LCD 
//-----------------------------------------------------------------------------
//
void lcd_display(void)
{       	
	//rprintfInit(lcd_data);

	#if !DEBUG_1LN									// First line of LCD not used for Debug
	//
	//-------------------------------------------------------
	// Display stuff in first line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,0);								// First char, first line

	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	if(Status1 & PA_CAL)							// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA BiasCalibrate"));	// Lean and pgm space efficient print
	}
	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
	//-------------------------------------------------------
	// Display if new Frequency is stored by Shaft Encoder func
	//-------------------------------------------------------
	else if (Status2 & ENC_STORED)
	{
		lcd_putchar_P(PSTR("Memory Stored   "));
		static int count = 0;						// Display for ENC_STORE_DISP * 100ms
		count++;
		if(count>=ENC_STORE_DISP)
		{
			Status2 &= ~ENC_STORED;
			count = 0;
		}
	}
	#endif
	//-------------------------------------------------------
	// Display the Si570 frequency (divided by 4) on the LCD:
	// (Two separate compile time options below)
	//-------------------------------------------------------
	else
	{
		// Display the current Si570 frequency on the LCD
		int32_t freq;
	
		freq = R.Freq[0];

		#if PSDR_IQ_OFFSET36				// Display a fixed frequency offset during RX only.
		if(!(Status1 & TX_FLAG))
		{
			freq = freq + R.LCD_RX_Offset;
		}
		#endif	

		uint32_t dispFreq;
		dispFreq = (double) freq * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

		#if PSDR_IQ_OFFSET68					// Display a fixed frequency offset during RX only.
		if(!(Status1 & TX_FLAG))
		{
			dispFreq = dispFreq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
		}
		#endif

		uint16_t fMHz = dispFreq / 1000000;
		uint32_t fHz = dispFreq % 1000000;

		rprintf("%3u.%06luMHz RX", fMHz,fHz);
	}	

	//--------------------------------------------
	// Display the Transmit/Receive Status
	//--------------------------------------------
	lcd_gotoxy(14,0);								// 15th char, first line
	if(Status1 & TX_FLAG)
	{
		lcd_putchar_P(PSTR("TX"));					// Display a blinking "TX"
		lcd_command(LCD_DISP_ON_BLINK);				// Blinking block cursor ON
	}
	else
	{
		lcd_putchar_P(PSTR("RX"));					// Display a steady "RX"
		lcd_command(LCD_DISP_ON);					// Blinking block cursor OFF
	}
	#endif//DEBUG_1LN

	
	#if !DEBUG_2LN									// Second line of LCD not used for Debug
	//
	//-------------------------------------------------------
	// Display stuff in second line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,1);								// First char, second line

	// Variables used for print formatting, to emulate a floating point printf
	// AVRLIB floating point printf formatting would be very costly in size
	uint16_t vdd_tot, vdd, vdd_tenths;
	
	#if POWER_SWR									// Power and SWR measurement	
	uint16_t swr, swr_hundredths, swr_tenths, pow_tot, pow, pow_dw, pow_cw, pow_mw;
	#endif
	
	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	int16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.i/128) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	vdd_tot = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	vdd = vdd_tot / 10;
	vdd_tenths = vdd_tot % 10;

	// Fetch and normalize PA current
	uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
	uint16_t idd = idd_ca / 100; 
	idd_ca = idd_ca % 100;

	#if POWER_SWR									// Power and SWR measurement
	// Prepare Power readout
	pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

	#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
	static uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer

	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure

	static uint8_t i = 0;

	#if	PWR_PEP_ADJUST								// Option to adjust the number of samples in PEP measurement
	pow_avg[i] = pow_tot;							// Store value in ringbuffer
	i++;
	if (i >= R.PEP_samples) i = 0;

	pow_tot = 0;
	for (uint8_t j = 0; j < R.PEP_samples; j++)		// Retrieve the largest value out of the measured window
	{
		if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
	}
	#else
	pow_avg[i] = pow_tot;							// Store value in ringbuffer
	i++;
	if (i >= PEP_PERIOD) i = 0;

	pow_tot = 0;
	for (uint8_t j = 0; j < PEP_PERIOD; j++)		// Retrieve the largest value out of the measured window
	{
		if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
	}
	#endif
	#endif

	pow = pow_tot / 1000; 							// Watts
	pow_mw = pow_tot % 1000;						// milliWatts for certain LCD prints
	pow_cw = pow_mw/10;								// centiWatts for other LCD prints
	pow_dw = pow_mw/100;							// deciWatts for yet other LCD prints

	// Prepare SWR readout
	swr = measured_SWR / 100;						// SWR
	swr_hundredths = measured_SWR % 100;			// sub decimal point, 1/100 accuracy
	swr_tenths = swr_hundredths/10;					// sub decimal point, 1/10 accuracy
	#endif//POWER_SWR								// Power and SWR measurement

	//-------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------
	if(Status1 & TX_FLAG)
	{
		//--------------------------------------------
		// Display a PA thermal runaway situation		
		//--------------------------------------------
		if(Status1 & TMP_ALARM)						// Thermal runaway, TX shutdown
		{
			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("TempShutdwn:%3uF", tmp_F);
			#else
			rprintf("TempShutdwn%3d", tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}

		//---------------------------------------------------------
		// Display current during a PA Bias Autotune
		//---------------------------------------------------------
		else if(Status1 & PA_CAL)
		{
			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("%2u.%1uV %01u.%02uA %3uF", vdd,vdd_tenths,idd,idd_ca,tmp_F);
			#else
			rprintf("%2u.%1uV %01u.%02uA%3u", vdd,vdd_tenths,idd,idd_ca,tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}
		
		//--------------------------------------------
		// Display a SWR Alarm situation
		//--------------------------------------------
		#if	POWER_SWR								// Power and SWR measurement
		#if SWR_ALARM_FUNC							// SWR alarm function, activates a secondary PTT
		else if(Status1 & SWR_ALARM)				// SWR Alarm flag set
		{
			#if AUTOSCALE_PWR						// Display sub 1W power in mW
			if (pow == 0)							// Power less than a watt
			{
				rprintf("SWRA%3umW SWR", pow_mw);
			}
			else
			#endif//AUTOSCALE_PWR
			{										// Power more than a watt
				rprintf("SWRA%2u.%1uW SWR", pow,pow_dw);
			}
			if(swr >= 100)  						// SWR more than 99
				lcd_putchar_P(PSTR("-- "));
			else									// SWR less than 99	
				// this line ca be up to 17 chars long
				rprintf("%u.%u", swr, swr_tenths);
		}		

		//--------------------------------------------
		// Display Power and SWR
		//--------------------------------------------
		else
		#endif//SWR_ALARM_FUNC						// SWR alarm function, activates a secondary PTT
		{
			#if AUTOSCALE_PWR						// Display sub 1W power in mW
			if (pow == 0)							// Power less than a watt
			{
				rprintf("P %3umW SWR",(uint16_t)pow_mw);
				if (swr >= 100)						// SWR more than 99
					lcd_putchar_P(PSTR("--  "));
				else								// SWR less than 99
				rprintf("%2u.%02u", swr, swr_hundredths);					
			}
			else
			#endif//AUTOSCALE_PWR
			{
				rprintf("P%2u.%02uW SWR", pow, pow_cw);
				if (swr >= 100)	  					// SWR more than 99
					lcd_putchar_P(PSTR("--  "));
				else								// SWR less than 99
					rprintf("%2u.%02u", swr, swr_hundredths);
			}
		}
		
		//---------------------------------------------------------
		// Alternate Display if PWR_SWR compile option not selected
		//---------------------------------------------------------
		#else//not POWER_SWR
		else
		{
			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("%2u.%1uV %01u.%02uA %3uF", vdd,vdd_tenths,idd,idd_ca,tmp_F);
			#else
			rprintf("%2u.%1uV %01u.%02uA%3u", vdd,vdd_tenths,idd,idd_ca,tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}
		#endif//POWER_SWR							// Power and SWR measurement
	}

	//-------------------------------------------
	// Display stuff when in Receive ("RX") mode
	//-------------------------------------------
	else
	{
		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif		


		//
		//-------------------------------------------------------
		// Display Bias setting in second line of LCD
		//-------------------------------------------------------
		//
		const char bias[]= {'R','L','H'};
		lcd_data(bias[biasInit]);					// Print Bias status, 'Low' or 'H'igh
													// 'R' shown when going into RX immediately
													// after a SWR-Alarm situation
		
		//
		//-------------------------------------------------------
		// Display "  Voltage and temperature" in second line
		//-------------------------------------------------------
		//
		#if DISP_FAHRENHEIT							// Display temperature in Fahrenheit
													// (threshold still set in deg C)
		rprintf("-%x-%x %2u.%1uV %3uF ", 
			selectedFilters[0],selectedFilters[1],vdd, vdd_tenths, tmp_F);
		#else
		rprintf("-%x%x %2d.%1uV %3d", 
			selectedFilters[0],selectedFilters[1],vdd, vdd_tenths, tmp_C);
			lcd_data(0xdf); lcd_data('C');
		#endif
	}
	#endif//DEBUG_2LN
	
	//-------------------------------------------
	// Put "blinking" cursor at the correct location
	// (if in RX mode and not blinking, it doesn't matter)
	//-------------------------------------------
	if(Status1 & SWR_ALARM)							// SWR Alarm flag set
	{
		lcd_gotoxy(3,1);							// then put blinking cursor "A" in "SWRA"
	}
	else
	{
		lcd_gotoxy(14,0);							// If blinking cursor, then put on "T"
	}
}
#endif//LCD_DISPLAY_16x2// Original Default Display Routines for 16x2 LCD



//*******************************************************************************************************************



#if LCD_DISPLAY_16x2_b// Alternate 16x2 LCD display with bargraph.
//
//-----------------------------------------------------------------------------
//			Display stuff on a 16x2 LCD, including a couple of bargraphs 
//-----------------------------------------------------------------------------
//
void lcd_display(void)
{       	
	//rprintfInit(lcd_data);

	lcd_command(LCD_DISP_ON);					// Blinking block cursor OFF

	// Variables used for print formatting, to emulate a floating point printf
	// AVRLIB floating point printf formatting would be very costly in size
	uint16_t vdd_tot, vdd, vdd_tenths, swr, swr_hundredths, swr_tenths, pow_tot, pow, pow_mw;

	#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
	static uint16_t pow_avg[PEP_MAX_PERIOD];	// Power measurement ringbuffer

	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure
	#endif
		
	#if !DEBUG_1LN									// First line of LCD not used for Debug
	//
	//-------------------------------------------------------
	// Display stuff in first line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,0);								// First char, first line

	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	if(Status1 & PA_CAL)							// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA BiasCalibrate"));	// Lean and pgm space efficient print
	}
	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
	//-------------------------------------------------------
	// Display if new Frequency is stored by Shaft Encoder func
	//-------------------------------------------------------
	else if (Status2 & ENC_STORED)
	{
		lcd_putchar_P(PSTR("Memory Stored   "));
		static int count = 0;						// Display for ENC_STORE_DISP * 100ms
		count++;
		if(count>=ENC_STORE_DISP)
		{
			Status2 &= ~ENC_STORED;
			count = 0;
		}
	}
	#endif
	//-------------------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//
	// Power Output Bargraph
	//-------------------------------------------------------
	else if(Status1 & TX_FLAG)
	{
		// Prepare Power readout
		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		static uint8_t i = 0;

		#if	PWR_PEP_ADJUST							// Option to adjust the number of samples in PEP measurement
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= R.PEP_samples) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < R.PEP_samples; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#else
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= PEP_PERIOD) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < PEP_PERIOD; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#endif
		#endif

		// progress, maxprogress, len
		lcdProgressBar(pow_tot/100, R.PWR_fullscale*10, 8);

		pow = pow_tot / 1000; 						// Watts
		pow_mw = pow_tot % 1000;					// milliWatts

		rprintf("P%2u.%03uW", pow, pow_mw);
	}

	//-------------------------------------------------------
	// Display stuff while in Receive "RX" mode
	//
	// Display the Si570 frequency (divided by 4) on the LCD:
	// (Two separate compile time options below)
	//-------------------------------------------------------
	else
	{
		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)	// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif		

		// Display the current Si570 frequency on the LCD

		int32_t freq;
	
		freq = R.Freq[0];

		#if PSDR_IQ_OFFSET36					// Display a fixed frequency offset during RX only.
		if(!(Status1 & TX_FLAG))
		{
			freq = freq + R.LCD_RX_Offset;
		}
		#endif	

		uint32_t dispFreq;
		dispFreq = (double) freq * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

		#if PSDR_IQ_OFFSET68					// Display a fixed frequency offset during RX only.
		if(!(Status1 & TX_FLAG))
		{
			dispFreq = dispFreq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
		}
		#endif

		uint16_t fMHz = dispFreq / 1000000;
		uint32_t fHz = dispFreq % 1000000;

		rprintf("%3u.%06luMHz RX", fMHz,fHz);
	}	

	#endif//DEBUG_1LN

	
	#if !DEBUG_2LN									// Second line of LCD not used for Debug
	//
	//-------------------------------------------------------
	// Display stuff in second line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,1);								// First char, second line

	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	int16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.i/128) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	vdd_tot = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	vdd = vdd_tot / 10;
	vdd_tenths = vdd_tot % 10;


	//-------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------
	if (Status1 & TX_FLAG)
	{
		//--------------------------------------------
		// Display a PA thermal runaway situation		
		//--------------------------------------------
		if(Status1 & TMP_ALARM)						// Thermal runaway, TX shutdown
		{
			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("TempShutdwn:%3uF", tmp_F);
			#else
			rprintf("TempShutdwn%3d", tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}

		//---------------------------------------------------------
		// Display current during a PA Bias Autotune
		//---------------------------------------------------------
		else if(Status1 & PA_CAL)
		{
			// Fetch and normalize PA current
			uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
			uint16_t idd = idd_ca / 100; 
			idd_ca = idd_ca % 100;

			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("%2u.%1uV %01u.%02uA %3uF", vdd,vdd_tenths,idd,idd_ca,tmp_F);
			#else
			rprintf("%2u.%1uV %01u.%02uA%3d", vdd,vdd_tenths,idd,idd_ca,tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}

		//--------------------------------------------
		// Display SWR
		//--------------------------------------------
		else
		{
			// Prepare SWR readout
			swr = measured_SWR / 100;				// SWR
			swr_hundredths = measured_SWR % 100;	// sub decimal point, 1/100 accuracy
			swr_tenths = swr_hundredths / 10;		// sub decimal point, 1/10 accuracy

			// progress, maxprogress, len
			#if	BARGRAPH_SWR_SCALE					// Add option to adjust the Fullscale value for the SWR bargraph
		
		// possible bug fix for SWR exceeding lcdProgressBar range
		uint16_t display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? R.SWR_fullscale*100 : (measured_SWR-100);
		lcdProgressBar(display_SWR, R.SWR_fullscale*100, 12);
		#else
		display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? SWR_FULL_SCALE : (measured_SWR-100);
		lcdProgressBar(display_SWR - 100, SWR_FULL_SCALE, 12);
		#endif	


			lcd_putchar_P(PSTR("SWR"));				// Print string from flash rom

			rprintf("%2u.", swr);					// Print the super decimal portion of the SWR

			#if SWR_ALARM_FUNC						// SWR alarm function, activates a secondary PTT
			//
			//--------------------------------------------
			// Display a SWR Alarm situation
			//--------------------------------------------
			//
			if(Status1 & SWR_ALARM)					// SWR Alarm flag set
			{
				if(swr >= 100)						// SWR more than 99
					lcd_putchar_P(PSTR("  --"));
				else
					lcd_data(swr_tenths+0x30);		// Print the sub-decimal value of the SWR, single digit precision
													// (a lean byte saving way to print a single decimal)
	
				lcd_data('A');						// A in SWRA indication
				lcd_command(LCD_DISP_ON_BLINK);		// Blinking block cursor ON
			}
			else
			#endif//SWR_ALARM_FUNC
				rprintf("%02u", swr_hundredths);	// Print the sub-decimal value of the SWR, double digit precision
	
			lcd_gotoxy(15,1);						// In case blinking cursor, put it on "A" in "SWR Alarm"
		}
	}

	//-------------------------------------------
	// Display stuff when in Receive ("RX") mode
	//-------------------------------------------
	else
	{
		//
		//-------------------------------------------------------
		// Display Bias setting in second line of LCD
		//-------------------------------------------------------
		//
		const char bias[]= {'R','L','H'};
		lcd_data(bias[biasInit]);					// Print Bias status, 'Low' or 'H'igh
													// 'R' shown when going into RX immediately
													// after a SWR-Alarm situation
		//
		//-------------------------------------------------------
		// Display "  Voltage and temperature" in second line
		//-------------------------------------------------------
		//
		#if DISP_FAHRENHEIT							// Display temperature in Fahrenheit
													// (threshold still set in deg C)
		rprintf("-%x-%x %2u.%1uV %3uF ", 
			selectedFilters[0],selectedFilters[1],vdd, vdd_tenths, tmp_F);
		#else


		rprintf("-%x%x %2u.%1uV %3d", 
			selectedFilters[0],selectedFilters[1],vdd, vdd_tenths, tmp_C);
			lcd_data(0xdf); lcd_data('C');
		#endif
	}


	#endif//DEBUG_2LN
}	
#endif



//*******************************************************************************************************************



#if LCD_DISPLAY_20x2_b	// Alternate 20x2 LCD display.
//
//-----------------------------------------------------------------------------
//			Display stuff on a 20x2 LCD 
//-----------------------------------------------------------------------------
//
void lcd_display(void)
{       	
	//rprintfInit(lcd_data);

	// Variables used for print formatting, to emulate a floating point printf
	// AVRLIB floating point printf formatting would be very costly in size
	uint16_t vdd_tot, vdd, vdd_tenths, swr, swr_hundredths, swr_tenths, pow_tot, pow, pow_mw;
	
	#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
	static uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer

	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure
	#endif
		
	lcd_command(LCD_DISP_ON);						// Blinking block cursor OFF

	//
	//-------------------------------------------------------
	// Display stuff in first line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,0);								// First char, first line

	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	if(Status1 & PA_CAL)							// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA Bias Calibrate   "));// Lean pgm space efficient LCD print
	}
	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
	//-------------------------------------------------------
	// Display if new Frequency is stored by Shaft Encoder func
	//-------------------------------------------------------
	else if (Status2 & ENC_STORED)
	{
		lcd_putchar_P(PSTR("VFO Memory Stored   "));
		static int count = 0;						// Display for ENC_STORE_DISP * 100ms
		count++;
		if(count>=ENC_STORE_DISP)
		{
			Status2 &= ~ENC_STORED;
			count = 0;
		}
	}
	#endif
	//-------------------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//
	// Power Output Bargraph
	//-------------------------------------------------------
	else if(Status1 & TX_FLAG)
	{
		// Prepare Power readout
		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		static uint8_t i = 0;

		#if	PWR_PEP_ADJUST							// Option to adjust the number of samples in PEP measurement
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= R.PEP_samples) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < R.PEP_samples; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#else
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= PEP_PERIOD) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < PEP_PERIOD; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#endif
		#endif

		// progress, maxprogress, len
		lcdProgressBar(pow_tot/100, R.PWR_fullscale*10, 12);

		pow = pow_tot / 1000; 						// Watts
		pow_mw = pow_tot % 1000;					// milliWatts

		rprintf("P%2u.%03uW", pow, pow_mw);
	}

	//-------------------------------------------------------
	// Display stuff while in Receive "RX" mode
	//
	// Display the Si570 frequency (divided by 4) on the LCD:
	// (Two separate compile time options below)
	//-------------------------------------------------------
	else
	{
		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif		

		// Display the current Si570 frequency on the LCD
		int32_t freq;
	
		freq = R.Freq[0];

		#if PSDR_IQ_OFFSET36					// Display a fixed frequency offset during RX only.
		if(!(Status1 & TX_FLAG))
		{
			freq = freq + R.LCD_RX_Offset;
		}
		#endif	

		uint32_t dispFreq;
		dispFreq = (double) freq * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

		#if PSDR_IQ_OFFSET68					// Display a fixed frequency offset during RX only.
		if(!(Status1 & TX_FLAG))
		{
			dispFreq = dispFreq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
		}
		#endif

		uint16_t fMHz = dispFreq / 1000000;
		uint32_t fHz = dispFreq % 1000000;

		rprintf("%3u.%06lu MHz    RX", fMHz,fHz);
	}	

	
	//
	//-------------------------------------------------------
	// Display stuff in second line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,1);								// First char, second line

	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	int16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.i/128) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	vdd_tot = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	vdd = vdd_tot / 10;
	vdd_tenths = vdd_tot % 10;


	//-------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------
	if (Status1 & TX_FLAG)
	{
		//--------------------------------------------
		// Display a PA thermal runaway situation		
		//--------------------------------------------
		if(Status1 & TMP_ALARM)						// Thermal runaway, TX shutdown
		{
			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("Temp Shutdown:  %3uF", tmp_F);
			#else
			rprintf("Temp Shutdown:  %3d", tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}

		//---------------------------------------------------------
		// Display current during a PA Bias Autotune
		//---------------------------------------------------------
		else if(Status1 & PA_CAL)
		{
			// Fetch and normalize PA current
			uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
			uint16_t idd = idd_ca / 100; 
			idd_ca = idd_ca % 100;

			#if DISP_FAHRENHEIT						// Display temperature in Fahrenheit
													// (threshold still set in deg C)
			rprintf("%2u.%1uV   %01u.%02uA   %3uF", vdd,vdd_tenths,idd,idd_ca,tmp_F);
			#else
			rprintf("%2u.%1uV   %01u.%02uA  %3d", vdd,vdd_tenths,idd,idd_ca,tmp_C);
			lcd_data(0xdf); lcd_data('C');
			#endif
		}

		//--------------------------------------------
		// Display SWR
		//--------------------------------------------
		else
		{
			// Prepare SWR readout
			swr = measured_SWR / 100;				// SWR
			swr_hundredths = measured_SWR % 100;	// sub decimal point, 1/100 accuracy
			swr_tenths = swr_hundredths/10;			// sub decimal point, 1/10 accuracy

			// progress, maxprogress, len
			#if	BARGRAPH_SWR_SCALE					// Add option to adjust the Fullscale value for the SWR bargraph
		// possible bug fix for SWR exceeding lcdProgressBar range
		uint16_t display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? R.SWR_fullscale*100 : (measured_SWR-100);
		lcdProgressBar(display_SWR, R.SWR_fullscale*100, 12);
		#else
		display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? SWR_FULL_SCALE : (measured_SWR-100);
		lcdProgressBar(display_SWR - 100, SWR_FULL_SCALE, 12);
		#endif	
			lcd_putchar_P(PSTR("SWR"));				// Print string from flash rom

			rprintf("%2u.", swr);					// Print the super decimal portion of the SWR
	
			#if SWR_ALARM_FUNC						// SWR alarm function, activates a secondary PTT
			//
			//--------------------------------------------
			// Display a SWR Alarm situation
			//--------------------------------------------
			//
			if(Status1 & SWR_ALARM)					// SWR Alarm flag set
			{
				if(swr >= 100)						// SWR more than 99
					lcd_putchar_P(PSTR("  --"));
				else
					lcd_data(swr_tenths+0x30);		// Print the sub-decimal value of the SWR, single digit precision
													// (a lean byte saving way to print a single decimal)
	
				lcd_data('A');						// A in SWRA indication
				lcd_command(LCD_DISP_ON_BLINK);		// Blinking block cursor ON
			}
			else
			#endif//SWR_ALARM_FUNC
				rprintf("%02u", swr_hundredths);	// Print the sub-decimal value of the SWR, double digit precision
			lcd_gotoxy(19,1);						// In case blinking cursor, put it on "A" in "SWR Alarm"
		}
	}

	//-------------------------------------------
	// Display stuff when in Receive ("RX") mode
	//-------------------------------------------
	else
	{
		//
		//-------------------------------------------------------
		// Display Bias setting in second line of LCD
		//-------------------------------------------------------
		//
		lcd_putchar_P(PSTR("B:"));

		const char bias[]= {'R','L','H'};
		lcd_data(bias[biasInit]);					// Print Bias status, 'Low' or 'H'igh
													// 'R' shown when going into RX immediately
													// after a SWR-Alarm situation
		//
		//-------------------------------------------------------
		// Display "  Voltage and temperature" in second line
		//-------------------------------------------------------
		//

		// Display "  Voltage and temperature" in second line
		#if DISP_FAHRENHEIT							// Display temperature in Fahrenheit
													// (threshold still set in deg C)
		rprintf(" F:%u-%x %2u.%1uV %3uF", 
			selectedFilters[0],selectedFilters[1],vdd, vdd_tenths, tmp_F);
		#else
		rprintf(" F%u-%x %2u.%1uV %3d", 
			selectedFilters[0],selectedFilters[1],vdd, vdd_tenths, tmp_C);
			lcd_data(0xdf); lcd_data('C');
		#endif
	}
}	
#endif//LCD_DISPLAY_20x2_b// Alternate 20x2 LCD display.



//*******************************************************************************************************************



#if LCD_DISPLAY_20x4_b	// 20x4 LCD display.
//
//-----------------------------------------------------------------------------
//			Display stuff on a 20x4 LCD 
//-----------------------------------------------------------------------------
//
void lcd_display(void)
{       	
	#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
	static uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer

	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure
	#endif
		
	//rprintfInit(lcd_data);

	lcd_clrscr();									// Clear Screen is a cheaper operation than
													// writing blanks, though it causes flicker

	lcd_command(LCD_DISP_ON);						// Blinking block cursor OFF

	//
	//-------------------------------------------------------
	// Display stuff in second line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,1);								// First char, first line

	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	uint16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.w>>7) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Display "  Voltage and temperature" in second line
	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
													// (threshold still set in deg C)
	rprintf("%3uF", tmp_F);
	#else
	rprintf("%3d", tmp_C);
	lcd_data(0xdf); lcd_data('C');
	#endif
	
	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	uint16_t vdd_tenths = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	uint16_t vdd = vdd_tenths / 10;
	vdd_tenths = vdd_tenths % 10;
	
	lcd_gotoxy(7,1);
	rprintf("%2u.%1uV", vdd, vdd_tenths);

	//
	//-------------------------------------------------------
	// During TX, Display PA current in second line of LCD
	//-------------------------------------------------------
	//
	if (Status1 & (PA_CAL | TX_FLAG))
	{
		// Fetch and normalize PA current
		uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
		uint16_t idd = idd_ca/100; idd_ca = idd_ca%100;
	
		lcd_gotoxy(13,1);							// Second line, 13th position
		rprintf("%u.%02uA", idd, idd_ca);			// Display current while in transmit
	}

	//
	//-------------------------------------------------------
	// Display alternate text during RX in second line of LCD
	//-------------------------------------------------------
	//
	else
	{
		lcd_putchar_P(PSTR("  Bias:"));				// Print string from flash rom
	}

	//
	//-------------------------------------------------------
	// Display Bias setting in second line of LCD
	//-------------------------------------------------------
	//
	const char bias[]= {'R','L','H'};
	lcd_gotoxy(19,1);								// Second line, 19th position
	lcd_data(bias[biasInit]);						// Print Bias status 'R'educed
													// 'Low' or 'H'igh
	//
	//-------------------------------------------------------
	// Display stuff in first line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,0);								// First char, first line

	//
	//-------------------------------------------------------
	// Display the Si570 frequency (divided by 4) on the LCD:
	//-------------------------------------------------------
	//
	if(Status1 & TX_FLAG)
	{
		lcd_data('T');
	}
	else
	{
		lcd_data('R');
	}

	// Display the current Si570 frequency on the LCD
	int32_t freq;
	
	freq = R.Freq[0];

	#if PSDR_IQ_OFFSET36					// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		freq = freq + R.LCD_RX_Offset;
	}
	#endif

	uint32_t dispFreq;
	dispFreq = (double) freq * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

	#if PSDR_IQ_OFFSET68					// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		dispFreq = dispFreq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
	}
	#endif

	uint16_t fMHz = dispFreq / 1000000;
	uint32_t fHz = dispFreq % 1000000;

	rprintf("X%3u.%06luMHz F%x-%x", fMHz,fHz,selectedFilters[0],selectedFilters[1]);


	//
	//-------------------------------------------------------
	// Display non-TX stuff in third and fourth line
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,3);								// Fourth line

	//--------------------------------------------
	// Display a PA thermal runaway situation		
	//--------------------------------------------
	if(Status1 & TMP_ALARM)							// Thermal runaway, TX shutdown
	{
		lcd_putchar_P(PSTR("Temperature Shutdown"));// A trick to print string from flash rom
													// rather than from SRAM
	}

	//
	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	//
	else if(Status1 & PA_CAL)						// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA Bias Calibrate"));	// Print string from flash rom rather than from SRAM
	}
	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
	//
	//-------------------------------------------------------
	// Display if new Frequency is stored by Shaft Encoder func
	//-------------------------------------------------------
	//
	else if (Status2 & ENC_STORED)
	{
		lcd_putchar_P(PSTR("VFO Memory Stored"));	// Print string from flash rom rather than from SRAM

		static int count = 0;						// Display for ENC_STORE_DISP * 100ms
		count++;
		if(count>=ENC_STORE_DISP)
		{
			Status2 &= ~ENC_STORED;
			count = 0;
		}
	}
	#endif


	//
	//-------------------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------------------
	//
	else if(Status1 & TX_FLAG)
	{

		//
		//-------------------------------------------------------
		// Display TX specific stuff in third and fourth line of LCD
		//-------------------------------------------------------
		//
		lcd_gotoxy(0,2);							// Third line


		//
		//-------------------------------------------------------
		// Power Output Bargraph in third line
		//-------------------------------------------------------
		//
		uint16_t pow_tot, pow, pow_mw;

		// Prepare Power readout
		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		static uint8_t i = 0;

		#if	PWR_PEP_ADJUST							// Option to adjust the number of samples in PEP measurement
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= R.PEP_samples) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < R.PEP_samples; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#else
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= PEP_PERIOD) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < PEP_PERIOD; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#endif
		#endif

		// progress, maxprogress, len
		lcdProgressBar(pow_tot/100, R.PWR_fullscale*10, 12);

		pow = pow_tot / 1000; 						// Watts
		pow_mw = pow_tot % 1000;					// milliWatts

		rprintf("P%2u.%03uW", pow, pow_mw);


		//
		//--------------------------------------------
		// Display SWR Bargraph in fourth line
		//--------------------------------------------
		//
		lcd_gotoxy(0,3);							// Fourth line
		// Prepare SWR readout
		uint16_t swr, swr_hundredths, swr_tenths;
		swr = measured_SWR / 100;					// SWR
		swr_hundredths = measured_SWR % 100;		// sub decimal point, 1/100 accuracy
		swr_tenths = swr_hundredths / 10;			// sub decimal point, 1/10 accuracy

		// progress, maxprogress, len
		#if	BARGRAPH_SWR_SCALE						// Add option to adjust the Fullscale value for the SWR bargraph
		// possible bug fix for SWR exceeding lcdProgressBar range
		uint16_t display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? R.SWR_fullscale*100 : (measured_SWR-100);
		lcdProgressBar(display_SWR, R.SWR_fullscale*100, 12);
		#else
		display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? SWR_FULL_SCALE : (measured_SWR-100);
		lcdProgressBar(display_SWR - 100, SWR_FULL_SCALE, 12);
		#endif	

		lcd_putchar_P(PSTR("SWR"));					// Print string from flash rom

		rprintf("%2u.", swr);						// Print the super decimal portion of the SWR

		#if SWR_ALARM_FUNC							// SWR alarm function, activates a secondary PTT
		//
		//--------------------------------------------
		// Display a SWR Alarm situation
		//--------------------------------------------
		//
		if(Status1 & SWR_ALARM)						// SWR Alarm flag set
		{
			if(swr >= 100)							// SWR more than 99
				lcd_putchar_P(PSTR("  --"));
			else
				lcd_data(swr_tenths+0x30);			// Print the sub-decimal value of the SWR, single digit precision
													// (a lean byte saving way to print a single decimal)

			lcd_data('A');							// A in SWRA indication
			lcd_command(LCD_DISP_ON_BLINK);			// Blinking block cursor ON
		}
		else
		#endif//SWR_ALARM_FUNC
			rprintf("%02u", swr_hundredths);		// Print the sub-decimal value of the SWR, double digit precision

		lcd_gotoxy(19,3);							// In case blinking cursor, put it on "A" in "SWR Alarm"
	}

	//
	//-------------------------------------------
	// Display stuff when in Receive ("RX") mode
	//-------------------------------------------
	//
	else
	{
		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif		

		#if DISP_TERSE								// Option for 20x4 and 40x2 displays
		lcd_gotoxy(12,3);							// Line 4 position 13
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VERBOSE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,2);							// Line 3 position 0
		lcd_putchar_P(PSTR("Softrock 6.3  &"));		// Print string from flash rom
		lcd_gotoxy(12,3);							// Line 4 position 13
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VER_SIMPLE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,3);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		lcd_gotoxy(16,3);
		rprintf("%s",VERSION);						// Print version info
		#endif
		#if DISP_VER_SHORT							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,3);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3  "));			// Print string from flash rom
		rprintf("%s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
		#if DISP_VERSION							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(0,2);							// Line 3 position 0
		lcd_putchar_P(PSTR("SR 6.3 <--> Mobo 4.3"));// Print string from flash rom
		lcd_gotoxy(5,3);							// Line 4 position 5
		rprintf("Ver: %s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
	}
}	
#endif//LCD_DISPLAY_20x4_b// 20x4 LCD display.



//*******************************************************************************************************************



#if LCD_DISPLAY_40x2_b	// Alternate 40x2 LCD display.
//
//-----------------------------------------------------------------------------
//			Display stuff on a 40x2 LCD 
//-----------------------------------------------------------------------------
//
void lcd_display(void)
{       	
	#if PWR_PEAK_ENVELOPE							//PEP measurement. Highest value in buffer shown
	static uint16_t pow_avg[PEP_MAX_PERIOD];		// Power measurement ringbuffer

	if (R.PEP_samples > PEP_MAX_PERIOD) R.PEP_samples = PEP_MAX_PERIOD;// Safety measure
	#endif
		
	//rprintfInit(lcd_data);

	lcd_clrscr();									// Clear Screen is a cheaper operation than
													// writing blanks, though it causes flicker

	lcd_command(LCD_DISP_ON);						// Blinking block cursor OFF

	//
	//-------------------------------------------------------
	// Display stuff in second line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,1);								// First char, first line

	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
	int16_t tmp_F;									// (threshold still set in deg C)
	tmp_F = ((tmp100_data.i/128) * 9)/10 + 32;
	#else
	int8_t tmp_C = tmp100_data.i1;
	#endif

	// Display "  Voltage and temperature" in second line
	#if DISP_FAHRENHEIT								// Display temperature in Fahrenheit
													// (threshold still set in deg C)
	rprintf("%3uF", tmp_F);
	#else
	rprintf("%3d", tmp_C);
	lcd_data(0xdf); lcd_data('C');
	#endif
	
	// Prep Voltage readout.  Max voltage = 15.6V (5V * 14.7k/4.7k)
	uint16_t vdd_tenths = ((uint32_t) ad7991_adc[AD7991_PSU_VOLTAGE].w * 156) / 0xfff0;
	uint16_t vdd = vdd_tenths / 10;
	vdd_tenths = vdd_tenths % 10;
	
	lcd_gotoxy(7,1);								// Second line, seventh position
	rprintf("%2u.%1uV", vdd, vdd_tenths);

	//
	//-------------------------------------------------------
	// During TX, Display PA current in second line of LCD
	//-------------------------------------------------------
	//
	if (Status1 & (PA_CAL | TX_FLAG))
	{
		// Fetch and normalize PA current
		uint16_t idd_ca = ad7991_adc[AD7991_PA_CURRENT].w / 262;
		uint16_t idd = idd_ca/100; idd_ca = idd_ca % 100;
	
		lcd_gotoxy(13,1);							// Second line, 13th position
		rprintf("%u.%02uA", idd, idd_ca);			// Display current while in transmit
	}

	//
	//-------------------------------------------------------
	// Display alternate text during RX in second line of LCD
	//-------------------------------------------------------
	//
	else
	{
		lcd_putchar_P(PSTR("  Bias:"));				// Print string from flash rom
	}

	//
	//-------------------------------------------------------
	// Display Bias setting in second line of LCD
	//-------------------------------------------------------
	//
	const char bias[]= {'R','L','H'};
	lcd_gotoxy(19,1);								// Second line, 19th position
	lcd_data(bias[biasInit]);						// Print Bias status 'R'educed
													// 'Low' or 'H'igh
	//
	//-------------------------------------------------------
	// Display stuff in first line of LCD
	//-------------------------------------------------------
	//
	lcd_gotoxy(0,0);								// First char, first line

	//
	//-------------------------------------------------------
	// Display the Si570 frequency (divided by 4) on the LCD:
	//-------------------------------------------------------
	//
	if(Status1 & TX_FLAG)
	{
		lcd_data('T');
	}
	else
	{
		lcd_data('R');
	}

	// Display the current Si570 frequency on the LCD
	int32_t freq;
	
	freq = R.Freq[0];

	#if PSDR_IQ_OFFSET36					// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		freq = freq + R.LCD_RX_Offset;
	}
	#endif

	uint32_t dispFreq;
	dispFreq = (double) freq * 1000000.0 / _2(23);	// R.Freq / 2^21 / 4 = Lo frequency

	#if PSDR_IQ_OFFSET68					// Display a fixed frequency offset during RX only.
	if(!(Status1 & TX_FLAG))
	{
		dispFreq = dispFreq + R.LCD_RX_Offset * 1000; // 1000.0 * _2(23);
	}
	#endif

	uint16_t fMHz = dispFreq / 1000000;
	uint32_t fHz = dispFreq % 1000000;

	rprintf("X%3u.%06luMHz F%x-%x", fMHz,fHz,selectedFilters[0],selectedFilters[1]);

	//
	//-------------------------------------------------------
	// Display non-TX stuff in second half of first and second line
	//-------------------------------------------------------
	//
	lcd_gotoxy(20,1);								// Second half, second line

	//--------------------------------------------
	// Display a PA thermal runaway situation		
	//--------------------------------------------
	if(Status1 & TMP_ALARM)							// Thermal runaway, TX shutdown
	{
		lcd_putchar_P(PSTR("Temperature Shutdown"));// A trick to print string from flash rom
													// rather than from SRAM
	}

	//
	//-------------------------------------------------------
	// Display PA Bias tune indication
	//-------------------------------------------------------
	//
	else if(Status1 & PA_CAL)						// PA bias cal in progress flags
	{
		lcd_putchar_P(PSTR("PA Bias Calibrate"));	// Print string from flash rom rather than from SRAM
	}
	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
	//
	//-------------------------------------------------------
	// Display if new Frequency is stored by Shaft Encoder func
	//-------------------------------------------------------
	//
	else if (Status2 & ENC_STORED)
	{
		lcd_putchar_P(PSTR("VFO Memory Stored"));	// Print string from flash rom rather than from SRAM

		static int count = 0;						// Display for ENC_STORE_DISP * 100ms
		count++;
		if(count>=ENC_STORE_DISP)
		{
			Status2 &= ~ENC_STORED;
			count = 0;
		}
	}
	#endif


	//
	//-------------------------------------------------------
	// Display stuff while in Transmit "TX" mode
	//-------------------------------------------------------
	//
	else if(Status1 & TX_FLAG)
	{

		//
		//-------------------------------------------------------
		// Display TX specific stuff in second half of first and second line of LCD
		//-------------------------------------------------------
		//
		lcd_gotoxy(20,0);							// Second half, first line

		//
		//-------------------------------------------------------
		// Power Output Bargraph in third line
		//-------------------------------------------------------
		//
		uint16_t pow_tot, pow, pow_mw;

		// Prepare Power readout
		pow_tot = measured_Power(ad7991_adc[AD7991_POWER_OUT].w);// Power in mW (max 65535mW)

		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		static uint8_t i = 0;

		#if	PWR_PEP_ADJUST							// Option to adjust the number of samples in PEP measurement
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= R.PEP_samples) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < R.PEP_samples; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#else
		pow_avg[i] = pow_tot;						// Store value in ringbuffer
		i++;
		if (i >= PEP_PERIOD) i = 0;

		pow_tot = 0;
		for (uint8_t j = 0; j < PEP_PERIOD; j++)	// Retrieve the largest value out of the measured window
		{
			if (pow_avg[j] > pow_tot) pow_tot = pow_avg[j];
		}
		#endif
		#endif

		// progress, maxprogress, len
		lcdProgressBar(pow_tot/100, R.PWR_fullscale*10, 12);

		pow = pow_tot / 1000; 						// Watts
		pow_mw = pow_tot % 1000;					// milliWatts

		rprintf("P%2u.%03uW", pow, pow_mw);


		//
		//--------------------------------------------
		// Display SWR Bargraph in fourth line
		//--------------------------------------------
		//
		lcd_gotoxy(20,1);							// Second half, second line
		// Prepare SWR readout
		uint16_t swr, swr_hundredths, swr_tenths;
		swr = measured_SWR / 100;					// SWR
		swr_hundredths = measured_SWR % 100;		// sub decimal point, 1/100 accuracy
		swr_tenths = swr_hundredths / 10;			// sub decimal point, 1/10 accuracy

		// progress, maxprogress, len
		#if	BARGRAPH_SWR_SCALE						// Add option to adjust the Fullscale value for the SWR bargraph	
		// possible bug fix for SWR exceeding lcdProgressBar range
		uint16_t display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? R.SWR_fullscale*100 : (measured_SWR-100);
		lcdProgressBar(display_SWR, R.SWR_fullscale*100, 12);
		#else
		display_SWR = ((measured_SWR-100) > R.SWR_fullscale*100) ? SWR_FULL_SCALE : (measured_SWR-100);
		lcdProgressBar(display_SWR - 100, SWR_FULL_SCALE, 12);
		#endif	
		lcd_putchar_P(PSTR("SWR"));					// Print string from flash rom

		rprintf("%2u.", swr);						// Print the super decimal portion of the SWR

		#if SWR_ALARM_FUNC							// SWR alarm function, activates a secondary PTT
		//
		//--------------------------------------------
		// Display a SWR Alarm situation
		//--------------------------------------------
		//
		if(Status1 & SWR_ALARM)						// SWR Alarm flag set
		{
			if(swr >= 100)							// SWR more than 99
				lcd_putchar_P(PSTR("  --"));
			else
				lcd_data(swr_tenths+0x30);			// Print the sub-decimal value of the SWR, single digit precision
													// (a lean byte saving way to print a single decimal)

			lcd_data('A');							// A in SWRA indication
			lcd_command(LCD_DISP_ON_BLINK);			// Blinking block cursor ON
		}
		else
		#endif//SWR_ALARM_FUNC
			rprintf("%02u", swr_hundredths);		// Print the sub-decimal value of the SWR, double digit precision

		lcd_gotoxy(39,1);							// In case blinking cursor, put it on "A" in "SWR Alarm"
	}

	//
	//-------------------------------------------
	// Display stuff when in Receive ("RX") mode
	//-------------------------------------------
	//
	else
	{
		#if PWR_PEAK_ENVELOPE						//PEP measurement. Highest value in buffer shown
		for (uint8_t j = 0; j < PEP_MAX_PERIOD; j++)// Clear PEP storage during receive
		{
			pow_avg[j]=0;
		}
		#endif		

		#if DISP_TERSE								// Option for 20x4 and 40x2 displays
		lcd_gotoxy(32,1);							// Line 2 position 32
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VERBOSE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,0);							// Line 1 position 21
		lcd_putchar_P(PSTR("Softrock 6.3  &"));		// Print string from flash rom
		lcd_gotoxy(32,1);							// Line 2 position 32
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		#endif
		#if DISP_VER_SIMPLE							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,1);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3"));			// Print string from flash rom
		lcd_gotoxy(36,1);
		rprintf("%s",VERSION);						// Print version info
		#endif
		#if DISP_VER_SHORT							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,1);							// Line 4 position 0
		lcd_putchar_P(PSTR("Mobo 4.3 "));			// Print string from flash rom
		rprintf("%s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
		#if DISP_VERSION							// Option for 20x4 and 40x2 displays
		lcd_gotoxy(21,0);							// Line 1 position 20
		lcd_putchar_P(PSTR("SR 6.3 <--> Mobo 4.3"));// Print string from flash rom
		lcd_gotoxy(25,1);							// Line 2 position 25
		rprintf("Ver: %s-%u.%u",VERSION,VERSION_MAJOR,VERSION_MINOR);// Print version info
		#endif
	}
}	
#endif//LCD_DISPLAY_40x2_b// Alternate 40x2 LCD display.

#endif		//(LCD_PAR_DISPLAY || LCD_I2C_DISPLAY)// Identical routines for I2C and parallel connected displays


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
//**
//** Initial version.: 2009-09-06, Loftur Jonasson, TF3LJ
//** Last update.....: 2010-02-28, Loftur Jonasson, TF3LJ
//**                   Check the Mobo.c file
//**
//*********************************************************************************


#include "Mobo.h"

#if POWER_SWR													// Power and SWR measurement



//
//-----------------------------------------------------------------------------------------
// 						Do SWR calcultions and control the PTT2 output
//-----------------------------------------------------------------------------------------
// Read the ADC inputs. 
// The global variable ad7991_adc[AD7991_POWER_OUT] contains a measurement of the
// Power transmitted,
// and the global variable ad7991_adc[AD7991_POWER_REF] contains a measurement of the
// Power reflected,
void Test_SWR(void)
{

	uint16_t swr = 100;											// Initialize SWR = 1.0

	#if SWR_ALARM_FUNC											// SWR alarm function, activates a secondary PTT
	static uint8_t second_pass=0, timer=0;
	#endif//SWR_ALARM_FUNC										// SWR alarm function, activates a secondary PTT

	// There is no point in doing a SWR calculation, unless keyed and on the air
	if(!(Status1 & TMP_ALARM) && (Status1 &TX_FLAG))
	{
		//-------------------------------------------------------------
		// Calculate SWR
		//-------------------------------------------------------------
		// Quick check for an invalid result 
		if (ad7991_adc[AD7991_POWER_OUT].w < V_MIN_TRIGGER*0x10)
			swr = 100;											// Too little for valid measurement, SWR = 1.0
		else if (ad7991_adc[AD7991_POWER_REF].w >= ad7991_adc[AD7991_POWER_OUT].w)
			swr = 9990; 										// Infinite (or more than infinite) SWR:

		// Standard SWR formula multiplied by 100, eg 270 = SWR of 2.7
		else
		{
			swr = (uint32_t) 100 * 
			((uint32_t)ad7991_adc[AD7991_POWER_OUT].w + (uint32_t)ad7991_adc[AD7991_POWER_REF].w)
				/
			(uint32_t)(ad7991_adc[AD7991_POWER_OUT].w - ad7991_adc[AD7991_POWER_REF].w);
		}	
		if (swr < 9990)											// Set an upper bound to avoid overrrun.
			measured_SWR = swr;
		else measured_SWR = 9990;		

		#if SWR_ALARM_FUNC										// SWR alarm function, activates a secondary PTT
		//-------------------------------------------------------------
		// SWR Alarm function
		// If PTT is keyed, key the PTT2 line according to SWR status
		//-------------------------------------------------------------
		// On measured Power output and high SWR, force clear RXTX2 and seed timer

		// Compare power measured (in mW) with min Trigger value
		if ((measured_Power(ad7991_adc[AD7991_POWER_OUT].w) > R.P_Min_Trigger) 	

			&& (measured_SWR > 10*R.SWR_Trigger)) 				// SWR Trigger value is a 10x value,
																// e.g. 27 corresponds to an SWR of 2.7.
		{
			if (!second_pass)									// First time, set flag, no action yet
				second_pass++;
			else												// There have been two or more consecutive measurements
																// with high SWR, take action
			{
				Status1 |= SWR_ALARM;							// Set SWR alarm flag
				#if MOBO_STYLE_IO
				#if  REVERSE_PTT2_LOGIC							// Switch the PTT2 logic
				MoboPCF_clear(Mobo_PCF_TX2);					// Clear PTT2 line
				#else//not REVERSE_PTT2_LOGIC					// Normal PTT2 logic
				MoboPCF_set(Mobo_PCF_TX2);						// Set PTT2 line
				#endif//REVERSE_PTT2_LOGIC						// end of Switch the PTT2 logic
				#endif//MOBO_STYLE_IO
				timer = R.SWR_Protect_Timer;					// Seed SWR Alarm patience timer
			}
		}
		// If SWR OK and timer has been zeroed, set the PTT2 line
		else 
		{
			if (timer == 0)
			{
				Status1 &= ~SWR_ALARM;							// Clear SWR alarm flag
				#if MOBO_STYLE_IO
				#if  REVERSE_PTT2_LOGIC							// Switch the PTT2 logic
				MoboPCF_set(Mobo_PCF_TX2);						// Set PTT2 line
				#else//not REVERSE_PTT2_LOGIC					// Normal PTT2 logic
				MoboPCF_clear(Mobo_PCF_TX2);					// Clear PTT2 line
				#endif//REVERSE_PTT2_LOGIC						// end of Switch the PTT2 logic
				#endif//MOBO_STYLE_IO
				second_pass = 0;								// clear second pass flag
			}
			else
			{
				timer--;
			} 
		}
		#endif//SWR_ALARM_FUNC									// SWR alarm function, activates a secondary PTT
	}

	//-------------------------------------------------------------
	// Not Keyed - Clear PTT2 line
	//-------------------------------------------------------------
	else
	{
				Status1 &= ~SWR_ALARM;							// Clear SWR alarm flag
				#if MOBO_STYLE_IO
				#if  REVERSE_PTT2_LOGIC							// Switch the PTT2 logic
				MoboPCF_clear(Mobo_PCF_TX2);					// Clear PTT2 line
				#else//not REVERSE_PTT2_LOGIC					// Normal PTT2 logic
				MoboPCF_set(Mobo_PCF_TX2);						// Set PTT2 line
				#endif//REVERSE_PTT2_LOGIC						// end of Switch the PTT2 logic
				#endif//MOBO_STYLE_IO
	}
}



//
//-----------------------------------------------------------------------------------------
// 				Convert AD reading into "Measured Power in milliWatts"
//-----------------------------------------------------------------------------------------
//

// A simplified integer aritmetic version, still with decent accuracy
// (the return value overflows above 65W max)
// (comparison Ref 11604 bytes)

uint16_t measured_Power(uint16_t voltage)
{
	// All standard stuff
	// normalise the measured value from the VSWR bridge
	// Reference voltage is 5V, 
	// diode offset ~ .15V
	// R.PWR_Calibrate = Power meter calibration value
	uint32_t measured_P; 
	
	if (voltage > 0) voltage = voltage/0x10 + 82;				// If no input voltage, then we do not add offset voltage
																// as this would otherwise result in a bogus minimum power
																// reading
																// voltage is a MSB adjusted 12 bit value, therefore 
																// dividing by 10 does not lose any information
																// 4096 = 5V,
																// 82 = 100mV, compensating for schottky diode loss
	// Formula roughly adjusted for the ratio in the SWR bridge
	measured_P = (uint32_t)voltage * R.PWR_Calibrate/84;
	measured_P = (measured_P*measured_P)/50000;
	return (uint16_t) measured_P;								// Return power in mW
}
#endif//POWER_SWR												// Power and SWR measurement



//
//-----------------------------------------------------------------------------------------
// 								RD16HHF1 PA Bias management
//-----------------------------------------------------------------------------------------
//
void PA_bias(void)


{
	uint8_t static calibrate = 0;								// Current calibrate value

	switch (R.Bias_Select)
	{
		//-------------------------------------------------------------
		// Set RD16HHF1 Bias to LO setting, using stored calibrated value
		//-------------------------------------------------------------
		case 1:													// Set RD16HHF1 PA bias for Class AB
			if(biasInit != 1)									// Has this been done already?
				ad5301(R.AD5301_I2C_addr, R.cal_LO);			// No, set bias
			biasInit = 1;
			break;
		//-------------------------------------------------------------
		// Set RD16HHF1 Bias to HI setting,  using stored calibrated value
		//-------------------------------------------------------------
		case 2:													// Set RD16HHF1 PA bias for Class A
			if (Status1 & SWR_ALARM)							// Whoops, we have a SWR situation
			{
				ad5301(R.AD5301_I2C_addr, R.cal_LO);			// Set lower bias setting
				biasInit = 0;
			}
			else if(biasInit != 2 )								// Has this been done already?
			{
				ad5301(R.AD5301_I2C_addr, R.cal_HI);				// No, set bias
				biasInit = 2;
			}
			break;
		//-------------------------------------------------------------
		// Calibrate RD16HHF1 Bias 
		//-------------------------------------------------------------
		default:												// Calibrate RD16HHF1 PA bias
			if (!(Status1 & TMP_ALARM))							// Proceed if there are no inhibits
			{
				Status1 |= TX_FLAG | PA_CAL;					// Set a couple of progress flags

				// Switch to Transmit mode, set TX out
				#if MOBO_STYLE_IO
				MoboPCF_clear(Mobo_PCF_TX);
				#endif//MOBO_STYLE_IO
				#if OLDSTYLE_IO
				IO_PORT_PTT_CWKEY |= IO_PTT;
				#endif//OLDSTYLE_IO

				// Is current larger or equal to setpoint for class AB?
				if((ad7991_adc[AD7991_PA_CURRENT].b1 >= R.Bias_LO) && !(Status1 & PA_CAL_LO))
				{
					Status1 |= PA_CAL_LO;						// Set flag, were done with class AB
					R.cal_LO = calibrate;						// We have bias, store
					// eeprom_write_block appears to take (14 bytes) less pgm space than eeprom_write_byte 
					eeprom_write_block(&R.cal_LO, &E.cal_LO, sizeof (uint8_t));
				}
				
				// Is current larger or equal to setpoint for class A?
				if((ad7991_adc[AD7991_PA_CURRENT].b1 >= R.Bias_HI) && !(Status1 & PA_CAL_HI))
				{
					Status1 |= PA_CAL_HI;						// Set flag, we're done with class A
					R.cal_HI = calibrate;						// We have bias, store
					eeprom_write_block(&R.cal_HI, &E.cal_HI, sizeof (uint8_t));
				}
				
				// Have we reached the end of our rope?
				if(calibrate == 0xff)	
				{
					Status1 |= PA_CAL_HI;						// Set flag as if done with class AB
					R.cal_HI = R.cal_LO = 0;					// We have no valid bias setting
					eeprom_write_block(&R.cal_LO, &E.cal_LO, sizeof (uint8_t));	// store 0 for both Class A and Class AB
					eeprom_write_block(&R.cal_HI, &E.cal_HI, sizeof (uint8_t));
				}
				
				// Are we finished?
				if (Status1 & PA_CAL_HI)
				{
					Status1 &= ~(PA_CAL_LO | PA_CAL_HI | PA_CAL | TX_FLAG );// We're done, Clear all flags

					// Swtich back to Receive mode, key the TX down
					#if MOBO_STYLE_IO
					MoboPCF_set(Mobo_PCF_TX);
					#endif//MOBO_STYLE_IO
					#if OLDSTYLE_IO
					IO_PORT_PTT_CWKEY &= ~IO_PTT;
					#endif//OLDSTYLE_IO

					calibrate = 0;								// Clear calibrate value (for next round)
					R.Bias_Select = 2;							// Set bias select for class A and store
					eeprom_write_block(&R.Bias_Select, &E.Bias_Select, sizeof (uint8_t));
					//implicit, no need:			
					//ad5301(R.AD5301_I2C_addr, R.cal_HI);			// Set bias	at class A value				
				}

				// Crank up the bias
				else
				{
					calibrate++;								// Crank up the bias by one notch
					ad5301(R.AD5301_I2C_addr, calibrate);		// for the next round of measurements
				}
			}
	}
}


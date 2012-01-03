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
//**                   The code, although bloated up to about four times its original
//**                   size to cater for the functions needed for a "full featured"
//**                   SDR transceiver, still retains the original command
//**                   structure and is compatible with V 15.12 of the PE0FKO code.
//**                   Thanks Fred!!!
//**
//** 
//**                   Functionality highlights description:
//**
//**                    1) Control of the Si570 Programmable Crystal Oscillator,
//**                       including "smoothtune" for seamless tuning between small
//**					   frequency steps (<3500ppm)
//**                    2) Variable speed VFO, using a rotary encoder, and 9 short/long term memories,
//**                       using a pushbutton (cycle with short pushes, store with long)
//**					3) Accomodate different Encoder Resolutions (PPT) through a User command
//**                    4) CW Paddle inputs
//**                    5) PTT output
//**                    6) PTT2 output (connected to SWR protect function)
//**                    7) Automatic selection between 8 Bandpass filters,
//**                       with user selectable switchpoints
//**                    8) Automatic selection between [Default] 8 or 16 Lowpass filters, with user
//**                       selectable switchpoints, by controlling 2x external PCF8574 8 bit GPIO extenders.
//**                       Alternately, it can also provide direct control of 4 to 8 Lowpass filters
//**                       through a 3 bit directly connected GPIO
//**                       port  (Mobo 4.3 P1 pins 4-6).
//**                    9) Measurement of Input voltage, PA current, Power forward and Power refleced 
//**                       (uses external Power/SWR bridge)
//**                   10) Hi-SWR protect function, when using an external Power/SWR bridge
//**                       (automatically lower PA bias and assert an external signal)
//**                   11) PEP indication for power output, shows the highest measured value within a
//**                       user adjustable time window of up to 2 seconds (10 samples per second).
//**                   12) Power Amplifier (PA) Temperature measurement and automatic Transmit Disable
//**                       on a high temperature condition
//**                   13) Cooling FAN control, using two temperature trigger settings, one for on and
//**                       another one for off.
//**                   14) Manual PA Bias setpoint control, switching between different bias setpoints
//**                       (such as between classes A and AB), and automatic calibration of the
//**                       PA bias current
//**                   15) 20x4 or 40x2 LCD display with analog-style bargraphs for enhanced power output 
//**                       and SWR visualisation.
//**                   16) optional 16x2 and 20x2 LCD display alternatives with analog-style bargraphs for
//**                       power output and SWR visualisation.
//**                   17) an alternative optional 16x2 LCD display without the bargraphs, providing Frequency
//**                       readout, Power/SWR metering and status diplay (RX/TX/hi-SWR/hi-TMP/Mem-store...)
//**                   18) LCD frequency display offset option, for readout of the actual transmit/receive
//**                       frequency, when using PowerSDR-IQ
//**                   19) Read/Write a byte from/to external I2C connected General Purpose IO extenders
//**                       PCF8574 (filters, attenuators, amplifiers)
//**
//**
//** Initial version.: 2009-09-09, Loftur Jonasson, TF3LJ
//**
#define                VERSION "1.07"   // 1.07 beta 2011-01-15
//**                   (version optionally displayed on larger LCDs)
//**
//**
//** History.........: Check the Mobo.c file
//**                   
//**
//*********************************************************************************

/*
  USB functionality is realised with the LUFA package:	

  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef _TF3LJ_MOBO_H_
#define _TF3LJ_MOBO_H_ 1

#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/power.h>
#include "AVRLIB/rprintf.h"							// AVRLIB (not AVRLibc) functions

//#include "USB-Descriptors.h"	// This would be used with the USB_SERIAL_ID #define



#define	VERSION_MAJOR		16	// USB Protocol Version scheme, co-ordinated with PE0FKO
#define	VERSION_MINOR		13

#define	EXT_SET_FEATURES	1	// Use an external file, Mobo-Features.h to select the
								// working configuration.  If this option is selected,
								// then the features section below becomes inactive.

#if EXT_SET_FEATURES			// Mobo-Features.h contains the Compile Time Features Selection
#include "Mobo-Features.h"
#endif

#if !EXT_SET_FEATURES			// Features are selected in the below section								
//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x02// When started, the firmware examines this "Serial Number
								// and enforce factory reset if there is a mismatch.
								// This is useful if the EEPROM structure has been modified

//-----------------------------------------------------------------------------
// LED Blink
//
// A couple of totally useless defines with no clear function or purpose
// If either one of these two defines is selected, then normally none of the LED defines
// in the two groups further down should be selected (possible to mix to some extent)
#define	FADE_LOOP_THRU_LEDS	0	// Fun and games. Slowly adjust the blink period, both leds  *OR*
								// This one also asserts Led2 constantly ON during TX
								// (26 bytes more than USBTRAFFIC_LED1 & SLOW_LOOP_THRU_LED2)
#define	BLNK_LOOP_THRU_LEDS	0	// More fun and games. Slowly increase the blink frequency
								// This one also asserts Led2 constantly ON during TX
								// (46 bytes more than USBTRAFFIC_LED1 & SLOW_LOOP_THRU_LED2)

// None, or only one of the four, USBTRAFFIC_LED1 or FAST/SLOW LOOP_THRU_LED1 should be selected
#define USBTRAFFIC_LED1		1	// Blink PB2 LED to show USB activity, *OR*
#define FAST_LOOP_THRU_LED1	0	// Blink PB2 LED every time, when going through the mainloop *OR*
#define	MED_LOOP_THRU_LED1	0	// Blink PB2 LED every 10ms, when going through the mainloop *OR*
#define SLOW_LOOP_THRU_LED1	0	// Blink PB2 LED every 100ms, when going through the mainloop

// None, or only one of the six, typically FAST/SLOW LOOP_THRU_LED2 or OLDSTYLE_IO should be selected
#define	FADE_LOOP_THRU_LED2	0	// Some slight fun and games.  Slowly increase the blink period *OR*
								// (uses 30 bytes more than SLOW_LOOP_THRU_LED2)
#define	BLNK_LOOP_THRU_LED2	0	// More fun and games.  Slowly increase the blink frequency     *OR*
								// (uses 34 bytes more than SLOW_LOOP_THRU_LED2)
#define FAST_LOOP_THRU_LED2	0	// Blink PB3 LED every time, when going through the mainloop    *OR*
#define	MED_LOOP_THRU_LED2	0	// Blink PB3 LED every 10ms, when going through the mainloop    *OR*
#define SLOW_LOOP_THRU_LED2	1	// Blink PB3 LED every 100ms, when going through the mainloop   *OR*

//-----------------------------------------------------------------------------
// Legacy commands and functions
// Depending on the implementation, these are most likely not be needed
//
#define LEGACY_CMD01_04		0	// Commands 0x01 - 0x04. Direct GPIO. Normally not needed
								// (Cost 34 bytes)
#define LEGACY_CMD15_16		0	// Commands 0x15 - 0x16. Direct GPIO. Normally not needed
								// (Cost 40 bytes)
#define LEGACY_CMD_20		0	// Command 0x20. Write byte to Si570 register.  Normally not needed
								// (Cost 32 bytes)
#define LEGACY_CMD_40		0	// Command 0x40. I2CErrors. Normally not needed
								// (Cost 18 bytes)

//-----------------------------------------------------------------------------
// Which kind of output are we going to use?
//
#define OLDSTYLE_IO			0	// TX bit output on PORTB (LED on PB3)
#define MOBO_STYLE_IO		1	// TX and TX2 output on Mobo PCF8574 chip
#define REVERSE_PTT2_LOGIC	0	// Reverse the logic of the PTT2 signal pin

// TX Filter Controls.  Select only one of the below:
#define DDRD_TX_FILTER_IO	0	// TX filter controls on PortD.  Used for control of external Low Pass filters
								// other than the Mega Filter Mobo.  Output is 4bit binary on Port D, pins 0 - 3.
								// Use only in combination with I2C_TX_FILTER_IO (Cost 38 bytes)
#define I2C_08_FILTER_IO	1	// TX filter controls using 1x external PCF8574 over I2C
								// (Cost 28 bytes)
#define I2C_16_FILTER_IO	0	// TX filter controls using 2x external PCF8574 over I2C        *OR*
								// (Cost 64 bytes [12224-12288])
#define	PCF_FILTER_IO		0	// 8x BCD TX filter control using 3 bits on the Mobo PCF8574 chip
								// switches P1 pins 4-6 (Cost 18 bytes)                          *OR*
#define M0RZF_FILTER_IO		0	// TX filter control using 3 bits on the Mobo PCF8574 chip to
								// provide control for the 4 switchable LPFs on the M0RZF
								// 20W amplifier module. P1 Pins 4-6 are used (Cost 52 bytes)

// These two can be used for various peripheral control (cooling fan, attenuators/preamp/antenna switching...)
#define PCF_WRITE_COMMAND	1	// Enable command 0x6e for direct control of PCF8574 extenders
								// (cost 20 bytes)
#define PCF_READ_COMMAND	1	// Enable command 0x6f for direct control of PCF8574 extenders
								// (cost 36 bytes)

//-----------------------------------------------------------------------------
// A number of Bells and whistles, some of them optional, based on the particular appliation
//
#define	POWER_SWR			1	// Measure, and if LCD is enabled, then display Power and SWR. 
								// If this is not defined, while 16x2 non-bargraph LCD is defined,
								// then LCD displays Vdd and I-Pa (Cost 200 or 412 bytes)
#define SWR_ALARM_FUNC		1	// SWR alarm function, activates a secondary PTT
								// with auto Hi-SWR shutdown. Is dependent
								// on POWER_SWR being defined as well
								// (Costs an additional 238 or 356 bytes).

// None, or only one of the two, SLOW_POLL_DURING_RX or SSLO_POLL_DURING_RX should be selected
#define SLOW_POLL_DURING_RX	0	// Poll the I2C bus at 1s intervals only during Receive (potentially less RX noise)
								// (Cost 84 bytes)
#define SSLO_POLL_DURING_RX	1	// Poll the I2C bus at 10s intervals only during Receive (potentially less RX noise)
								// (Cost 98 bytes)

// Normally either one or the other of the two below is disabled to protect transmitter hardware
#define FRQ_CGH_DURING_TX	1	// Enable Si570 frequency change during Transmit
#define FLTR_CGH_DURING_TX	0	// Allow Filter changes when frequency is changed during TX

// None, or only one of the two, CALC_FREQ_MUL_ADD or CALC_BAND_MUL_ADD should be selected
#define CALC_FREQ_MUL_ADD	0	// Frequency Subtract and Multiply Routines (for smart VFO)
								// normally not needed with Mobo 4.3.  (Costs about 216 bytes), *OR*
#define CALC_BAND_MUL_ADD	0	// Band dependent Frequency Subtract and Multiply Routines
								// (for smart VFO) normally not needed with Mobo 4.3.
								// Not fully tested.  Will not work on an AT90USB162, due to 
								// memory size limitations (works on ATmega32u2).
								// (Costs 512 bytes)

#define SCRAMBLED_FILTERS	0	// Enable a non contiguous order of filters (Cmds 0x18, 19, 1a & 1b)
								// (Cost 178 bytes)

// Either one or neither of the two below (first one is legacy, second now preferred):
#define PSDR_IQ_OFFSET36	0	// Display a fixed frequency offset during RX only.  This can be used
								// to always display the actual used frequency, when using PowerSDR-IQ
								// (Cost appr 58 bytes)
#define PSDR_IQ_OFFSET68	1	// Display a fixed frequency offset during RX only.  This can be used
								// to always display the actual used frequency, when using PowerSDR-IQ
								// (Cost appr 78 bytes)

#define	FAN_CONTROL			1	// Turn PA Cooling FAN On/Off, based on temperature
// Only one of the three below is selected with the FAN Control
#define	PORTD_FAN			0	// Port D Pin for FAN Control (Normally PD0, but definable below) 
								// This command is not compatible with DDRD_TX_FILTER_IO
								// as it uses one of the same Port D output bits
								// (Cost appr 146 bytes)
#define	BUILTIN_PCF_FAN		0	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		1	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select either one of the two below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines *OR*
#define LCD_PAR_DISPLAY		0	// 4 bit Parallel (HD44780) connected LCD display, more memory efficient,
								// but noisier (constant writes to LCD)             *OR*
#define LCD_I2C_DISPLAY		0	// Alternate 16x2 I2C connected LCD display... incomplete, placeholder

// If using LCD display, define one of the five below.  20x4 and 40x2 are available for both LCD_PAR_DISPLAY2
// and LCD_PAR_DISPLAY.  16x2 and 20x2 are only available for the 'noisier' LCD_PAR_DISPLAY
// Only one of these can be defined at a time.
#define LCD_DISPLAY_16x2	0	// Original Default Display Routines for 16x2 LCD
								// (Cost up to 3.8kB depending on options below) *OR*
#define LCD_DISPLAY_16x2_b	0	// Alternate 16x2 LCD display with bargraph.     *OR*
								// Requires PWR_SWR option to be enabled
								// (Costs 310 bytes more than LCD_DISPLAY_16x2 with Autoscale)
#define LCD_DISPLAY_20x2_b	0	// Alternate 20x2 LCD display with bargraph.     *OR*
								// (costs 30 bytes in addition to the 16x2 bargraph display)
#define LCD_DISPLAY_20x4_b	1	// Alternate 20x4 LCD display with bargraph.     *OR*
								// (costs 102 bytes in addition to the 16x2 bargraph display)
#define LCD_DISPLAY_40x2_b	0	// Alternate 40x2 LCD display.
								// (costs 86 bytes in addition to the 16x2 bargraph display)

#define CHANGE_I2C_LCD_ADDR	0	// Only used once with a fresh I2C LCD display which has the default address 
								// 0x28 (0x50).This changes the I2C address of the LCD to the address defined
								// further down in this file.  Use only once and then disable, conflicts with
								// the AD7991-0 address

#define PWR_PEAK_ENVELOPE	1	// PEP measurement for LCD display. Highest value in buffer shown (Cost 64 bytes)

#define	PWR_PEP_ADJUST		1	// Option to adjust the number of samples in PEP measurement through USB Cmd 0x66

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with 16x2 and 20x2 non-bargraph LCD
								// (Costs 136 bytes)

#define DISP_FAHRENHEIT		1	// Used with LCD display, Display temperature in Fahrenheit
								// threshold still set in deg C. (Cost up to 34 bytes)

#define DISP_TERSE			0	// Optionally used with 20x4 and 40x2 displays to display:
								// "                    "     (uses approximately 22 bytes)
								// "            Mobo 4.3"                                  *OR*       
#define DISP_VERBOSE		0	// Optionally used with 20x4 and 40x2 displays to display:
								// "Softrock 6.3 &      "     (uses approximately 52 bytes or 30 more than above)
								// "            Mobo 4.3"                                  *OR* 
#define DISP_VER_SIMPLE		0	// Optionally used with 20x4 and 40x2 displays to display version information
								// "                    "     (uses approximately 94 bytes)
								// "Mobo 4.3        1.05"
								// (uses most bytes, or about 120 in total)
#define DISP_VER_SHORT		0	// Optionally used with 20x4 and 40x2 displays to display version information
								// "                    "     (uses approximately 94 bytes)
								// "Mobo 4.3  1.05-16.12"
								// (uses most bytes, or about 120 in total)
#define DISP_VERSION		0	// Optionally used with 20x4 and 40x2 displays to display version information
								// "SR 6.3 <--> Mobo 4.3"     (wasteful, uses approximately 120 bytes)
								// "     Ver: 1.03-16.11"
								// (uses most bytes, or about 120 in total)


//-----------------------------------------------------------------------------
// Bargraph options.
//
#define EXTERNAL_BARGRAPH	1	// Add option in Cmd 0x66 to adjust the Fullscale value for the Power bargraph
								// This option is also autoenabled if LCD_DISPLAY_AAxB_b options are selected
								// If no LCD, then used to calibrate displayed range used by external software
								// such as the Control_Mobo_GUI. (Cost upto 62 bytes)
#define	BARGRAPH_SWR_SCALE	1	// Add option in Cmd 0x66 to adjust the Fullscale value for the SWR bargraph
								// Normally enabled. (Cost 50 bytes)

#define BARGRAPH_STYLE_1	1	// Used if LCD bargraph alternatives.  N8LP LP-100 look alike bargraph         *OR*
#define BARGRAPH_STYLE_2	0	// Used if LCD bargraph alternatives.  Bargraph with level indicators          *OR*
#define BARGRAPH_STYLE_3	0	// Used if LCD bargraph alternatives.  Another bargraph with level indicators  *OR*
#define BARGRAPH_STYLE_4	0	// Used if LCD bargraph alternatives.  Original bargraph, Empty space enframed *OR*
#define BARGRAPH_STYLE_5	0	// Used if LCD bargraph alternatives.  True bargraph, Empty space is empty

//-----------------------------------------------------------------------------
// Rotary Encoder Functions
//
// Either one of the two below to be used if Rotary Encoder VFO is used
#define ENCODER_SCAN_STYLE	1	// Scanning type Rotary Encoder routine, regularly scans the GPIO inputs
								// Gives full resolution of rotary encoder (Cost 644 bytes)                    *OR*
#define ENCODER_INT_STYLE	0	// Interrupt driven Rotary Encoder VFO, uses one interrupt, gives
								// only half the resolution of the encoder (every other click inactive)
								//(Cost 652 bytes)

// It makes little sense to have both of the two below enabled at the same time.
#define ENCODER_CMD_INCR	0	// USB Cmd 0x36. Modify Encoder Resolution, 32 bit signed integer.
								// (cost up to 122 bytes)								
#define ENCODER_RESOLUTION	1	// USB Cmd 0x67. Set the Encoder Resolvable States per Revolution
								// for 1kHz tune per Rev (cost up to 170 bytes)
								
#define	ENCODER_DIR_REVERSE	0	// Reverse the Encoder Direction (Cost 0, simply reverses direction)
#define ENCODER_FAST_ENABLE	1	// Variable speed Rotary Encoder feature (Cost 206 bytes)
#define ENCODER_DIR_SENSE	1	// Direction change sense.  Used with variable speed feature.
								// Direction change drops immediately out of fast speed mode
								// (cost 42 bytes)

//-----------------------------------------------------------------------------
// Debug stuff, do not use
//
#define DEBUG_1LN			0	// Turns off regular display in first line for Debug
#define DEBUG_2LN			0	// Turns off second line of LCD for Debug
#define DEBUG_CMD_1LN		0	// Display Commands in 1st line, use with DEBUG_1LN
#define DEBUG_CMD_2LN		0	// Display Commands in 2nd line, use with DEBUG_1LN
#define DEBUG_SMTH_OFFS_1LN 0	// Display smoothtune offset in 1st line, use with DEBUG_1LN
#define DEBUG_SMTH_OFFS_2LN 0	// Display smoothtune offset in 1st line, use with DEBUG_2LN
#define TEST_FIXTURE		0	// Different I2C settings used in Test fixture
#define NO_I2C_DURING_RX	0	// I2C noise test, no I2C traffic during RX

#endif//!EXT_SET_FEATURES

//
//-----------------------------------------------------------------------------
// Some conditional includes.  Do not touch these.  They are controlled by the switches above
//-----------------------------------------------------------------------------
//
#if (LCD_PAR_DISPLAY || LCD_PAR_DISPLAY2)	// parallel LCD displays
#include "lcd.h"
#elif LCD_I2C_DISPLAY			// I2C connected LCD displays... incomplete, test code
#include "lcd_i2c.h"
#endif

#if (LCD_PAR_DISPLAY || LCD_PAR_DISPLAY2 || LCD_I2C_DISPLAY)// Identical routines for I2C and parallel connected displays
#if LCD_DISPLAY_16x2_b || LCD_DISPLAY_20x2_b || LCD_DISPLAY_20x4_b || LCD_DISPLAY_40x2_b || LCD_PAR_DISPLAY2
#define BARGRAPH			1	// LCD Bargraph routines
#endif
#elif EXTERNAL_BARGRAPH			// Support bargraph calibration of external software such as Control_Mobo_GUI
#define BARGRAPH			1	// Storage and maintenance of Bargraph parameters in Firmware
#else
#define BARGRAPH			0	// no Bargraph functionality supported
#endif

#if		I2C_08_FILTER_IO
#define	TXF	8
#elif	I2C_16_FILTER_IO
#define	TXF	16
#elif	PCF_FILTER_IO
#define	TXF	8
#elif	M0RZF_FILTER_IO
#define	TXF	4
#else
#define	TXF	8
#endif


//
//-----------------------------------------------------------------------------
// Implementation dependent definitions (user tweak stuff)
//-----------------------------------------------------------------------------
//


// DEFS for PA BIAS selection and autocalibration
#define	BIAS_SELECT			1		// Which bias, 0 = Cal, 1 = AB, 2 = A
									// If BIAS_SELECT is set at 0, then the first operation
									// after any Reset will be to autocalibrate
#define	BIAS_LO				2		// PA Bias in 10 * mA, Class B ( 2 =  20mA)
#define	BIAS_HI				35		// PA Bias in 10 * mA, Class A  (35 = 350mA)
#define	CAL_LO				0		// PA Bias setting, Class B
#define	CAL_HI				0		// PA Bias setting, Class A



// Defs for Power and SWR measurement (values adjustable through USB command 0x44)
#define SWR_TRIGGER			30				// Max SWR threshold (10 x SWR, 30 = 3.0)
#define SWR_PROTECT_TIMER	200				// Timer loop value in increments of 10ms
#define P_MIN_TRIGGER 		49				// Min P out in mW for SWR trigger
#define V_MIN_TRIGGER		0x20			// Min Vin in 1/4096 Full Scale, for valid SWR measurement
											// (SWR = 1.0 if lower values measured)
#define PWR_CALIBRATE		1000			// Power meter calibration value
#define PEP_MAX_PERIOD		20				// Max Time period for PEP measurement (in 100ms)
#define PEP_PERIOD			10				// Time period for PEP measurement (in 100ms)
											// PEP_PERIOD is used with PWR_PEAK_ENVELOPE func
											// under LCD display.


// These two are only used if BARGRAPH is defined:
#define PWR_FULL_SCALE		4				// Bargraph Power fullscale in Watts
#if	BARGRAPH_SWR_SCALE						// Add option to adjust the Fullscale value for the SWR bargraph
#define SWR_FULL_SCALE		4				// Bargraph SWR fullscale: Max SWR = Value + 1 (4 = 5.0:1)
#else
#define SWR_FULL_SCALE		400				// Bargraph SWR fullscale: Max SWR = Value/100 + 1 (400 = 5.0:1)
#endif



// DEFS for I2C comms
#define BIT_SDA				PC6
#define BIT_SCL 			PC7
#define	I2C_DDR				DDRC
#define	I2C_PIN				PINC
#if 	LCD_I2C_DISPLAY						// 16x2 I2C connected LCD display... incomplete, test code
#define	I2C_KBITRATE		300.0			// The I2C LCD display cannot handle more than 150kb/s
											// may only be able to handle 100 kb/s.
#else
#define	I2C_KBITRATE		800.0			// Rate x 2 (100 = 50kb/s).  Some I2C devices
											// may only be able to handle 100 kb/s.
#endif



//DEFS for CW input, PTT output and LEDS
#define IO_DDR_PTT_CWKEY 	DDRB			// Port used for PTT and CW key inputs
#define IO_PORT_PTT_CWKEY	PORTB
#define IO_PIN_PTT_CWKEY	PINB
#define IO_LED1				(1 << PB2)		// Port pins used for LEDs
#define IO_LED2				(1 << PB3)
#define IO_PTT 				(1 << PB3)		// Port pins used for "OldStyle" PTT (LED)
#define IO_CWKEY1 			(1 << PB0)		// Port pins used for CW input
#define IO_CWKEY2 			(1 << PB1)
#define REG_CWSHORT 		(1 << 5)		// Bits used for CW in reg
#define REG_CWLONG  		(1 << 1)
//#define REG_PTT_1			(1 << 2)		// Used with SDR-Widget, not Mobo - so far
//#define REG_PTT_2			(1 << 3)		// Used with SDR-Widget, not Mobo - so far
//#define REG_PTT_3			(1 << 4)		// Used with SDR-Widget, not Mobo - so far
#define REG_TX_state		(1 << 6)		// Indicate transmit status



// DEFS for multipurpose port usage (PORTD)
// read from and write to PORTD, see Commands 0x01, 0x02, 0x03, 0x04, 0x015, 0x016
#define IO_DDR_MP			DDRD
#define IO_PORT_MP			PORTD
#define IO_PIN_MP			PIND


//DEFS for onbard Parallel TX LPFs, using four consecutive bits on PORTD
#define IO_DDR_LPF			DDRD
#define IO_PORT_LPF			PORTD
#define IO_TX_LPF			PD1				// First bit of the four consecutive bits
											// this bit can be set as PD0, PD1, PD2...
											// If PD0, then PD0, PD1, PD2 and PD3 used

//Defs for onboard PCF8574 chip
// I2C Addresses for this chip can be:
// If NXP 8574P,  then: 0x20, 0x21. 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
// If NXP 8574Ax, then: 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
// Normal address is 0x3f
#if TEST_FIXTURE							// Different I2C settings used in Test fixture
#define PCF_MOBO_I2C_ADDR	0x24			// I2C address for the onboard PCF8574
#else
#define PCF_MOBO_I2C_ADDR	0x3f			// I2C address for the onboard PCF8574
#endif
#define Mobo_PCF_FILTER		(3 << 0)		// First bit of three consecutive for BPF
//#define MoboPCFUndefined1	(1 << 3)		// [Option] 8bit BCD or M0RZF style for LPF switching
//#define MoboPCFUndefined2	(1 << 4)		// [Option] 8bit BCD or M0RZF style for LPF switching
//#define MoboPCFUndefined3	(1 << 5)		// [Option] 8bit BCD or M0RZF style for LPF switching
#define Mobo_PCF_TX2		(1 << 6)		// SWR Protect secondary PTT
#define Mobo_PCF_TX			(1 << 7)		// TX PTT, active low

#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
// Default filter addresses for Band Pass filters
#define Mobo_PCF_FLT0		0x00			// Filter setting is a 3 bit value
#define Mobo_PCF_FLT1		0x01			// Filter order could be scrambled if desired
#define Mobo_PCF_FLT2		0x02			// Can be read/modified through
#define Mobo_PCF_FLT3		0x03			// USB commands 0x18/19 (see Readme.txt file)
#define Mobo_PCF_FLT4		0x04
#define Mobo_PCF_FLT5		0x05
#define Mobo_PCF_FLT6		0x06
#define Mobo_PCF_FLT7		0x07
#endif


//Defs for two offboard PCF8574A chips in the MegaFilter Mobo
// I2C Addresses for these chips can be:
// If NXP 8574P,  then: 0x20, 0x21. 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
// If NXP 8574Ax, then: 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
// Normal addresses are 0x39 and 0x3a
#if TEST_FIXTURE							// Different I2C settings used in Test fixture
#define PCF_LPF1_I2C_ADDR	0x21			// I2C address for first Megafilter Mobo PCF8574
#define PCF_LPF2_I2C_ADDR	0x22			// I2C address for second Megafilter Mobo PCF8574
#else
#define PCF_LPF1_I2C_ADDR	0x39			// I2C address for first Megafilter Mobo PCF8574
#define PCF_LPF2_I2C_ADDR	0x3a			// I2C address for second Megafilter Mobo PCF8574
#endif

#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
// Default filter addresses for Band Pass filters
#define I2C_EXTERN_FLT0		0x00			// Filter setting is a 4bit value
#define I2C_EXTERN_FLT1		0x01			// Filter order could be scrambled if desired
#define I2C_EXTERN_FLT2		0x02			// Can be read/modified through
#define I2C_EXTERN_FLT3		0x03			// USB commands 0x1a/1b (see Readme.txt file)
#define I2C_EXTERN_FLT4		0x04
#define I2C_EXTERN_FLT5		0x05
#define I2C_EXTERN_FLT6		0x06
#define I2C_EXTERN_FLT7		0x07
#define I2C_EXTERN_FLT8		0x08
#define I2C_EXTERN_FLT9		0x09
#define I2C_EXTERN_FLTa		0x0a
#define I2C_EXTERN_FLTb		0x0b
#define I2C_EXTERN_FLTc		0x0c
#define I2C_EXTERN_FLTd		0x0d
#define I2C_EXTERN_FLTe		0x0e
#define I2C_EXTERN_FLTf		0x0f
#endif


// FAN Control Definitions
//
// DEFS for FAN Control (PORTD)
// read from and write to PORTD, see Commands 0x01, 0x02, 0x03, 0x04, 0x015, 0x016
#define IO_DDR_FC			DDRD
#define IO_PORT_FC			PORTD
#define	IO_FC				(1 << PD0)		// Port pin used for FAN Control
//
//Defs used for a PCF8574 used for control of a cooling FAN, attenuators, etc...
//Normally this will be an external PCF8574, but alternately the on-board PCF (PCF_MOBO_I2C_ADDR)
//can be used to provide the cooling fan control, depending on features selection.
#define PCF_EXT_I2C_ADDR	0x38			// I2C address for an external PCF8574 used for FAN, attenuators etc
#define PCF_EXT_FAN_BIT		0x01			// Bit 0 (of 0-7) used for FAN control (high for ON)
//IF the onboard PCF8574 is used, then use the below bit for FAN control
#define PCF_MOBO_FAN_BIT	0x20			// Bit 5 (of 0-7) used for FAN control (high for ON)

// DEFS for the Si570 chip
// I2C Address for this chip is normally 0x55
#define	SI570_I2C_ADDRESS	( 0x55 )		// Default Si570 I2C address (can change per device)
#define	DEVICE_XTAL			( 0x7248F5C2 )	// 114.285 * _2(24)
#define	DCO_MIN				4850			// min VCO frequency 4850 MHz
#define DCO_MAX				5670			// max VCO frequency 5670 MHz



// DEFS for the TMP100 chip
// I2C Addresses for this chip can be:
// 0x48. 0x4a, 0x4c, 0x0x4e
// Normal address is 0x4e
#if TEST_FIXTURE							// Different I2C settings if Test fixture
#define TMP100_I2C_ADDRESS	0x4a
#else
#define TMP100_I2C_ADDRESS	0x4e
#endif
#define TMP101_I2C_ADDRESS	0x4a			// Used by TMP101 Autosense routine
// default mode is 9 bit resolution, could be set at 12 bit resolution, but no need
//     0 =    0 deg C
// 32767 =  128 deg C
// 32768 = -128 deg C
#define	HI_TMP_TRIGGER		55				// If measured PA temperature goes above this point
											// then disable transmission. Value is in deg C
											// even if the LCD is set to display temp in deg F

// DEFS for the AD5301 DAC chip
// I2C Addresses for this chip can be:
// 0x0c or 0x0d
// Normal address is 0x0d
#define AD5301_I2C_ADDRESS	0x0d



// DEFS for the AD7991 4 x ADC chip
// This  chip comes in two versions
// AD7991-500 has I2C address 0x28
// AD7991-1500 has I2C address 0x29
// Normal address 0x28
#if TEST_FIXTURE							// Different I2C settings if Test fixture
#define AD7991_I2C_ADDRESS	0x29
#else
#define AD7991_I2C_ADDRESS	0x28
#endif
#define AD7991_PA_CURRENT	0
#define AD7991_POWER_OUT	1
#define AD7991_POWER_REF	2
#define AD7991_PSU_VOLTAGE	3



//DEFS for LCD Display
// The below #defines have been moved from the lcd.h file
#define LCD_PORT			PORTB		// port for the LCD lines
#define LCD_DATA0_PORT		LCD_PORT	// port for 4bit data bit 0
#define LCD_DATA1_PORT		LCD_PORT	// port for 4bit data bit 1
#define LCD_DATA2_PORT		LCD_PORT	// port for 4bit data bit 2
#define LCD_DATA3_PORT		LCD_PORT	// port for 4bit data bit 3
#define LCD_DATA0_PIN		4			// pin  for 4bit data bit 0
#define LCD_DATA1_PIN		5			// pin  for 4bit data bit 1
#define LCD_DATA2_PIN		6			// pin  for 4bit data bit 2
#define LCD_DATA3_PIN		7			// pin  for 4bit data bit 3
#define LCD_RS_PORT			PORTC		// port for RS line
#define LCD_RS_PIN			2			// pin  for RS line
#define LCD_RW_PORT			PORTC		// port for RW line
#define LCD_RW_PIN			4			// pin  for RW line
#define LCD_E_PORT			PORTC		// port for Enable line
#define LCD_E_PIN			5			// pin  for Enable line

// A couple of #defines moved from lcd.h:
#if LCD_DISPLAY_16x2
#define LCD_LINES			2			// number of visible lines of the display 
#define LCD_DISP_LENGTH		16			// visibles characters per line of the display
#endif
#if LCD_DISPLAY_16x2_b
#define LCD_LINES			2			// number of visible lines of the display 
#define LCD_DISP_LENGTH		16			// visibles characters per line of the display
#endif
#if LCD_DISPLAY_20x2_b
#define LCD_LINES			2			// number of visible lines of the display 
#define LCD_DISP_LENGTH		20			// visibles characters per line of the display
#endif
#if LCD_DISPLAY_20x4_b
#define LCD_LINES			4			// number of visible lines of the display 
#define LCD_DISP_LENGTH		20			// visibles characters per line of the display
#endif
#if LCD_DISPLAY_40x2_b
#define LCD_LINES			2			// number of visible lines of the display 
#define LCD_DISP_LENGTH		40			// visibles characters per line of the display
#endif

// DEFS for I2C connected LCD display (very experimental!!!)
#if 	LCD_I2C_DISPLAY					// 16x2 I2C connected LCD display... incomplete, test code
#define LCD_I2C_ADDRESS		0x18		// Some I2C displays come default with I2C address 0x28,
										// which clashes with the AD7991-0 device.  This address
										// can be changed manually.  However the command may confuse
										// the AD7991 settings, so this should only be done once
#endif



// DEFS for the Shaft Encoder VFO function
#if ENCODER_INT_STYLE						// Interrupt driven Shaft Encoder
// Useable external interrupt pins on the AT90USB162:
// INT0->PD0, INT1->PD1, INT2->PD2, INT3->PD3,
// INT5->PD4, INT6->PD6, INT7->PD7
// Interrupt Configuration Parameters
#define ENC_A_SIGNAL	INT6_vect			// Interrupt signal name: INTx, x = 0-7
#define ENC_A_IREG		EIMSK				// Interrupt register, always EIMSK
#define ENC_A_ICR		EICRB				// Interrupt Config Register
											// INT0-3: EICRA, INT4-7: EICRB
#define ENC_A_INT		(1 << INT6)			// matching INTx bit in EIMSK
#define ENC_A_ISCX0		(1 << ISC60)		// Interrupt Sense Config bit0 (ISCx0)
#define ENC_A_ISCX1		(1 << ISC61)		// Interrupt Sense Config bit1 (ISCx1)
#endif
#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Common for both Interrupt and Scan style Encoder
// Configuration of the two input pins, Phase A and Phase B
// They can be set up to use any pin on two separate input ports
// Phase A connects to the pin tied to the Interrupt above
#define ENC_A_PORT		PORTD				// PhaseA port register
#define ENC_A_DDR		DDRD				// PhaseA port direction register
#define ENC_A_PORTIN	PIND				// PhaseA port input register
#define ENC_A_PIN		(1 << PD6)			// PhaseA port pin
#define ENC_B_PORT		PORTD				// PhaseB port register
#define ENC_B_DDR		DDRD				// PhaseB port direction register
#define ENC_B_PORTIN	PIND				// PhaseB port input register
#define ENC_B_PIN		(1 << PD5)			// PhaseB port pin

// Definitions for the Pushbutton Encoder functionality
#define ENC_PUSHB_PORT		PORTD
#define ENC_PUSHB_DDR		DDRD
#define	ENC_PUSHB_INPORT	PIND
#define	ENC_PUSHB_PIN		(1 << PD7)
#define ENC_PULSES			1024			// Number of resolvable Encoder States per revolution.
											// Note that the pulses per revolution parameter is not consistent
											// for all encoders.  For some Rotary Encoders "pulses per revolution"
											// indicates the number of pulses per revolution for each of the two
											// phase outputs, while for others, this parameter indicates the total
											// number of resolvable states.  In case of the former, the total number
											// of resolvable states is 4 times the specified pulses per revolution.
#define	ENC_INCREMENTS		1000/(.11920929 * ENC_PULSES) // One kHz per revolution of Encoder
											// Frequency increments in units of
											// ~0.11920929 Hz (or 8 ~ 1 Hz)
#define ENC_PUSHB_MIN		1				// Min pushdown for valid push (x 10ms)
#define	ENC_PUSHB_MAX		10				// Min pushdown for memory save (x 10ms)
#define ENC_STORE_DISP		20				// Time to display "Memory Stored" on LCD (x 100ms)
#endif

#if ENCODER_FAST_ENABLE						// Variable speed Rotary Encoder feature
// Definitions for Variable Speed Rotary Encoder function
// The below parameters are hopefully more or less generic for any encoder, down to 32 pulses or so,
// but are optimized for a 1024 state Rotary Encoder
#define ENC_FAST_SENSE		96000/ENC_PULSES// Maximum time threshold (in steps of 1/65536 s) per click to enable fast mode
#define ENC_FAST_TRIG		ENC_PULSES/5	// Number of fast clicks to enable fast Mode
											// Make sure this does not exceed uint8_t
#define ENC_FAST_MULTIPLY	100				// Encoder click multiplier during FAST mode (max 128)
#define ENC_FAST_PATIENCE	5				// Time in 1/10th of seconds.  Time to revert to
											// normal encoder mode if no encoder activity
#endif



//
//-----------------------------------------------------------------------------
// Miscellaneous software defines, functions and variables
//-----------------------------------------------------------------------------
//



// DEFS for all kinds of Flags
extern uint8_t		Status1;

#define TX_FLAG		(1 << 0)				// Indicates status of TX/RX = TX
#define REBOOT		(1 << 1)				// Hang and reboot by watchdog
#define SWR_ALARM	(1 << 2)				// SWR alarm condition
#define	TMP_ALARM	(1 << 3)				// Temperature alarm condition
#define PA_CAL_LO	(1 << 4)				// Used by PA Bias auto adjust routine
#define PA_CAL_HI	(1 << 5)				// Used by PA Bias auto adjust routine
#define	PA_CAL		(1 << 6)				// Indicates PA Bias auto adjust in progress
#define I2C_BUSY	(1 << 7)				// Indicates that I2C comms are busy (collision protect)

extern uint8_t		Status2;
// DEFS for all kinds of Flags
#define SI570_OFFL	(1 << 0)				// SI570 is offline
#define	ENC_NEWFREQ	(1 << 1)				// Shaft Enc interrupt routine has a result
#define ENC_STORED	(1 << 2)				// Shaft Enc pushbutton status flag "STORED"
#define ENC_CHANGE	(1 << 3)				// Encoder Changed flag (used with variable rate enc)
#define	ENC_FAST	(1 << 4)				// Encoder FAST mode enabled
#define ENC_DIR		(1 << 5)				// Encoder Direction Change
#define ENC_RES		(1 << 6)				// Manual Encoder resolution through cmd 0x36
#define COOLING_FAN	(1 << 7)				// Status of PA Cooling Fan

#define	True		1
#define	False		0

#define	_2(x)		((uint32_t)1<<(x))		// Take power of 2


// Structures and Unions
typedef union {								// Union of unsigned and signed 16 bit
	uint16_t		w;						// and 8bit[2]
	int16_t			i;
	struct {
		uint8_t		b0;
		uint8_t		b1;
	};
	struct {
		int8_t		i0;
		int8_t		i1;
	};
} sint16_t;

typedef union {								// Union of an unsigned 32bit and the 
	uint32_t		dw;						// above defined sint16_t[2]
	struct {
		sint16_t	w0;
		sint16_t	w1;
	};
} sint32_t;

typedef union {								// For Si570 register use
	uint8_t			bData[6];
	uint16_t		wData[3];
	struct {
		uint8_t		N1:5;
		uint8_t		HS_DIV:3;
		uint8_t		RFREQ_b4;				// N1[1:0] RFREQ[37:32]
		sint32_t	RFREQ;					// RFREQ[31:0]
	};
} Si570_t;

typedef struct 
{
		uint8_t		EEPROM_init_check;		// If value mismatch,
											// then update EEPROM to factory settings
		uint8_t		Si570_I2C_addr;			// Si570 I2C addres, default 0x55 (85 dec)
		uint8_t		TMP100_I2C_addr;		// I2C address for the onboard TMP100 temperature sensor
		uint8_t		AD5301_I2C_addr;		// I2C address for the onboard AD5301 8 bit DAC
		uint8_t		AD7991_I2C_addr;		// I2C address for the onboard AD7991 4 x ADC
		uint8_t		PCF_I2C_Mobo_addr;		// I2C address for the onboard PCF8574
		uint8_t		PCF_I2C_lpf1_addr;		// I2C address for the first PCF8574 used in the MegaFilterMobo
		uint8_t		PCF_I2C_lpf2_addr;		// I2C address for the second PCF8574 used in the MegaFilterMobo
		uint8_t		PCF_I2C_Ext_addr;		// I2C address for an external PCF8574 used for FAN, attenuators etc
		uint8_t		hi_tmp_trigger;			// If PA temperature goes above this point, then
											// disable transmission
		uint16_t	P_Min_Trigger;			// Min P out measurement for SWR trigger
		uint16_t	SWR_Protect_Timer;		// Timer loop value
		uint16_t	SWR_Trigger;			// Max SWR threshold
		uint16_t	PWR_Calibrate;			// Power meter calibration value
		uint8_t		Bias_Select;			// Which bias, 0 = Cal, 1 = LO, 2 = HI
		uint8_t		Bias_LO;				// PA Bias in 10 * mA, typically  20mA or Class B
		uint8_t		Bias_HI;				// PA Bias in 10 * mA, typically 350mA or Class A
		uint8_t		cal_LO;					// PA Bias setting, LO
		uint8_t		cal_HI;					// PA Bias setting, HI
		uint32_t	FreqXtal;				// crystal frequency[MHz] (8.24bits)
		uint16_t	SmoothTunePPM;			// Max PPM value for the smooth tune
		uint32_t	Freq[10];				// Running frequency[MHz] (11.21bits)
											// The first one is the running frequency
											// the next nine are memory stores, used with
											// the Shaft Encoder function, one for each
											// band, 1.8, 3.5, 7,... 28 MHz
		uint8_t		SwitchFreq;				// Which freq is in use (used with ShaftEncoder)
		sint16_t	FilterCrossOver[8];		// 8x Cross Over points for Band Pass Filter (11.5bits)
		#if I2C_08_FILTER_IO				// TX filter controls using 1x external PCF8574 over I2C
		sint16_t	TXFilterCrossOver[8];	// 8x Cross Over points for TX Low Pass Filter
		#elif I2C_16_FILTER_IO				// TX filter controls using 2x external PCF8574 over I2C
		sint16_t	TXFilterCrossOver[16];	// 16x Cross Over points for TX Low Pass Filter
		#elif PCF_FILTER_IO					// 8x BCD control for LPF switching, switches P1 pins 4-6
		sint16_t	TXFilterCrossOver[8];	// 8x Cross Over points for TX Low Pass Filter
		#elif M0RZF_FILTER_IO					// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
		sint16_t	TXFilterCrossOver[4];	// 4x Cross Over points for TX Low Pass Filter
		#else
		sint16_t	TXFilterCrossOver[8];	// 8x Cross Over points for TX Low Pass Filter
		#endif
		#if BARGRAPH
		uint8_t		PWR_fullscale;			// Full Scale setting for Power Output Bargraph
		#if	BARGRAPH_SWR_SCALE				// Add option to adjust the Fullscale value for the SWR bargraph
		uint8_t		SWR_fullscale;			// Full Scale setting for SWR Bargraph
		#endif
		#endif
		#if	PWR_PEP_ADJUST					// Add option to adjust the number of samples in PEP measurement
		uint8_t		PEP_samples;			// Number of samples in PEP measurement
		#endif
		#if CALC_FREQ_MUL_ADD				// Frequency Subtract and Multiply Routines
		uint32_t	FreqSub;				// Freq subtract value[MHz] (11.21bits)
		uint32_t	FreqMul;				// Freq multiply value (11.21bits)
		#endif
		#if ENCODER_CMD_INCR				// USB Command to modify Encoder Resolution
		int32_t		Encoder_Resolution;		// Frequency increments in units of ~0.12 Hz (or 8 ~ 1 Hz)
		#endif
		#if	ENCODER_RESOLUTION				// USB command to modify the Ecoder Resolution
		uint16_t	Resolvable_States;		// Number of Resolvable States per Revolution
		#endif
		#if PSDR_IQ_OFFSET36				// Display a fixed frequency offset during RX only.
		int32_t		LCD_RX_Offset;			// Freq add/subtract value is 0.0MHz (11.21bits)
											// signed integer, 0.000 MHz * 4.0 * _2(21)
		#elif PSDR_IQ_OFFSET68				// Display a fixed frequency offset during RX only.
		int8_t		LCD_RX_Offset;			// Freq add/subtract value is in kHz
		#endif
		#if	FAN_CONTROL						// Turn PA Cooling FAN On/Off, based on temperature
		uint8_t		Fan_On;					// Fan On trigger temp
		uint8_t		Fan_Off;				// Fan Off trigger temp
		uint8_t		PCF_fan_bit;			// Which bit is used to control the Cooling Fan
		#endif
		#if SCRAMBLED_FILTERS				// Enable a non contiguous order of filters
		uint8_t		FilterNumber[8];		// Which Band Pass filter to select at each crossover
		uint8_t		TXFilterNumber[16];		// Which TX Low Pass filter to select at each crossover
		#endif
		//#if USB_SERIAL_ID					// A feature to change the last char of the USB Serial  number
		//uint8_t	SerialNumber;			// Last char (X) of the USB device Serial Number "TF3LJ-1.X"
		//#endif
		#if CALC_BAND_MUL_ADD				// Band dependent Frequency Subtract and Multiply
		uint32_t	BandSub[8];				// Freq Subtract values [MHz] (11.21bits) for each of
											// the 8 (BPF) Bands
		uint32_t	BandMul[8];				// Freq Multiply values [MHz] (11.21bits) for each of
											// the 8 (BPF) Bands
		#endif
} var_t;


typedef struct								// USB command structure - from the LUFA library
{
	uint8_t  NotificationType; 				//< Notification type, a mask of REQDIR_*, REQTYPE_* and REQREC_* constants
    	                        			//   from the library StdRequestType.h header
        	                    			//
	uint8_t  bRequest; 						//< Notification value, a NOTIF_* constant
	sint16_t wValue; 						//< Notification wValue, notification-specific
	sint16_t wIndex; 						//< Notification wIndex, notification-specific
	uint16_t wLength; 						//< Notification wLength, notification-specific
} USB_Notification_Header_t;


// Various global variables
extern	EEMEM 		var_t E;				// Default Variables in EEPROM
extern				var_t R;				// Runtime Variables in Ram

extern	sint16_t	replyBuf[];				// USB Reply buffer (Command 0x3f)
extern	sint16_t	tmp100_data;			// Last measured value read from the TMP100 temperature
											// sensor
extern	sint16_t	ad7991_adc[];			// Last measured values read from the AD7991 ADC
											// adjusted for a full scale 16 bit unsigned int
extern	uint16_t	measured_SWR;			// SWR value x 100, in unsigned int format

extern	uint8_t		biasInit;				// Power Amplifier Bias initiate flag
											// (0 = uninitiated, 1 = class AB, 2 = class A)

extern	uint8_t		selectedFilters[];		// Contains info on which filters are selected, for LCD print

//extern 	PROGMEM		USB_Descriptor_String_t	SerialNumberString;// Contains the USB Serial number
											// This would be used with the USB_SERIAL_ID #define

extern	uint8_t 	pcf_data_out;			// Data to onboard PCF8574 register
											// This variable used for the builtin PCF8574 on the Mobo 4.3

// prototypes for Mobo_ABPF.c
#if CALC_BAND_MUL_ADD						// Band dependent Frequency Subtract and Multiply
extern	uint8_t		SetFilter(uint32_t);
#else
extern	void		SetFilter(uint32_t);
#endif


// prototypes for Mobo_I2C_Peripherals.c
//extern	void		i2c_queue(void);				// Rudimentary I2C Queueing
//extern	void		i2c_release(void);				// Release I2C port
extern	void		MoboPCF_set(uint8_t byte);		// Set a bit/bits in builtin Mobo PCF8574 output
extern	void		MoboPCF_clear(uint8_t byte);	// Clear a bit/bits in builtin Mobo PCF8574 output
extern	void		pcf8574_byte(uint8_t, uint8_t);	// Write I2C, a full byte
extern	uint8_t		pcf8574_read(uint8_t);			// Read I2C data from PCF8574 Remote 8bit I/O expander
extern	void		tmp100(uint8_t);				// Read temperature from TMP100 device
extern	void		ad5301(uint8_t, uint8_t);		// Write data to the AD5301 DAC
extern	void		ad7991_setup(uint8_t);			// Setup AD7991 to do interesting stuff
extern	void		ad7991_poll(uint8_t);			// Poll the AD7991 4 x ADC chip


// prototypes for Mobo_PWR_SWR_and_Bias_cal.c
extern void			Test_SWR(void);					// Do SWR calcultions and control the PTT2 output


extern uint16_t		measured_Power(uint16_t);		// Convert AD reading into "Measured Power in milliWatts"
extern void			PA_bias(void);					// RD16HHF1 PA Bias management

				
// prototypes for Mobo_LCD_Display.c
extern void			lcd_initialize(void);			// Initialize the LCD
extern void			lcd_display(void);				// Display stuff on a LCD

// prototypes for 20x4, low traffic (low noise) Mobo_LCD_Display.c routines
extern void			lcd_display_TRX_status_on_change(void);	// Display TX/RX transition stuff when change
extern void			lcd_display_TRX_stuff(void);	// Display TX/RX transition stuff
extern void			lcd_display_freq_and_filters(void);// Display frequency and filters
extern void 		lcd_display_P_SWR_V_C_T(void);	// Display non-static measured values
extern void			lcd_display_Memory_Stored(void);// Display Memory Stored for a certain amount of time



// prototypes for Mobo_LCD_bargraph_lowlevel.c
extern void			lcdProgressBar(uint16_t, uint16_t, uint8_t);		// Draw a bargraph on LCD
extern void			lcd_bargraph_init(void);		// Load the custom bargraph charaters to LCD


// prototypes for Mobo_ShaftEncoder.c
extern void 		shaftEncoderInit(void);			// Initialise the VFO Shaft Encoder
													// Separate functions swapped in for the Interrupt
													// driven Encoder or the Scan driven Encoder
#if ENCODER_SCAN_STYLE								// Shaft Encoder which scans the GPIO inputs
extern void			encoder_scan(void);				// Scan the Shaft Encoder
#endif


// prototyes for DeviceSi570.c
extern	uint8_t		GetRegFromSi570(void);
extern	void		SetFreq(uint32_t freq);
extern	void		DeviceInit(void);
extern	void		Si570CmdReg(uint8_t reg, uint8_t data);


// prototypes for I2Copencollector.c
extern /*register*/ uint8_t	I2CErrors /*asm("r8")*/;	// Saves 18 bytes to change this from Reg int
extern	void		I2CSendStart(void);
extern	void		I2CSendStop(void);
extern	void		I2CSendByte(uint8_t b);
extern	void 		I2CSend0(void);
extern	void 		I2CSend1(void);
extern	uint8_t		I2CReceiveByte(void);
extern	void		I2CStretch(void);


// prototypes for CalcVFO.c
#if CALC_BAND_MUL_ADD								// Band dependent Frequency Subtract and Multiply
extern uint32_t	CalcFreqMulAdd(uint32_t, uint32_t, uint32_t);
#else
extern uint32_t	CalcFreqMulAdd(uint32_t);
#endif


// This function, in USB-EP0.c, when called, starts the USB works
extern void Initialize_USB(void);


// The three all important functions in Mobo, hooks from the USB works
void usbFunctionWrite(USB_Notification_Header_t *, unsigned char *, unsigned char);
unsigned char usbFunctionSetup(USB_Notification_Header_t *);
void maintask(void);

#endif

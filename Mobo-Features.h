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
//**                   15) 20x4 LCD display with analog-style bargraphs for enhanced power output and 
//**                       SWR visualisation
//**                   16) alternative 40x2 LCD display alternative with analog-style bargraphs for enhanced
//**                       power output and SWR visualisation (same as 20x4, different layout)
//**                   17) optional 16x2 and 20x2 LCD display alternatives with analog-style bargraphs for
//**                       power output and SWR visualisation.
//**                   18) an alternative 16x2 LCD display without the bargraphs, providing Frequency
//**                       readout, Power/SWR metering and status diplay (RX/TX/hi-SWR/hi-TMP/Mem-store...)
//**                   19) LCD frequency display offset option, for readout of the actual transmit/receive
//**                       frequency, when using PowerSDR-IQ
//**                   20) Read/Write a byte from/to external I2C connected General Purpose IO extenders
//**                       PCF8574 (filters, attenuators, amplifiers)
//**
//**                   Note that some of the items described above, such as the Rotary
//**                   Encoder VFO, the PTT2 output, the Power/SWR bridge and the LCD,
//**                   while not necessary, are optional features provided by the
//**				   firmware
//**
//** Initial version.: 2009-09-09, Loftur Jonasson, TF3LJ
//**
//**
//** History.........: Check the Mobo.c file
//**                   
//**
//*********************************************************************************

//-----------------------------------------------------------------------------
//
// This file is used to generate several different standard compile time options,
// and it is nothing more than a copy/paste of the compile time options menu in
// Mobo.h, copied several times over, each copy containing the settings used to
// prepare a specific HEX file.  The relevant options list for each HEX file is
// selected by selecting one of the twelve #defines below.
// 
// See description of precomipled HEX files, contained in the Readme.txt file
//
//-----------------------------------------------------------------------------

#define Mobo_01_BasicFunction							0
#define	Mobo_02_PWRSWR_SWRalarm							0
#define	Mobo_03_20x4_LCD_PWRSWR_SWRalarm_VFO    		0
#define	Mobo_04_20x4_LCD_PWRSWR_SWRalarm_VFO_F			1
#define Mobo_05_20x4_LCD_PWRSWR_SWRalarm_VFO_P1LPF    	0
#define Mobo_06_20x4_LCD_PWRSWR_SWRalarm_VFO_P1LPF_F	0
#define Mobo_07_20x4_LCD_PWRSWR_SWRalarm_VFO_P1M0RZF    0
#define Mobo_08_20x4_LCD_PWRSWR_SWRalarm_VFO_P1M0RZF_F	0
#define Mobo_09_40x2_LCD_PWRSWR_SWRalarm_VFO    		0
#define Mobo_10_40x2_LCD_PWRSWR_SWRalarm_VFO_F			0


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_01_BasicFunction
	
//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x21// When started, the firmware examines this "Serial Number
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
#define	POWER_SWR			0	// Measure, and if LCD is enabled, then display Power and SWR. 
								// If this is not defined, while 16x2 non-bargraph LCD is defined,
								// then LCD displays Vdd and I-Pa (Cost 200 or 412 bytes)
#define SWR_ALARM_FUNC		0	// SWR alarm function, activates a secondary PTT
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
#define PSDR_IQ_OFFSET68	0	// Display a fixed frequency offset during RX only.  This can be used
								// to always display the actual used frequency, when using PowerSDR-IQ
								// (Cost appr 78 bytes)

#define	FAN_CONTROL			1	// Turn PA Cooling FAN On/Off, based on temperature
// Only one of the three below is selected with the FAN Control
#define	PORTD_FAN			0	// Port D Pin for FAN Control (Normally PD0, but definable below) 
								// This command is not compatible with DDRD_TX_FILTER_IO
								// as it uses one of the same Port D output bits
								// (Cost appr 146 bytes)
#define	BUILTIN_PCF_FAN		1	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		0	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	0	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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
#define LCD_DISPLAY_20x4_b	0	// Alternate 20x4 LCD display with bargraph.     *OR*
								// (costs 102 bytes in addition to the 16x2 bargraph display)
#define LCD_DISPLAY_40x2_b	0	// Alternate 40x2 LCD display.
								// (costs 86 bytes in addition to the 16x2 bargraph display)

#define CHANGE_I2C_LCD_ADDR	0	// Only used once with a fresh I2C LCD display which has the default address 
								// 0x28 (0x50).This changes the I2C address of the LCD to the address defined
								// further down in this file.  Use only once and then disable, conflicts with
								// the AD7991-0 address

#define PWR_PEAK_ENVELOPE	0	// PEP measurement for LCD display. Highest value in buffer shown (Cost 64 bytes)

#define	PWR_PEP_ADJUST		0	// Option to adjust the number of samples in PEP measurement through USB Cmd 0x66

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

#define DISP_FAHRENHEIT		0	// Used with LCD display, Display temperature in Fahrenheit
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

#define BARGRAPH_STYLE_1	0	// Used if LCD bargraph alternatives.  N8LP LP-100 look alike bargraph         *OR*
#define BARGRAPH_STYLE_2	0	// Used if LCD bargraph alternatives.  Bargraph with level indicators          *OR*
#define BARGRAPH_STYLE_3	0	// Used if LCD bargraph alternatives.  Another bargraph with level indicators  *OR*
#define BARGRAPH_STYLE_4	0	// Used if LCD bargraph alternatives.  Original bargraph, Empty space enframed *OR*
#define BARGRAPH_STYLE_5	0	// Used if LCD bargraph alternatives.  True bargraph, Empty space is empty

//-----------------------------------------------------------------------------
// Rotary Encoder Functions
//
// Either one of the two below to be used if Rotary Encoder VFO is used
#define ENCODER_SCAN_STYLE	0	// Scanning type Rotary Encoder routine, regularly scans the GPIO inputs
								// Gives full resolution of rotary encoder (Cost 644 bytes)                    *OR*
#define ENCODER_INT_STYLE	0	// Interrupt driven Rotary Encoder VFO, uses one interrupt, gives
								// only half the resolution of the encoder (every other click inactive)
								//(Cost 652 bytes)

// It makes little sense to have both of the two below enabled at the same time.
#define ENCODER_CMD_INCR	0	// USB Cmd 0x36. Modify Encoder Resolution, 32 bit signed integer.
								// (cost up to 122 bytes)								
#define ENCODER_RESOLUTION	0	// USB Cmd 0x67. Set the Encoder Resolvable States per Revolution
								// for 1kHz tune per Rev (cost up to 170 bytes)
								
#define	ENCODER_DIR_REVERSE	0	// Reverse the Encoder Direction (Cost 0, simply reverses direction)
#define ENCODER_FAST_ENABLE	0	// Variable speed Rotary Encoder feature (Cost 206 bytes)
#define ENCODER_DIR_SENSE	0	// Direction change sense.  Used with variable speed feature.
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if	Mobo_02_PWRSWR_SWRalarm							

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x22// When started, the firmware examines this "Serial Number
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
#define PSDR_IQ_OFFSET68	0	// Display a fixed frequency offset during RX only.  This can be used
								// to always display the actual used frequency, when using PowerSDR-IQ
								// (Cost appr 78 bytes)

#define	FAN_CONTROL			1	// Turn PA Cooling FAN On/Off, based on temperature
// Only one of the three below is selected with the FAN Control
#define	PORTD_FAN			0	// Port D Pin for FAN Control (Normally PD0, but definable below) 
								// This command is not compatible with DDRD_TX_FILTER_IO
								// as it uses one of the same Port D output bits
								// (Cost appr 146 bytes)
#define	BUILTIN_PCF_FAN		1	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		0	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	0	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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
#define LCD_DISPLAY_20x4_b	0	// Alternate 20x4 LCD display with bargraph.     *OR*
								// (costs 102 bytes in addition to the 16x2 bargraph display)
#define LCD_DISPLAY_40x2_b	0	// Alternate 40x2 LCD display.
								// (costs 86 bytes in addition to the 16x2 bargraph display)

#define CHANGE_I2C_LCD_ADDR	0	// Only used once with a fresh I2C LCD display which has the default address 
								// 0x28 (0x50).This changes the I2C address of the LCD to the address defined
								// further down in this file.  Use only once and then disable, conflicts with
								// the AD7991-0 address

#define PWR_PEAK_ENVELOPE	0	// PEP measurement for LCD display. Highest value in buffer shown (Cost 64 bytes)

#define	PWR_PEP_ADJUST		0	// Option to adjust the number of samples in PEP measurement through USB Cmd 0x66

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

#define DISP_FAHRENHEIT		0	// Used with LCD display, Display temperature in Fahrenheit
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

#define BARGRAPH_STYLE_1	0	// Used if LCD bargraph alternatives.  N8LP LP-100 look alike bargraph         *OR*
#define BARGRAPH_STYLE_2	0	// Used if LCD bargraph alternatives.  Bargraph with level indicators          *OR*
#define BARGRAPH_STYLE_3	0	// Used if LCD bargraph alternatives.  Another bargraph with level indicators  *OR*
#define BARGRAPH_STYLE_4	0	// Used if LCD bargraph alternatives.  Original bargraph, Empty space enframed *OR*
#define BARGRAPH_STYLE_5	0	// Used if LCD bargraph alternatives.  True bargraph, Empty space is empty

//-----------------------------------------------------------------------------
// Rotary Encoder Functions
//
// Either one of the two below to be used if Rotary Encoder VFO is used
#define ENCODER_SCAN_STYLE	0	// Scanning type Rotary Encoder routine, regularly scans the GPIO inputs
								// Gives full resolution of rotary encoder (Cost 644 bytes)                    *OR*
#define ENCODER_INT_STYLE	0	// Interrupt driven Rotary Encoder VFO, uses one interrupt, gives
								// only half the resolution of the encoder (every other click inactive)
								//(Cost 652 bytes)

// It makes little sense to have both of the two below enabled at the same time.
#define ENCODER_CMD_INCR	0	// USB Cmd 0x36. Modify Encoder Resolution, 32 bit signed integer.
								// (cost up to 122 bytes)								
#define ENCODER_RESOLUTION	0	// USB Cmd 0x67. Set the Encoder Resolvable States per Revolution
								// for 1kHz tune per Rev (cost up to 170 bytes)
								
#define	ENCODER_DIR_REVERSE	0	// Reverse the Encoder Direction (Cost 0, simply reverses direction)
#define ENCODER_FAST_ENABLE	0	// Variable speed Rotary Encoder feature (Cost 206 bytes)
#define ENCODER_DIR_SENSE	0	// Direction change sense.  Used with variable speed feature.
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if	Mobo_03_20x4_LCD_PWRSWR_SWRalarm_VFO    		

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x23// When started, the firmware examines this "Serial Number
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
#define	BUILTIN_PCF_FAN		1	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		0	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

#define DISP_FAHRENHEIT		0	// Used with LCD display, Display temperature in Fahrenheit
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
#define DISP_VER_SHORT		1	// Optionally used with 20x4 and 40x2 displays to display version information
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if	Mobo_04_20x4_LCD_PWRSWR_SWRalarm_VFO_F			

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x23// When started, the firmware examines this "Serial Number
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
#define	BUILTIN_PCF_FAN		1	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		0	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

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
#define DISP_VER_SHORT		1	// Optionally used with 20x4 and 40x2 displays to display version information
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_05_20x4_LCD_PWRSWR_SWRalarm_VFO_P1LPF    	

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x24// When started, the firmware examines this "Serial Number
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
#define I2C_08_FILTER_IO	0	// TX filter controls using 1x external PCF8574 over I2C
								// (Cost 28 bytes)
#define I2C_16_FILTER_IO	0	// TX filter controls using 2x external PCF8574 over I2C        *OR*
								// (Cost 64 bytes [12224-12288])
#define	PCF_FILTER_IO		1	// 8x BCD TX filter control using 3 bits on the Mobo PCF8574 chip
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
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

#define DISP_FAHRENHEIT		0	// Used with LCD display, Display temperature in Fahrenheit
								// threshold still set in deg C. (Cost up to 34 bytes)

#define DISP_TERSE			1	// Optionally used with 20x4 and 40x2 displays to display:
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_06_20x4_LCD_PWRSWR_SWRalarm_VFO_P1LPF_F	

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x24// When started, the firmware examines this "Serial Number
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
#define I2C_08_FILTER_IO	0	// TX filter controls using 1x external PCF8574 over I2C
								// (Cost 28 bytes)
#define I2C_16_FILTER_IO	0	// TX filter controls using 2x external PCF8574 over I2C        *OR*
								// (Cost 64 bytes [12224-12288])
#define	PCF_FILTER_IO		1	// 8x BCD TX filter control using 3 bits on the Mobo PCF8574 chip
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
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_07_20x4_LCD_PWRSWR_SWRalarm_VFO_P1M0RZF    

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x25// When started, the firmware examines this "Serial Number
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
#define I2C_08_FILTER_IO	0	// TX filter controls using 1x external PCF8574 over I2C
								// (Cost 28 bytes)
#define I2C_16_FILTER_IO	0	// TX filter controls using 2x external PCF8574 over I2C        *OR*
								// (Cost 64 bytes [12224-12288])
#define	PCF_FILTER_IO		0	// 8x BCD TX filter control using 3 bits on the Mobo PCF8574 chip
								// switches P1 pins 4-6 (Cost 18 bytes)                          *OR*
#define M0RZF_FILTER_IO		1	// TX filter control using 3 bits on the Mobo PCF8574 chip to
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
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

#define DISP_FAHRENHEIT		0	// Used with LCD display, Display temperature in Fahrenheit
								// threshold still set in deg C. (Cost up to 34 bytes)

#define DISP_TERSE			1	// Optionally used with 20x4 and 40x2 displays to display:
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
#define ENCODER_DIR_SENSE	0	// Direction change sense.  Used with variable speed feature.
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_08_20x4_LCD_PWRSWR_SWRalarm_VFO_P1M0RZF_F	

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x25// When started, the firmware examines this "Serial Number
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
#define I2C_08_FILTER_IO	0	// TX filter controls using 1x external PCF8574 over I2C
								// (Cost 28 bytes)
#define I2C_16_FILTER_IO	0	// TX filter controls using 2x external PCF8574 over I2C        *OR*
								// (Cost 64 bytes [12224-12288])
#define	PCF_FILTER_IO		0	// 8x BCD TX filter control using 3 bits on the Mobo PCF8574 chip
								// switches P1 pins 4-6 (Cost 18 bytes)                          *OR*
#define M0RZF_FILTER_IO		1	// TX filter control using 3 bits on the Mobo PCF8574 chip to
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
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

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
#define ENCODER_DIR_SENSE	0	// Direction change sense.  Used with variable speed feature.
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_09_40x2_LCD_PWRSWR_SWRalarm_VFO    		

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x23// When started, the firmware examines this "Serial Number
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
#define	BUILTIN_PCF_FAN		1	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		0	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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
#define LCD_DISPLAY_20x4_b	0	// Alternate 20x4 LCD display with bargraph.     *OR*
								// (costs 102 bytes in addition to the 16x2 bargraph display)
#define LCD_DISPLAY_40x2_b	1	// Alternate 40x2 LCD display.
								// (costs 86 bytes in addition to the 16x2 bargraph display)

#define CHANGE_I2C_LCD_ADDR	0	// Only used once with a fresh I2C LCD display which has the default address 
								// 0x28 (0x50).This changes the I2C address of the LCD to the address defined
								// further down in this file.  Use only once and then disable, conflicts with
								// the AD7991-0 address

#define PWR_PEAK_ENVELOPE	1	// PEP measurement for LCD display. Highest value in buffer shown (Cost 64 bytes)

#define	PWR_PEP_ADJUST		1	// Option to adjust the number of samples in PEP measurement through USB Cmd 0x66

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

#define DISP_FAHRENHEIT		0	// Used with LCD display, Display temperature in Fahrenheit
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
#define DISP_VER_SHORT		1	// Optionally used with 20x4 and 40x2 displays to display version information
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

#endif


//-------------------------------------------------------------------------------------------
//*******************************************************************************************
//-------------------------------------------------------------------------------------------
#if Mobo_10_40x2_LCD_PWRSWR_SWRalarm_VFO_F			

//-----------------------------------------------------------------------------
// Features Selection
  

// EEPROM settings Serial Number. Increment this number when firmware mods necessitate
// fresh "Factory Default Settings" to be forced into the EEPROM at first boot after
// an upgrade
#define COLDSTART_REF		0x23// When started, the firmware examines this "Serial Number
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
#define	BUILTIN_PCF_FAN		1	// This alternative uses a pin on the builtin PCF8574
								// pin is defineable by Cmd 64 index 3, normally header P1, pin 5
								// (Cost appr 162 bytes)
#define	EXTERN_PCF_FAN		0	// This alternative uses a pin on an external PCF8574
								// pin is defineable by Cmd 64 index 3
								// (Cost appr 250 bytes)

//#define USB_SERIAL_ID		0	// A feature to change the last char of the USB Serial  number
								// not implemented due to insufficient RAM resources
								// USB descriptors are stored in PGM Flash, which can only be updated
								// in pages of 128 bytes... A potential TODO for ATmega32u2

//-----------------------------------------------------------------------------
// LCD Reporting Related Features
//
// Select one of the three below if a LCD display is connected
#define	LCD_PAR_DISPLAY2	1	// 4 bit Parallel (HD44780) connected LCD display, new low traffic
								// (low noise) 20x4 Mobo_LCD_Display.c routines     *OR*
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
#define LCD_DISPLAY_20x4_b	0	// Alternate 20x4 LCD display with bargraph.     *OR*
								// (costs 102 bytes in addition to the 16x2 bargraph display)
#define LCD_DISPLAY_40x2_b	1	// Alternate 40x2 LCD display.
								// (costs 86 bytes in addition to the 16x2 bargraph display)

#define CHANGE_I2C_LCD_ADDR	0	// Only used once with a fresh I2C LCD display which has the default address 
								// 0x28 (0x50).This changes the I2C address of the LCD to the address defined
								// further down in this file.  Use only once and then disable, conflicts with
								// the AD7991-0 address

#define PWR_PEAK_ENVELOPE	1	// PEP measurement for LCD display. Highest value in buffer shown (Cost 64 bytes)

#define	PWR_PEP_ADJUST		1	// Option to adjust the number of samples in PEP measurement through USB Cmd 0x66

#define AUTOSCALE_PWR		0	// Display on LCD sub 1W power in mW.  Only used with non-bargraph LCD (Costs 136 bytes)

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
#define DISP_VER_SHORT		1	// Optionally used with 20x4 and 40x2 displays to display version information
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

#endif

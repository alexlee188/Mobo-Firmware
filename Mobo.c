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
//**
//** History.........: 2009-09-09, Version 1.0 & USB Protocol V. 16.10
//**
//**                   2009-09-20, Version 1.01 & USB Protocol V. 16.10
//**                   (Minor update.  Commands 0x18, 19, 1a & 1b made optional,
//**                    Command 0x67 selected as default, Commands 0x6e/6f selected
//**                    as default, this may be useful during beta tests.
//**                    LPF filter selection available both through PD1-PD4 and by
//**                    MegaFilter Mobo style I2C comms.
//**                    Rough calibration of power meter, diode offset reduced to 100mV.
//**                    PEP indication now samples within a 2 second window.
//**                    A few edits in Readme.txt)
//**
//**                   2009-10-23, Version 1.02 & USB Protocol V. 16.11
//**                   (Command 0x67 replaced with Command 0x36
//**                    PEP measurements are now cleared during RX, previously there was
//**                    residual window of measurement from last TX.
//**                    Command 0x66 expanded with Index 6 for adjustible PEP measurement
//**                    period (1 = not PEP and up to 20 for PEP within a 2 second window),
//**                    default PEP window is set at 5 (0.5 seconds).
//**                    Variable Speed Rotary Encoder VFO function added.
//**                    Optional Direction Sense added to the Variable Speed VFO funtion.
//**                    I2C queueing removed, not necessary.  Band Sub/Mul function bugs
//**                    corrected.  
//**                    Power Bargraph now works for values higher than 15W.
//**                    Some alternate display options added for lines 3/4 when using a 20x4
//**                    display and similar with 40x2.
//**                    Readme file edits.  Some other minor things...)
//**
//**                   2009-12-30, Version 1.03 & USB Protocol V. 16.11
//**                   (Command 0x36, Index 10 added (LCD frequency offset during RX to support
//**                    accurate frequency readout when using PowerSDR-IQ.
//**                    Option for direct control of PCF8574 now disabled by default.
//**					However, it can easily be re-activated.
//**                    Default PEP window set back to 10 (1 second window).
//**                    Added a new #define to reverse the Rotary Encoder VFO direction.
//**                    Code cleanup to free a few hundred bytes (particularly PCF8574 I2C routines).
//**                    Option for TX filter controls on the CPU PortD now disabled.  If the user needs
//**                    direct control signals for LPF swithcing, then it is better to use the new
//**                    PCF_FILTER_IO or M0RZF_FILTER_IO LPF options described below. 
//**                    Added a new #define PCF_FILTER_IO to support switching of up to 8 low pass
//**                    filters through the 3 previously unused bits on the onboard PCF8574 chip.
//**                    Added a new #define M0RZF_FILTER_IO to support low pass filter switching on the
//**                    M0RZF 20W ampt hrough the 3 previously unused bits on the onboard PCF8574 chip.
//**                    Code for Command 0x43 partially in place, to support modification of
//**                    the USB Serial Number descriptor.  However, this is not fully
//**                    implemented, due to lack of pgm space (RAM) resources.
//**                    Rearrangement of EEPROM memory data structure.
//**                    Some edits in Readme.txt
//**
//**                   2010-01-23, Version 1.03-01 & USB Protocol V. 16.11
//**                   (Minor update.  A couple of bug corrections, mainly to do with 
//**                    Add/Subtract and Multiply functions (Cmd 0x31).  This update
//**                    does not affect the precompiled HEX files with version 1.03 (2009-12-30))
//**
//**                   2010-02-28, Version 1.04 & USB Protocol V. 16.11
//**                   (Minor update.  One potential bug correction, to do with SWR measurements
//**                    at very low power.  SWR is now calculated once the Vforward input value
//**                    reaches a value of 32/4096 (appr 36mW).  At lower values, the SWR is forced
//**                    to 1:1)
//**
//**                   2010-04-30, Version 1.05 & USB Protocol V. 16.12
//**                   (Two releases, AT90USB162 release with all essential features and a separate
//**                    ATmega32u2 release, with a number of additional features enabled, such as
//**                    scrambled order of filters, frequency subtract & multiply, legacy commands, etc.
//**                    The AT90USB162 version uses WinAVR20080430 and LUFA090810.
//**                    The ATmega32u2 version uses a different makefile, LUFA091223 and a slightly
//**                    patched WinAVR20100110 (contains only partial support for the ATmega32u2)
//**                    or potentialy an unpatched later version of WinAVR (not tested yet).
//**                    Legacy commands 0x15,0x16,0x20 and 0x40 made Optional.  New command 0x67 added,
//**                    enabling user settable value for the Rotary Encoder Resolution (resolvable
//**                    states per revolution).  This command is used to set the correct value for 1kHz
//**                    per revolution, corresponding dual speed variables are automatically derived.
//**                    New Command 68 added, enabling PSDR-IQ RX frequency display offset.  Replaces
//**                    Cmd 0x36 index 10 and is bidirectional.
//**                    Command 0x36 suppressed (still optional).
//**                    Fan Control setpoints added to Command 0x64 (Hi Tmp trigger for Fan On, default
//**                    45 deg C, Lo Tmp trigger for Fan Off, default 40 deg C, and FAN bit select).
//**                    Three different Fan Control options. PortD Output, builtin PCF8574 output,
//**                    or external PCF8574 output.  Also I2C address for PCF Fan control added to
//**                    Cmd 0x41, index 7.
//**                    TX Filter options tidied up.  Now the preferred option is 8 Filters through an
//**                    external PCF8574 (connected over I2C bus).  All other options still retained.
//**                    Commands 0x6e and 0x6f, User Write/Read to any external PCF8574 has been
//**                    enabled as default.
//**                    Mobo-Features.h contains new automatic features selection macros, to simplify
//**                    generation of standardized precompiled HEX images.)
//**
//**                   2010-05-29, Version 1.06 & USB Protocol V. 16.12
//**                   (Two releases, AT90USB162 release with all essential features and a separate
//**                    ATmega32u2 release, with a number of additional features enabled, such as
//**                    scrambled order of filters, frequency subtract & multiply, legacy commands, etc.
//**                    The AT90USB162 version uses WinAVR20080430 and LUFA090810.
//**                    The ATmega32u2 version uses a different makefile, LUFA091223 and a slightly
//**                    patched WinAVR20100110 (contains only partial support for the ATmega32u2)
//**                    or potentialy an unpatched later version of WinAVR (not tested yet).
//**                    Super slow poll of I2C during RX, once every 10s.
//**                    Rewrite of 20x4 and 40x2 LCD routines for RF noise reduction, no LCD traffic
//**                    unless update to display is needed.  Support for 16x2 and 20x2 LCDs dropped,
//**                    however the old routines are still retained and can be enabled.
//**                    Autosense if TMP101 rather than TMP100.
//**                    TMP alarm function disabled if TMP100 absent, output read as signed integer,
//**                    255 = -1deg C).
//**
//**                   2011-01-15, Version 1.07 & USB Protocol V. 16.13
//**                    (Two separate releases, one for the AT90USB162, one for the ATmega32u2.
//**                     In previous versions, bits 2 and 6 (of 8) in the byte returned by USB commands
//**                     0x50 and 0x51 indicated the state of the two CW/PTT inputs (dit and dash).
//**                     In this version, these commands have been expanded to also indicate the current
//**                     TX/RX state through bit 7 (of 8).  This is useful when using more than one
//**                     piece of software concurrently in a co-ordinated manner, such as sdrshell and
//**                     fldigi, as set up by Alex. In addition, the ATmega32u2 update contains some
//**                     frivolous LCD display gymnastics at the startup, shows whether startup is
//**                     "Init OK"  or whether an "EEPROM Reset" is performed".) 
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

/*
  USB-EP0.c/h and USB-Descriptors.c/h setup the USB EP0 comms hooks which
  spawn our three main functions:

  		usbFunctionWrite(): Process USB Host to Device transmissions.
		usbFunctionSetup():	Process USB query commands and return a result
		maintask():			Continuous loop - Do stuff while not serving USB
*/



// Pin assignment (defined in the Mobo.h and LCD.h files):

// PB0 = CW input 1
// PB1 = CW input 2
// PB2 = Loop Thru LED (watchdog) or USB traffic LED
// PB3 = PTT OUT LED or Loop Thru LED (watchdog)
// PB4 = LCD D4
// PB5 = LCD D5
// PB6 = LCD D6
// PB7 = LCD D7

// PC0 = -0- No PC0 available in normal config of AT90USB162
// PC1 = -0- No PC1 available in normal config of AT90USB162
// PC2 = LCD control RS
// PC3 = -0- No PC3 available in normal config of AT90USB162
// PC4 = LCD control RW
// PC5 = LCD control E
// PC6 = I2C SDA to Si570
// PC7 = I2C SCL to Si570

// If DDRD_TX_FILTER_IO and/or Rotary Encoder are defined:
// (else all of Port D unassigned/controllable through Command 0x15)
// DDRD_TX_FILTER_IO pins:
// PD0 = Unassigned/User bit, controllable through [Optional] Command 0x15
//        alternately [Option] [normally disabled] Cooling Fan control output
// PD1 = [Option] TX LP filter bit 0
// PD2 = [Option] TX LP filter bit 1
// PD3 = [Option] TX LP filter bit 2
// PD4 = [Option] TX LP filter bit 3
// If Rotary Encoder VFO pins:
// (if not, then User bits, controllable through [Optional] Command 0x15)
// PD5 = Rotary Encoder, Phase B
// PD6 = Rotary Encoder, Phase A
// PD7 = Rotary Encoder, Push button

// Mobo PCF8574 chip (normally at I2C addr 63 (0x3f))
// P0 = Onboard BPF addr 0
// P1 = Onboard BPF addr 1
// P2 = Onboard BPF addr 2
// P3 = [Option] addr 0 of PCF_FILTER_IO or M0RZF_FILTER_IO, external LPF switching
// P4 = [Option] addr 1 of PCF_FILTER_IO or M0RZF_FILTER_IO, external LPF switching
// P5 = [Option] addr 2 of PCF_FILTER_IO or M0RZF_FILTER_IO, external LPF switching
//      [alternate use Option, normally active if external LPF control] Cooling Fan control
// P6 = Secondary PTT, high = active (SWR Protect), reversible, see Mobo.h
// P7 = PTT out, high = active

// LPF Mobo PCF8574 chip pair  (normally at I2C addresses 57, 58 (0x39, 0x3a))
// (only one may be enabled, based on configuration)
// P0 = 160m filter
// P1 = 80m filter
// P2 = 40m filter
// P3 = 30m filter
// P4 = 20m filter
// P5 = 18m filter
// P6 = 15m filter
// P7 = 10m filter
// P8 = unassigned
// P9 = unassigned
// P10= unassigned
// P11= unassigned
// P12= unassigned
// P13= unassigned
// P14= unassigned
// P15= unassigned

// EXT F PCF8574  (normally at I2C addr 56 (0x38))
// used for cooling fan control in HEX configs which do not provide the cooling fan control
// through the internal PCF8574
// P0 = user accessible, [Option] Cooling Fan control
// P1 = user accessible/multipurpose
// P2 = user accessible/multipurpose
// P3 = user accessible/multipurpose
// P4 = user accessible/multipurpose
// P5 = user accessible/multipurpose
// P6 = user accessible/multipurpose
// P7 = user accessible/multipurpose {RX pre-amplifier switching}


#include "Mobo.h"
//#include <rprintf.h>							// AVRLIB (not AVRLibc) functions

EEMEM		var_t	E;							// Variables in eeprom (user modifiable, stored)
			var_t	R							// Variables in ram/flash rom (default)
					=
					{
						COLDSTART_REF			// Update into eeprom if value mismatch	
					,	SI570_I2C_ADDRESS		// Si570 I2C address or Si570_I2C_addr
					,	TMP100_I2C_ADDRESS		// I2C address for the TMP100 chip
					,	AD5301_I2C_ADDRESS		// I2C address for the AD5301 DAC chip
					,	AD7991_I2C_ADDRESS		// I2C address for the AD7991 4 x ADC chip
					,	PCF_MOBO_I2C_ADDR		// I2C address for the onboard PCF8574
					,	PCF_LPF1_I2C_ADDR		// I2C address for the first MegaFilterMobo PCF8574
					,	PCF_LPF2_I2C_ADDR		// I2C address for the second MegaFilterMobo PCF8574
					,	PCF_EXT_I2C_ADDR		// I2C address for an external PCF8574 used for FAN, attenuators etc
					,	HI_TMP_TRIGGER			// If PA temperature goes above this point, then
												// disable transmission
					,	P_MIN_TRIGGER			// Min P out measurement for SWR trigger
					,	SWR_PROTECT_TIMER		// Timer loop value (in 10ms increments)
					,	SWR_TRIGGER				// Max SWR threshold (10 x SWR)
					,	PWR_CALIBRATE			// Power meter calibration value
					,	BIAS_SELECT				// Which bias, 0 = Cal, 1 = LO, 2 = HI
					,	BIAS_LO					// PA Bias in 10 * mA, typically  20mA or Class B
					,	BIAS_HI					// PA Bias in 10 * mA, typically 350mA or Class A
					,	CAL_LO					// PA Bias setting, Class LO
					,	CAL_HI					// PA Bias setting, Class HI
					,	DEVICE_XTAL				// FreqXtal
					,	3500					// SmoothTunePPM
					,{	 7.050 * 4.0 * _2(21)	// Default Freq at startup, 
					,	 1.820 * 4.0 * _2(21) 	// Freq Channel 1 
					,	 3.520 * 4.0 * _2(21) 	// Freq Channel 2
					,	 7.020 * 4.0 * _2(21) 	// Freq Channel 3
					,	10.120 * 4.0 * _2(21)	// Freq Channel 4
					,	14.020 * 4.0 * _2(21) 	// Freq Channel 5
					,	18.090 * 4.0 * _2(21) 	// Freq Channel 6
					,	21.020 * 4.0 * _2(21) 	// Freq Channel 7
					,	24.910 * 4.0 * _2(21) 	// Freq Channel 8
					,	28.020 * 4.0 * _2(21)}	// Freq Channel 9
					, 	3						// Which Channel was last in use
					, {	{  2.0 * 4.0 * _2(5) }	// Default filter cross over
					,	{  6.0 * 4.0 * _2(5) }	// frequencies for Mobo V4.3
					,	{  8.0 * 4.0 * _2(5) }	// BPF. eight value array.
					,	{ 12.0 * 4.0 * _2(5) }
					,	{ 16.0 * 4.0 * _2(5) }
					,	{ 20.0 * 4.0 * _2(5) }
					,	{ 24.0 * 4.0 * _2(5) }
					,	{ True } }				
					#if I2C_08_FILTER_IO		// TX filter controls using 1x external PCF8574 over I2C
					, { {  2.0 * 4.0 * _2(5) }	// Default filter crossover
					,	{  4.0 * 4.0 * _2(5) }	// frequencies as per Alex email
					,	{  8.0 * 4.0 * _2(5) }	// 2009-08-15
					,	{ 11.0 * 4.0 * _2(5) }
					,	{ 14.5 * 4.0 * _2(5) }
					,	{ 18.2 * 4.0 * _2(5) }
					,	{ 21.0 * 4.0 * _2(5) }
					,	{ True } }
					#elif I2C_16_FILTER_IO		// TX filter controls using 2x external PCF8574 over I2C
					, { {  2.0 * 4.0 * _2(5) }	// Default filter crossover
					,	{  4.0 * 4.0 * _2(5) }	// frequencies as per Alex email
					,	{  8.0 * 4.0 * _2(5) }	// 2009-08-15
					,	{ 11.0 * 4.0 * _2(5) }
					,	{ 14.5 * 4.0 * _2(5) }
					,	{ 18.2 * 4.0 * _2(5) }
					,	{ 21.0 * 4.0 * _2(5) }
					,	{ 30.0 * 4.0 * _2(5) }
					,	{ 31.0 * 4.0 * _2(5) }
					,	{ 32.0 * 4.0 * _2(5) }
					,	{ 33.0 * 4.0 * _2(5) }
					,	{ 34.0 * 4.0 * _2(5) }
					,	{ 35.0 * 4.0 * _2(5) }
					,	{ 36.0 * 4.0 * _2(5) }
					,	{ 37.0 * 4.0 * _2(5) }
					,	{ True } }
					#elif PCF_FILTER_IO			// 8x BCD control for LPF switching, switches P1 pins 4-6
					, { {  2.0 * 4.0 * _2(5) }	// Default filter crossover
					,	{  4.0 * 4.0 * _2(5) }	// frequencies as per Alex email
					,	{  8.0 * 4.0 * _2(5) }	// 2009-08-15
					,	{ 11.0 * 4.0 * _2(5) }
					,	{ 14.5 * 4.0 * _2(5) }
					,	{ 18.2 * 4.0 * _2(5) }
					,	{ 21.0 * 4.0 * _2(5) }
					,	{ True } }
					#elif M0RZF_FILTER_IO		// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
					, { {  5.0 * 4.0 * _2(5) }	// Default filter crossover
					,	{  9.0 * 4.0 * _2(5) }	// frequencies as per M0RZFR
					,	{ 15.0 * 4.0 * _2(5) }
					,	{ True } }
					#else	
					, { {  2.0 * 4.0 * _2(5) }	// Default filter crossover
					,	{  4.0 * 4.0 * _2(5) }	// frequencies as per Alex email
					,	{  8.0 * 4.0 * _2(5) }	// 2009-08-15
					,	{ 11.0 * 4.0 * _2(5) }
					,	{ 14.5 * 4.0 * _2(5) }
					,	{ 18.2 * 4.0 * _2(5) }
					,	{ 21.0 * 4.0 * _2(5) }
					,	{ True } }
					#endif
					#if BARGRAPH				// IF Bargraph feature is selected
					,	PWR_FULL_SCALE			// Full Scale setting for Power Output Bargraph, in W
					#if	BARGRAPH_SWR_SCALE		// Add option to adjust the Fullscale value for the SWR bargraph
					,	SWR_FULL_SCALE			// Full Scale setting for SWR Bargraph,
												// (Max SWR = Value + 1, or 4 = SWR of 5.0)
					#endif
					#endif
					#if	PWR_PEP_ADJUST			// Add option to adjust the number of samples in PEP measurement
					,	PEP_PERIOD				// Number of samples in PEP measurement
					#endif
					#if CALC_FREQ_MUL_ADD		// Frequency Subtract and Multiply Routines (for Smart VFO)
					,	0.000 * _2(21)			// Freq subtract value is 0.0MHz (11.21bits)
					,	1.000 * _2(21)			// Freq multiply value os 1.0    (11.21bits)
					#endif
					#if ENCODER_CMD_INCR		// USB Command to modify Encoder Resolution
					,	ENC_INCREMENTS			// Frequency increments in units of ~0.12 Hz (or 8 ~ 1 Hz)
					#endif						// signed integer, 0.000 MHz * 4.0 * _2(21)
					#if	ENCODER_RESOLUTION		// USB command to modify the Ecoder Resolution
					,	ENC_PULSES				// Number of Resolvable States per Revolution
					#endif
					#if PSDR_IQ_OFFSET36		// Display a fixed frequency offset during RX only.
					//,	0.009000 * 4.0 * _2(21)	// Freq offset value is 0.009000MHz (11.21bits)
					,	0.000000 * 4.0 * _2(21)	// Freq offset value is 0.000000MHz (11.21bits)
												// signed integer, 0.000 MHz * 4.0 * _2(21)
					#elif PSDR_IQ_OFFSET68		// Display a fixed frequency offset during RX only.
					//,	9						// Freq offset value is a signed value in kHz
					,	0						// Freq offset value is a signed value in kHz
					#endif
					#if	FAN_CONTROL				// Turn PA Cooling FAN On/Off, based on temperature
					,	45						// Fan On trigger temp in degrees C
					,	40						// Fan Off trigger temp in degrees C
					,	PCF_EXT_FAN_BIT			// Which bit is used to control the Cooling Fan
					#endif
					#if SCRAMBLED_FILTERS		// Enable a non contiguous order of filters
					,	{ Mobo_PCF_FLT0			// Band Pass filter selection
					,	  Mobo_PCF_FLT1			// these values are mapped against the result of the
					,	  Mobo_PCF_FLT2			// filter crossover point comparison  
					,	  Mobo_PCF_FLT3			// Filter selected by writing value to output port
					,	  Mobo_PCF_FLT4
					,	  Mobo_PCF_FLT5
					,	  Mobo_PCF_FLT6
					,	  Mobo_PCF_FLT7	}
					,	{ I2C_EXTERN_FLT0		// External LPF filter selection
					,	  I2C_EXTERN_FLT1		// these values are mapped against the result of the
					,	  I2C_EXTERN_FLT2		// filter crossover point comparison 
					,	  I2C_EXTERN_FLT3		// Value is used to set 1 out of 16 bits in a double
					,	  I2C_EXTERN_FLT4		// 8bit port (2x PCF8574 GPIO)
					,	  I2C_EXTERN_FLT5
					,	  I2C_EXTERN_FLT6
					,	  I2C_EXTERN_FLT7
					,	  I2C_EXTERN_FLT8
					,	  I2C_EXTERN_FLT9
					,	  I2C_EXTERN_FLTa
					,	  I2C_EXTERN_FLTb
					,	  I2C_EXTERN_FLTc
					,	  I2C_EXTERN_FLTd
					,	  I2C_EXTERN_FLTe
					,	  I2C_EXTERN_FLTf }
					#endif
					//#if USB_SERIAL_ID			// A feature to change the last char of the USB Serial  number
					//,	'0'						// Default USB SerialNumber ID
					//#endif
					#if CALC_BAND_MUL_ADD		// Frequency Subtract and Multiply Routines (for smart VFO)
					,	{ 0.000 * _2(21)		// Freq subtract value is 0.0MHz (11.21bits)
					,	  0.000 * _2(21)
					,	  0.000 * _2(21)
					,	  0.000 * _2(21)
					,	  0.000 * _2(21)
					,	  0.000 * _2(21)
					,	  0.000 * _2(21)
					,	  0.000 * _2(21) }
					,	{ 1.000 * _2(21)		// Freq multiply value is 1.0MHz (11.21bits)
					,	  1.000 * _2(21)
					,	  1.000 * _2(21)
					,	  1.000 * _2(21)
					,	  1.000 * _2(21)
					,	  1.000 * _2(21)
					,	  1.000 * _2(21)
					,	  1.000 * _2(21) }
					#endif
					};

sint16_t	replyBuf[16];						// USB Reply buffer ([32 bytes maximum])

uint8_t		*usbMsgPtr;							// global "Messy" pointer to pass a return value to USB-EP0.c

uint8_t		Status1 = 0;						// Contains various status flags
uint8_t		Status2 = 0;						// (broken into two 8bit registers for more compact code size)

uint8_t		biasInit = 0;						// Power Amplifier Bias initiate flag
												// (0 = uninitiated => forces init, 1 = class AB, 2 = class A)

sint16_t	ad7991_adc[4];						// Last measured values read from the AD7991 ADC
												// all values adjusted for a full scale 16 bit unsigned int

sint16_t	tmp100_data;						// Last measured value read from the TMP100 temperature
												// sensor
uint16_t	measured_SWR;						// SWR value x 100, in unsigned int format

uint8_t 	pcf_data_out = 0xff;				// Data to the builtin PCF8574 on the Mobo 4.3
												// First time init, all bits high

uint8_t		selectedFilters[2];					// Contains info on which filters are selected, used for LCD print

uint8_t		I2CErrors;							// Indicate timeout (no response) on I2C bus


#include "pe0fko_FreqFromSi570.c"				// Include code is small size and it compiles smaller this way


//
//-----------------------------------------------------------------------------------------
//			Process USB Host to Device transmissions.  No result is returned.
//
//			This function processes control of USB commands 0x30 - 0x36
//-----------------------------------------------------------------------------------------
//
void usbFunctionWrite(USB_Notification_Header_t *rq, uint8_t *data, uint8_t len)
{
	//debug stuff useable with 16x2 (non bargraph) LCD display 
	#if DEBUG_CMD_1LN							// LCD print USB command received in 1st line
	char buf[8];
	lcd_gotoxy(0,0);
	utoa(rq->bRequest,buf,10);
	lcd_puts(buf);
	lcd_putc(';');
	utoa(rq->wValue.w,buf,10);
	lcd_puts(buf);
	lcd_putc(',');
	utoa(rq->wIndex.w,buf,10);
	lcd_puts(buf);
	lcd_putc('.');
	#endif
	#if DEBUG_CMD_2LN							// LCD print USB command received in 2nd line
	char buf[8];
	lcd_gotoxy(0,1);
	utoa(rq->bRequest,buf,10);
	lcd_puts(buf);
	lcd_putc(';');
	utoa(rq->wValue.w,buf,10);
	lcd_puts(buf);
	lcd_putc(',');
	utoa(rq->wIndex.w,buf,10);
	lcd_puts(buf);
	lcd_putc('.');
	utoa(*(uint32_t*)data,buf,10);
	lcd_puts(buf);
	lcd_putc('~');
	#endif

	#if USBTRAFFIC_LED1							// Show USB activity on LED connected to PB2
	PORTB = PORTB ^ IO_LED1;  					// Switch LED state
	#endif

	switch (rq->bRequest)
	{
		case 0x30:
			if (len == 6) {
				CalcFreqFromRegSi570(data);		// Calc the freq from the Si570 register value
				R.Freq[0] = *(uint32_t*)data;
				Status2 |= ENC_NEWFREQ;			// Refresh the active frequency to R.Freq[0]
			}
			break;

		#if CALC_FREQ_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
		case 0x31:								// Write the frequency subtract multiply to the eeprom
			if (len == 8) 
			{
				memcpy(&R.FreqSub, data, 2*sizeof(uint32_t));
				eeprom_write_block(data, &E.FreqSub, 2*sizeof(uint32_t));
			}
			break;
		#endif
		#if CALC_BAND_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
		case 0x31:								// Write the frequency subtract multiply to the eeprom
			if (len == 2*sizeof(uint32_t))
			{
				memcpy(&R.BandSub[rq->wIndex.b0 & 0x0f], data, sizeof(uint32_t));
				memcpy(&R.BandMul[rq->wIndex.b0 & 0x0f], data+4, sizeof(uint32_t));
				eeprom_write_block(data, &E.BandSub[rq->wIndex.b0], sizeof(uint32_t));
				eeprom_write_block(data+4, &E.BandMul[rq->wIndex.b0], sizeof(uint32_t));
			}
			break;
		#endif

		case 0x32:								// Set frequency by value and load Si570
			if (len == 4) {
				if (rq->wIndex.b0 < 10)			// Is it a "legal" memory location
				{
					R.Freq[rq->wIndex.b0] = *(uint32_t*)data;
					Status2 |= ENC_NEWFREQ;		// Refresh the active frequency to R.Freq[0]
												// (even if the change was made to a different frq)
				}
			}
			break;

		case 0x33:								// Write new crystal frequency to EEPROM and use it.
			if (len == 4) {
				R.FreqXtal = *(uint32_t*)data;
				eeprom_write_block(data, &E.FreqXtal, sizeof(E.FreqXtal));
				Status2 |= ENC_NEWFREQ;			// Refresh the active frequency to R.Freq[0]
			}
			break;

		case 0x34:								// Write new startup frequency to eeprom
			if (len == 4) {
				#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
				if (rq->wIndex.b0 < 10)			// Is it a "legal" memory location
				{
					eeprom_write_block(data, &E.Freq[rq->wIndex.b0], sizeof(E.Freq[0]));
					eeprom_write_block(&rq->wIndex.b0, &E.SwitchFreq, sizeof(E.SwitchFreq));
				}
				#else
				eeprom_write_block(data, &E.Freq[0], sizeof(E.Freq[0]));
				#endif
			}
			break;

		case 0x35:								// Write new smooth tune to eeprom and use it.
			if (len == 2) {
				R.SmoothTunePPM = *(uint16_t*)data;
				eeprom_write_block(data, &E.SmoothTunePPM, sizeof(E.SmoothTunePPM));
			}

		#if ENCODER_CMD_INCR					// USB Command to modify Encoder Resolution
		case 0x36:	// Modify Rotary Encoder Increment Resolution
					// Frequency increments in units of ~0.12 Hz (or 8 ~ 1 Hz)
					// Also note that if this value is set too high,
					// or more than 3500ppm of the operating frequency, then
					// "smoothtune does not work"
			if (len == 4) {
				#if PSDR_IQ_OFFSET36			// Display a fixed frequency offset during RX only.
				if (rq->wIndex.b0 == 10)		// Is this a RX frequency Offset input
				{								// This can be used always display the actual
					R.LCD_RX_Offset = *(uint32_t*)data;// used frequency, when using PowerSDR-IQ
					eeprom_write_block(data, &E.LCD_RX_Offset, sizeof(E.LCD_RX_Offset));
				}												
				else
				#endif
				{
					R.Encoder_Resolution = *(uint32_t*)data;
					eeprom_write_block(data, &E.Encoder_Resolution, sizeof(E.Encoder_Resolution));
					Status2 |= ENC_RES;
				}
			}
		#elif PSDR_IQ_OFFSET36 && !ENCODER_CMD_INCR// Display a fixed frequency offset during RX only.
		case 0x36:	// Display a fixed frequency offset during RX only.
			if (len == 4) {
				if (rq->wIndex.b0 == 10)		// Is this a RX frequency Offset input
				{								// This can be used always display the actual
					R.LCD_RX_Offset = *(uint32_t*)data;// used frequency, when using PowerSDR-IQ
					eeprom_write_block(data, &E.LCD_RX_Offset, sizeof(E.LCD_RX_Offset));
				}												
			}
		#endif
	}
}



//
//-----------------------------------------------------------------------------------------
// 			Process USB query commands and return a result (flexible size data payload)
//
//			This function processes control of all USB commands except for 0x30 - 0x36
//-----------------------------------------------------------------------------------------
//
unsigned char usbFunctionSetup(USB_Notification_Header_t *rq)
{
	uint8_t index = rq->wIndex.b0;				// This substitution saves codespace								

	usbMsgPtr = (uint8_t*) &replyBuf;			// global "Messy" pointer for Si570 work
	replyBuf[0].b0 = 0xff;						// Return value 0xff 0=> command not supported

	//debug stuff useable with 16x2 (non bargraph) LCD display 
	#if DEBUG_CMD_1LN							// LCD print USB command received in 1st line
	char buf[8];
	lcd_gotoxy(0,0);
	utoa(rq->bRequest,buf,10);
	lcd_puts(buf);
	lcd_putc(':');
	utoa(rq->wValue.w,buf,10);
	lcd_puts(buf);
	lcd_putc(',');
	utoa(rq->wIndex.w,buf,10);
	lcd_puts(buf);
	lcd_putc('.');
	#endif
	#if DEBUG_CMD_2LN							// LCD print USB command received in 2nd line
	char buf[8];
	lcd_gotoxy(0,1);
	utoa(rq->bRequest,buf,10);
	lcd_puts(buf);
	lcd_putc(':');
	utoa(rq->wValue.w,buf,10);
	lcd_puts(buf);
	lcd_putc(',');
	utoa(rq->wIndex.w,buf,10);
	lcd_puts(buf);
	lcd_putc('.');
	#endif

	#if USBTRAFFIC_LED1							// Show USB activity on LED connected to PB2
	PORTB = PORTB ^ IO_LED1;  					// Switch LED state
	#endif

	switch(rq->bRequest)
	{
		case 0x00:								// Return software version number
			replyBuf[0].w = (VERSION_MAJOR<<8)|(VERSION_MINOR);
			return sizeof(uint16_t);


		#if LEGACY_CMD01_04						// Legacy Commands 0x01 - 0x04. Normally not needed
		case 0x01:								// set port pin directions (PORTD)
			//
			// Here we may want to insert a #define-able protection for the GPIO style
			// Low Pass filter select signals.  A later project, if needed.
			//
			#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
												// Protect Shaft encoder bits
			IO_DDR_MP = rq ->wValue.b0 & ~(ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
			#else
			IO_DDR_MP = rq->wValue.b0;
			#endif	
			break;

		case 0x02:								// read PORTD
			replyBuf[0].b0 = IO_PIN_MP;
			return sizeof(uint8_t);

		case 0x03:								// read port states, PORTD 
			replyBuf[0].b0 = IO_PORT_MP;
			return sizeof(uint8_t);

		case 0x04:								// set outputs, PORTD
			//
			// Here we may want to insert a #define-able protection for the GPIO style
			// Low Pass filter select signals.  A later project, if needed.
			//
			#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
												// Protect Shaft encoder bits
			IO_PORT_MP = rq ->wValue.b0 | (ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
			#else
			IO_PORT_MP = rq->wValue.b0;
			#endif
			break;
		#endif//LEGACY_CMD01_04


		case 0x0f:								// Watchdog reset
			Status1 |= REBOOT;
			return 0;

		#if LEGACY_CMD15_16						// Legacy Commands 0x01 - 0x04. Normally not needed
		case 0x15:								// Set IO port with mask and data bytes
			//
			// Here we may want to insert a #define-able protection for the GPIO style
			// Low Pass filter select signals.  A later project, if needed.
			//
			#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
												// Protect Shaft encoder bits
			IO_DDR_MP  = rq ->wValue.b0 & ~(ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
			IO_PORT_MP = index |  (ENC_A_PIN | ENC_B_PIN | ENC_PUSHB_PIN);
			#else
			IO_DDR_MP  = rq->wValue.b0;
			IO_PORT_MP = index;
			#endif
			// passthrough to case 0x16

		case 0x16:								// Read I/O bits
			replyBuf[0].b0 = IO_PIN_MP;
        	return sizeof(uint16_t);
		#endif//LEGACY_CMD15_16


		case 0x17:								// Read and Write the Filter Cross over point's and use it.
			{
				// RX Filter cross over point table.
				if (rq->wIndex.b1 == 0)
				{
					if (index < 8)		// Make sure we don't overwrite other parts of table
					{
					R.FilterCrossOver[index].w = rq->wValue.w;

					eeprom_write_block(&R.FilterCrossOver[index].w, 
						&E.FilterCrossOver[index].w, 
						sizeof(E.FilterCrossOver[0].w));
					}
					usbMsgPtr = (uint8_t*)&R.FilterCrossOver;
					return 8 * sizeof(uint16_t);
				}
				
				// TX Filter cross over point table.
				else
				{
					if (index < TXF)			// Make sure we don't overwrite other parts of table
					{
						R.TXFilterCrossOver[index].w = rq->wValue.w;

						eeprom_write_block(&R.TXFilterCrossOver[index].w, 
							&E.TXFilterCrossOver[index].w, 
							sizeof(E.TXFilterCrossOver[0].w));
					}
					usbMsgPtr = (uint8_t*)&R.TXFilterCrossOver;
					return TXF * sizeof(uint16_t);
				}
			}


		#if SCRAMBLED_FILTERS					// Enable a non contiguous order of filters
		case 0x18:								// Set the Band Pass Filter Address for one band: 0,1,2...7
			eeprom_write_block(&rq->wValue.b0, &E.FilterNumber[index & 0x03], sizeof (uint8_t));
			R.FilterNumber[index & 0x03] = rq->wValue.b0;
			// passthrough to case 0x19

		case 0x19:								// Read the Band Pass Filter Addresses for bands 0,1,2...7
			usbMsgPtr = (uint8_t*)R.FilterNumber; 
			return sizeof(R.FilterNumber);


		case 0x1a:								// Set the Low Pass Filter Address for one band: 0,1,2...15
			eeprom_write_block(&rq->wValue.b0, &E.TXFilterNumber[index & 0x07], sizeof (uint8_t));
			R.TXFilterNumber[index & 0x03] = rq->wValue.b0;


		case 0x1b:								// Read the Low Pass Filter Addresses for bands 0,1,2...15
			usbMsgPtr = (uint8_t*)R.TXFilterNumber;
			return sizeof(R.TXFilterNumber);
		#endif


		#if LEGACY_CMD_20						// Legacy Command 0x20. Normally not needed
		case 0x20:								// [DEBUG] Write byte to Si570 register
			Si570CmdReg(rq->wValue.b1, index);  // Value high byte and Index low byte

			Status2 |= SI570_OFFL;				// Next SetFreq call no smoothtune

			replyBuf[0].b0 = I2CErrors;			// return I2C transmission error status
    	    return sizeof(uint8_t);
		#endif


		//case 0x30:							// Set frequnecy by register and load Si570
		//case 0x31:							// Write the FREQ mul & add to the eeprom
		//case 0x32:							// Set frequency by value and load Si570
		//case 0x33:							// write new crystal frequency to EEPROM and use it.
		//case 0x34:							// Write new startup frequency to eeprom
		//case 0x35:							// Write new smooth tune to eeprom and use it.
		//case 0x36:							// USB Command to modify Encoder Resolution
		//case 0x37:
		//	return 0		;					// Hey we're not supposed to be here
												// 	we use usbFunctionWrite() to transfer data

		
		#if CALC_FREQ_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
		case 0x39:								// Return the frequency subtract multiply
			usbMsgPtr = (uint8_t*)&R.FreqSub;
		return 2 * sizeof(uint32_t);
		#endif
		#if CALC_BAND_MUL_ADD					// Frequency Subtract and Multiply Routines (for smart VFO)
		case 0x39:								// Return the frequency subtract multiply
			memcpy(&replyBuf[0].w, &R.BandSub[index & 0x07], sizeof(uint32_t));
			memcpy(&replyBuf[2].w, &R.BandMul[index & 0x07], sizeof(uint32_t));
        	return 2 * sizeof(uint32_t);
		#endif


		case 0x3a:								// Return running frequnecy
			usbMsgPtr = (uint8_t*)&R.Freq[index];
        	return sizeof(uint32_t);


		case 0x3b:								// Return smooth tune ppm value
			usbMsgPtr = (uint8_t*)&R.SmoothTunePPM;
        	return sizeof(uint16_t);


		case 0x3c:								// Return the startup frequency
			eeprom_read_block(replyBuf, &E.Freq[index], sizeof(E.Freq[index]));
			return sizeof(uint32_t);


		case 0x3d:								// Return the XTal frequnecy
			usbMsgPtr = (uint8_t*)&R.FreqXtal;
			return sizeof(uint32_t);


//		case 0x3e:								// read out calculated frequency control registers
//			usbMsgPtr = (uint8_t*)&Si570_Data;
//			return sizeof(Si570_t);
        
 
		case 0x3f:								// read out chip frequency control registers
			return GetRegFromSi570();			// read all registers in one block to replyBuf[]


		#if LEGACY_CMD_40						// Legacy Command 0x40. Normally not needed
		case 0x40:								// return I2C transmission error status
			replyBuf[0].b0 = I2CErrors;
			return sizeof(uint8_t);
		#endif


		case 0x41:		// Set a new i2c address for a device, or reset the EEPROM to factory default
						// if Value contains 0xff, then factory defaults will be loaded on reset.
						
						// Index 0 reads/modifies the Si570_I2C_addr
						// Index 1 reads/modifies the I2C address for the onboard PCF8574
						// Index 2 reads/modifies the I2C address for the first PCF8574 used in the LPF Mobo
						// Index 3 reads/modifies the I2C address for the second PCF8574 used in the LPF Mobo
						// Index 4 reads/modifies the I2C address for the onboard TMP100 temperature sensor
						// Index 5 reads/modifies the I2C address for the onboard AD5301 8 bit DAC
						// Index 6 reads/modifies the I2C address for the onboard AD7991 4 x ADC
						// Index 7 reads/modifies the I2C address for the external PCF8574 used for FAN, attenuators etc

			if (rq->wValue.b0 == 0xff)
        	{
				// Force an EEPROM update:
				// eeprom_write_block appears to take (14 bytes) less pgm space than eeprom_write_byte 
				eeprom_write_block(&rq->wValue.b0, &E.EEPROM_init_check, sizeof (uint8_t));
				Status1 |= REBOOT;				// Reboot by watchdog timer
				return 0;
			}
			
			if (rq->wValue.b0)					// If value field > 0, then update EEPROM settings
			{
				replyBuf[0].b0 = rq->wValue.b0;

				switch (index) 
				{
					case 0:
						eeprom_write_block(&rq->wValue.b0, &E.Si570_I2C_addr, sizeof (uint8_t));
						break;
					case 1:	
						eeprom_write_block(&rq->wValue.b0, &E.PCF_I2C_Mobo_addr, sizeof (uint8_t));
						break;
					case 2:
						eeprom_write_block(&rq->wValue.b0, &E.PCF_I2C_lpf1_addr, sizeof (uint8_t));
						break;
					case 3:
						eeprom_write_block(&rq->wValue.b0, &E.PCF_I2C_lpf2_addr, sizeof (uint8_t));
						break;
					case 4:
						eeprom_write_block(&rq->wValue.b0, &E.TMP100_I2C_addr, sizeof (uint8_t));
						break;
					case 5:	
						eeprom_write_block(&rq->wValue.b0, &E.AD5301_I2C_addr, sizeof (uint8_t));
						break;
					case 6:
						eeprom_write_block(&rq->wValue.b0, &E.AD7991_I2C_addr, sizeof (uint8_t));
						break;
					#if FAN_CONTROL && EXTERN_PCF_FAN	// Fan Control by External PCF8574
					case 7:
						eeprom_write_block(&rq->wValue.b0, &E.PCF_I2C_Ext_addr, sizeof (uint8_t));
						break;
					#endif
				}
			}
			else								// Else just read and return the current value
			{
				switch (index) 
				{
					case 0:	
						replyBuf[0].b0 = R.Si570_I2C_addr;
						break;
					case 1:
						replyBuf[0].b0 = R.PCF_I2C_Mobo_addr;
						break;
					case 2:
						replyBuf[0].b0 = R.PCF_I2C_lpf1_addr;
						break;
					case 3:
						replyBuf[0].b0 = R.PCF_I2C_lpf2_addr;
						break;
					case 4:	
						replyBuf[0].b0 = R.TMP100_I2C_addr;
						break;
					case 5:	
						replyBuf[0].b0 = R.AD5301_I2C_addr;
						break;
					case 6:	
						replyBuf[0].b0 = R.AD7991_I2C_addr;
						break;
					#if FAN_CONTROL && EXTERN_PCF_FAN	// Fan Control by External PCF8574
					case 7:	
						replyBuf[0].b0 = R.PCF_I2C_Ext_addr;
						break;
					#endif
				}
			}
			return sizeof(uint8_t);

		
		//#if USB_SERIAL_ID						// A feature to change the last char of the USB Serial  number
		//case 0x43:							// Get/Set the USB SeialNumber ID
		//	replyBuf[0].b0 = R.SerialNumber;
		//	if (rq->wValue.b0 != 0) {			// Only set if Value != 0
		//		R.SerialNumber = rq->wValue.b0;
		//		eeprom_write_byte(&E.SerialNumber, R.SerialNumber);
		//	}
		//	return sizeof(R.SerialNumber);
		//#endif


		case 0x50:								//Set/Release PTT and get cw-key status
			if (rq->wValue.b0 == 0)
			{
				// Clear PTT flag
				Status1 = Status1 & ~TX_FLAG;
				#if MOBO_STYLE_IO
				MoboPCF_set(Mobo_PCF_TX);
				#endif//MOBO_STYLE_IO
				#if OLDSTYLE_IO
				IO_PORT_PTT_CWKEY &= ~IO_PTT;
				#endif//OLDSTYLE_IO
			}
			else
			{
				
				Status1 = Status1 | TX_FLAG;	// Set the TX flag

				// Set PTT if there are no inhibits
				if (!(Status1 & (TMP_ALARM | PA_CAL)))
				{
					biasInit = 0;				// Ensure that correct bias is set by PA_bias()
					
					// Switch to Transmit mode, set TX out
					#if MOBO_STYLE_IO
					MoboPCF_clear(Mobo_PCF_TX);
					#endif//MOBO_STYLE_IO
					#if OLDSTYLE_IO
					IO_PORT_PTT_CWKEY |= IO_PTT;
					#endif//OLDSTYLE_IO
				}
			}
			// Passthrough to Cmd 0x51
/*
		case 0x51:								// read CW key levels
			replyBuf[0].b0 = 0x00;
			// read pin and set regbit accordingly
			if (IO_PIN_PTT_CWKEY & IO_CWKEY1) replyBuf[0].b0 |= REG_CWSHORT;
			// read pin and set regbit accordingly
			if (IO_PIN_PTT_CWKEY & IO_CWKEY2) replyBuf[0].b0 |= REG_CWLONG;
        	return sizeof(uint8_t);
*/

		case 0x51:								// read CW key levels
			replyBuf[0].b0 = 0x00;
			// read pin and set regbit accordingly
			if (IO_PIN_PTT_CWKEY & IO_CWKEY1) replyBuf[0].b0 |= REG_CWSHORT;
			// read pin and set regbit accordingly
			if (IO_PIN_PTT_CWKEY & IO_CWKEY2) replyBuf[0].b0 |= REG_CWLONG;
			// read current PTT state and set regbit accordingly
			if (Status1 & TX_FLAG) replyBuf[0].b0 |= REG_TX_state;
        	return sizeof(uint8_t);


		case 0x61:		// Read ADC inputs,
						// Index byte points to which ADC input to read.
						
						// Index 0 = PA current, full scale 0xfff0 = 2.5A
						// Index 1 = Power Output
						// Index 2 = Power reflected
						// Index 3 = Supply voltage, full scale 0xfff0 = 15.64V
						// Index 4 = Temperature in degC.Signed Int.  0 = 0 deg C
						// 			 32640 =  128 deg C, 32768 = -128 deg C

			if (index < 4)						// Values from AD7991
			{
				replyBuf[0].w = ad7991_adc[index].w;
			}				
			else								// Read current temperature
			{
				replyBuf[0].w = tmp100_data.w;
			}
			return sizeof(uint16_t);

 
		case 0x64:		// Read/Modify the PA High Temperature limit
						// If wValue contains a value higher than 0,
						// then hi_tmp_trigger is updated with this value.
						// If 0, then read current value.

						// If Fan Control is enabled, then also two trigger point values
						// Value selected by using Index
						// Index 0:	PA High Temperature limit (deg C)
						// Index 1:	PA Fan On trigger point (deg C)
						// Index 2: PA Fan Off trigger point (deg C)
						// Index 3: Which bit is used to control the Cooling Fan

			#if	FAN_CONTROL						// Turn PA Cooling FAN On/Off, based on temperature
			if (rq->wValue.b0 > 0)				// If value field > 0, then update EEPROM settings
			{
				switch (index) 
				{
					case 0:
						eeprom_write_block(&rq->wValue.b0, &E.hi_tmp_trigger, sizeof (uint8_t));
						R.hi_tmp_trigger = rq->wValue.b0;
						break;
					case 1:
						eeprom_write_block(&rq->wValue.b0, &E.Fan_On, sizeof (uint8_t));
						R.Fan_On = rq->wValue.b0;
						break;
					case 2:
						eeprom_write_block(&rq->wValue.b0, &E.Fan_Off, sizeof (uint8_t));
						R.Fan_Off = rq->wValue.b0;
						break;
					#if FAN_CONTROL && EXTERN_PCF_FAN	// Fan Control by External PCF8574
					case 3:
						eeprom_write_block(&rq->wValue.b0, &E.PCF_fan_bit, sizeof (uint8_t));
						R.PCF_fan_bit = rq->wValue.b0;
						break;
					#endif
				}
			}
			else
			{
				switch (index) 
				{
					case 0:
						replyBuf[0].b0 = R.hi_tmp_trigger;
						break;
					case 1:
						replyBuf[0].b0 = R.Fan_On;
						break;
					case 2:
						replyBuf[0].b0 = R.Fan_Off;
						break;
					#if FAN_CONTROL && EXTERN_PCF_FAN	// Fan Control by External PCF8574
					case 3:
						replyBuf[0].b0 = R.PCF_fan_bit;
						break;
					#endif
				}
			}
			return sizeof(uint8_t);

			#else
			if (rq->wValue.b0)
			{		// New value
					eeprom_write_block(&rq->wValue.b0, &E.hi_tmp_trigger, sizeof (uint8_t));
					R.hi_tmp_trigger = rq->wValue.b0;
			}
			// Return current value
			replyBuf[0].b0 = R.hi_tmp_trigger;
			return sizeof(uint8_t);
			#endif



		case 0x65:		// Read/Modify five settings (all 8 bit values).  
						// If Value = 0 then read, else modify and read back:

						// Index 0:	Bias_Select; 0xff = Force Calibrate, 1 = LO, 2 = HI
						// Index 1:	PA Bias in 10 * mA, typically 2 = 20ma = Class B
						// Index 2: PA Bias in 10 * mA, typically 35 = 350ma = Class A
						// Index 3: PA Bias setting, LO (normally an auto adjusted value)
						// Index 4: PA Bias setting, HI  (normally an auto adjusted value)

			if (rq->wValue.b0 > 0)				// If value field > 0, then update EEPROM settings
			{
				switch (index) 
				{
					case 0:						// Which bias, 0 = Cal, 1 = LO, 2 = HI
						eeprom_write_block(&rq->wValue.b0, &E.Bias_Select, sizeof (uint8_t));
						replyBuf[0].b0 = R.Bias_Select = rq->wValue.b0;
						break;
	
					case 1:						// PA Bias in 10 * mA, Low bias setting
						eeprom_write_block(&rq->wValue.b0, &E.Bias_LO, sizeof (uint8_t));
						replyBuf[0].b0 = R.Bias_LO = rq->wValue.b0;
						break;
					case 2:						// PA Bias in 10 * mA, High bias setting
						eeprom_write_block(&rq->wValue.b0, &E.Bias_HI, sizeof (uint8_t));
						replyBuf[0].b0 = R.Bias_HI = rq->wValue.b0;
						break;

					case 3:						// PA Bias setting, Low bias setting
						eeprom_write_block(&rq->wValue.b0, &E.cal_LO, sizeof (uint8_t));
						replyBuf[0].b0 = R.cal_LO = rq->wValue.b0;
						break;

					case 4:						// PA Bias setting, High bias setting
						eeprom_write_block(&rq->wValue.b0, &E.cal_HI, sizeof (uint8_t));
						replyBuf[0].b0 = R.cal_HI = rq->wValue.b0;
						break;
				}
			}
			else								// Else just read and return the current value
			{
				switch (index) 
				{
					case 0:						// Which bias, 0 = Cal, 1 = LO, 2 = HI
						replyBuf[0].b0 = R.Bias_Select;
						break;
	
					case 1:						// PA Bias in mA, LO
						replyBuf[0].b0 = R.Bias_LO;
						break;
					case 2:						// PA Bias in mA, HI
						replyBuf[0].b0 = R.Bias_HI;
						break;
					case 3:						// PA Bias setting, LO
						replyBuf[0].b0 = R.cal_LO;
						break;
					case 4:						// PA Bias setting, HI
						replyBuf[0].b0 = R.cal_HI;
						break;
				}
			}
			return sizeof(uint8_t);


		case 0x66:		// Read/Modify four settings (all 16 bit values).
						// If Value = 0 then read, else modify and read back:

						// 	Index 0: Min P out measurement for SWR trigger 
						// 	Index 1: SWR Timer expiry value (in units of 10ms, can be set awfully long:)
						// 	Index 2: Max SWR threshold in units of 10x SWR (SWR of 2.7 = 27)
						//	Index 3: Power meter calibration value
						//	Index 4: Power Meter bargraph Fullscale in W
						//	Index 5: SWR Meter bargraph Fullscale in SWR - 1 (if enabled by #define)
						//  Index 6: Number of PEP measurement samples for LCD power display (1-20)

			if (rq->wValue.w)					// If value field > 0, then update EEPROM settings
			{
				switch (index) 
				{
					case 0:						// Min P out measurement for SWR trigger
						eeprom_write_block(&rq->wValue.w, &E.P_Min_Trigger, sizeof (E.P_Min_Trigger));
						replyBuf[0].w = R.P_Min_Trigger = rq->wValue.w;
						break;
	
					case 1:						// Timer loop value
						eeprom_write_block(&rq->wValue.w, &E.SWR_Protect_Timer, sizeof (E.SWR_Protect_Timer));
						replyBuf[0].w = R.SWR_Protect_Timer = rq->wValue.w;
						break;
					case 2:						// Max SWR threshold
						eeprom_write_block(&rq->wValue.w, &E.SWR_Trigger, sizeof (E.SWR_Trigger));
						replyBuf[0].w = R.SWR_Trigger = rq->wValue.w;
						break;
					case 3:						// Max SWR threshold
						eeprom_write_block(&rq->wValue.w, &E.PWR_Calibrate, sizeof (E.PWR_Calibrate));
						replyBuf[0].w = R.PWR_Calibrate = rq->wValue.w;
						break;
					#if BARGRAPH
					case 4:						// Fullscale Power Bargraph value
						eeprom_write_block(&rq->wValue.b0, &E.PWR_fullscale, sizeof (E.PWR_fullscale));
						replyBuf[0].b0 = R.PWR_fullscale = rq->wValue.b0;
						break;
					#if	BARGRAPH_SWR_SCALE		// Add option to adjust the Fullscale value for the SWR bargraph
					case 5:						// Fullscale SWR Bargraph value
						eeprom_write_block(&rq->wValue.b0, &E.SWR_fullscale, sizeof (E.SWR_fullscale));
						replyBuf[0].b0 = R.SWR_fullscale = rq->wValue.b0;
						break;
					#endif
					#endif
					#if	PWR_PEP_ADJUST			// Add option to adjust the number of samples in PEP measurement
					case 6:						// Number of samples in PEP measurement
						eeprom_write_block(&rq->wValue.b0, &E.PEP_samples, sizeof (E.PEP_samples));
						replyBuf[0].b0 = R.PEP_samples = rq->wValue.b0;
						break;
					#endif
				}
			}

			else								// Else just read and return the current value
			{
				switch (index) 
				{
					case 0:						// Min P out measurement for SWR trigger
						replyBuf[0].w = R.P_Min_Trigger;
						break;
	
					case 1:						// Timer loop value
						replyBuf[0].w = R.SWR_Protect_Timer;
						break;
					case 2:						// Max SWR threshold
						replyBuf[0].w = R.SWR_Trigger;
						break;
					case 3:						// Max SWR threshold
						replyBuf[0].w = R.PWR_Calibrate;
						break;
					#if BARGRAPH
					case 4:						// Fullscale Power Bargraph value
						replyBuf[0].b0 = R.PWR_fullscale;
						break;
					#if	BARGRAPH_SWR_SCALE		// Add option to adjust the Fullscale value for the SWR bargraph
					case 5:						// Fullscale SWR Bargraph value
						replyBuf[0].b0 = R.SWR_fullscale;
						break;
					#endif
					#endif
					#if	PWR_PEP_ADJUST			// Add option to adjust the number of samples in PEP measurement
					case 6:						// Number of samples in PEP measurement
						replyBuf[0].b0 = R.PEP_samples;
						break;
					#endif		
				}
			}
			return sizeof(uint16_t);


		#if	ENCODER_RESOLUTION		// USB command to modify the Ecoder Resolution
		case 0x67:					// Read/Modify Rotary Encoder Increment Resolution Setting
									// Normally set to the number of resolvable states per revolution
			if (rq->wValue.w)
			{		// New value
					eeprom_write_block(&rq->wValue.w, &E.Resolvable_States, sizeof (uint16_t));
					R.Resolvable_States = rq->wValue.w;
			}
			// Return current value
			replyBuf[0].w = R.Resolvable_States;
			return sizeof(uint16_t);
		#endif


		#if PSDR_IQ_OFFSET68 		// Display a fixed frequency offset during RX only.
		case 0x68:					// Display a fixed frequency offset during RX only.
			if (index)				// If Index>0, then New value contained in Value
			{
				eeprom_write_block(&rq->wValue.b0, &E.LCD_RX_Offset, sizeof (uint8_t));
				R.LCD_RX_Offset = rq->wValue.b0;// used frequency, when using PowerSDR-IQ
			}
			// Return current value
			replyBuf[0].b0 = R.LCD_RX_Offset;
			return sizeof(uint8_t);
		#endif		


		#if PCF_WRITE_COMMAND					// [Option] Enable command 0x6e for

												// direct control of PCF8574 extenders
		case 0x6e:								// Send byte to (PCF8574) GPIO Extender
			pcf8574_byte(index, rq->wValue.b0);
			// Passthrough to Cmd 0x6f or END XXXXXXXXXXXX
		#endif


		#if PCF_READ_COMMAND					// [Option] Enable command 0x6f for
												// direct control of PCF8574 extenders
		case 0x6f:								// Read byte from (PCF8574) GPIO Extender
			replyBuf[0].b0 = pcf8574_read(index);
			return sizeof(uint8_t);
		#endif
		
		default:
			break;
	}
	return 1;
}



//
//-----------------------------------------------------------------------------------------
// 							Do stuff while not serving USB
//
// This task runs in an endless loop, whenever the USB tasks are not stealing clock cycles
//-----------------------------------------------------------------------------------------
//
void maintask(void)
{
	// Now we can do all kinds of business, such as serving LCD, SWR alarm, etc ...

	static uint16_t lastIteration1, lastIteration2;	// Counters to keep track of time

	#if SLOW_POLL_DURING_RX	|| SSLO_POLL_DURING_RX	// Minimize I2C traffic during receive
	static uint16_t Timerval = 0;					// Timer used for 1 second polls
	#endif

	uint16_t Timer1val, Timer1val2;					// Timers used for 100ms and 10ms polls

	static uint8_t pushcount=0;						// If Shaft Encoder, then used to time a push button (max 2.5s)
	
	//-------------------------------------------------------------------------------
	// Here we do routines which are to be run through as often as possible
	// currently measured to be approximately once every 30 us (1/33333 s)
	//-------------------------------------------------------------------------------
	{
		#if ENCODER_SCAN_STYLE						// Shaft Encoder which scans the GPIO inputs
		encoder_scan();								// Scan the Shaft Encoder
		#endif

		#if FAST_LOOP_THRU_LED1						// Blink PB2 LED every time when going through the mainloop 
		PORTB = PORTB ^ IO_LED1;  					// Blink a led
		#endif
		#if FAST_LOOP_THRU_LED2						// Blink PB3 LED every time when going through the mainloop
		PORTB = PORTB ^ IO_LED2;  					// Blink a led
		#endif

		#if	FADE_LOOP_THRU_LED2						// Some slight fun and games. Slowly increase the blink period
		static uint8_t period, onoff;
		if (onoff==0) period++;
		if (onoff<period) PORTB = PORTB | IO_LED2;  // Led on
		else PORTB = PORTB & ~IO_LED2;				// Led off
		onoff++;
		#endif
		
		#if	FADE_LOOP_THRU_LEDS						// Fun and games. Slowly adjust the blink period, both leds
		static uint8_t period, onoff;
		if (onoff==0) period++;
		if (onoff<period)
		{
			PORTB = PORTB | IO_LED1;  				// Led1 on
			if (!(Status1 & TX_FLAG))
				PORTB = PORTB & ~IO_LED2;			// Led2 off
		}
		else
		{
			PORTB = PORTB & ~IO_LED1;				// Led1 off
			PORTB = PORTB | IO_LED2;				// Led2 on
		}
		onoff++;
		#endif
		
		while (Status1 & REBOOT);					// If REBOOT flag is set, then get
													// stuck here, and reboot by watchdog
	}
	
	//-------------------------------------------------------------------------------
	// Here we do routines which are to be accessed once every ~second or less
	// We have a free running timer which matures once every ~1.05 seconds
	//-------------------------------------------------------------------------------
	#if SLOW_POLL_DURING_RX							// Poll the I2C bus at 1s intervals only during Receive
	if (Timerval > TCNT1)							// When the timer overflows, do stuff
	{
		if(!(Status1 & TX_FLAG))					// Only do the below during RX
		{
			tmp100(R.TMP100_I2C_addr);				// Update temperature reading,
													// value read into tmp100data variable

			ad7991_poll(R.AD7991_I2C_addr);			// Polls the AD7991 every time (9 bytes)
													// => constant traffic on I2C
			#if LCD_PAR_DISPLAY2
			lcd_display_P_SWR_V_C_T();				// Display non-static measured values
			#endif
		}
	}
	Timerval = TCNT1;
	#elif SSLO_POLL_DURING_RX						// Poll the I2C bus at 10s intervals only during Receive
	if (Timerval > TCNT1)							// When the timer overflows, do stuff
	{
		static uint8_t a; a++;						// Second Counter
		if (a > 10)
		{
			if(!(Status1 & TX_FLAG))				// Only do the below during RX
			{
				tmp100(R.TMP100_I2C_addr);			// Update temperature reading,
													// value read into tmp100data variable

				ad7991_poll(R.AD7991_I2C_addr);		// Polls the AD7991 every time (9 bytes)
													// => constant traffic on I2C
				#if LCD_PAR_DISPLAY2
				lcd_display_P_SWR_V_C_T();			// Display non-static measured values
				#endif
			}
			a = 0;
		}
	}
	Timerval = TCNT1;
	#endif//SLOW_POLL_DURING_RX	

	//-------------------------------------------------------------------------------
	// Here we do routines which are to be accessed once every 1/10th of a second
	// We have a free running timer which matures once every ~1.05 seconds
	//-------------------------------------------------------------------------------
	Timer1val = TCNT1/6554; // get current Timer1 value, changeable every ~1/10th sec
	if (Timer1val != lastIteration1)	// Once every 1/10th of a second, do stuff
	{
		lastIteration1 = Timer1val;					// Make ready for next iteration

		#if SLOW_LOOP_THRU_LED1						// Blink PB2 LED every 100ms, when going through the mainloop 
		PORTB = PORTB ^ IO_LED1;  					// Blink a led
		#endif
		#if SLOW_LOOP_THRU_LED2						// Blink PB3 LED every 100ms, when going through the mainloop
		PORTB = PORTB ^ IO_LED2;  					// Blink a led
		#endif
		
		#if NO_I2C_DURING_RX						// I2C noise test, no I2C traffic during RX
		if (Status1 & TX_FLAG)
		#endif

		// Minimize I2C traffic during receive
		#if !(SLOW_POLL_DURING_RX || SSLO_POLL_DURING_RX)
		tmp100(R.TMP100_I2C_addr);					// Update temperature reading,
													// value read into tmp100data variable
		#endif

		//
		// Protect the Transmit Power Amplifier against overtemperature
		//
		if(Status1 & TX_FLAG)						// If Transmitter is on the air
		{
			// Minimize I2C traffic during receive, only read tmp rapidly during TX
			#if SLOW_POLL_DURING_RX || SSLO_POLL_DURING_RX
			tmp100(R.TMP100_I2C_addr);				// Update temperature reading,
			#endif									// value read into tmp100data variable

			if(tmp100_data.i1 > R.hi_tmp_trigger)	// Do we have a thermal runaway of the PA?
			{
				Status1 |= TMP_ALARM;				// Set the Temperature Alarm flag

				#if MOBO_STYLE_IO
				MoboPCF_set(Mobo_PCF_TX);			// Shut down transmitter
				#endif//MOBO_STYLE_IO
				#if OLDSTYLE_IO
				IO_PORT_PTT_CWKEY &= ~IO_PTT;
				#endif//OLDSTYLE_IO
			}
		}
		else Status1 &= ~TMP_ALARM;					// Clear the Hi Temp flag


		#if	FAN_CONTROL				// Turn PA Cooling FAN On/Off, based on temperature
		//
		// Activate Cooling Fan for the Transmit Power Amplifier, if needed
		//
		// Are we cool?
		if(Status2 & COOLING_FAN)
		{
			if(tmp100_data.i1 <= R.Fan_Off)
			{
				Status2 &= ~COOLING_FAN;			// Set FAN Status Off
				#if	PORTD_FAN
				IO_PORT_FC = IO_PORT_FC & ~IO_FC; 	// Set Fan Bit low	
				#elif	BUILTIN_PCF_FAN
				MoboPCF_clear(PCF_MOBO_FAN_BIT);	// Builtin PCF, set fan bit low
				#elif	EXTERN_PCF_FAN
				//Read current status of the PCF
				uint8_t x = pcf8574_read(R.PCF_I2C_Ext_addr);
				//and turn off the FAN bit
				x &= ~R.PCF_fan_bit;				// Extern PCF, set fan bit low
				pcf8574_byte(R.PCF_I2C_Ext_addr, x);
				#endif
			}
		}
		// Do we need to start the cooling fan?
		else
		{
			if(tmp100_data.i1 > R.Fan_On)
			{
				Status2 |= COOLING_FAN;				// Set FAN Status ON
				#if	PORTD_FAN
				IO_PORT_FC = IO_PORT_FC | IO_FC; 	// Set Fan Bit high
				#elif	BUILTIN_PCF_FAN
				MoboPCF_set(PCF_MOBO_FAN_BIT);		// Builtin PCF, set fan bit high
				#elif	EXTERN_PCF_FAN
				//Read current status of the PCF
				uint8_t x = pcf8574_read(R.PCF_I2C_Ext_addr);
				//and turn on the FAN bit
				x |= R.PCF_fan_bit;					// Extern PCF, set fan bit high
				pcf8574_byte(R.PCF_I2C_Ext_addr, x);
				#endif
			}
		}
		#endif


		#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE	// Shaft Encoder VFO function
		#if ENCODER_FAST_ENABLE						// Variable speed Rotary Encoder feature
		//
		// Encoder activity watchdog, if Fast mode is active
		//
		static uint8_t fast_patience;				// Patience timer

		if (Status2 & ENC_FAST)						// Is fast mode active?
		{
			if (Status2 & ENC_NEWFREQ)				// Encoder activity, reset timer
				fast_patience=0;
			else									// No activity, increase timer
				fast_patience++;
					
			if(fast_patience>=ENC_FAST_PATIENCE)	// No activity for a long time, revert to normal mode
			{
				Status2 &= ~ENC_FAST;
				fast_patience = 0;
			}
		}
		#endif
		//
		// Read Pushbutton state from Shaft encoder and manage Frequency band memories
		//
		if (pushcount >= ENC_PUSHB_MAX)				// "Long Push", store settings
		{
			eeprom_write_block(&R.Freq[0], &E.Freq[R.SwitchFreq], sizeof(R.Freq[0]));
			eeprom_write_block(&R.SwitchFreq, &E.SwitchFreq, sizeof (uint8_t));
			// // Maybe a bit redundant: Store in memory location 0:
			//eeprom_write_block(&R.Freq[0], &E.Freq[0], sizeof(R.Freq[0]));
			Status2 = Status2 | ENC_NEWFREQ | ENC_STORED;	// We have a new frequency stored.
													// NEWFREQ signals a frq update
													// STORED signals an LCD message
		}
		else if (ENC_PUSHB_INPORT & ENC_PUSHB_PIN) 	// Pin high = just released, or not pushed
		{
			if (pushcount >= ENC_PUSHB_MIN)			// Release after a "Short push"
			{	
				#if ENCODER_INT_STYLE				// Interrupt driven Shaft Encoder
				cli();								// Don't want a rogue interrupt to mess with us
				#endif
				R.SwitchFreq++;						// rotate through memories
				if (R.SwitchFreq > 9) R.SwitchFreq = 1;
				R.Freq[0] = R.Freq[R.SwitchFreq];	// Fetch last stored frequency in next band
				Status2 |= ENC_NEWFREQ;				// Signal a new frequency to be written
													// to the Si570 device
				#if ENCODER_INT_STYLE				// Interrupt driven Shaft Encoder
				sei();
				#endif
			}
			else
			{										// No push or a very short push, do nothing
				pushcount = 0;						// Initialize push counter for next time
			}
		}
		else if (!(Status2 & ENC_STORED))			// Button Pushed, count up the push timer
		{											// (unless this is tail end of a long push,
			pushcount++;							//  then do nothing)
		}
		
		if(Status2 & ENC_STORED)
		{
			lcd_display_Memory_Stored();			// Display Memory Stored for a certain amount of time
		}

		#endif		


		#if LCD_PAR_DISPLAY||LCD_I2C_DISPLAY
		//
		// Print to LCD Display
		//
		lcd_display();
		
		#elif LCD_PAR_DISPLAY2
		lcd_display_TRX_status_on_change();			// Display TX/RX transition stuff
		if (Status1 & TX_FLAG)
		{
			lcd_display_P_SWR_V_C_T();				// Display non-static measured values
		}
		#endif
	}
	
	//-------------------------------------------------------------------------------
	// Here we do routines which are to be accessed once every 1/100th of a second (10ms)
	// We have a free running timer which matures once every ~1.05 seconds
	//-------------------------------------------------------------------------------
	Timer1val2 = TCNT1/656; // get current Timer1 value, changeable every ~1/100th sec
	if (Timer1val2 != lastIteration2)				// Once every 1/100th of a second, do stuff
	{
		lastIteration2 = Timer1val2;				// Make ready for next iteration

		#if	BLNK_LOOP_THRU_LED2						// Fun and games. Slowly increase the blink frequency
		static uint8_t period, onoff;
		if (onoff==0) period++;
		if (onoff<period) PORTB = PORTB ^ IO_LED2;	// Blink a led
		onoff++;
		#endif

		#if	BLNK_LOOP_THRU_LEDS						// Fun and games. Slowly increase the blink frequency
		static uint8_t period, onoff;
		if (onoff==0) period++;
		if (onoff<period)
		{
			PORTB = PORTB ^ IO_LED1;				// Blink Led1
			if (!(Status1 & TX_FLAG))
				PORTB = PORTB ^ IO_LED2;			// Blink Led2
			else
				PORTB = PORTB | IO_LED2;			// Led2 on
		}
		onoff++;
		#endif

		#if MED_LOOP_THRU_LED1						// Blink PB2 LED every 10ms, when going through the mainloop 
		PORTB = PORTB ^ IO_LED1;  					// Blink a led
		#endif
		#if MED_LOOP_THRU_LED2						// Blink PB3 LED every 10 ms, when going through the mainloop
		PORTB = PORTB ^ IO_LED2;  					// Blink a led
		#endif

		//
		// Update all ADC readings
		//
		// I2C noise reduction, less or no I2C traffic during RX
		#if SSLO_POLL_DURING_RX ||	SLOW_POLL_DURING_RX	|| NO_I2C_DURING_RX
		if (Status1 & TX_FLAG)
		#endif
		ad7991_poll(R.AD7991_I2C_addr);				// Polls the AD7991 every time (9 bytes)
													// => constant traffic on I2C
		//
		// RD16HHF1 PA Bias management
		//
		PA_bias();									// Autobias and other bias management functions
													// This generates no I2C traffic unless bias change or
													// autobias measurement

		#if	POWER_SWR								// Power/SWR measurements and related actions
		//
		// SWR Protect
		//
		// I2C noise reduction, less or no I2C traffic during RX
		#if NO_I2C_DURING_RX ||	SLOW_POLL_DURING_RX	|| SSLO_POLL_DURING_RX
		if (Status1 & TX_FLAG)
		#endif
		Test_SWR();									// Calculate SWR and control the PTT2 output
													// (SWR protect).  Updates measured_SWR variable (SWR*100)
													// Writes to the PCF8574 every time (2 bytes)
													// => constant traffic on I2C (can be improved to slightly
		#endif										// reduce I2C traffic, at the cost of a few extra bytes)

		//
		// Enact (write) frequency changes resulting from interrupt routine or
		// from the pushbutton memory management routine above
		//
		if (Status2 & ENC_NEWFREQ)					// VFO was turned or freq updated above
		{
			#if ENCODER_INT_STYLE					// Interrupt driven Shaft Encoder
			cli();									// Don't want a rogue interrupt to mess with us
			#endif
			R.Freq[R.SwitchFreq] = R.Freq[0];		// Keep track, move into short term memory
			SetFreq(R.Freq[0]);						// Write the new frequency to Si570
			Status2 &= ~ENC_NEWFREQ;				// and clear flag
			pushcount = 0;							// Clear the push counter for next time
			#if ENCODER_INT_STYLE					// Interrupt driven Shaft Encoder
			sei();
			#endif

			#if LCD_PAR_DISPLAY2
			lcd_display_freq_and_filters();			// Display frequency and filters
			#endif
		}
	}

	wdt_reset();									// Whoops... must remember to reset that running watchdog
}



//
//-----------------------------------------------------------------------------------------
// 			Setup Ports, timers, start the works and never return, unless reset
//								by the watchdog timer
//						then - do everything, all over again
//-----------------------------------------------------------------------------------------
//
int main(void)
{
	MCUSR &= ~(1 << WDRF);							// Disable watchdog if enabled by bootloader/fuses
	wdt_disable();

	clock_prescale_set(clock_div_1); 				// with 16MHz crystal this means CLK=16000000

	// 16-bit Timer1 Initialization
	TCCR1A = 0; //start the timer
	TCCR1B = (1 << CS12); // prescale Timer1 by CLK/256
	// 16000000 Hz / 256 = 62500 ticks per second
	// 16-bit = 2^16 = 65536 maximum ticks for Timer1
	// 65536 / 62500 = ~1.05 seconds
	// so Timer1 will overflow back to 0 about every 1 seconds
	// Timer1val = TCNT1; // get current Timer1 value

	IO_DDR_PTT_CWKEY = IO_LED1 | IO_LED2 | IO_PTT;	// Set pins for output
	IO_PORT_PTT_CWKEY = IO_CWKEY1 | IO_CWKEY2;		// Set pullups for CW key input pins

	#if	FAN_CONTROL									// Turn PA Cooling FAN On/Off, based on temperature
	#if	PORTD_FAN
	IO_DDR_FC = IO_DDR_FC | IO_FC;  				// Set FAN_CONTROL Pin as output
	#else
	Status2 |= COOLING_FAN;							// Ensure proper init the first time
	#endif
	#endif

	//
	// Set run time parameters to Factory default under certain conditions
	//
	// Enforce "Factory default settings" when firmware is run for the very first time after
	// a fresh firmware installation with a new "serial number" in the COLDSTART_REF #define
	// This may be necessary if there is garbage in the EEPROM, preventing startup
	// To activate, roll "COLDSTART_REF" Serial Number in the Mobo.h file
	if (eeprom_read_byte(&E.EEPROM_init_check) != R.EEPROM_init_check)
	{
		if (pcf8574_read(TMP101_I2C_ADDRESS) != 255)// Autosense if TMP101
		{
			R.TMP100_I2C_addr = TMP101_I2C_ADDRESS;	// Then modify TMP100 address
		}

		eeprom_write_block(&R, &E, sizeof(E));		// Initialize eeprom to "factory defaults".
	}
	else
	{
		eeprom_read_block(&R, &E, sizeof(E));		// Load the persistent data from eeprom
	}

	//#if USB_SERIAL_ID								// A feature to change the last char of the USB Serial  number
	// Modify the last byte of the USB descriptor Serial number ("TF3LJ-1.X")
	// This function would need some means to only modify one byte in FLASH ROM,
	// rather than a full page of 128 bytes... something like:
	// WriteFlashByte(&SerialNumberString + offset, R.SerialNumber);
	//#endif

	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
	R.Freq[0] = R.Freq[R.SwitchFreq];				// Fetch last frequency stored
	#endif

	Status2 |= SI570_OFFL;							// Si570 is offline, not initialized

	DeviceInit();									// Initialize the Si570 device.

	#if CHANGE_I2C_LCD_ADDR							// Normally not enabled.  Crude code to change
													// default address of the I2C LCD display
	lcd_i2c_address_init(LCD_I2C_ADDRESS);	
	#endif
	
	// Parallel connected LCD display
	#if (LCD_PAR_DISPLAY || LCD_PAR_DISPLAY2)
   	lcd_init(LCD_DISP_ON);
    #endif

	// Identical routines for I2C and parallel connected displays
	#if (LCD_PAR_DISPLAY || LCD_PAR_DISPLAY2 || LCD_I2C_DISPLAY)
	rprintfInit(lcd_data);							// Init AVRLIB rprintf()
	#if BARGRAPH
	// load the first 8 custom characters
	lcd_bargraph_init();
	#endif
	#endif

	#if	BLNK_LOOP_THRU_LEDS							// Fun and games. Slowly increase the blink frequency
	PORTB = PORTB ^ IO_LED1;						// Change status of Led1
	#endif

	// Don't need this.  Default settings are good enough
	// ad7991_setup(R.AD7991_I2C_addr);
	
	// Start the USB task and "spawn" the three all important functions
	// for the Mobo:
	// void usbFunctionWrite(USB_Notification_Header_t *, unsigned char *, unsigned char);
	// unsigned char usbFunctionSetup(USB_Notification_Header_t *);
	// void maintask(void);

	wdt_enable(WDTO_250MS);							// Start the Watchdog Timer, 250ms

	#if ENCODER_INT_STYLE || ENCODER_SCAN_STYLE		// Shaft Encoder VFO function
													// Two flavours available, interrupt driven, or
													// regular scanning of the Encoder pins
	shaftEncoderInit();								// Init shaft encoder
	#endif

	Initialize_USB();								// Start the works, we're in business
}


// Note that the below 'percentage' statistic is not accurate, since the bootloader already
// occupies 4kBytes of the Program space.  Max available program space is therefore 12288 bytes.
// Similarly, the pgm stack uses at least ~60 bytes of the 512 bytes available for Data.

/* Version 0-9-9: 2009-08-09 - WinAVR20080430 - LUFA090605

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LEGACY_CMD01_04, CALC_FREQ_MUL_ADD, LCD_DISPLAY,
HI_RES_FRQ_READOUT, POWER_SWR, SWR_ALARM_FUNC, DISP_FAHRENHEIT,
ENCODER_INT_STYLE

Program:   12266 bytes (74.9% Full)
(.text + .data + .bootloader)

Data:        437 bytes (85.4% Full)
(.data + .bss + .noinit)

EEPROM:      124 bytes (24.2% Full)
(.eeprom)
*/


/* Version 0-9-9-2: 2009-08-15 - WinAVR20080430 - LUFA090810

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LEGACY_CMD01_04, CALC_FREQ_MUL_ADD, LCD_DISPLAY,
HI_RES_FRQ_READOUT, POWER_SWR, SWR_ALARM_FUNC, DISP_FAHRENHEIT,
ENCODER_INT_STYLE

Program:   11946 bytes (72.9% Full)
(.text + .data + .bootloader)

Data:        425 bytes (83.0% Full)
(.data + .bss + .noinit)

EEPROM:      124 bytes (24.2% Full)
(.eeprom)
*/


/* Version 0-9-9-3: 2009-08-16 - WinAVR20080430 - LUFA090810

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LEGACY_CMD01_04, CALC_FREQ_MUL_ADD, LCD_DISPLAY,
HI_RES_FRQ_READOUT, POWER_SWR, SWR_ALARM_FUNC, DISP_FAHRENHEIT,
ENCODER_INT_STYLE

Program:   11950 bytes (72.9% Full)
(.text + .data + .bootloader)

Data:        425 bytes (83.0% Full)
(.data + .bss + .noinit)

EEPROM:      124 bytes (24.2% Full)
(.eeprom)
*/


/* Version 0-9-9-5: 2009-08-23 - WinAVR20080430 - LUFA090810 - rprintf
-> TX filters expanded to 16x - cleanup, mainly in LCD routines

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, CALC_FREQ_MUL_ADD, LCD_DISPLAY, HI_RES_FRQ_READOUT,
POWER_SWR, SWR_ALARM_FUNC, AUTOSCALE_PWR, DISP_FAHRENHEIT, 
ENCODER_INT_STYLE

Program:   12160 bytes (74.2% Full)
(.text + .data + .bootloader)

Data:        452 bytes (88.3% Full)
(.data + .bss + .noinit)

EEPROM:      149 bytes (29.1% Full)
(.eeprom)
*/


/* Version 0-9-9-6: 2009-08-25 - WinAVR20080430 - LUFA090810 - rprintf
-> Cmd 0x17 bugfix; Various LCD print formatting improvements, including
Temp °C indication; provisions made for alternate LCD print routines.

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, CALC_FREQ_MUL_ADD, LCD_DISPLAY, HI_RES_FRQ_READOUT,
POWER_SWR, SWR_ALARM_FUNC, AUTOSCALE_PWR, DISP_FAHRENHEIT, 
ENCODER_INT_STYLE

Program:   12140 bytes (74.0% Full)
(.text + .data + .bootloader)

Data:        444 bytes (86.7% Full)
(.data + .bss + .noinit)

EEPROM:      149 bytes (29.1% Full)
(.eeprom)
*/


/* Version 0-9-9-7: 2009-08-28 - WinAVR20080430 - LUFA090810 - rprintf
-> Added 16x2 and 20x2 LCD Bargraph display routines and PEP style
Power Output indication (largest value measured within a 1s window).
Also added a low I2C activity during Receive option.

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LCD_PAR_DISPLAY, LCD_DISPLAY_16x2_b, HI_RES_FRQ_READOUT,
POWER_SWR, SWR_ALARM_FUNC, DISP_FAHRENHEIT, PWR_PEAK_ENVELOPE
ENCODER_INT_STYLE, BARGRAPH_STYLE_1, SLOW_POLL_DURING_RX

Program:   12190 bytes (74.4% Full)
(.text + .data + .bootloader)

Data:        455 bytes (88.9% Full)
(.data + .bss + .noinit)

EEPROM:      142 bytes (27.7% Full)
(.eeprom)
*/


/* Version 0-9-9-8: 2009-08-30 - WinAVR20080430 - LUFA090810 - rprintf
-> Minor bugfix to 16x2 and 20x2 LCD Bargraph display routines.
Power calc routine streamlined and calibrated

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LCD_PAR_DISPLAY, LCD_DISPLAY_16x2_b, HI_RES_FRQ_READOUT,
POWER_SWR, SWR_ALARM_FUNC, DISP_FAHRENHEIT, PWR_PEAK_ENVELOPE
ENCODER_INT_STYLE, BARGRAPH_STYLE_1, SLOW_POLL_DURING_RX

Program:   12168 bytes (74.2% Full)
(.text + .data + .bootloader)

Data:        455 bytes (88.9% Full)
(.data + .bss + .noinit)

EEPROM:      142 bytes (27.7% Full)
(.eeprom)
*/


/* Version 0-9-9-9: 2009-09-05 - WinAVR20080430 - LUFA090810 - rprintf
-> Cleanup of LCD display routines.  I2C functions squeezed for bytes.
A 40x2 display added. Several SWR alarm related and PA bias related improvements

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LCD_PAR_DISPLAY, LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT,
POWER_SWR, SWR_ALARM_FUNC, PWR_PEAK_ENVELOPE, DISP_FAHRENHEIT
ENCODER_INT_STYLE, SLOW_POLL_DURING_RX, BARGRAPH_STYLE_1

Program:   12230 bytes (74.6% Full)
(.text + .data + .bootloader)

Data:        413 bytes (80.7% Full)
(.data + .bss + .noinit)

EEPROM:      142 bytes (27.7% Full)
(.eeprom)
*/


/* Version 1.0: 2009-09-08 - WinAVR20080430 - LUFA090810 - rprintf
-> Rotary Encoder function exchanged for a scan style routine, providing
full precision.  Interrupt style Rotary Encoder retained as a user
selectable feature.  Added a user selectable feature, Cmd 0x67, which
provides for user control of Rotary Encoder resolution (normally disabled
due to memory space limitations).  Upper bound of SWR bargraph as well as
PWR bargraph can now be set through Cmd 0x66.  Bounding of extreme values for
power and SWR indication. The address for the second MegaFilterMobo PCF8574 is
now modifiable (had been forgotten in previous code).
Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT,USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, LCD_PAR_DISPLAY, LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT,
POWER_SWR, SWR_ALARM_FUNC, PWR_PEAK_ENVELOPE, DISP_FAHRENHEIT
ENCODER_SCAN_STYLE, SLOW_POLL_DURING_RX, BARGRAPH_STYLE_1

Program:   12270 bytes (74.9% Full)
(.text + .data + .bootloader)

Data:        417 bytes (81.4% Full)
(.data + .bss + .noinit)

EEPROM:      143 bytes (27.9% Full)
(.eeprom)
*/


/* Version 1.01: 2009-09-20 - WinAVR20080430 - LUFA090810 - rprintf
->(Minor update.  Commands 0x18, 19, 1a & 1b made optional,
 Command 0x67 selected as default, Commands 0x6e/6f selected
as default, this may be useful during beta tests.
LPF filter selection available both through PD1-PD4 and by
MegaFilter Mobo style I2C comms
Rough calibration of power meter, diode offset reduced to 100mV.
PEP indication now samples within a 2 second window. 
A few edits in Readme.txt

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT, USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
DDRD_TX_FILTER_IO, I2C_TX_FILTER_IO, POWER_SWR, SWR_ALARM_FUNC, 
SLOW_POLL_DURING_RX, PCF_DIRECT_CONTROL, LCD_PAR_DISPLAY, 
LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT, PWR_PEAK_ENVELOPE, DISP_FAHRENHEIT, 
BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1, ENCODER_SCAN_STYLE, CMD_ENC_INCREMENTS

Program:   12260 bytes (74.8% Full)
(.text + .data + .bootloader)

Data:        416 bytes (81.3% Full)
(.data + .bss + .noinit)

EEPROM:      143 bytes (27.9% Full)
(.eeprom)
*/


/* Version 1.02 2009-10-21 - WinAVR20080430 - LUFA090810 - rprintf
->Minor update.  Command 0x67 replaced with Command 0x36
PEP measurements are now cleared during RX, previously there was
residual window of measurement from last TX.
Command 0x66 expanded with Index 6 for adjustible PEP measurement
period (1 = not PEP and up to 20 for PEP within a 2 second window),
default PEP window is set at 5 (0.5 seconds).
Variable Speed Rotary Encoder VFO function added.
Optional Direction Sense added to the Variable Speed VFO funtion.
I2C queueing removed, not necessary.  Band Sub/Mul function bugs
corrected.  Power Bargraph now works for values higher than 15W
Some alternate display options added for lines 3/4 when using a 20x4
display and similar with 40x2.
Some other minor things...
A few edits in Readme.txt

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)):

FACTORY_DEFAULT, USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
DDRD_TX_FILTER_IO, I2C_TX_FILTER_IO, POWER_SWR, SWR_ALARM_FUNC, 
SLOW_POLL_DURING_RX, FRQ_CGH_DURING_TX, PCF_DIRECT_CONTROL, LCD_PAR_DISPLAY, 
LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT, PWR_PEAK_ENVELOPE, PWR_PEP_ADJUST,
DISP_VERBOSE, BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1,
ENCODER_SCAN_STYLE, ENCODER_CMD_INCR, ENCODER_FAST_ENABLE, ENCODER_DIR_SENSE

Program:   12280 bytes (75.0% Full)
(.text + .data + .bootloader)

Data:        423 bytes (82.6% Full)
(.data + .bss + .noinit)

EEPROM:      124 bytes (24.2% Full)
(.eeprom)
*/


/* Version 1.03 2009-12-30 - WinAVR20080430 - LUFA090810 - rprintf
->Minor update. Command 0x36 expanded (LCD frequency offset during RX to support
accurate frequency readout when using PowerSDR-IQ.
Option for direct control of PCF8574 now disabled by default.
However, it can easily be re-activated.
Default PEP window set back to 10 (1 second window).
Added a new #define to reverse the Rotary Encoder VFO direction.
Code cleanup to free a few hundred bytes (particularly PCF8574 I2C routines).
Option for TX filter controls on the CPU PortD now disabled.  If the user needs
direct control signals for LPF swithcing, then it is better to use the new
PCF_FILTER_IO or M0RZF_FILTER_IO LPF options described below. 
Added a new #define PCF_FILTER_IO to support switching of up to 8 low pass
filters through the 3 previously unused bits on the onboard PCF8574 chip.
Added a new #define M0RZF_FILTER_IO to support low pass filter switching on the
M0RZF 20W ampt hrough the 3 previously unused bits on the onboard PCF8574 chip.
Code for Command 0x43 partially in place, to support modification of
the USB Serial Number descriptor.  However, this is not fully
implemented, due to lack of pgm space (RAM) resources.
Rearrangement of EEPROM memory data structure.
Some edits in Readme.txt

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)
Note that the DISP_VERSION option selected below is wasteful of memory):

FACTORY_DEFAULT, USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, POWER_SWR, SWR_ALARM_FUNC, 
SLOW_POLL_DURING_RX, FRQ_CGH_DURING_TX, PSDR_IQ_OFFSET, LCD_PAR_DISPLAY, 
LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT, PWR_PEAK_ENVELOPE, PWR_PEP_ADJUST,
DISP_VERSION, BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1,
ENCODER_SCAN_STYLE, ENCODER_CMD_INCR, ENCODER_FAST_ENABLE, ENCODER_DIR_SENSE

Program:   12286 bytes (75.0% Full)
(.text + .data + .bootloader)

Data:        433 bytes (84.6% Full)
(.data + .bss + .noinit)

EEPROM:      128 bytes (25.0% Full)
(.eeprom)
*/


/* Version 1.04 2010-02-28 - WinAVR20080430 - LUFA090810 - rprintf
->Minor update. One potential bug correction, to do with SWR measurements at very low power.
SWR is now calculated once the Vforward input value reaches a value of 32/4096 (appr 36mW).
At lower values, the SWR is forced to 1:1

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp C is smaller than Temp F)
Note that the DISP_VERSION option selected below is wasteful of memory):

FACTORY_DEFAULT, USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_TX_FILTER_IO, POWER_SWR, SWR_ALARM_FUNC, 
SLOW_POLL_DURING_RX, FRQ_CGH_DURING_TX, PSDR_IQ_OFFSET, LCD_PAR_DISPLAY, 
LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT, PWR_PEAK_ENVELOPE, PWR_PEP_ADJUST,
DISP_VERSION, BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1,
ENCODER_SCAN_STYLE, ENCODER_CMD_INCR, ENCODER_FAST_ENABLE, ENCODER_DIR_SENSE

Program:   12288 bytes (75.0% Full)
(.text + .data + .bootloader)

Data:        433 bytes (84.6% Full)
(.data + .bss + .noinit)

EEPROM:      128 bytes (25.0% Full)
(.eeprom)
*/


/* Version 1.05 AT90USB162 - 2010-04-30 - WinAVR20080430 - LUFA090810 - rprintf
->Features update.  

Legacy commands 0x15,0x16,0x20 and 0x40 made Optional.

New command 0x67 added, enabling user settable value for the Rotary Encoder Resolution
(resolvable states per revolution).  This command is used to set the correct value for
1kHz per revolution, corresponding dual speed variables are automatically derived.

New Command 68 added, enabling PSDR-IQ RX frequency display offset.  Replaces Cmd
0x36 index 10 and is bidirectional.  

Command 0x36 suppressed (still optional).

Fan Control setpoints added to Command 0x64 (Hi Tmp trigger for Fan On, default
45 deg C, Lo Tmp trigger for Fan Off, default 40 deg C, and FAN bit select).
Three different Fan Control options. PortD Output, builtin PCF8574 output,
or external PCF8574 output.  Also I2C address for PCF Fan control added to
Cmd 0x41, index 7.

TX Filter options tidied up.  Now the preferred option is 8 Filters through an
external PCF8574 (connected over I2C bus).  Other options still retained.
Commands 0x6e and 0x6f, User Write/Read to any external PCF8574 has been
enabled as default.

Mobo-Features.h contains new automatic features selection macros, to simplify
generation of standardized precompiled HEX images.

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp F chosen as tmp C is smaller than
Temp F:

USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_08_FILTER_IO, PCF_WRITE_COMMAND, PCF_READ_COMMAND, POWER_SWR, SWR_ALARM_FUNC, 
SLOW_POLL_DURING_RX, FRQ_CGH_DURING_TX, PSDR_IQ_OFFSET68, FAN_CONTROL, EXTERN_PCF_FAN,
LCD_PAR_DISPLAY, LCD_DISPLAY_20x4_b, HI_RES_FRQ_READOUT, PWR_PEAK_ENVELOPE, PWR_PEP_ADJUST,
DISP_FAHRENHEIT, DISP_VER_SHORT, EXTERNAL_BARGRAPH, BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1,
ENCODER_SCAN_STYLE, ENCODER_RESOLUTION, ENCODER_FAST_ENABLE, ENCODER_DIR_SENSE

Program:   12272 bytes (74.9% Full)
(.text + .data + .bootloader)

Data:        415 bytes (81.1% Full)
(.data + .bss + .noinit)

EEPROM:      111 bytes (21.7% Full)
(.eeprom)

*/

/* Version 1.06 AT90USB162 - 2010-05-29 - WinAVR20080430 - LUFA090810 - rprintf
->
Super slow poll of I2C during RX, once every 10s.

Rewrite of 20x4 and 40x2 LCD routines
for RF noise reduction, no LCD traffic unless update to display is needed.  

Support for 16x2 and 20x2 LCDs dropped, however the old routines are still retained and can
be enabled.

Autosense if TMP101 rather than TMP100.

TMP alarm function disabled if TMP100 absent, output read as signed integer,
255 = -1deg C).

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp F chosen as tmp C is smaller than
Temp F:

USBTRAFFIC_LED1, SLOW_LOOP_THRU_LED2, MOBO_STYLE_IO, 
I2C_08_FILTER_IO, PCF_WRITE_COMMAND, PCF_READ_COMMAND, POWER_SWR, SWR_ALARM_FUNC, 
SSLO_POLL_DURING_RX, FRQ_CGH_DURING_TX, PSDR_IQ_OFFSET68, FAN_CONTROL, EXTERN_PCF_FAN,
LCD_PAR_DISPLAY2, LCD_DISPLAY_20x4_b, PWR_PEAK_ENVELOPE, PWR_PEP_ADJUST,
DISP_FAHRENHEIT, EXTERNAL_BARGRAPH, BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1,
ENCODER_SCAN_STYLE, ENCODER_RESOLUTION, ENCODER_FAST_ENABLE, ENCODER_DIR_SENSE

Program:   12278 bytes (74.9% Full)
(.text + .data + .bootloader)

Data:        412 bytes (80.5% Full)
(.data + .bss + .noinit)

EEPROM:      111 bytes (21.7% Full)
(.eeprom)

*/

/* Version 1.07 AT90USB162 - 2010-05-29 - WinAVR20080430 - LUFA090810 - rprintf
-> Update/expansion of USB commands 0x50 and 0x51

In previous versions, bits 2 and 6 (of 8) in the byte returned by USB commands
0x50 and 0x51 indicated the state of the two CW/PTT inputs (dit and dash).
In this version, these commands have been expanded to also indicate the current
TX/RX state through bit 7 (of 8).  This is useful when using more than one
piece of software concurrently in a co-ordinated manner, such as sdrshell and
fldigi, as set up by Alex.

In order to scrounge for necessary bytes to accomodate this update, the
lcd_init() routine contained in lcd.c was simplified. 

Options (indicative of what can be selected and still fit under the
maximum program memory size of 12288 bytes (Temp F chosen as tmp C is smaller than
Temp F:

USBTRAFFIC_LED1, FADE_LOOP_THRU_LED2, BLNK_LOOP_THRU_LED2, SLOW_LOOP_THRU_LED2, 
LEGACY_CMD01_04, LEGACY_CMD15_16, LEGACY_CMD_20, LEGACY_CMD_40, MOBO_STYLE_IO, 
I2C_08_FILTER_IO, PCF_WRITE_COMMAND, PCF_READ_COMMAND, POWER_SWR, SWR_ALARM_FUNC, 
SSLO_POLL_DURING_RX, FRQ_CGH_DURING_TX, CALC_BAND_MUL_ADD, SCRAMBLED_FILTERS,
PSDR_IQ_OFFSET68, FAN_CONTROL, EXTERN_PCF_FAN,
LCD_PAR_DISPLAY2, LCD_DISPLAY_20x4_b, PWR_PEAK_ENVELOPE, PWR_PEP_ADJUST,
DISP_FAHRENHEIT, DISP_TERSE, EXTERNAL_BARGRAPH, BARGRAPH_SWR_SCALE, BARGRAPH_STYLE_1,
ENCODER_SCAN_STYLE, ENCODER_RESOLUTION, ENCODER_FAST_ENABLE, ENCODER_DIR_SENSE

Program:   12276 bytes (74.9% Full)
(.text + .data + .bootloader)

Data:        412 bytes (80.5% Full)
(.data + .bss + .noinit)

EEPROM:      111 bytes (21.7% Full)
(.eeprom)



*/

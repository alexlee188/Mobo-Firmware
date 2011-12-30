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
//** Initial version.: 2009-08-09, Loftur Jonasson, TF3LJ
//**                   Check the Mobo.c file
//**
//** Interrupt driven Encoder routine updated to double (half) precicion,
//**                   using similar methodology as used in the
//**                   AVRLIB encoder.c.
//**                   2009-09-05, Loftur Jonasson, TF3LJ
//**
//** An alternative polling version of the Encoder routine added as an 
//**                   alternate choice.
//**                   2009-09-08, Loftur Jonasson, TF3LJ
//**
//** Update..........: Variable Speed VFO.  2009-09-22, Loftur Jonasson, TF3LJ
//** Last update.....: User Cmd for Encoder Resolution (resolvable states per revolution)  
//**                   2010-04-11, Loftur Jonasson, TF3LJ
//**
//*********************************************************************************


#include "Mobo.h"

#if ENCODER_INT_STYLE			// Interrupt driven Shaft Encoder

#include <avr/io.h>
#include <avr/interrupt.h>

//
// Init Encoder and Interrupt for use
//
void shaftEncoderInit(void)
{
	//
	// enable interrupts & set inputs with pullup
	//
    //ENC_A_DDR &= ~(ENC_A_PIN | ENC_B_PIN);
	//ENC_A_PORT |= (ENC_A_PIN | ENC_B_PIN);
    // Better to do it this way to get more freedom of assigning the Phase B
	// pin to any port (costs a few more bytes:
	ENC_A_DDR &= ~ENC_A_PIN;	// Enable pin for input
	ENC_A_PORT |= ENC_A_PIN;	// Activate internal pullup resistor
    ENC_B_DDR &= ~ENC_B_PIN;
	ENC_B_PORT |= ENC_B_PIN;
	ENC_PUSHB_DDR |= ENC_PUSHB_PIN;
	ENC_PUSHB_PORT |= ENC_PUSHB_PIN;

	// Configure interrupts for any edge
	ENC_A_ICR |= (ENC_A_ISCX0 & ~ENC_A_ISCX1);

	// Enable interrupt vector
	ENC_A_IREG |= ENC_A_INT;

	sei();
}

//
// Shaft Encoder interrupt handler
// PCINT could be used as a substitute for INT.  Would need a revision of
// the Interrupt init above.
//
ISR(ENC_A_SIGNAL)
{
	int8_t	increment = 0;						// This interim variable used to add up changes
												// rather than enacting them directly on the
												// uint32_t variable which contains the frequency
												// this saves a bunch of bytes

	#if ENCODER_FAST_ENABLE						// Variable speed Rotary Encoder feature
	uint16_t		enc_time;					// Read and use time from TCNT1
	static uint16_t	enc_last;					// Measure the time elapsed since the last encoder pulse
	static uint8_t	fast_counter=0;				// Number of rapid movements in succession
	#if ENCODER_DIR_SENSE						// Direction change sense.  Used with variable speed feature.
	static int8_t	direction;					// Direction of last encoder pulse
	#endif
	#endif

	#if ENCODER_RESOLUTION
	uint16_t enc_fast_sense = ENC_FAST_SENSE;
	uint16_t enc_fast_trig = ENC_FAST_TRIG;
	#if !ENCODER_CMD_INCR
	uint16_t encoder_resolution = 8388/ENC_PULSES;
	encoder_resolution = 8388/R.Resolvable_States;
	enc_fast_sense = 96000/R.Resolvable_States;
	enc_fast_trig = R.Resolvable_States/5;
	#else
	if(!(Status2 & ENC_RES))					// If resolution has not been modified by Cmd 0x36
	{
		#if ENCODER_CMD_INCR
		R.Encoder_Resolution = 8388/R.Resolvable_States;
		#else
		encoder_resolution = 8388/R.Resolvable_States;
		#endif
		enc_fast_sense = 96000/R.Resolvable_States;
		enc_fast_trig = R.Resolvable_States/5;
	}
	#endif
	#endif

	// encoder has generated a pulse
	// check the relative phase of the input channels
	// and update position accordingly
	if(((ENC_A_PORTIN & ENC_A_PIN) == 0) ^ ((ENC_B_PORTIN & ENC_B_PIN) == 0))
	{
		increment++;							// Increment
	}
	else
	{
		increment--;							// Decrement
	}

	#if ENCODER_FAST_ENABLE						// Feature for variable speed Rotary Encoder
	//
	// Variable speed function
	//

	// Measure the time since last encoder activity in units of appr 1/65536 seconds
	enc_time=TCNT1;
	if (enc_last > enc_time); 					// Timer overrun, it is code efficient to do nothing
	#if ENCODER_RESOLUTION
	else if (enc_fast_sense >= (enc_time-enc_last)) fast_counter++;// Fast movement detected
	#else
	else if (ENC_FAST_SENSE >= (enc_time-enc_last)) fast_counter++;// Fast movement detected
	#endif
	else fast_counter = 0;						// Slow movement, reset counter
	enc_last = enc_time;						// Store for next time measurement

	// We have the required number of fast movements, enable FAST mode
	#if ENCODER_RESOLUTION
	if (fast_counter > enc_fast_trig) Status2 |= ENC_FAST;
	#else
	if (fast_counter > ENC_FAST_TRIG) Status2 |= ENC_FAST;
	#endif	
		#if ENCODER_DIR_SENSE					// Direction change sense.  Used with variable speed feature.
		// If direction has changed, force a drop out of FAST mode
		if (direction != increment)
		{
			if (Status2 & ENC_DIR)	Status2 &= ~ENC_DIR;// Previous change was just a one shot event, clear flag
			else Status2 |= ENC_DIR;					// This is the first event in a new direction, set flag
		}
		else if (Status2 & ENC_DIR)				// Second shot with different direction, 
		{										// now we take action and drop out of fast mode

			Status2 = Status2 & ~ENC_FAST & ~ENC_DIR;
		}
		
		direction = increment;					// Save encoder direction for next time
		#endif

	// When fast mode, multiply the increments by the set MULTIPLY factor
	if (Status2 & ENC_FAST)	increment = increment * ENC_FAST_MULTIPLY;

	// This one could just as well be outside of the parental if sentence
	#if ENCODER_CMD_INCR						// USB Command to modify Encoder Resolution
	R.Freq[0] += increment*R.Encoder_Resolution;// Add or subtract VFO frequency
	#elif ENCODER_RESOLUTION
	R.Freq[0] += (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
	#else
	R.Freq[0] += increment*ENC_INCREMENTS;		// Add or subtract VFO frequency
	#endif

	#else										// Not Variable speed function
	#if ENCODER_CMD_INCR						// USB Command to modify Encoder Resolution
	#if	ENCODER_DIR_REVERSE
	R.Freq[0] -= increment*R.Encoder_Resolution;// Add or subtract VFO frequency
	#else
	R.Freq[0] += increment*R.Encoder_Resolution;// Add or subtract VFO frequency
	#endif
	#elif ENCODER_RESOLUTION
	#if	ENCODER_DIR_REVERSE
	R.Freq[0] -= (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
	#else
	R.Freq[0] += (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
	#endif
	#else
	#if	ENCODER_DIR_REVERSE
	R.Freq[0] -= increment*ENC_INCREMENTS;		// Add or subtract VFO frequency
	#else
	R.Freq[0] += increment*ENC_INCREMENTS;		// Add or subtract VFO frequency
	#endif
	#endif
	#endif

	Status2 |= ENC_NEWFREQ;						// Frequency was modified
	
}
#endif//ENCODER_INT_STYLE						// Interrupt driven Shaft Encoder

//***********************************************************************************

#if ENCODER_SCAN_STYLE							// Shaft Encoder which scans the  the GPIO inputs
//
// Init Encoder for use
//
void shaftEncoderInit(void)
{
	//
	// Set inputs with pullup
	//
	ENC_A_DDR &= ~ENC_A_PIN;					// Enable pin for input
	ENC_A_PORT |= ENC_A_PIN;					// Activate internal pullup resistor
    ENC_B_DDR &= ~ENC_B_PIN;
	ENC_B_PORT |= ENC_B_PIN;
}


//
// Scan the Shaft Encoder
//
void encoder_scan(void)
{
	static uint8_t old_pha, old_phb;			// Variables conaining the previous encoder states

	uint8_t pha = 0, phb= 0;					// Variables containing the current encoder states

	int8_t	increment = 0;						// This interim variable used to add up changes
												// rather than enacting them directly on the
												// uint32_t variable which contains the frequency
												// this saves a bunch of bytes

	#if ENCODER_FAST_ENABLE						// Variable speed Rotary Encoder feature
	uint16_t		enc_time;					// Read and use time from TCNT1
	static uint16_t	enc_last;					// Measure the time elapsed since the last encoder pulse
	static uint8_t	fast_counter=0;				// Number of rapid movements in succession
	#if ENCODER_DIR_SENSE						// Direction change sense.  Used with variable speed feature.
	static int8_t	direction;					// Direction of last encoder pulse
	#endif
	#endif

	#if ENCODER_RESOLUTION
	uint16_t enc_fast_sense = ENC_FAST_SENSE;
	uint16_t enc_fast_trig = ENC_FAST_TRIG;
	#if !ENCODER_CMD_INCR
	uint16_t encoder_resolution;
	encoder_resolution = 8388/R.Resolvable_States;
	enc_fast_sense = 96000/R.Resolvable_States;
	enc_fast_trig = R.Resolvable_States/5;
	#else
	if(!(Status2 & ENC_RES))					// If resolution has not been modified by Cmd 0x36
	{
		#if ENCODER_CMD_INCR
		R.Encoder_Resolution = 8388/R.Resolvable_States;
		#else
		encoder_resolution = 8388/R.Resolvable_States;
		#endif
		enc_fast_sense = 96000/R.Resolvable_States;
		enc_fast_trig = R.Resolvable_States/5;
	}
	#endif
	#endif

	if (ENC_A_PORTIN & ENC_A_PIN) pha++;		// Read Phase A
	if (ENC_B_PORTIN & ENC_B_PIN) phb++;		// Read Phase B

	if ((pha != old_pha) && (phb != old_phb));	// Both states have changed, invalid
	
	else if (pha != old_pha)					// State of Phase A has changed
	{
		old_pha = pha;							// Store for next iteration

		if(old_pha != old_phb)					// Decide direction and
			increment++;						// increment
		else
			increment--;						// or decrement
		Status2 |= (ENC_NEWFREQ | ENC_CHANGE);	// Frequency was modified
	}
	
	else if (phb != old_phb)					// State of Phase B has changed
	{
		old_phb = phb;							// Store for next iteration

		if(old_pha != old_phb)					// Decide direction and
			increment --;						// decrement
		else
			increment ++;						// or increment
		Status2 |= (ENC_NEWFREQ | ENC_CHANGE);	// Frequency was modified
	}

	#if ENCODER_FAST_ENABLE						// Feature for variable speed Rotary Encoder
	//
	// Variable speed function
	//
	if (Status2 & ENC_CHANGE)					// If encoder activity
	{
		Status2 &= ~ENC_CHANGE;					// Clear activity flag

		// Measure the time since last encoder activity in units of appr 1/65536 seconds
		enc_time=TCNT1;
		if (enc_last > enc_time); 				// Timer overrun, it is code efficient to do nothing
		#if ENCODER_RESOLUTION
		else if (enc_fast_sense >= (enc_time-enc_last)) fast_counter++;// Fast movement detected
		#else
		else if (ENC_FAST_SENSE >= (enc_time-enc_last)) fast_counter++;// Fast movement detected
		#endif
		else fast_counter = 0;					// Slow movement, reset counter
		enc_last = enc_time;					// Store for next time measurement

		// We have the required number of fast movements, enable FAST mode
		#if ENCODER_RESOLUTION
		if (fast_counter > enc_fast_trig) Status2 |= ENC_FAST;
		#else
		if (fast_counter > ENC_FAST_TRIG) Status2 |= ENC_FAST;
		#endif
		#if ENCODER_DIR_SENSE					// Direction change sense.  Used with variable speed feature.
		// If direction has changed, force a drop out of FAST mode
		if (direction != increment)
		{
			if (Status2 & ENC_DIR)	Status2 &= ~ENC_DIR;// Previous change was just a one shot event, clear flag
			else Status2 |= ENC_DIR;					// This is the first event in a new direction, set flag
		}
		else if (Status2 & ENC_DIR)				// Second shot with different direction, 
		{										// now we take action and drop out of fast mode

			Status2 = Status2 & ~ENC_FAST & ~ENC_DIR;
		}
		
		direction = increment;					// Save encoder direction for next time
		#endif

		// When fast mode, multiply the increments by the set MULTIPLY factor
		if (Status2 & ENC_FAST)	increment = increment * ENC_FAST_MULTIPLY;

		// This one could just as well be outside of the parental if sentence
		#if ENCODER_CMD_INCR					// USB Command to modify Encoder Resolution
		// This one could just as well be outside of the parental if sentence
		#if	ENCODER_DIR_REVERSE
		R.Freq[0] -= increment*R.Encoder_Resolution;// Add or subtract VFO frequency
		#else
		R.Freq[0] += increment*R.Encoder_Resolution;// Add or subtract VFO frequency
		#endif

		#elif ENCODER_RESOLUTION
		#if	ENCODER_DIR_REVERSE
		R.Freq[0] -= (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
		#else
		R.Freq[0] += (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
		#endif
		#else
		#if	ENCODER_DIR_REVERSE
		R.Freq[0] -= increment*ENC_INCREMENTS;	// Add or subtract VFO frequency
		#else
		R.Freq[0] += increment*ENC_INCREMENTS;	// Add or subtract VFO frequency
		#endif
		#endif
	}
	#else
	#if ENCODER_CMD_INCR						// USB Command to modify Encoder Resolution
	#if	ENCODER_DIR_REVERSE
	R.Freq[0] -= increment*R.Encoder_Resolution;// Add or subtract VFO frequency
	#else
	R.Freq[0] += increment*R.Encoder_Resolution;// Add or subtract VFO frequency
	#endif
	#elif ENCODER_RESOLUTION
	#if	ENCODER_DIR_REVERSE
	R.Freq[0] -= (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
	#else
	R.Freq[0] += (int32_t) increment*encoder_resolution;// Add or subtract VFO frequency
	#endif
	#else
	#if	ENCODER_DIR_REVERSE
	R.Freq[0] -= increment*ENC_INCREMENTS;		// Add or subtract VFO frequency
	#else
	R.Freq[0] += increment*ENC_INCREMENTS;		// Add or subtract VFO frequency
	#endif
	#endif
	#endif

}
#endif//ENCODER_SCAN_STYLE						// Shaft Encoder which scans the inputs

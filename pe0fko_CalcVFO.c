//************************************************************************
//**
//** Project......: Firmware USB AVR Si570 controller.
//**
//** Platform.....: ATtiny45
//**
//** Licence......: This software is freely available for non-commercial 
//**                use - i.e. for research and experimentation only!
//**
//** Programmer...: F.W. Krom, PE0FKO
//** 
//** Description..: Calculations the LO frequency with the offset and
//**                multiply factor. "LO = ( F - offset ) * multiply".
//**                If the offset is bigger than the frequency there will
//**                be no subtraction to prevent the Si570.
//**
//** History......: Check the main.c file
//**
//**				Some minor modificatons.  
//**				Multiband selectable Multiply/Subtract values
//**
//**					2009-07-26, Loftur E. Jonasson, TF3LJ
//**
//**				PE0FKO bugfix for negative subtract values, 2009-07-27
//**
//**************************************************************************

//#include "main.h"
#include "Mobo.h"
#if CALC_FREQ_MUL_ADD | CALC_BAND_MUL_ADD	// Frequency Subtract and Multiply Routines (for smart VFO)
// LO    = (freq - offset) * multiply
// 22.42 =  --- 11.21 ---  * 11.21

//	Register:
//	    D0        C0        B0        A0        D1        C1        B1        A1
//	    7         6         5         4         3         2         1         0
//	xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx 
//	0000.0000 000-.---- ----.--== ====.==== ====.==== ====.==== ====.==== ====.==== 
//	----.---- ---= ==== ====.==== ====.====[====.==== ====.==== ====.=]
//               0 9876 5432 10-1 2345 6789 0123 4567 8901
//  0987.6543 2101.2345 6789.0123 4567.8901

#if CALC_BAND_MUL_ADD		// Band dependent Frequency Subtract and Multiply
uint32_t CalcFreqMulAdd(uint32_t iFreq, uint32_t Sub, uint32_t Mul)
#else
uint32_t CalcFreqMulAdd(uint32_t iFreq)
#endif
{
	uint32_t	oFreq = 0;
	uint8_t		cnt = 32+1;

	// Multiply 64bits = 32bits * 32bits
	asm volatile (
	"sub %A1,%A2			\n\t"	// Subtrac the offset from the Frequency
	"sbc %B1,%B2			\n\t"	// iFreq -= R.FreqSub
	"sbc %C1,%C2			\n\t"
	"sbc %D1,%D2			\n\t"
//	 ---- BUG in V15.10 ----
//	"brcc L_X_%=			\n\t"	// if iFreq is negative

//	"add %A1,%A2			\n\t"	// then
//	"adc %B1,%B2			\n\t"	//   iFreq += R.FreqSub
//	"adc %C1,%C2			\n\t"	// Prevent a negative number
//	"adc %D1,%D2			\n\t"
//	 ---- BUG in V15.10 ----

"L_X_%=:					\n\t"
	"clc					\n\t"	// oFreq:iFreq *= R.FreqMul

"L_A_%=:					\n\t"
	"brcc L_B_%=			\n\t"

	"add %A0,%A3			\n\t"
	"adc %B0,%B3			\n\t"
	"adc %C0,%C3			\n\t"
	"adc %D0,%D3			\n\t"

"L_B_%=:					\n\t"
	"ror %D0				\n\t"
	"ror %C0				\n\t"
	"ror %B0				\n\t"
	"ror %A0				\n\t"

	"ror %D1				\n\t"
	"ror %C1				\n\t"
	"ror %B1				\n\t"
	"ror %A1				\n\t"

	"dec %4					\n\t"
	"brne L_A_%=			\n\t"

	"ldi %4,8+3				\n\t"	// Move 32bits to high dword is oFreq.
"L_C_%=:					\n\t"
	"lsl %C1				\n\t"
	"rol %D1				\n\t"
	"rol %A0				\n\t"
	"rol %B0				\n\t"
	"rol %C0				\n\t"
	"rol %D0				\n\t"
	"dec %4					\n\t"
	"brne L_C_%=			\n\t"

	// Output operand list
	//--------------------
	: "=r" (oFreq)			// %0
	, "=r" (iFreq)			// %1

	// Input operand list
	//-------------------
	#if CALC_BAND_MUL_ADD	// Band dependent Frequency Subtract and Multiply
	: "r" (Sub)				// %2	First Offset subtract
	, "r" (Mul)				// %3	Then Frequency multiply
	#else
	: "r" (R.FreqSub)		// %2	First Offset subtract
	, "r" (R.FreqMul)		// %3	Then Frequency multiply
	#endif
	, "r" (cnt)				// %4	Loop counter
	, "0" (oFreq)			// %0
	, "1" (iFreq)			// %1
	);

	return oFreq;
}

#endif

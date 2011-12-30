//************************************************************************
//**
//** Project......: Firmware USB AVR Si570 controler.
//**
//** Platform.....: ATtiny45
//**
//** Licence......: This software is freely available for non-commercial 
//**                use - i.e. for research and experimentation only!
//**                Copyright: (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH
//**                Based on ObDev's AVR USB driver by Christian Starkjohann
//**
//** Programmer...: F.W. Krom, PE0FKO
//**                I like to thank Francis Dupont, F6HSI for some usefull comment!
//** 
//** Description..: Calculations the frequency from the Si570 register value.
//**                The xtal 114.285 will be used as default because that is also
//**                used in the application.
//**
//** History......: V15.1 02/12/2008: First release of PE0FKO.
//**                Check the main.c file
//**
//**************************************************************************

//#include "main.h"
#include "Mobo.h"

static
void
CalcFreqFromRegSi570(uint8_t* reg)
{
	// As I think about the problem:
	//  Freq  = xtal * RFREQ / N
	//  Freq  = 114.285 * RFREQ / N
	//  20.52 = 8.24    * 12.28 / 16.0
	//  20.52 = 20.52 / 16.0
	//  19.53 = (20.52 << 1) / 16.0
	//  19.53 --> 19.21	(remove 4 lowest bytes)
	//  19.21 = 19.21 / 16.0
	//  19.21 --> 11.21  (remove 1 high byte)
	//  xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx xxxx.xxxx 
	//      8         7         6         5         4         3         2         1         0
	//      A3        A2        A1        A0        B4        B3        B2        B1        B0
	//  9876 5432 1098 7654 3210|0123 4567 8901 2345 6789 0
	//            1098 7654 3210|0123 4567 8901 2345 6789 0

	// Or as Francis Dupont explain my comment :-)
	//  I have got also some trouble with this Si570CalcFreq, but after some
	//  rewriting, here is what I have understood:
	//  We need: Freq = F_DCO/N  = xtal*RFREQ /N
	//  with xtal [8.24] , RFREQ [12.28] and N [16.0]
	//  F_DCO = xtal*RFREQ is [20.52] (in A3, A2, A1, A0, B4, B3, B2, B1, B0)
	//  Then F_DCO <<1 is [19.53]
	//  as we don't need such resolution we can discard the 4 lower bytes.
	//  F_DCO is now [19.21] (in A3, A2, A1, A0, B4), so:
	//  Freq = F_DCO/N is also [19.21], but the first 8 bits are
	//  always 0, ignore them -> Freq is [11.21] in (A2, A1, A0, B4).

	uint8_t		cnt;
	uint8_t		A0,A1,A2,A3,B0,B1,B2,B3,B4;
	uint8_t		N1,HS_DIV;
	uint16_t	N;
//	sint32_t	Freq;

	HS_DIV = (reg[0] >> 5) & 0x07;
	N1 = ((reg[0] << 2) & 0x7C) | ((reg[1] >> 6) & 0x03);

	N1 = N1 + 1;
	HS_DIV = HS_DIV + 4;
	N = HS_DIV * N1;

	A0 = 0;
	A1 = 0;
	A2 = 0;
	A3 = 0;
	B0 = reg[5];
	B1 = reg[4];
	B2 = reg[3];
	B3 = reg[2];
	B4 = reg[1] & 0x3F;

	cnt = 40+1;

	asm volatile (
	"clc					\n\t"

"L_A_%=:					\n\t"		// do {
	"brcc L_B_%=			\n\t"		//   if (C)

	"add %0,%A10			\n\t"		//     A += FreqXtal
	"adc %1,%B10			\n\t"
	"adc %2,%C10			\n\t"
	"adc %3,%D10			\n\t"

"L_B_%=:					\n\t"		//   C -> A:B -> C
	"ror %3					\n\t"
	"ror %2					\n\t"
	"ror %1					\n\t"
	"ror %0					\n\t"

	"ror %8					\n\t"
	"ror %7					\n\t"
	"ror %6					\n\t"
	"ror %5					\n\t"
	"ror %4					\n\t"

	"dec %9					\n\t"		// } while(--cnt != 0);
	"brne L_A_%=			\n\t"

	// Shift comma one place left, so whe get a
	// byte boundery for tha actual number.
	"lsl %7					\n\t"		// A[3-0]:B[4-3] << 1
	"rol %8					\n\t"
	"rol %0					\n\t"
	"rol %1					\n\t"
	"rol %2					\n\t"
	"rol %3					\n\t"

	// Output operand list
	//--------------------
	: "=r" (A0)				// %0		A[3-0]:B[4-0] = A[3-0] * B[4-0]
	, "=r" (A1)				// %1
	, "=r" (A2)				// %2
	, "=r" (A3)				// %3
	, "=r" (B0)				// %4
	, "=r" (B1)				// %5
	, "=r" (B2)				// %6
	, "=r" (B3)				// %7
	, "=r" (B4)				// %8
	, "=r" (cnt)			// %9		Loop counter

	// Input operand list
	//-------------------
//	: "r" (R.FreqXtal)		// %10		FreqXtal
	: "r" (DEVICE_XTAL)		// %10		114.285 * _2(24)
	, "0" (A0)				// 
	, "1" (A1)				// 
	, "2" (A2)				// 
	, "3" (A3)				// 
	, "4" (B0)				// 			RFREQ[0]
	, "5" (B1)				// 			RFREQ[1]
	, "6" (B2)				// 			RFREQ[2]
	, "7" (B3)				// 			RFREQ[3]
	, "8" (B4)				// 			RFREQ[4]
	, "9" (cnt)				// 			Loop counter
	);

	// A[3-0]:B[4] = A[3-0]:B[4] / N[1-0]
	// B[1-0] is reminder

	B0 = 0;			// Reminder = 0
	B1 = 0;

	cnt = 40+1;

	asm volatile (
	"clc					\n\t"

"L_A_%=:					\n\t"		// do {
	"rol %2					\n\t"		//   C <- A[3-0]:B[4] <- C
	"rol %3					\n\t"
	"rol %4					\n\t"
	"rol %5					\n\t"
	"rol %6					\n\t"

	"rol %0					\n\t"		//   C <- Remainder <- C
	"rol %1					\n\t"

	"sub %0,%A8				\n\t"		//   Remainder =  Remainder - Divisor
	"sbc %1,%B8				\n\t"
	
	"brcc L_B_%=			\n\t"		//   If result negative
										//   Then
	"add %0,%A8				\n\t"		//     Restore Remainder
	"adc %1,%B8				\n\t"

	"clc					\n\t"		//     Partial_result = 0
	"rjmp L_C_%=			\n\t"

"L_B_%=:					\n\t"		//   Else
	"sec					\n\t"		//     Partial_result = 1

"L_C_%=:					\n\t"		//   End If
	"dec %7					\n\t"		// } while(--cnt == 0)
	"brne L_A_%=			\n\t"

"L_X_%=:					\n\t"
	// Output operand list
	//--------------------
	: "=r" (B0)				// %0		Reminder
	, "=r" (B1)				// %1		Reminder
//	, "=r" (Freq.b0)		// %2		Frequency
//	, "=r" (Freq.b1)		// %3		....
//	, "=r" (Freq.b2)		// %4		....
//	, "=r" (Freq.b3)		// %5		....
	, "=r" (reg[0])			// %2		Frequency return in reg[3..0]
	, "=r" (reg[1])			// %3		....
	, "=r" (reg[2])			// %4		....
	, "=r" (reg[3])			// %5		....
	, "=r" (A3)				// %6		Not used
	, "=r" (cnt)			// %7		Loop counter

	// Input operand list
	//-------------------
	: "r" (N)				// %8		Divisor_16
	, "0" (B0)				// 			Reminder_16
	, "1" (B1)				// 			Reminder_16
	, "2" (B4)				// 			A[3-0]:B[4] = A[3-0]:B[4] / N[1-0]
	, "3" (A0)				// 			....
	, "4" (A1)				// 			....
	, "5" (A2)				// 			....
	, "6" (A3)				// 			....
	, "7" (cnt)				// 			Loop counter
	);

//	SetFreq(Freq.dw, R.Si570_PPM != 0);
}


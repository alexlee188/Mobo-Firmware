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
//** Programmer...: F.W. Krom, PE0FKO and
//**                thanks to Tom Baier DG8SAQ for the initial program.
//** 
//** Description..: I2C Protocol, trying to simulate a open Collector output.
//**
//** History......: V15.1 02/12/2008: First release of PE0FKO.
//**                Check the main.c file
//**
//**************************************************************************

//#include "main.h"
#include "Mobo.h"

#define SDA					(1<<BIT_SDA)
#define SCL					(1<<BIT_SCL)
#define I2C_SDA_LO			I2C_DDR |= SDA
#define I2C_SDA_HI			I2C_DDR &= ~SDA
#define I2C_SCL_LO			I2C_DDR |= SCL
#define I2C_SCL_HI			I2C_DDR &= ~SCL

#define	I2C_DELAY_uS		(1000.0 / I2C_KBITRATE)

static void 
I2CDelay(void)
{
	_delay_us(I2C_DELAY_uS);
}

//PE0FKO: The original code has no stop condition (hang on SCL low)
//static 
void 
//I2CStretch()							// Wait until clock hi
I2CStretch(void)							// Wait until clock hi
{										// Terminate the loop @ max 2.1ms
	uint16_t i = 50;					// 2.1mS
	do {
		I2CDelay();						// Delay some time
		if (i-- == 0)
		{
			I2CErrors = True;			// Error timeout
			break;
		}
	}
	while(!(I2C_PIN & SCL));			// Clock line still low
}

/*
 * Generates a start condition on the bus.
 *
 *	SDA: ..\____..
 *	       __
 *	SCL: ..  \__.. 
 */
void 
I2CSendStart(void)
{
	I2CErrors = False;					// reset error flag
	I2C_SCL_HI;
	I2C_SDA_LO;  	I2CDelay(); 		// Start SDA to low
	I2C_SCL_LO;  	I2CDelay();			// and the clock low
}

/*
 * Generates a stop condition on the bus.
 *	           __
 *	SDA: ..___/  ..
 *	        _____
 *	SCL: ../     .. 
 */
void 
I2CSendStop(void)
{
	I2C_SDA_LO;
	I2C_SCL_HI;		I2CDelay();
	I2C_SDA_HI;		I2CDelay();
}

void 
I2CSend0(void)
{
	I2C_SDA_LO;							// Data low = 0
	I2C_SCL_HI;		I2CStretch();
	I2C_SCL_LO;		I2CDelay();
}

void 
I2CSend1(void)
{
	I2C_SDA_HI;							// Data high = 1
	I2C_SCL_HI;		I2CStretch();
	I2C_SCL_LO;		I2CDelay();
}

static uint8_t
I2CGetBit(void)
{
	uint8_t b;
	I2C_SDA_HI;							// Data high = input (opencollector)
	I2C_SCL_HI;		I2CStretch();		// SDA Hi Z and wait
	b = (I2C_PIN & SDA);				// get bit
	I2C_SCL_LO;							// clock low
	return b;
}

void
I2CSendByte(uint8_t b)
{
	uint8_t i,p;
	p = 0x80;
    for (i=0; i<8; i++)
	{
		if ((p & b) == 0) I2CSend0(); else I2CSend1();
    	p = p >> 1;
	};
    I2CErrors |= I2CGetBit();	 		//Acknowledge
  	return; 
}

uint8_t
I2CReceiveByte(void)
{
	uint8_t i;
	uint8_t b = 0;
    for (i=0; i<8; i++)
	{
		b = b << 1;
		if (I2CGetBit()) b |= 1;
  	};
  	return b;
}



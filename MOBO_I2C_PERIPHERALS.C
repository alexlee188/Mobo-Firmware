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
//** Initial version.: 2009-09-08, Loftur Jonasson, TF3LJ
//** Last update.....: 2010-04-30, Loftur Jonasson, TF3LJ
//**                   Check the Mobo.c file
//**
//*********************************************************************************


#include "Mobo.h"


//
//-----------------------------------------------------------------------------------------
// 						Set or Clear bits in the builtin Mobo PCF8574 output
//					(saves pgm space, as these actions are performed a number of times)
//-----------------------------------------------------------------------------------------
//
void MoboPCF_set(uint8_t byte)
{
	// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
	pcf_data_out = pcf_data_out | byte;				// Set bits
	pcf8574_byte(R.PCF_I2C_Mobo_addr, pcf_data_out);// Write out to Mobo PCF8574
}

void MoboPCF_clear(uint8_t byte)
{
	// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
	pcf_data_out = pcf_data_out & ~byte;			// Clear bits
	pcf8574_byte(R.PCF_I2C_Mobo_addr, pcf_data_out);// Write out to Mobo PCF8574
}



//
//-----------------------------------------------------------------------------------------
// 						Rudimentary I2C Queuing
//-----------------------------------------------------------------------------------------
//
//void i2c_queue(void)
//{
//	while(Status1 & I2C_BUSY);			// Wait for I2C port to become free
//	Status1 |= I2C_BUSY;				// Indicate the I2C port is busy
//}
//
//void i2c_release(void)
//{
//	Status1 &= ~I2C_BUSY;				// Indicate the I2C port is clear
//}



//
//-----------------------------------------------------------------------------------------
// 						PCF8574 write out all 8 bits at once
//-----------------------------------------------------------------------------------------
//
// This function writes all 8 bits at once, nothing stored 
void pcf8574_byte(uint8_t i2c_address, uint8_t data)
{
	//i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte(i2c_address<<1);		// Send address of device
	I2CSendByte(data);					// Send a byte
	I2CSendStop();						// Stop the I2C comms

	//i2c_release();						// Release I2C port
}



//
//-----------------------------------------------------------------------------------------
// 							PCF8574 Read 8 bits
//-----------------------------------------------------------------------------------------
//
// This function reads all 8 input bits from the PCF8574
uint8_t pcf8574_read(uint8_t i2c_address)
{
	uint8_t data_received;

	//i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte((i2c_address<<1)|0x01);	// Send "receive" address of device
	data_received = I2CReceiveByte();	// Receve a byte
	I2CSend1();							// Last byte
	I2CSendStop();						// Stop the I2C comms

	//i2c_release();						// Release I2C port

	return data_received;
}



//
//-----------------------------------------------------------------------------------------
// 							Read temperature from TMP100 device
//-----------------------------------------------------------------------------------------
//
// Returned temperature reading can be converted to degrees C, by using the formula:
// temp = 128.0 / 32768 * tmp100().i;
//
void tmp100(uint8_t i2c_address)
{
	//i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte((i2c_address<<1)|0x01);	// Send "receive" address of device

	tmp100_data.b1 = I2CReceiveByte();	// Receve high byte
	I2CSend0();
	tmp100_data.b0 = I2CReceiveByte();	// Receve low byte
	I2CSend1();							// Last byte
	I2CSendStop();						// Stop the I2C comms

	//i2c_release();						// Release I2C port
}



//
//-----------------------------------------------------------------------------------------
// 							Write data to the AD5301 DAC
//-----------------------------------------------------------------------------------------
//
void ad5301(uint8_t i2c_address, uint8_t value)
{
	unsigned data1_out, data2_out;

	data1_out = (value & 0xf0)>>4;		// Move upper 4 bits into lower 4 bits
	data2_out = (value & 0x0f)<<4;		// Move lower 4 bits into upper 4 bits

	//i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte(i2c_address<<1);		// Send address of device
	I2CSendByte(data1_out);				// Send a byte
	I2CSendByte(data2_out);				// Send a byte
	I2CSendStop();						// Stop the I2C comms

	//i2c_release();						// Release I2C port
}



//
//-----------------------------------------------------------------------------------------
// 							Setup AD7991 to do interesting stuff
//-----------------------------------------------------------------------------------------
//
// Looks like this is not needed.
/*
void ad7991_setup(uint8_t i2c_address)
{
	i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte(i2c_address<<1);		// Send address of device
	I2CSendByte(0xf0);					// Setup Reg.  0xf0 is default setup
										// meaning that all 4 A/Ds are polled,
										// Vref = Vdd etc...

	I2CSendStop();						// Stop the I2C comms

	i2c_release();						// Release I2C port
}	
*/



//
//-----------------------------------------------------------------------------------------
// 							Poll the AD7991 4 x ADC chip
//-----------------------------------------------------------------------------------------
//
// This function reads all four A/D inputs and makes the data available in four
// global variables, ad7991_adc[4], set up in Mobo.c
void ad7991_poll(uint8_t i2c_address)
{
	//
	// This clean and "pretty" version is 24 bytes larger than the uglier version below
	//
	/*
	union {
		struct {
			uint8_t		b_nil;			// adjustment padding
			uint8_t		b0;				// lsb read from AD7991
			uint8_t		b1;				// msb read from AD7991
		};
		struct {
			unsigned	nil:	4;		// padding for adjustment
			unsigned	a0:		8;		// 8 least significant bits (4 msb)
			unsigned	a1:		8;		// 8 most significant bits out of 12
			unsigned	addr: 	4;		// highest 4 bits from AD7991 contain A/D address
		};
	} ad;

	ad.b_nil = 0;						// initialize lowest 4 bits of output as 0

	i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte((i2c_address<<1)|0x01);	// Send address of device
	
	// Each A/D value consists of two bytes, whereas the first 4 bits contain the A/D address
	// and the rest contains a 12 bit value.  Grab value and left adjust:
	for (int i=0;i<4;i++)
	{
		ad.b1 = I2CReceiveByte();		// Receve high byte
		I2CSend0();						// Send ack
		ad.b0 = I2CReceiveByte();		// Receve low byte
		if (i < 3) I2CSend0();			// If not last byte, send ack

		// Write left adjusted into global var uint16_t	ad7991_adc[4]
		if ((ad.addr) < 4)				// If data not garbled
		{
			ad7991_adc[ad.addr].b1 = ad.a1;
			ad7991_adc[ad.addr].b0 = ad.a0;
		}
	}
	I2CSend1();							// 1 Last byte
	I2CSendStop();						// Stop the I2C comms

	i2c_release();						// Release I2C port
	*/


	//
	// This ugly version is 24 bytes smaller
	//
	sint16_t ad7991;

	//i2c_queue();						// Wait for I2C port to become free

	I2CSendStart();						// Start the I2C comms
	I2CSendByte((i2c_address<<1)|0x01);	// Send address of device
	
	//I2C_SCL_HI;
	//_delay_us(10);;					// Give device some time to convert

	// Each A/D value consists of two bytes, whereas the first 4 bits contain the A/D address
	// and the rest contains a 12 bit value.  Grab value and left adjust:
	for (int i=0;i<4;i++)
	{
		ad7991.b1 = I2CReceiveByte();	// Receve high byte
		I2CSend0();						// Send ack
		ad7991.b0 = I2CReceiveByte();	// Receve low byte
		if (i < 3) I2CSend0();			// If not last byte, send ack

		// Write left adjusted into global var uint16_t	ad7991_adc[4]
		if ((ad7991.b1>>4) < 4)			// If data not garbled
		{
			ad7991_adc[ad7991.b1>>4].b1 = 
				((ad7991.b1 & 0x0f)<<4) + ((ad7991.b0 & 0xf0)>>4);
			ad7991_adc[ad7991.b1>>4].b0 = (ad7991.b0 & 0x0f)<<4;
		}
	}
	I2CSend1();							// 1 Last byte
	I2CSendStop();						// Stop the I2C comms

	//i2c_release();						// Release I2C port
}	




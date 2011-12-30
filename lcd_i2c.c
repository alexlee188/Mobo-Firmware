//-----------------------------------------------------------------------------------------
//			Simple routines for an I2C connected 16x2 LCD diplay
//
//					Loftur E. Jonasson, TF3LJ, 2009-06-30
//-----------------------------------------------------------------------------------------

#include "Mobo.h"

#if LCD_I2C_DISPLAY		// 16x2 I2C connected secondary LCD display

#include "lcd_i2c.h"


void lcd_i2c_address_init(uint8_t new_addr)
{
	I2CSendStart();
	I2CSendByte(DEF_LCD_I2C_ADDRESS<<1);// Default LCD I2C address is 0x18
									// which clashes with the AD7991 address
	I2CSendByte(LCD_I2C_COMMAND);
	I2CSendByte(0x62);				// Change address command
	I2CSendByte(new_addr<<1);		// New address (in 7bit format)
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free
}

//void lcd_i2c_puts(char *string)
void lcd_puts(char *string)
{
	while(Status1 & I2C_BUSY);		// Wait for I2C port to become free
	Status1 |= I2C_BUSY;				// Indicate the I2C port is busy
	I2CSendStart();
	I2CSendByte(LCD_I2C_ADDRESS<<1);

	uint8_t i;
	for (i=0;string[i] != 0;i++)
			I2CSendByte(string[i]);	// send data 
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free
}

//void lcd_i2c_putc(uint8_t c)
void lcd_data(uint8_t c)
{
	while(Status1 & I2C_BUSY);		// Wait for I2C port to become free
	Status1 |= I2C_BUSY;				// Indicate the I2C port is busy

	I2CSendStart();
	I2CSendByte(LCD_I2C_ADDRESS<<1);
	I2CSendByte(c);
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free

}

//void lcd_i2c_command(uint8_t cmd)
void lcd_command(uint8_t cmd)
{
	I2CSendStart();
	I2CSendByte(LCD_I2C_ADDRESS<<1);
	I2CSendByte(LCD_I2C_COMMAND);
	I2CSendByte(cmd);
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free
}

void lcd_i2c_set_contrast(uint8_t contrast)
{
	while(Status1 & I2C_BUSY);		// Wait for I2C port to become free
	Status1 |= I2C_BUSY;				// Indicate the I2C port is busy

	I2CSendStart();
	I2CSendByte(LCD_I2C_ADDRESS<<1);
	I2CSendByte(LCD_I2C_COMMAND);
	I2CSendByte(LCD_I2C_SET_CONTRAST);
	I2CSendByte(contrast);
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free
}

void lcd_i2c_clrscr(void)
{
	while(Status1 & I2C_BUSY);		// Wait for I2C port to become free
	Status1 |= I2C_BUSY;				// Indicate the I2C port is busy

	I2CSendStart();
	I2CSendByte(LCD_I2C_ADDRESS<<1);
	I2CSendByte(LCD_I2C_COMMAND);
	I2CSendByte(LCD_I2C_CLEAR_SCREEN);
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free
}

//void lcd_i2c_gotoxy(uint8_t x, uint8_t y)
void lcd_gotoxy(uint8_t x, uint8_t y)
{
	unsigned cursor = x + 0x40 * y;

	while(Status1 & I2C_BUSY);		// Wait for I2C port to become free
	Status1 |= I2C_BUSY;				// Indicate the I2C port is busy

	I2CSendStart();
	I2CSendByte(LCD_I2C_ADDRESS<<1);
	I2CSendByte(LCD_I2C_COMMAND);
	I2CSendByte(LCD_I2C_SET_CURSOR);
	I2CSendByte(cursor);
	I2CSendStop();

	Status1 &= ~I2C_BUSY;			// Indicate I2C port is free
}
#endif//LCD_I2C_DISPLAY		// 16x2 I2C connected secondary LCD display

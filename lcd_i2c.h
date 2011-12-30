//-----------------------------------------------------------------------------------------
//			Simple routines for an I2C connected 16x2 LCD diplay
//
//					Loftur E. Jonasson, TF3LJ, 2009-08-18
//-----------------------------------------------------------------------------------------

#include "Mobo.h"

#if LCD_I2C_DISPLAY						// 16x2 I2C connected secondary LCD display

#define DEF_LCD_I2C_ADDRESS		0x28	// Default I2C display address

#define LCD_I2C_COMMAND			0xFE	// this is a command

#define LCD_I2C_ON				0x41
#define LCD_I2C_OFF				0x42
#define LCD_I2C_SET_CURSOR		0x45	// Takes one byte arg: [pos]
#define LCD_I2C_HOME_CURSOR		0x46
#define LCD_I2C_CUR_UL_ON		0x47
#define LCD_I2C_CUR_UL_OFF		0x48
#define LCD_I2C_CUR_MV_LEFT		0x49
#define LCD_I2C_CUR_MV_RIGHT	0x4A
#define LCD_I2C_CUR_BLINK_ON	0x4B
#define LCD_I2C_CUR_BLINK_OFF	0x4C
#define LCD_I2C_BACK_SPACE		0x4E
#define LCD_I2C_CLEAR_SCREEN	0x51
#define LCD_I2C_SET_CONTRAST	0x52	// Takes one byte arg: [contrast]
#define LCD_I2C_SET_BRIGHTNESS	0x53	// Takes one byte arg: [brightness]
#define LCD_I2C_LOAD_CUST_CHARS	0x54	// Takes 9 byte arg: [addr][8 byte char map]
#define LCD_I2C_SHIFT_LEFT		0x55
#define LCD_I2C_SHIFT_RIGHT		0x56
#define LCD_I2C_DISP_VERSION	0x70
#define LCD_I2C_CHG_ADDRESS		0x72
#define LCD_I2C_HD44780_CMD		0xFE

// Map to HD44780U cmds:
#define LCD_DISP_ON_BLINK       LCD_I2C_CUR_BLINK_ON
#define LCD_DISP_ON				LCD_I2C_CUR_BLINK_OFF
 
extern uint8_t		Status;				// Contains current status of I2C comms, etc.
// DEFS for Flags used with the global variable "Status"
#ifndef I2C_BUSY
#define I2C_BUSY	(1 << 7)
#endif

/*
extern void lcd_i2c_puts(char *);
extern void lcd_i2c_command(uint8_t);
extern void lcd_i2c_putc(uint8_t);
extern void lcd_i2c_clrscr(void);
extern void lcd_i2c_gotoxy(uint8_t, uint8_t);
extern void lcd_i2c_set_contrast(uint8_t);
*/

extern void lcd_i2c_address_init(uint8_t);
extern void lcd_puts(char *);
extern void lcd_command(uint8_t);
extern void lcd_putc(uint8_t);
extern void lcd_data(uint8_t);
extern void lcd_clrscr(void);
extern void lcd_gotoxy(uint8_t, uint8_t);
extern void lcd_i2c_set_contrast(uint8_t);

#endif// LCD_I2C_DISPLAY					// 16x2 I2C connected secondary LCD display

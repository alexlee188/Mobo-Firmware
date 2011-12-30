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
//**  The below code is mostly a direct swipe from the AVRLIB lcd.c/h.
//**                   Therefore see the AVRLIB copyright notice below
//** 
//** The essentials for bargraph display have been copied and minimally adapted
//** for use with the non AVRLIB LCD display routines used in this project
//**
//** Initial version.: 2009-09-08, Loftur Jonasson, TF3LJ
//**
//** Last update to this file: 2010-01-20, Loftur Jonasson, TF3LJ
//**
//**                   Check the Mobo.c file
//**
//*********************************************************************************

// Copy/Paste of copyright notice from AVRLIB lcd.h:

//*****************************************************************************
//
// File Name	: 'lcd.h'
// Title		: Character LCD driver for HD44780/SED1278 displays
//					(usable in mem-mapped, or I/O mode)
// Author		: Pascal Stang
// Created		: 11/22/2000
// Revised		: 4/30/2002
// Version		: 1.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
///	\ingroup driver_hw
/// \defgroup lcd Character LCD Driver for HD44780/SED1278-based displays (lcd.c)
/// \code #include "lcd.h" \endcode
/// \par Overview
///		This display driver provides an interface to the most common type of
///	character LCD, those based on the HD44780 or SED1278 controller chip
/// (about 90% of character LCDs use one of these chips).  The display driver
/// can interface to the display through the CPU memory bus, or directly via
/// I/O port pins.  When using the direct I/O port mode, no additional
/// interface hardware is needed except for a contrast potentiometer.
/// Supported functions include initialization, clearing, scrolling, cursor
/// positioning, text writing, and loading of custom characters or icons
/// (up to 8).  Although these displays are simple, clever use of the custom
/// characters can allow you to create animations or simple graphics.  The
/// "progress bar" function that is included in this driver is an example of
/// graphics using limited custom-chars.
///
/// \Note The driver now supports both 8-bit and 4-bit interface modes.
///
/// \Note For full text output functionality, you may wish to use the rprintf
/// functions along with this driver
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include "Mobo.h"

#if (LCD_PAR_DISPLAY || LCD_PAR_DISPLAY2 ||LCD_I2C_DISPLAY)// Identical routines for I2C and parallel connected displays
#if BARGRAPH					// LCD Bargraph routines		

#define LCDCHAR_PROGRESS05		0	// 0/5 full progress block
#define LCDCHAR_PROGRESS15		1	// 1/5 full progress block
#define LCDCHAR_PROGRESS25		2	// 2/5 full progress block
#define LCDCHAR_PROGRESS35		3	// 3/5 full progress block
#define LCDCHAR_PROGRESS45		4	// 4/5 full progress block
#define LCDCHAR_PROGRESS55		5	// 5/5 full progress block
//#define LCDCHAR_REWINDARROW		6	// rewind arrow
//#define LCDCHAR_STOPBLOCK		7	// stop block
//#define LCDCHAR_PAUSEBARS		8	// pause bars
//#define LCDCHAR_FORWARDARROW	9	// fast-forward arrow
//#define LCDCHAR_SCROLLUPARROW	10	// scroll up arrow
//#define LCDCHAR_SCROLLDNARROW	11	// scroll down arrow
//#define LCDCHAR_BLANK			12	// scroll down arrow
//#define LCDCHAR_ANIPLAYICON0	13	// animated play icon frame 0
//#define LCDCHAR_ANIPLAYICON1	14	// animated play icon frame 1
//#define LCDCHAR_ANIPLAYICON2	15	// animated play icon frame 2
//#define LCDCHAR_ANIPLAYICON3	16	// animated play icon frame 3

// progress bar defines
#define PROGRESSPIXELS_PER_CHAR	6

// custom LCD characters
unsigned char __attribute__ ((progmem)) LcdCustomChar[] =
{
	//
	// Five different alternatives, the fourth alternative is the original
	// bargraph alternative in the AVRLIB library.  TF3LJ - 2009-08-25
	//
#if BARGRAPH_STYLE_1		// Used if LCD bargraph alternatives.  N8LP LP-100 look alike bargraph
	0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, // 0. 0/5 full progress block
	0x00, 0x10, 0x10, 0x15, 0x10, 0x10, 0x00, 0x00, // 1. 1/5 full progress block
	0x00, 0x18, 0x18, 0x1d, 0x18, 0x18, 0x00, 0x00, // 2. 2/5 full progress block
	0x00, 0x1c, 0x1c, 0x1d, 0x1C, 0x1c, 0x00, 0x00, // 3. 3/5 full progress block
	0x00, 0x1e, 0x1e, 0x1E, 0x1E, 0x1e, 0x00, 0x00, // 4. 4/5 full progress block
	0x00, 0x1f, 0x1f, 0x1F, 0x1F, 0x1f, 0x00, 0x00, // 5. 5/5 full progress block
#endif
#if BARGRAPH_STYLE_2		// Used if LCD bargraph alternatives.  Bargraph with level indicators
	0x01, 0x01, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00, // 0. 0/5 full progress block
	0x01, 0x01, 0x1f, 0x10, 0x10, 0x1f, 0x00, 0x00, // 1. 1/5 full progress block
	0x01, 0x01, 0x1f, 0x18, 0x18, 0x1f, 0x00, 0x00, // 2. 2/5 full progress block
	0x01, 0x01, 0x1f, 0x1C, 0x1C, 0x1f, 0x00, 0x00, // 3. 3/5 full progress block
	0x01, 0x01, 0x1f, 0x1E, 0x1E, 0x1f, 0x00, 0x00, // 4. 4/5 full progress block
	0x01, 0x01, 0x1f, 0x1F, 0x1F, 0x1f, 0x00, 0x00, // 5. 5/5 full progress block
#endif
#if BARGRAPH_STYLE_3		// Used if LCD bargraph alternatives.  Another bargraph with level indicators
	0x01, 0x01, 0x1f, 0x00, 0x00, 0x00, 0x1F, 0x00, // 0. 0/5 full progress block
	0x01, 0x01, 0x1f, 0x10, 0x10, 0x10, 0x1F, 0x00, // 1. 1/5 full progress block
	0x01, 0x01, 0x1f, 0x18, 0x18, 0x18, 0x1F, 0x00, // 2. 2/5 full progress block
	0x01, 0x01, 0x1f, 0x1C, 0x1C, 0x1C, 0x1F, 0x00, // 3. 3/5 full progress block
	0x01, 0x01, 0x1f, 0x1E, 0x1E, 0x1E, 0x1F, 0x00, // 4. 4/5 full progress block
	0x01, 0x01, 0x1f, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, // 5. 5/5 full progress block
#endif
#if BARGRAPH_STYLE_4		// Used if LCD bargraph alternatives.  Original bargraph, Empty space enframed
	0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, // 0. 0/5 full progress block
	0x00, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00, // 1. 1/5 full progress block
	0x00, 0x1F, 0x18, 0x18, 0x18, 0x18, 0x1F, 0x00, // 2. 2/5 full progress block
	0x00, 0x1F, 0x1C, 0x1C, 0x1C, 0x1C, 0x1F, 0x00, // 3. 3/5 full progress block
	0x00, 0x1F, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F, 0x00, // 4. 4/5 full progress block
	0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, // 5. 5/5 full progress block
#endif
#if BARGRAPH_STYLE_5		// Used if LCD bargraph alternatives.  True bargraph, Empty space is empty
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0. 0/5 full progress block
	0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, // 1. 1/5 full progress block
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, // 2. 2/5 full progress block
	0x00, 0x1c, 0x1C, 0x1C, 0x1C, 0x1C, 0x1c, 0x00, // 3. 3/5 full progress block
	0x00, 0x1e, 0x1E, 0x1E, 0x1E, 0x1E, 0x1e, 0x00, // 4. 4/5 full progress block
	0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, // 5. 5/5 full progress block
#endif
	// Unused customizable characters
	//	0x03, 0x07, 0x0F, 0x1F, 0x0F, 0x07, 0x03, 0x00, // 6. rewind arrow
	//	0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, // 7. stop block
	//	0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x00, // 8. pause bars
	//	0x18, 0x1C, 0x1E, 0x1F, 0x1E, 0x1C, 0x18, 0x00, // 9. fast-forward arrow
	//	0x00, 0x04, 0x04, 0x0E, 0x0E, 0x1F, 0x1F, 0x00, // 10. scroll up arrow
	//	0x00, 0x1F, 0x1F, 0x0E, 0x0E, 0x04, 0x04, 0x00, // 11. scroll down arrow
	//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 12. blank character
	//	0x00, 0x0E, 0x19, 0x15, 0x13, 0x0E, 0x00, 0x00,	// 13. animated play icon frame 0
	//	0x00, 0x0E, 0x15, 0x15, 0x15, 0x0E, 0x00, 0x00,	// 14. animated play icon frame 1
	//	0x00, 0x0E, 0x13, 0x15, 0x19, 0x0E, 0x00, 0x00,	// 15. animated play icon frame 2
	//	0x00, 0x0E, 0x11, 0x1F, 0x11, 0x0E, 0x00, 0x00,	// 16. animated play icon frame 3
};

void lcdLoadCustomChar(uint8_t* lcdCustomCharArray, uint8_t romCharNum, uint8_t lcdCharNum)
{
	uint8_t i;

	// multiply the character index by 8
	lcdCharNum = (lcdCharNum<<3);	// each character occupies 8 bytes
	romCharNum = (romCharNum<<3);	// each character occupies 8 bytes

	// copy the 8 bytes into CG (character generator) RAM
	for(i=0; i<8; i++)
	{
		// set CG RAM address
		lcd_command((1<<LCD_CGRAM) | (lcdCharNum+i));
		// write character data
		lcd_data( pgm_read_byte(lcdCustomCharArray+romCharNum+i));
	}
}

void lcdProgressBar(uint16_t progress, uint16_t maxprogress, uint8_t length)
{
	uint8_t i;
	uint16_t pixelprogress;
	uint8_t c;

	if (progress >= maxprogress) progress = maxprogress;	// Clamp the upper bound to prevent funky readings

	// draw a progress bar displaying (progress / maxprogress)
	// starting from the current cursor position
	// with a total length of "length" characters
	// ***note, LCD chars 0-5 must be programmed as the bar characters
	// char 0 = empty ... char 5 = full

	// total pixel length of bargraph equals length*PROGRESSPIXELS_PER_CHAR;
	// pixel length of bar itself is
	pixelprogress = ((uint32_t)(progress*(length*PROGRESSPIXELS_PER_CHAR))/maxprogress);
	
	// print exactly "length" characters
	for(i=0; i<length; i++)
	{
		// check if this is a full block, or partial or empty
		// (u16) cast is needed to avoid sign comparison warning
		if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)+5) > pixelprogress )
		{
			// this is a partial or empty block
			if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)) > pixelprogress )
			{
				// this is an empty block
				// use space character?
				c = 0;
			}
			else
			{
				// this is a partial block
				c = pixelprogress % PROGRESSPIXELS_PER_CHAR;
			}
		}
		else
		{
			// this is a full block
			c = 5;
		}
		
		// write character to display
		lcd_data(c);
	}

}

void lcd_bargraph_init(void)
{
	// load the first 6 custom characters
	for (uint8_t i=0; i<6; i++)
	{
		lcdLoadCustomChar((uint8_t*)LcdCustomChar,i,i);
	}
}
#endif
#endif

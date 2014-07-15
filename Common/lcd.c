// lcd.c

#include "HardwareProfile.h"
#include "AppConfig.h"

#include "lcd.h"
#include "delays.h"

#pragma code

/* WRITE A BYTE TO LCD */ 
void Write_Lcd (unsigned char data, unsigned char mode)
{
	// Set LCD operating mode (data / instruction)
	LCD_RS = mode ;

	// Push current character onto 8-bit data port
	LCD_DATA = data ;
	
	// Pulse enable line; ensuring pulse width is greater than 480 ns
	LCD_EN = 1 ;
	Delay1TCY () ;
	Delay1TCY () ;
	Delay1TCY () ;
	Delay1TCY () ;
	Delay1TCY () ;
	LCD_EN = 0 ;
}

/* CLEAR THE LCD DISPLAY */
void LCD_ClearScreen(void)
{
	// Clear LCD display
	Write_Lcd (0x01,0) ;

	// Delay for approximately 1640 microseconds
	Delay100TCYx (0) ;	
}

/* INITIALISATION ROUTINE FOR LCD MODULE */
void LCD_Init (void)
{
	// Configure Enable, Register Select, Backlight and data lines as outputs
	LCD_EN_TRIS  = 0 ;
	LCD_RS_TRIS  = 0 ;
	LCD_LED_TRIS = 0 ;
	LCD_DT_TRIS  = 0 ;

	// De-assert enable line and turn off backlight
	LCD_EN  = 0 ;
	LCD_LED = 0 ;	// LED is active low

	// Clear display
	LCD_ClearScreen () ;

	// Entry Mode Set - increment cursor position, no display shift
	Write_Lcd   (0x06,0) ;
	Delay10TCYx (48)     ; // 37.5 us

	// Display On/Off Control - display on, cursor off, no cursor blink
	Write_Lcd   (0x0C,0) ;
	Delay10TCYx (48)     ; // 37.5 us

	// Cursor Display/Shift - move cursor, shift right
	Write_Lcd   (0x14,0) ;
	Delay10TCYx (48)     ; // 37.5 us

	// Function Set - 8-bit interface, 2 lines, 5 by 10 dots
	Write_Lcd   (0x3C,0) ;
	Delay10TCYx (48)     ; // 37.5 us
}			

/* DISPLAY A STRING ON LCD */
void LCD_PrintString (const char * msg, LCD_ROW row, BYTE cursor_mode)
{
	unsigned char count = 0 ;

	// Set display row accordingly
	if (row) Write_Lcd (0xC0,0) ;
	else     Write_Lcd (0x80,0) ;

	Delay10TCYx (48) ; // 40 us

	// Send out each character to the LCD module
	while (*msg != 0)
	{
		count++ ;
		
		if (count > 16) return ;
		
		Write_Lcd (*msg++,1) ;

		Delay10TCYx (48) ; // 40 us
	}

	// 2012-05-02 Liz: Display blinking cursor if cursor_mode is ON
	if(cursor_mode==1)
	{
		Write_Lcd   (0x0D,0) ;	//0x0C
		Delay10TCYx (48)     ; // 37.5 us
	}
	else
	{
		// Fill unused character cells with white space
		while (count++ < 16)
		{
			Write_Lcd (' ', 1) ;
		
			Delay10TCYx (48) ; // 40 us
		}
	}

}

/* LOAD CONSTANT STRING INTO RAM AND DISPLAY ON LCD */
void LCD_PrintStringPGM (const rom far unsigned char * msg, LCD_ROW row)
{
	char forDisp[17] ;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(msg == 0)		return;
	//
	
	strcpypgm2ram   (forDisp, msg) ;
	LCD_PrintString (forDisp, row, 0) ;
}

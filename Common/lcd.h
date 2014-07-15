// lcd.h

#ifndef __LCD_H

#define __LCD_H

typedef char LCD_ROW ;

#define LCD_ROW0 0
#define LCD_ROW1 1

void Write_Lcd          (unsigned char data, unsigned char mode)         ;
void LCD_ClearScreen    (void)                                           ;
void LCD_Init           (void)                                           ;
//void LCD_PrintString    (const char * msg, LCD_ROW row)                  ;
void LCD_PrintStringPGM (const rom far unsigned char * msg, LCD_ROW row) ;
void LCD_PrintString (const char * msg, LCD_ROW row, BYTE cursor_mode)	 ;
#endif
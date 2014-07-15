// menu.c

//#include "p18f87j50.h"

#include "lcd.h"
//#include "main.h"
#include "menu.h"
#include "power.h"
#include "zigbee.h"

//Variables for error status messages.
#define ERROR_CHECK_QUEUE_SIZE		2

unsigned 	char 			ERROR_MSG_DISPLAY_COUNTER;
static 		char 			ERROR_CHECK_QUEUE[ERROR_CHECK_QUEUE_SIZE] = {CHECK_MAXQ3180_COMM, CHECK_SDCARD_EXISTS};
unsigned	char 			ERROR_CHECK_QUEUE_STATE=0;
	

			int				REAL_TIME_DISPLAY=0;

static		char 			BUTTONS_STATE;

static		char 			BUTTON_UP_PREV_STATE		= 0;
static		char 			BUTTON_DOWN_PREV_STATE		= 0;
static		char 			BUTTON_LEFT_PREV_STATE		= 0;
static		char 			BUTTON_RIGHT_PREV_STATE		= 0;

// -1 when menu was not entered.
// 1 when showing MAIN_MENU_0_X.
// 2 when showing MAIN_MENU_1_X.
static char MENU_CURRENT_LEVEL = -1;
#define MENU_SIZE			4

#define MAIN_MENU_0_SIZE	3
char MAIN_MENU_0_LEVEL = 0;
const static rom far char MAIN_MENU_0_0[17] = "RT Readings     \0";
const static rom far char MAIN_MENU_0_1[17] = "Settings        \0";
const static rom far char MAIN_MENU_0_2[17] = "Status          \0";
static const rom far unsigned char * MAIN_MENU_0[MAIN_MENU_0_SIZE] = {&MAIN_MENU_0_0, &MAIN_MENU_0_1, &MAIN_MENU_0_2};

#define MAIN_MENU_1_SIZE	7
char MAIN_MENU_1_LEVEL = 0;
const static rom far char MAIN_MENU_0_0_0[17] = "Phase A Volt    \0";
const static rom far char MAIN_MENU_0_0_1[17] = "Phase A Current \0";
const static rom far char MAIN_MENU_0_0_2[17] = "Phase B Volt    \0";
const static rom far char MAIN_MENU_0_0_3[17] = "Phase B Current \0";
const static rom far char MAIN_MENU_0_0_4[17] = "Phase C Volt    \0";
const static rom far char MAIN_MENU_0_0_5[17] = "Phase C Current \0";
const static rom far char MAIN_MENU_0_0_6[17] = "Neutral Current \0";
static const rom far char * MAIN_MENU_1[MAIN_MENU_1_SIZE] = {&MAIN_MENU_0_0_0, &MAIN_MENU_0_0_1, &MAIN_MENU_0_0_2, &MAIN_MENU_0_0_3, &MAIN_MENU_0_0_4, &MAIN_MENU_0_0_5, &MAIN_MENU_0_0_6};

#define MAIN_MENU_2_SIZE		3
char MAIN_MENU_2_LEVEL = 0;
const static rom far char MAIN_MENU_0_1_0[17] = "Set Date        \0";
const static rom far char MAIN_MENU_0_1_1[17] = "Set Time        \0";
const static rom far char MAIN_MENU_0_1_2[17] = "Info";
static const rom far char * MAIN_MENU_2[MAIN_MENU_2_SIZE] = {&MAIN_MENU_0_1_0, &MAIN_MENU_0_1_1, &MAIN_MENU_0_1_2};

#define MAIN_MENU_3_SIZE	5
char MAIN_MENU_3_LEVEL = 0;
const static rom far char MAIN_MENU_0_2_0[17] = "WL signal       \0";
const static rom far char MAIN_MENU_0_2_1[17] = "Network Address \0";
const static rom far char MAIN_MENU_0_2_2[17] = "Parent Address  \0";
const static rom far char MAIN_MENU_0_2_3[17] = "Serial Number   \0";
const static rom far char MAIN_MENU_0_2_4[17] = "Registered?     \0";
static const rom far char * MAIN_MENU_3[MAIN_MENU_3_SIZE] = {&MAIN_MENU_0_2_0, &MAIN_MENU_0_2_1, &MAIN_MENU_0_2_2, &MAIN_MENU_0_2_3, &MAIN_MENU_0_2_4};

unsigned short long MAIN_MENU_OPTIONS[MENU_SIZE] = {&MAIN_MENU_0, &MAIN_MENU_1, &MAIN_MENU_2, &MAIN_MENU_3};
//static const rom far unsigned char * MENU_OPTION_TEXT[SUB_MENU_SIZE][3] = {
//	{&option_text_0_0, &option_text_0_1, &option_text_0_2},
//	{&option_text_1_0, &option_text_1_1, &option_text_1_2},
//	{&option_text_2_0, &option_text_2_1, &option_text_2_2}
//};	


void Menu_Init(void)
{
	//Set all menu pins as inputs. Since all menu buttons are
	//on PORTH we use bit manipulation to save clock cycles.
	TRISH = 0b00001111;
	LATH &= 0xF0;
}

// This function handles users input though the menu buttons.
// Call this function as often as needed from anywhere in the codes
// to have a reasonable response from the device.
//Function returns 1 if the user is in the menu. Else returns 0.
//Calling functions can show idle messages when function returns 0.
int Menu_Handler(void)
{
	int i;
	rom far unsigned char * j;
	

	
	char BUTTON_UP_STATE		= BUTTON_UP_PRESSED;
	char BUTTON_DOWN_STATE		= BUTTON_DOWN_PRESSED;
	char BUTTON_LEFT_STATE		= BUTTON_LEFT_PRESSED;
	char BUTTON_RIGHT_STATE		= BUTTON_RIGHT_PRESSED;
	
	if( REAL_TIME_DISPLAY )
	{
		switch( MAIN_MENU_1_LEVEL )
		{
			case 0: LCD_PrintString(&PHASEA_VOLT, LCD_ROW0); break;
			case 1: LCD_PrintStringPGM("Ex L1 1\0", LCD_ROW0); break;
			case 2: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); break;
			case 3: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); break;
			case 4: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); break;
			case 5: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); break;
		}	
	}			
	
	//if( BUTTON_UP_STATE && DebounceButton(BUTTON_UP) )
	//if( BUTTON_UP_PREV_STATE != BUTTON_UP_STATE )
	{
		BUTTON_UP_PREV_STATE = BUTTON_UP_STATE;
		
		if( BUTTON_UP_STATE && DebounceButton(BUTTON_UP) )
		{
			Buzz(SHORT_PULSE);
			if( MENU_CURRENT_LEVEL == 0 )
			{
				if( MAIN_MENU_0_LEVEL >= MAIN_MENU_0_SIZE-1 ) return;
				MAIN_MENU_0_LEVEL++;
				LCD_PrintStringPGM(MAIN_MENU_0[MAIN_MENU_0_LEVEL], LCD_ROW0);
			}
			if( MENU_CURRENT_LEVEL == 1 )
			{
				if( MAIN_MENU_1_LEVEL >= MAIN_MENU_1_SIZE-1 ) return;
				MAIN_MENU_1_LEVEL++;
				LCD_PrintStringPGM(MAIN_MENU_1[MAIN_MENU_1_LEVEL], LCD_ROW0);
			}	
			if( MENU_CURRENT_LEVEL == 2 )
			{
				if( MAIN_MENU_2_LEVEL >= MAIN_MENU_2_SIZE-1 ) return;
				MAIN_MENU_2_LEVEL++;
				LCD_PrintStringPGM(MAIN_MENU_2[MAIN_MENU_2_LEVEL], LCD_ROW0);
			}
			if( MENU_CURRENT_LEVEL == 3 )
			{
				if( MAIN_MENU_3_LEVEL >= MAIN_MENU_3_SIZE-1 ) return;
				MAIN_MENU_3_LEVEL++;
				LCD_PrintStringPGM(MAIN_MENU_3[MAIN_MENU_3_LEVEL], LCD_ROW0);
			}
		}
	}
	//if( BUTTON_DOWN_PREV_STATE != BUTTON_DOWN_STATE )									//DOWN FUNCTION.
	{
		BUTTON_DOWN_PREV_STATE = BUTTON_DOWN_STATE;
		
		if( BUTTON_DOWN_STATE && DebounceButton(BUTTON_DOWN) )
		{
			Buzz(SHORT_PULSE);
			if( MENU_CURRENT_LEVEL == 0 )
			{
				if( MAIN_MENU_0_LEVEL <= 0 ) return;
				MAIN_MENU_0_LEVEL--;
				LCD_PrintStringPGM(MAIN_MENU_0[MAIN_MENU_0_LEVEL], LCD_ROW0);
			}
			if( MENU_CURRENT_LEVEL == 1 )
			{
				if( MAIN_MENU_1_LEVEL <= 0 ) return;
				MAIN_MENU_1_LEVEL--;
				LCD_PrintStringPGM(MAIN_MENU_1[MAIN_MENU_1_LEVEL], LCD_ROW0);
			}
			if( MENU_CURRENT_LEVEL == 2 )
			{
				if( MAIN_MENU_2_LEVEL <= 0 ) return;
				MAIN_MENU_2_LEVEL--;
				LCD_PrintStringPGM(MAIN_MENU_2[MAIN_MENU_2_LEVEL], LCD_ROW0);
			}
			if( MENU_CURRENT_LEVEL == 3 )
			{
				if( MAIN_MENU_3_LEVEL <= 0 ) return;
				MAIN_MENU_3_LEVEL--;
				LCD_PrintStringPGM(MAIN_MENU_3[MAIN_MENU_3_LEVEL], LCD_ROW0);
			}
		}
	}
	//if( BUTTON_LEFT_PREV_STATE != BUTTON_LEFT_STATE )							//BACK FUNCTION.
	{
		BUTTON_LEFT_PREV_STATE = BUTTON_LEFT_STATE;
		
		if( BUTTON_LEFT_STATE && DebounceButton(BUTTON_LEFT) )
		{
			Buzz(SHORT_PULSE);
			REAL_TIME_DISPLAY=0;
			switch( MENU_CURRENT_LEVEL )
			{
				case 0: 
					MENU_CURRENT_LEVEL=-1; 
					MAIN_MENU_0_LEVEL=0; 
					LCD_ResetDisplay();
					break;
				case 1: 
				case 2:
				case 3:
					MENU_CURRENT_LEVEL=0; 
					LCD_PrintStringPGM(MAIN_MENU_0[MAIN_MENU_0_LEVEL], LCD_ROW0);
					LCD_PrintStringPGM("Menu >          \0", LCD_ROW1);
					break;
				case 11:
					MENU_CURRENT_LEVEL = 1;
					LCD_PrintStringPGM(MAIN_MENU_1[MAIN_MENU_1_LEVEL], LCD_ROW0);
					LCD_PrintStringPGM("Menu >>         \0", LCD_ROW1);
					break;
				case 21:
					MENU_CURRENT_LEVEL = 2;
					LCD_PrintStringPGM(MAIN_MENU_2[MAIN_MENU_2_LEVEL], LCD_ROW0);
					LCD_PrintStringPGM("Menu >>         \0", LCD_ROW1);
					break;
				case 31:
					MENU_CURRENT_LEVEL = 3;
					LCD_PrintStringPGM(MAIN_MENU_3[MAIN_MENU_3_LEVEL], LCD_ROW0);
					LCD_PrintStringPGM("Menu >>         \0", LCD_ROW1);
					break;
			}
			//MENU_CURRENT_LEVEL--;
		}
	}
	//if( BUTTON_RIGHT_PREV_STATE != BUTTON_RIGHT_STATE )							//ENTER FUNCTION.
	{
		BUTTON_RIGHT_PREV_STATE = BUTTON_RIGHT_STATE;
		
		if( BUTTON_RIGHT_STATE && DebounceButton(BUTTON_RIGHT) )
		{
			Buzz(SHORT_PULSE);
			//Codes for...
			if( MENU_CURRENT_LEVEL == -1 )
			{
				MENU_CURRENT_LEVEL = 0;
				LCD_PrintStringPGM(MAIN_MENU_0[0], LCD_ROW0);
				LCD_PrintStringPGM("Menu >          \0", LCD_ROW1);
				return;
			}	
			if( MENU_CURRENT_LEVEL == 0 )
			{
				switch( MAIN_MENU_0_LEVEL )
				{
					case 0:
						MENU_CURRENT_LEVEL = 1;
						MAIN_MENU_1_LEVEL = 0;
						LCD_PrintStringPGM(MAIN_MENU_1[0], LCD_ROW0);
						break;
					case 1:
						MENU_CURRENT_LEVEL = 2;
						MAIN_MENU_2_LEVEL = 0;
						LCD_PrintStringPGM(MAIN_MENU_2[0], LCD_ROW0);
						break;
					case 2:
						MENU_CURRENT_LEVEL = 3;
						MAIN_MENU_3_LEVEL = 0;
						LCD_PrintStringPGM(MAIN_MENU_3[0], LCD_ROW0);
						break;
//					case 3:
//						MENU_CURRENT_LEVEL = 4;
//						break;
				}	
				LCD_PrintStringPGM("Menu >>         \0", LCD_ROW1);
				return;
			}	
			if( MENU_CURRENT_LEVEL == 1 )
			{
				MENU_CURRENT_LEVEL = 11;
				REAL_TIME_DISPLAY=1;
				switch( MAIN_MENU_1_LEVEL )
				{
					case 0: LCD_PrintString(&PHASEA_VOLT, LCD_ROW0); LCD_PrintStringPGM("Menu >> PHA (V)\0", LCD_ROW1); break;
					case 1: LCD_PrintStringPGM(&PHASEA_CURRENT, LCD_ROW0); LCD_PrintStringPGM("Menu >> PHA (A)\0", LCD_ROW1); break;
					case 2: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); LCD_PrintStringPGM("Menu >> PHB   \0", LCD_ROW1); break;
					case 3: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); LCD_PrintStringPGM("Menu >> PHB   \0", LCD_ROW1); break;
					case 4: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); LCD_PrintStringPGM("Menu >> PHC   \0", LCD_ROW1); break;
					case 5: LCD_PrintStringPGM("Ex L1 2\0", LCD_ROW0); LCD_PrintStringPGM("Menu >> PHC   \0", LCD_ROW1); break;
				}	
			}	
			if( MENU_CURRENT_LEVEL == 2 )
			{
				MENU_CURRENT_LEVEL = 21;
				switch( MAIN_MENU_2_LEVEL )
				{
					case 0: LCD_PrintStringPGM("NA", LCD_ROW0); break;
					case 1: LCD_PrintStringPGM("NA", LCD_ROW0); break;
					case 2: LCD_PrintStringPGM("FW Version", LCD_ROW0); LCD_PrintStringPGM(FIRMWARE_VERSION, LCD_ROW1); break;
				}	
			}
			if( MENU_CURRENT_LEVEL == 3 )
			{
				MENU_CURRENT_LEVEL = 31;
				switch( MAIN_MENU_3_LEVEL )
				{
					case 0: LCD_PrintStringPGM("NA\0", LCD_ROW0); break;
					case 1: LCD_PrintString(NETWORK_ADDRESS, LCD_ROW0); break;
					case 2: LCD_PrintString(PARENT_ADDRESS, LCD_ROW0); break;
					case 3: LCD_PrintString(SERIAL_NUMBER, LCD_ROW0); break;
					case 4: (IS_REGISTERED_TO_SERVER == 0) ? LCD_PrintStringPGM("NO", LCD_ROW0) : LCD_PrintStringPGM("YES", LCD_ROW0); break;
				}	
			}	
		}
	}

	return (MENU_CURRENT_LEVEL!=-1);
}

//void ShowIdleMessage(void)
//{
//	LCD_PrintString(IDLE_MSG, LCD_ROW0);
//}

//Debounce code.
int DebounceButton(MENU_BUTTONS button)
{
	char button_port, i, n=0;

	for( i=0; i<100; i++ )
	{
		switch( button )
		{
			case BUTTON_UP:
				button_port = BUTTON_UP_PRESSED;
				break;
			case BUTTON_DOWN:
				button_port = BUTTON_DOWN_PRESSED;
				break;
			case BUTTON_LEFT:
				button_port = BUTTON_LEFT_PRESSED;
				break;
			case BUTTON_RIGHT:
				button_port = BUTTON_RIGHT_PRESSED;
				break;
		}	
		
		if( button_port )
			n++;
		else
			n--;
		
		if( n > 5 )  //Confirm that button was pressed.
		{
			//while//Wait for button release.
			return 1;
		}	
		Delay1KTCYx(12);	//Delay of 1ms @ 48MHz.
	}
	
	return 0;
}	

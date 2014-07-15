// menu.h

//#include "p18f87j50.h"

#include "hardwareprofile.h"


//*********************************************
// Global defines.
//*********************************************
#define CHECK_MAXQ3180_COMM		0
#define CHECK_SDCARD_EXISTS		1

	
//***************************************
// Menu pin assignment.
//***************************************

//#define BUTTON_UP				PORTHbits.RH1
//#define BUTTON_DOWN			PORTHbits.RH0
//#define BUTTON_LEFT			PORTHbits.RH2
//#define BUTTON_RIGHT			PORTHbits.RH3

#define TOTAL_MENU_LEVEL		3


int DebounceButton(MENU_BUTTONS button);
void Menu_Init(void);
int Menu_Handler(void);
void ShowIdleMessage(void);

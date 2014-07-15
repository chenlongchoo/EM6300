/*	Menu task	*/
#include "delays.h"
#include <string.h>
#include "AppConfig.h"
#include "HardwareProfile.h"
#include "menu.h"
#include "utility.h"
#include "TCPIP Stack/TCPIP.h"

#ifdef APP_USE_MCU
	#include "mcu.h"
#endif
#ifdef APP_USE_LCD
	#include "lcd.h"
#endif

#include "Main_Meter_Top_Board.h"

#ifdef APP_USE_MENU

// Deckare variables used to control states of menu and button
enum DISPLAYREADINGSTATES currentreadingdisplaystate = -1;
enum STATES icurrentstate;
static int current_menu;
static short int button_down_old_state, button_up_old_state, button_escape_old_state, button_enter_old_state;
static short int is_button_pressed;
static int readingstate;
static char ph;
static BYTE ph_4total = 0;	// 1
static BYTE current_pw_digit = 0;	// 2012-05-04 Liz: use for displaying user input
static char char_menu_pw[4];		// 2012-05-04 Liz: use for displaying user input
static DWORD Timer;
static BOOL is_cursor_on = 0;		// 2012-05-04 Liz: use for displaying user input
static BYTE digit = 48;				// 2012-05-04 Liz: use for displaying user input

#pragma idata memory_1

//declare reset table
#define reset_table_max 1
const char reset_table_0[17] = "Total kWh";	//2013-03-22 Liz: customize for TPS project only
//const char reset_table_0[17] = "Anacle Systems"; 	//2014-06-16 Liz removed.
const char *reset_table[reset_table_max] = {&reset_table_0};

//declare main menu table
#define main_table_max 3
const char main_table_0[17] = "Measurements 1/3";
const char main_table_1[17] = "Settings     2/3";
const char main_table_2[17] = "Info.        3/3";
const char *main_table[main_table_max] = {&main_table_0, &main_table_1, &main_table_2};

//declare measurements menu table
#define meas_table_max 6
const char meas_table_0[17] = "Voltage(V)   1/6";
const char meas_table_1[17] = "Current(A)   2/6";
const char meas_table_2[17] = "Power(kW)    3/6";
const char meas_table_3[17] = "Pwr Factor   4/6";
const char meas_table_4[17] = "Energy(kWh)  5/6";
const char meas_table_5[17] = "MaxDemand(kW)6/6";
const char *meas_table[meas_table_max] = {&meas_table_0, &meas_table_1, &meas_table_2, &meas_table_3, &meas_table_4, meas_table_5};

// declare phase menu table
#define phase_table_max 4
const char phase_table_0[17] = "Phase A      1/4";
const char phase_table_1[17] = "Phase B      2/4";
const char phase_table_2[17] = "Phase C      3/4";
const char phase_table_3[17] = "Misc         4/4";
const char *phase_table[phase_table_max] = {&phase_table_0, &phase_table_1, &phase_table_2, &phase_table_3};

//declare Misc table
#define misc_table_max	3
const char misc_table_0[17] = "Frequency    1/3";
const char misc_table_1[17] = "Real_Power_T 2/3";
const char misc_table_2[17] = "Current_N    3/3";
const char *misc_table[misc_table_max] = {&misc_table_0, &misc_table_1, &misc_table_2};

//declare settings menu table
// 2012-05-04 Liz: use for displaying user input
#define sett_table_max 3
const char sett_table_0[17] = "Set CT_Range 1/3";
const char sett_table_1[17] = "Reset Energy 2/3";
const char sett_table_2[17] = "Set IP addr  3/3";
const char *sett_table[sett_table_max] = {&sett_table_0, &sett_table_1, &sett_table_2};

//declare info. menu table
#define info_table_max 5
const char info_table_0[17] = "Mac Addr     1/5";
const char info_table_1[17] = "IP Addr      2/5";
const char info_table_2[17] = "Credit Bal.  3/5";
const char info_table_3[17] = "CT Rating    4/5";	// 2012-05-04 Liz: edited
const char info_table_4[17] = "build ver.   5/5";
const char *info_table[info_table_max] = {&info_table_0, &info_table_1, &info_table_2, &info_table_3, &info_table_4};

#pragma idata

/** P R O T O T Y P E S **************************************/
void gotoMenuIdleState(void);
void gotoMenuMeasurementState(void);
void gotoMenuMiscState(void);
void gotoMenuPhaseState(void);
void gotoMenuSettingState(void);
void gotoMenuInfoState(void);
void setPhase(char k);
void resetUserInput(void);	// 2012-05-04 Liz: use for displaying user input
void UserInput(void);		// 2012-05-04 Liz: use for displaying user input
BYTE user_input_control(BYTE number_of_digits);	// 2012-05-04 Liz: use for displaying user input
#ifdef APP_USE_BUZZER
	void sound(void);
#endif

//////////////////////////////////////////////////////
//	These functions set and get value of state of menu
//////////////////////////////////////////////////////
void setMenuCurrentState(enum STATES s)
{
	icurrentstate = s;
}

enum STATES getMenuCurrentState(void)
{
	return icurrentstate;
}


//////////////////////////////////////////////////////
//	These functions set and get value of state of menu
//////////////////////////////////////////////////////
void setCurrentReadingDisplay(enum DISPLAYREADINGSTATES s)
{
	currentreadingdisplaystate = s;
}

enum DISPLAYREADINGSTATES getCurrentReadingDisplay(void)
{
	return currentreadingdisplaystate;
}

void setPhase(char k)
{
	ph = k;
}	
//////////////////////////////////////////////////////
//	These functions set and get index of elements
//		in menu table
//////////////////////////////////////////////////////
void set_current_menu(int i)
{
	current_menu = i;
}	

int get_current_menu(void)
{
	return current_menu;
}

//////////////////////////////////////////////////////
//	This function initialize states of 4 buttons
//////////////////////////////////////////////////////
void set_button_state(void)
{
	button_down_old_state = 0;
	button_up_old_state = 0;
	button_escape_old_state = 0;
	button_enter_old_state = 0;
	is_button_pressed = 0;
	Timer = TickGet() + 5*TICK_SECOND;	// 2014-07-02 Liz. Top board will immediately send request after initialisation.
	LCD_PrintString((char *)reset_table[0], LCD_ROW0, 0);	// 2012-05-04 Liz: add in cursor display mode for LCD_PrintString()
}

//////////////////////////////////////////////////////
//	This function creates the sound.
//////////////////////////////////////////////////////
#ifdef APP_USE_BUZZER
void sound(void)
{
	unsigned long i = 0;
	
	BUZZER = 1;
	while( i++ < 30000 );
	BUZZER = 0;
}
#endif
//////////////////////////////////////////////////////
//	Detect EXIT BUTTON
//////////////////////////////////////////////////////
short int checkMenuExitButton(void)
{
	if (BUTTON_EXIT == 0) button_escape_old_state = 1;
	if ((BUTTON_EXIT == 1) && (button_escape_old_state))
	{
		button_escape_old_state = 0;
		return 1;
	}	
	else
		return 0;
}	

// 2012-05-04 Liz: use for displaying user input

//////////////////////////////////////////////////////
//	This function reset user input array 
//////////////////////////////////////////////////////
void resetUserInput(void)
{
//	BYTE i = 0;
	current_pw_digit = 0;
	is_cursor_on = 0;
	digit = 48;

	// Clear pw buffer after processing.
	// 2012-05-10(Eric) - Save code space.
	memset(char_menu_pw, 0, 4);
}

//////////////////////////////////////////////////////
//	This function display current input from user
//////////////////////////////////////////////////////
void UserInput(void)
{
	//Write_Lcd(0x0D, 0);
	char_menu_pw[current_pw_digit] = digit;
	char_menu_pw[current_pw_digit+1] = 0;
	LCD_PrintString(char_menu_pw, LCD_ROW1, 1);
}


/*====	Control 4 Buttons: UP, DOWN, ENTER, ESCAPE	=====*/
int menu_control(const char * table[], int max)
{
	int i = get_current_menu();
	short int button_press = 0;
	
	// Displays current item in menu
	if (is_button_pressed)
	{
		LCD_PrintString((char *)table[current_menu], LCD_ROW0, 0);
		is_button_pressed = 0;
		#ifdef APP_USE_BUZZER
		sound();
		#endif
	}	

	// If BUTTON_DOWN is pressed, display next item in menu and exit.
	if (i < (max - 1))
	{
		if (BUTTON_DOWN == 0) button_down_old_state = 1;
		if ((BUTTON_DOWN == 1) && (button_down_old_state == 1))
		{
			LCD_PrintString(table[++i], LCD_ROW0, 0);
			button_down_old_state = 0;
			button_press = 1;
			set_current_menu(i);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			return -2;
		}
	}
	
	// If BUTTON_UP is pressed, display previous item in menu and exit.	
	if (i > 0)
	{
		if (BUTTON_UP == 0) button_up_old_state = 1;
		if ((BUTTON_UP == 1) && (button_up_old_state == 1))
		{
			LCD_PrintString(table[--i], LCD_ROW0, 0);
			button_up_old_state = 0;
			button_press = 1;
			set_current_menu(i);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			return -2;
		}
	}
	// If BUTTON_ENTER is pressed, return the index of chosen menu.
	if (BUTTON_ENTER == 0) button_enter_old_state = 1;
	if ((BUTTON_ENTER == 1) && (button_enter_old_state == 1))
	{
		button_enter_old_state = 0;
		button_press = 1;
		is_button_pressed = 1;
		set_current_menu(i);
		return current_menu;
	}
		
	// If BUTTON_EXIT is pressed, display previous menu.	
	if (BUTTON_EXIT == 0) button_escape_old_state = 1;
	if ((BUTTON_EXIT == 1) && (button_escape_old_state))
	{
		button_escape_old_state = 0;
		button_press = 1;
		is_button_pressed = 1;
		set_current_menu(0);
		setCurrentReadingDisplay(NONE);
		return -1;
	}
		
	// If there is no button is pressed, exit.
	if ((!button_press) || (i == (max - 1)) || (i == 0))
		return -2;
}
/*----------------------------------------------------------*/

/*====	2012-04-30 Liz: Control 3 Buttons: UP, DOWN, ENTER to let user key in password/CT_rating 	=====*/
BYTE user_input_control(BYTE number_of_digits)
{
	BYTE i = 0;
	
	// Display cursor to let user know to start key in value
	//if(current_pw_digit == 0 && is_cursor_on==0)
	if(is_cursor_on==0)
	{
		if(getMenuCurrentState() == MENU_PASSWORD_STATE)	LCD_PrintStringPGM("PASSWORD?    1/1", LCD_ROW0);
		UserInput();
		is_cursor_on = 1;
	}

	// If BUTTON_DOWN is pressed, decrease digit value (from 0-9)
	if (digit > 48)
	{
		if (BUTTON_DOWN == 0) button_down_old_state = 1;
		if ((BUTTON_DOWN == 1) && (button_down_old_state == 1))
		{
			digit--;
			UserInput();
			button_down_old_state = 0;
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			return 1;
		}
	}
	
	// If BUTTON_UP is pressed, increase digit value (from 0-9)	
	if (digit < 57)
	{
		if (BUTTON_UP == 0) button_up_old_state = 1;
		if ((BUTTON_UP == 1) && (button_up_old_state == 1))
		{
			digit++;
			UserInput();
			button_up_old_state = 0;
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			return 2;
		}
	}
	// If BUTTON_ENTER is pressed, move to next digit or submit user input
	if (BUTTON_ENTER == 0) button_enter_old_state = 1;
	if ((BUTTON_ENTER == 1) && (button_enter_old_state == 1))
	{
		button_enter_old_state = 0;
		if(current_pw_digit < number_of_digits - 1)
		{
			UserInput();
			current_pw_digit++; 
			digit = 48;
			is_cursor_on = 0;
		}
		else
		{
			// User already key in 4-digit pw, checking if pw is correct
			if(getMenuCurrentState() == MENU_PASSWORD_STATE)
			{
				if(char_menu_pw[0]==AppConfig.Password[0] && char_menu_pw[1]==AppConfig.Password[1]
			 	  && char_menu_pw[2]==AppConfig.Password[2] && char_menu_pw[3]==AppConfig.Password[3])
				{
					LCD_ClearScreen();
					setMenuCurrentState(MENU_SETTINGS_STATE);
					set_current_menu(0);
					gotoMenuSettingState();
				}
				else
				{
					LCD_PrintStringPGM("Wrong Password", LCD_ROW1);
					Delay10KTCYx(0);
					Delay10KTCYx(0);
					LCD_ClearScreen();
					is_cursor_on = 0;
					//LCD_PrintStringPGM("Settings     2/3", LCD_ROW0);
				}
			}
			else if(getMenuCurrentState() == SUBMENU_SETTINGS_STATE && get_current_menu() == 0)	// Get CT_Rating input
			{
				CALIBRATION_VALUES stCalib;
				
				// Get Calib value first
				if( !MCURequestToBOTTOMBoard(MMT_GET_CALIBRATION_DATA, &stCalib, sizeof(CALIBRATION_VALUES), TRUE, TRUE) )
				{
					LCD_PrintStringPGM("Try Again...", LCD_ROW1);
				}
				else
				{
					// Update CT value
					stCalib.CT_RANGE = atoi(char_menu_pw);

					// There are changes... save it!
					MCURequestToBOTTOMBoard(MMT_SET_CALIBRATION_DATA_NEW, &stCalib, sizeof(CALIBRATION_VALUES), TRUE, TRUE);
			
					if( ((BYTE*)&stCalib)[0] != 'O' && ((BYTE*)&stCalib)[1] != 'K' )
						LCD_PrintStringPGM("Try Again...", LCD_ROW1);
					else
					{
						LCD_PrintStringPGM("OK-PLS RESET ENR", LCD_ROW1);
						setMenuCurrentState(MENU_SETTINGS_STATE);
						resetUserInput();
					}
				}
			
				Delay10KTCYx(0);
				Delay10KTCYx(0);
				LCD_ClearScreen();
				LCD_PrintStringPGM("Set CT_Range 1/3", LCD_ROW0);
			}
			else if(getMenuCurrentState() == SUBMENU_SETTINGS_STATE && get_current_menu() == 2)	// Get IPAddr input
			{
				char s[16] = "192.168.1.";

				s[10] = 0;
				strcat(s, char_menu_pw);			

				if(!StringToIPAddress((BYTE*)s, &AppConfig.MyIPAddr))
					LCD_PrintStringPGM("Try Again...", LCD_ROW1);
				else
				{
					LCD_PrintString(s, LCD_ROW1, 0);
					setMenuCurrentState(MENU_SETTINGS_STATE);
					resetUserInput();

					SaveAppConfig();
					Reset();
				}
				Delay10KTCYx(0);
				Delay10KTCYx(0);
				LCD_ClearScreen();
				LCD_PrintStringPGM("Set IP addr  3/3", LCD_ROW0);	
			}
			
			resetUserInput();
		}
		#ifdef APP_USE_BUZZER
			sound();
		#endif	
		return 3;
	}
}
/*----------------------------------------------------------*/


/*==========================================================*/
/*==================  State functions	====================*/
void gotoMenuPhaseState(void)
{
	switch (menu_control(phase_table, phase_table_max))
	{
		case -2:
			break;
		case -1:
			setMenuCurrentState(MENU_IDLE_STATE);
			set_current_menu(0);			
			gotoMenuIdleState();
			break;
		case 0:
			setMenuCurrentState(MENU_MEASUREMENTS_STATE);
			set_current_menu(0);
			setPhase(1);	//setPhase('A');
			gotoMenuMeasurementState();
			break;
		case 1:
			setMenuCurrentState(MENU_MEASUREMENTS_STATE);
			set_current_menu(0);
			setPhase(2);	//setPhase('B');
			gotoMenuMeasurementState();
			break;
		case 2:
			setMenuCurrentState(MENU_MEASUREMENTS_STATE);
			set_current_menu(0);
			setPhase(3);	//setPhase('C');
			gotoMenuMeasurementState();
			break;
		case 3:
			setMenuCurrentState(MENU_MISC_STATE);
			set_current_menu(0);
			setPhase(0);	//setPhase('M');
			gotoMenuMiscState();
			break;
		default:
			break;
	}	
}	

void gotoMenuMeasurementState()
{
	switch (menu_control(meas_table, meas_table_max))
	{
		case -2:
			break;
		case -1:
			//setMenuCurrentState(MENU_IDLE_STATE);
			setMenuCurrentState(MENU_PHASE_STATE);
			//set_current_menu(ph - 65);			
			set_current_menu(ph-1);
			gotoMenuPhaseState();
			break;
		case 0:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(0);	
			// Put Voltage function here
			setCurrentReadingDisplay(VOLTAGE);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 1:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(1);
			// Put Current function here
			setCurrentReadingDisplay(CURRENT);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;	
		case 2:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(2);
			// Put Power function here 
			setCurrentReadingDisplay(POWER);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 3:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(3);
			// Put Power function here
			setCurrentReadingDisplay(PWR_FACTOR);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 4:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(4);
			// Put Energy function here
			setCurrentReadingDisplay(ENERGY);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 5:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(5);
			// Put Energy function here
			setCurrentReadingDisplay(MAX_DEMAND);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		default:
			break;
	}	
}

void gotoMenuMiscState(void)
{
	switch (menu_control(misc_table, misc_table_max))
	{
		case -2:
			break;
		case -1:
			//setMenuCurrentState(MENU_IDLE_STATE);
			setMenuCurrentState(MENU_PHASE_STATE);
			set_current_menu(3);			
			gotoMenuPhaseState();
			break;
		case 0:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(0);	
			// Put Voltage function here
			setCurrentReadingDisplay(FREQUENCY);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 1:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(1);	
			// Put Voltage function here
			setCurrentReadingDisplay(TOTALPOWER);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 2:
			setMenuCurrentState(SUBMENU_MEASUREMENTS_STATE);
			set_current_menu(2);	
			// Put Voltage function here
			setCurrentReadingDisplay(CURRENT_NEUTRAL);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		default:
			break;
	}
}

void gotoMenuSettingState(void)
{
	switch (menu_control(sett_table, sett_table_max))
	{
		case -2:
			break;
		case -1:
			setMenuCurrentState(MENU_IDLE_STATE);
			set_current_menu(1);
			//user_input_control(4);
			break;
		case 0:		// 2012-05-02 Liz: CT_Rating input
			setMenuCurrentState(SUBMENU_SETTINGS_STATE);
			set_current_menu(0);
			user_input_control(4);	
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 1:		// 2012-05-02 Liz: Reset Energy
			setMenuCurrentState(SUBMENU_SETTINGS_STATE);
			set_current_menu(1);
			{
				char t[10] = {'T'};
				MCURequestToBOTTOMBoard(MMT_RESET_ENR_COUNTER, t, 1, TRUE, TRUE);
				if( t[0] == 'O' && t[1] == 'K' ) LCD_PrintStringPGM("OK", LCD_ROW1);
				else LCD_PrintStringPGM("Failed", LCD_ROW1);
			}	
			#ifdef APP_USE_BUZZER
			sound();		
			#endif
			break;
		case 2:		// 2012-06-07 Liz: IP address input
			setMenuCurrentState(SUBMENU_SETTINGS_STATE);
			set_current_menu(2);
			user_input_control(3);	
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		default:
			break;
	}		
}

void gotoMenuInfoState(void)
{
	switch (menu_control(info_table, info_table_max))
	{
		case -2:
			break;
		case -1:
			setMenuCurrentState(MENU_IDLE_STATE);
			set_current_menu(2);
			gotoMenuIdleState();
			break;
		case 0:
			setMenuCurrentState(SUBMENU_INFO_STATE);
			set_current_menu(0);
			// Put MAC Address function here
			LCD_PrintString(strMACAddress, LCD_ROW1, 0);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 1:
			setMenuCurrentState(SUBMENU_INFO_STATE);
			set_current_menu(1);
			// Put IP Address function here
			LCD_PrintString(strIPAddress, LCD_ROW1, 0);
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		case 2:
			setMenuCurrentState(SUBMENU_INFO_STATE);
			set_current_menu(2);
			
//			if (Credit_Balance[0] != '$')
//				LCD_PrintStringPGM("NA",LCD_ROW1);
//			else
// Remove due to customer request to change the symbol to 'RM' instead of '$' so that cannot add this check point. by Stephen-16/07/2012
			LCD_PrintString(Credit_Balance,LCD_ROW1, 0);
			
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;	
		case 3:
		{	
			BYTE ct_resp[10];
			setMenuCurrentState(SUBMENU_INFO_STATE);	
			set_current_menu(3);
			
			// 2012-05-03 Liz: Put CT display here
			// 2012-05-09(Eric) - Modified to save code and data space...
			// Eric(2012-09-19) - When bWait_Response = TRUE, msg_in cannot be null.
			if(!MCURequestToBOTTOMBoard(MMT_CT_RATING_REQUEST, ct_resp, 10, TRUE, TRUE))
				LCD_PrintStringPGM("Try Again...", LCD_ROW1);
			else
				LCD_PrintString(ct_resp, LCD_ROW1, 0);
				
			#ifdef APP_USE_BUZZER
			sound();
			#endif
		}		
			break;	
		case 4:
			setMenuCurrentState(SUBMENU_INFO_STATE);
			set_current_menu(4);
			// Put BUILD VER, function here
			{
				char s[16];
				memcpypgm2ram(s, (ROM void*)FIRMWARE_VERSION, 16);
				LCD_PrintString(s, LCD_ROW1, 0);
			}	
			#ifdef APP_USE_BUZZER
			sound();
			#endif
			break;
		default:
			break;
	}		
}

void gotoMenuResetState(void)
{
	static unsigned int boardrequest = 12000;
	int i;
	char s[17] = "";
	// Sends a request to bottom board to get readings.
	// Any readings received will be processed in mcu.c.

	if( TickGet()-Timer > (5*TICK_SECOND) )
	{
		//s[0] = ph_4total;		// 2014-07-01 Liz removed
		s[0] = 0;				// 2014-07-01 Liz added
		s[1] = 10;
		s[2] = 'N'; //2012-09-24: Liz added to distinguish from EEPROM request.
		// Show on the LCD the reading that is in the local variable
		// while at the same time request for a new reading from the
		// bottom board.
		
		// 2012-05-10(Eric) - No need to retry here since it is not critical.
		// More impt to release control back to main loop.
		MCURequestToBOTTOMBoard(MMT_READING_REQUEST, s, 3, FALSE, FALSE);
		//MCURequestToBOTTOMBoard(MMT_READING_REQUEST, s, 2, FALSE, TRUE);

		// 2014-07-01 Liz removed
//		ph_4total++;
//		if( ph_4total >= 4)
//		{
//			ph_4total = 1;
//			//Convert3RealEnergyToCharArray(s);
//			Convert3RealEnergyToCharArray(s, 1, 1);
//		
//			LCD_PrintString(s, LCD_ROW1, 0);
//		}
		
		// 20114-07-01 Liz added
		Convert3RealEnergyToCharArray(s, 1, 1);
		LCD_PrintString(s, LCD_ROW1, 0);
		///////////////////////
		
		Timer = TickGet();
	}	
	
	switch (menu_control(reset_table, reset_table_max))
	{
		case -2:
			break;
		case 0:
			setMenuCurrentState(MENU_IDLE_STATE);
			set_current_menu(0);
			gotoMenuIdleState();
			LCD_PrintStringPGM("",LCD_ROW1);
			break;
		default:
			break;
	}
}

void gotoMenuIdleState()
{
	switch (menu_control(main_table, main_table_max))
	{
		case -2:
			break;
		case -1:
			setMenuCurrentState(MENU_RESET_STATE);
			set_current_menu(0);
			gotoMenuResetState();
			break;
		case 0:
			setMenuCurrentState(MENU_PHASE_STATE);
			set_current_menu(0);
			gotoMenuPhaseState();
			break;
		case 1:
			setMenuCurrentState(MENU_PASSWORD_STATE);	// 2012-04-30 Liz: User need to key in password for access 
			set_current_menu(1);
			user_input_control(4);
			break;	
		case 2:
			setMenuCurrentState(MENU_INFO_STATE);
			set_current_menu(0);
			gotoMenuInfoState();		
			break;
		default:
			break;
	}
}
/*----------------------------------------------------------*/

/*==========================================================*/
/*===============  Main menu task function	================*/
void menu_task()
{
	switch (getMenuCurrentState())
	{
		case MENU_RESET_STATE:
			gotoMenuResetState();
			break;
		case MENU_IDLE_STATE:
			gotoMenuIdleState();
			break;
		case MENU_PHASE_STATE:
			gotoMenuPhaseState();
			break;
		case MENU_MEASUREMENTS_STATE:
			gotoMenuMeasurementState();
			break;
		case MENU_MISC_STATE:
			gotoMenuMiscState();
			break;
		case MENU_SETTINGS_STATE:
			gotoMenuSettingState();
			break;
		case MENU_INFO_STATE:
			gotoMenuInfoState();
			break;
		case SUBMENU_MEASUREMENTS_STATE:
		{	
			static unsigned int boardrequest = 1200;
			static char is_LCD_printed = 0;
			
			// Sends a request to bottom board to get readings.
			// Any readings received will be processed in mcu.c.
			if( boardrequest++ > 1200 )		//1200
			{
				boardrequest = 0;
				if( (readingstate = getCurrentReadingDisplay()) != NONE )
    			{
	    			char s[20] = "";
	    			s[0] = ph;
	    			s[1] = readingstate;
	    			s[2] = 'N'; //2012-09-24: Liz added to distinguish from EEPROM request.
	    			// Show on the LCD the reading that is in the local variable
	    			// while at the same time request for a new reading from the
	    			// bottom board.
	    			// 2012-05-10(Eric) - No need to retry here since it is not critical.
	    			// More impt to release control back to main loop.
	    			MCURequestToBOTTOMBoard(MMT_READING_REQUEST, s, 3, FALSE, FALSE);
	    			//MCURequestToBOTTOMBoard(MMT_READING_REQUEST, s, 2, FALSE, TRUE);  
    				ConvertPowerReadingsToCharArray(ph, readingstate, s);
    				LCD_PrintString(s, LCD_ROW1, 0);
				}
			} 
			
			if (checkMenuExitButton())
			{
				if(ph == 0)
					setMenuCurrentState(MENU_MISC_STATE);
				else
					setMenuCurrentState(MENU_MEASUREMENTS_STATE);
				LCD_ClearScreen();
			}
		}	
			break;		
		case SUBMENU_SETTINGS_STATE:		
		{	
			if(getMenuCurrentState() == SUBMENU_SETTINGS_STATE)	
			{
				if(get_current_menu() == 0)	// Get CT_Rating input
					user_input_control(4);
				else if(get_current_menu() == 2)	// Get IPAddr input
					user_input_control(3);
			}
	
			if (checkMenuExitButton())
			{
				setMenuCurrentState(MENU_SETTINGS_STATE);
				resetUserInput();
				LCD_ClearScreen();
			}
		}	
			break;
		case SUBMENU_INFO_STATE:
			if (checkMenuExitButton())
			{
				setMenuCurrentState(MENU_INFO_STATE);
				LCD_ClearScreen();
			}	
			break;
		case MENU_PASSWORD_STATE:	// 2012-04-30 Liz: added password for Setting state
		{
			// Get input from user
			user_input_control(4);
			
			if (checkMenuExitButton())
			{
				setMenuCurrentState(MENU_IDLE_STATE);
				resetUserInput();

				LCD_ClearScreen();
			}
		}
			break;
		default:
			break;
	}
}	
#endif
/*----------------------------------------------------------*/

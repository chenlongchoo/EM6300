//menu.h

#ifndef MENU_H
#define MENU_H


//#define main_table_max 3
//#define meas_table_max 4
//#define sett_table_max 2
//#define info_table_max 3
//
enum DISPLAYREADINGSTATES
{
	NONE=-1, VOLTAGE=1, CURRENT=2, PWR_FACTOR=0, 	//VOLTAGE=2, CURRENT=4, PWR_FACTOR=1
	ENERGY=10, DATE=-6, IP_ADDR=-7, MAC_ADDR=-8, 	//ENERGY=1024
	TCP_VER=-9, BUILD_VER=-10, POWER=7, FREQUENCY=0,	//POWER=128, FREQUENCY=1
	TOTALPOWER = 12, CURRENT_NEUTRAL=11, MAX_DEMAND = 13
};
enum STATES
{
	MENU_RESET_STATE = 0, 
	MENU_IDLE_STATE,
	MENU_MEASUREMENTS_STATE,
	MENU_PHASE_STATE, 
	MENU_SETTINGS_STATE, 
	MENU_INFO_STATE, 
	MENU_MISC_STATE,
	SUBMENU_MEASUREMENTS_STATE, 
	SUBMENU_SETTINGS_STATE, 
	SUBMENU_INFO_STATE,
	MENU_PASSWORD_STATE	
};

//enum BUTTONS {button_down_old_state, button_up_old_state, button_escape_old_state, button_enter_old_state};
//enum STATES icurrentstate;
//
//
/*	Prototype functions	*/
enum DISPLAYREADINGSTATES getCurrentReadingDisplay(void);
void setCurrentReadingDisplay(enum DISPLAYREADINGSTATES s);
void setMenuCurrentState(enum STATES s);
void set_current_menu(int i);
int get_current_menu(void);
enum STATES getMenuCurrentState(void);
void set_button_state(void);
short int checkMenuExitButton(void);
void Check_Device(void);
int menu_control(const char * [], int);
void reset_state(void);
void phase_state(void);
void misc_state(void);
void settings_state();
void info_state();
void menu_task(void);

#endif


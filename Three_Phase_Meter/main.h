//main.h


#ifndef __MAIN__H
#define __MAIN__H


//**********************************
// Global Variables.
//**********************************
static unsigned int		PARENT_PING_COUNTER;
static char				IS_TIME_INITIALISED;

//extern char 			IS_CONNECTED_TO_NETWORK;
//extern int 				REAL_TIME_DISPLAY;
//extern char				IS_STATUS_FLAG_SENT;

extern ENR_OVF_COUNTER		EnrOvfCounter1;  // 2012-04-17(Eric) - ENR_CONFIG and EnrConfig Renamed. Refer to release notes.
extern ENR_OVF_COUNTER		EnrOvfCounter2;  // 2012-04-17(Eric) - EnrOvfCounter2 renamed to EnrOvfCounter2. Refer to release notes.
//extern CALIBRATION_VALUES	CalibrationData;


/******************************************/
/*********** Structures *******************/
/******************************************/
#if defined (SINGLE_PHASE_BOTTOM_BOARD)
	#define EEPROM_ENERGY_QUEUE_SIZE				5
#endif
#if defined (THREE_PHASE_BOTTOM_BOARD)
	#define EEPROM_ENERGY_QUEUE_SIZE				15
#endif

	static unsigned int * EEPROM_ENR_OVFC1[EEPROM_ENERGY_QUEUE_SIZE] =   // 2012-04-17(Eric) - Renamed EEPROM_ENR_REGISTERS_OVFC_POS to EEPROM_ENR_OVFC1 for clarity.
	{
		&EnrOvfCounter1.PHASE_A_REAL_POS,
		&EnrOvfCounter1.PHASE_A_REAL_NEG,
		&EnrOvfCounter1.PHASE_A_REACTIVE_POS,
		&EnrOvfCounter1.PHASE_A_REACTIVE_NEG,
		&EnrOvfCounter1.PHASE_A_APPARENT,

		#if defined( THREE_PHASE_BOTTOM_BOARD )
			&EnrOvfCounter1.PHASE_B_REAL_POS,
			&EnrOvfCounter1.PHASE_B_REAL_NEG,
			&EnrOvfCounter1.PHASE_B_REACTIVE_POS,
			&EnrOvfCounter1.PHASE_B_REACTIVE_NEG,
			&EnrOvfCounter1.PHASE_B_APPARENT,

			&EnrOvfCounter1.PHASE_C_REAL_POS,
			&EnrOvfCounter1.PHASE_C_REAL_NEG,
			&EnrOvfCounter1.PHASE_C_REACTIVE_POS,
			&EnrOvfCounter1.PHASE_C_REACTIVE_NEG,
			&EnrOvfCounter1.PHASE_C_APPARENT
		#endif	
	};
	
	static unsigned int * EEPROM_ENR_OVFC2[EEPROM_ENERGY_QUEUE_SIZE] =   // 2012-04-17(Eric) - Renamed EEPROM_ENR_REGISTERS_OVFC_POS to EEPROM_ENR_OVFC2 for clarity.
	{
		&EnrOvfCounter2.PHASE_A_REAL_POS,
		&EnrOvfCounter2.PHASE_A_REAL_NEG,
		&EnrOvfCounter2.PHASE_A_REACTIVE_POS,
		&EnrOvfCounter2.PHASE_A_REACTIVE_NEG,
		&EnrOvfCounter2.PHASE_A_APPARENT,

		#if defined( THREE_PHASE_BOTTOM_BOARD )
			&EnrOvfCounter2.PHASE_B_REAL_POS,
			&EnrOvfCounter2.PHASE_B_REAL_NEG,
			&EnrOvfCounter2.PHASE_B_REACTIVE_POS,
			&EnrOvfCounter2.PHASE_B_REACTIVE_NEG,
			&EnrOvfCounter2.PHASE_B_APPARENT,

			&EnrOvfCounter2.PHASE_C_REAL_POS,
			&EnrOvfCounter2.PHASE_C_REAL_NEG,
			&EnrOvfCounter2.PHASE_C_REACTIVE_POS,
			&EnrOvfCounter2.PHASE_C_REACTIVE_NEG,
			&EnrOvfCounter2.PHASE_C_APPARENT
		#endif	
	};
	
	static unsigned int * EEPROM_CALIBRATION_DATA[12] =   // 2012-04-17(Eric) - EEPROM_DEV_REGISTERS_POS renamed to EEPROM_METER_CONFIG.
	{
		&CalibrationData.VOLTCC, &CalibrationData.AMPCC,
		&CalibrationData.ENRCC, &CalibrationData.PWRCC,
		&CalibrationData.VOLT_GAIN_A, &CalibrationData.CURRENT_GAIN_A
		
		#if defined( THREE_PHASE_BOTTOM_BOARD )
		, &CalibrationData.VOLT_GAIN_B, &CalibrationData.CURRENT_GAIN_B
		, &CalibrationData.VOLT_GAIN_C, &CalibrationData.CURRENT_GAIN_C
		#endif	
		, &CalibrationData.AUX_CONFIG, &CalibrationData.CT_RANGE
	};	

//*********************************************
// Global defines.
//*********************************************

#define IS_SDCARD_INSERTED				SDCARD_CD
#define IS_RUNNING_HIGH_SPEED			OSCTUNEbits.PLLEN


//*********************************************
// Allows to select which modules are enabled.
//*********************************************

#define ENABLE_RTCC
//#define ENABLE_SDCARD


//*********************************************
//Function declarations.
//*********************************************
void main(void);
void OutputReading(unsigned short reg, unsigned char reg_length, long result_high, long result_low, unsigned char reading_type);
void AutoCalibration(void);
char FindLSB(float value, unsigned char reg);
void PowerWriteCALBToMAXQ(void);
void SaveCalibrationData(void);
void ResetBot(void);	// 2012-11-14 Liz added. Reset bot every 12 hours
#endif

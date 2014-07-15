//power_hold.h
#ifndef POWER_HOLD_H
#define POWER_HOLD_H

//typedef struct //__attribute__((__packed__)) 
//	{
//		char		VOLT_A[8];
//		char		CURRENT_A[8];
//		char		REAL_ENERGY_A[8];
//		char		REAL_POWER_A[8];
//		char		APPARANT_POWER_A[8];
//		char		REACTIVE_POWER_A[8];
//		char		APPARANT_ENERGY_A[8];
//		char		REACTIVE_ENERGY_A[8];
//		char		HARMONIC_VOLTAGE[8];
//		char		HARMONIC_CURRENT[8];
//		char		VOLT_B[8];
//		char		VOLT_C[8];	
//		char		CURRENT_B[8];
//		char		CURRENT_C[8];
//		char		REAL_ENERGY_B[8];
//		char		REAL_ENERGY_C[8];
//		char		REAL_POWER_B[8];
//		char		REAL_POWER_C[8];
//		char		APPARANT_POWER_B[8];
//		char		APPARANT_POWER_C[8];
//		char		REACTIVE_POWER_B[8];
//		char		REACTIVE_POWER_C[8];
//		char		APPARANT_ENERGY_B[8];
//		char		APPARANT_ENERGY_C[8];
//		char		REACTIVE_ENERGY_B[8];
//		char		REACTIVE_ENERGY_C[8];
//	
//	} READING_STORAGE;

//typedef struct //__attribute__((__packed__))
//	{
//		char		POWER_FACTOR_A[4];
//		char 		LINE_FREQ[4];
//		char		VOLTCC[4];
//		char		AMPCC[4];
//		char		ENRCC[4];
//		char		PWRCC[4];
//		char		HARMONIC_CONFIG[2];
//		char		VOLT_GAIN_A[4];
//		char		CURRENT_GAIN_A[4];
//		char		RAW_VOLTAGE_A[4];
//		char		RAW_CURRENT_A[4];
//		char		PHASE_A_STATUS[1];
//		char		PHASE_B_STATUS[1];
//		char		PHASE_C_STATUS[1];
//		
//		char 		POWER_FACTOR_B[4];
//		char		VOLT_GAIN_B[4]; 
//		char		CURRENT_GAIN_B[4]; 
//		char		RAW_VOLTAGE_B[4]; 
//		char		RAW_CURRENT_B[4]; 
//	
//		char 		POWER_FACTOR_C[4];
//		char 		VOLT_GAIN_C[4];
//		char		CURRENT_GAIN_C[4];
//		char		RAW_VOLTAGE_C[4];
//		char 		RAW_CURRENT_C[4];
//	
//	} READING_STORAGE_1;
//**********************************
// Variables.
//**********************************

//extern READING_STORAGE		reading_storage;
//extern READING_STORAGE_1	reading_storage_1;

extern char RESPONSE_RECEIVED_POINTER;
extern char RESPONSE_RECEIVED[10][20];
extern char PENDING_REQUESTS_POINTER;
extern char PENDING_REQUESTS[4][14];

//#define POWER_REGISTERS_QUEUE_SIZE				13
//
//extern rom near char * POWER_READINGS_M[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near char * POWER_READINGS_A[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near char * POWER_READINGS_B[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near char * POWER_READINGS_C[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near char * POWER_READINGS[4];

//unsigned long Power_Hold_Consumer(void);

#endif

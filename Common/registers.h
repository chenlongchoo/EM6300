//power.h

#ifndef REGISTERS_H
#define REGISTERS_H


/************************************************/
/******** Temporary storage for readings ********/
/************************************************/


typedef struct __attribute__((__packed__))
	{
		char		VOLT_A[8];
		char		CURRENT_A[8];

		char		REAL_ENERGY_A[12];
		char		APPARENT_ENERGY_A[12];
		char		REACTIVE_ENERGY_A[12];

		char		REAL_POWER_A[8];
		char		APPARENT_POWER_A[8];
		char		REACTIVE_POWER_A[8];

//		char		HARMONIC_VOLTAGE[8];
		char		REAL_ENERGY_T[8];
		char		HARMONIC_CURRENT[8];
		
		char		REAL_POWER_T[8];
		char		REACTIVE_POWER_T[8];
		char		APPARENT_POWER_T[8];
		char 		CURRENT_NEUTRAL[8];
		
	#if defined( THREE_PHASE_BOTTOM_BOARD )	|| defined METER_TOP_BOARD
		char		VOLT_B[8];
		char		VOLT_C[8];	
		char		CURRENT_B[8];
		char		CURRENT_C[8];

		char		REAL_ENERGY_B[12];
		char		REAL_ENERGY_C[12];
		char		APPARENT_ENERGY_B[12];
		char		APPARENT_ENERGY_C[12];
		char		REACTIVE_ENERGY_B[12];
		char		REACTIVE_ENERGY_C[12];

		char		REAL_POWER_B[8];
		char		REAL_POWER_C[8];
		char		APPARENT_POWER_B[8];
		char		APPARENT_POWER_C[8];
		char		REACTIVE_POWER_B[8];
		char		REACTIVE_POWER_C[8];
		
	#endif
	} READING_STORAGE;

typedef struct __attribute__((__packed__))
{
	unsigned long PHASE_A_REAL_POSITIVE;
	unsigned long PHASE_A_REAL_NEGATIVE;
	unsigned long PHASE_A_REACTIVE_POSITIVE;
	unsigned long PHASE_A_REACTIVE_NEGATIVE;
	unsigned long PHASE_A_APPARENT;
	unsigned long PHASE_B_REAL_POSITIVE;
	unsigned long PHASE_B_REAL_NEGATIVE;
	unsigned long PHASE_B_REACTIVE_POSITIVE;
	unsigned long PHASE_B_REACTIVE_NEGATIVE;
	unsigned long PHASE_B_APPARENT;
	unsigned long PHASE_C_REAL_POSITIVE;
	unsigned long PHASE_C_REAL_NEGATIVE;
	unsigned long PHASE_C_REACTIVE_POSITIVE;
	unsigned long PHASE_C_REACTIVE_NEGATIVE;
	unsigned long PHASE_C_APPARENT;
} RAW_ENERGY_STORAGE;

typedef struct __attribute__((__packed__))
	{
		char		POWER_FACTOR_A[4];
		char 		LINE_FREQ[4];
		char		VOLTCC[4];
		char		AMPCC[4];
		char		ENRCC[4];
		char		PWRCC[4];
		char		HARMONIC_CONFIG[4];
		char		VOLT_GAIN_A[4];
		char		CURRENT_GAIN_A[4];
		char		RAW_VOLTAGE_A[4];
		char		RAW_CURRENT_A[4];
		char		PHASE_A_STATUS[4];
		char		PHASE_B_STATUS[4];
		char		PHASE_C_STATUS[4];
		char		GLOBAL_STATUS[4];
		char		CURRENT_TRANSFORMER_RANGE[4];
		
	#if defined( THREE_PHASE_BOTTOM_BOARD ) || defined METER_TOP_BOARD
		char 		POWER_FACTOR_B[4];
		char		VOLT_GAIN_B[4]; 
		char		CURRENT_GAIN_B[4]; 
		char		RAW_VOLTAGE_B[4]; 
		char		RAW_CURRENT_B[4]; 
	
		char 		POWER_FACTOR_C[4];
		char 		VOLT_GAIN_C[4];
		char		CURRENT_GAIN_C[4];
		char		RAW_VOLTAGE_C[4];
		char 		RAW_CURRENT_C[4];
				
	#endif
	
	} READING_STORAGE_1;

// 2012-06-15 Liz: Added for MAX_MIN voltage/current/power record
typedef struct __attribute__((__packed__))
{
	char 		MAX_POWER_A[4];		// 2013-05-05 Liz changed zise from 8 to 4 BYTES since actual value is only 4 bytes.
	char 		MAX_POWER_B[4];
	char 		MAX_POWER_C[4];
//	char 		TEMP_POWER_A[8];	// 2013-03-25 Liz: added to store current reading from MAXQ
//	char 		TEMP_POWER_B[8];	// 2013-03-25 Liz: added to store current reading from MAXQ
//	char 		TEMP_POWER_C[8];	// 2013-03-25 Liz: added to store current reading from MAXQ
//	char		PRE_MAX_POWER_A[8];
//	char		PRE_MAX_POWER_B[8];
//	char		PRE_MAX_POWER_C[8];
} MAX_MIN_STORAGE;

#pragma code
//**********************************
// Variables.
//**********************************
#if defined THREE_PHASE_BOTTOM_BOARD
	extern RAW_ENERGY_STORAGE			raw_energy_storage_2;	// 2014-02-21 Liz added
#endif
extern RAW_ENERGY_STORAGE			raw_energy_storage;
extern READING_STORAGE				reading_storage;
extern READING_STORAGE_1			reading_storage_1;
extern MAX_MIN_STORAGE				max_min_storage;	// 2012-06-15 Liz: Added for MAX_MIN voltage/current/power record

	#define POWER_STATUS					0x000  //1-byte value.
	#define POWER_STATUS_LEN				1
	#define IRQ_FLAG_MASK					0x006  //2-byte value.
	#define IRQ_FLAG_MASK_LEN				2  //2-byte value.
//	#define NS								0x040  //0x062	//2-byte value, 98.
//	#define NS_LEN							4		//2-byte value, 98.

/*	=========	Flags registera	===========	*/	
	#define IRQ_FLAG						0x004  //2-byte value.
	#define IRQ_FLAG_LEN					2  	
	#define	NS								0x040	//4-byte value.
	#define	NS_LEN							4
	
	#define	ENERGY_OVERFLOW_A				0x146	//1-byte value.
	#define	ENERGY_OVERFLOW_A_LEN			1
	#define	ENERGY_OVERFLOW_B				0x232	//1-byte value.
	#define	ENERGY_OVERFLOW_B_LEN			1
	#define	ENERGY_OVERFLOW_C				0x31E	//1-byte value.
	#define	ENERGY_OVERFLOW_C_LEN			1
	
	#define	INTERRUPT_FLAGS_A				0x144	//1-byte value.
	#define	INTERRUPT_FLAGS_A_LEN			1		
	#define	INTERRUPT_FLAGS_B				0x230	//1-byte value.
	#define	INTERRUPT_FLAGS_B_LEN			1
	#define	INTERRUPT_FLAGS_C				0x31C	//1-byte value.
	#define	INTERRUPT_FLAGS_C_LEN			1	
/*===========================================*/

/*	===========		General		============= */
	#define LINE_FREQUENCY					0x062	//2-byte value, 98.
	#define LINE_FREQUENCY_MAP				0x000001
	#define LINE_FREQUENCY_LEN				2		//2-byte value, 98.
//	
	#define ENR_CC							0x01A	//2-byte value, 20.
	#define ENR_CC_MAP						0x000004
	#define ENR_CC_LEN						2		//2-byte value, 20.
	#define PWR_CC							0x018	//2-byte value, 20.
	#define PWR_CC_MAP						0x000010
	#define PWR_CC_LEN						2		//2-byte value, 20.
	#define VOLT_CC							0x014	//2-byte value, 20.
	#define VOLT_CC_MAP						0x000008
	#define VOLT_CC_LEN						2		//2-byte value, 20.
	#define AMP_CC							0x016	//2-byte value, 22.
	#define AMP_CC_MAP						0x000002
	#define AMP_CC_LEN						2		//2-byte value, 22.
/*===========================================*/

/*	===========		Current		============= */
	
	#define RAW_RMS_CURRENT_MAP				0x000010
	#define	PHASE_A_RAW_RMS_CURRENT			0x1CC	//4-byte value, 460.
	#define	PHASE_A_RAW_RMS_CURRENT_LEN		4		//4-byte value, 460.
	#define	PHASE_B_RAW_RMS_CURRENT			0x2B8	//4-byte value.
	#define	PHASE_B_RAW_RMS_CURRENT_LEN		4		//4-byte value.
	#define	PHASE_C_RAW_RMS_CURRENT			0x3A4	//4-byte value.	
	#define	PHASE_C_RAW_RMS_CURRENT_LEN		4		//4-byte value.
//	
	#define CURRENT_GAIN_MAP				0x000040
	#define PHASE_A_CURRENT_GAIN			0x130	//2-byte value, 304.
	#define PHASE_A_CURRENT_GAIN_LEN		2		//2-byte value, 304.
	#define PHASE_B_CURRENT_GAIN			0x21C	//2-byte value.
	#define PHASE_B_CURRENT_GAIN_LEN		2		//2-byte value.
	#define PHASE_C_CURRENT_GAIN			0x308	//2-byte value.
	#define PHASE_C_CURRENT_GAIN_LEN		2		//2-byte value.
//
	#define RMS_CURRENT_MAP					0x000004
	#define	PHASE_A_RMS_CURRENT				0x841	//4-byte value, 2113.
	#define	PHASE_A_RMS_CURRENT_LEN			8	//4-byte value, 2113.
	#define	PHASE_B_RMS_CURRENT				0x842	//4-byte value.
	#define	PHASE_B_RMS_CURRENT_LEN			8	//4-byte value.
	#define	PHASE_C_RMS_CURRENT				0x844	//4-byte value.
	#define	PHASE_C_RMS_CURRENT_LEN			8	//4-byte value.
	#define	NEUTRAL_RMS_CURRENT				0x840	//4-byte value.
	#define	NEUTRAL_RMS_CURRENT_LEN			8	//4-byte value.
	
/*===========================================*/

/*	===========		Voltage		============= */

	#define RAW_RMS_VOLTAGE_MAP				0x000008
	#define	PHASE_A_RAW_RMS_VOLTAGE			0x1C8	//4-byte value, 456.
	#define	PHASE_A_RAW_RMS_VOLTAGE_LEN		4		//4-byte value, 456.
	#define	PHASE_B_RAW_RMS_VOLTAGE			0x2B4	//4-byte value.
	#define	PHASE_B_RAW_RMS_VOLTAGE_LEN		4		//4-byte value.
	#define	PHASE_C_RAW_RMS_VOLTAGE			0x3A0	//4-byte value.
	#define	PHASE_C_RAW_RMS_VOLTAGE_LEN		4		//4-byte value.
//
	#define VOLTAGE_GAIN_MAP				0x000020
	#define PHASE_A_VOLTAGE_GAIN			0x132	//2-byte value, 306.
	#define PHASE_A_VOLTAGE_GAIN_LEN		2		//2-byte value, 306.
	#define PHASE_B_VOLTAGE_GAIN			0x21E	//2-byte value.
	#define PHASE_B_VOLTAGE_GAIN_LEN		2		//2-byte value.
	#define PHASE_C_VOLTAGE_GAIN			0x30A	//2-byte value.
	#define PHASE_C_VOLTAGE_GAIN_LEN		2		//2-byte value.
//
	#define RMS_VOLTAGE_MAP					0x000002
	#define	PHASE_A_RMS_VOLTAGE				0x831	//4-byte value, 2097.
	#define	PHASE_A_RMS_VOLTAGE_LEN			8		//8-byte value, 2097.
	#define	PHASE_B_RMS_VOLTAGE				0x832	//4-byte value.
	#define	PHASE_B_RMS_VOLTAGE_LEN			8	//4-byte value.
	#define	PHASE_C_RMS_VOLTAGE				0x834	//4-byte value.
	#define	PHASE_C_RMS_VOLTAGE_LEN			8	//4-byte value.
	
/*===========================================*/

/*	===========		Power		============= */

	#define POWER_FACTOR_MAP				0x000001
	#define	PHASE_A_POWER_FACTOR			0x1C6
	#define	PHASE_A_POWER_FACTOR_LEN		2
	#define	PHASE_B_POWER_FACTOR			0x2B2
	#define	PHASE_B_POWER_FACTOR_LEN		2
	#define	PHASE_C_POWER_FACTOR			0x39E
	#define	PHASE_C_POWER_FACTOR_LEN		2	
	#define	TOTAL_POWER_FACTOR				0x867
	#define	TOTAL_POWER_FACTOR_LEN			2	
//
	#define REAL_POWER_MAP				0x00100
	#define	PHASE_A_REAL_POWER			0x801	//4-byte value.
	#define	PHASE_A_REAL_POWER_LEN		8
	#define	PHASE_B_REAL_POWER			0x802	//4-byte value.
	#define	PHASE_B_REAL_POWER_LEN		8
	#define	PHASE_C_REAL_POWER			0x804	//4-byte value.
	#define	PHASE_C_REAL_POWER_LEN		8
	#define TOTAL_REAL_POWER			0x807	//4-byte value.
	#define TOTAL_REAL_POWER_LEN		8
//
	#define REACTIVE_POWER_MAP				0x000400
	#define	PHASE_A_REACTIVE_POWER			0x811	//4-byte value.
	#define	PHASE_A_REACTIVE_POWER_LEN		8
	#define	PHASE_B_REACTIVE_POWER			0x812	//4-byte value.
	#define	PHASE_B_REACTIVE_POWER_LEN		8
	#define	PHASE_C_REACTIVE_POWER			0x814	//4-byte value.
	#define	PHASE_C_REACTIVE_POWER_LEN		8
	#define TOTAL_REACTIVE_POWER			0x817	//4-byte value.
	#define TOTAL_REACTIVE_POWER_LEN		8
//
	#define APPARANT_POWER_MAP				0x000200
	#define	PHASE_A_APPARANT_POWER			0x821	//4-byte value.
	#define	PHASE_A_APPARANT_POWER_LEN		8
	#define	PHASE_B_APPARANT_POWER			0x822	//4-byte value.
	#define	PHASE_B_APPARANT_POWER_LEN		8
	#define	PHASE_C_APPARANT_POWER			0x824	//4-byte value.
	#define	PHASE_C_APPARANT_POWER_LEN		8
	#define TOTAL_APPARANT_POWER			0x827	//4-byte value.
	#define TOTAL_APPARANT_POWER_LEN		8
//
	#define	PHASE_A_F_REAL_POWER		0x881	//4-byte value.
	#define	PHASE_A_F_REAL_POWER_LEN	8
//
	// 2012-07-19 Liz: Max Demand
	#define PHASE_A_MAX_DEMAND				0x950
	#define PHASE_A_MAX_DEMAND_LEN			4
	#define PHASE_B_MAX_DEMAND				0x951
	#define PHASE_B_MAX_DEMAND_LEN			4
	#define PHASE_C_MAX_DEMAND				0x952
	#define PHASE_C_MAX_DEMAND_LEN			4
/*===========================================*/

/*	===========		Energy		============= */

	#define REAL_ENERGY_MAP				0x000080
	#define	PHASE_A_REAL_ENERGY			0x8C1	//4-byte value.
	#define	PHASE_A_REAL_ENERGY_LEN		8
	#define	PHASE_B_REAL_ENERGY			0x8C2	//4-byte value. // 8C2
	#define	PHASE_B_REAL_ENERGY_LEN		8
	#define	PHASE_C_REAL_ENERGY			0x8C4	//4-byte value.
	#define	PHASE_C_REAL_ENERGY_LEN		8
	#define	TOTAL_REAL_ENERGY			0x8C7	//4-byte value.
	#define	TOTAL_REAL_ENERGY_LEN		8
//
	#define REACTIVE_ENERGY_MAP				0x001000
	#define	PHASE_A_REACTIVE_ENERGY			0x8D1	//4-byte value.
	#define	PHASE_A_REACTIVE_ENERGY_LEN		8
	#define	PHASE_B_REACTIVE_ENERGY			0x8D2	//4-byte value.
	#define	PHASE_B_REACTIVE_ENERGY_LEN		8
	#define	PHASE_C_REACTIVE_ENERGY			0x8D4	//4-byte value.
	#define	PHASE_C_REACTIVE_ENERGY_LEN		8
	#define	TOTAL_REACTIVE_ENERGY			0x8D7	//4-byte value.
	#define	TOTAL_REACTIVE_ENERGY_LEN		8
//
	#define APPARANT_ENERGY_MAP				0x000800
	#define	PHASE_A_APPARANT_ENERGY			0x871	//4-byte value.
	#define	PHASE_A_APPARANT_ENERGY_LEN		8
	#define	PHASE_B_APPARANT_ENERGY			0x872	//4-byte value.
	#define	PHASE_B_APPARANT_ENERGY_LEN		8
	#define	PHASE_C_APPARANT_ENERGY			0x874	//4-byte value.
	#define	PHASE_C_APPARANT_ENERGY_LEN		8
	#define	TOTAL_APPARANT_ENERGY			0x877	//4-byte value.
	#define	TOTAL_APPARANT_ENERGY_LEN		8
//
	#define	PHASE_A_RAW_POS_REAL_ENERGY		0x1E8	//4-byte value.
	#define	PHASE_A_RAW_POS_REAL_ENERGY_LEN	4
	#define	PHASE_A_ENERGY_REAL_NEG			0x1EC	//4-byte value.
	#define	PHASE_A_ENERGY_REAL_NEG_LEN		4
	#define	PHASE_A_ENERGY_REAC_POS			0x1F0	//4-byte value.
	#define	PHASE_A_ENERGY_REAC_POS_LEN		4
	#define	PHASE_A_ENERGY_REAC_NEG			0x1F4	//4-byte value.
	#define	PHASE_A_ENERGY_REAC_NEG_LEN		4
//
	#define	PHASE_B_RAW_POS_REAL_ENERGY		0x2D4	//4-byte value.
	#define	PHASE_B_RAW_POS_REAL_ENERGY_LEN	4
	#define	PHASE_B_ENERGY_REAL_NEG			0x2D8	//4-byte value.
	#define	PHASE_B_ENERGY_REAL_NEG_LEN		4
	#define	PHASE_B_ENERGY_REAC_POS			0x2DC	//4-byte value.
	#define	PHASE_B_ENERGY_REAC_POS_LEN		4
	#define	PHASE_B_ENERGY_REAC_NEG			0x2E0	//4-byte value.
	#define	PHASE_B_ENERGY_REAC_NEG_LEN		4
//
	#define	PHASE_C_RAW_POS_REAL_ENERGY		0x3C0	//4-byte value.
	#define	PHASE_C_RAW_POS_REAL_ENERGY_LEN	4
	#define	PHASE_C_ENERGY_REAL_NEG			0x3C4	//4-byte value.
	#define	PHASE_C_ENERGY_REAL_NEG_LEN		4
	#define	PHASE_C_ENERGY_REAC_POS			0x3C8	//4-byte value.
	#define	PHASE_C_ENERGY_REAC_POS_LEN		4
	#define	PHASE_C_ENERGY_REAC_NEG			0x3CC	//4-byte value.
	#define	PHASE_C_ENERGY_REAC_NEG_LEN		4
//
	#define	PHASE_A_RAW_APPARANT_ENERGY			0x1F8	//0x871	//4-byte value.
	#define	PHASE_A_RAW_APPARANT_ENERGY_LEN		4		//8
	#define	PHASE_B_RAW_APPARANT_ENERGY			0x2E4	//0x872	//4-byte value.
	#define	PHASE_B_RAW_APPARANT_ENERGY_LEN		4		//8
	#define	PHASE_C_RAW_APPARANT_ENERGY			0x3D0	//0x874	//4-byte value.
	#define	PHASE_C_RAW_APPARANT_ENERGY_LEN		4		//8
/*===========================================*/	

/*	===========		Harmonic	============= */
	#define	AUX_CFG							0x010
	#define	AUX_CFG_LEN						2
	#define HARMONIC_RMS_VOLTAGE			0x830
	#define HARMONIC_RMS_VOLTAGE_LEN		8
	#define HARMONIC_RMS_CURRENT			0x840
	#define HARMONIC_RMS_CURRENT_LEN		8
/*===========================================*/	

//**********************************
// Structures.
//**********************************
#ifdef SINGLE_PHASE_BOTTOM_BOARD
	#define POWER_CALIBRATION_QUEUE_SIZE				7
#endif
#if defined THREE_PHASE_BOTTOM_BOARD || defined METER_TOP_BOARD
	#define POWER_CALIBRATION_QUEUE_SIZE				11
#endif

// The order of the items here must correspond to the order of items in DEV_CONFIG struct.
extern rom near unsigned short POWER_CALIBRATION_QUEUE[POWER_CALIBRATION_QUEUE_SIZE];
extern rom near unsigned short POWER_CALIBRATION_SIZE_QUEUE[POWER_CALIBRATION_QUEUE_SIZE];

#ifdef SINGLE_PHASE_BOTTOM_BOARD
	#define PHASE_STATUS_FLAGS_QUEUE_SIZE				2
#endif
#if defined THREE_PHASE_BOTTOM_BOARD || defined METER_TOP_BOARD
	#define PHASE_STATUS_FLAGS_QUEUE_SIZE				4
#endif

extern rom near unsigned short PHASE_STATUS_FLAGS_QUEUE[PHASE_STATUS_FLAGS_QUEUE_SIZE];
extern rom near unsigned short PHASE_STATUS_FLAGS_SIZE_QUEUE[PHASE_STATUS_FLAGS_QUEUE_SIZE];

#ifdef SINGLE_PHASE_BOTTOM_BOARD
	#define ENERGY_OVERFLOW_FLAGS_QUEUE_SIZE			1
#endif
#if defined THREE_PHASE_BOTTOM_BOARD || defined METER_TOP_BOARD
	#define ENERGY_OVERFLOW_FLAGS_QUEUE_SIZE			3
#endif

extern rom near char * PHASE_RAW_ENERGY_STORAGE_QUEUE[3][5];
extern rom near unsigned short PHASE_ENERGY_REGISTERS_QUEUE[3][5];

// 2012-06-15 Liz: Added for MAX_MIN voltage/current/power record
extern rom near char * MAX_MIN_STORAGE_QUEUE[3][1];	
//extern rom near char * PREV_MAX_MIN_STORAGE_QUEUE[3][1];
//

//extern rom near short * PHASE_ENERGY_OVERFLOW_COUNTER_QUEUE[3][5];
extern rom near unsigned short ENERGY_OVERFLOW_FLAGS_QUEUE[ENERGY_OVERFLOW_FLAGS_QUEUE_SIZE];
extern rom near unsigned short ENERGY_OVERFLOW_FLAGS_SIZE_QUEUE[ENERGY_OVERFLOW_FLAGS_QUEUE_SIZE];

#define POWER_REGISTERS_QUEUE_SIZE				16

//extern rom near char * POWER_READINGS_0[POWER_REGISTERS_QUEUE_SIZE];
extern rom near char * POWER_READINGS_M[POWER_REGISTERS_QUEUE_SIZE];
extern rom near char * POWER_READINGS_A[POWER_REGISTERS_QUEUE_SIZE];
extern rom near char * POWER_READINGS_B[POWER_REGISTERS_QUEUE_SIZE];
extern rom near char * POWER_READINGS_C[POWER_REGISTERS_QUEUE_SIZE];
extern rom near char * POWER_READINGS[4];

#if defined(APP_USE_MANUAL_READING)
	#define POWERM_REGISTERS_QUEUE_SIZE			1
	static unsigned char POWERM_REGISTERS_QUEUE_STATE = 0;
	//The POWER_REGISTERS_QUEUE array holds the registers that will be read
	//from the MAXQ3180 and sent back to the server.
	//Each register will be read sequentially by the program.
	//POWER_REGISTERS_QUEUE_STATE points to the next register which will
	//be read from the array.
	static const unsigned short POWERM_REGISTERS_QUEUE[POWERM_REGISTERS_QUEUE_SIZE] = 
	{
		PHASE_A_VOLTAGE//, PHASE_A_CURRENT,
		//PHASE_B_VOLTAGE//, PHASE_B_CURRENT,
		//PHASE_C_VOLTAGE, PHASE_C_CURRENT,
		//PHASE_N_CURRENT
	};
#endif


static unsigned char POWER_REGISTERS_QUEUE_STATE = 0;
//extern rom near const unsigned short POWER_REGISTERS_QUEUE[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned short POWER_REGISTERS_QUEUE_M[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned short POWER_REGISTERS_QUEUE_A[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned short POWER_REGISTERS_QUEUE_B[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned short POWER_REGISTERS_QUEUE_C[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned short * POWER_REGISTERS_QUEUE[4];

//extern rom near const unsigned char POWER_REGISTERS_SIZE_QUEUE[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char POWER_REGISTERS_SIZE_QUEUE_M[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char POWER_REGISTERS_SIZE_QUEUE_A[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char POWER_REGISTERS_SIZE_QUEUE_B[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char POWER_REGISTERS_SIZE_QUEUE_C[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char * POWER_REGISTERS_SIZE_QUEUE[4];	

//extern rom near const unsigned char POWER_REGISTERS_DIVIDE_BY[POWER_REGISTERS_QUEUE_SIZE];
extern unsigned char POWER_REGISTERS_DIVIDE_BY_M[POWER_REGISTERS_QUEUE_SIZE];
extern unsigned char POWER_REGISTERS_DIVIDE_BY_A[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near const unsigned char POWER_REGISTERS_DIVIDE_BY_B[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near const unsigned char POWER_REGISTERS_DIVIDE_BY_C[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near const unsigned char * POWER_REGISTERS_DIVIDE_BY[4];

//extern rom near const unsigned char POWER_REGISTERS_DIVIDE_BY[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char POWER_SIGNED_REGISTERS_M[POWER_REGISTERS_QUEUE_SIZE];
extern rom near const unsigned char POWER_SIGNED_REGISTERS_A[POWER_REGISTERS_QUEUE_SIZE];
//extern rom near const unsigned char * POWER_SIGNED_REGISTERS[2];

void Set_div(unsigned char c, unsigned char reg, unsigned char phase);

#endif

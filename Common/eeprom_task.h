#ifndef __EEPROM_TASK_H
#define __EEPROM_TASK_H

#include "GenericTypeDefs.h"
#include "AppConfig.h"

/*=============================================================================
*	The allocated memory spaces for Communicator Board:  
*	_ 0x00 -> POWER_SAVE_HEAD (30k Bytes): Stored webpage + AppConfig. 
*	_ POWER_SAVE_HEAD -> POWER_SAVE_END (5k Bytes): Stored power readings recieved from Meter.
*	_ TELNET_INTERVAL (1 Byte): Stored time delay for telnet.
*	_ IP_SPACE (41 Bytes): Stored IP_LIST.	
=============================================================================*/
#ifdef COMMUNICATOR_BOARD	// Reserve 
	#define	POWER_SAVE_HEAD		0x07531	// Start address for Power_Save block in EEPROM
	#define	POWER_SAVE_END		0x088B8	// End address for Power_Save block in EEPROM	
	#define	MAX_FREE_SPACE		5000

	#define TELNET_INTERVAL		0x0C355	
	#define IP_SPACE			0x0C35A
	
//	extern char is_cycle_completed;
#endif

/*=============================================================================
*	The allocated memory spaces for TOP Board:  
*	_ 0x00 -> POWER_SAVE_HEAD (58.5k Bytes): Stored webpage + AppConfig. 
*	_ POWER_SAVE_HEAD -> POWER_SAVE_END (5k Bytes): Stored power readings recieved from Meter.
*	_ CHECKED_LIST (4 Bytes): Stored list of checked boxes on webpage.
=============================================================================*/
#ifdef METER_TOP_BOARD
	#ifdef USE_25LC1024
		#define	POWER_SAVE_HEAD		0x007D1		//0x109A1	// Start address for Power_Save block in EEPROM
		#define	POWER_SAVE_END		0x1F0FD	// End address for Power_Save block in EEPROM	
		#define	MAX_FREE_SPACE		125228
		
		#define CREDIT_BALANCE_EEP_POS	0x0076C		// 2013-09-24 Liz: Added location of credit balance in eeprom
		#define TWELVE_HOUR_RESET_FLAG	0x007C6		// 2013-09-20 Liz: Location of 12-hour-auto-reset flag
		#define EEPROM_INTERVAL_POS		0x0079E		// 2013-09-25 Liz: Added location of Eeprom record interval
	#endif
	#ifdef USE_25LC512
		#define	POWER_SAVE_HEAD		0x0E485	// Start address for Power_Save block in EEPROM
		#define	POWER_SAVE_END		0x0F80D	// End address for Power_Save block in EEPROM	
		#define	MAX_FREE_SPACE		5000
		
		#define	CHECKED_LIST		0x0F811
	#endif
#endif

#define EEP_RECORD_LENGTH	36	// 2014-01-07 Liz added
extern BOOL is_cycle_completed;	//2014-01-07 Liz added
extern BYTE no_of_paras;		// 2014-01-07 Liz added

extern DWORD WriteCursor;	// Cursor to keep track current position of writing process
extern DWORD ReadCursor;		// Cursor to keep track current position of reading process
extern unsigned long free_space;
extern unsigned int unread_records;

BOOL EEPROMEnergyRecord(void);
void InitEEPROMSavePower(void);
BOOL EEPROM_Write_Readings(char * source, char length);
BOOL EEPROM_Read_Readings(char * rbuffer, char length);
unsigned long CheckFreeSpace(void);
void ResetMemory(void);
BOOL UpdateRecords(unsigned long location);
void Split3P_ToSinglePhase(char * source);

#endif	// define __EEPROM_TASK_H
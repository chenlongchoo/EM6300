#define __EEPROM_TASK_C

#include <math.h>
#include "eeprom_task.h"
#include "TCPIP Stack/TCPIP.h"
#include "Main_Meter_Top_Board.h"

#if defined(APP_USE_MCU)
	#include "mcu.h"
#endif

DWORD WriteCursor;	// Cursor to keep track current position of writing process
DWORD ReadCursor;		// Cursor to keep track current position of reading process
unsigned long free_space;
unsigned int unread_records;

//2013-12-27 Liz: added to keep track which parameters will be stored in eeprom
unsigned short eep_para_list[14] = {0x0911};	
static BYTE current_reg = 0;
BOOL is_cycle_completed = FALSE;
BYTE no_of_paras = 1;	// 2014-01-07 Liz added
//

#ifdef METER_TOP_BOARD

/*----------------------------------------------------------------------- 
	This function requests for total energy from BOT and save in eeprom 
	2013-12-27 Liz added required parameters list
------------------------------------------------------------------------*/
BOOL EEPROMEnergyRecord(void)  
{
	char s[45] = {0, 10, 'E', 0};	//2014-06-10 Liz increase length.
	BOOL result = FALSE;
	unsigned short reg = 0;
		
	memcpy(&s[0], &eep_para_list[current_reg], 2);
	reg = eep_para_list[current_reg];
			
	// Request energy reading from BOT board
	result = MCURequestToBOTTOMBoard(MMT_GET_3PHASE_FOR_EEPROM, &s[0], 3, TRUE, TRUE);
	
	//result = 1;
	
	if(result == TRUE)
	{
		// 2014-06-04 Liz. 3 phase energy is not ready to be collected yet.
		if(s[0] == 'F' && s[1] == 'A' && s[2] == 'I')
			return FALSE;
		
		// Write into eeprom
		result = FALSE;

		// 2014-06-10 Liz updated datetime flag
		if(s[6] == 1)
			meter_flag.Flags.Flag1.bDateTimeUpdated = 1;
		else
			meter_flag.Flags.Flag1.bDateTimeUpdated = 0;

		// Remove 2 ENR_CC included in phase C raw reading
		//memmove(&s[31], &s[33], 6);
		memmove(&s[32], &s[34], 6);
	
		// Remove 2 ENR_CC included in phase C raw reading
		//memmove(&s[21], &s[23], 14);
		memmove(&s[22], &s[24], 14);
	
		// Remove register name since we only store 1 type of reading in eeprom (3 phase real energy)
		//memmove(&s[6], &s[8], 27);
		memmove(&s[6], &s[9], 27);
		
		// Write to eeprom
		result = EEPROM_Write_Readings(&s[0], EEP_RECORD_LENGTH);
				
//		if(reg == 0x0911)
//		{
//			Split3P_ToSinglePhase(&s[0]);
//			result = TRUE;
//		}
//		else
//			result = EEPROM_Write_Readings(&s[0], EEP_RECORD_LENGTH);	
		
		//if successful get the reading. Proceed to next required parameter.
		if(result)
		{
			//if(current_reg < 5)
			current_reg++;	// Proceed to next required parameter if not in the end of the list yet.
			if(current_reg >= no_of_paras)
			{
				current_reg = 0;
				is_cycle_completed = TRUE;	// 
			}
		}
		
		//return EEPROM_Write_Readings(&s[5], 15);
		return result;
	}
	
	return FALSE;	
}
	
#endif

/*----------------------------------------------------------------------- 
	This function initialises pointers and flags for eeprom 
------------------------------------------------------------------------*/
void InitEEPROMSavePower(void)
{
	// Initialize all parameters
	WriteCursor = POWER_SAVE_HEAD;
	ReadCursor = POWER_SAVE_HEAD;
	current_reg = 0;				//2013-12-27 Liz added
	is_cycle_completed = FALSE;		//2013-12-27 Liz added
	free_space = CheckFreeSpace();
	no_of_paras = 1;				//2014-01-07 Liz added
}

//===========================================================
//	Data structure for Power readings to be stored in EEPROM
//		has followed format:
//	data[0] - IsDeleted_Flag
//		_ 0x00: data is deleted, can overwrite new data for this slot.
//		_ 0x01: data is available for reading, cannot be overwritten.
//	data[1] - IsDatetimeUpdated_Flag
//  data[2] - Checksum
//	data[3:8] - Date/Time
//	data[9:10] - Register
//	data[11-22] - Actual reading value (in BYTE array format)
//	
//	2013-12-27 Liz edited data format
//===========================================================
BOOL EEPROM_Write_Readings(char * source, char length)
{
	unsigned char is_success = 0, i = 0, checksum = 0;
	static int count = 0;
	
	// 2013-11-11 Liz added to check 0 reference
	if(source == 0)		return FALSE;
	//
	
	if(free_space < length)	return FALSE; // Stop writing if there is no space to store
	
	#if defined(EEPROM_CS_TRIS)

	// Calculate checksum
	for (i=0; i<length-3; i++)
	{
		checksum += source[i];
	}	
	
	// check if there is any data at the starting address
	do{
		// Come back to the starting point if WriteCursor reach the end of reserved block for Power_Save
		if(WriteCursor > (POWER_SAVE_END - length))
			WriteCursor = POWER_SAVE_HEAD;
				
		XEEBeginRead(WriteCursor);		
		if(XEERead() != 0x01)	// There is no data at starting address
		{
			BYTE flag = 0;

			XEEEndRead();
			if(meter_flag.Flags.Flag1.bDateTimeUpdated)
				flag = 1;
			else
				flag = 0;
			
			// write data from "source" to EEPROM with started addr defined by WriteCursor
			XEEBeginWrite(WriteCursor);
	    	XEEWrite(0x01);	 //	"0x01" means data is available for reading. 
	    	XEEWrite(flag);
	    	XEEWrite(checksum);
	    	XEEWriteArray(&source[0], length-3);	// Write actual data
	    	is_success = 1;
	    	free_space -= length;	// reduce free_space in memory
			unread_records++;			// increase number of records in eeprom
			meter_flag.Flags.Flag1.bHaveEepRecords = 1;	// Update meter_status flag
		}
		WriteCursor += length;
	}while((!is_success) && (count++<1000));	
	
	count = 0;
	
	if(is_success)
		return TRUE;
	else 
		return FALSE;
    #endif
}	

/*----------------------------------------------------------------------- 
	This function gets record from eeprom and send back to server. 
	Once the reading is read, the flag IsDeleted_Flag is not marked as "Deleted"
		yet. The flag will be update once meter receive ACK from server which
		acknowledge that reading is succesfully received. 
		
	Data structure for records get from EEPROM has followed format:
	data[0:2] - Position of record in eeprom
    data[1]	  -	IsDeleted_Flag
	data[2]	  - Checksum	
	data[3:8] - Date/Time
	data[9:10] - Register
	data[11-22] - Actual reading value (in BYTE array format)

	2013-12-27 Liz edited data format
------------------------------------------------------------------------*/
BOOL EEPROM_Read_Readings(char * rbuffer, char length)
{
	short int is_success = 0;
	static int count = 0;
	
	// 2013-11-11 Liz added to check 0 reference
	if(rbuffer == 0)	return FALSE;
	//
	
	if(free_space > MAX_FREE_SPACE - length) 
	{
		meter_flag.Flags.Flag1.bHaveEepRecords = 0;	// Update meter_status flag
		memcpypgm2ram(&rbuffer[0], "EOF", 3);
		
		return FALSE; // There is no stored data for reading
	}
	
	// Read data from EEPROM with started addr defined by ReadCursor
	#if defined(EEPROM_CS_TRIS)
	do{
		// Come back to the starting point if ReadCursor reach the end of reserved block for Power_Save
		if(ReadCursor > (POWER_SAVE_END - length))
			ReadCursor = POWER_SAVE_HEAD;
			
		XEEBeginRead(ReadCursor);
		if(XEERead() == 0x01)
		{   	
	    	// 2014-01-13 Liz. Add Eeprom version
	    	rbuffer[0] = '#';
	    	memcpy(&rbuffer[1], EEPROM_VERSION, 1);
	    	
	    	// Add position of record in eeprom
	    	memcpy(&rbuffer[2], &ReadCursor, 3);
	    	//memcpy(&rbuffer[0], &ReadCursor, 3);
	    	
	    	// Get actual record from eeprom
	    	ReadCursor++;
	    	XEEReadArray(ReadCursor, &rbuffer[5], length-1);	// Read actual data
	    	//XEEReadArray(ReadCursor, &rbuffer[3], length-1);	// Read actual data
	    	is_success = 1;
	    	//free_space += length + 1;
	    	ReadCursor += length-1;
	 	}   
	 	else
	    	ReadCursor += length;
	    
	}while(!is_success && count++<8000);
	
	count = 0;
	
	if(is_success)
		return TRUE;
	else 
		return FALSE;
    #endif
}	

/*----------------------------------------------------------------------- 
	This function checks free space in eeprom. 
	Note: current length assigned for each record is fixed at 16 bytes
------------------------------------------------------------------------*/
unsigned long CheckFreeSpace(void)
{
	unsigned long i;
	unsigned long result = MAX_FREE_SPACE;
	
	unread_records = 0;
	for(i = POWER_SAVE_HEAD; i < POWER_SAVE_END; i += EEP_RECORD_LENGTH)
	{
		//2012-10-18 Liz added to prevent watchdog timeout while stay in this loop
		ClrWdt();
		
		XEEBeginRead(i);
		if(XEERead() == 0x01)
		{
			result -= EEP_RECORD_LENGTH;
			unread_records++;
		}
		XEEEndRead();				
	}
		
	// Update meter_status flag
	if(unread_records > 0)
		meter_flag.Flags.Flag1.bHaveEepRecords = 1;	
	else
		meter_flag.Flags.Flag1.bHaveEepRecords = 0;
	
	return result;
}	

/*----------------------------------------------------------------------- 
	This function clears all records stored in eeprom.
	Note: current length assigned for each record is fixed at 16 bytes.
------------------------------------------------------------------------*/
void ResetMemory(void)
{
	unsigned long i;

	for(i = POWER_SAVE_HEAD; i < POWER_SAVE_END; i += EEP_RECORD_LENGTH)
	{
		//2012-10-18 Liz added to prevent watchdog timeout while stay in this loop
		ClrWdt();
		
		XEEBeginWrite(i);
	    XEEWrite(0x00);	 //	"0x00" means data is deleted. 
	    XEEEndWrite();			
	}	

	//Reset all cursors
	WriteCursor = POWER_SAVE_HEAD;
	ReadCursor = POWER_SAVE_HEAD;
	free_space = MAX_FREE_SPACE;
	unread_records = 0;	
	meter_flag.Flags.Flag1.bHaveEepRecords = 0;	// Update meter_status flag
	return;
}
	
/*----------------------------------------------------------------------- 
	This function delete records store in eeprom once it received ACK
		from server.
	Note: current length assigned for each record is fixed at 22 bytes.
------------------------------------------------------------------------*/
BOOL UpdateRecords(unsigned long location)
{
	// 2012-10-11 Liz removed. Shouldn't use ReadCursor here since it may crash 
	//		whole eeprom-read process if location is wrong	
	// Get location of sucessful-sent record
	//ReadCursor = location;
	//
	
	// 2012-10-19 Liz added location validation
	BOOL is_valid = FALSE;
	unsigned int remainder = 0;
	
	location = location & 0x00FFFFFF;
	remainder = location % 36;
	
	if(remainder == 21)
		is_valid = TRUE;
	else
	{
		is_valid = FALSE;
		return FALSE;
	}
	
	// Update number of available records
	if(unread_records > 0)
	{
		// Update IsDeleted_Flag
		XEEBeginWrite(location);
		XEEWrite(0x00);	 //	"0x00" means data is deleted. 
		XEEEndWrite();
		
		// 2012-10-11 Liz removed. Shouldn't use ReadCursor here since it may crash 
		//		whole eeprom-read process if location is wrong
		// Move ReadCursor to next record.
		// Come back to the starting point if ReadCursor reach the end of reserved block for Power_Save
		//if(ReadCursor > (POWER_SAVE_END - 16))
		//	ReadCursor = POWER_SAVE_HEAD;
		//else
		//	ReadCursor += 16;
		//
			
		unread_records--;
		free_space += EEP_RECORD_LENGTH;
	}
	
	// Update meter_status flag
	if(unread_records > 0)
		meter_flag.Flags.Flag1.bHaveEepRecords = 1;	
	else
		meter_flag.Flags.Flag1.bHaveEepRecords = 0;
		
	return TRUE;
}

/*----------------------------------------------------------------------- 
	2014-01-08 Liz added
	This function split 3 phase energy data into 3 single phases energy data 
		and write into eeprom.
	Note: current length assigned for each record is fixed at 22 bytes.
------------------------------------------------------------------------*/
void Split3P_ToSinglePhase(char * source)
{
	char datetime[6] = "", SPdata[22] = "";
	BYTE divide_by = source[0];
	char k = 0, i = 0;
	BOOL is_success = FALSE;
	
	//2014-05-07 Liz. make sure no reference to addr 0
	if(source == 0 || datetime == 0 || SPdata == 0)	return;
	
	// Get DateTime
	memcpy(&datetime[0], &source[1], 6);

	for(k=0; k<3; k++)
	{
		// Clear buffer
		memset(&SPdata[0], 0, 22);
		
		// Copy divide_by and datetime 
		memcpy(&SPdata[0], &datetime[0], 6);
		
		// Get register
		switch(k)
		{
			case 0:
			{
				SPdata[6] = 0xC1;
				SPdata[7] = 0x08;	
				i = 9;
			}
				break;
			case 1:
			{
				SPdata[6] = 0xC2;
				SPdata[7] = 0x08;
				i = 19;
			}
				break;
			case 2:
			{
				SPdata[6] = 0xC4;
				SPdata[7] = 0x08;
				i = 29;
			}
				break;
			default:
				break;			
		}	
		
		SPdata[8] = 10;
		SPdata[9] = divide_by;
		SPdata[10] = 0;
		
		// Get single phase energy
		memcpy(&SPdata[11], &source[i], 10);
		
		// Write to eeprom
		if(free_space > EEP_RECORD_LENGTH)
		{
			do{
				is_success = EEPROM_Write_Readings(&SPdata[0], EEP_RECORD_LENGTH);
			}while(!is_success);
		}		 		
	}		
}
		
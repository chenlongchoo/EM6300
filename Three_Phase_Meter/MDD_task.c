
#include <delays.h>
#include <timers.h>
#include <string.h>
#include <spi.h>

#include "stdlib.h"
#include "HardwareProfile.h"
#include "AppConfig.h"
#include "rtcc.h"

#include "eep.h"
#include "FSIO.h"
#include "MDD_task.h"
//#include "Power.h"
#include "registers.h"


/*	Variables	*/
char title[15] = "Date/Registers";
char new_line[2] = {13, 10};
char tap[1] = {9};
unsigned char printlist[5] = {1,2,7,10};
int s = 0;

/*==============================================================*/
/*	========== 		Main function of MDD_task	==============	*/

/*==============================================================*/

//////////////////////////////////////////////////////////////////////
//	Call MDD_Open() before opening file to write/read
//	Remember to call ResetSPIDevices() in main.c before MDD_OPEN()	
//////////////////////////////////////////////////////////////////////	
FSFILE * MDD_Open(char * filename)
{
	FSFILE * pointer1;
	char mode[] = "a";
	
	SD_CS_TRIS = 0;
	SD_CS_LAT = 1;
	SD_CS = 0;
	
	#if defined(__18F87J50)
		OpenSPI2(SPI_FOSC_64, MODE_00, SMPMID);
	#elif defined(__18F2455)
		ADCON1 = 0x0F;
		OpenSPI(SPI_FOSC_64, MODE_00, SMPMID);
	#elif defined(__18F26K20) || defined( __18F46K20 )
		OpenSPI(SPI_FOSC_64, MODE_00, SMPMID);
	#else
		#error "No processor defined."
	#endif
	
	while (!FSInit());
	// Create a file
//	pointer1 = FSfopenpgm ("FILE1.TXT", "a");
	pointer1 = FSfopen (filename, mode);
	if (pointer1 == NULL)
		return NULL;	// Cannot open file
	else	
		return pointer1;
}
	
//////////////////////////////////////////////////
//	Call MDD_Close() after closing file.
/////////////////////////////////////////////////		
char MDD_Close(FSFILE * fileptr)
{
	//	Close file
	if (FSfclose (fileptr))
		return -1;	// CAnnot close file
	
	#if defined(__18F87J50)
	CloseSPI2();
	#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
	CloseSPI();
	#else
		#error "No processor defined."
	#endif
	SD_CS_LAT = 1;
	
	return 1;
}	

/*	========	Write .txt file into SD card	==============	*/
/*	This function allows user to write string into SD card.    
	Remember to open and close file before and after write data.*/
	
#ifdef ALLOW_WRITES
//////////////////////////////////////////////////////////////////////
//	This function is used to write power reading into FILE1.TXT	
//		All the readings are written in BYTE format. May need another 
//		software to convert BYTE array into Char Array.
//////////////////////////////////////////////////////////////////////	
char MDD_write_power(FSFILE * fileptr)
{
	char date[7], i;
	int n = 0;
	
//	RTCC_Open();
	// Get the date and time when data is written.
//	RTCC_DateTimeToCharArray(&date[0]);
//	RTCC_GetDateTime(date);		
	for(i = 0; i<6; i++)
		date[i] = *((unsigned char *)DATE_TIME_POS[i]);
					
	// Write new line
	if (FSfwrite ((void *)new_line, 1, 2, fileptr) != 2)
		return -1;
	
	// Write date and time when data is written.
//	if (FSfwrite ((void *)date, 1, 12, fileptr) != 12)
//		return -1;	
	if (FSfwrite ((void *)date, 1, 6, fileptr) != 6)
		return -1;		
	
	if (FSfwrite ((void *)tap, 1, 1, fileptr) != 1)
		return -1;	
				
	// Write power reading data
#if defined SINGLE_PHASE_BOTTOM_BOARD
	for (n = 1; n < 2; n++)
#endif
#if defined THREE_PHASE_BOTTOM_BOARD
	for (n = 1; n < 4; n++)
#endif
	{
		//for (s = 0; s < POWER_REGISTERS_QUEUE_SIZE; s++)
		for (s = 0; s < 4; s++)
		{
			unsigned char t = printlist[s];
			//unsigned char length = POWER_REGISTERS_SIZE_QUEUE[n][t];
			int * jj = POWER_READINGS[n];
			long * kk = jj[t];
			
			if( kk != NULL )
			{	
				if((n>0) && (t>=10))	// Handle special case: energy readings.
					MDD_write(kk, 10, fileptr);
				else
					MDD_write(kk, POWER_REGISTERS_SIZE_QUEUE[n][t], fileptr);
			}	
		}
	}	

//	RTCC_Close();
	return 1;
}
/*
char MDD_write_title(FSFILE * fileptr)
{
	char number[5];
	
	// Write new line
	if (FSfwrite ((void *)new_line, 1, 2, fileptr) != 2)
		return -1;
	
	if (FSfwrite ((void *)title, 1, 14, fileptr) != 14)
		return -1;
	
	if (FSfwrite ((void *)tap, 1, 1, fileptr) != 1)
		return -1;
	
	// Write register's code				
	#ifdef SINGLE_PHASE_BOTTOM_BOARD
	for (s = 0; s < 16; s++)
	#elif defined THREE_PHASE_BOTTOM_BOARD
	for (s = 0; s < 38; s++)
	#endif
	{
		if( POWER_READINGS_0[s] != NULL )
		{
			ConvertIntToHexString(number, POWER_REGISTERS_QUEUE[s]);
			// Write Data.
//			MDD_write(number);	
			if (FSfwrite ((void *)number, 1, 5, fileptr) != 5)
				return -1;
			// Write a tap
			if (FSfwrite ((void *)tap, 1, 1, fileptr) != 1)
				return -1;
		}	
	}
	return 1;
}	
*/
//////////////////////////////////////////////////////////////////////
//	This function is used for general write string/byte into FILE1.TXT
//			
//////////////////////////////////////////////////////////////////////	
char MDD_write(char source[], char length, FSFILE * fileptr)
{
	unsigned int n = 0;
	
//	n = strlen(source);
	
	if (FSfwrite ((void *)source, 1, length, fileptr) != length)
		return -1;
	if (FSfwrite ((void *)tap, 1, 1, fileptr) != 1)
		return -1;
	else
		return 1;
}	
#endif	
/*==============================================================*/


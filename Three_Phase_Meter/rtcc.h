//rtcc.h

#ifndef __RTCC_H
#define __RTCC_H

//#include "p18f87j50.h"

	// DATETIME is the block of variables which stores the values of real time clock.
	typedef struct __attribute__((__packed__))
	{
		unsigned char	date;
		unsigned char	month;
		unsigned char	year;
		unsigned char	hour;
		unsigned char	minute;
		unsigned char	second;
		
		struct
		{
			unsigned char : 7;
			unsigned char bIsModified : 1;
		} Flags;                          	// Flag structure
	} DATETIME;

extern DATETIME		rtcc;
extern BOOL			EEP_RECORD_FLAG; 	// 2014-01-16 Liz
extern BOOL			EEP_TIMER_FLAG;		// 2014-04-22 Liz
	
	static unsigned char * DATE_TIME_POS[6] = 
	{
		&rtcc.date, &rtcc.month, &rtcc.year,
		&rtcc.hour, &rtcc.minute, &rtcc.second
	};
	
//
void InitRTCC(void);
void RTCC_SetDateTime(char * input);
char * RTCC_GetDateTime(char * dest);
char * RTCC_DateTimeToCharArray(char * results);
void HandleRTCC(void);
char not_leap_year(void);
void SaveRTCCToEEPROM(void);	// 2013-09-13 Liz added.
void ReadRTCCFromEEPROM(void);	// 2014-03-12 Liz added.
#endif  // #define __RTCC_H


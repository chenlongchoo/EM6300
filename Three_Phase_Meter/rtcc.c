//rtcc.c

#include "delays.h"
#include <stdio.h>
#include <string.h>

#include "HardwareProfile.h"

#include "rtcc.h"
#include "utility.h"
#include "eep.h"		// 2014-03-12 Liz added.

DATETIME	rtcc;
BOOL		EEP_RECORD_FLAG = FALSE; 	// 2014-01-16 Liz
BOOL		EEP_TIMER_FLAG = FALSE;		// 2014-04-22 Liz

void InitRTCC(void)
{
	// Initialize the calendar
	rtcc.second = 0;
	rtcc.minute = 1;
	rtcc.hour = 0;
	rtcc.date = 1;
	rtcc.month = 1;
	rtcc.year = 14;
}	

//This function takes a string and sets the date.
//Format of the string is ddmmyyhhmmss.
void RTCC_SetDateTime(char * input)
{
	char i = 0;
	unsigned char * rtc_data; 
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(input == 0 || rtc_data == 0)	return;
	//
	
	for(i=0; i<6; i++)
	{
		rtc_data = (unsigned char *)DATE_TIME_POS[i];

		*rtc_data = *input;
		input++;	
	}	
}

char * RTCC_GetDateTime(char * dest)
{
	char i = 0;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(dest == 0)	return dest;
	//
	
	for(i = 0; i<6; i++)
	{
		*dest = *((unsigned char *)DATE_TIME_POS[i]);	
		dest++;
	}
	return dest;
}	

char * RTCC_DateTimeToCharArray(char * results)
{
	char ch[3];
	char i = 0;
	char len = 0;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(results == 0)	return results;
	//
	
	for(i=0; i<6; i++)
	{	
		unsigned char * rtc_data = (unsigned char *)DATE_TIME_POS[i];
		
		// 2013-11-12 Liz. Check if refenrece is 0
		if(rtc_data == 0)	continue;
		//
		
		if(*rtc_data < 10)
		{	
			ch[0] = '0';
			itoa(*rtc_data, &ch[1]);
		}
		else
			itoa(*rtc_data, &ch[0]);
		strcat(results, ch);
	}	
	
	return results;
}

// This function will update the clock and calendar when there is an interrupt from Timer1.
void HandleRTCC(void)
{	
	if(++rtcc.second==60)
	{
		rtcc.second = 0;		
		if(++rtcc.minute==60)
		{
			rtcc.minute = 0;
			if(++rtcc.hour==24)
			{
				rtcc.hour=0;
				if(++rtcc.date==32)
				{
					rtcc.month++;
					rtcc.date=1;
				}	
				else if(rtcc.date==31)
				{
					if((rtcc.month==4) || (rtcc.month==6) || (rtcc.month==9) || (rtcc.month==11))
					{
						rtcc.month++;
						rtcc.date=1;
					}	
				}
				else if(rtcc.date==30)
				{
					if(rtcc.month==2)
					{
						rtcc.month++;
						rtcc.date=1;
					}	
				}
				else if(rtcc.date==29)
				{
					if((rtcc.month==2) && (not_leap_year()))
					{
						rtcc.month++;
						rtcc.date=1;
					}	
				}
				if(rtcc.month==13)
				{
					rtcc.month = 1;
					rtcc.year++;
				}				
			}	
		}	
		
		//if(rtcc.minute == 30)			
	}
	
	if((rtcc.minute % 30) == 0 )
	{
		// 2014-01-16 Liz. Keep track if it's time to record
		EEP_TIMER_FLAG = TRUE;
	}	
	else
	{
		if(EEP_TIMER_FLAG == TRUE)
			EEP_TIMER_FLAG = FALSE;
		
		if(EEP_RECORD_FLAG == TRUE)
			EEP_RECORD_FLAG = FALSE;
	}
}	

char not_leap_year(void)
{
	if(!(rtcc.year%100))
		return (char)(rtcc.year%400);
	else
		return (char)(rtcc.year%4);
}

/*-------------------------------------------------------------------------
	2013-09-13 Liz added.
	Save datetime to eeprom.
--------------------------------------------------------------------------*/
void SaveRTCCToEEPROM(void)
{
	char i = 0;
	BYTE * cnfg = (BYTE*)&rtcc;		

	if( (void*)cnfg == 0 ) return;
	
	// Write 0x80 to starting location
	while( Read_b_eep(150) != 0x80u )
	{
		Write_b_eep(150, 0x80u);  //0
		Busy_eep();
	}
	
	for( i=0; i<6; i++ )
	{
		// while loop to make sure the correct value was written in.
		while( Read_b_eep(i+151) != cnfg[i] )
		{
			Write_b_eep(i+151, cnfg[i]);  //0
			Busy_eep();
		}
	}
}

/*-------------------------------------------------------------------------
	2014-03-12 Liz added.
	Read datetime which was saved in eeprom.
--------------------------------------------------------------------------*/
void ReadRTCCFromEEPROM(void)
{
	BYTE * cnfg;
	BYTE temp = 0, i = 0;
	BYTE setTime[6] = {0x08,0x05,0x0E,0x07,0x02,0x05};
	BOOL is_data_correct = FALSE;
	
	//2012-11-14 Liz: read datetime stored in eeprom
	temp = Read_b_eep(150);
	if(temp == 0x80)
	{
		cnfg = (BYTE*)&rtcc;
		
		// 2014-05-07 Liz. Make sure no reference to addr 0
		if(cnfg == 0)	return;
			
		for( i=0; i<6; i++ )
		{
			do
			{
				cnfg[i] = Read_b_eep(i+151);
				temp = Read_b_eep(i+151);
				if(cnfg[i] == temp)	is_data_correct = TRUE;
				else				is_data_correct = FALSE;	
			} while(!is_data_correct);
		}
	}	
	else
		RTCC_SetDateTime(setTime);
}		
	

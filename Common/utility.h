#ifndef UTILITY_H
#define UTILITY_H

//#include "StackTsk.h"
#include "GenericTypeDefs.h"

#if defined(APP_USE_HUB)
BOOL StringToMACAddress(BYTE* ptr, MAC_ADDR* MACAddress);
#endif

#if defined(METER_TOP_BOARD)
	char * MACAddressToString(MAC_ADDR MACVal, char * str);
	BOOL IPAddressToString(IP_ADDR IPAddress, BYTE* str);
	char * Convert3RealEnergyToCharArray(char *dest, BYTE phase_start, BYTE mode);
	char * ConvertPowerReadingsToCharArray(BYTE phase, BYTE state, char *dest);	// 2013-10-03 Liz modified 
//	char * ConvertTotalRealEnergyToCharArray(char * dest);	//2013-12-19 Liz. Test.
#endif

char * LongToCharArray(unsigned long input, unsigned char length, char * output, unsigned char sign);
unsigned long MultiplyTwoLongNumbers(unsigned long num1, unsigned long num2);

/*** U N I T  C O N V E R S I O N **************************************/
/*****MATH FUNCTION**************/
unsigned int math_power(BYTE base, BYTE n);


// Only support BCD values up to 88 (1111 1111).
#if defined (SINGLE_PHASE_BOTTOM_BOARD) || defined (THREE_PHASE_BOTTOM_BOARD)
	char * CopyModbusPowerReadings(char phase, char state, char * dest);
	char * CopyPowerReadings(char phase, char state, char * dest);
	WORD PowerCalculateCALIBChecksum(void);	//2012-10-03 Liz modified
	void ComputeTotalRealEnergy(void);
//	char * Modbus_3P_THD(char * dest, BYTE mode);	//2013-04-29 Liz added
	char * Modbus3PEnergy(char * dest, BOOL use_time_stamp);	// 2014-01-07 Liz modified
	char * Modbus3PMaxDemand(char * dest);			//2013-05-05 Liz added
	char * Modbus3PReading(char * dest, char index);	//2013-05-23 Liz added
#endif

#endif

//utility.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "HardwareProfile.h"
#include "AppConfig.h"
#include "utility.h"

#if defined (SINGLE_PHASE_BOTTOM_BOARD) || defined (THREE_PHASE_BOTTOM_BOARD)	|| defined (METER_TOP_BOARD)
	#include "registers.h"
#endif

#if defined (SINGLE_PHASE_BOTTOM_BOARD) || defined (THREE_PHASE_BOTTOM_BOARD)
		#include "main.h"
		#include "rtcc.h"
#endif

#if defined( METER_TOP_BOARD ) || defined( COMMUNICATOR_BOARD ) 
	#include "TCPIP Stack\TCPIP.h"
//	#include "eeprom_task.h"
#endif

#if defined(APP_USE_HUB)
BOOL StringToMACAddress(BYTE* str, MAC_ADDR* MACAddress)
{
	// Read new MAC address
	WORD w;
	BYTE i;

	BYTE* ptr = str;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(ptr == 0 || str == 0 || MACAdress == 0)		return FALSE;
	//
	
	for(i = 0; i < 12u; i++)
	{// Read the MAC address
		
		// Skip non-hex bytes
		while( *ptr != 0x00u && !(*ptr >= '0' && *ptr <= '9') && !(*ptr >= 'A' && *ptr <= 'F') && !(*ptr >= 'a' && *ptr <= 'f') )
			ptr++;

		// MAC string is over, so zeroize the rest
		if(*ptr == 0x00u)
		{
			for(; i < 12u; i++)
				ptr[i] = '0';
			break;
		}
		
		// Save the MAC byte
		str[i] = *ptr++;
	}
	
	// Read MAC Address, one byte at a time
	for(i = 0; i < 6u; i++)
	{
		((BYTE*)&w)[1] = str[i*2];
		((BYTE*)&w)[0] = str[i*2+1];
		MACAddress.v[i] = hexatob(*((WORD_VAL*)&w));
	}
	
	return TRUE;
}

#endif

#if defined(METER_TOP_BOARD)
BOOL IPAddressToString(IP_ADDR IPAddress, BYTE* str)
{
	char i = 0;
	char tmp[4];
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(str == 0)		return FALSE;
	//
	
	str[0] = 0;
	for( i=0; i<4; i++ )
	{
		itoa(IPAddress.v[i], tmp);
		strcat(str, tmp);
		if( i != 3 ) strcatpgm2ram(str, ".");
	}	
}

char * MACAddressToString(MAC_ADDR MACVal, char * str)
{
	char i = 0, j = 0, t = 0;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(str == 0)	return str;
	//
	
	for( i=0; i<6; i++ )
	{
		for(j=2; j>0; j--)
		{
			char c = (MACVal.v[i] & 0x0F);
			if( c > 9 && c < 16 ) c += 55;
			else c += 48;
	
			MACVal.v[i] >>= 4;
			str[i*2+j-1] = c;
		}
	}	
	str[12] = 0;
	return str;
}	
#endif	

#if defined( METER_TOP_BOARD )
char ConvertCharToHex(char c1)
{
	if( c1 >= '0' && c1 <= '9' ) return c1 - 48;
	if( c1 >= 'A' && c1 <= 'F' ) return c1 - 55;
	return 0;
}	
#endif

/***********MATH FUNCTIONS******************************/
// 2012-04-30(Eric) - Multiples base by itself n times.
// We write this custom function as the math.h library takes up too much code space.
unsigned int math_power(BYTE base, BYTE n)
{
	BYTE i=0;
	unsigned int result = 1;

	// 2014-05-07 liz added.
	if(n == 0)	return 1;

	for (i=0; i<n; i++)
		result *= base;
	return result;
}

/***************************************************************/

/*** Data Type Conversion **************************************/
#if defined( METER_TOP_BOARD )
char * LongToCharArray(unsigned long input, unsigned char length, char * output, unsigned char sign)
{
	// 2013-11-12 Liz. Check if refenrece is 0
	if(output == 0)	return output;
	//
	
	switch( length )
	{
		case 1: 
			if(sign)	
				itoa(input & 0x000000FF, output); 
			else
				ultoa(input & 0x000000FF, output);
			break;
		case 2: 
			if(sign)
				itoa(input & 0x0000FFFF, output); 
			else
				ultoa(input & 0x0000FFFF, output); 
			break;
//		case 3: ltoa(input & 0x00FFFFFF, output); break;
		case 4:
		case 8: 
			if(sign)
				ltoa(input, output); 
			else
				ultoa(input, output);
			break;
	}
	return output;
}
#endif

// This function works specifically for energy readings only.
unsigned long MultiplyTwoLongNumbers(unsigned long num1, unsigned long num2)
{
	unsigned long temp1 = 0, temp2 = 0;
	unsigned long result = 0;
	
	temp1 = (num1 & 0x0000FFFF);
	temp2 = (num2 & 0x0000FFFF);
	result = (temp1*temp2)>>16;
	
	
	temp1 = (num1 & 0xFFFF0000)>>16;
	result += temp1*temp2;
	
	temp1 = (num1 & 0x0000FFFF);
	temp2 = (num2 & 0xFFFF0000);
	result += temp1*temp2;
	result >>= 16;
	
	temp1 = (num1 & 0xFFFF0000);
	result += temp1*temp2;
	
	// 2012-04-18(Eric) - Most significant 4-bytes of the results is dropped.
	return result; 
}	

//
/*** U N I T  C O N V E R S I O N **************************************/

#if defined (SINGLE_PHASE_BOTTOM_BOARD) || defined (THREE_PHASE_BOTTOM_BOARD)
// This function copy the power readings into RX_BUFFER[] before it's being sent back to top board.
// The final format returned by this function is
// +--------+-----------+--------+----------+----------+---------+
// | Length | Divide By |  Sign  | DateTime | Register | Reading |
// | 1-byte |  1-byte   | 1-byte |  6-bytes |  2-bytes |         |
// +--------+-----------+--------+----------+----------+---------+
char * CopyPowerReadings(char phase, char state, char * dest)
{
	char i = 0;
	char date[6] = "";
	int * phase_storage = POWER_READINGS[phase];
	char * storage = phase_storage[state];
	char length = POWER_REGISTERS_SIZE_QUEUE[phase][state];
	unsigned short reg = POWER_REGISTERS_QUEUE[phase][state];
	
	// 2013-11-12 Liz. Check if refenrece is 0
	//if(dest == 0)	while(1);
	if(dest == 0)	return dest;
    if(storage == 0)	return dest;
	//
	
	// Handle special case: energy reading has length of 12 bytes
	if( (reg&0x0FF0) == 0x0870 ||
		(reg&0x0FF0) == 0x08C0 ||
		(reg&0x0FF0) == 0x08D0 )
		length = 10;
	
	dest[0] = length;
	
	if(phase == 0)  // Miscellaneous.
	{
		dest[1] = POWER_REGISTERS_DIVIDE_BY_M[state];
		dest[2] = POWER_SIGNED_REGISTERS_M[state];
	}
	else
	{
		dest[1] = POWER_REGISTERS_DIVIDE_BY_A[state];	
		dest[2] = POWER_SIGNED_REGISTERS_A[state];
	}	
	
	{
		// Add in datetime.
		for(i = 0; i<6; i++)
			dest[i+3] = *((unsigned char *)DATE_TIME_POS[i]);	//date[i];
			
		// Add in register.
		dest[9] = (reg & 0xFF00)>>8;
		dest[10] = (reg & 0x00FF);
			
		for (i = 0; i < length; i++)
			dest[i+11] = *storage++;
	}
		
	return dest;
}		

// This is a clone of the function CopyPowerReadings modified to take out the overheads.
// The final format returned by this function is
// +--------+-----------+--------+---------+
// | Length | Divide By |  Sign  | Reading |
// | 1-byte |  1-byte   | 1-byte |         |
// +--------+-----------+--------+---------+
char * CopyModbusPowerReadings(char phase, char state, char * dest)
{
	char i = 0;

	int * phase_storage = POWER_READINGS[phase];
	char * storage = phase_storage[state];
	char length = POWER_REGISTERS_SIZE_QUEUE[phase][state];
	unsigned short reg = POWER_REGISTERS_QUEUE[phase][state];
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(dest == 0)	return dest;
	if(storage == 0)	return dest;
	//
	
	// Handle special case: energy reading has length of 10 bytes.
	if( (reg&0x0FF0) == 0x0870 ||
		(reg&0x0FF0) == 0x08C0 ||
		(reg&0x0FF0) == 0x08D0 )
		length = 10;
	
	dest[0] = length;
	
	if(phase == 0)
	{
		dest[1] = POWER_REGISTERS_DIVIDE_BY_M[state];
		dest[2] = POWER_SIGNED_REGISTERS_M[state];
	}
	else
	{
		dest[1] = POWER_REGISTERS_DIVIDE_BY_A[state];	
		dest[2] = POWER_SIGNED_REGISTERS_A[state];
	}	
	
	for (i = 0; i < length; i++)
		dest[i+3] = *storage++;
		
	return dest;
}		

/*--------------------------------------------------------------------
	2012-05-09 Liz added.
	2012-10-03 Liz: Modified
	This function calculates checksum of calibration data.
--------------------------------------------------------------------*/
WORD PowerCalculateCALIBChecksum(void)
{
	BYTE i = 0, checksum = 0;	
	BYTE * cnfg = (BYTE*)&CalibrationData;
	
	// Clear checksum before calculation
	checksum = 0;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(cnfg == 0)	return checksum;
	//
	
	for( i=0; i<26; i++ )	
		checksum += cnfg[i];
	
	return checksum;
}

/*-----------------------------------------------------------------
	2012-11-21 Liz added.
	This function form a packet data which inlcude energy reading
		for 3 phase.
	2014-01-07 Liz added variable "use_time_stamp"
		+ use_time_stamp = TRUE: include DateTime in data reading
		+ use_time_stamp = FALSE: exclude DateTime in data reading
-----------------------------------------------------------------*/
char * Modbus3PEnergy(char * dest, BOOL use_time_stamp)
{
	char i = 0, j = 0, k = 0;

	int * phase_storage;	// = POWER_READINGS[phase];
	char * storage;	// = phase_storage[state];
	//char length = POWER_REGISTERS_SIZE_QUEUE[phase][state];
	//unsigned short reg = POWER_REGISTERS_QUEUE[phase][state];
		
	if(use_time_stamp)
	{	
		k = 9;
			
		// Add datetime.
		for(i = 0; i<6; i++)
			dest[i] = *((unsigned char *)DATE_TIME_POS[i]);	//date[i];

		// 2014-06-10 Liz. Add bIsGetDatetime flag to indicate if Datetime is valid
		if(CalibrationData.Flag2.bIsGetDatetime == 1)
			dest[6] = 1;
		else
			dest[6] = 0;
			
		// Add register
		dest[7] = 0x09;
		dest[8] = 0x11;
		dest[9] = POWER_REGISTERS_DIVIDE_BY_A[10];
	}
	else
	{
		k = 0;
		dest[0] = POWER_REGISTERS_DIVIDE_BY_A[10];
	}
	
	for( j = 0; j < 3; j++)
	{
		phase_storage = (int *)POWER_READINGS[j+1];
		storage = (char *)phase_storage[10];
		
		// Eric(2013-05-16) - Dont do anything if pointer value is 0.
		if( storage == 0 )	return dest;	
					
		memcpy(&dest[j*10+1+k], storage, 10);
//		for (i = 0; i < 10; i++)
//			dest[j*10+i+1] = *storage++;
	}	
	return dest;
}		


/*-----------------------------------------------------------------
	2013-05-05 Liz added.
	This function form a packet data which inlcude max demand reading
		for 3 phase.
-----------------------------------------------------------------*/
char * Modbus3PMaxDemand(char * dest)
{
	// 2013-11-11 Liz added to check 0 reference
	if(dest == 0)	return dest;
	//
	
	dest[0] = POWER_REGISTERS_DIVIDE_BY_A[13];	
	memcpy(&dest[1],&max_min_storage, sizeof(MAX_MIN_STORAGE));

	return dest;
}

/*-----------------------------------------------------------------
	2013-05-23 Liz added.
	This function form a packet data which inlcude energy reading
		for 3 phase.
-----------------------------------------------------------------*/
char * Modbus3PReading(char * dest, char index)
{
	char i = 0, j = 0;
	char length = 0;
	
	int * phase_storage;	// = POWER_READINGS[phase];
	char * storage;	// = phase_storage[state];
	//char length = POWER_REGISTERS_SIZE_QUEUE[phase][state];
	//unsigned short reg = POWER_REGISTERS_QUEUE[phase][state];

	// 2013-11-11 Liz added to check 0 reference
	if(dest == 0)	return dest;
	//

	dest[0] = POWER_REGISTERS_DIVIDE_BY_A[index];	
	length = POWER_REGISTERS_SIZE_QUEUE[1][index];
	
	for( j = 0; j < 3; j++)
	{
		phase_storage = (int *)POWER_READINGS[j+1];
		storage = (char *)phase_storage[index];
		
		// 2013-11-12 Liz. Check if refenrece is 0
		if (storage == 0)	return dest;
		//
		
		if(length >= 8)					
			memcpy(&dest[j*4+1], &storage[2], 4);
		else
			memcpy(&dest[j*4+1], &storage[0], length);
	}	
	return dest;
}		

/*-----------------------------------------------------------------
// 2012-09-25 Liz added
// This function is called by bottom board to calculate total energy.
-----------------------------------------------------------------*/
void ComputeTotalRealEnergy(void)
{
	BYTE i = 0, divide_by = 0;
	unsigned int power_factor_10 = 1, carrier = 0, total_carrier = 0;
	char string_result[4];

	unsigned long hresults = 0, lresults = 0, hresults_total = 0, final_result = 0;
	unsigned long ovfc2_max = 0;

	for ( i = 1; i < 4; i++) // Convert total energy reading
	{
		int * jj = POWER_READINGS[i];
		long * kk = jj[10];  // Storage is the location where the reading will be saved. 
		divide_by = POWER_REGISTERS_DIVIDE_BY_A[10];	
		
		// 2013-11-12 Liz. Check if refenrece is 0
		if(kk == 0)	return;
		//
		
		// Read reading value.
		hresults = kk[1];
		lresults = kk[0];
		
		// 2012-12-07 Liz: dont compute total energy if all 3 phases havent been updated yet.
		if( hresults == 0 && lresults == 0 )
		{
			EnrOvfCounter1.Flags.bIs_3Pkwh_Ready = 0;
			return;
		}
		else
			EnrOvfCounter1.Flags.bIs_3Pkwh_Ready = 1;
				
		{
			// Higher 4-bytes of lresults is ENR_CC.
			unsigned long enr_cc = (lresults&0xffff0000)>>16;
			unsigned long ovfc1 = (lresults&0x0000ffff);  // Counter 1.
			unsigned long ovfc2 = (kk[2]&0x0000ffff);	  // Counter 2.
			
			hresults_total = MultiplyTwoLongNumbers(hresults, enr_cc) + (ovfc1 * enr_cc);
			
			// Rounded to 2 decimal places
			power_factor_10 = math_power(10, divide_by-2);		// Get the number of decimal places needed to be dropped
			ovfc2_max = 5*power_factor_10;						// Get the value of ovfc2 which kwh_value reach 1000000000
																//	simplified from 1000000000/(200000000/power_factor_10)	
			carrier = ovfc2 / ovfc2_max;		// get carrier
			ovfc2 = ovfc2 - carrier*ovfc2_max;	// get remainer
			hresults_total /= power_factor_10;
			hresults_total += ovfc2*(200000000/power_factor_10);

		}	 
		final_result += hresults_total;
		total_carrier += carrier;
	}
		
	// Abstract carrier once more time if final_result > 1000000000
	if(final_result > 1000000000)
	{
		carrier = final_result/1000000000;
		total_carrier += carrier;
		final_result = final_result - carrier*1000000000;
	} 
	
	// Update total real energy
	{
		int * jj = POWER_READINGS[0];
		long * kk = jj[10];  // Storage is the location where the reading will be saved. 
		
		// Eric(2013-05-16) - Dont do anything if pointer value is 0;
		if( jj != 0 && kk != 0 ) {
			kk[0] = (final_result & 0xFFFFFFFF);
			kk[1] = (total_carrier & 0x0000FFFF);
		}	
	}		
}	
/////////////

#endif

#if defined( METER_TOP_BOARD )
// This function will be called in top board in order convert power readings to char array.
// 2012-04-30(Eric) - When mode == 0, we convert energy reading for just one phase. The phase being converted depends on phase_start.
//					- When mode == 1, we convert energy reading for 3 phases total.
// 2014-07-01 Liz modified. When mode == 1, we directly convert total energy reading of 3 phases which has been pre-calculated in bottom board.	
char * Convert3RealEnergyToCharArray(char *dest, BYTE phase_start, BYTE mode)
{
	BYTE i = 0, divide_by = 0, phase_end = 0;
	unsigned int power_factor_10 = 1, carrier = 0, total_carrier = 0;
	char string_result[17];

	unsigned long hresults = 0, lresults = 0, hresults_total = 0, final_result = 0;
	unsigned long ovfc2_max = 0;
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(dest == 0)	return dest;
	//
	
	// 2014-07-02 Liz removed	
//	if(mode == 0)	// Convert single phase energy reading
//		phase_end = phase_start + 1;
//	else
//		phase_end = 4;

	// 2014-07-02 Liz modified.
//	for ( i = phase_start; i < phase_end; i++) // Convert total energy reading
	if(mode == 0)
	{
		// Convert single phase energy reading
		//int * jj = POWER_READINGS[i];
		int * jj = POWER_READINGS[phase_start];
		long * kk = jj[10];  // Storage is the location where the reading will be saved. 
		divide_by = POWER_REGISTERS_DIVIDE_BY_A[10];	
		
		// 2013-11-12 Liz. Check if refenrece is 0
		if(kk == 0)	return dest;
		//
		
		// Read reading value.
		hresults = kk[1];
		lresults = kk[0];
		
		// This will prevent rubbish from being shown on the screen when total KWH is displayed for the first time.
		if( hresults == 0 && lresults == 0 )
		{
			final_result = 0;
			total_carrier = 0;
		}	
		else
		{
			// Higher 4-bytes of lresults is ENR_CC.
			unsigned long enr_cc = (lresults&0xffff0000)>>16;
			unsigned long ovfc1 = (lresults&0x0000ffff);  // Counter 1.
			unsigned long ovfc2 = (kk[2]&0x0000ffff);	  // Counter 2.
			
			hresults_total = MultiplyTwoLongNumbers(hresults, enr_cc) + (ovfc1 * enr_cc);
			
			// Rounded to 2 decimal places
			power_factor_10 = math_power(10, divide_by-2);		// Get the number of decimal places needed to be dropped
			ovfc2_max = 5*power_factor_10;						// Get the value of ovfc2 which kwh_value reach 1000000000
																//	simplified from 1000000000/(200000000/power_factor_10)	
			carrier = ovfc2 / ovfc2_max;		// get carrier
			ovfc2 = ovfc2 - carrier*ovfc2_max;	// get remainer
			hresults_total /= power_factor_10;
			hresults_total += ovfc2*(200000000/power_factor_10);

			final_result += hresults_total;
			total_carrier += carrier;
		}	 

		// Abstract carrier once more time if final_result > 1000000000
		if(final_result > 1000000000)
		{
			carrier = final_result/1000000000;
			total_carrier += carrier;
			final_result = final_result - carrier*1000000000;
		} 
	}
	else if (mode == 1)
	{
		// Convert total energy reading
		int * jj = POWER_READINGS[0];
		long * kk = jj[10];  // Storage is the location where the reading will be saved. 
		divide_by = POWER_REGISTERS_DIVIDE_BY_A[10];

		// Read reading value.
		hresults = kk[1];
		lresults = kk[0];

		final_result = (lresults&0xffffffff);
		total_carrier = (hresults&0x0000ffff); 
	}
			
	// Convert reading value from long to Char Array.
	LongToCharArray((unsigned long)final_result, 4, string_result, 0);	// If energy discard 32 bits LSB; sign is always 0.
	
	if(total_carrier > 0)
	{
		char zero[2] ={'0',0}; 
		itoa(total_carrier,dest);
		if(strlen(string_result) < 9)
		{
			for(i = 0; i < (9-strlen(string_result)); i++)	
				strcat(dest, zero);
		}
	}
	else
		dest[0] = 0;
	strcat(dest, string_result);
	divide_by = 2;

	// Filling in decimal places.
	if( divide_by != 0 )
	{
		char t_len = 0, k = 0;
		t_len = strlen(dest);
		if(t_len > divide_by) // If length of reading is > the number of times the decimal is moved...
		{	
			dest[t_len+1] = 0;
			for( k=0; k<divide_by; k++ )
				dest[t_len-k] = dest[t_len-k-1];
			dest[t_len-divide_by] = '.';
		}
		else
		{
			dest[divide_by + 2] = 0;
			for( k=0; k<t_len; k++ )
				dest[divide_by-k+1] = dest[t_len-k-1];
			dest[0] = '0';
			dest[1] = '.';
			for( k = divide_by - t_len+1; k>1; k--)
				dest[k] = '0';
		}
	}

	return dest;
}	

char * ConvertPowerReadingsToCharArray(BYTE phase, BYTE state, char *dest)
{
	char i = 0, length = 0, divide_by = 0, sign = 0;
	//char length = POWER_REGISTERS_SIZE_QUEUE[phase][state];
	int temp = 0;
	int * jj = POWER_READINGS[phase];
	long * kk = jj[state];  // Storage is the location where the reading will be saved.
	unsigned short reg = 0;
	unsigned long hresults = 0, lresults = 0;

	// 2013-11-12 Liz. Check if refenrece is 0
	if(dest == 0)	return dest;
	if(kk == 0)		return dest;
	//
	
	memcpypgm2ram(&length, &POWER_REGISTERS_SIZE_QUEUE[phase][state], 1);
	memcpypgm2ram(&reg, &POWER_REGISTERS_QUEUE[phase][state], 2);
	
	if(phase == 0)
	{
		divide_by = POWER_REGISTERS_DIVIDE_BY_M[state];
		sign = POWER_SIGNED_REGISTERS_M[state];
	}
	else
	{
		divide_by = POWER_REGISTERS_DIVIDE_BY_A[state];	
		sign = POWER_SIGNED_REGISTERS_A[state];
	}	
		
	// Read reading value.
	if (length == 8)
	{
		hresults = kk[1];
		lresults = kk[0];	
	}
	else
	{
		hresults = 0;
 		lresults = kk[0];	
	}	
	
	// Convert value to percentage if it is power_factor
	if((reg == 0x1C6) || (reg == 0x2B2) || (reg == 0x39E))
	{
		int temp = fabs(lresults & 0xFFFF);
		lresults = temp;
		lresults = lresults*10000/16384;
	}		
	
	// Accumulate the actual value for energy readings
	if( (reg&0x0FF0) == 0x0870 ||
		(reg&0x0FF0) == 0x08C0 ||
		(reg&0x0FF0) == 0x08D0 )
	{
		Convert3RealEnergyToCharArray(dest, (BYTE)phase, 0);
	}
	else
	{
		// Convert reading value from long to Char Array.
		if (length == 8)
		{
			LongToCharArray(((hresults << 16) | (lresults >> 16)), 8, dest, sign); // else discard 16 bits LSB
		}	
		else
			LongToCharArray(lresults, length, dest, sign);
		
		// Filling in decimal places.
		if( divide_by != 0 )
		{
			char t_len = 0, k = 0;
			t_len = strlen(dest);
			if(t_len > divide_by) // If length of reading is > the number of times the decimal is moved...
			{	
				if((dest[0] == '-') && (divide_by == t_len - 1))
				{
					dest[t_len+2] = 0;
					for( k=0; k<divide_by; k++ )
						dest[t_len-k+1] = dest[t_len-k-1];
					dest[0] = '-';
					dest[1] = '0';
					dest[2] = '.';
				}	
				else
				{
					dest[t_len+1] = 0;
					for( k=0; k<divide_by; k++ )
						dest[t_len-k] = dest[t_len-k-1];
					dest[t_len-divide_by] = '.';
				}
			}
			else
			{
				if(dest[0] == '-')  // If negative value...
				{
					dest[divide_by+3] = 0;
					for( k=0; k<t_len-1; k++ )
						dest[divide_by-k+2] = dest[t_len-k-1];
					dest[0] = '-';
					dest[1] = '0';
					dest[2] = '.';
					for( k = divide_by - t_len+3; k>2; k--)
						dest[k] = '0';
				}	
				else	
			   	{
					dest[divide_by + 2] = 0;
					for( k=0; k<t_len; k++ )
						dest[divide_by-k+1] = dest[t_len-k-1];
					dest[0] = '0';
					dest[1] = '.';
					for( k = divide_by - t_len+1; k>1; k--)
						dest[k] = '0';
				}
			}
		}
	}	
	return dest;
}

#endif

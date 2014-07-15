//power.c

#include "HardwareProfile.h"
#include "AppConfig.h"
#include "main.h"	//2013-02-27 Liz added
#include "mcu.h"
#include "power.h"
#include "registers.h"
#include "utility.h"
#include "rtcc.h"	//2013-12-27 Liz added
#include <delays.h>
#include <spi.h>

/*****************************************/
/********* P R O T O T Y P E S ***********/
//
unsigned char PowerReadChar(void);
unsigned char PowerWriteChar(char c);
unsigned char PowerIsBufferFull(void);
static char fail_maxq_count = 0;
static char error_list[10];
static BOOL bReadyToRead = 0;

void PowerOpen(void)
{
	POWER_SCK_TRIS = 0;
	POWER_MOSI_TRIS = 0;
	POWER_MISO_TRIS = 1;
	POWER_CS_TRIS = 0;
	POWER_CS_LAT = 1;
	
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
}

// Applications should call this function. This wraps all the necessary formulas and returns the final value
// required.
char PowerGetReadings(MCU_REQUEST_BLOCK* request)
{
	char i, r=0, retries=0;
	short reg = (*request).w.header.start_addr;
	BYTE sg[30];
	WORD startsWith = (*request).w.header.start_addr&0x0FF0;
//	static long * max_value;
	// 2013-11-11 Liz added to check 0 reference
	if(request == 0)	return 0;
	//
	
	(*request).w.IsRawValue = 1;
	r = 1;
	// CopyModbusPowerReadings has an overhead of 3 bytes - length, divide_by and sign.
	switch(reg)
	{
		case 0x0911:	//2012-11-22 Liz added
		{
			// 2013-02-27(Liz) Dont return value if total 3P kwh is not ready
			if(EnrOvfCounter1.Flags.bIs_3Pkwh_Ready != 1)
				break;
			else
				Modbus3PEnergy(&(*request).w.data, FALSE);
		}
			break;
		case 0x08C1:
			CopyModbusPowerReadings(1, 10, sg);
			break;
		case 0x08C2:
			CopyModbusPowerReadings(2, 10, sg);
			break;
		case 0x08C4:
			CopyModbusPowerReadings(3, 10, sg);
			break;
		case 0x08D1:
			CopyModbusPowerReadings(1, 12, sg);
			break;
		case 0x08D2:
			CopyModbusPowerReadings(2, 12, sg);
			break;
		case 0x08D4:
			CopyModbusPowerReadings(3, 12, sg);
			break;
		case 0x0871:
			CopyModbusPowerReadings(1, 11, sg);
			break;
		case 0x0872:
			CopyModbusPowerReadings(2, 11, sg);
			break;
		case 0x0874:
			CopyModbusPowerReadings(3, 11, sg);
			break;
		case 0x0950:
		case 0x0951:
		case 0x0952:
		{
			BYTE location = reg - 0x094F; 
			CopyModbusPowerReadings(location, 13, sg);
		}
			break;
		case 0x08C7:
		{	
			// 2013-02-27(Liz) Dont return value if total 3P kwh is not ready
			if(EnrOvfCounter1.Flags.bIs_3Pkwh_Ready != 1)
				break;	
			else
				CopyModbusPowerReadings(0, 10, sg);
		}
			break;
		case 0x0970:  // Eric(2012-09-20) - Get calibration data.
		{
			memcpy(&(*request).w.data, &CalibrationData, ((*request).w.header.ByteCount*2));
			(*request).w.IsRawValue = 1;
		}
			break;	
		default:
		{
			// Form 3 Phase reading
			FormThreePhaseReadings(reg, &sg[0]);
			
			// Copy to modbus request
			memcpy(&(*request).w.data, &sg[8], 15);
		}
			break;
//		default:
//		{
//			if(startsWith == 0x0830)
//				(*request).w.data.Others[0] = POWER_REGISTERS_DIVIDE_BY_A[1];
//			if(startsWith == 0x0840)
//				(*request).w.data.Others[0] = POWER_REGISTERS_DIVIDE_BY_A[2];
//			if(startsWith == 0x0800 || startsWith == 0x0810 || startsWith == 0x0820)
//				(*request).w.data.Others[0] = POWER_REGISTERS_DIVIDE_BY_A[7];
//			//if(startsWith == 0x08C0 || startsWith == 0x08D0 || startsWith == 0x0870) (*request).w.raw[0] = POWER_REGISTERS_DIVIDE_BY_A[10];
//
//			PowerOpen();
//			
//				do{
//					r = PowerSendReadCommand((*request).w.header.start_addr, 
//							(*request).w.header.ByteCount, &(*request).w.data.hResult, 
//							&(*request).w.data.lResult);
//					
//					if(r!=1)
//					{
//						// When there is an error talking to MAXQ3180...
//						#if (CLOCK_SPEED == 32000000)
//							Delay100TCYx(20);	//2012-10-15 Liz: changed from 10 to 20. Requesting too fast makes MAXQ unable to response
//						#else
//							#error "No valid clock speed defined."
//						#endif
//					}	
//				}while(retries<3 && r!=1);	
//			
//			PowerClose();
//			
//			(*request).w.IsRawValue = 0;
//
//			break;
//		}	
	}

	if(startsWith == 0x08C0 || startsWith == 0x08D0 || startsWith == 0x0870 )	//|| startsWith == 0x0950)
		memcpy(&(*request).w.data, &sg[0], sg[0]+3);
				
	return r;
}

// This function is for internal use only. Applications should refrain from calling this method.
// Use PowerGetReadings instead.
//
// Data is read LSB first.
// Error codes.
// -1 = First command did not receive 0xC1 as response.
// -2 = Second command did not recevie 0xC2 as response.
// -3 = Did not receive ACK or NACK after sending commands to MAXQ3180.
// -4 = MAXQ3180 has dropped transaction while sending data.
// -5 = Invalid data length specified in command byte.
// -6 = MAXQ3180 did not ACK the commands that were sent to it.
// -7 = No response.
// 2012-05-13(Eric) - Multiple changes made to this function because when error is occurred in the function,
// interrupts were not enabled back.
char PowerSendReadCommand(short address, char data_length, long * result_high, long * result_low)
{
	char response[8];
	unsigned int retries, retries1, data_count;
	char command1 = 0, command2 = 0;
	char result_array[8] = {0,0,0,0,0,0,0,0};
	long lresult;
//	unsigned char d = 40;	//2014-06-16 Liz removed
	char error_flag = 0;

	// 2013-11-12 Liz. Check if refenrece is 0
	if(result_high == 0 || result_low == 0)	return 0;
	//
	
	*result_high = *result_low = 0;
	
	// Disable interrupts
	INTCONbits.PEIE = 0;
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;

	//Build the command1.
	switch( data_length )
	{
		case 1: break;
		case 2: command1 = 0x10; break;
		case 4: command1 = 0x20; break;
		case 8: command1 = 0x30; break;
		default:
		{
			command1 = 0x30;
			data_length = 8;
			//error_flag = -5;
			//goto PowerReadFailure;
		}
	}
	
	command1 |= ((address & 0xF00) >> 8);
	command2 = (address & 0x0FF);
	
	POWER_CS_LAT = 1;  // Deactivate the chip.
	#if( CLOCK_SPEED == 16000000 )
		Delay10TCYx(14);
	#elif( CLOCK_SPEED == 32000000 )
//		Delay10TCYx(d);		//2014-06-16 Liz removed
		Delay10TCYx(40);	//2014-06-16 Liz
	#else
		#error "Processor clock speed not defined."
	#endif
	POWER_CS_LAT = 0;  // Activate the chip.
	//while( PORTCbits.RC2 ) POWER_CS_LAT = 0;  // Activate the chip.
		
	// Delay to make sure that the MAXQ3180 does not
	// mistaken this command to be part of a previous
	// incomplete command.

	// Send first command byte. 0xC1 will be received.
	retries = 0;  while((0 != (PowerWriteChar(command1))) && (retries++ < 20));
	if(!POWER_INT_FLAG)
	{
		error_flag = -7;
		goto PowerReadFailure;
	}	
	response[0] = POWER_RX_BUFFER;
	if(POWER_RX_BUFFER != 0xC1)
	{
		error_flag = -1;
		goto PowerReadFailure;
	}	
	#if( CLOCK_SPEED == 16000000 )
		Delay10TCYx(14);
	#elif( CLOCK_SPEED == 32000000 )
//		Delay10TCYx(d);		//2014-06-16 Liz removed
		Delay10TCYx(40);	////2014-06-16 Liz
	#else
		#error "Processor clock speed not defined."
	#endif
	
	// Send second command byte. 0xC2 will be received.
	retries = 0;  while((0 != (PowerWriteChar(command2))) && (retries++ < 20));
	response[1]=POWER_RX_BUFFER;
	if(POWER_RX_BUFFER != 0xC2)
	{
		error_flag = -2;
		goto PowerReadFailure;
	}	
	#if( CLOCK_SPEED == 16000000 )
		Delay10TCYx(14);
	#elif( CLOCK_SPEED == 32000000 )
//		Delay10TCYx(d);		//2014-06-16 Liz removed
		Delay10TCYx(40);	//2014-06-16 Liz
	#else
		#error "Processor clock speed not defined."
	#endif

	// Wait for MAXQ3180 to complete its processing of the commands.
	// Basically loop until ACK(0x41) is received.
	retries = 0;
	do
	{
		if( retries++>20 )
		{
			error_flag = -6;
			goto PowerReadFailure;
		}	
		retries1 = 0;  while((0 != (PowerWriteChar(0x00))) && (retries1++ < 20));
		response[2]=POWER_RX_BUFFER;
		if(POWER_RX_BUFFER != 0x41 && POWER_RX_BUFFER != 0x4E)
		{
			error_flag = -3;  // We are not receiving valid response from MAXQ3180. Break loop.
			goto PowerReadFailure;
		}	
		#if( CLOCK_SPEED == 16000000 )
			Delay10TCYx(14);
		#elif( CLOCK_SPEED == 32000000 )
//			Delay10TCYx(d);		//2014-06-16 Liz removed
			Delay10TCYx(40);	//2014-06-16 Liz
		#else
			#error "Processor clock speed not defined."
		#endif
	} while( POWER_RX_BUFFER != 0x41 );
	response[3] = 0x00;
	#if( CLOCK_SPEED == 16000000 )
		Delay10TCYx(14);
	#elif( CLOCK_SPEED == 32000000 )
//		Delay10TCYx(d);		//2014-06-16 Liz removed
		Delay10TCYx(40);	//2014-06-16 Liz
	#else
		#error "Processor clock speed not defined."
	#endif

	// After receiving ACK from MAXQ3180, now start reading data by sending dummy bytes.
	//while(data_length != 0)
	for( data_count=0; data_count<data_length; data_count++ )
	{
		retries = 0;  while((0 != (PowerWriteChar(0x00))) && (retries++ < 20));
		if( retries>20 )
		{
			error_flag = -4;  // Check if the chip recevied the dummy bytes successfully.
			goto PowerReadFailure;
		}	
		response[data_count+4] = POWER_RX_BUFFER;
		result_array[data_length-data_count-1] = POWER_RX_BUFFER;
		////////////////////////////////////////
		// 2013-11-21 Liz removed. It's possible that value contains C1 byte
		//if(POWER_RX_BUFFER == 0xC1)
		//{
		//	error_flag = -4;  // Detect if MAXQ3180 has abandoned the current transaction.
		//	goto PowerReadFailure;
		//}	
		////////////////////////////////////////
		#if( CLOCK_SPEED == 16000000 )
			Delay10TCYx(14);
		#elif( CLOCK_SPEED == 32000000 )
//			Delay10TCYx(d);		//2014-06-16 Liz removed
			Delay10TCYx(40);	//2014-06-16 Liz
		#else
			#error "Processor clock speed not defined."
		#endif
	}		

	response[data_length+4] = 0xFF;  // Mark end of read.
		
	// There were no problems with the communication. Decode the result.
	if( data_length == 8 )
	{
		*result_high = 0;
		for( data_count=0; data_count<4; data_count++ )
		{
			*result_high |= (result_array[data_count] & 0x0FF);
			if( data_count!=3 ) *result_high <<= 8;  //* (data_length+1);
		}
		*result_low = 0;
		for( data_count=0; data_count<4; data_count++ )
		{
			*result_low |= (result_array[data_count+4] & 0x0FF);
			if( data_count!=3 ) *result_low <<= 8;  //* (data_length+1);
		}
	}
	else
	{	
		*result_high = 0;
		*result_low = 0;
		for( data_count=0; data_count<data_length; data_count++ )
		{
			*result_low |= (result_array[data_count] & 0x0FF);
			if( data_count!=data_length-1 ) *result_low <<= 8;  //* (data_length+1);
		}
	}

	POWER_CS_LAT = 1;  // Deactivate the chip.

	//Enable interrupts
	INTCONbits.PEIE = 1;
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;

	// 2013-12-27 Liz removed
	//if(address==0x144)
	//POWER_CS_LAT = 1;
	
	//Delay1KTCYx(10);
	return 1;
	
PowerReadFailure:
	//Enable interrupts
	INTCONbits.PEIE = 1;
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;
	
	// 2012-06-07(Eric) - Bug fix, deactivate MAXQ when problem is encountered.
	POWER_CS_LAT = 1;  // Deactivate the chip.

	return error_flag;
}

// Data is written LSB first.
// Error codes.
// -1 = Command1 did not receive 0xC1 as response.
// -2 = Command2 did not receive 0xC2 as response.
// -3 = Problem trying to get MAXQ3180 to receive data.
// -4 = We are not receiving valid response from MAXQ3180 while writing data.
// -5 = We are not receiving ACK(0x41) from MAXQ3180 after sending all the command and data from writing.
//
// Param data - 32-bits long data capable.
char Power_SendWriteCommand(short address, char data_length, unsigned long data)
{
	// processing_retry - Counts the number of times to retry waiting or MAXQ3180 to complete processing.
	// retries - Counts the number of times to retry this function.
//	struct spi_response response;
	unsigned int dataloopcount = 2, retries, retries1;
	unsigned int error = 0;  // Make this variable as 1 when an error occurs.
	//unsigned long mask = 0x000000FF;
	char command1 = 0x80, command2 = 0;  // Setting 128 in command1 specifies that this request is a write command.

	//Power_Enable();

	//Build the command1.
	switch( data_length )
	{
		case 2: command1 |= 0x10; break;
		case 4: command1 |= 0x20; break;
		case 8: command1 |= 0x30; break;
		default: return -5;
	}

	//command1 |= ((address >> 8) & 0x0F);
	command1 |= ((address & 0xF00) >> 8);
	command2 = (address & 0x0FF);

	POWER_CS_LAT = 1;  // Deactivate the chip.
	Delay10TCYx(40);  // MicrosecondDelay100x(200);
	Delay10TCYx(40);  // MicrosecondDelay100x(200);  // Delay10KTCYx(340);    //1000ms
	POWER_CS_LAT = 0;  // Activate the chip.

	// Delay to make sure that the MAXQ3180 does not
	// mistaken this command to be part of a previous
	// incomplete command.
	//Delay10KTCYx(450);  //1125ms.

	// Send in command1.
	retries = 0;  while((0 != (PowerWriteChar(command1))) && (retries++ < 20));
	//retries = 0;  while((SSP2STATbits.BF==0) && (retries++ < 20));
	//response.spi_in[0] = POWER_RX_BUFFER;
	//SSP2STATbits.BF = 0;
	if(POWER_RX_BUFFER != 0xC1) return -1;
	Delay10TCYx(40);  // MicrosecondDelay100x(10);//Delay1KTCYx(6);   //1.5ms
		
	// Send in command2.
	retries = 0;  while((0 != (PowerWriteChar(command2))) && (retries++ < 20));
	//retries = 0;  while((SSP2STATbits.BF==0) && (retries++ < 20));
	//response.spi_in[1]=POWER_RX_BUFFER;
	//SSP2STATbits.BF = 0;
	if(POWER_RX_BUFFER != 0xC2) return -2;
	
	Delay10TCYx(40);  // MicrosecondDelay100x(10);
//	//1.5ms delay.
//	#if( SIXTEEN_MHZ_CLOCK )
//	Delay1KTCYx(6);   //1.5ms
//	#endif
//	#if( FORTYEIGHT_MHZ_CLOCK )
//	Delay1KTCYx(18);   //1.5ms
//	#endif
	
	// Send in the data into the MAXQ3180.
	while(data_length != 0)
	{
		Delay10TCYx(40);  // MicrosecondDelay100x(10);
		
		retries = 0;  while((0 != (PowerWriteChar((char)(data & 0x0FF)))) && (retries++ < 20));
		//retries = 0;  while((SSP2STATbits.BF==0) && (retries++ < 20));
		//response.spi_in[dataloopcount]=POWER_RX_BUFFER;
		//SSP2STATbits.BF = 0;
		if(POWER_RX_BUFFER != 0x41) return -4;  // We are not receiving valid response from the MAXQ3180. Break loop.
		
		data_length--;
		dataloopcount++;
		data >>= 8;
		
		
//		//4ms delay.
//		#if( SIXTEEN_MHZ_CLOCK )
//		Delay1KTCYx(16);     //4ms
//		#endif
//		#if( FORTYEIGHT_MHZ_CLOCK )
//		Delay1KTCYx(48);     //4ms
//		#endif
	}		

	// Mark the last return byte with a 0.
	//response.spi_in[dataloopcount] = 0xFF;

	// Sending of command is complete. Now just wait for MAXQ3180 to finish processing.
	// Wait for MAXQ3180 to complete its processing of the commands.
	// At the end of this loop, check if the loop exited becuase it reached its
	// retry limit. If it is, give a continue command to retry the whole command from
	// the beginning.
	retries = 0;  // Initialise retry limit.
	do {
		if( retries++ > 10 ) return -6;
		retries1 = 0;  while((0 != (PowerWriteChar(0x00))) && (retries1++ < 20));
		//retries1 = 0;  while((SSP2STATbits.BF==0) && (retries1++ < 20));
		//response.spi_in[dataloopcount+1]=POWER_RX_BUFFER;
		//SSP2STATbits.BF = 0;
		if(POWER_RX_BUFFER != 0x41 && POWER_RX_BUFFER != 0x4E) return -7;  // We are not receiving valid response from the MAXQ3180. Break loop.
		
		//MicrosecondDelay100x(10);
		#if( CLOCK_SPEED == 16000000 )
			Delay10TCYx(14);
		#elif( CLOCK_SPEED == 32000000 )
			Delay10TCYx(30);
		#else
			#error "Processor clock speed not defined."
		#endif
//		//4ms delay.
//		#if( SIXTEEN_MHZ_CLOCK )
//		Delay1KTCYx(16);     //4ms
//		#endif
//		#if( FORTYEIGHT_MHZ_CLOCK )
//		Delay1KTCYx(48);     //4ms
//		#endif
	} while( POWER_RX_BUFFER != 0x41 );

	// Mark the last return byte with a 0.
	//response.spi_in[dataloopcount+2] = 0xFF;
		
	//Power_Disable();
	POWER_CS_LAT = 1;  // Deactivate chip.
	return 1;
}

/*---------------------------------------------------------------------------------
	2013-05-27 Liz
	Need to put some delay before next retry if fail to write into MAXQ. Maxq may
		be hanged and cause microcontroler hanged if there is no delay between retries.
	
_ If parameter "retries" is set to 0, function will try to write to MAXQ until
		successfull.
---------------------------------------------------------------------------------*/
char Power_WriteToMAXQ(short address, char data_length, unsigned long data, unsigned char retries)
{
	unsigned char count = 0;
	char r = 0;
	
	do{
		r = Power_SendWriteCommand(address, data_length, data);
			
		if(r!=1)
		{
			// When there is an error talking to MAXQ3180...
			#if (CLOCK_SPEED == 32000000)
				Delay1KTCYx(10);
			#else
				#error "No valid clock speed defined."
			#endif
		}	
		count++;
	}while(r!=1 && count < retries);
	
	return r;
}	

void PowerClose(void)
{
	#if defined(__18F87J50)
	CloseSPI2();
	#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
	CloseSPI();
	#else
		#error "No processor defined."
	#endif

	POWER_CS_LAT = 1;
}

//************************
//* Private Functions.    
//************************
/*
unsigned char PowerReadChar(void)
{
	#if defined(__18F87J50)
	return ReadSPI2();
	#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
	return ReadSPI();
	#else
		#error "No processor defined."
	#endif
}
*/

unsigned char PowerWriteChar(char c)
{
	#if defined(__18F87J50)
	return WriteSPI2(c);
	#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
	return WriteSPI(c);
	#else
		#error "No processor defined."
	#endif
}

/*
unsigned char PowerIsBufferFull(void)
{
	#if defined(__18F87J50)
	return DataRdySPI2();
	#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
	return DataRdySPI();
	#else
		#error "No processor defined."
	#endif
}	
*/

/*-----------------------------------------------------------------
	2013-04-03 Liz added.
	This function form a packet data of specific parameter for 3 phase.
   +-----------+---------+---------+
   | Divide By | Reading |
   |  1-byte   |         |
   +-----------+---------+---------+
-----------------------------------------------------------------*/
BYTE * FormThreePhaseReadings(short reg, BYTE * dest)
{
	char r = 0, t = 1, i = 0, length = 0, retry=0;
	long hresults = 0, lresults = 0;
	WORD startsWith = (reg&0x0FF0);
	WORD endsWith = (reg&0x000F);
	short idividual_reg = 0;
	BOOL mode = FALSE;
	
	// 2013-11-11 Liz added to check 0 reference
	if(dest == 0)	return dest;
	//
	
	// Check if register is 3-Phase or Single-Phase. Register for 3-Phase is endsWith 0x0**9
	if(endsWith == 0x0009)
		mode = TRUE;
	else
		mode = FALSE;
		
	// Get datetime
	memcpy(&dest[0], &rtcc, 6);
	
	// get register
	memcpy(&dest[6], &reg, 2);
	
	// Special handle Neutral current and frequency for eeprom record
	if(reg == 0x0989)
	{
		// Get divide_by for frequency
		dest[8] = POWER_REGISTERS_DIVIDE_BY_M[0];
		
		// Get frequency value
		memcpy(&dest[9], &reading_storage_1.LINE_FREQ, 2);

		// Get divide_by for neutral current
		dest[11] = POWER_REGISTERS_DIVIDE_BY_A[2];

		// Get neutral value
		memcpy(&dest[12], &reading_storage.CURRENT_NEUTRAL, 8);

		return dest;
	}

	// Get divide_by number
	if(startsWith == 0x0830)
	{
		dest[8] = POWER_REGISTERS_DIVIDE_BY_A[1];
		length  = 8;
	}
	else if(startsWith == 0x0840)
	{
		dest[8] = POWER_REGISTERS_DIVIDE_BY_A[2];
		length  = 8;
	}
	else if(startsWith == 0x0800 || startsWith == 0x0810 || startsWith == 0x0820)
	{
		dest[8] = POWER_REGISTERS_DIVIDE_BY_A[7];
		length  = 8;
	}
	else
	{
		dest[8] = 2;
		length  = 2;
	}
		
	if(!mode)
	{
		//	Request to form individual phase reading
			
		// Check if it's request for energy. Need to special handle energy
		if(startsWith == 0x0870 || startsWith == 0x08C0 || startsWith == 0x08D0)
		{
			switch(reg)
			{	
				case 0x08C1:	// Real energy Phase A
					CopyModbusPowerReadings(1, 10, &dest[8]);
					break;
				case 0x08C2:	// Real energy Phase B
					CopyModbusPowerReadings(2, 10, &dest[8]);
					break;
				case 0x08C4:	// Real energy Phase C
					CopyModbusPowerReadings(3, 10, &dest[8]);
					break;
				default:
					break;
			}
		}
		else
		{	
			// Directly get from MAXQ	
			PowerOpen();
			
			// Retry at most 5 times to get request reading from MAXQ. Better dont stay here for too long.
			retry = 0;	
			do{
				r = PowerSendReadCommand(reg, length, &hresults, &lresults);
					
				if(r!=1)
				{
					// When there is an error talking to MAXQ3180...
					#if (CLOCK_SPEED == 32000000)
						Delay100TCYx(20);	//2012-10-15 Liz: changed from 10 to 20. Requesting too fast makes MAXQ unable to response
					#else
						#error "No valid clock speed defined."
					#endif
				}	
				retry++;
			}while(r!=1 && retry<5);	
			
			PowerClose();
			
			if(r==1)
			{		
				// return value if successfully get from MAXQ
				if(length >= 8)
				{
					memcpy(&dest[9], &lresults, 4);
					memcpy(&dest[13], &hresults, 4);		
				}
				else
					memcpy(&dest[9], &lresults, 2);
			}
			else
			{
				//
				memcpypgm2ram(&dest[9], "NULL", 4);
			}	
		}
	}	
	else
	{
		// Request to form 3 phases reading
		
		// Check if it's request for energy. Need to specially handle energy
		if(startsWith == 0x0870 || startsWith == 0x08C0 || startsWith == 0x08D0)
		{
			// Form three-phase real energy.
			// *Note*: this function already handle divide_by number.
			//Modbus3PEnergy(&dest[0]);
			Modbus3PEnergy(&dest[8], FALSE);
			//
		}
		else
		{
			switch(reg)
			{
				case 0x0809:	// 3 Phase Real Power
				{
					Modbus3PReading(&dest[8], 7);
				}	
					break;
				case 0x0819:	// 3 Phase Reactive Power
				{
					Modbus3PReading(&dest[8], 9);
				}	
					break;
				case 0x0829:	// 3 Phase Apparent
				{
					Modbus3PReading(&dest[8], 8);
				}	
					break;
				case 0x0839:	// 3 Phase voltage
				{
					Modbus3PReading(&dest[8], 1);
				}	
					break;
				case 0x0849:	// 3 Phase current
				{
					Modbus3PReading(&dest[8], 2);
				}	
					break;
				case 0x0869:	// 3 Phase Power Factor
				{
					Modbus3PReading(&dest[8], 0);
				}	
					break;
				case 0x0959:	// 3 Phase Max Demand
				{
					Modbus3PMaxDemand(&dest[8]);
				}
					break;
				default:
					break;	
			}	
		}	
	}	

	return dest;
}



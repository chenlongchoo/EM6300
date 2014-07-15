//
#include <delays.h>
#include <stdlib.h>
#include <string.h>
#include <usart.h>

#include "HardwareProfile.h"
#include "AppConfig.h"
#include "mcu.h"
#include "menu.h"
#include "registers.h"
#include "utility.h"

#if defined (SINGLE_PHASE_BOTTOM_BOARD) || defined (THREE_PHASE_BOTTOM_BOARD)
	#include "eeprom.h"
	#include "main.h"
	#include "power.h"
	#include "eep.h"
	#include "rtcc.h"
#endif

#include "registers.h"

#ifdef METER_TOP_BOARD
	#include "Main_Meter_Top_Board.h"
	#ifdef APP_USE_LCD
		#include "lcd.h"
	#endif
	#ifdef APP_USE_MENU
		#include "menu.h"
	#endif
#endif

#ifdef APP_USE_MCU

/***************************************/
/********* Prototypes ******************/
/***************************************/

BYTE MCUNewMessageLength = 0;
BYTE MCUHasNewMessage = 0;
char MCUNewMessage[40] = "";
BOOL IS_STATUS_FLAG_SENT = 0;	//2012-10-16 Liz added.
char maxq_mask[3] = {0, 0, 0};	//2012-11-09 Liz added.

// 2012-05-07(Eric) - To indicate on-going MCU request.
#if defined(METER_TOP_BOARD)
DWORD dwMCUHasOngoingRequest = 0;
long LED_INTERVAL = 1;	// 2014-04-01 Liz added
BOOL EEP_RECORD_FLAG = FALSE;	// 2014-01-16 Liz.
BOOL UPDATE_DATETIME = FALSE;	// 2014-01-21 liz.
char DateTime[6] = "";			// 2014-01-21 Liz.
#endif

//2012-08-10 Liz: Removed, not in use
//char MCU_PENDING_REQUESTS_POINTER = 0;
//char MCU_PENDING_REQUESTS[10][20];
//

#define MCU_RX_BUFFER_SIZE				60
static unsigned char MCU_RX_BUFFER_POINTER = 0;
// 2012-05-12(Eric) - Changed type from volatile far char to volatile far BYTE as the signed type
// could cause problems when computing checksum.
volatile far BYTE MCU_RX_BUFFER[MCU_RX_BUFFER_SIZE] = {"\0"};
static char b_reset_maxq = 0;
static unsigned char is_7ESB_found = 0;

//2012-08-03 Liz: must declare here instead of inside MMT_MODBUS_REQUEST
//	unknown bug which caused
MCU_REQUEST_BLOCK mcu_request;


#if defined(METER_TOP_BOARD)
void SetMCUIsBusy(BOOL a);
#endif

void MCUWriteByte(BYTE b);


void MCUOpen(void)
{	
	#if !defined(CLOCK_SPEED)
		#error "No processor clock speed defined."
	#else
		#if defined(__18F87J50)
			Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				//77);
				38);
		#elif defined(__18F2455)

		#elif (defined(__18F26K20) || defined( __18F46K20 )) && ( CLOCK_SPEED == 32000000 )
			OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_ON
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				103);
				//207);
		#else
		#error "No supported controller defined."
		#endif
	#endif
	INTCONbits.PEIE = 1;
	INTCONbits.GIEH = 1;
}

void MCUTasks(void)
{
	// Check for errors with USART.
#if defined(__18F26K20) || defined(__18F46K20)
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
#elif defined(__18F87J50)		
	if( RCSTA1bits.OERR )
	{
		RCSTA1bits.CREN = 0;
		RCSTA1bits.CREN = 1;
	}
#endif
	
	MCUProcessIncomingMessage();
}

void MCUUnloadData(void)
{
	// 2012-05-12(Eric) - Changed from char to BYTE.
	BYTE c1;

#if defined(__18F26K20) || defined(__18F46K20)
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
#elif defined(__18F87J50)		
	if( RCSTA1bits.OERR )
	{
		RCSTA1bits.CREN = 0;
		RCSTA1bits.CREN = 1;
	}
#endif

	// Unload as much data as possible.
	while( MCU_RX_INTERRUPT_FLAG )	// && (MCU_RX_BUFFER_POINTER < MCU_RX_BUFFER_SIZE-1) )
	{		
		c1 = MCUReadByte();
		
		// Unload all data inside the buffer, don't need to check for the start byte. 
		// Received data will be processed later in MCUProcessIncomingMessage
		
		// Check if first byte 7E is found, start to load data from begining of the buffer
		// If already found 1 byte 7E, don't care about the rest 7E inside msg
		if(!is_7ESB_found && c1 == 0x7E)
		{
			is_7ESB_found = 1;
			MCU_RX_BUFFER[0] = MCU_RX_BUFFER_POINTER = 0;	
		}	
		
		if(MCU_RX_BUFFER_POINTER < MCU_RX_BUFFER_SIZE-1)
			MCU_RX_BUFFER[MCU_RX_BUFFER_POINTER++] = c1;	
		
		#if( CLOCK_SPEED == 32000000 )
			Delay100TCYx(10);	//40
		#elif( CLOCK_SPEED == 48000000 )
			Delay100TCYx(15);	//80
		#else
			#error "No clock speed defined."
		#endif
	}
}

#if defined(__18F26K20) || defined(__18F46K20)
void MCUProcessIncomingMessage(void)
{
	static char sg[45];	//2012-08-10 Liz: Increased buffer from 30 to 40 bytes
	
	// 2013-03-18 Liz: check if need to reset maxq and bottom board
	b_reset_maxq = 0;
	
	// Clear any errors.
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
	
	if( MCU_RX_BUFFER_POINTER == 0 ) 
		return;

	// 7EPROB
	// ***************
	// Before starting make sure that we can successfully disable the interrupt so that we do not get
	// disrupted when checking for start byte...
	// ***************
	// When the first byte is not a valid start byte, looking for a valid start byte within MCU_RX_BUFFER.
	// When a valid start byte is found, move the array forward until 7E is the start byte.
	if( MCU_RX_BUFFER[0] != 0x7E ) 
	{
		MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0;
		is_7ESB_found = 0;
		return;
	}

	if( MCU_RX_BUFFER_POINTER < 3 ) // wait until get start byte 0x7E and data_length before verifying message
	{
		// 2012-05-12(Eric) - We might get stuck in this state for some time... maybe infinitely?
		INTCONbits.GIEH = 1;
		return;
	}
	// END 7EPROB
	
	{
		// Verify checksum.
		// 2012-05-12(Eric) - Changed from char to BYTE.
		BYTE data_length = MCU_RX_BUFFER[1];
		unsigned char i = 0, checksum = 0;
		
		// 2014-05-07 Liz. 
		// **Note: Valid message in MCU_RX_BUFFER should content Data with MCU_RX_BUFFER_SIZE = data_length + 3
		//		   Data format: 0x7E | data_length(1 BYTE) | Data(data_length BYTES) | Checksum(1 BYTE)
		if( data_length >= (MCU_RX_BUFFER_SIZE-3) ) // invalid msg, clear start byte and data_length
		{ 
			MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
			is_7ESB_found = 0;
			return;
		}
		
		if( data_length > MCU_RX_BUFFER_POINTER-3 ) // wait if msg not fully loaded yet
		{ 
			// 2012-05-12(Eric) - We might get stuck in this state for some time... maybe infinitely?
			INTCONbits.GIEH = 1;
			return;
		}
		
		for( i=2; i<data_length+2; i++ )
		{
			// 2012-05-12(Eric) - As MCU_RX_BUFFER is defined as char type and checksum a unsigned type,
			// using the operator += could cause undesirable result. Eg. checksum(1000) += MCU_RX_BUFFER(-20).
			checksum += MCU_RX_BUFFER[i];
		}
		if(checksum == 0x7E || checksum == 0x00)	checksum++;
		
		if( MCU_RX_BUFFER[i] != checksum ) 
		{
			MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
			is_7ESB_found = 0;
			INTCONbits.GIEH = 1;
			MCU_RX_BUFFER[data_length+2] = 0;	// Remove checksum byte.
			return;
		}	
		// Remove checksum byte.
		MCU_RX_BUFFER[data_length+2] = 0;
		// 2012-05-08(Eric) - DEBUG send out checksum to indicate that message have been received.
		MCUWriteByte(checksum);
		
		// Process the messages in the buffer.
		{
			#if defined(APP_USE_MCU_TRANSACTIONID)
			// 2012-05-08(Eric) - Add in Transaction ID...
			// The bottom board have to be modified later to return back the Transaction ID to top board.
			// This is in order for top board match the request the response was generated for.
			DWORD dwTransactionID = *((DWORD*)&MCU_RX_BUFFER[2]);
			MCU_MESSAGE_TYPE msg_type = (MCU_MESSAGE_TYPE)MCU_RX_BUFFER[6];
			BYTE * data = &MCU_RX_BUFFER[7];
			#else
			MCU_MESSAGE_TYPE msg_type = (MCU_MESSAGE_TYPE)MCU_RX_BUFFER[2];
			BYTE * data = &MCU_RX_BUFFER[3];
			#endif
			
			// 2013-11-12 Liz. Check if refenrece is 0
			if(data == 0)
			{	
				MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
				is_7ESB_found = 0;	
				return;
			}
			//
			
			// Process messages according to message type.
			switch( msg_type )
			{
				case MMT_MODBUS_READ_REQUEST:	// Received a Modbus request for reading from the top board.
				{
					//2012-08-03 Liz: cannot declare this one inside MMT_MODBUS_REQUEST (unknown bug)
					//MCU_REQUEST_BLOCK req;
					
					char is_success=0;
					
					memset(&mcu_request, 0, sizeof(MCU_REQUEST_BLOCK));
					memcpy(&mcu_request.Serialised[0], &MCU_RX_BUFFER[2], data_length);
					
					mcu_request.w.header.message_type = MMT_MODBUS_READ_RESPONSE;
					
					switch(mcu_request.w.header.start_addr)
					{
						case 0x0907:	// Get DateTime
						{
							RTCC_GetDateTime((char*)&mcu_request.w.data);
							is_success = 1;
						}
							break;
						case 0x0904:	// 2013-03-11 Liz: added additional flag
						{
							// 2013-03-27 Liz. Added MAXQ_RESET_FLAG
							//BYTE maxqFlag[6];
							BYTE maxqFlag[5];
							
							short int * ptr1 = &CalibrationData.Flags;
							short int * ptr2 = &CalibrationData.Flag2;
							
							// 2013-11-12 Liz. Check if refenrece is 0
							if(ptr1 == 0 || ptr2 == 0)
							{
								MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
								is_7ESB_found = 0;	
								return;
							}	
							//
							
							memcpy(&maxqFlag[0], (char*)reading_storage_1.PHASE_A_STATUS, 1);
							memcpy(&maxqFlag[1], (char*)reading_storage_1.PHASE_B_STATUS, 1);
							memcpy(&maxqFlag[2], (char*)reading_storage_1.PHASE_C_STATUS, 1);
							maxqFlag[3] = *ptr1;
							maxqFlag[4] = *ptr2;
							
							// 2013-03-27 Liz. Added MAXQ_RESET_FLAG
							//maxqFlag[5] = MAXQ_RESET_FLAG[0];
							//memcpy((char*)&mcu_request.w.data, &maxqFlag[0], 6);
							//
							
							memcpy((char*)&mcu_request.w.data, &maxqFlag[0], 5);
							is_success = 1; 
						}
							break;		
						default:
						{
							is_success = PowerGetReadings(&mcu_request);
						}
							break;			
					}	
									
					if(is_success == 0x01)
					{
						MCUSendString(sizeof(MCU_REQUEST_HEADER)+(mcu_request.w.header.ByteCount), mcu_request.Serialised);
					}
					else
					{
						mcu_request.w.header.start_addr |= 0x8000;
						MCUSendString(sizeof(MCU_REQUEST_HEADER), mcu_request.Serialised);
					}
					break;
				}
				case MMT_MODBUS_WRITE_REQUEST:
				{
					memset(&mcu_request, 0, sizeof(MCU_REQUEST_BLOCK));
					memcpy(&mcu_request.Serialised[0], &MCU_RX_BUFFER[2], data_length);
					
					mcu_request.w.header.message_type = MMT_MODBUS_WRITE_RESPONSE;

					switch(mcu_request.w.header.start_addr)
					{
						case 0x0901:	// Set CFP output and threshold
						{
							// 2013-12-06 Liz added.
							//	Configure CFP output and threshold
							BYTE CFP_output = (*((BYTE*)(&mcu_request.w.data) + 4));
							long CFP_threshold = 0;
							BYTE r = 0;
							memcpy(&CFP_threshold, &mcu_request.w.data, 4);
							
							PowerOpen();
							// Configure CFP
							do{
								r = Power_SendWriteCommand(0x01E, 2, CFP_output);
							}while(r!=1);
					
							r = 0;		
							// Configure CFP threshold
							do{
								r = Power_SendWriteCommand(0x022, 4, CFP_threshold);
							}while(r!=1);								
							PowerClose();						
						}
							break;	
						case 0x0970:  // Set Calibration data.
						{
							memcpy(&CalibrationData, &mcu_request.w.data, (mcu_request.w.header.ByteCount*2));
							// Eric(2012-10-23) This line is not needed since handled by call to AutoCalibration() below...
							//CalibrationData.Flags.bChecksumChanged = 1;
							AutoCalibration();
						}
							break;
						case 0x0907:	// Set DateTime
						{
							RTCC_SetDateTime((char*)&mcu_request.w.data);
							// 2014-03-11 Liz added DateTime flag
							CalibrationData.Flag2.bIsGetDatetime = 1;
						}
							break;
						case 0x0912:	// Clear Energy over_flow_counters
						{
							char i = 0;
							for( i=0; i<15; i++ )
							{
								*EEPROM_ENR_OVFC1[i] = 0;
								*EEPROM_ENR_OVFC2[i] = 0;
							}
														
							// 2014-04-22 Liz added
							// Reset raw_energy_2
							memset(&raw_energy_storage_2, 0, sizeof(raw_energy_storage_2));
							// Clear raw_energy_2 in eep. Just need to clear first BYTE
							while( Read_b_eep(160) != 0x10u )
							{
								Write_b_eep(160, 0x10u);  //0
								Busy_eep();
							}
							
							// Update flag
							CalibrationData.Flags.bIsModified = 1;
							CalibrationData.Flags.bEnergyReset = 1;
							
							// 2012-11-21 Liz: Reset MAXQ after reset energy
							// *Important*: Should reset bottom board after that since 
							//		MAXQ need to be re-configured ater reset
							POWER_RESET_LAT = 1;	// Keep RESET pin high	
							Delay100TCYx(1);
							POWER_RESET_LAT = 0;
							Delay100TCYx(1);
							POWER_RESET_LAT = 1;
							SaveCalibrationData();	// Save Calibration data before reset
							
							// 2013-03-18 Liz: check if need to reset maxq and bottom board
							//	Cannot reset bottom board here. Need to send ACK to top board before reset
							b_reset_maxq = 1;			
							//ResetBot();
							// End
						}		
							break;
						case 0x0904:	//Clear maxq status flag
						{							
						
							BYTE flag = ~(*((BYTE*)(&mcu_request.w.data) + 3));
							BYTE flag1 = ~(*((BYTE*)(&mcu_request.w.data) + 4));
							CalibrationData.Flags.bBoardReset &= (flag&0x02)>>1;
							CalibrationData.Flags.bChecksumChanged &= (flag&0x20)>>5;
							CalibrationData.Flags.bEnergyReset &= (flag&0x10)>>4;
							CalibrationData.Flags.bPhaseA_RealPos_C2_Overflow &= (flag1&0x20)>>5;
							CalibrationData.Flags.bPhaseB_RealPos_C2_Overflow &= (flag1&0x40)>>6;
							CalibrationData.Flags.bPhaseC_RealPos_C2_Overflow &= (flag1&0x80)>>7;
							
							// 2013-03-21 (Liz) added to clear bAutoReset flag
							CalibrationData.Flag2.bAutoReset &= (flag1&0x08)>>3;						
							CalibrationData.Flag2.bTOReset &= (flag1&0x02)>>1;;	//2013-03-28 Liz
						
							IS_STATUS_FLAG_SENT = 1;
							// 2012-11-09 Liz Get Maxq mask
							memcpy(&maxq_mask[0], (char*)&mcu_request.w.data, 3);
						}
							break;	
						case 0x950:		//Clear max demand
						case 0x951:
						case 0x952:
						{
							long * max_value;
							BYTE reg = mcu_request.w.header.start_addr&0x000F;
							
							// 2013-11-12 Liz. Check if refenrece is 0
							if(max_value == 0)
							{
								MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
								is_7ESB_found = 0;	
								return;
							}	
							//
							 
							max_value = (long *)MAX_MIN_STORAGE_QUEUE[reg][0];
							max_value[0] = 0;
							max_value[1] = 0;
						}
							break;
						case 0x959:	// Clear 3 phase max Demand
						{
							memset(&max_min_storage, 0, sizeof(max_min_storage));
						}
							break;
						case 0x914:		// 2013-03-18 Liz: Added function to reset MAXQ
						{
							// 2012-11-21 Liz: Reset MAXQ after reset energy
							// *Important*: Should reset bottom board after that since 
							//		MAXQ need to be re-configured ater reset
							POWER_RESET_LAT = 1;	// Keep RESET pin high	
							Delay100TCYx(1);
							POWER_RESET_LAT = 0;
							Delay100TCYx(1);
							POWER_RESET_LAT = 1;
							SaveCalibrationData();	// Save Calibration data before reset
							
							// 2013-03-18 Liz: check if need to reset maxq and bottom board
							//	Cannot reset bottom board here. Need to send ACK to top board before reset
							b_reset_maxq = 1;
							//ResetBot();
							// End
						}	
							break;
						default:
							mcu_request.w.header.start_addr |= 0x8000;
							break;
					}	
			
					// Eric(2012-09-21) - For now, always assume data written is always error free
					// since memcpy does not provide any return values.
					
					// Eric(2012-09-21) - For write request, there is nothing to return. So we just return the header.
					MCUSendString(sizeof(MCU_REQUEST_HEADER), mcu_request.Serialised);
				}
				break;	
				case MMT_READING_REQUEST:
				{
					char _phase = data[0];
					unsigned char length = 0;
					char _reg = data[1];
					char req_mode = data[2];	//2012-09-24: Liz added to know if it's EEPROM or normal req. 
					
					if(req_mode == 'E')
					{
						sg[0] = MMT_GENERIC_RESPONSE;
					}	
					else
						sg[0] = MMT_READING_RESPONSE;  //(msg_type & 0x0F) + 0x80;  // Return msg type is 0x82.
					sg[1] = _phase;  	// phase.
					sg[2] =	_reg;		// reg.
					sg[3] = 0;
					
					// 2013-02-27(Liz) Dont return value if total 3P kwh is not ready
					if(_phase==0 && _reg==10) // Request for total kwh. 
					{
						// 2013-09-12 Liz. Have to send Fail status/ACK reponse if value is not ready.
						
						if(EnrOvfCounter1.Flags.bIs_3Pkwh_Ready != 1)
						{
							sg[3] = 6;
							sg[4] = 'A';
							sg[5] = 'B';
							sg[6] = 'C';
							
						//	MCUSendString(7, &sg[0]);
							break;
						}	
					}	
								
					//CopyPowerReadings(_phase, _reg, &sg[3]);
					CopyModbusPowerReadings(_phase, _reg, &sg[3]);
					
					// 2014-06-03 Liz. Need to check if CopyModbusPowerReadings() return valid message
					//	   If not, dont send invalid response to top board.									
					length = sg[3];
					if(length != 0)
						MCUSendString(14+sg[3], sg); // where sg[3] contains the length of the reading.
				}
					break;	
				case MMT_GET_CALIBRATION_DATA:
				{
					// 2012-08-10 Liz: Removed. Dont need to declare another array here.
					//BYTE sm[sizeof(CALIBRATION_VALUES)+2];
					
					sg[0] = (BYTE)MMT_GENERIC_RESPONSE;
					memcpy(&sg[1], &CalibrationData, sizeof(CALIBRATION_VALUES));
					MCUSendString(sizeof(CALIBRATION_VALUES)+1, sg);
				}
					break;	
				case MMT_SET_CALIBRATION_DATA_NEW:
				{
					#if defined(APP_USE_MCU_TRANSACTIONID)
					if( data_length-5 == sizeof(CALIBRATION_VALUES) )
					#else
					if( data_length-1 == sizeof(CALIBRATION_VALUES) )
					#endif
					{
						memcpy(&CalibrationData, &data[0], sizeof(CALIBRATION_VALUES));
						AutoCalibration();	// 2012-05-05 Liz: Recalibrate meter after user changed config
						//PowerWriteCALBToMAXQ();	//2012-08-10 Liz: Removed. Function is called in AutoCalibration().
						CalibrationData.Flags.bIsModified = 1;
						// Eric(2012-10-23) This line is not needed since handled by call to AutoCalibration() above...
						//CalibrationData.Flags.bChecksumChanged = 1;
						{
							char t[3] = {(BYTE)MMT_GENERIC_RESPONSE, 'O', 'K'};
							MCUSendString(3, &t[0]);
						}
					}	
				}
					break;	
				case MMT_RESET_ENR_COUNTER:  // Reset energy counters.
				{
					// Reset energy counters.
					char i = 0;
					for( i=0; i<15; i++ )
					{
						*EEPROM_ENR_OVFC1[i] = 0;
						*EEPROM_ENR_OVFC2[i] = 0;
					}	
										
					// 2014-04-22 Liz added
					// Reset raw_energy_2
					memset(&raw_energy_storage_2, 0, sizeof(raw_energy_storage_2));
					// Clear raw_energy_2 in eep. Just need to clear first BYTE
					while( Read_b_eep(160) != 0x10u )
					{
						Write_b_eep(160, 0x10u);  //0
						Busy_eep();
					}
					
					CalibrationData.Flags.bIsModified = 1;
					CalibrationData.Flags.bEnergyReset = 1;
					{
						char t[3] = {(BYTE)MMT_GENERIC_RESPONSE, 'O', 'K'};
						MCUSendString(3, &t[0]);
					}	
					// 2012-11-21 Liz: Reset MAXQ after reset energy
					// *Important*: Should reset bottom board after that since 
					//		MAXQ need to be re-configured ater reset 
					POWER_RESET_LAT = 1;	// Keep RESET pin high	
					Delay100TCYx(1);
					POWER_RESET_LAT = 0;
					Delay100TCYx(1);
					POWER_RESET_LAT = 1;
					SaveCalibrationData();	// Save Calibration data before reset
					b_reset_maxq = 1;
					//ResetBot();
					// End
				}
					break;
				case MMT_CLEAR_STATUS_FLAG:	// Clear the status flag if it's already transfered to top board.
				{
					BYTE flag = data[0];
					BYTE flag1 = data[1];
					CalibrationData.Flags.bBoardReset &= (flag&0x02)>>1;
					CalibrationData.Flags.bChecksumChanged &= (flag&0x20)>>5;
					CalibrationData.Flags.bEnergyReset &= (flag&0x10)>>4;
					CalibrationData.Flags.bPhaseA_RealPos_C2_Overflow &= (flag1&0x20)>>5;
					CalibrationData.Flags.bPhaseB_RealPos_C2_Overflow &= (flag1&0x40)>>6;
					CalibrationData.Flags.bPhaseC_RealPos_C2_Overflow &= (flag1&0x80)>>7;

					// 2012-05-09(Eric) - All requests should have a response.
					// 2012-10-25(Eric) - All requests MUST have a response!!
					{
						char t[3] = {(BYTE)MMT_GENERIC_RESPONSE, 'O', 'K'};
						MCUSendString(3, &t[0]);
					}
				}
					break;
#warning Eric(2012-09-21) - To be deprecated. Should now call MMT_GET_CALIBRATION_DATA.
				case MMT_CT_RATING_REQUEST:
				{
					unsigned int value = *((int *)EEPROM_CALIBRATION_DATA[11]);
					
					sg[0] = (BYTE)MMT_GENERIC_RESPONSE;
					itoa(value, &sg[1]);
					//PowerCalculateCALIBChecksum(sg);
					MCUSendString(10, &sg[0]); 
				}
					break;		
				case MMT_BOT_NORMAL_RESET:
				{
					// 2012-04-27 Liz: added for Meter reset
					char t[3] = {(BYTE)MMT_GENERIC_RESPONSE, 'O', 'K'};
					MCUSendString(3, &t[0]);	  
					Delay1KTCYx(100);
					RCONbits.NOT_POR = 0;
					ResetBot();	// 2012-11-14 Liz added: save datetime before reset
				}
					break;
				case MMT_BOT_BOOTLOAD_RESET:
					RCONbits.NOT_POR = 1;
					//Reset();
					ResetBot();	// 2012-11-14 Liz added: save datetime before reset
					break;
				// 2013-12-27 Liz added for eeprom record
				case MMT_GET_3PHASE_FOR_EEPROM:
				{
					short reg = 0;
					char i = 0;
					
					sg[0] = MMT_GENERIC_RESPONSE;
					//memcpy(&reg, &data[0], 2);
					//FormThreePhaseReadings(reg, &sg[1]);
					
					// 2014-06-03 Liz. Need to check if Modbus3PEnergy return valid message
					//	   If not, dont send invalid response to top board.					
					if(EnrOvfCounter1.Flags.bIs_3Pkwh_Ready != 1)
					{
						sg[1] = 'F';
						sg[2] = 'A';
						sg[3] = 'I';
						sg[4] = 0;	
						MCUSendString(4, &sg[0]);
						break;
					}
					
					sg[1] = 50;		// Set invalid value to check if Modbus3PEnergy() working
					Modbus3PEnergy(&sg[1], TRUE);
					
					if(sg[1] != 50)									
						MCUSendString(41, &sg[0]);
				}
					break;	
				case MMT_SET_DATETIME:
				{
					//BOOL result = TRUE;
					// Compare_datetime. If the diff is not more than 5 mins, do not update
					//result = Compare_datetime(&data[0]);
					
					// 2014-01-22 liz. Update DateTime 
					//if(result)
					RTCC_SetDateTime(&data[0]);
					CalibrationData.Flag2.bIsGetDatetime = 1;			 
				}
					break;	
				default:
				{
					strcpypgm2ram(&sg[0], "#ERROR#");
					MCUSendString(7, &sg[0]);
				}		
				break;
			}	
		}
	}
	// 7EPROB
	// Whenever we want to clear the buffer, just set the start byte to 0.
	MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
	is_7ESB_found = 0;
	// END 7EPROB
	
	INTCONbits.GIEH = 1;
	
	// 2013-03-18 Liz: check if need to reset maxq and bottom board
	if(b_reset_maxq == 1)
		InitMAXQ();
		//ResetBot();
}
#endif

#if defined(__18F87J50)
void MCUProcessIncomingMessage(void)
{
	// Clear any errors.
	if( RCSTA1bits.OERR )
	{
		RCSTA1bits.CREN = 0;
		RCSTA1bits.CREN = 1;
	}
	
	// Wait if not receive any valid data	
	if( MCU_RX_BUFFER_POINTER == 0 ) 
	{
		return;
	}

	if( MCU_RX_BUFFER[0] != 0x7E ) // valid data should start with byte 0x7E
	{
		MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0;
		is_7ESB_found = 0;
		return;
	}

	if( MCU_RX_BUFFER_POINTER < 3 ) // wait until get start byte 0x7E and data_length before verifying message
	{
		INTCONbits.GIEH = 1;
		return;
	}
	
	{
		// Verify checksum.
		{
			unsigned char i = 0, checksum = 0;
			unsigned char data_length = MCU_RX_BUFFER[1];

			// 2014-05-07 Liz. 
			// **Note: Valid message in MCU_RX_BUFFER should content Data with MCU_RX_BUFFER_SIZE = data_length + 3
			//		   Data format: 0x7E | data_length(1 BYTE) | Data(data_length BYTES) | Checksum(1 BYTE)			
			if( data_length >= (MCU_RX_BUFFER_SIZE-3) ) // invalid msg, clear start byte and data_length
			{ 
				MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
				is_7ESB_found = 0;
				SetMCUIsBusy(FALSE);	// 2014-01-27 Liz. Release MCU for new requests.
				return;
			}
		
			if( data_length > MCU_RX_BUFFER_POINTER-3 ) // wait if msg not fully loaded yet
			{ 
				INTCONbits.GIEH = 1;
				return;
			}
					
			for( i=2; i<data_length+2; i++ )	// calculate checksum
			{
				checksum += MCU_RX_BUFFER[i];
			}
			if(checksum == 0x7E || checksum == 0x00)	checksum++;
			
			if( MCU_RX_BUFFER[i] != checksum )	// verify checksum
			{
				MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0;
				is_7ESB_found = 0;
				MCU_RX_BUFFER[data_length+2] = 0;	// clear checksum
				SetMCUIsBusy(FALSE);	// 2014-01-27 Liz. Release MCU for new requests.
				return;
			}

			// Remove checksum byte.
			MCU_RX_BUFFER[data_length+2] = 0;
			// Send the checksum to indicate receipt.
			MCUWriteByte(checksum);
			
			// Toggle LED1.
			// 2012-05-12(Eric) - This is status LED.
			// Modified to switch off here and light up at MCURequestToBottomBoard instead of toggling.
			LED1_IO = 0;
		}
		
		// Process received message.
		{
			BYTE data_length = MCU_RX_BUFFER[1];
			BYTE msg_type = MCU_RX_BUFFER[2];
			BYTE * data = &MCU_RX_BUFFER[3];
			
			// 2013-11-12 Liz. Check if refenrece is 0
			if(data == 0)
			{
				MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
				is_7ESB_found = 0;	
				SetMCUIsBusy(FALSE);	// 2014-01-27 Liz. Release MCU for new requests.
				return;
			}	
			//
						
			switch( msg_type )
			{
				case MMT_MODBUS_READ_RESPONSE:
				case MMT_MODBUS_WRITE_RESPONSE:	//2012-10-02 Liz added
				{
					MCU_REQUEST_BLOCK * mcuPtr = (MCU_REQUEST_BLOCK*)&MCU_RX_BUFFER[2];
					
					// 2013-11-12 Liz. Check if refenrece is 0
					if(mcuPtr == 0)
					{
						MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
						is_7ESB_found = 0;
						SetMCUIsBusy(FALSE);	// 2014-01-27 Liz. Release MCU for new requests.	
						return;
					}	
					//
					
					if(data_length == (*mcuPtr).w.header.ByteCount+sizeof(MCU_REQUEST_HEADER) ||
						data_length == sizeof(MCU_REQUEST_HEADER))
					{
						memcpy(&MCUNewMessage[0], mcuPtr, data_length);
						MCUNewMessageLength = data_length;
						MCUHasNewMessage = 1;
					}
				}
					break;
				case MMT_GET_ANALOG_SETTING_RESP:
				{
					if( data_length-1 == sizeof(CALIBRATION_VALUES) )
					{
						MCUNewMessageLength = sizeof(CALIBRATION_VALUES);
						memcpy(&MCUNewMessage, &data[0], MCUNewMessageLength);
						MCUNewMessage[MCUNewMessageLength] = 0;
						MCUHasNewMessage = 1;
					}
				}
					break;	
				case MMT_GENERIC_RESPONSE:
				{
					MCUNewMessageLength = data_length-1;
					memcpy(&MCUNewMessage[0], &data[0], MCUNewMessageLength);	
					MCUHasNewMessage = 1;
				}		
					break;
				case MMT_METER_STATUS_CHANGED:
				{
					// Eric(2012-10-23) - This flag here actually represents the 1-byte flag in CalibrationData block.
					BYTE flag = data[0];
					BYTE flag2 = data[1];	//2013-02-21 Liz test
					
					meter_flag.Flags.Flag1.bBOTTOMReset = (flag&0x01);
					meter_flag.Flags.Flag1.bCalibCksModified = (flag&0x20)>>5;
					meter_flag.Flags.Flag1.bEnergyReset = (flag&0x10)>>4;					
					meter_flag.Flags.Flag2.bPhaseA_RealPos_C2_Overflow = (flag&0x02)>>1;
					meter_flag.Flags.Flag2.bPhaseB_RealPos_C2_Overflow = (flag&0x04)>>2;
					meter_flag.Flags.Flag2.bPhaseC_RealPos_C2_Overflow = (flag&0x08)>>3;
				
					//2013-02-21 Liz test
					meter_flag.Flags.Flag2.bBOTAutoReset = (flag2&0x80)>>7;	
					meter_flag.Flags.Flag2.bBOTTOReset = (flag2&0x40)>>6;	//2013-03-28 Liz: added for WDTO-reset flag
					meter_flag.Flags.Flag1.bDateTimeUpdated = (flag2&0x20)>>5;	//2014-03-12 Liz added.
					//
				}		
					break;
				//case 0x86:  // Readings received here should be saved into local variables.
				//case 0x82:	// Readings received here should be saved into local variables and displayed on LCD.
				case MMT_READING_RESPONSE:
				{
					char phase = data[0];
					char reg = data[1];
					char data_length = data[2];
					
					int * phase_storage = POWER_READINGS[phase];
					char * storage = phase_storage[reg];  // Storage is the location where the reading will be saved.
					long * kk = phase_storage[reg];
					
					// 2013-11-12 Liz. Check if refenrece is 0
					if(kk == 0 || storage == 0)
					{
						MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0; 
						is_7ESB_found = 0;	
						SetMCUIsBusy(FALSE);	// 2014-01-27 Liz. Release MCU for new requests.
						return;
					}	
					//
					
					// Clear old value before getting new value from bottom board
					if(data_length >= 8)
					{
						kk[0] = 0;
						kk[1] = 0;
						if(data_length > 8)
							kk[2] = 0;
					}
					else
						kk[0] = 0;
					
					// Check and update if POWER_REGISTERS_DIVIDE_BY_[] has been changed
					Set_div(data[3], reg, phase);	
					
					// Update data
					// 2013-12-27 Liz removed datetime and reg in data packet since it's not necessary for this req.
					//memcpy(storage, &data[13], data_length);
					memcpy(storage, &data[5], data_length);
						
					// Eric(20120-09-19) - Release MCU for new requests.
					//SetMCUIsBusy(FALSE);
																
					break;	
				}
					break;
				//2014-2-25 CL added :Receive ADC reading of Super cap from bottom board ( Only if the super capacitor voltage is large or equal to 3V )
				case MMT_BOT_VBATVOLTAGE:
				{	
					char flag = MCU_RX_BUFFER[3];
					
					if(flag == 'H')
						LED_INTERVAL = 2;
					else 
						LED_INTERVAL = 1;
					
					meter_flag.Flags.Flag2.bBOTupdateADC = 1; //set this flag if the bottom super cap voltage >=3V
				}				
					break;
//				case MMT_BOT_UPDATE_TOP_DATETIME:
//				{	
//					BYTE flag = data[0];
//					//2014-02-27 CL added : Update bottom board bIsGetDatetime to top board bDateTimeUpdated
//					meter_flag.Flags.Flag1.bDateTimeUpdated  = (flag&0x20)>>5;
//					LCD_PrintStringPGM("dateime", LCD_ROW1); //debugging purpose
//
//					break;
//				}	
				case MMT_EEP_RECORD_TIME:
				{
					char msg[4] = {'#', 'E', 'P', 0};
					BYTE received = 0, c = 0;
					
					if( !memcmp(&data[0], &msg[0], 3) )
					{
						EEP_RECORD_FLAG =  TRUE;
					}
				}
					break;			
				default:
					if( data[0] == 'B' && data[1] == 'P' )
					{
						#ifdef APP_USE_BUZZER
						BUZZER = 1;
						{ unsigned int i = 0; while(i++<10000); }
						BUZZER = 0;
						#endif
					}	
					break;
			}
		}
	}	
	MCU_RX_BUFFER[0] = MCU_RX_BUFFER[1] = MCU_RX_BUFFER_POINTER = 0;
	is_7ESB_found = 0;
}	
#endif

#if defined(METER_TOP_BOARD)
BOOL CheckMCUIsBusy(void)
{
	// Eric(2012-09-19) - The smaller this timeout is, the less likely we are going to get CLS
	// during a Modbus request.
	return !(dwMCUHasOngoingRequest == 0 || TickGet()-dwMCUHasOngoingRequest > TICK_SECOND*1);
}
#endif

#if defined(METER_TOP_BOARD)
void SetMCUIsBusy(BOOL a)
{
	if(a) dwMCUHasOngoingRequest = TickGet();
	else dwMCUHasOngoingRequest = 0;
}
#endif

	
#if defined(METER_TOP_BOARD)
// bWait_Response indicates whether the function should wait for a response for the request.
// bWait_Response = TRUE means the function will wait for a response for the request.
// If return value of this function is TRUE, there was a response received from the bottom board.
// The response received is copied into msg_in.
//
// If return value is FALSE, the command was not received by the bottom board successfully or
// no response was received when bWait_Response = TRUE.
//
// bWait_Response = FALSE means the function should not wait for response.
// Return value is always TRUE if bWait_Response=1.
BOOL MCURequestToBOTTOMBoard(char msg_type, char * msg_in, char msg_size, BOOL bWait_Response, BOOL bAllow_Retry)
{
	// 2012-05-03(Eric) - Modification added to retry 3 times when bWait_Response = TRUE.
	BYTE retry = 0;
	BYTE s[40] = "";
	static BYTE reset_uart_count = 0;
	
	// Eric(2012-09-19) - Sanity check. If we are waiting for data, data is passed out through msg_in.
	// We cannot allow msg_in to be null when bWait_Response = TRUE. The response should never be
	// received through MCUNewMessage as the contents might have already changed.
	if( bWait_Response && msg_in == 0 )
		while(1);
		
	if(CheckMCUIsBusy()) return FALSE;
	SetMCUIsBusy(TRUE);
	
	// Check if we should reset mcu.c...
	// 2012-06-06(Eric) - HT Meter experience mcu hang. Status LED stays lighted. Unable to do normal reset.
	// Bootload Reset which resets only the top board is able to solve the problem. Problem could be because
	// reset_uart_count was not declared as static. Now changed and declared static.
	if( reset_uart_count > 5 )
	{
		// Reset everything about the MCU...
		MCU_RX_BUFFER[0] = MCU_RX_BUFFER_POINTER = 0;
		is_7ESB_found = 0;
		reset_uart_count = 0;
		
		MCUClose();
		Delay100TCYx(10);
		MCUOpen();
	}		
	
	do
	{
		retry++;
	
		#if defined(APP_USE_MCU_TRANSACTIONID)
		// Append message type before the data.
		*((DWORD*)&s[0]) = (DWORD)TickGet();  // 2012-05-08(Eric) - Add 4-byte Transaction ID...
		s[4] = msg_type;
		if(msg_in != 0)
			memcpy(&s[5], msg_in, msg_size);
		s[msg_size+5] = 0;
		#else
		s[0] = msg_type;				// 0x20 Direct request for readings. To process immediately.
		if(msg_in != 0)
			memcpy(&s[1], msg_in, msg_size);  // Need to take check that msg_in is not null.
		s[msg_size+1] = 0;
		#endif
		
		// Clear new message flag. Note this is not a MCU message consumer... just clearing the flag...
		MCUHasNewMessage = MCUNewMessage[0] = 0;
		
		// 2012-05-13(Eric) - Sense the number of times we are not able to successfully send request to bottom board.
		// If it exceeds 10, reopen and close mcu.c.
		#if defined(APP_USE_MCU_TRANSACTIONID)
		if(!MCUSendString(msg_size+5, &s[0]))
		{
			reset_uart_count++;
			SetMCUIsBusy(FALSE);
			continue;
		}	
		#else
		if(!MCUSendString(msg_size+1, &s[0]))
		{
			reset_uart_count++;
			SetMCUIsBusy(FALSE);
			continue;
		}	
		#endif

		
		// 2012-05-12(Eric) - This is status LED.
		// Modified to switch on here and off at MCUProcessIncomingMessage instead of toggling.
		LED1_IO = 1;
		reset_uart_count = 0;

		// Eric(2012-09-19) - At this point the request was successfully sent to MCU.
		if( !bWait_Response ) 
			return TRUE;
		
		// 2012-05-07(Eric) - Added to prevent wdt timeout during retries.
		ClrWdt();
		
		// Code to wait for response from MCU1.
		if( RCSTA1bits.OERR )
		{
			RCSTA1bits.CREN = 0;
			RCSTA1bits.CREN = 1;
		}

		{
			unsigned long timeout = 0;
			while( !MCUHasNewMessage && timeout++ < 300000 ) { MCUProcessIncomingMessage();}
		}	
		ClrWdt();

		if( MCUHasNewMessage )
		{
			MCUHasNewMessage = 0;
			if(msg_in != 0)
				memcpy(&msg_in[0], &MCUNewMessage[0], MCUNewMessageLength);
			// Eric(2012-09-19) - Should we set SetMCUIsBusy(FALSE) here? MCUNewMessage may not have
			// time to be consumed and the consumer might be receiving data from a new request...
			// Indicate transaction ID in MCUNewMessage too?
			// Realised that we can call SetMCUIsBusy(FALSE) here safely because contents of MCUNewMessage is
			// copied out to msg_in... However still need to consider this for the other consumers.
			SetMCUIsBusy(FALSE);
			return TRUE;
		}
	} while(bAllow_Retry && (retry < 3));

	return FALSE;
}	
#endif

// Return TRUE if the other board returned the same checksum for the message that was sent.
// External modules should not call this function. Call MCURequestToBottomBoard() instead.
// MCURequestToBottomBoard() helps handle errors and queue for requests.
BOOL MCUSendString(unsigned char data_length, char * str)
{
	// 2012-05-12(Eric) - Changed from int to BYTE.
	BYTE i = 0;
	unsigned char checksum = 0;

	// Wait for incoming message to clear.
	// At this point ISR will still run and this flag will be cleared in MCUUnloadData.
	while(MCU_RX_INTERRUPT_FLAG);

	//#if defined( __18F87J50 )	
	INTCONbits.GIEH = 0;
	INTCONbits.GIEL = 0;
	//#endif
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(str == 0)
	{
		//#if defined( __18F87J50 )
		INTCONbits.GIEH = 1;
		INTCONbits.GIEL = 1;
		//#endif
		return FALSE;
	}
	//
	
	// 2012-05-12(Eric) - Removed as this this might cause indefinite loop after interrupts are disabled.
	//while(MCU_RX_INTERRUPT_FLAG);

	// 2014-03-21 Liz removed. The Pin J4 not in used	
//	#if defined( __18F87J50 )
//	TRISJbits.TRISJ4 = 0;
//	LATJbits.LATJ4 = 0;
//	Delay100TCYx(0);
//	LATJbits.LATJ4 = 1;
//	#endif
	/////////////////////////////////////////////////

#if defined(__18F26K20) || defined(__18F46K20)
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
#elif defined(__18F87J50)		
	if( RCSTA1bits.OERR )
	{
		RCSTA1bits.CREN = 0;
		RCSTA1bits.CREN = 1;
	}
#endif
	
	// Add start byte.
	MCUWriteByte(0x7E);
	// 2012-04-26(Eric) - Removed.
	//Delay100TCYx(10);
	
	// Add data length.
	MCUWriteByte(data_length);
	// 2012-04-26(Eric) - Removed.
	//Delay100TCYx(10);

	for( i=0; i<data_length; i++ )
	{
		checksum += *(str+i);
		MCUWriteByte(str[i]);
	}
	
	// Send checksum.
	if( (checksum == 0) || (checksum == 0x7E) )
		checksum++;
	MCUWriteByte(checksum);
	
	// 2012-05-08(Eric) - Wait for checksum.
	// 2012-05-10(Eric) - If the retry limit is too high, it causes lagging issues.
	// Adjust the retry limit to about 2-3 times the typical time taken for successful messages.
	{
//		static WORD check_typ[14]; 
//		static BYTE check_typ_count = 0;
		BOOL bresult = FALSE;
		WORD wRetry = 0;
		while(!MCU_RX_INTERRUPT_FLAG && wRetry++<44000);

//		check_typ[check_typ_count++] = wRetry;
//		if(check_typ_count>=14)
//			check_typ_count=0;

#if defined(__18F26K20) || defined(__18F46K20)
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		RCSTAbits.CREN = 1;
	}
#elif defined(__18F87J50)		
	if( RCSTA1bits.OERR )
	{
		RCSTA1bits.CREN = 0;
		RCSTA1bits.CREN = 1;
	}
#endif
		
		if(wRetry<44000)
		{
			BYTE c = MCUReadByte();
			bresult = (checksum == c);
		}

		//#if defined( __18F87J50 )		
		INTCONbits.GIEH = 1;
		INTCONbits.GIEL = 1;
		//#endif
		
		return bresult;
	}	
}	

void MCUWriteByte(BYTE c)
{
	#if defined(__18F87J50)
		while(MCUIsBusy());
		Write1USART(c);
	#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
		while(MCUIsBusy());
		WriteUSART(c);
	#else
		#error "No supported controller defined."
	#endif
}				

#endif

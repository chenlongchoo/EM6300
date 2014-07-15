// ModbusServerTCP.c

/*********************************************************************
 *
 *  ModBus Server Application
 *  Module for Microchip TCP/IP Stack
 *   -Implements an example ModBus client and should be used as a basis 
 *	  for creating new TCP client applications
 *	 -Reference: None.  Hopefully AN833 in the future.
 *
 ********************************************************************/

#include "AppConfig.h"
#include "mcu.h"
#include "ModbusTCPServer_Meter.h"
#include "TCPIPConfig.h"
#include "zigbee.h"
#include "eeprom_task.h"
#include "Main_Meter_Top_board.h"
#include "HardwareProfile.h"

#if defined(APP_METER_USE_MODBUS_SERVER)

#include "TCPIP Stack/TCPIP.h"

SOCKET_INFO CommInfo;

// 2012-08-14 Liz: Removed. Need to declare as global variable.
//		Otherwise, fw may crash, some variable will be override.
MCU_REQUEST_BLOCK mcu_req;

// 2012-09-29 Liz
static BOOL bBootloadActivated = FALSE;
static BOOL bResetEeprom = FALSE;
static BOOL bReqForTop = FALSE;
static BOOL bChangeIP = FALSE;

// 2013-09-24 Liz
static BOOL bResetMeter = FALSE;

//static char dig[3]; 
static char new_IP[16] = {0};
static char ver[16] = FIRMWARE_VERSION;

void ModbusReqHandler(MODBUS_REQUEST_BLOCK * req);
//

/*********************************************************
 *
 * Listens for ModBus requests and processes them.
 *
 *********************************************************/
void Modbus_MeterServerTask(void)
{
	static MODBUS_REQUEST_BLOCK modbus_request;
	static DWORD Timer, ACK_Timer;
	static char retries;
	
	static TCP_SOCKET MySocket;
	static MODBUS_SERVER_STATE ModbusServerState;
	
	static BOOL bModbusInitialized = FALSE;
	
	static BYTE CLS_ReceiveCount;
	static char error_flag = 0;
	static char e_array[10], e_i = 0;

	// 2012-05-10(Eric) - Support variable to combat 54/60 issues.
	static BYTE bWait;
	
	// Perform one time initialization on power up
	if(!bModbusInitialized)
	{
		MySocket = INVALID_SOCKET;
		ModbusServerState = MBSS_HOME;
		bModbusInitialized = TRUE;
		memset(&modbus_request, 0, sizeof(MODBUS_REQUEST_BLOCK));
	}

	if((MySocket == INVALID_SOCKET && ModbusServerState != MBSS_DISCONNECT))
		ModbusServerState = MBSS_HOME;	
	
	// Handle session state
	switch(ModbusServerState)
	{
		case MBSS_HOME: // Opens the socket at the specified port to listen for request.
		{
			static char invalid_count = 0;
			// Connect a socket to the remote TCP server
			MySocket = TCPOpen(0, TCP_OPEN_SERVER, MODBUS_PORT, TCP_PURPOSE_MODBUS);
			
			// Abort operation if no TCP socket of type TCP_PURPOSE_MODBUS is available
			// If this ever happens, you need to go add one to TCPIPConfig.h
			if(MySocket == INVALID_SOCKET)
			{
				if( invalid_count++ > 10 )
				{
					// Eric(2012-09-28) - Error codes redefined.
					error_flag = MBE_SERVER_NO_FREE_SOCKET;
					//error_flag = 70;

					invalid_count = 0;
					ModbusServerState = MBSS_DISCONNECT;
				}
				break;
			}	

			Timer = TickGet();
			ModbusServerState = MBSS_LISTENING;
		}	
			break;
		
		case MBSS_LISTENING:  // This listens out for new requests and checks the message header.
		{
			static BYTE received = 0;
			static BYTE w = 0;

			//if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
			if( !TCPIsConnected(MySocket) )
				goto DisconnectSocket;
			
			//2012-10-17 Liz removed. Not in used
			// Get socket info (IP, MAC, remote port)
			//CommInfo = *(SOCKET_INFO *)TCPGetRemoteInfo(MySocket);

			Timer = TickGet();
			/* 
			 * Receive and process the ModBus header.
			 * +------------------+--------------+----------+
			 * |  Transaction ID  | Protocol ID  |  Length  |
			 * |    2-byte        |   2-byte     |  2-byte  |
			 * +------------------+--------------+----------+
			 */
			{
				w += TCPIsGetReady(MySocket);
				
				if(w == 0 && received == 0) break;
				else if(w == 0) 
				{
					// If there are no data to receive, we assume the receive is completed.
					w = 0;
					received = 0;
					Timer = TickGet();
					CLS_ReceiveCount = 0;
					ModbusServerState = MBSS_PROCESS_REQUEST;
					break;
				}
				
				while( w ) 	
				{
					BYTE rec;
					rec = TCPGetArray(MySocket, &modbus_request.Serialised[received], w);
					
					w -= rec;
					received += rec;
					
// Eric(2012-09-14) - Removed. Need to remove this from ModbusClient also.			
//					if( sizeof(MODBUS_REQUEST_BLOCK)-received >= w )
//						received += TCPGetArray(MySocket, &modbus_request.Serialised[received], w);
//					else
//						received += TCPGetArray(MySocket, &modbus_request.Serialised[received], sizeof(MODBUS_REQUEST_BLOCK)-received);
					
//					if( received == sizeof(MODBUS_REQUEST_BLOCK) )
//					{

//						//TCPPutROMString(MySocket, (ROM BYTE*)"ACK");
//						//TCPFlush(MySocket);
//						break;
//					}
//					else if( received > sizeof(MODBUS_REQUEST_BLOCK) )
//					{
//						error_flag = 99;
//						ModbusServerState = MBSS_DISCONNECT;
//						break;
//					}
//					else
//					{
//						w = TCPIsGetReady(MySocket);
//					}
				}
				return;
			}
			
DisconnectSocket:
			w = 0; 
			received = 0;
			
			// Eric(2012-09-28) - Error codes redefined.
			error_flag = MBE_SERVER_CONNECTION_LOST;
			//error_flag = 58;
			
			//ModbusServerState = MBSS_DISCONNECT;
			
			return;
		}
			break;
			
		case MBSS_PROCESS_REQUEST:  
		{	
			// At this point the full modbus request should be received.
			// We process the message here.
			
			/* Format of MODBUS request message format.
			 * Register is in little endian format.
			 * checksum is not required for Modbus TCP protocol.
			 * +-----------+---------------+---------------+------------------+
			 * | Device ID | Function Code | Starting Addr | No. of registers |
			 * |  1-byte   |    1 byte     |    2-byte     |      2-byte      |
			 * +-----------+---------------+---------------+------------------+
			 */
			
			// Connection lost.
			// Eric(2012-09-19) - Check for TCPWasReset() has to be removed otherwise it will not work with C# app.
			//if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
			if( !TCPIsConnected(MySocket) )
			{
				error_flag = 48;
				ModbusServerState = MBSS_DISCONNECT;
				break;
			}	

			ModbusReqHandler(&modbus_request);

			if(bReqForTop)
			{
				// Wait for response.
				ACK_Timer = Timer = TickGet();
				error_flag = 0;
				bWait = 0;  // 2012-05-10(Eric).
				ModbusServerState = MBSS_SEND_RESPONSE;
			}
			else
			{
				switch(modbus_request.w.mbRegister.FunctionCode)
				{
					case 3:  // MODBUS read request (Read Holding Registers).
					{
						// 2012-08-14 Liz: Removed. Need to declare as global variable.
						//		Otherwise, fw may crash, some variable will be override.
						//MCU_REQUEST_BLOCK mcu_req;
						
						// Initialise the MCU_REQUEST_BLOCK.
						memset(&mcu_req, 0, sizeof(MCU_REQUEST_BLOCK));
						// Eric(2012-09-18) - Basically we are copying the requested Modbus data into MCURequestBlock format.
						// We need to copy the results from the MCURequestBlock back into the ModbusRequestBlock later.
						mcu_req.w.header.message_type = MMT_MODBUS_READ_REQUEST;
						mcu_req.w.header.start_addr = modbus_request.w.mbRegister.StartAddr;
						mcu_req.w.header.ByteCount = modbus_request.w.mbRegister.RegisterCount*2;
						mcu_req.w.data.hResult = mcu_req.w.data.lResult = 0;
						
						// Request for reading for this particular register from bottom board.
						if(MCURequestToBOTTOMBoard(MMT_MODBUS_READ_REQUEST, &mcu_req.w.header.start_addr, 4, FALSE, FALSE))
						{
							// Wait for response.
							ACK_Timer = Timer = TickGet();
							error_flag = 0;
							bWait = 0;  // 2012-05-10(Eric).
							ModbusServerState = MBSS_WAIT_RESPONSE;
						}				
					
						break;
					}
					case 6:  // MODBUS write request (Write Single Register).
					case 16: // MODBUS write request (Write Multiple Registers).
					{
						// Eric(2012-09-21) - Moving forward, all registers are assumed to be
						// requests for the MCU except for registers from 0x0C00 and onwards.
					
						// These are all requests for the bottom board.
						mcu_req.w.header.message_type = MMT_MODBUS_WRITE_REQUEST;
						mcu_req.w.header.start_addr = modbus_request.w.mbRegister.StartAddr;
						mcu_req.w.header.ByteCount = modbus_request.w.mbRegister.RegisterCount*2;
						if(mcu_req.w.header.ByteCount >= 40)
						{
							// Return error.
						}
						memcpy(&mcu_req.w.data, &modbus_request.w.data, mcu_req.w.header.ByteCount);
					
						// Request for reading for this particular register from bottom board.
						if(MCURequestToBOTTOMBoard(
							MMT_MODBUS_WRITE_REQUEST, &mcu_req.w.header.start_addr, 
							sizeof(MCU_REQUEST_HEADER)-1+mcu_req.w.header.ByteCount,
							FALSE, FALSE))
						{
							// Wait for response.
							ACK_Timer = Timer = TickGet();
							error_flag = 0;
							bWait = 0;  // 2012-05-10(Eric).
							ModbusServerState = MBSS_WAIT_RESPONSE;
						}
	
						//2012-10-02 Liz: Added. Update meter status flag
						switch(modbus_request.w.mbRegister.StartAddr)
						{
							// 2014-06-10 Liz removed.
//							case 0x0907: 	// Request to update DateTime
//								meter_flag.Flags.Flag1.bDateTimeUpdated = 1;
//								break;
							case 0x0904:	// Request to clear Meter status flag
							{
								BOOL hasReturned = FALSE;
								BYTE mask[2], hold[2];

								mask[0] = ~(*((BYTE*)(&modbus_request.w.data) + 3) & 0b00111111);
								meter_flag.v[0] &= mask[0];
								mask[1] = ~(*((BYTE*)(&modbus_request.w.data) + 4));
								meter_flag.v[1] &= mask[1];		
							}
								break;	
							default:
								break;
						}	
	
						//2012-10-02 Liz: Removed. 
						// Wait for response.
						//ACK_Timer = Timer = TickGet();
						//error_flag = 0;
						//ModbusServerState = MBSS_SEND_RESPONSE;
						break;
					}
					default:
					{
						// Unknown incoming request.
						error_flag = 30;
						ModbusServerState = MBSS_DISCONNECT;
						break;
					}	
				}
			}
		}	
			break;
		
		case MBSS_WAIT_RESPONSE:
		{
			// Connection lost.
			// Eric(2012-09-19) - Check for TCPWasReset() has to be removed otherwise it will not work with C# app.
			//if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
			if( !TCPIsConnected(MySocket) )
			{
				//DWORD lapse = (TickGet()-Timer)/TICK_SECOND;
				if(error_flag!=77 && error_flag != 76)
					error_flag = 78;
				// 2012-05-10(Eric) - This additional state seems to help remove the 54/62 error flag.
				ModbusServerState = MBSS_SEND_ERROR_CODE;
				//ModbusServerState = MBSS_DISCONNECT;
				
				break;
			}	
			
			if( MCUHasNewMessage==0 )
			{
				// Timeout waiting for data from bottom board.
				// 2012-05-10(Eric) - Modifying this section seems to have totally removed 54/60 issues.
				if( TickGet()-Timer > (TICK_SECOND/2) )
				{
					bWait++;
					
					// Instead of abruptly terminating the line, try to send a msg to inform the client.
					// When the client receives the "CLS", it should terminate the connection.
					// We wait 3 more counts to see if the client terminates the connection. Otherwise we force
					// the termination.
					if(bWait == 3)
					{
						// Allow one retry with CLS is received.
						if(CLS_ReceiveCount++ == 0)
							ModbusServerState = MBSS_PROCESS_REQUEST;
						else
						{
							TCPPutROMString(MySocket, (ROM BYTE*)"CLS");
							TCPFlush(MySocket);
						}
						break;
					}	
					// Wait for client to terminate.
					if(bWait==4)
					{
						// Force termination after waiting.
						error_flag = 79;
						ModbusServerState = MBSS_DISCONNECT;
						break;
					}
					Timer = TickGet();
					break;
				}
				
				// Send regulated 'ACK' messages every second to keep the connection alive while waiting for data to come in.
				if( (TickGet()-ACK_Timer>TICK_SECOND) && TCPIsPutReady(MySocket) > 5 )
				{
					error_flag = 77;
					// Eric(2012-09-12) - Why do we need to run MCUTask() here?
					MCUTasks();
					// Eric(2012-09-14) - Removed. Need to remove this from ModbusClient also.
					//TCPPutROMString(MySocket, (ROM BYTE*)"ACK");
					//TCPFlush(MySocket);
					ACK_Timer = TickGet();
					break;
				}
				
				// Still waiting for response from MCU.
				error_flag = 76;
				break;
			}

			Timer = TickGet();
			ModbusServerState = MBSS_SEND_RESPONSE;
			break;
		}
			
		case MBSS_SEND_RESPONSE:
		{
			BYTE w;
			if((TickGet()-Timer)>(3*TICK_SECOND))
			{
				// If we do not have a response back to the client, output an error message instead.
				// 2012-05-09(Eric) - Debug. Change CLS to DLS to differentiate the error occurrance in communicator.
				TCPPutROMString(MySocket, (ROM BYTE*)"DLS");
				TCPFlush(MySocket);

				error_flag = 2;
				ModbusServerState = MBSS_DISCONNECT;
				break;
			}	
			
			switch (modbus_request.w.mbRegister.FunctionCode)
			{
				case 3:
				{
					MCUHasNewMessage = 0;

					if( TCPIsPutReady(MySocket) < sizeof(MODBUS_REQUEST_BLOCK) )
						break;

					// Eric(2012-09-18) - We should copy the needed results from the MCURequestBlock back
					// into the ModbusRequestBlock.
					if(!bReqForTop)
					{
						// Eric(2012-09-20) - Flags removed in ModbusRequestBlock.
						MCU_REQUEST_BLOCK * respMCUBlock = &MCUNewMessage[0];
						
						// 2013-11-12 Liz. Check if refenrece is 0
						if(respMCUBlock == 0)		return;
						//
						
						// 2012-12-07 Liz: added general status flag after MAXQ status flag. 
						switch(respMCUBlock.w.header.start_addr)
						{
							case 0x0904: 	// Request to update Meter Statuc flag
							{
								//memcpy(&respMCUBlock.w.data, meter_flag.v, 2);
								BYTE flag1 = MCUNewMessage[8];
								BYTE flag2 = MCUNewMessage[9];	//2013-03-22 Liz: added for auto-reset flag
								
								meter_flag.Flags.Flag1.bBOTTOMReset = (flag1&0x01);
								meter_flag.Flags.Flag1.bCalibCksModified = (flag1&0x20)>>5;
								meter_flag.Flags.Flag1.bEnergyReset = (flag1&0x10)>>4;					
								meter_flag.Flags.Flag2.bPhaseA_RealPos_C2_Overflow = (flag1&0x02)>>1;
								meter_flag.Flags.Flag2.bPhaseB_RealPos_C2_Overflow = (flag1&0x04)>>2;
								meter_flag.Flags.Flag2.bPhaseC_RealPos_C2_Overflow = (flag1&0x08)>>3;								
								meter_flag.Flags.Flag2.bBOTAutoReset = (flag2&0x80)>>7;	//2013-03-22 Liz: added for auto-reset flag
								meter_flag.Flags.Flag2.bBOTTOReset = (flag2&0x40)>>6;	//2013-03-28 Liz: added for WDTO-reset flag
								meter_flag.Flags.Flag1.bDateTimeUpdated = (flag2&0x20)>>5;	//2014-03-12 Liz added.
								
								memcpy(&MCUNewMessage[8], meter_flag.v, 2);
							}
								break;
							default:
								break;
						}
						
						// Copy the data received through MCURequestBlock into ModbusRequestBlock.
						memcpy(&modbus_request.w.data, &(*respMCUBlock).w.data, (*respMCUBlock).w.header.ByteCount);
						// Also copy the register which can indicate any errors returned by MCU.
						modbus_request.w.mbRegister.StartAddr = (*respMCUBlock).w.header.start_addr;
							
						// Eric(2012-09-19) - We have finised consuming data from MCUNewMessage, release MCU...
						SetMCUIsBusy(FALSE);
					}
					
					// Send the response.
					// Eric(2012-09-18) - Return the ModbusRequestBlock as response.
					w = TCPPutArray(MySocket, &modbus_request, sizeof(MODBUS_REQUEST_BLOCK));
					if( w >= sizeof(MODBUS_REQUEST_BLOCK) )
					{
						// Sending completed.
						TCPFlush(MySocket);
						ModbusServerState = MBSS_COMPLETE;
						break;
					}
					break;
				}
				case 6:
				case 16:
				{
					if( TCPIsPutReady(MySocket) < sizeof(MODBUS_REQUEST_BLOCK) )
						break;
					
					if(!bReqForTop)
					{
						MCU_REQUEST_BLOCK * respMCUBlock = &MCUNewMessage[0];
						
						// 2013-11-12 Liz. Check if refenrece is 0
						if(respMCUBlock == 0)		return;
						//
						
						// For WRITE requests, we just return the if the register value returned from MCU
						// for the software to know if there was an error.
						modbus_request.w.mbRegister.StartAddr = (*respMCUBlock).w.header.start_addr;
						// Release MCU...
						SetMCUIsBusy(FALSE);
					}
					
					// Send the response.
					// Eric(2012-09-18) - Return the ModbusRequestBlock as response.
					//w = TCPPutArray(MySocket, MCUNewMessage, sizeof(MCU_REQUEST_BLOCK));
					w = TCPPutArray(MySocket, &modbus_request, sizeof(MODBUS_REQUEST_BLOCK));
					if( w >= sizeof(MODBUS_REQUEST_BLOCK) )
					{
						// Sending completed.
						TCPFlush(MySocket);
						ModbusServerState = MBSS_COMPLETE;
						break;
					}
					break;
				}
				default:
					break;
			}
			
			break;
		}
			
		case MBSS_COMPLETE:
		{
			// Eric(2012-09-25) - Wait a while for client disconnect.
			static DWORD wait_disconnect = 0;
			if(!TCPIsConnected(MySocket))
			{
				wait_disconnect = 0;
				ModbusServerState = MBSS_DISCONNECT;
				return;
			}
				
			if( wait_disconnect == 0 ) wait_disconnect = TickGet();
			if( TickGet() - wait_disconnect > (TICK_SECOND*4) )
			{
				wait_disconnect = 0;
				error_flag = 5;
				ModbusServerState = MBSS_DISCONNECT;
			}
			break;
		}
		case MBSS_DISCONNECT:
		{
			TCPDisconnect(MySocket);
			TCPDiscard(MySocket);
			TCPClose(MySocket);
			MySocket = INVALID_SOCKET;
			ModbusServerState = MBSS_HOME;
			e_array[e_i] = error_flag;
			if(e_i++ > 9)
				e_i = 0;	
			error_flag = 0;
			memset(&modbus_request, 0, sizeof(MODBUS_REQUEST_BLOCK));
			
			// 2012-09-27 Liz: Hardcode. Set to bootloadmode after complete modbus cycle.
			if(bBootloadActivated)
			{
				RCONbits.NOT_POR = 1;
				Reset();
			}
			else if(bResetMeter)
			{
				// 2013-09-24 Liz added. Reset meter fi have request from server
				RCONbits.NOT_POR = 0;
				Reset();
			}		
			else if(bChangeIP)
			{
				//char s[16] = "192.168.1.179";
				//s[10] = 0;
				//strcat(s, dig);
				//if(StringToIPAddress((BYTE*)s, &AppConfig.MyIPAddr))
				if(StringToIPAddress((BYTE*)new_IP, &AppConfig.MyIPAddr))
				{	
					SaveAppConfig();
					Reset();
				}
			}
			else if(bResetEeprom)
			{
				ResetMemory();
				bResetEeprom = FALSE;
				meter_flag.Flags.Flag1.bEepromReset = 1;
			}	
			
		}
			break;
		case MBSS_SEND_ERROR_CODE:
		{
			ModbusServerState = MBSS_LISTENING;
			e_array[e_i] = error_flag;
			if(e_i++ > 9)
				e_i = 0;	
			error_flag = 0; 
		}	
			break;
	}
		
}
	
/*--------------------------------------------------------------
	Handle modbus request register. Verify if request is for 
		Top or Bottom board.
--------------------------------------------------------------*/
void ModbusReqHandler(MODBUS_REQUEST_BLOCK * req)
{		
	static BYTE t[4] = "";
	
	// 2013-11-12 Liz. Check if refenrece is 0
	if(req == 0)		return;
	//
							
	// If request register is <= 0x0980, it's req for bottom board.
	//		Otherwise, it's for Top board.
	if ((*req).w.mbRegister.StartAddr < 0x0980)
		bReqForTop = FALSE;
	else
		bReqForTop = TRUE;
		
	// 2014-01-17 Liz. Get and remove datetime from the request
	if((*req).w.mbRegister.StartAddr != 0x0970 && (*req).w.mbRegister.StartAddr != 0x0907)
	{
		// get the datetime from system
		memcpy(&DateTime[0], &(*req).w.data.Others[0], 6);
		
		// update Flag so top board can send DateTime to Bottom board later.
		// 	Do not send DateTime to Bottom board here, need to quickly finish Modbus task
		//		and release socket.
		UPDATE_DATETIME = TRUE;
		memmove(&(*req).w.data.Others[0], &(*req).w.data.Others[6], 34);	 
	}	
	//
	
	if(bReqForTop)
	{
		switch((*req).w.mbRegister.FunctionCode)
		{
			case 3:
			{
				switch((*req).w.mbRegister.StartAddr)
				{
					case 0x09C7:	// Request to get eeprom reading
					{
						BOOL result = 0;
						result = EEPROM_Read_Readings(&(*req).w.data,EEP_RECORD_LENGTH);
						
								
						// May need to check if successfully get record
						//if(result)
						//.............................................
					}
						break;
					case 0x0984:	// Request to get Meter status flag
					{
						memcpy(&(*req).w.data, meter_flag.v, 2);
						
						//WORD temp = meter_flag.Flags;
						//(*req).w.data.hResult = (BYTE)(meter_flag.Flags);
					}	
						break;
					case 0x0992:	// Request to get number of unread records
					{
						WORD w = unread_records;
						//(*req).w.data.hResult = w;
						memcpy(&(*req).w.data, &w, 2);
					}
						break;
					case 0x0999:	// Request to read firmware version
					{
						memcpy(&(*req).w.data,ver,16);
					}
						break;	
					case 0x0997:	// Request to toggle LCD backlight
					{
						// 2013-09-24 Liz added.
						// LCD Backlight on
						if(LCD_LED == 1)
						{
							LCD_LED = 0;	// LED is active low
							//(*req).w.data.hResult = 0x0000;
							(*req).w.data.Others[0] = 0;
						}
						else
						{
							LCD_LED = 1;
							//(*req).w.data.hResult = 0x0001;
							(*req).w.data.Others[0] = 1;
						}
					}
						break;	
					case 0x0989:
					{
						// 2013-09-26 Liz added.
						memcpy(&(*req).w.data, (BYTE*)&EEPROM_INTERVAL, 4);
					}
						break;	
					default:
						break;	
				}	
			}
				break;
			case 6:
			case 16:
			{				
				switch((*req).w.mbRegister.StartAddr)
				{
					//Stephen(2012-12-07) Add in new register to change IP.
					case 0x0995:	// Req to change IP
					{
						//memcpy(dig,&(*req).w.data,3);
						memcpy(&new_IP[0],&(*req).w.data,16);
						bChangeIP = TRUE;
					}
						break;
					case 0x0991: 	// Req to enter Bootload mode
					{
						// Turn on the flag, go to disconnect state before
						//		set into Bootload mode.
						bBootloadActivated = TRUE;
					}
						break;
					case 0x09C7:	// Req to clear INDIVIDUAL record in eeprom 
					{
						DWORD location = 0;
								
						// Get location of record need to be clear
						//location = (DWORD)((*req).w.data.hResult);
						memcpy(&location, &(*req).w.data.Others[2], 3);
						
						UpdateRecords(location);
					}
						break;	
					case 0x0982:	// Reset meter (will reset both top and bottom board)
					{
						MCURequestToBOTTOMBoard(MMT_BOT_NORMAL_RESET, t, 1, TRUE, TRUE);
						bResetMeter = TRUE;
					}	
						break;
					case 0x0987:	// Clear ALL records in eeprom
					{
						bResetEeprom = TRUE;
					}	
						break;
					case 0x0981:
					{
						// 2013-09-24 Liz added Credit Balance
						unsigned char len = 0, retries = 0;
						char verifyStr[20];
						
						strcpy(Credit_Balance, &(*req).w.data);
						len = strlen(&Credit_Balance[0]);
						
						while(1)
						{
							XEEBeginWrite(CREDIT_BALANCE_EEP_POS);
							XEEWrite(0x80);
							XEEWriteArray((BYTE*)&Credit_Balance, len+1);	
							XEEReadArray(CREDIT_BALANCE_EEP_POS+1, &verifyStr[0], len+1);
							
							if(memcmp((BYTE*)&Credit_Balance, (BYTE*)&verifyStr, len+1) == 0 )
								break;
							else
								retries++;
							
							if(retries > 1)
								break;
						}
					}	
						break;
					case 0x0989:
					{
						// 2013-09-25 Liz added. Request to change eeprom record interval
						unsigned char retries = 0;
						char verifyStr[4];
						
						memcpy((BYTE*)&EEPROM_INTERVAL, &(*req).w.data, 4);
						
						while(1)
						{
							XEEBeginWrite(EEPROM_INTERVAL_POS);
							XEEWrite(0x80);
							XEEWriteArray((BYTE*)&EEPROM_INTERVAL, 4);	
							XEEReadArray(EEPROM_INTERVAL_POS+1, &verifyStr[0], 4);
							
							if(memcmp((BYTE*)&EEPROM_INTERVAL, (BYTE*)&verifyStr, 4) == 0 )
								break;
							else
								retries++;
							
							if(retries > 1)
								break;
						}
					}
						break;	
					case 0x0984:	// Request to clear Meter status flag
					{
						// 2013-09-23 Liz removed. Register not in use.
						//BOOL hasReturned = FALSE;
						//BYTE mask[2], hold[2];

						//mask[0] = ~(*((BYTE*)(&(*req).w.data)) & 0b00111111);
						//meter_flag.v[0] &= mask[0];
						//mask[1] = ~(*((BYTE*)(&(*req).w.data) + 1));
						//meter_flag.v[1] &= mask[1];		
					}
						break;	
					default:
						break;	
				}
			}
				break;
			default:
				break;		
		}	
	}
	
}

//						//2012-07-18 Liz: Added Save credit balance into eeprom
//						if (modbus_request.w.mbRegister.StartAddr == 0x0941)
//						{
//							unsigned char len = 0, retries = 0;
//							char verifyStr[20];
//						
//							strcpy(Credit_Balance, &modbus_request.w.data);
//							len = strlen(&Credit_Balance[0]);
//						
//							while(1)
//							{
//								XEEBeginWrite(CREDIT_BALANCE_EEP_POS);
//								XEEWrite(0x80);
//								XEEWriteArray((BYTE*)&Credit_Balance, len+1);	
//								XEEReadArray(CREDIT_BALANCE_EEP_POS+1, &verifyStr[0], len+1);
//							
//								if(memcmp((BYTE*)&Credit_Balance, (BYTE*)&verifyStr, len+1) == 0 )
//									break;
//								else
//									retries++;
//							
//								if(retries > 1)
//									break;
//							}
//						}	
	

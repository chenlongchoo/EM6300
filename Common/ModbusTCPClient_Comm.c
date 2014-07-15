// ModbusClientTCP.c

/*********************************************************************
 *
 *  ModBus Client Application
 *  Module for Microchip TCP/IP Stack
 *   -Implements an example ModBus client and should be used as a basis 
 *	  for creating new TCP client applications
 *	 -Reference: None.  Hopefully AN833 in the future.
 *
 *********************************************************************
 * FileName:        GenericTCPClient.c
 * Dependencies:    TCP, DNS, ARP, Tick
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 ********************************************************************/
//#define __MODBUS_C

#include "AppConfig.h"  // Include this so that the app knows whether to include this module when compiling.
#include "mcu.h"
#include "TCPIPConfig.h"
#include "ModbusTCPClient.h"
#include "zigbee.h"

#if defined(APP_USE_MODBUS_CLIENT)

#include "TCPIP Stack/TCPIP.h"

static MODBUS_REQUEST_BLOCK modbus_request;
static char bModbusClientInitialised = 0;
static MODBUS_CLIENT_STATE ModbusClientState = (MODBUS_CLIENT_STATE)MBCS_HOME;

// Returns 0 if there is a current request being processed.
int CreateNewModbusRequest(MAC_ADDR mac_addr, char function_code, WORD start_addr, char reg_count, char* request_data)
{
	BYTE i;
	IP_ADDR ip;
		
	// We need to lookup IP address based on the MAC address.
	ip.Val = 0;
	for( i=0; i<10; i++ )
	{
		NODE_INFO node = *(NODE_INFO*)&iplist.v[i*10];
		
		if( !(node.MACAddr.v[0] == mac_addr.v[0] &&
		      node.MACAddr.v[1] == mac_addr.v[1] &&
	  	      node.MACAddr.v[2] == mac_addr.v[2] &&
		      node.MACAddr.v[3] == mac_addr.v[3] &&
		      node.MACAddr.v[4] == mac_addr.v[4] &&
		      node.MACAddr.v[5] == mac_addr.v[5]) )
			continue;
		
		ip.Val = node.IPAddr.Val;
		break;
	}
	
	// If the MAC address maps successfully to a IP address, set up the modbus request.
	if( ip.Val != 0 && ModbusClientState == MBCS_DISCONNECT )
	{
		ModbusClientState = (modbus_request.w.IPAddr.Val == ip.Val) ? MBCS_DISCONNECT : MBCS_RELEASE;
		
		modbus_request.w.MACAddr = mac_addr;
		modbus_request.w.IPAddr = ip;
		modbus_request.w.FunctionCode = function_code;
		modbus_request.w.StartAddr = start_addr;
		modbus_request.w.RegisterCount = reg_count;
		modbus_request.w.Flags.IsNewRequest = 1;
		for(i=0;i<20;i++)
		{
			modbus_request.w.Request_Data[i] = request_data[i];
		}	
		return 1;
	}
	else
		return 0;  // Modbus request not set up.
}

/*********************************************************
 *
 * Listens for ModBus requests and processes them.
 *
 *********************************************************/
MODBUS_CLIENT_STATE ModbusRequestTask(void)
{
	BYTE 		i, k;
	BYTE		vModbusSession;
	static TCP_SOCKET	MySocket = INVALID_SOCKET;
	//static char		buffer[20];
	static DWORD	Timer;
	static char mbap_response[20] = "";
	static char mbap_header[7] = "";
	static char mbap_data[20] = "";
	static unsigned char modbus_msg_len = 0, modbus_unit_id = 0, dropped_msg = 0;
	static BOOL bModbusInitialized = FALSE;
	static MCU_REQUEST_BLOCK response;
	static unsigned char received = 0;


	static BYTE error_flag = 0;
	static char e_array[10], e_i = 0, e_1 = 0, e_2 = 0;

	if( !bModbusClientInitialised )
	{
		modbus_request.w.IPAddr.Val = 0;
		modbus_request.w.FunctionCode = 0;
		modbus_request.w.Flags.IsNewRequest = 0;
		ModbusClientState = MBCS_DISCONNECT;
		bModbusClientInitialised = 1;
	}

	
	{
		// Check if there are requests to be processed.
		if( modbus_request.w.FunctionCode == 0 )
		{
			ModbusClientState = MBCS_DISCONNECT;
			return ModbusClientState;
		}	
		
		if((MySocket == INVALID_SOCKET && ModbusClientState != MBCS_DISCONNECT))
			ModbusClientState = MBCS_HOME;	
		
		// Handle session state
		switch(ModbusClientState)
		{
			case MBCS_HOME: // Opens the socket at the specified port to listen for request.
			{
				static char invalid_counts = 0;
				
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen(modbus_request.w.IPAddr.Val, TCP_OPEN_IP_ADDRESS, MODBUS_PORT, TCP_PURPOSE_MODBUS);
				
				// Abort operation if no TCP socket of type TCP_PURPOSE_MODBUS is available
				// If this ever happens, you need to go add one to TCPIPConfig.h
				if(MySocket == INVALID_SOCKET)
				{
					if( invalid_counts++ > 2 )	
					{
						invalid_counts = 0;
						error_flag = 68;
						ModbusClientState = MBCS_DISCONNECT;
					}	
					break;
				}	
	
				invalid_counts = 0;
				Timer = TickGet();
				ModbusClientState = MBCS_SOCKET_OBTAINED;
			}	
				break;
			
			case MBCS_SOCKET_OBTAINED:  // This listens out for new requests and checks the message header.
			{
				if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
				{
					//if(TickGet()-Timer > 4*TICK_SECOND)
					if((float)TickGet()-(float)Timer > 1.5f*(float)TICK_SECOND)
					{
						error_flag = 1;
						ModbusClientState = MBCS_DISCONNECT;
					}
					break;
				}
				
				{
					unsigned char w;

					// This is starting of a new message. Clear the TCPIP rx buffer first.
					TCPDiscard(MySocket);

					
					/* Form the MODBUS message format.
					 * Register is in little endian format.
					 * checksum is not required for Modbus TCP protocol.
					 * +-----------+---------------+---------------+------------------+
					 * | Device ID | Function Code | Starting Addr | No. of registers |
					 * |  1-byte   |    1 byte     |    2-byte     |      2-byte      |
					 * +-----------+---------------+---------------+------------------+
					 */

					// Make certain the socket can be written to
					if( TCPIsPutReady(MySocket) < sizeof(MODBUS_REQUEST_BLOCK) )
						break;
					
					w = TCPPutArray(MySocket, modbus_request.Serialised, sizeof(MODBUS_REQUEST_BLOCK));
					
					if( w >= sizeof(MODBUS_REQUEST_BLOCK))
					{
						// Sending completed.
						TCPFlush(MySocket);
						Timer = TickGet();
						received = 0;
						ModbusClientState = MBCS_WAIT_RESPONSE;
						modbus_request.w.Flags.IsNewRequest = 0;
						break;
					}
				
					break;			
				}	
			}
			
			case MBCS_WAIT_RESPONSE:
			{
				char w;
				static char s[50] = "";
				
				if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
				{
					// 2012-05-08(Eric) - When we have error_flag 30, it may mean that the top board has sent "CLS".
					// However since the top board goes to disconnect state before we respond, the connection
					// could already have been cut by the top board before we process the "CLS".
					if( (error_flag / 10) == 6 )
						error_flag += 150;
					else if( error_flag == 0 )
						error_flag = 30;  // Socket reset.
					ModbusClientState = MBCS_DISCONNECT;
					break;
				}
				if( (TickGet()-Timer) > (4*TICK_SECOND) )
				{
					// 2012-05-09(Eric) - Debug.
					if( (error_flag / 10) == 6 )
						error_flag += 100;
					else if( error_flag == 0 )
						error_flag = 31;
//					error_flag = (error_flag==0 ? 31 : error_flag+1);
					// End debug.
					ModbusClientState = MBCS_DISCONNECT;
					break;
				}

				w = TCPIsGetReady(MySocket);
				received += TCPGetArray(MySocket, &s[received], w);
				
				// We are waiting for a MCU_REQUEST_BLOCK data here...
				//if( received >= 3 && strcmppgm2ram(s, (ROM BYTE*)"ACK") == 0 )
				if( received>=3 && s[0]=='A' && s[1]=='C' && s[2]=='K' )
				{
					// Wait patiently...
					// Move along the buffer until we have pass the string of ACKs.
					char k = 0;
					for( k=0; k<(received/3); k++ )
					{
						if( s[k*3]=='A' && s[(k*3)+1] == 'C' && s[(k*3)+2] == 'K' ) {}
						else break;
					}
					// Once we are at the end of the ACKs and the buffer still contains data
					// move the data forward to the first position.
					if( received > k*3 )
					{
						// There are other messages detected.
						// These other messages start at (k*3) with length (received-(k*3)).
						char j;
						for( j=0;j<(received-(k*3));j++ )
							s[j] = s[(k*3)+j];
						received -= (k*3);
					}
					else received = 0;
					
					TCPPut(MySocket, 0xFF);  	// An attempt to keep the link alive. 
												// This is newly added and not supported in ModbusTCPServer.c.
					
					if(error_flag == 0) error_flag = 60;
					else error_flag++;
					
					break;
				}
				else if( received>=3 && s[0]=='C' && s[1]=='L' && s[2]=='S' )
				{
					// Meter is unable to gather the response needed for the request and sends CLS.
					// This is not a big problem. Just need to retry on server end.
					error_flag = 54;
					ModbusClientState = MBCS_DISCONNECT;
					break;
				}
				else if( received>=3 && s[0]=='D' && s[1]=='L' && s[2]=='S' )
				{
					error_flag = 55;
					ModbusClientState = MBCS_DISCONNECT;
					break;
				}	
				else
				{
					// When we reach here, it means we seem to have received actual data.
					switch(modbus_request.w.FunctionCode)
					{
						case 3:
						{
							if( received >= sizeof(MCU_REQUEST_BLOCK) )
							{
								// Receive completed.
								// We do not need an active connection to the server anymore.
								// No checking of the connection needed.
								char p;
								for( p=0; p<sizeof(MCU_REQUEST_BLOCK); p++ )
									response.Serialised[p] = s[p];
	
								// 2012-09-10(Eric) - Set error code to 0 if we can successfully proceed to next state.
								error_flag = 0;
								ModbusClientState = MBCS_PROCESS_RESPONSE;
								break;
							}
						}	
							break;
						case 6:
						{
							if( received == 0 ) break;

							if( received>=3 )
							{
								if((s[0] == 'O') && (s[1] == 'K') && (s[2] == '~'))
									received = 0;
								else
								{
									s[0] = 'F';
									s[1] = 'A';
									s[2] = '~';
									received = 0;
								}	
									//ZigbeeTransmitRequest(3,s);
									strcpy(response.w.raw,s);
									
									// 2012-09-10(Eric) - Set error code to 0 if we can successfully proceed to next state.
									error_flag = 0;
									ModbusClientState = MBCS_PROCESS_RESPONSE;
							}	
							
						}
					}
			
				}
				break;
			}	

			case MBCS_PROCESS_RESPONSE:  // Processes the request and forms the response packet.
			{
				switch(modbus_request.w.FunctionCode)
				{
					case 3:  // MODBUS read request.
					{
						// Send out the responses back via Zigbee.
						// 1) MAC addr - 6 bytes.
						// 2) Register - 2 bytes.
						// 3) Reading - 8 bytes.
						BYTE n;
						BYTE reply[24];
						int d_len = response.w.raw[0];
				
						// Message format.
						// +------+-------------+---------------+--------------+------+
						// | 0xA1 | MAC Address | Start Address | Is_Raw_Value | Data |
						// +------+-------------+---------------+--------------+------+
						//
						reply[0] = 0xA1;  // Message type for Modbus response.
						for(n=0;n<6;n++) reply[n+1] = modbus_request.w.MACAddr.v[n];
						*((WORD*)&reply[7]) = response.w.start_addr;	// changed to response.w.StartAddr instead of modbus_request
						reply[9] = response.w.is_raw_value;
						if(response.w.is_raw_value == 1)  // This would usually be energy readings where registers start with 8C, 8D and 87.
						{
							for(n=0;n<d_len+3;n++) reply[10+n] = response.w.raw[n];
							ZigbeeTransmitRequest(13+d_len, reply);
						}	
						else
						{
							WORD startsWith = response.w.start_addr & 0xFF0;	// LIZ 2012-02-16 changed to response.w.StartAddr instead of modbus_request
							
							// Determine the divide by value.
							if(startsWith == 0x0840 || startsWith == 0x0800 || startsWith == 0x0810 || startsWith == 0x0820)
								reply[10] = response.w.raw[0];
							else reply[10] = 0xFF;
			
							*(long*)&reply[11] = response.w.result_l;
							*(long*)&reply[15] = response.w.result_h;
							ZigbeeTransmitRequest(19, reply);
						}
						ModbusClientState = MBCS_DISCONNECT;						
						error_flag = 5;
						break;
					}
					case 6:  // MODBUS write request.
					{
						BYTE n;
						BYTE reply[24];
						//int d_len = sizeof(response.w.raw);
						reply[0] = 0xA1;  // Message type for Modbus response.
						for(n=0;n<6;n++) reply[n+1] = modbus_request.w.MACAddr.v[n];
						*((WORD*)&reply[7]) = modbus_request.w.StartAddr;
						reply[9] = 1;
						for(n=0;n<3;n++)reply[10+n] = response.w.raw[n];
						
						ZigbeeTransmitRequest(13, reply);
						
						ModbusClientState = MBCS_DISCONNECT;	
						error_flag = 5;
						break;
					}	
					default:
						error_flag = 7;
						break;
				}	
			}
				break;
			
			case MBCS_DISCONNECT:
			{	
				if( MySocket != INVALID_SOCKET )
				{
					TCPDiscard(MySocket);
					TCPDisconnect(MySocket);
					TCPClose(MySocket);
					MySocket = INVALID_SOCKET;
				}	
				e_array[e_i] = error_flag;
				// If reading was not successfully returned, send a error message back.
				if(error_flag!=5)
				{
					// +------+-------------+------------+
					// | 0xA2 | MAC Address | error_code |
					// +------+-------------+------------+
					//
					BYTE n;
					BYTE reply[24];
					
					reply[0] = 0xA2;
					for(n=0;n<6;n++) reply[n+1] = modbus_request.w.MACAddr.v[n];
					reply[7] = error_flag;
					
					ZigbeeTransmitRequest(8, reply);
				}
				e_i++;
				if(e_i > 10)
					e_i = 0;	

				error_flag = 0;
				// We already reported back the error, so we just drop the message and wait for a new request.
				//if( modbus_request.w.Flags.IsNewRequest == 0 )
				{
					modbus_request.w.FunctionCode = 0;
					modbus_request.w.IPAddr.Val = 0;
				}
				
				ModbusClientState = (MODBUS_CLIENT_STATE)MBCS_HOME;
				break;
			}
			case MBCS_RELEASE:
			{
				// Call this if we are going to connect to another server.
				if( MySocket != INVALID_SOCKET )
				{
					TCPDiscard(MySocket);
					TCPDisconnect(MySocket);
					TCPClose(MySocket);
					MySocket = INVALID_SOCKET;
				}
				ModbusClientState = (MODBUS_CLIENT_STATE)MBCS_DISCONNECT;
				break;
			}
			default:
				break;
		}
		
		return ModbusClientState;
	}		
}  // #if defined(APP_USE_MODBUS_CLIENT).
	

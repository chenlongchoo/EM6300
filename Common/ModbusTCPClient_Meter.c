// ModbusClientTCP.c

//#define __MODBUS_C

#include "AppConfig.h"  // Include this so that the app knows whether to include this module when compiling.
#warning Eric(2012-09-25) - To remove this mcu.h.
//#include "mcu.h"
#include "TCPIPConfig.h"
#include "ModbusTCPClient_Comm.h"
#include "zigbee.h"

#if defined(APP_METER_USE_MODBUS_CLIENT)

#include "TCPIP Stack/TCPIP.h"

static MODBUS_REQUEST_BLOCK modbus_request;
static IP_ADDR request_ip;
static MAC_ADDR request_mac;

static char bModbusClientInitialised = 0;
static MODBUS_CLIENT_STATE ModbusClientState = (MODBUS_CLIENT_STATE)MBCS_HOME;

void DebugCreateMbRequest(void)
{
	IP_ADDR ip;
	MAC_ADDR dmac;
	BYTE mac_str[] = "00:04:A3:15:90:4B";
	BYTE data_d[] = {0x38, 0x40, 0xBF, 0x12, 0xB2, 0x75, 0xD3, 0x1E, 0x2E, 0x40,
					0x54, 0x3D, 0x2E, 0x40, 0x54, 0x3D, 0x2E, 0x40, 0x54, 0x3D,
					0x41, 0x00, 0x3C, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00
	};

	StringToMACAddress(mac_str, &dmac);
	ROMStringToIPAddress("192.168.1.8", &ip);	
	
		request_mac = dmac;
		// Eric(2012-05-25) - Changed...
		//modbus_request.w.IPAddr = ip;
		request_ip = ip;
		modbus_request.w.mbRegister.FunctionCode = 16;
		modbus_request.w.mbRegister.StartAddr = 0x970;
		modbus_request.w.mbRegister.RegisterCount = 15;
		memcpy(&modbus_request.w.data, data_d, 30);
		modbus_request.w.header.TransactionID = RandomGet();
//		modbus_request.w.Flags.IsNewRequest = 1;
}	

// Returns 0 if there is a current request being processed.
int Create_MeterModbusRequest(MAC_ADDR mac_addr, char function_code, WORD start_addr, char reg_count, char* request_data)
{
	BYTE i;
	IP_ADDR ip;
		
	// We need to lookup IP address based on the MAC address.
	ip.Val = 0;
	for( i=0; i<10; i++ )
	{
		NODE_INFO node = *(NODE_INFO*)&iplist.v[i*10];
		
		// 2012-05-32(Eric) - Change checking order.
		if( !(node.MACAddr.v[5] == mac_addr.v[5] &&
		      node.MACAddr.v[4] == mac_addr.v[4] &&
	  	      node.MACAddr.v[3] == mac_addr.v[3] &&
		      node.MACAddr.v[2] == mac_addr.v[2] &&
		      node.MACAddr.v[1] == mac_addr.v[1] &&
		      node.MACAddr.v[0] == mac_addr.v[0]) )
			continue;
		
		ip.Val = node.IPAddr.Val;
		break;
	}
	
	// If the MAC address maps successfully to a IP address, set up the modbus request.
	// 2012-05-31(Eric) - The state machine doesn't actually stay in MBCS_DISCONNECT for a long time.
	// So the condition, ModbusClientState == MBCS_DISCONNECT, may cause new requests to be skipped.
	// Change to ModbusClientState == MBCS_HOME.
	if( ip.Val != 0 && ModbusClientState == MBCS_HOME )
	{
		//ModbusClientState = (modbus_request.w.IPAddr.Val == ip.Val) ? MBCS_DISCONNECT : MBCS_RELEASE;
		
		// Eric(2012-05-25) - Changed...
		//modbus_request.w.MACAddr = mac_addr;
		request_mac = mac_addr;
		// Eric(2012-05-25) - Changed...
		//modbus_request.w.IPAddr = ip;
		request_ip = ip;
		modbus_request.w.mbRegister.FunctionCode = function_code;
		modbus_request.w.mbRegister.StartAddr = start_addr;
		modbus_request.w.mbRegister.RegisterCount = reg_count;
		modbus_request.w.header.TransactionID = RandomGet();
//		modbus_request.w.Flags.IsNewRequest = 1;
		
		// Eric(2012-09-25) - Check null;
		if( request_data != 0 )
			memcpy(&modbus_request.w.data, request_data, 20);

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
MODBUS_CLIENT_STATE Modbus_MeterClientTask(void)
{
	BYTE 		i, k;
	BYTE		vModbusSession;
	static TCP_SOCKET	MySocket = INVALID_SOCKET;
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
		request_ip.Val = 0;
		modbus_request.w.mbRegister.FunctionCode = 0;
		ModbusClientState = MBCS_DISCONNECT;
		bModbusClientInitialised = 1;
	}
	
	// Check if there are requests to be processed.
	if(MySocket == INVALID_SOCKET & request_ip.Val == 0)
	{
		// Eric(2012-09-25) - Force the state machine to return to MBCS_HOME via MBCS_DISCONNECT and wait
		// for a new request. Do not call return here as we want the state machine to disconnect now.
		if(ModbusClientState == MBCS_HOME) return ModbusClientState;
		else ModbusClientState = MBCS_DISCONNECT;
//		else return ModbusClientState;  // Eric(2012-09-25) - Changed return value.
	}
	
	// Handle session state
	switch(ModbusClientState)
	{
		case MBCS_HOME: // Opens the socket at the specified port to listen for request.
		{
			static char invalid_counts = 0;
			
			// Connect a socket to the remote TCP server// Eric(2012-09-25)
			//MySocket = TCPOpen(modbus_request.w.IPAddr.Val, TCP_OPEN_IP_ADDRESS, MODBUS_PORT, TCP_PURPOSE_MODBUS);
			MySocket = TCPOpen(request_ip.Val, TCP_OPEN_IP_ADDRESS, MODBUS_PORT, TCP_PURPOSE_MODBUS);
			
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
			//if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
			if( !TCPIsConnected(MySocket) )
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
				BYTE request_length;

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


				// Eric(2012-09-25) - In the new codes, not the entire Modbus request block is sent.
				// The length of data to sent is dependent on the "Length" data field specified in the Modbus header.
				switch(modbus_request.w.mbRegister.FunctionCode)
				{
					case 3:
						request_length = 11;
						break;
					case 6:
					case 16:
						request_length = 11+(modbus_request.w.mbRegister.RegisterCount*2);
						break;
					default:
						// Eric(2012-09-25) - Function code not supported. We should throw an error as 
						// required by the Modbus Specs.
						break;
				}
				
				// Make certain the socket can be written to.
				if( TCPIsPutReady(MySocket) < sizeof(MODBUS_REQUEST_BLOCK) )
					break;
					
					
				w = TCPPutArray(MySocket, modbus_request.Serialised, request_length);
				
				if( w >= request_length)
				{
					// Sending completed.
					TCPFlush(MySocket);
					Timer = TickGet();
					received = 0;
					ModbusClientState = MBCS_WAIT_RESPONSE;
//					modbus_request.w.Flags.IsNewRequest = 0;
					break;
				}
			
				break;			
			}	
		}
		
		case MBCS_WAIT_RESPONSE:
		{
			char w;
			static char s[60] = "";
			
			//if( !TCPIsConnected(MySocket) || TCPWasReset(MySocket) )
			if( !TCPIsConnected(MySocket) )
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

			w += TCPIsGetReady(MySocket);
			
			if(w == 0 && received == 0) break;
			else if(w == 0)
			{
				// There are no more data to receive, we assume the receive is complete.
				MODBUS_REQUEST_BLOCK *respMbBlock = &s[0];
				
				// Check the TransactionID, register and copy data.
				if((*respMbBlock).w.header.TransactionID != modbus_request.w.header.TransactionID)
				{
					// Eric(2012-09-25) - Should throw an error here and exit.
					break;
				}	
				else
				{
					modbus_request.w.mbRegister.StartAddr = (*respMbBlock).w.mbRegister.StartAddr;
					memcpy(&modbus_request.w.data, &(*respMbBlock).w.data, (*respMbBlock).w.mbRegister.RegisterCount*2);
				}
				
				// Reset parameters for next request.
				w = 0;
				received = 0;
				ModbusClientState = MBCS_PROCESS_RESPONSE;
				break;
			}
			
			while(w)
			{
				BYTE rec;
				rec = TCPGetArray(MySocket, &s[received], w);
				
				w -= rec;
				received += rec;
			}	
			
			return;
			
			// We are waiting for a MCU_REQUEST_BLOCK data here...
			if( received>=3 && s[0]=='A' && s[1]=='C' && s[2]=='K' )
			{
				// Eric(2012-09-25) - Entire code removed.We will no longer be receiving ACK in the new codes.
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
				// Just retry the request on server end.
				error_flag = 55;
				ModbusClientState = MBCS_DISCONNECT;
				break;
			}	
			break;
		}	

		case MBCS_PROCESS_RESPONSE:  // Processes the request and forms the response packet.
		{
			static BYTE reply[55];

			switch(modbus_request.w.mbRegister.FunctionCode)
			{
				case 3:  // MODBUS read request.
				{
					// Send out the responses back via Zigbee.
					// 1) MAC addr - 6 bytes.
					// 2) Register - 2 bytes.
					// 3) Reading - 8 bytes.
					int d_len = modbus_request.w.mbRegister.RegisterCount*2;
			
					// Message format.
					// +------+-------------+---------------+--------------+------+
					// | 0xA1 | MAC Address | Start Address | Is_Raw_Value | Data |
					// +------+-------------+---------------+--------------+------+
					//
					reply[0] = 0xA1;  // Message type for Modbus response.
					// Eric(2012-09-25) - Removed to use memcpy.
					memcpy(&reply[1], request_mac.v, 6);  // Copy mac address.
					
					*((WORD*)&reply[7]) = modbus_request.w.mbRegister.StartAddr;	// changed to response.w.StartAddr instead of modbus_request
					reply[9] = response.w.is_raw_value;
					// Eric(2012-09-25) - Copy data freely, regardless of whether it is raw value or not.
					//if(response.w.is_raw_value == 1)  // This would usually be energy readings where registers start with 8C, 8D and 87.
					{
						// Eric(2012-09-25) - Changed to use memcpy.
						memcpy(&reply[10], &modbus_request.w.data, d_len);
						ZigbeeTransmitRequest(13+d_len, reply);
					}

					ModbusClientState = MBCS_DISCONNECT;						
					error_flag = 5;
					break;
				}
				case 6:  // MODBUS write request.
				case 16:
				{
					// Eric(2012-09-25) - For write requests, we will only be receiving the modbus header
					// and register block. Any errors is indicated in the start address where
					// (start_addr & 0xF000) = 0x8000.
					reply[0] = 0xA1;  						// Message type for Modbus response.
					memcpy(&reply[1], request_mac.v, 6);  	// Copy mac address.
					*((WORD*)&reply[7]) = modbus_request.w.mbRegister.StartAddr;
					
					ZigbeeTransmitRequest(9, reply);
					
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
			if(error_flag!=5 && error_flag!=0)
			{
				// +------+-------------+------------+
				// | 0xA2 | MAC Address | error_code |
				// +------+-------------+------------+
				//
				BYTE n;
				BYTE reply[24];
				
				reply[0] = 0xA2;
				for(n=0;n<6;n++)
					reply[n+1] = request_mac.v[n];
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
				modbus_request.w.mbRegister.FunctionCode = 0;
				request_ip.Val = 0;
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
			
}  // #if defined(APP_METER_USE_MODBUS_CLIENT).
	

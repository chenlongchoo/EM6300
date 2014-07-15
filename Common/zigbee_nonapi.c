//zigbee.c


#include "HardwareProfile.h"
#include "AppConfig.h"

#include "C:\MCC18\h\usart.h"

//#ifdef APP_USE_ZIGBEE

#include <delays.h>
#include <stdlib.h>
#include <string.h>
#include <timers.h>
#include "C:\MCC18\h\usart.h"

#include "power_hold.h"
#include "zigbee.h"


char Zigbee_getcUSART(void);
unsigned char Zigbee_EnterCommandMode(void);

#pragma idata ZigbeeNodesBuffer
char node_list[10][12];

#pragma idata ZigbeeDataBuffer
//char INCOMING_BUFFER[60] = "";

#pragma idata

char ZIGBEE_TASKS = 0;

char			NETWORK_ADDRESS[5];
char			DEVICE_TYPE[2];
char			SERIAL_NUMBER[17];
char			PARENT_ADDRESS[32] = "";


static char		CURRENT_DEST_ADDR[5] = "FFFF";
char			IS_REGISTERED_TO_SERVER=0;
unsigned char 	LOCK_RX_BUFFER = 0;
unsigned char	RECEIVED_SERVER_NUDGE = 0;




#define ZIGBEE_RX_BUFFER_SIZE	60
#define ZIGBEE_TX_BUFFER_SIZE	2
// Variable for Zigbee buffers holding data to be transmitted.
volatile unsigned char 	ZIGBEE_TX_BUFFER_POINTER = 0;
volatile far char 		ZIGBEE_TX_BUFFER[ZIGBEE_TX_BUFFER_SIZE][60] = {"\0"};
// Variable for Zigbee buffers holding recevied data.
volatile unsigned char 	ZIGBEE_RX_BUFFER_POINTER = 0;
volatile far char 		ZIGBEE_RX_BUFFER[ZIGBEE_RX_BUFFER_SIZE] = {"\0"};

volatile unsigned char 	HI_SPEED_INTERVAL=0;
//Variables for ping interval. Each count is an interval of 1s set up in Timer0.
volatile unsigned long 	PING_INTERVAL=0;

//int warning_alarm_counter=0;


//#pragma code high_vector=0x08
//void interrupt_at_high_vector(void)
//{
//  _asm GOTO zigbee_high_isr _endasm
//}
//#pragma code /* return to the default code section */

//#pragma interrupt zigbee_high_isr
//void zigbee_high_isr (void)
//{
//	char c;
//	
//	if( INTCONbits.TMR0IF ) //TIMER0.
//	{
//		INTCONbits.GIEH = 0;  //Stop interrupts.
//		//
//		if( HI_SPEED_INTERVAL > 0 ) HI_SPEED_INTERVAL--;
//		//
//		PING_INTERVAL++;
//		WriteTimer0(46876);
//		INTCONbits.TMR0IF = 0;
//		INTCONbits.GIEH = 1;  //Enable interrupts.
//	}
//	if( LOCK_RX_BUFFER == 0 && ZIGBEE_DATARDY )
//	{
//		//if( LOCK_RX_BUFFER == 1 ) return;
//		//INTCONbits.GIEH = 0;  //Stop interrupts.
//		while( ZIGBEE_DATARDY )
//		{
//			if( ZIGBEE_RX_BUFFER_POINTER >= ZIGBEE_RX_BUFFER_SIZE-1 ) ZIGBEE_RX_BUFFER_POINTER = 0;
//			c = Zigbee_getcUSART();  //c = ZIGBEE_READ;
//			ZIGBEE_RX_BUFFER[ZIGBEE_RX_BUFFER_POINTER++] = c == 0 ? 13 : c;
//			//ZIGBEE_RX_BUFFER[ZIGBEE_RX_BUFFER_POINTER] = 0;
//			
//			if( c == 13 )
//			{
//				LOCK_RX_BUFFER = 1;
//				ZIGBEE_RX_INTERRUPT_EN = 0;
//			}	
//		}
//		//INTCONbits.GIEH = 1;  //Enable interrupts.
//	}	
//}
//
#pragma code
//#pragma SECTION stackregion

void ZigbeeOpen(void)
{
	unsigned int br = 0;
	
	#if !defined(CLOCK_SPEED)
		#error "No processor clock speed defined."
	#else
		#if defined(__18F87J50) && (CLOCK_SPEED == 48000000)
			Open1USART(
				USART_TX_INT_OFF
				& USART_RX_INT_OFF
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				77);
		#elif defined(__18F2455) && (CLOCK_SPEED == 48000000)
			OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_OFF
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_LOW,
				77);
		#elif defined(__18F26K20) && (CLOCK_SPEED == 32000000)
			OpenUSART(
				USART_TX_INT_OFF
				& USART_RX_INT_OFF
				& USART_ASYNCH_MODE
				& USART_EIGHT_BIT
				& USART_CONT_RX
				& USART_BRGH_HIGH,
				207);
		#else
			#error "No supported controller defined."
		#endif
	#endif
	
	INTCONbits.GIEL = 1;
	INTCONbits.GIEH = 1;
}

void ZigbeeClose(void)
{
	#if defined(__18F87J50)
		Close1USART();
	#elif defined(__18F2455) || defined(__18F26K20)
		CloseUSART();
	#else
		#error "No supported controller defined."
	#endif
//	while( Busy1USART() );
//	Close1USART();
}

void ZigbeeSendStringPGM(rom char * str)
{
	char s[40];
	
	strcpypgm2ram(s, str);
	ZigbeeSendString(s);
}

//void ZigbeeAPISendStringPGM(char identifier, unsigned int data_length, rom char * data)
//{
//	char s[40];
//	
//	strcpypgm2ram(s, data);
//	ZigbeeAPISendString(identifier, data_length, s);
//}
	
// Four bytes of data are general headers and applies for every packets that will be sent.
// The following four bytes will be populated by this function.
// First byte - start byte denoted by 0x7E.
// Second and third byte - size of the entire packet less these 4 bytes.
// Last byte - the checksum.
// The first byte within the data packet is the message type.
void ZigbeeAPISendString(unsigned int data_length, char * data)
{
	unsigned char checksum = 0, retry = 0, ch = 0;
	unsigned long i = 0;

// Set frame ID.
//	data[1] = ZIGBEE_TX_BUFFER_POINTER + 1;  // We cannot set frame ID to 0.
//	{
//		ZIGBEE_TX_BUFFER[ZIGBEE_TX_BUFFER_POINTER][0] = 1;  // Retry byte. Activate by setting to 1.
//		ZIGBEE_TX_BUFFER[ZIGBEE_TX_BUFFER_POINTER][1] = data_length;
//		for( i=0; i<data_length; i++ )
//			ZIGBEE_TX_BUFFER[ZIGBEE_TX_BUFFER_POINTER][i+2] = data[i];
//	
//		// Increment ZIGBEE_TX_BUFFER_POINTER.
//		ZIGBEE_TX_BUFFER_POINTER++;
//		if( ZIGBEE_TX_BUFFER_POINTER >= ZIGBEE_TX_BUFFER_SIZE ) ZIGBEE_TX_BUFFER_POINTER = 0;
//	}	
	//while( retry < 10 )
	{
		Zigbee_WriteByte(0x7E);
		Zigbee_WriteByte(data_length & 0xFF00);
		Zigbee_WriteByte(data_length & 0x00FF);
		for( i=0; i<data_length; i++ )
		{
			Zigbee_WriteByte(data[i]);
			checksum += data[i];
		}
		Zigbee_WriteByte(0xFF-checksum);
		
		Delay1KTCYx(40);
		
		// This polling and waiting for transmit status is taking too long and stalling
		// the TCP/IP and USB tasks. Either recode this or remove it.
		// Wait for transmit status only if this is a transmit request.
		if( data[0] == 0x10 )
		{
			while( ((ch = ProcessIncomingMessage()) != 0x8B) && (i++ < 0x4FFFE) );
		}
		if( i > 0x4FFFE && data[0] == 0x10 ) retry++;
		else
			retry = 0xFF;
	}
}

void ZigbeeSendString(char * str)
{
	while(*str)
	{
		Zigbee_WriteByte(*str);
		*str++;	
	}
	Zigbee_WriteByte(13);
}

void ZigbeeAPIDiscoverNodes(void)
{
	//char counter = 0, para_num = 0, node_num = 0, c1 = 0, c2 = 0, c3 = 0;
	//unsigned int timeout = 0;

	{
		int i=0;
		for( i=0; i<10; i++ )
			node_list[i][0] = 0;
	}
	{  // Reset receive buffers.
		ZIGBEE_RX_BUFFER_POINTER = ZIGBEE_RX_BUFFER[0] = 0;
	}
	{
		char s[8] = {0x08, 0x01, 0x4E, 0x44};  // , 0x64};
		//char s[8] = {0x7E, 0x00, 0x04, 0x08, 0x01, 0x4E, 0x44, 0x64};
		ZigbeeAPISendString(4, s);
		
	}
	{
		unsigned long i=0, j=0;
		while( !ZIGBEE_RX_INTERRUPT_FLAG && i++ < 400000 );
		//while( i++ < 64000 )
		j=0;
		ProcessIncomingMessage();
			
		//i = 0;
	}		
}

unsigned char ProcessOutgoingMessage(void)
{
	// Go through the list of TX buffer and increment the retry byte.
	int i = 0;
	for( i=0; i<ZIGBEE_TX_BUFFER_SIZE; i++ )
	{
		// Ignore if retry byte is 0;
		if( ZIGBEE_TX_BUFFER[i][0] == 0 ) continue;
		ZIGBEE_TX_BUFFER[i][0]++;
		// If retry byte reaches 0x0F, resend the message.
		if( ZIGBEE_TX_BUFFER[i][0] >= 0x0F )
		{
			ZigbeeAPISendString(ZIGBEE_TX_BUFFER[i][1], &ZIGBEE_TX_BUFFER[i][2]);
			ZIGBEE_TX_BUFFER[i][0] = 0;
		}	
	}	
}

#if defined( SOLARPANEL_BOARD )
unsigned char ProcessIncomingMessage(void)
{
	char counter = 0, para_num = 0, node_num = 0, c1 = 0, c2 = 0, c3 = 0;
	
	if( !ZIGBEE_RX_INTERRUPT_FLAG ) return -1;
	
	// Unload all data from USART into buffer.
	while( ZIGBEE_RX_INTERRUPT_FLAG )
	{
		if( (c1 = Zigbee_getcUSART()) != 0x7E && ZIGBEE_RX_BUFFER_POINTER == 0 ) break;
		ZIGBEE_RX_BUFFER[ZIGBEE_RX_BUFFER_POINTER++] = c1;
		Delay100TCYx(130);
	}	
}	
#endif

#if defined( TRANSCEIVER_BOARD )
// Returns identifier of received message.
unsigned char ProcessIncomingMessage(void)
{
	char counter = 0, para_num = 0, node_num = 0, c1 = 0, c2 = 0, c3 = 0;
	
	if( !ZIGBEE_RX_INTERRUPT_FLAG ) return -1;
	
	// Unload all data from USART into buffer.
	while( ZIGBEE_RX_INTERRUPT_FLAG )
	{
		if( (c1 = Zigbee_getcUSART()) != 0x7E && ZIGBEE_RX_BUFFER_POINTER == 0 ) break;
		ZIGBEE_RX_BUFFER[ZIGBEE_RX_BUFFER_POINTER++] = c1;
		Delay100TCYx(130);
	}
	
	// Process the messages in the buffer.
	{
		char identifier;  // , cmd_name[2];
		unsigned char checksum = 0;
		unsigned int data_length = 0;
				
		//if( ZIGBEE_RX_BUFFER_POINTER < 5 || ZIGBEE_RX_BUFFER[0] != 0x7E )
		if( ZIGBEE_RX_BUFFER[0] != 0x7E )
			ZIGBEE_RX_BUFFER[0] = ZIGBEE_RX_BUFFER_POINTER = 0;
			
		if( ZIGBEE_RX_BUFFER_POINTER < 5 ) return -1;
		
		// Get expected length of data we should receive.
		data_length |= ZIGBEE_RX_BUFFER[1];
		data_length <<= 8;
		data_length |= ZIGBEE_RX_BUFFER[2];
		
		// Check if we received this length of data.
		if( data_length < ZIGBEE_RX_BUFFER_POINTER - 4 ) return -1;
		
		identifier = ZIGBEE_RX_BUFFER[3];
		
		switch( identifier )
		{
			case 0x8B:
				{
				char frameid = ZIGBEE_RX_BUFFER[4];
				char status = ZIGBEE_RX_BUFFER[8];
				ZIGBEE_TX_BUFFER[frameid-1][0] = 0;  // Set retry count to 0 to stop retry checks.
				//int i = 0;
				}
				break;
			case 0x90:  // Packet received.
				{
					char i=0;
					char src[12] = "", data[20] = "";
					unsigned int data_length = 0;
					
					// Get the length of the data in this packet.
					data_length = ZIGBEE_RX_BUFFER[1];
					data_length <<= 8;
					data_length |= ZIGBEE_RX_BUFFER[2];
					
					// Get serial number and network address of the source node.
					for( i=0; i<10; i++ )
						src[i] = ZIGBEE_RX_BUFFER[4+i];
					
					// Extract the data from this packet.
					for( i=0; i<data_length-12; i++ )
						data[i] = ZIGBEE_RX_BUFFER[15+i];
					data[data_length-12] = 0;
			
					{  // Process the data.
						char msg_type = data[0];
						switch( msg_type )
						{
							case 0x10:  // Request reading.
								{
									char * msg = &data[1];
									char i = 0;
									
									for( i=0; i<4; i++ )
									{
										char * r = PENDING_REQUESTS[PENDING_REQUESTS_POINTER];
										PENDING_REQUESTS_POINTER++;
										if( PENDING_REQUESTS_POINTER > 3 ) PENDING_REQUESTS_POINTER = 0;

										//if( PENDING_REQUESTS[i][0] == 0 && PENDING_REQUESTS[i][1] == 0 )
										{
											char j = 0;
											char para[6];
											unsigned int para_i;
											
											strcpy(para, &msg[1]);
											//strncpy(para, msg, 4);
											para_i = atoi(para);
											// Append register.
											r[0] = (para_i >> 8);
											r[1] = (para_i & 0x00FF);
											// Append phase.
											r[2] = msg[0];
											// Append node details.
											for( j=3; j<12; j++ )
												r[j] = src[j-3];
											break;
										}
									}	
								}	
								break;	
							case 0x20:  // Reading value.
								{
									// Populate RESPONSE_RECEIVED.
									char i=0;
									char * r = RESPONSE_RECEIVED[RESPONSE_RECEIVED_POINTER++];
									if( RESPONSE_RECEIVED_POINTER == 10 ) RESPONSE_RECEIVED_POINTER = 0;
									// Copy 2-byte reading register.
									r[0] = data[1];
									r[1] = data[2];
									// Copy phase.
									r[2] = data[3];
									for( i=0; i<10; i++ )
										r[3+i] = src[i];
									for( i=0; i<data_length-13-2; i++ )
										r[13+i] = data[i+4];
								}	
								break; 
							default:
								ZIGBEE_RX_BUFFER[0] = ZIGBEE_RX_BUFFER_POINTER = 0;
						}	
//						char s[30] = {0x10, 0x01};
//						//char data_len = 16;
//						for( i=0; i<10; i++ )
//						{
//							s[2+i] = src[i];
//						}	
//						s[12] = 0;
//						s[13] = 0;
//				
//						s[14] = 'N';
//						s[15] = 'A';
//							
//						s[16] = 0;
//				
//						ZigbeeAPISendString(16, s);
					}
					
					i=0;
				}
				break;
			case 0x88:
				if( ZIGBEE_RX_BUFFER[5] == 'N' && ZIGBEE_RX_BUFFER[6] == 'D' )
				{
					char i, j;
					for( i=0; i<10; i++ )
					{
						if( node_list[i][0] == 0 )
						{
							node_list[i][0] = '#';
							for( j=0; j<10; j++ )
								node_list[i][j+1] = ZIGBEE_RX_BUFFER[8+j];
							//strncpy(&node_list[i], &ZIGBEE_RX_BUFFER[8], 10);	
							break;
						}	
					}	
				}	
				break;
			default:
				ZIGBEE_RX_BUFFER[0] = ZIGBEE_RX_BUFFER_POINTER = 0;
		}
		ZIGBEE_RX_BUFFER[0] = ZIGBEE_RX_BUFFER_POINTER = 0;
		//ProcessOutgoingMessage();
		return identifier;
	}
	return 1;	
}

#endif

void ZigbeeDiscoverNodes(void)
{
	char counter = 0, para_num = 0, node_num = 0, c1 = 0, c2 = 0, c3 = 0;
	//unsigned int timeout = 0;

	{
		int i=0;
		for( i=0; i<10; i++ )
			node_list[i][0] = 0;
	}
	
	if( Zigbee_EnterCommandMode() )
	{
		ZigbeeSendStringPGM("ATND");
		
		while( !ZIGBEE_RX_INTERRUPT_FLAG );//&& timeout++ < 84000 );
		
		while( ZIGBEE_RX_INTERRUPT_FLAG == 1 )
		{
			//if( (s[counter] = Zigbee_getcUSART()) == 13 )
			if( (c1 = Zigbee_getcUSART()) == 13 )
			{
				//if( c1 == 20 )
				//	c1 = 20;
				if( c1 == 13 && c2 == 13 && c3 == 13 )  // ATND ended.
					break;
				if( c1 == 13 && c2 == 13 )  // Current node info ended.
				{
					//if( para_num == 2 )
					//	c1 = c1;
					
					if( para_num > 6 )
					{
						counter = para_num = 0;
						node_num++;
						while( !ZIGBEE_RX_INTERRUPT_FLAG );
					}
					
				}	
				else para_num++;
			}
			else
			{
				if( para_num == 0 || para_num == 1 || para_num == 2 )
					node_list[node_num][counter++] = c1;
			}
			
			//if( c3 == 'A' && c2 == '13' && c1 == 20 )
			//	c1 = 0;
			c3 = c2;
			c2 = c1;
	
			#if ( CLOCK_SPEED == 48000000 )
				Delay100TCYx(130);
				//Delay100TCYx(136);  // Somehow this amount of delay needs to be quite precise.
			#else
				#error "No valid processor speed defined."
			#endif	
		}
		//counter=0;
		ZigbeeSendStringPGM("ATCN");
		while( ZIGBEE_RX_INTERRUPT_FLAG ) Zigbee_getcUSART();  // Flush out USART.
	
		//INTCONbits.GIEH = 1;
	}
}

void ZigbeeTasks(void)
{
	//ZigbeeOpen();
	if( ZIGBEE_TASKS != 0 )
	{
		if( ZIGBEE_TASKS && ZIGBEE_TASKS_RUN_ATND )
		{
			INTCONbits.GIEH = 0;
			ZigbeeAPIDiscoverNodes();
			INTCONbits.GIEH = 1;
		
			ZIGBEE_TASKS &= 0b11111110;
		}		
	}
	
	ProcessIncomingMessage();
	//ZigbeeClose();
}	

//***********************************/
//**** Private Functions.************/
//***********************************/
char Zigbee_getcUSART(void)
{
	char c;
	
	if( RCSTAbits.OERR )
	{
		RCSTAbits.CREN = 0;
		c = ZIGBEE_READ;
		RCSTAbits.CREN = 1;
	}
	else c = ZIGBEE_READ;

	return c;
}	

unsigned char Zigbee_EnterCommandMode(void)
{
	unsigned char is_expected_result = -1;
	unsigned int timeout = 0;
	//char result[10];
	
	//unsigned long count=0, count1=0, count2=0;
	char c1, c2;
	
	//INTCONbits.GIEH = 0;
	Delay10KTCYx(200);  //Guard time delay.

	Zigbee_WriteByte('+');
	Zigbee_WriteByte('+');
	Zigbee_WriteByte('+');
		
	#if( CLOCK_SPEED == 48000000 )
		Delay1KTCYx(0);
	#else
		#error "Clock speed not defined."
	#endif


//	#ifdef __18F87J50
//	Delay1KTCYx(0);
//	#elif defined(__18F2455)
//		#error "Not implemented."
//		//for( count=0; count<800000; count++ )
//	#else
//		#error At least one device must be supported.
//	#endif
//		
	while( ZIGBEE_RX_INTERRUPT_FLAG == 0 && timeout++ < 84000);
	
	if( ZIGBEE_RX_INTERRUPT_FLAG == 1 ) c1 = Zigbee_getcUSART();
	Delay100TCYx(0);
	if( ZIGBEE_RX_INTERRUPT_FLAG == 1 ) c2 = Zigbee_getcUSART();
	if( c1=='K' && c2=='O' )
		is_expected_result = 1;

	//Flush the buffer.
	while( ZIGBEE_RX_INTERRUPT_FLAG == 1 )
		c1 = Zigbee_getcUSART();
	
	return (is_expected_result);
}

	
void Zigbee_WriteByte(char c)
{
	#if defined(__18F87J50)
		Write1USART(c);
	#elif defined(__18F2455) || defined(__18F26K20)
		WriteUSART(c);
	#else
		#error "Processor not supported."
	#endif

//	Write1USART(c);
	Delay100TCYx(0);
}

#ifdef APP_USE_ROUTER_CODE
// Zigbee is not in API mode.
int Zigbee_UpdateNetworkInfo(char * serial_number, char * network_address, char * parent_address, char * device_type)
{
	char result_array[12];
	
	if( Zigbee_EnterCommandMode() == 0 )
	{
		//Get and store the serial number of the module.
		if( serial_number != 0 )
		{
			serial_number[0] = 0;
			
			Zigbee_WriteStringPGM("ATSH", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12);
			result_array[6] = 0;
			strcpy(serial_number, result_array);
		
			Zigbee_WriteStringPGM("ATSL", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12);
			strcat(serial_number, result_array);
		
			//Zigbee_WriteStringPGM("ATD7=1", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array);
			//strcat(serial_number, result_array);
			if( serial_number[0] == 0 ) return -1;
		}
		
		//Get network address.
		if( network_address != 0 )
		{
			network_address[0] = 0;
			
			Zigbee_WriteStringPGM("ATMY", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12);
			if( strcmppgm2ram(result_array, "0") == 0 ) strcpypgm2ram(network_address, "0000");
			else strcpy(network_address, result_array);
		
			if( network_address[0] == 0 ) return -1;
		}
		
		//Get parent address.
		if( parent_address != 0 )
		{
			Zigbee_WriteStringPGM("ATMP", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12);
			strcpy(parent_address, result_array);
		}
		
		//Apply changes and close command mode.
		Zigbee_WriteStringPGM("ATCN", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12);
	
	
		if( device_type != 0 )
		{
			#if defined( DEVICE_TYPE_ROUTER )
			strcpypgm2ram(device_type, "1");
			#elif defined( DEVICE_TYPE_END_DEVICE )
			strcpypgm2ram(device_type, "2");
			#else
			strcpypgm2ram(device_type, "-1");
			#endif
			//if( strcmppgm2ram(PARENT_ADDRESS, "FFFE") == 0 ) strcpypgm2ram(DEVICE_TYPE, "1");
			//else strcpypgm2ram(DEVICE_TYPE, "2");
		}
	}
	
	return 1;
}
#endif
/*

void Zigbee_Reset(void)
{
	//Reset only works when Zigbee is enabled by calling Zigbee_Enable().
	//Enabling zigbee will set the pin as output.
	ZB_RESET_TRIS = 0;
	ZB_RESET = 0;
	#if( CLOCK_SPEED == 48000000 )
	{
		Delay10KTCYx(0);
		Delay10KTCYx(0);
		Delay10KTCYx(0);
	}
	#else
		#error "Clock speed not defined."
		//Delay10KTCYx(120);
	#endif
	
	ZB_RESET = 1;
	#if( CLOCK_SPEED == 48000000 )
	{
		Delay10KTCYx(0);
		Delay10KTCYx(0);
	}
	#else
		#error "Clock speed not defined."
	#endif
}

//This function stops the USART RX interrupts and manually polls
//the USART for incoming bytes until the null character is received.
//A timeout is implemented to prevent code from staying in this function forever.
//Function returns the number of chars received.
unsigned int Zigbee_ReceiveBytes(char * buffer, unsigned char buffer_size)
{
	unsigned char counter=0;
	unsigned long timeout=0, TIMEOUT;
	char c;
	
	//if( IS_RUNNING_HIGH_SPEED == 1 ) TIMEOUT = 6240000;
	//else TIMEOUT = 1240000;
	TIMEOUT = 1040000;

	while(counter < buffer_size)
	{
		timeout = 0;
		
		while( (ZIGBEE_RX_INTERRUPT_FLAG==0) && timeout++ < TIMEOUT );
		if( ZIGBEE_RX_INTERRUPT_FLAG==0 )
		{
			//counter = 1;
			break;
			
		}
		else
		{
			c = Zigbee_getcUSART();  //c = ZIGBEE_READ;
			
			if( (c == 13) )
			{
				*buffer = 0;
				//counter = 0;
				break;
			}
			else
			{
				*buffer = c;
				counter++;
			}	
				
			*buffer++;
		}	
//		if( ZIGBEE_RX_INTERRUPT_FLAG==1 )
//		{
//			c = ZIGBEE_READ;
//			
//			if( (c == 13) )
//			{
//				*buffer = 0;
//				//counter = 0;
//				break;
//			}
//			else
//			{
//				*buffer = c;
//				counter++;
//			}	
//				
//			*buffer++;
//		}	

	}
	
	return counter;
}
	
//Anytime the send_broadcast parameter is set to 1, the parameter address is ignored.
//When send_broadcast is 0, destination_address must be a valid parameter.
int Zigbee_SetDestAddressL(char * address, int send_broadcast)
{
	//return -1;
	
	char result_array[12] = "", message[10] = "";
	
	//Sending to FFFF broadcasts the message.
	//Sending to 0000 sends the message to the coordinator.
	if( send_broadcast == 1 )
	{
		strcpypgm2ram(message, "ATDL=0000");
		strcpypgm2ram(CURRENT_DEST_ADDR, "0000");
		
		//if( strcmpram2pgm("FFFF", CURRENT_DEST_ADDR) == 0 ) return 1;
		//else strcpypgm2ram(message, "ATDL=FFFF");
		//strcpypgm2ram(CURRENT_DEST_ADDR, "FFFF");
	}
	else
	{
		if( send_broadcast == 0 )
		{
			strcpypgm2ram(message, "ATDL=FFFF");
			strcpypgm2ram(CURRENT_DEST_ADDR, "FFFF");

			//if( strcmp(address, CURRENT_DEST_ADDR) == 0 ) { return 1; }
			//else sprintf(message, "ATDL=%s", address);
			//strcpy(CURRENT_DEST_ADDR, address);
		}
	}	
	
	INTCONbits.GIEH = 0;
	if( Zigbee_EnterCommandMode() == 0 )
	{
		Zigbee_WriteString(message, ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 10);
		if( strcmppgm2ram(result_array, "OK") != 0 ) 
		{
		}
		
		//Apply changes and close command mode.
		Zigbee_WriteStringPGM("ATCN", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 4);
	}
	INTCONbits.GIEH = 1;
	
	return 1;

}
*/




/*

int Zigbee_CheckConnection(void)
{
//	if( NETWORK_ADDRESS[0] != 0 && strcmpram2pgm("FFFE", &NETWORK_ADDRESS) != 0 )
//	{
//		WARNING_ALARM_OFF;
//		#ifdef ENABLE_LCD
//		strcpy(TEXT_RESET_SCREEN_LINE1, STARTUP_TITLE1);
//		strcpy(TEXT_RESET_SCREEN_LINE2, STARTUP_TITLE2);
//		#endif
//		IS_CONNECTED_TO_NETWORK = 1;
//		return 1;
//	}		
//	
//	WARNING_ALARM_ON;
//	#ifdef ENABLE_LCD
//	strcpypgm2ram(TEXT_RESET_SCREEN_LINE1, "NOT CONNECTED\0");
//	strcpypgm2ram(TEXT_RESET_SCREEN_LINE2, "TO SERVER.\0");
//	#endif
//	IS_CONNECTED_TO_NETWORK = 0;
//	return 0;
}	

void Zigbee_LeavePAN(void)
{
	//char result_array[12];
	if( Zigbee_EnterCommandMode() == 0 )
	{
		//Get and store the serial number of the module.
		Zigbee_WriteStringPGM("ATCB4", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, 0, 0);
		
		//Apply changes and close command mode.
		Zigbee_WriteStringPGM("ATCN", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, 0, 0);
	}
}

void Zigbee_SetSpeed(void)
{
	char result_array[12];
	
	if( Zigbee_EnterCommandMode() == 0 )
	{
		Zigbee_WriteStringPGM("ATBD=7", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12);
		if( strncmppgm2ram(result_array, "OK", 2) == 0 )
		{
			#ifdef ENABLE_LCD
			//LCD_PrintStringPGM("Speed updated.  \0", LCD_ROW0);
			#endif
			Delay10KTCYx(0);	
		}
		if( Zigbee_WriteStringPGM("ATBD", ZB_WRITE_SEND_CR & ZB_WRITE_WAIT_RESPONSE, &result_array, 12) > 0 )
		{
			#ifdef ENABLE_LCD
			//LCD_PrintStringPGM("Speed check...  \0", LCD_ROW0);
			//LCD_PrintString(result_array, LCD_ROW1);
			#endif
			Delay10KTCYx(0);	
		}
	}
}	

void Zigbee_FlushBuffer(void)
{
	while( ZIGBEE_DATARDY ) Zigbee_getcUSART();
}	

unsigned int Zigbee_WriteString(char * msg, unsigned char write_config, char * response, unsigned char buffer_size)
{
	unsigned int chars_received;
	unsigned char config = ~write_config;
	
	ZB_RESET = 1;
	
	if( config & ZB_WRITE_WAIT_RESPONSE_MASK )
		INTCONbits.GIEH = 0;
	
	if( config & ZB_WRITE_INCLUDE_PREDELAY_MASK )
	{
		if( IS_RUNNING_HIGH_SPEED )
		{
			Delay10KTCYx(0);
			Delay10KTCYx(0);
			Delay10KTCYx(0);
			Delay10KTCYx(0);
			Delay10KTCYx(0);
			Delay10KTCYx(0);
		}
		else
		{
			Delay10KTCYx(0);
			Delay10KTCYx(0);
		}	
	}	
		
	while( *msg )
	{
		Zigbee_WriteByte(*msg);
		*msg++;
	}
	if( config & ZB_WRITE_SEND_CR_MASK ) Zigbee_WriteByte(13);
	
	if( config & ZB_WRITE_WAIT_RESPONSE_MASK )
	{
		chars_received = Zigbee_ReceiveBytes(response, buffer_size);
		INTCONbits.GIEH = 1;
	}
	
	return chars_received;
}


//The 2nd parameter of this function takes in a set of configuration that are ANDED together.
// Eg. Zigbee_WriteStringPGM("Send this text.", ZB_WRITE_INCLUDE_PREDELAY & ZB_WRITE_SEND_CR, &result_array);
// 3 configurations available.
// ZB_WRITE_INCLUDE_PREDELAY	- Adds a 1 second delay before starting to send text.
// ZB_WRITE_SEND_CR				- Sends byte 13 after sending the last char in param 1.
// ZB_WRITE_WAIT_RESPONSE		- The function waits a short period of time for response from Zigbee.
//Function returns the number of chars received.
unsigned int Zigbee_WriteStringPGM(far char rom * msg, unsigned char write_config, char * response, unsigned char buffer_size)
{
	unsigned int chars_received;
	unsigned char config = ~write_config;
	
	ZB_RESET = 1;
	
	if( config & ZB_WRITE_WAIT_RESPONSE_MASK )
		INTCONbits.GIEH = 0;
	
	if( config & ZB_WRITE_INCLUDE_PREDELAY_MASK )
	{
		Delay10KTCYx(0);
		Delay10KTCYx(0);
	}	
	
	while( *msg )
	{
		Zigbee_WriteByte(*msg);
		*msg++;
	}

	if( config & ZB_WRITE_SEND_CR_MASK ) Zigbee_WriteByte(13);
	
	if( config & ZB_WRITE_WAIT_RESPONSE_MASK )
	{
		chars_received = Zigbee_ReceiveBytes(response, buffer_size);
		INTCONbits.GIEH = 1;
	}
	
	return chars_received;
}

unsigned int Zigbee_BroadcastStringPGM(ZB_MESSAGE_TYPE command_code, rom char * msg, int chars_to_send)
{
	int i;
	char c;
	char current_paddress[6] = "";
		
	ZB_RESET = 1;
	
	//cr[0]=13; cr[1]=10; cr[2]=0;
	
	if( (command_code == ZB_CMD_SERVER_PING) || (command_code == ZB_CMD_REGNODE) )
	{
		sprintf(pmsg1, "%s#%s#", SERIAL_NUMBER, DEVICE_TYPE);
		//strcpy(pmsg, NETWORK_ADDRESS);
		//strcatpgm2ram(pmsg, "#");
		//strcat(pmsg, DEVICE_TYPE);
		//strcatpgm2ram(pmsg, "#");

		//Compile the list of parent addresses.
		if( PARENT_ADDRESS[0] == 0 ) strcatpgm2ram(pmsg1, "****");
		else
		{
			for( i=0; i<6; i++ )
			{
				//Increment parent timeout counter here.
				if( PARENT_ADDRESS[i*5] == 10 || PARENT_ADDRESS[i*5] == 0 ) continue;					//If slot is empty, continue.
				if( PARENT_ADDRESS[(i*5)+4] < PARENT_TIMEOUT ) PARENT_ADDRESS[(i*5)+4] += 1;
				else
				{
					//If the coordinator is detected to have expired.									//This parent address at this slot has expired.
					if( strncmppgm2ram(&PARENT_ADDRESS[i*5], "0000", 4) == 0 )
					{
						//Parent address "0000" is detected to have expired.
						//Coordinator is offline.
						IS_REGISTERED_TO_SERVER = 0;
//						LCD_PrintStringPGM("CDR OFFLINE.", LCD_ROW0);
//						strcpypgm2ram(TEXT_RESET_SCREEN_LINE1, "CDR OFFLINE");
						//WARNING_ALARM_ON;
					}
					
					PARENT_ADDRESS[i*5] = 10;
					PARENT_ADDRESS[(i*5)+4] = 10;
					
					continue;
				}	
				
				strncpy(current_paddress, &PARENT_ADDRESS[i*5], 5);
				c = current_paddress[4];
				current_paddress[4] = 0;
				if( IsLettersAndDigits(current_paddress) != 1 )
				{
					PARENT_ADDRESS[i*5] = 10;
					PARENT_ADDRESS[(i*5)+4] = 10;
				}
				else
				{
					current_paddress[4] = (char)c;
					current_paddress[5] = 0;
					if( current_paddress[0] != 0 && current_paddress[4] != 0 && current_paddress[4] < PARENT_TIMEOUT )
					{
						if( pmsg1[16+(i*5)] != '#' ) strcatpgm2ram(pmsg1, "&");
						current_paddress[4] = 0;
d:						strcat(pmsg1, current_paddress);
					}
				}		
			}
		}		
		
		return Zigbee_BroadcastString_1(command_code, &pmsg1, strlen(pmsg1), "FFFF");
	}
	else	
	{
		strcpypgm2ram(pmsg1, msg);
		return Zigbee_BroadcastString_1(command_code, &pmsg1, chars_to_send, "FFFF");
	}	
}	

unsigned int Zigbee_BroadcastString_1(char command_code, char * msg, int chars_to_send, rom char * destination_address)
{
	
	char dest_addr[5] = "", broadcast_address[2] = "0";
	unsigned int rvalue = 0;
	
	strcpypgm2ram(dest_addr, destination_address);
	//Zigbee_SetDestAddressL(dest_addr, 0);
	rvalue = Zigbee_BroadcastString_2(command_code, msg, chars_to_send);
	
	return rvalue;
}


//This function is intended for sending messages out to the server.
//This is different from the function Zigbee_WriteString as the messages
//will be wrapped with some header bytes to identify the sender
//of the message.
//Function returns the number of chars sent.
unsigned int Zigbee_BroadcastString_2(char command_code, char * msg, int chars_to_send)
{
	unsigned char i, parity=0;
	int cts = chars_to_send+14;  //14 is added to prevent Profiler.Monitor class from mistaking this packet as 2 individual packets.
	char timestamp[13] = "111111111111";
	
	ZB_RESET = 1;
	
	//Error check.
	if( chars_to_send > 100 )
		return -1;
	
	//Get timestamp.
	#ifdef ENABLE_RTCC
	RTCC_Open();
	if( RTCC_IsDateTimeSet() )
		RTCC_DateTimeToCharArray(timestamp);
	if( timestamp[0] == 0 || strlen(timestamp) < 12 ) strcpypgm2ram(timestamp, "111111111111");
	RTCC_Close();
	#endif
	
	//sprintf(pmsg, "%s%s", timestamp, SERIAL_NUMBER, (int)command_code, cts, chars_to_send);
	strcpy(pmsg2, timestamp);				//First 12 bytes timestamp.
	strcat(pmsg2, NETWORK_ADDRESS);			//Next 4 bytes node ID.
	pmsg2[16] = 0;
	strncat(pmsg2, &command_code, 1);
	pmsg2[17] = 0;
	strncat(pmsg2, &cts, 1);
	pmsg2[18] = 0;
	
	strncat(pmsg2, msg, chars_to_send);
	pmsg2[18+chars_to_send] = 0;

	for( i=0; i<18+chars_to_send; i++ )
	{
		parity += pmsg2[i];
		Zigbee_WriteByte(pmsg2[i]);
	}
	Zigbee_WriteByte((char)parity);					//Transmit parity byte.
	Zigbee_WriteByte(10);
	Zigbee_WriteByte(13);
	
	return i;
}


void Zigbee_SendMessageToDevice(ZB_MESSAGE_TYPE command_code, char * destination_address, char * msg)
{
	//char cr[3], 
	char pmsg[60] = "";
	unsigned char parity = 0;
	unsigned int i=0, cts=strlen(msg)+14;
	
	//cr[0]=13; cr[1]=10; cr[2]=0;
	
	if( command_code != ZB_DISCOVER_DEVICES )
	{
		//Modify the destination device.
		if( destination_address == 0 )
		{
			//Send as dedicated message to the coordinator.
			Zigbee_SetDestAddressL(0, 1);
			sprintf(pmsg, "CMD0000%c%c#%s", (int)command_code, cts, msg);
		}
		else
		{
			//Send as broadcast across the network.
			Zigbee_SetDestAddressL(destination_address, 0);
			sprintf(pmsg, "CMD%s%c%c#%s", destination_address, (int)command_code, cts, msg);
		}
	}	

	//Calculate parity bit for the entire message to be transmitted.
	for( i=0; i<cts-4; i++ )
	{
		parity += pmsg[i];
		Zigbee_WriteByte(pmsg[i]);
	}
	//parity = 189;
	Zigbee_WriteByte((char)parity);					//Transmit parity byte.
	Zigbee_WriteByte(10);
	Zigbee_WriteByte(13);
	
	Delay10KTCYx(20);
}


void ResetDevice(void)
{
	_asm
	reset
	_endasm	
}	

int IsValidCommandCode(char code)
{
	if( (code >= 1 && code <= 8) || (code >= 14 && code <= 16) || (code >= 0x40 && code <= 0x44) ) return 1;
	return -1;	
}
	
//************************************************************
//************************************************************
//Incoming messages are stored in a buffer.
//Call this function periodically to process
//the messages in the buffer.
void Zigbee_ProcessIncomingMessage(void)
{
	char * cmd, * cmd_start = NULL, * current_paddress;
	char cmd_code, address[5], message[30];		//buffer[12];  //message[30];
	unsigned char i, data_size, parity = 0;
	ZB_ERROR_CODE	error_code = 0;
	
	//INTCONbits.GIEH = 0;
	
	//CHECK HARDWARE ERROR.
	if( USART_OVERRUN_ERROR == 1 ) { USART_RX_EN = 0; Delay1KTCYx(10); USART_RX_EN = 1; }		//Check USART error.
	if( strlen(NETWORK_ADDRESS) != 4 || IsLettersAndDigits(NETWORK_ADDRESS) != 1 ) ResetDevice();	//Check variable corrupt error.
	//END CHECK HARDWARE ERROR.
	
	if( LOCK_RX_BUFFER == 0 ) return;
	
	while(1)					//NEVER EVER CALL RETURN IN THIS WHILE LOOP.
	{
		if( ZIGBEE_RX_BUFFER_POINTER==0 ) { error_code = 0xF1; break; }
		if( ZIGBEE_RX_BUFFER[ZIGBEE_RX_BUFFER_POINTER-1] != 13 ) { error_code = 0xF1; break; }
		//ZIGBEE_RX_BUFFER_POINTER = ZIGBEE_RX_BUFFER_SIZE;				//Stop buffer from receiving new data.
		
		#ifdef __DEBUG
		//LCD_PrintStringPGM("Zigbee - I.", LCD_ROW1);
		#endif

		cmd_start = cmd = strstrrampgm(ZIGBEE_RX_BUFFER, "CMD");
		if( cmd == NULL )
		{
			ZIGBEE_RX_BUFFER[0] = ZIGBEE_RX_BUFFER_POINTER = 0;
			error_code = 0xF1;
			break;
		}	
		//if( strchr(cmd, 0)==NULL ) break;			//NULL character must be found.
	
		//Parity check.
		while(*cmd)
		{
			if( (*(cmd+sizeof(char))) == 10 )					//We are at the parity bit.
			{
				if( parity == *(cmd) ) {}
				else error_code = RX_PARITY_ERROR;			//Parity error.
				break;
			}
			parity += *cmd++;	
		}
		if( cmd == ZIGBEE_RX_BUFFER + ZIGBEE_RX_BUFFER_SIZE - 1 ) error_code = RX_PARITY_ERROR;
		if( error_code != 0 ) break;
		
		cmd = cmd_start;
		
		#ifdef __DEBUG
		#ifdef ENABLE_SDCARD
//		if( !IS_SDCARD_INSERTED )
//		{
//			while( !FSInit() );
//			pNewFile = FSfopen(newfile, writeArg);
//			bytesWritten = FSfwrite((void *) cmd_start, 1, strlen(cmd_start), pNewFile);
//			FSfclose(pNewFile);
//		}
		#endif
		#ifdef ENABLE_LCD
		LCD_PrintStringPGM("Zigbee - F.", LCD_ROW1);
		
		strncpy(message, cmd, 15);
		message[15] = 0;
		LCD_PrintString(message, LCD_ROW0);
		//Delay10KTCYx(0);
		#endif
		#endif
		
		//There is a valid command in the buffer. Process this buffer.
		cmd += sizeof(char)*3;					//Skip the three chars "CMD".
		address[0] = *cmd++;					//Get destination address.
		address[1] = *cmd++;
		address[2] = *cmd++;
		address[3] = *cmd++;
		address[4] = 0;
		if( IsLettersAndDigits(address) != 1 ) { error_code = RX_INVALID_DESTINATION_ADDRESS; break; }
		if( (strcmppgm2ram(address, "0000")!=0) && (strcmp(address, NETWORK_ADDRESS)!=0) ) break;			//Message was not intended for this node.
		
		#ifdef __DEBUG
		#ifdef ENABLE_LCD
		LCD_PrintStringPGM("Zigbee - J.", LCD_ROW1);
		
		//strncpy(message, cmd, 15);
		//message[15] = 0;
		//LCD_PrintString(message, LCD_ROW0);
		#endif
		#endif

		cmd_code = *cmd++;						//Get command code.
		if( IsValidCommandCode(cmd_code) != 1 ) { error_code = RX_INVALID_COMMAND_CODE; break; }
		
		#ifdef __DEBUG
		#ifdef ENABLE_LCD
		LCD_PrintStringPGM("Zigbee - K.", LCD_ROW1);
		
		//strncpy(message, cmd, 15);
		//message[15] = 0;
		//LCD_PrintString(message, LCD_ROW0);
		#endif
		#endif

		data_size = (*cmd++) - 14;				//Get the number of chars in the msg.
		if( data_size == 0 || data_size > 20 ) { error_code = RX_INVALID_DATA_SIZE; break; }
		if( cmd[0] != '#' ) { error_code = RX_INVALID_START_MARKER; break; }
		cmd++;
		cmd[data_size] = 0;
		
		#ifdef __DEBUG
		#ifdef ENABLE_LCD
//		LCD_PrintStringPGM("Zigbee - H.", LCD_ROW1);
		//sprintf(message, "*%02u,%02u,%s", data_size, cmd_code, cmd);
		//strncpy(message, cmd, data_size);
		//message[data_size] = 0;
		//LCD_PrintString(message, LCD_ROW0);
		//LCD_PrintString(cmd, LCD_ROW0);
		#endif
		#endif

		strncpy(message, cmd, data_size);
		cmd[data_size] = 13;
		message[data_size] = 0;
		
		#ifdef __DEBUG
		#ifdef ENABLE_LCD
		//if( LCD_LED == 0 )
		LCD_PrintStringPGM("Zigbee - G.", LCD_ROW1);
		//LCD_PrintString(cmd, LCD_ROW0);
		#endif
		#endif
		
		//Check that the msg is intended for itself before processing.
		if( (strcmppgm2ram(address, "0000")==0) || (strcmp(address, NETWORK_ADDRESS)==0) )
		{
			switch( cmd_code )
			{
				case ZB_REQUEST_NODE_JOIN:												//Server is requesting to resend registration info.
					#ifdef __DEBUG
					//LCD_PrintStringPGM("Zigbee - A.", LCD_ROW1);
					#endif
					Zigbee_BroadcastStringPGM(ZB_CMD_REGNODE, "1", 1);
					break;
				case ZB_NODE_REGISTERED:
					#ifdef __DEBUG
//					LCD_PrintStringPGM("Zigbee - B.", LCD_ROW1);
					#endif
					IS_REGISTERED_TO_SERVER = 1;
					break;
				case ZB_CMD_START_HISPEED:
					HI_SPEED_INTERVAL = 30;
					break;
				case ZB_CMD_SETDATETIME:
					#ifdef __DEBUG
//					LCD_PrintStringPGM("Zigbee - C.", LCD_ROW1);
					#endif
					#ifdef ENABLE_RTCC
					RTCC_SetDateTime(message);
					#endif
//					IS_TIME_INITIALISED = 1;
					//if( !IS_USER_IN_MENU )
					{
						//LCD_PrintStringPGM("Command rcvd.   \0", LCD_ROW0);
						//LCD_PrintSringPGM("Set Date/Time", LCD_ROW1);
					}
					break;
				case ZB_CMD_DEVICE_PING:
					#ifdef __DEBUG
//					LCD_PrintStringPGM("Zigbee - D.", LCD_ROW1);
//					LCD_PrintString(message, LCD_ROW0);
					#endif
	
				
					//Validate message received.
					if( IsLettersAndDigits(message) != 1 ) break;
					
					//If ping was received from the server set IS_REGISTERED_TO_SERVER = 1.
					if( IS_REGISTERED_TO_SERVER == 0 && strcmppgm2ram(message, "0000") == 0 && PARENT_ADDRESS[0] != 0 )
						Zigbee_BroadcastStringPGM(ZB_CMD_REGNODE, "1", 1);
					
					current_paddress = strstr(PARENT_ADDRESS, message);
					if( current_paddress == NULL )
					{
						//The address is not in the list. Find a slot and save it.	
						for( i=0; i<6; i++ )
						{
							current_paddress = PARENT_ADDRESS + (i*5);
							
							if( current_paddress[0] == 0 || current_paddress[0] == 10 )  					//If slot is empty.
							{
								strncpy(current_paddress, message, 4);
								current_paddress[4] = 14;						//Reset parent timeout counter.
								PARENT_ADDRESS[31] = 0;
								//current_paddress[5] = 0;
								break;
							}	
						}	
					}
					else
					{
						//The address is found. Reset the counter.
						current_paddress[4] = 14;
					}		
					//if( strcmp( cmd, PARENT_ADDRESS ) == 0 )
					//	PARENT_PING_COUNTER = 0;
					break;
				case ZB_SEND_BEEP:
//					BUZZER = 1;
//					Delay10KTCYx(80);
//					BUZZER = 0;
					break;
				case ZB_DISCOVER_DEVICES:
					#ifdef __DEBUG
					//LCD_PrintStringPGM("Zigbee - E.", LCD_ROW1);
					#endif
					//This command is sent by other devices to find devices around it.
					//Devices which receive this command should respond with a inter-device ping directed for the sending node.
					cmd[4] = 0;
					Zigbee_SendMessageToDevice(ZB_CMD_DEVICE_PING, message, NETWORK_ADDRESS);
					break;
				case ZB_FORCE_RESET:
					ResetDevice();
					break;
				case ZB_NUDGE:
					if( strcmp(address, NETWORK_ADDRESS)==0 ) RECEIVED_SERVER_NUDGE = 1;
					break;
			}
		}
		
		break;		//If all above runs properly, break away from the while loop.
		
		//Move the pointer buffer away.
		//cmd_start[0] = '0';					//Modify "CMD" to "0MD".
		//if( ZIGBEE_RX_BUFFER_POINTER >= ZIGBEE_RX_BUFFER_SIZE ) 
		//ZIGBEE_RX_BUFFER_POINTER = 0;
	}
		
	if( cmd_start != NULL ) cmd_start[0] = '0';
	#ifdef __DEBUG
	if( error_code != 0 )
	{
		sprintf(message, "ZB_ERROR:%#x%c%c", error_code, 13, 10);

		//SD CARD.
		#ifdef ENABLE_SDCARD
		if( !IS_SDCARD_INSERTED )
		{
			while( !FSInit() );
			pNewFile = FSfopenpgm("log.txt", "a");
			bytesWritten = FSfwrite((void *) message, 1, strlen(message)+2, pNewFile);
			FSfclose(pNewFile);
		}
		#endif
		//END SD CARD.

		#ifdef ENABLE_LCD
		//LCD_PrintString(message, LCD_ROW1);
		#endif
	}
	else
		ZIGBEE_RX_BUFFER[0] = ZIGBEE_RX_BUFFER_POINTER = 0;
	#endif
	LOCK_RX_BUFFER = 0;
	//INTCONbits.GIEH = 1;
	ZIGBEE_RX_INTERRUPT_EN = 1;
}	
*/

//#endif


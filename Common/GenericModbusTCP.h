// GenericModbusTCP.h

#include "TCPIP Stack/TCPIP.h"

typedef union
{
	char Serialised[36];
	struct
	{
		MAC_ADDR MACAddr;
		IP_ADDR IPAddr;
		char FunctionCode;
		int StartAddr;
		int RegisterCount;
		char Request_Data[20];
		
		struct
		{
			unsigned char : 7;
			unsigned char IsNewRequest : 1;
		} Flags; 
	} w;	
} MODBUS_REQUEST_BLOCK;


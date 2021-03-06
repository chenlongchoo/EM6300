// ModbusTCP.h

#include "TCPIP Stack/TCPIP.h"

// Eric(2012-09-18) - Clearer abstraction.
typedef struct
{
	BYTE FunctionCode;
	WORD StartAddr;
	WORD RegisterCount;
} MODBUS_REGISTER;

typedef struct
{
	WORD TransactionID;
	WORD ProtocolID;
	WORD Length;
} MODBUS_REQUEST_HEADER;

typedef struct
{
//	DWORD hResult;
//	long lResult;
	BYTE Others[40];	// 32
} MODBUS_REQUEST_DATA;

typedef union
{
	char Serialised[51];
	struct
	{
		MODBUS_REQUEST_HEADER header;
		MODBUS_REGISTER mbRegister;
		MODBUS_REQUEST_DATA data;
	} w;	
} MODBUS_REQUEST_BLOCK;

// First 16 are reserved for error codes defined by Modbus Specs.
typedef enum
{
	MBE_NO_ERROR = 0,
	MBE_ILLEGAL_FUNCTION,			// When the requested function code is not implemented.
	MBE_RESERVED2,
	MBE_RESERVED3,
	MBE_RESERVED4,
	MBE_RESERVED5,
	MBE_RESERVED6,
	MBE_RESERVED7,
	MBE_RESERVED8,
	MBE_RESERVED9,
	MBE_RESERVED10,
	MBE_RESERVED11,
	MBE_RESERVED12,
	MBE_RESERVED13,
	MBE_RESERVED14,
	MBE_RESERVED15,
	MBE_CLIENT_CANNOT_CONNECT,  	// When client cannot make a TCP connection to the MB server.
	MBE_CLIENT_CONNECTION_LOST,		// When the client detects that connection to server is lost.
	MBE_CLIENT_CONNECTION_TIMEOUT,	// 
	MBE_SERVER_NO_FREE_SOCKET,		// When the server is not able to allocate a socket to listen for requests.
	MBE_SERVER_CONNECTION_LOST		// When client expectedly disconnected.
} MODBUS_ERROR_FLAGS;


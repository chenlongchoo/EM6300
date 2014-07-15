// Modbus.h

#include "ModbusTCP.h"
#include "TCPIP Stack/TCPIP.h"



// -- ModBus Prototypes --------------------------------------------------

void Modbus_MeterServerTask(void);

typedef enum
{
	MBSS_HOME = 0,
	MBSS_LISTENING,
	MBSS_PROCESS_REQUEST,
	MBSS_WAIT_RESPONSE,
	MBSS_SEND_RESPONSE,
	MBSS_COMPLETE,
	MBSS_SEND_ERROR_CODE,
	MBSS_DISCONNECT
}
MODBUS_SERVER_STATE;


// -- ModBus Options -----------------------------------------------------
#define MODBUS_PORT				502


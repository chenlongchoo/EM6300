// ModbusTCPClient.h

#include "GenericModbusTCP.h"


#define MODBUS_PORT		502


/** C O N S T A N T S ***********************************************************/
typedef enum
{
	MBRT_HOME = 0
	
} MODBUS_REQUEST_TYPES	;

typedef enum
{
	MBCS_HOME = 0,
	MBCS_SOCKET_OBTAINED,
	MBCS_SEND_REQUEST,
	MBCS_WAIT_RESPONSE,
	MBCS_PROCESS_RESPONSE,
	MBCS_DISCONNECT,
	MBCS_RELEASE
}
MODBUS_CLIENT_STATE;




/** P R O T O T Y P E S ***********************************************************/
//int IsModbusRequestEmpty(void);
int CreateNewModbusRequest(MAC_ADDR mac_addr, char function_code, WORD start_addr, char reg_count,char* request_data);
MODBUS_CLIENT_STATE ModbusRequestTask(void);



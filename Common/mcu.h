//mcu.h

#ifndef MCU_H
#define MCU_H

/**** D E F I N I T I O N S ************************************************/

#if defined(__18F87J50)
	#define MCUClose()		Close1USART()
	#define MCUIsBusy()		Busy1USART()
	#define MCUIsDataRdy()	DataRdy1USART()
	#define MCUReadByte()	Read1USART()
#elif defined(__18F2455) || defined(__18F26K20) || defined( __18F46K20 )
	#define MCUClose()		CloseUSART()
	#define MCUIsBusy()		BusyUSART()
	#define MCUIsDataRdy()	DataRdy1USART()
	#define MCUReadByte()	(BYTE)ReadUSART()
#endif


/**** E N U M E R A T O R S ************************************************/
typedef enum
{
	MMT_MODBUS_READ_REQUEST = 0,
	MMT_MODBUS_READ_RESPONSE,
	MMT_MODBUS_WRITE_REQUEST,
	MMT_MODBUS_WRITE_RESPONSE,
	MMT_READING_REQUEST,
	MMT_READING_RESPONSE,
//	MMT_GET_DATETIME,	//2012-10-02 Liz Removed
	MMT_SET_DATETIME,	//2012-10-02 Liz Removed
	MMT_RESET_ENR_COUNTER,
	MMT_GET_CALIBRATION_DATA,
	MMT_SET_CALIBRATION_DATA_NEW,
	MMT_METER_STATUS_CHANGED,	//2012-10-02 Liz modified
	MMT_CLEAR_STATUS_FLAG,
	MMT_EEP_RECORD_TIME,	// 2014-01-16 Liz added
	MMT_GENERIC_RESPONSE,
	MMT_CT_RATING_REQUEST,
	MMT_GET_ANALOG_SETTING_RESP,
//	MMT_SET_MAX_DEMAND_REFRESH_INTERVAL,	// 2013-12-27 Liz removed. 2012-07-25 Liz: Added Max Demand settings
//	MMT_GET_MAX_DEMAND_REFRESH_INTERVAL,	// 2013-12-27 Liz removed. 2012-07-25 Liz: Added Max Demand settings
	MMT_GET_3PHASE_FOR_EEPROM,
	MMT_BOT_VBATVOLTAGE,				//2014-2-25 CL Added : Update top board the voltage level of Super Cap
	MMT_BOT_UPDATE_TOP_DATETIME,		//2014-2-25 CL Added : Update top board datetime is still running
	MMT_BOT_NORMAL_RESET = 0xF0,	// Do not change the index of MMT_BOT_NORMAL_RESET and 
	MMT_BOT_BOOTLOAD_RESET = 0xF1	// and MMT_NOT_BOOTLOAD_RESET else the bootloader will not work.
} MCU_MESSAGE_TYPE;

/**** S T R U C T U R E S ************************************************/

typedef struct
{
	MCU_MESSAGE_TYPE message_type;
	WORD start_addr;
	WORD ByteCount;
} MCU_REQUEST_HEADER;

typedef struct
{
	long hResult;
	long lResult;
	BYTE Others[32];
} MCU_REQUEST_DATA;

typedef union
{
	char Serialised[36] __PACKED;
	struct
	{
		MCU_REQUEST_HEADER header;
		MCU_REQUEST_DATA data;
		char IsRawValue;
	} w;
} MCU_REQUEST_BLOCK;

#define MCU_PURPOSE_MODBUS		1
#define MCU_PURPOSE_MENU		2
extern BOOL IS_STATUS_FLAG_SENT;	//2012-10-16 Liz added
extern char maxq_mask[3];	//2012-11-09 Liz added

#if !defined( COMMUNICATOR_BOARD )
	/**** P R O T O T Y P E S ************************************************/
	void MCUOpen(void);
	BOOL MCURequestToBOTTOMBoard(char msg_type, char * msg_in, char data_length, BOOL bWait_Response, BOOL bAllow_Retry);
	void MCUProcessIncomingMessage(void);
	void MCUUnloadData(void);
	void MCUTasks(void);
	BOOL MCUSendString(unsigned char data_length, char * s);
#if defined(METER_TOP_BOARD)
	BOOL CheckMCUIsBusy(void);
	void SetMCUIsBusy(BOOL a);
#endif

	
	/**** V A R I A B L E S ************************************************/
	
	extern BYTE MCUNewMessageLength;
	extern BYTE MCUHasNewMessage;
	extern char MCUNewMessage[40];

#endif

#endif

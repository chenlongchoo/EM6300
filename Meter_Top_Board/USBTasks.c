
#include "Main_Transceiver.c"
#include "./USB/usb.h"
#include "./USB/usb_function_cdc.h"
#include "USB\usb_device.h"
#include "usb_config.h"
#include "USB\usb.h"

//	Variables
char Out_Buffer[CDC_DATA_OUT_EP_SIZE] = "M0#2";
unsigned char  NextOut;

/*===================================================
|					Funtions						|
===================================================*/
void USBGetInput(void)
{
	if((USBUSARTIsTxTrfReady()) && (NextOut > 0))
	{
		putUSBUSART(&Out_Buffer[0], NextOut);
		NextOut = 0;
	}
}	
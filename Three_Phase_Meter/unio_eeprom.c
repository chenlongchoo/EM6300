//unio_eeprom.c


#include "unio_eeprom.h"

int Start()
{
	SCIO_TRIS = 0;
	
	//Set high for tandby pulse time of 600us. ~8000 cycles.
	SCIO_LAT = 1;
	Delay1KTCYx(8);
	
	//Set low for start header low pulse time of 10us. ~200 cycles.
	SCIO_LAT = 0;
	Delay10TCYx(40);

	//Bit period of 50us. ~600 cycles.
	SendByte(0x55);  //Send start header.
	return SendByte(0xA0);  //Send device address.
}	

int SendByte(char c)
{
	char mask = 0x80, count = 0, SAK_HIGH;
	
	SCIO_TRIS = 0;
	
	while( count++ < 8 )
	{
		if( c & mask )
		{
			SCIO_LAT = 0;
			Delay10TCYx(30);
			SCIO_LAT = 1;
			Delay10TCYx(30);
		}
		else
		{
			SCIO_LAT = 1;
			Delay10TCYx(30);
			SCIO_LAT = 0;
			Delay10TCYx(30);
		}
		
		mask >>= 1;	
	}	

	//Send MAK.
	SCIO_LAT = 0;
	Delay10TCYx(30);
	SCIO_LAT = 1;
	Delay10TCYx(30);

	//Receive SAK.
	SCIO_TRIS = 1;
	Delay10TCYx(15);
	SAK_HIGH = SCIO;
	Delay10TCYx(30);
	if( SAK_HIGH == 0 && SCIO == 1 )
		return 1;
	else
		return -1;
}

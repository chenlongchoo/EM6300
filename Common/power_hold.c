// power_hold.c

#ifdef APP_USE_ZIGBEE
	#include "zigbee.h"
#endif
#include "power_hold.h"

//#pragma udata memory_3
char RESPONSE_RECEIVED_POINTER = 0;
char RESPONSE_RECEIVED[10][16];
char PENDING_REQUESTS_POINTER = 0;
char PENDING_REQUESTS[4][14];

//#pragma udata

//#pragma udata memory_2
//READING_STORAGE		reading_storage;
//#pragma udata 
//
//READING_STORAGE_1	reading_storage_1;

//rom near char * POWER_READINGS_M[POWER_REGISTERS_QUEUE_SIZE] =
//{
//	&reading_storage_1.LINE_FREQ,  
//	&reading_storage_1.AMPCC, &reading_storage_1.ENRCC,
//	&reading_storage_1.VOLTCC, &reading_storage_1.PWRCC,
//	&reading_storage_1.PHASE_A_STATUS, &reading_storage_1.PHASE_B_STATUS,
//	&reading_storage_1.PHASE_C_STATUS, &reading_storage_1.HARMONIC_CONFIG,
//	&reading_storage.HARMONIC_VOLTAGE, &reading_storage.HARMONIC_CURRENT,
//	0, 0
//};

//rom near char * POWER_READINGS_A[POWER_REGISTERS_QUEUE_SIZE] =
//{	
//	&reading_storage_1.POWER_FACTOR_A,
//	&reading_storage.VOLT_A, &reading_storage.CURRENT_A,
//	&reading_storage_1.RAW_VOLTAGE_A, &reading_storage_1.RAW_CURRENT_A,
//	&reading_storage_1.VOLT_GAIN_A, &reading_storage_1.CURRENT_GAIN_A,
//	&reading_storage.REAL_ENERGY_A, &reading_storage.REAL_POWER_A,
//	&reading_storage.APPARANT_POWER_A, &reading_storage.REACTIVE_POWER_A,
//	&reading_storage.APPARANT_ENERGY_A, &reading_storage.REACTIVE_ENERGY_A
//};	

//rom near char * POWER_READINGS_B[POWER_REGISTERS_QUEUE_SIZE] =
//{
//	&reading_storage_1.POWER_FACTOR_B, 
//	&reading_storage.VOLT_B, &reading_storage.CURRENT_B,
//	&reading_storage_1.RAW_VOLTAGE_B, &reading_storage_1.RAW_CURRENT_B,
//	&reading_storage_1.VOLT_GAIN_B, &reading_storage_1.CURRENT_GAIN_B,
//	&reading_storage.REAL_ENERGY_B, &reading_storage.REAL_POWER_B,
//	&reading_storage.APPARANT_POWER_B, &reading_storage.REACTIVE_POWER_B,
//	&reading_storage.APPARANT_ENERGY_B, &reading_storage.REACTIVE_ENERGY_B
//};
	
//rom near char * POWER_READINGS_C[POWER_REGISTERS_QUEUE_SIZE] =
//{
//	&reading_storage_1.POWER_FACTOR_C, 
//	&reading_storage.VOLT_C, &reading_storage.CURRENT_C,
//	&reading_storage_1.RAW_VOLTAGE_C, &reading_storage_1.RAW_CURRENT_C,
//	&reading_storage_1.VOLT_GAIN_C, &reading_storage_1.CURRENT_GAIN_C,
//	&reading_storage.REAL_ENERGY_C, &reading_storage.REAL_POWER_C,
//	&reading_storage.APPARANT_POWER_C, &reading_storage.REACTIVE_POWER_C,
//	&reading_storage.APPARANT_ENERGY_C, &reading_storage.REACTIVE_ENERGY_C
//};	
//
//rom near char * POWER_READINGS[4] = 
//{
//	&POWER_READINGS_M, &POWER_READINGS_A, 
//	&POWER_READINGS_B, &POWER_READINGS_C
//};

/*
unsigned long Power_Hold_Consumer(void)
{
	char i=0;
	
	for( i=0; i<4; i++ )
	{
		if( PENDING_REQUESTS[i][0] != 0 || PENDING_REQUESTS[i][1] != 0 )
		{
			int pos = 0x0001;
			unsigned int q = PENDING_REQUESTS[i][0];
			q <<= 8;
			q |= PENDING_REQUESTS[i][1];
			while( pos != 0x8000 )
			{
				if( (q & pos) > 0 )
				{
					char k = 0;
					
					{
						int j = 0;
							char s[30] = {0x10, 0x01};
							char data_length = 6 + 16;
							char broadcast[12] = {0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFE, 0, 0};
							for( j=0; j<10; j++ )
								s[2+j] = broadcast[j];
							s[12] = 0;
							s[13] = 0;
							
							// Append message type before the data.
							s[14] = 0x20;
							
							// Append data.
							//for( j=0; j<data_length-15; j++ )
							{
								// 2-byte register.
								s[15] = (pos >> 8);
								s[16] = (pos & 0x00FF);
								// Phase.
								s[17] = PENDING_REQUESTS[i][2];
								// 4-byte value.
								s[18] = 'A';
								s[19] = 'B';
								s[20] = 'C';
								s[21] = 'D';
							}
							s[data_length] = 0;
							
							for( j=0; j<18; j++ )
								RESPONSE_RECEIVED[RESPONSE_RECEIVED_POINTER][j] = s[j];
							RESPONSE_RECEIVED_POINTER++;
							if( RESPONSE_RECEIVED_POINTER > 9 ) RESPONSE_RECEIVED_POINTER = 0;
							
							#ifdef APP_USE_ZIGBEE	
								//ZigbeeOpen();
								ZigbeeAPISendString(data_length, s);
								//ZigbeeClose();
							#endif
					}	
					
					
					q &= ~pos;
					PENDING_REQUESTS[i][0] = q >> 8;
					PENDING_REQUESTS[i][1] = (q & 0x00FF);
					break;
				}	
				pos <<= 1;
			}	
		}		
	}	
}
*/


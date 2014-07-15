//power.h

#ifndef POWER_H
#define POWER_H

/******************************************/
/*********** Prototypes *******************/
/******************************************/
void PowerOpen(void);
void PowerClose(void);
char PowerSendReadCommand(short address, char data_length, long * result_high, long * result_low);
char Power_SendWriteCommand(short address, char data_length, unsigned long data);
char PowerGetReadings(MCU_REQUEST_BLOCK* request);
BYTE * FormThreePhaseReadings(short reg, BYTE * dest); //2013-04-03 Liz added.

char Power_WriteToMAXQ(short address, char data_length, unsigned long data, unsigned char retries);	//2013-05-27 Liz added

#endif
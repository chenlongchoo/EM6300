// adc.c
//#include "adc.h"


#include "adcon.h"

#ifdef APP_USE_ADC

#include "delays.h"

unsigned char ADC_Read(BYTE channel)
{
	int result = 0;
		
	#ifndef __18F87J50
	ANSELbits.ANS0 = 1;
	#endif
	TRISAbits.TRISA0 = 1;
		
	OpenADC(
		ADC_FOSC_64 & ADC_RIGHT_JUST & ADC_20_TAD,
		channel & ADC_INT_OFF & ADC_REF_VDD_VSS,
		ADC_2ANA);
	
	Delay10TCYx(4);
	
	//SetChanADC(ADC_CH0);
	ConvertADC();
	while( BusyADC() );
	
	result = ReadADC();
	CloseADC();
	
	return result;
}	

#endif  // #ifdef APP_USE_ADC



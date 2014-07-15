//power_manual.c

//#include "adc.h"

#include "power_manual.h"

int PowerM_ReadADC(PHASE_CHANNEL ch)
{
	int results;
	
	//Delay100TCYx( 2 );     // Delay for 50TCY
	
	OpenADC( ADC_FOSC_64 & ADC_RIGHT_JUST & ADC_20_TAD,
         ch & ADC_REF_VDD_VSS & ADC_INT_OFF,
         ADC_15ANA );
         
    Delay10TCYx( 4 );     // Delay for 50TCY
    
//    ADCON1bits.ADCAL = 1;  //Calibrate.
//    ConvertADC();
//	while( BusyADC() );
//	ADCON1bits.ADCAL = 0;

    SetChanADC(ch);
	ConvertADC();
	while( BusyADC() );

	results = ReadADC();
	CloseADC();
	ADCON0=0;
	
	return results;
}
   
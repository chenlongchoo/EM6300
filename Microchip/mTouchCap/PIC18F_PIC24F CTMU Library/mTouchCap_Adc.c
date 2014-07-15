
/*****************************************************************************
 * FileName:        	mTouchCap_Adc.c
 * Dependencies:
 * Processor:       	PIC18, PIC24
 * Compiler:       		C18, C30
 * Linker:          	MPLINK,MPLAB LINK30
 * Company:         	Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright © 2009 Microchip Technology Inc.  
 * Microchip licenses this software to you solely for use with Microchip products, according to the terms of the  
 * accompanying click-wrap software license.  Microchip and its licensors retain all right, title and interest in 
 * and to the software.  All rights reserved. This software and any accompanying information is for suggestion only.  
 * It shall not be deemed to modify Microchip’s standard warranty for its products.  It is your responsibility to ensure 
 * that this software meets your requirements.

 * SOFTWARE IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT 
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP OR ITS LICENSORS BE LIABLE 
 * FOR ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO INCIDENTAL, 
 * SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, 
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, OR ANY CLAIMS BY THIRD PARTIES 
 * (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.  
 * The aggregate and cumulative liability of Microchip and its licensors for damages related to the use of the software 
 * will in no event exceed $1000 or the amount you paid Microchip for the software, whichever is greater.

 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS AND 
 * THE TERMS OF THE ACCOMPANYING CLICK-WRAP SOFTWARE LICENSE.
 * 
 *
 * Author               	Date        	Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Naveen. M				14 Apr 2009		Version 0.1 Initial Draft
 * Sasha. M	/ Naveen. M		4 May 2009  	Version 0.2 Updates
 * Sasha. M	/ Naveen. M		11 Nov 2009  	Version 1.0 Release
 * Sasha. M	/ Nithin. 		10 April 2010  	Version 1.20 Release
 * Arpitha P/ Prasad A		08 June 2011	Version 1.31 Release
 *****************************************************************************/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Includes	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#ifdef __PIC24F__
		#include "mTouchCap_PIC24_CTMU_Physical.h"
#else
		#include "mTouchCap_PIC18_CTMU_Physical.h"
#endif

#include "mTouchCap_Adc.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Variables	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Declarations  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Function Prototypes  ~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Function Definitions  ~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/********************************************************************
 * Function			:    void mTouchCapADC_SetChannelADC(WORD chan_param)
 *
 * PreCondition		:    None
 *
 * Input			:    channel_select - This contains the input select parameter to be 
                     	 configured into the ADCHS register as defined below
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:    This function sets the positive and negative inputs for 
                     	 the sample multiplexers A and B.
 *
 * Note				:
 *******************************************************************/
void mTouchCapADC_SetChannelADC(WORD channel_select)
{
    ADC_CHANNEL_SELECT = channel_select;
}



 /*********   	End of Function 	*********************/


/********************************************************************
 * Function			:   WORD mTouchCapADC_ReadADCBuff(BYTE bufIndex)
 *
 * PreCondition		:   None
 *
 * Input			:   bufIndex - This is the ADC buffer number which is to be read. 
 *
 * Output			:   Returns the contents of the ADC Buffer register (ADCBUF0 to ADCBUFF)
 *	
 * Side Effects		:   None
 *
 * Overview			: 	This function reads the ADC Buffer register which contains 
 *                		the conversion value.
 *
 * Note				:	User should provide bufIndex value between ‘0’ to ‘15’ to 
                     	ensure correct read of the ADCBUF0 to ADCBUFF.
 *******************************************************************/

WORD mTouchCapADC_ReadADCBuff(BYTE BufIndex)
{
#ifdef __PIC24F__
	return(*(&ADCBUF+BufIndex));
#else
     return(*(&ADRES+BufIndex));	//PIC18
#endif
}


 /*********   	End of Function 	*********************/

/********************************************************************
 * Function			:   void mTouchCapADC_InitializeInterrupt(WORD config)
 *
 * PreCondition		:   None
 *
 * Input			:   config - ADC interrupt priority and enable/disable information 
 *
 * Output			:   None
 *
 * Side Effects		:   None
 *
 * Overview			:	This function clears the Interrupt Flag (ADIF) bit and then 
 *                  	sets the interrupt priority and enables/disables the interrupt. 
 *
 * Note				:
 *******************************************************************/



void mTouchCapADC_InitializeInterrupt(BYTE Interrupt_State)
{
	
	//************************************
	//*Clearing the Interrupt Flag bit
	//************************************/
    ADC1_Clear_Intr_Status_Bit;

	//************************************
	//*Setting Priority
	//************************************/
	SetPriorityIntADC1(ADC_PRIORITY);
	
 	//************************************
	//*Configure the Interrupt enable/disable bit
	//************************************/
	Set_Adc_Enable_State(Interrupt_State);
	
}
 /*********   	End of Function 	*********************/


/********************************************************************
 * Function		:    BYTE mTouchCapADC_BusyADC(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:   If the value of DONE is ‘0’, then ‘1’ is returned,indicating that the
 *            		ADC is busy in conversion.If the value of DONE is ‘1’, then ‘0’ is 
 *            		returned, indicating that the ADC has completed conversion.
 *
 * Side Effects	:   None
 *
 * Overview		:   This function returns the complement of the ADCON1 <DONE> bit
 *            		status which indicates whether the ADC is busy in conversion.
 *
 * Note			:
 *******************************************************************/

BYTE mTouchCapADC_BusyADC(void)
{
    return !(ADC_Done_Bit);	/* returns the DONE bit status */
}

/*********   	End of Function 	*********************/


/********************************************************************
 * Function		:    void mTouchCapADC_CloseADC(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:   None
 *
 * Overview		:   This function first disables the ADC interrupt and then turns off the ADC
 *            		module.The Interrupt Flag bit (ADIF) is also cleared.
 *
 * Note			:
 *******************************************************************/

void mTouchCapADC_CloseADC(void)
{
	
	//************************************
	//*disable ADC interrupt
	//************************************/
	DisableIntADC1;	
	
	
	//************************************
	//*turn off ADC
	//************************************/
	DisableADC1;	


	//************************************
	//*Clearing the Interrupt Flag bit
	//************************************/
    ADC1_Clear_Intr_Status_Bit;
}

/*********   	End of Function 	*********************/


/********************************************************************
 * Function		:    void mTouchCapADC_OpenADC(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:   None
 *
 * Overview		:   This function first disables the ADC interrupt and then turns off the ADC
 *            		module.The Interrupt Flag bit (ADIF) is also cleared.
 *
 * Note			:
 *******************************************************************/
void mTouchCapADC_OpenADC(void)
{	
	//************************************
	//*digital/analog mode selection on the port bits
	//************************************/

#ifdef __PIC24F__	
		#ifdef ADC_LEGACY
		#ifdef PIC24F_LOW_PIN_DEVICE
		Adc_Port_Configuration(ADC_PORT_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value for Low Pin devices	
		#else
		Adc_Port_Configuration(ADC_PORT_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value
		Adc_Port_Hgh_Configuration(ADC_PORT_CONFIG_DEFAULT_VALUE); // Configure ADC Register with default value
		#endif
		#endif
		
		#ifdef ADC_NEW
		
		#if defined(__PIC24FJ128DA106__) ||  defined(__PIC24FJ128DA206__) || defined(__PIC24FJ256DA106__)\
   		 || defined(__PIC24FJ256DA206__) ||  defined(__PIC24FJ128GB206__) || defined(__PIC24FJ256GB206__)
		Adc_PortB_Configuration(ADC_PORTB_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value
		#elif defined  PIC24F_LOW_PIN_DEVICE
		Adc_PortA_Configuration(ADC_PORTA_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value
		Adc_PortB_Configuration(ADC_PORTB_CONFIG_DEFAULT_VALUE);	
        #else 
		#ifndef USE_MTOUCH_AND_GRAPHICS
		Adc_PortA_Configuration(ADC_PORTA_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value
		Adc_PortC_Configuration(ADC_PORTC_CONFIG_DEFAULT_VALUE);		
		#endif
		Adc_PortB_Configuration(ADC_PORTB_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value	
		Adc_PortE_Configuration(ADC_PORTE_CONFIG_DEFAULT_VALUE);		
		Adc_PortG_Configuration(ADC_PORTG_CONFIG_DEFAULT_VALUE);					
		#endif	
		#endif
#else
		#ifdef ADC_ANCON_LEGACY									//ADCON0 and ADCON1 register settings are used for ADC configuration				

		AD_CON_REG0 = ADC_CONFIG_ADCON0;						//Set the positive and negative Reference voltage
		AD_CON_REG1 = ADC_CONFIG_ADCON1;						//Set the Aquasition time and Convertion_Clock 
		Adc_Port_Configuration1(ADC_PORT_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value
		Adc_Port_Configuration2(ADC_PORT_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value

		#endif

		#ifdef ADC_ANSEL_NEW  									//ADCON1 and ADCON2 register settings are used for ADC configuration				
		AD_CON_REG1 = ADC_CONFIG_ADCON1;						//Set the positive and negative Reference voltage
		AD_CON_REG2 = ADC_CONFIG_ADCON2;						//Set the Aquasition time and Convertion_Clock  
		Adc_Port_Configuration1(ADC_PORT_CONFIG_DEFAULT_VALUE);	// Configure ADC Register with default value

		#endif
	
		#ifdef ADC_ANCON_NEW									//ADCON1 and ADCON2 register settings are used for ADC configuration
		AD_CON_REG1 = ADC_CONFIG_ADCON1;						//Set the positive and negative Reference voltage
		AD_CON_REG2 = ADC_CONFIG_ADCON2;						//Set the Aquasition time and Convertion_Clock  
		Adc_Port_Configuration1(ADC_PORT_CONFIG_DEFAULT_VALUE); // Configure ADC Register with default value
		Adc_Port_Configuration2(ADC_PORT_CONFIG_DEFAULT_VALUE); // Configure ADC Register with default value
		#endif

		#ifdef ADC_ADCON_LEGACY									//ADCON1 and ADCON2 register settings are used for ADC configuration
		AD_CON_REG1 = ADC_CONFIG_ADCON1;						//Set the positive and negative Reference voltage
		AD_CON_REG2 = ADC_CONFIG_ADCON2;						//Set the Aquasition time and Convertion_Clock  
		Adc_Port_Configuration1(ADC_PORT_CONFIG_DEFAULT_VALUE); // Configure ADC Register with default value
		
		#endif
		
#endif
}

/*********   	End of Function 	*********************/



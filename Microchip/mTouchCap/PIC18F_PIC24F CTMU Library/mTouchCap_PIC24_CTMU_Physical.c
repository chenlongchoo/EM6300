
/*****************************************************************************
 * FileName:        	mTouchCap_PIC24_CTMU_Physical.c
 * Dependencies:
 * Processor:       	PIC24
 * Compiler:      		C30
 * Linker:          	MPLAB LINK30
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
 * Author               		Date        	Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Naveen. M				    14 Apr 2009		  Version 0.1 Initial Draft
 * Sasha. M	/ Naveen. M			4 May 2009  	  Version 0.2 Updates
 * Sasha. M	/ Naveen. M			11 Nov 2009  	  Version 1.0 Release
 * Sasha. M	/ Nithin. 			10 April 2010  	  Version 1.20 Release
 * Nithin M						11 Aug 2010		  Implemetation of Low Power Demo 
 * Arpitha P					08 June 2011	  Version 1.31 Release
 *****************************************************************************/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Includes	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "mTouchCap_PIC24_CTMU_Physical.h"

	
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Variables	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
WORD	curRawData	   	[MAX_ADC_CHANNELS];	//Storage for CTMU channel values	 
WORD	tripValue   	[MAX_ADC_CHANNELS];	//Storage for the trip point for each channel	
WORD	hystValue   	[MAX_ADC_CHANNELS];	//Storage for the hysterisis value for each channel
WORD	pressedCount   	[MAX_ADC_CHANNELS];	//Storage for count of pressed value for each channel
WORD	unpressedCount  [MAX_ADC_CHANNELS];	//Storage for count of unpressed value for each channel
WORD	avg_delay 		[MAX_ADC_CHANNELS];//Storage for count for average update for each channel			
WORD	averageData 	[MAX_ADC_CHANNELS];	// running average of CTMU channels	
WORD	smallAvg		[MAX_ADC_CHANNELS];	// current button smallavg
WORD	actualValue		[MAX_ADC_CHANNELS];	// actual raw A/D counts for each channel
WORD	channel_TRIM_value	[MAX_ADC_CHANNELS];	// actual raw A/D counts for each channel
WORD	channel_IRNG_value	[MAX_ADC_CHANNELS];	// actual raw A/D counts for each channel
BYTE	chFilterType	[MAX_ADC_CHANNELS];	// Channel filter type
BYTE    Press_State 	[MAX_ADC_CHANNELS];	// Storage of pressed value
BYTE 	Channel_Pressed_Status[MAX_ADC_CHANNELS];//Status of pressed channel
extern  BYTE			trimbitsReady;		// Variable to see the button is ready for press
SHORT	startupCount;						// variable to 'discard' first N samples
WORD	currentADCValue;					// current button value
WORD	scaledBigValue;						// current button bigval
SHORT	loopCount = CTMU_CHARGE_TIME_COUNT; // Counter for Charge time of CTMU 
#ifdef LOW_POWER_DEMO_ENABLE
	WORD Counter[MAX_ADC_CHANNELS] ;			// Counter to hold maximum ADC channels
	extern 	BYTE trimbitsReady_with_FRC;		//status of the trimbits calulated using the FRC clock
	extern 	BYTE Default_RawData_with_FRC_Flag; //load the Current Raw data to the Average Data array when FRC is selected
#endif
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Function Prototypes  ~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Declarations  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */





/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Function Definitions  ~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


/////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 			Helper Functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//////////////////////////////////////////////////////////////////////////////////////


/********************************************************************
 * Function		:    void mTouchCapPhy_StablizeChannelData(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 This function will ignore the first few samples of data before 
 *				   	 processing the data for determining the press and unpressed data.
 *
 *
 * Note			: 	Stablizes the CTMU channel data for the time defined by User. 
 *				  	Change the count "INITIAL_STARTUP_COUNT" in mTouch_config.h file.
 *******************************************************************/
void mTouchCapPhy_StablizeChannelData(void)	
{
  	WORD Index;
 	startupCount	= INITIAL_STARTUP_COUNT;

	while (startupCount > 0)
	{
		while(!dataReadyCTMU);

		dataReadyCTMU = 0;			//clear flag

		Set_ScanTimer_IF_Bit_State(DISABLE);              //Clear timer 4 SHORT flag
		Set_ScanTimer_IE_Bit_State(DISABLE);              //Disable interrupt
		Set_ScanTimer_ON_Bit_State(DISABLE);              //Stop timer 4


		startupCount--;									  // Decrement 'n' times to establish startup


		for(Index=0; Index<ScanChannelIndex; Index++)	
    	{
			averageData[ScanChannels[Index]] = curRawData[ScanChannels[Index]];	// During start up time, set Average each pass.
		}

		 
		Set_ScanTimer_IF_Bit_State(DISABLE);              //Clear flag
		Set_ScanTimer_IE_Bit_State(ENABLE);              //Enable interrupt
    	Set_ScanTimer_ON_Bit_State(ENABLE);              //Run timer
	}
}


/********************************************************************
 * Function		:   void mTouchCapPhy_CTMUSetup(void)
 *
 * PreCondition	:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview		    : 	This function will setup the CTMU control registers.
 *
 *
 * Note				: Does the necessary CTMU setup. 
 *******************************************************************/

void mTouchCapPhy_CTMUSetup(void)
{


	Set_CTMU_ENABLE_Status(DISABLE);		//make sure CTMU is disabled 
	Idle_Mode_CTMU_Operation(DISABLE);		//CTMU continues to run in idle mode
	Set_CTMU_Time_Generation(DISABLE);		//disable edge delay generation mode of the CTMU
	Block_CTMU_Edge(DISABLE);			    //edges are blocked
	Set_Edge_Sequence(DISABLE);		        //edge sequence not needed
	Disble_CTMU_CurrentDischarge;		    //Do not ground the current source
	Set_CTMU_Trigger_Control(DISABLE);		//Trigger Output is disabled
	Enble_CTMU_Negative_Edge2_Polarity;
	CTMU_Edge2_Source_Select(TRIGGER_SOURCE_EXTERNAL1);
	Enble_CTMU_Negative_Edge1_Polarity;
	CTMU_Edge1_Source_Select(TRIGGER_SOURCE_EXTERNAL1);
	

/***************************************************************************
--------------- Demo of API "mTouchCapAPI_CTMU_SetupCurrentSource" -----------------------		
		Setup the current source using the API "mTouchCapAPI_SetUpCTMU_Default". Pass the current source 
		range and Trim value as parameters.
***************************************************************************/
	mTouchCapAPI_CTMU_SetupCurrentSource(CURRENT_RANGE_100XBASE_CURRENT,0x00);

}

 /*********   	End of Function 	*********************/


/********************************************************************
 * Function			:    void mTouchCapPhy_ChannelSetup(WORD ChannelNum)
 *
 * PreCondition		:    None
 *
 * Input			:    ChannelNum
  							CHANNEL_AN0 ,
							CHANNEL_AN1 ,
							CHANNEL_AN2 ,
							CHANNEL_AN3 ,
							CHANNEL_AN4 ,
							CHANNEL_AN5 ,
							CHANNEL_AN6 ,
							CHANNEL_AN7 ,
							CHANNEL_AN8 ,
							CHANNEL_AN9 ,
							CHANNEL_AN10 ,
							CHANNEL_AN11 ,
							CHANNEL_AN12 ,
							CHANNEL_AN13 ,	
							CHANNEL_AN14 ,	
							CHANNEL_AN15 ,
							CHANNEL_AN16 ,
							CHANNEL_AN17 ,
							CHANNEL_AN18 ,
							CHANNEL_AN19 ,	
							CHANNEL_AN20 ,	
							CHANNEL_AN21 ,	
							CHANNEL_AN22 ,	
							CHANNEL_AN23,
							//Check the availability of channels in the PIC MCU being used
 *
 * Output			:   None
 *
 * Side Effects		:   None
 *
 *
 * Overview			: 	This function will setup the ADC channels that are used by the CTMU
 *				   		Eval Board.
 *
 * Note				:	Does the necessary CTMU port setup.
 *******************************************************************/
void mTouchCapPhy_ChannelSetup(WORD ChannelNum)	 
{
		#ifdef ADC_LEGACY
		#ifdef PIC24F_LOW_PIN_DEVICE
		Adc_Port_Configuration(MAKE_ALL_PINS_DIGITAL)	// Configure all pin as Digital I/O for Low pin devices
		#else 
		Adc_Port_Configuration(MAKE_ALL_PINS_DIGITAL);	// Configure all pin as Digital I/O
		Adc_Port_Hgh_Configuration(MAKE_ALL_PINS_DIGITAL); // Configure all pin as Digital I/O 
        #endif
		#endif
		
		#ifdef ADC_NEW
		
		#if defined(__PIC24FJ128DA106__) ||  defined(__PIC24FJ128DA206__) || defined(__PIC24FJ256DA106__)\
   		 || defined(__PIC24FJ256DA206__) ||  defined(__PIC24FJ128GB206__) || defined(__PIC24FJ256GB206__)
		Adc_PortB_Configuration(MAKE_ALL_PINS_DIGITAL);	// Configure all pin as Digital I/O
		#elif defined  PIC24F_LOW_PIN_DEVICE
		Adc_PortA_Configuration(MAKE_ALL_PINS_DIGITAL);	// Configure all pin as Digital I/O
		Adc_PortB_Configuration(MAKE_ALL_PINS_DIGITAL);	
		#else 
		#ifndef USE_MTOUCH_AND_GRAPHICS
		Adc_PortA_Configuration(MAKE_ALL_PINS_DIGITAL);	// Configure all pin as Digital I/O
		Adc_PortC_Configuration(MAKE_ALL_PINS_DIGITAL);		
		#endif
		Adc_PortB_Configuration(MAKE_ALL_PINS_DIGITAL);	// Configure all pin as Digital I/O	
		Adc_PortE_Configuration(MAKE_ALL_PINS_DIGITAL);			
		Adc_PortG_Configuration(MAKE_ALL_PINS_DIGITAL);	
		#endif
		#endif
		
//Pins are set as output but all pins need not be output. User can change as per application.

		#ifdef ADC_LEGACY		
		
		TRISB = 0x0000;  
		LATB =  0x0000;
		PORTB = 0x0000;

		#if defined(__PIC24F04KA200__) || defined(__PIC24F04KA201__)||  defined(__PIC24F08KA101__)\
		|| defined(__PIC24F08KA102__)  || defined(__PIC24F16KA101__)  || defined(__PIC24F16KA102__)
		TRISA = 0x0000;  
		LATA =  0x0000;
		PORTA = 0x0000;
		
		TRISB = 0x0000;  
		LATB =  0x0000;
		PORTB = 0x0000;
		#endif
		#endif

		#ifdef ADC_NEW	
		#ifdef USE_MTOUCH_AND_GRAPHICS

//		TRISA = TRISA & !(0x00C0); 			
		TRISB = TRISB & SET_PORT_B_PINS_OUTPUT; 	
//		TRISC = TRISC & !(0x0010); 			
		TRISE = TRISE & SET_PORT_E_PINS_OUTPUT; 			
		TRISG = TRISG & SET_PORT_G_PINS_OUTPUT; 	

//		LATA = LATA & !(0x00C0); 			
//		LATB = LATB & !(0xFFFF); 	
		LATBbits.LATB5 = 0;
//		LATC = LATC & !(0x0010); 			
//		LATE = LATE & !(0x0200); 			
		LATEbits.LATE9 = 0;
//		LATG = LATG & !(0x03C0); 	
		LATGbits.LATG8 = 0;
				
//		PORTA = PORTA & !(0x00C0); 			
//		PORTB = PORTB & !(0xFFFF); 	
//		PORTC = PORTC & !(0x0010); 			
//		PORTE = PORTE & !(0x0200); 			
//		PORTG = PORTG & !(0x03C0); 	
		
		#elif defined(__PIC24F16KA301__)   || defined (__PIC24F16KA302__)  || defined(__PIC24F16KA304__)\
		||  defined(__PIC24F32KA301__)   || defined (__PIC24F32KA302__)  || defined(__PIC24F32KA304__)\
		||  defined(__PIC24FV16KA301__)  || defined (__PIC24FV16KA302__) || defined(__PIC24FV16KA304__)\
		||  defined(__PIC24FV32KA301__)  || defined (__PIC24FV32KA302__) || defined(__PIC24FV32KA304__)
		TRISA = TRISA & !(0x000F); 			
		TRISB = TRISB & !(0xF017); 	
		
		LATA = LATA & !(0x000F); 			
		LATB = LATB & !(0xF017); 	
						
		PORTA = PORTA & !(0x000F); 		
		PORTB = PORTB & !(0xF017);
		
		#elif defined(__PIC24FJ128DA106__) ||  defined(__PIC24FJ128DA206__) || defined(__PIC24FJ256DA106__)\
   		   || defined(__PIC24FJ256DA206__) ||  defined(__PIC24FJ128GB206__) || defined(__PIC24FJ256GB206__)
		 			
		TRISB = TRISB & !(0xFFFF); 	
		TRISC = TRISC & !(0x0010); 			
		TRISE = TRISE & !(0x0200); 			
		TRISG = TRISG & !(0x03C0); 	
			
		LATB = LATB & !(0xFFFF); 	
		LATC = LATC & !(0x0010); 			
		LATE = LATE & !(0x0200); 			
		LATG = LATG & !(0x03C0); 	
							
		PORTB = PORTB & !(0xFFFF); 	
		PORTC = PORTC & !(0x0010); 			
		PORTE = PORTE & !(0x0200); 			
		PORTG = PORTG & !(0x03C0); 	
		
		#else
			
		TRISA = TRISA & !(0x00C0); 			
		TRISB = TRISB & !(0xFFFF); 	
		TRISC = TRISC & !(0x0010); 			
		TRISE = TRISE & !(0x0200); 			
		TRISG = TRISG & !(0x03C0); 	

		LATA = LATA & !(0x00C0); 			
		LATB = LATB & !(0xFFFF); 	
		LATC = LATC & !(0x0010); 			
		LATE = LATE & !(0x0200); 			
		LATG = LATG & !(0x03C0); 	
				
		PORTA = PORTA & !(0x00C0); 			
		PORTB = PORTB & !(0xFFFF); 	
		PORTC = PORTC & !(0x0010); 			
		PORTE = PORTE & !(0x0200); 			
		PORTG = PORTG & !(0x03C0); 	
		
		#endif
		#endif
		Nop();    Nop();    Nop();    Nop();    Nop();    Nop();    Nop();    Nop();


    switch (ChannelNum)
	{

		case CHANNEL_AN0:
            TRIS_CHANNEL_AN0 = INPUT;               //make the channel 0 as input based on the Index
			ADC_CHANNEL_AN0(ANALOG);
                      
        break;

		case CHANNEL_AN1:
            TRIS_CHANNEL_AN1 = INPUT;               //make the channel 1 as input based on the Index
			ADC_CHANNEL_AN1(ANALOG);
		break;

		case CHANNEL_AN2:
            TRIS_CHANNEL_AN2 = INPUT;               //make the channel 2 as input based on the Index
			ADC_CHANNEL_AN2(ANALOG);
 		break;

		case CHANNEL_AN3:
            TRIS_CHANNEL_AN3 = INPUT;               //make the channel 3 as input based on the Index
			ADC_CHANNEL_AN3(ANALOG);
		break;

		case CHANNEL_AN4:
            TRIS_CHANNEL_AN4 = INPUT;               //make the channel 4 as input based on the Index
			ADC_CHANNEL_AN4(ANALOG);
		break;

		case CHANNEL_AN5:
            TRIS_CHANNEL_AN5 = INPUT;               //make the channel 5 as input based on the Index
			ADC_CHANNEL_AN5(ANALOG);
		break;

		case CHANNEL_AN6:
            TRIS_CHANNEL_AN6 = INPUT;               //make the channel 6 as input based on the Index
			ADC_CHANNEL_AN6(ANALOG);
		break;

		case CHANNEL_AN7:
            TRIS_CHANNEL_AN7 = INPUT;               //make the channel 7 as input based on the Index
			ADC_CHANNEL_AN7(ANALOG);
		break;

		case CHANNEL_AN8:
            TRIS_CHANNEL_AN8 = INPUT;               //make the channel 8 as input based on the Index
			ADC_CHANNEL_AN8(ANALOG);
		break;
		
		#ifndef PIC24F_LOW_PIN_DEVICE
		case CHANNEL_AN9:
            TRIS_CHANNEL_AN9 = INPUT;               //make the channel 9 as input based on the Index
			ADC_CHANNEL_AN9(ANALOG);
		break;

		case CHANNEL_AN10:
            TRIS_CHANNEL_AN1O = INPUT;               //make the channel 10 as input based on the Index
			ADC_CHANNEL_AN10(ANALOG);
		break;

		case CHANNEL_AN11:
            TRIS_CHANNEL_AN11 = INPUT;               //make the channel 11 as input based on the Index
			ADC_CHANNEL_AN11(ANALOG);
 		break;

		case CHANNEL_AN12:
            TRIS_CHANNEL_AN12 = INPUT;               //make the channel 12 as input based on the Index
			ADC_CHANNEL_AN12(ANALOG);
 		break;

		case CHANNEL_AN13:
            TRIS_CHANNEL_AN13= INPUT;               //make the channel 13 as input based on the Index
			ADC_CHANNEL_AN13(ANALOG);
 		break;

		case CHANNEL_AN14:
            TRIS_CHANNEL_AN14 = INPUT;               //make the channel 14 as input based on the Index
			ADC_CHANNEL_AN14(ANALOG);
 		break;
		
		case CHANNEL_AN15:
            TRIS_CHANNEL_AN15= INPUT;               //make the channel 15 as input based on the Index
			ADC_CHANNEL_AN15(ANALOG);
 		break;
		
#if defined(__PIC24FJ_DAXXX__)
		
		case CHANNEL_AN16:
            TRIS_CHANNEL_AN16 = INPUT;               //make the channel 16 as input based on the Index
			ADC_CHANNEL_AN16(ANALOG);
		break;

		case CHANNEL_AN17:
            TRIS_CHANNEL_AN17 = INPUT;               //make the channel 17 as input based on the Index
			ADC_CHANNEL_AN17(ANALOG);
		break;

		case CHANNEL_AN18:
            TRIS_CHANNEL_AN18 = INPUT;               //make the channel 18 as input based on the Index
			ADC_CHANNEL_AN18(ANALOG);
		break;

		case CHANNEL_AN19:
            TRIS_CHANNEL_AN19 = INPUT;               //make the channel 19 as input based on the Index
			ADC_CHANNEL_AN19(ANALOG);
 		break;

		case CHANNEL_AN20:
            TRIS_CHANNEL_AN20 = INPUT;               //make the channel 20 as input based on the Index
			ADC_CHANNEL_AN20(ANALOG);
 		break;

		case CHANNEL_AN21:
            TRIS_CHANNEL_AN21= INPUT;               //make the channel 21 as input based on the Index
			ADC_CHANNEL_AN21(ANALOG);
 		break;

		case CHANNEL_AN22:
            TRIS_CHANNEL_AN22 = INPUT;               //make the channel 22 as input based on the Index
			ADC_CHANNEL_AN22(ANALOG);
 		break;
		
		case CHANNEL_AN23:
            TRIS_CHANNEL_AN23= INPUT;               //make the channel 23 as input based on the Index
			ADC_CHANNEL_AN23(ANALOG);
 		break;		
		
		
#endif		
#endif
		default:
            TRIS_CHANNEL_AN0 = INPUT;               //make the channel 0 as input based on the Index
			ADC_CHANNEL_AN0(ANALOG);
 		break;

	}

	  /* Connect the selected channel to ADC MUX */
         //load the channel number that has to be read
        mTouchCapADC_SetChannelADC(ChannelNum);

}

 /*********   	End of Function 	*********************/

/********************************************************************
 * Function			:    void mTouchCapPhy_AdcSetup(void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:	This function will setup the ADC module
 *
 * Note				: 	Does the necessary ADC peripheral setup.
 *******************************************************************/
void mTouchCapPhy_AdcSetup(void)
{
	mTouchCapADC_SetChannelADC (0x00);
	mTouchCapADC_OpenADC();	
	
}

 /*********   	End of Function 	*********************/
 
/********************************************************************
 * Function			:    void mTouchCapPhy_SetupCurrentSourceRange(WORD CurrentSourceRange);
 *
 * PreCondition		:    None
 *
 * Input			:    Analog Current Source Range
 * 	 					 CURRENT_RANGE_100XBASE_CURRENT //Current source Range is 100*Base current (55uA) 
 *						 CURRENT_RANGE_10XBASE_CURRENT  //Current source Range is 10*Base current (5.5uA)
 *						 CURRENT_RANGE_BASE_CURRENT     //Current source Range is Base current (0.55uA)
 *						 CURRENT_SRC_DISABLED           //Current source disabled
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 This function will select the required Analog Current Source Range.
 *
  * Note			: 	 Sets up the CTMU current source.
 *******************************************************************/
void mTouchCapPhy_SetupCurrentSourceRange(WORD CurrentSourceRange)
{
	CTMU_Select_Current_Source(CurrentSourceRange);
 }

 /*********   	End of Function 	*********************/

 /********************************************************************
 * Function			:    void	mTouchCapPhy_SetTrimValue( WORD TrimValue);
 * PreCondition		:    None
 *
 * Input			:    TrimValue : CTMU TRIM bits Settings 
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:	 This function will setup the trim values for the selected current source range
 *
 * Note				: 	 Set the proper Trim  value of the current source as passed in the argument.
 *******************************************************************/

void	mTouchCapPhy_SetTrimValue( WORD TrimValue)
{

    CTMU_Trim_Value (TrimValue);
	
}

/********************************************************************
 * Function			:    void	mTouchCapPhy_Discharge_Current (void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	Discharges the current source.
 *
 *
 * Note				:
 *******************************************************************/

void	mTouchCapPhy_Discharge_Current (void)
{
		Enble_CTMU_CurrentDischarge;
		Nop(); Nop();    Nop();    Nop();    Nop();
		Nop(); Nop();    Nop();    Nop();    Nop();
		Nop();	 // Delay for CTMU charge time
		
		Disble_CTMU_CurrentDischarge;
	
}

 /*********   	End of Function 	*********************/

/********************************************************************
 * Function			:    void	mTouchCapPhy_Charge_Current (void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Charges the cusrrent for the time as per the value given 
 *						 to "CTMU_CHARGE_TIME_COUNT" in mTouchCap_Config.h file.
 *
 *
 * Note			:
 *******************************************************************/

void	mTouchCapPhy_Charge_Current (void)
{		
		WORD loopIndex;
		
		ADC1_Clear_Intr_Status_Bit;
		Enable_ADC_Sampling;
		Disbl_CTMUEdge2;
		Enble_CTMUEdge1;

		
		#ifndef CLOCK_SWITCH_DURING_SLEEP_ENABLE
			if (0 != loopCount) 
			{		
				/* When we require to enaable EDGE1 or EDGE2, should we pass as a param*/
				for (loopIndex = 0; loopIndex < loopCount; loopIndex++) 
				Nop();	 // Delay for CTMU charge time  // PIC18 - 4 TCY to execute Nop(); //PIC24 - 2 TCY to execute Nop();
			}
		
		//when the primary clock is switched to FRC after device wakes from sleep, 
		//the loopcount used for charging the CTMU current source has to be changed based on FRC selection
		#else
			//If the Low power is enabled
			if(Clock_Switch_Enable_Flag) //if the system clock is switched
			{
				// The below NOP() is the time required to charge the CTMU channels when the system
				//clock is switched from Primary clock to FRC.
				// The number of Nop() can be varied based on the system clock
			   	Nop();

		
			}
			
			else
			{
				//when the flag is disabled, the system clock is switched from FRC clock 
				//to Primary clock settings. hence restore the loop count value for primary clock
				loopCount = CTMU_CHARGE_TIME_COUNT;
				if (0 != loopCount)
				{		
					/* When we require to enable EDGE1 or EDGE2, should we pass as a param*/
					for (loopIndex = 0; loopIndex < loopCount; loopIndex++) 
					{
						Nop();	 // Delay for CTMU charge time  // PIC18 - 4 TCY to execute Nop(); //PIC24 - 2 TCY to execute Nop();
					}
				}//end of if (0 != loopCount) //n:ref	
			}
		
		#endif

   		
		Disbl_CTMUEdge1;
	
}

/*********************************************************************
 * Function			:    WORD mTouchCapPhy_Read_ADC(void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    Resultant ADC Buffer value
 *
 * Side Effects		:    None
 *
 * Overview			:	Reads the ADC data of CTMU Channel.
 *
 *
 * Note				:
 *******************************************************************/

WORD mTouchCapPhy_Read_ADC(void)
{

	WORD Channel_Data;

	ADC1_Clear_Intr_Status_Bit;
	Disable_ADC_Sampling;	
	while(!ADC1_IF_Bit);             // Wait for the A/D conversion to finish
	
	Channel_Data  = mTouchCapADC_ReadADCBuff(0);// Read the value from the A/D conversion

	Disable_ADC_Sampling;
	ADC1_Clear_Intr_Status_Bit;
	ADC_Done_Bit      = 0; 

	return Channel_Data;
		
	
}

 /*********   	End of Function 	*********************/



/********************************************************************
 * Function			:    void mTouchCapPhy_InitCTMU(void) 
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Does the initialization of all peripherals/ports
 *						 necessary for CTMU operation.
 *
 *
 * Note				:
 *******************************************************************/
void mTouchCapPhy_InitCTMU(void) 
{
	/* Initialize PORTs  */
	mTouchCapApp_PortSetup();
	
 	/* Initialize CTMU  */
	mTouchCapPhy_CTMUSetup();

	/* Initialize ADC  */
	mTouchCapPhy_AdcSetup();


	Set_Adc_Enable_State(ENABLE);

	Enable_ADC_Sampling;
	
	Set_CTMU_ENABLE_Status (ENABLE);

	Enble_CTMU_CurrentDischarge;        // Drain any charge on the circuit
	Nop(); Nop();    Nop();    Nop();    Nop();	
	Disble_CTMU_CurrentDischarge;
	Nop(); Nop();    Nop();    Nop();    Nop();						
}

/********************************************************************
 * Function			:    void mTouchCapPhy_ChargeTimeInit(void)
 *
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	Initializes the charge time. User can change this by 
 *						modifying the value given for "CTMU_CHARGE_TIME_COUNT"
 * 						in Config.h file.
 *
 * Note				:
 *******************************************************************/
void mTouchCapPhy_ChargeTimeInit(void)
{
	loopCount 	= CTMU_CHARGE_TIME_COUNT;	//Loop counter - determines charge time for each 									

}


/********************************************************************
 * Function			:    void mTouchCapPhy_ReadCTMU(WORD ChannelNumber) 
 *
 * PreCondition		:    None
 *
 * Input			:    ChannelNumber
  							CHANNEL_AN0 ,
							CHANNEL_AN1 ,
							CHANNEL_AN2 ,
							CHANNEL_AN3 ,
							CHANNEL_AN4 ,
							CHANNEL_AN5 ,
							CHANNEL_AN6 ,
							CHANNEL_AN7 ,
							CHANNEL_AN8 ,
							CHANNEL_AN9 ,
							CHANNEL_AN10 ,
							CHANNEL_AN11 ,
							CHANNEL_AN12 ,
							CHANNEL_AN13 ,	
							CHANNEL_AN14 ,	
							CHANNEL_AN15 ,
							CHANNEL_AN16 ,
							CHANNEL_AN17 ,
							CHANNEL_AN18 ,
							CHANNEL_AN19 ,	
							CHANNEL_AN20 ,	
							CHANNEL_AN21 ,	
							CHANNEL_AN22 ,	
							CHANNEL_AN23,
							//Check the availability of channels in the PIC MCU being used	
							
							
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Scans the CTMU channel for ADC voltage. It updates the
 * 						"curRawData" and "actualValue" buffers.
 *
 *
 * Note			:
 *******************************************************************/		
void mTouchCapPhy_ReadCTMU(WORD ChannelNumber) 
{



#ifdef DETAILED_CALCULATION
	DWORD total = 0;
	SHORT chrd;
	// Get the raw sensor reading.
	for(chrd=0; chrd< NUM_HF_READS; chrd++)
	{
		
	/***************************************************************************
	--------------- Demo of API "mTouchCapAPI_CTMU_GetChannelReading" -----------------------		
		Get the channdel ADC data using the API "mTouchCapAPI_CTMU_GetChannelReading". 
	***************************************************************************/
		currentADCValue = mTouchCapAPI_CTMU_GetChannelReading(ChannelNumber);
		//End of CTMU read
		total = total + currentADCValue;
	}

	
	#ifdef GROUND_TEST					// Note: A/D conversion not used 
		/* CHANGE_MCHP */				// A/D mux must connect to channel for CTMU to drain charge
		mTouchCapPhy_Discharge_Current ();
	#endif

	/* Error check */
	if (total != 0) 
	{
		currentADCValue = total/NUM_HF_READS;
	}
	else
	{
		currentADCValue = 0;
	}
	
#else /* One shot calculation using Iterative method */
		/***************************************************************************
		--------------- Demo of API "mTouchCapAPI_ScanChannelIterative" -----------------------
		
		Read the channel Rawdata in an iterative fashion for the mentioned sample count using the 
		API "mTouchCapAPI_ScanChannelIterative". 
	***************************************************************************/
	
	#ifndef CLOCK_SWITCH_DURING_SLEEP_ENABLE
		currentADCValue = mTouchCapAPI_ScanChannelIterative(ChannelNumber,NUM_HF_READS);

	#else
		//if the Low power demo is enabled
		if(Clock_Switch_Enable_Flag)	//If the system clock is switched to FRC
		{
			//The sample count is reduced since the system clock is switched from
			//Primary to FRC
			currentADCValue = mTouchCapAPI_ScanChannelIterative(ChannelNumber,NUM_HF_READS_SWITCHED_CLOCK);
		}		
		else
		{
			//if the system clock is the Primary Clock
			currentADCValue = mTouchCapAPI_ScanChannelIterative(ChannelNumber,NUM_HF_READS);
		}	
	

	#endif
	


#endif

	//For debug - channels 6&7 are not enabled. They are clk & data lines for ICSP. use dummy values
	#ifdef DEBUG
	if(ScanChannels[ChannelNumber] == CHANNEL_AN6 || ScanChannels[ChannelNumber]  == CHANNEL_AN7)
	{
		currentADCValue = 0x240;
	}
	// ...
	#endif

#ifdef GUI_USB_INTERFACE 
	scaledBigValue   = currentADCValue  * 64; 	// BIGVAL is current measurement left shifted 4 bits for GUI display
#else
	scaledBigValue   =  currentADCValue;		// Copy the ADC value of the channel
#endif

	curRawData[ChannelNumber] = scaledBigValue;	// curRawData array holds the most recent BIGVAL values
	actualValue[ChannelNumber] = currentADCValue;

	
} //end ReadCTMU()




/******************************************************************************
 * Function			:    void CTMU_Current_trim_config(int channel_no)
 *
 * PreCondition		:    None
 *
 * Input			:    current Channel number that is being processed by ADC
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:    This function will set the trim bits for 2 channel slider
 *						 4 channel slider, Direct key and MAtrix key board
 *
 * Note			    :    None
 *****************************************************************************/
void CTMU_Current_trim_config(int channel_no)
{

	//BYTE ChannelIndex;
	
	//The current range and the trim bits will be changed based on the frequency of the system clock
	#ifndef CLOCK_SWITCH_DURING_SLEEP_ENABLE	
	if(trimbitsReady)
	{
		mTouchCapAPI_CTMU_SetupCurrentSource(channel_IRNG_value[channel_no],channel_TRIM_value[channel_no]);
	}
	#else			
		//if the Low power demo is enabled
		if(Clock_Switch_Enable_Flag)	//If the system clock is switched to FRC
		{
//			//After the clock is switched from Primary to FRC, the current range and trim bits should also be
//			//changed to work with the lower clock.
		
			mTouchCapAPI_CTMU_SetupCurrentSource(CURRENT_RANGE_100XBASE_CURRENT,channel_TRIM_value[channel_no]);

		}
		else
		{
			if(trimbitsReady)
			{
				mTouchCapAPI_CTMU_SetupCurrentSource(channel_IRNG_value[channel_no],channel_TRIM_value[channel_no]);
			}
		} //end of if(Clock_Switch_Enable_Flag)
	#endif //end of CLOCK_SWITCH_DURING_SLEEP_ENABLE
}  

/********************************************************************
 * Function			:    void mTouchCapPhy_AverageData(WORD Index)
 *
 * PreCondition		:    None
 *
 * Input			:    Channel Number 
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			:	This function is used to average the Current Raw ADC value and the 
 *						Average ADC value based on the type of the decode method that is 
 *						selected by the application.
 * Note				:
 *******************************************************************/
 
void mTouchCapPhy_AverageData(WORD Index)
{

     BYTE Filter_type; 
	

	 /* Filtering based on channel specefic filter type */
	Filter_type = chFilterType[Index];

	/* Check the opted filter type of the channel. Copy if within limits. Else, the default type would be "1BY20" method. */
	if ( (Filter_type > FILTER_METHOD_FASTAVERAGE) )
	{
   		 Filter_type = FILTER_METHOD_SLOWAVERAGE; 
	}

	/* Process the average based on filter type selected by user in APi-mTouchCapAPI_SetUpChannel*/
	switch(Filter_type)
	{
	   case FILTER_METHOD_SLOWAVERAGE:
			// 7. Average in the new value (channel based delay)
					// Always Average (all buttons one after the other in a loop)

			//When Low Power Demo is selected
			#ifdef LOW_POWER_DEMO_ENABLE
			//Default_RawData_with_FRC_Flag --> flag set after FRC clock is selected
			// When the System clock is switched from the Primary to the FRC, the averageData[Index] should
			// be updated with the latest raw values which is calculated using the FRC
			//This is required because the curRawData[Index] value calculated using the Primary clock and the
			// FRC clock will be different.
				if(Default_RawData_with_FRC_Flag)
				{

					// Debounce logic
					if(Counter[Index] <DEBOUNCECOUNT_FRC)
					{
						Counter[Index] ++;
						averageData[Index] = curRawData[Index];
					}
					else
					{
						// If the debounce logic is applied for all the keys, then the subsequent raw values 
						//can be considered as stable.
						Counter[Index]  = 0;
						if(Index >= (ScanChannels[ScanChannelIndex-1]))
						{				
							Default_RawData_with_FRC_Flag = 0;
						
						}
					} //end of if(Counter[Index] <4)
				}//end of 	if(Default_RawData_with_FRC_Flag)

				
			#endif	//end of #ifdef LOW_POWER_DEMO_ENABLE
			
			if (curRawData[Index]  > averageData[Index])
			{
				averageData[Index] = curRawData[Index];				// If curRawData is above Average, reset to high average.
			}
	
			if(avg_delay[Index] < NUM_AVG)
			{
				avg_delay[Index]++;						// Counting 0..8 has effect of every 9th count cycling the next button.
			}
			else
			{
				avg_delay[Index] = 0;					// Counting 0..4 will average faster and also can use 0..4*m, m=0,1,2,3..
			}
			
			if(avg_delay[Index] == NUM_AVG)
			{
					smallAvg[Index] = averageData[Index] / NUM_AVG; 	// SMALLAVG is the current average right shifted 4 bits
					// Average in raw value.
					averageData[Index] = averageData[Index] + ((curRawData[Index] / NUM_AVG) - smallAvg[Index]);
			}
				
	  break;


	  case FILTER_METHOD_GATEDAVERAGE:
			//Filter Method CASE: Gated Average

			/* The latest current raw data would be the latest average data */
			//When Low Power Demo is selected
			#ifdef LOW_POWER_DEMO_ENABLE
			//Default_RawData_with_FRC_Flag --> flag set after FRC clock is selected
				if(Default_RawData_with_FRC_Flag)
				{
					//Counter[Index] ++;
					//averageData[Index] = curRawData[Index];
					if(Counter[Index] <DEBOUNCECOUNT_FRC)
					{
						Counter[Index] ++;
						averageData[Index] = curRawData[Index];
					}
					else
					{
						Counter[Index]  = 0;
						if(Index >= (ScanChannels[ScanChannelIndex-1]))
						{				
							Default_RawData_with_FRC_Flag = 0;
						
						}
					} //end of if(Counter[Index] <4)
				}//end of 	if(Default_RawData_with_FRC_Flag)
			#endif	//end of #ifdef LOW_POWER_DEMO_ENABLE
			//  Average in the new value (channel based delay)
			// Always Average (all buttons one after the other in a loop)
	
			if(avg_delay[Index] < NUM_AVG)
			{
				avg_delay[Index]++;						// Counting 0..8 has effect of every 9th count cycling the next button.
			}
			else
			{
				avg_delay[Index] = 0;					// Counting 0..4 will average faster and also can use 0..4*m, m=0,1,2,3..
			}
			if(avg_delay[Index] == NUM_AVG)
			{
				/* Gated average only of no key pressed. */
				/* Stop averaging when press is being sensed. //STD:*/
				if ( KEY_NOT_PRESSED == mTouchCapAPI_getChannelTouchStatus(Index,DECODE_METHOD_PRESS_ASSERT ))
				{
					smallAvg[Index] = averageData[Index] / NUM_AVG; 	// SMALLAVG is the current average right shifted 4 bits
					// Average in raw value.
					averageData[Index] = averageData[Index] + ((curRawData[Index] / NUM_AVG) - smallAvg[Index]);
				}
			}
     break;

	case FILTER_METHOD_FASTAVERAGE:
			/* The latest current raw data would be the latest average data */
					//When Low Power Demo is selected
			#ifdef LOW_POWER_DEMO_ENABLE
			//Default_RawData_with_FRC_Flag --> flag set after FRC clock is selected
				if(Default_RawData_with_FRC_Flag)
				{
					//Counter[Index] ++;
					//averageData[Index] = curRawData[Index];
					if(Counter[Index] <DEBOUNCECOUNT_FRC)
					{
						Counter[Index] ++;
						averageData[Index] = curRawData[Index];
					}
					else
					{
						Counter[Index]  = 0;
						if(Index >= (ScanChannels[ScanChannelIndex-1]))
						{				
							Default_RawData_with_FRC_Flag = 0;
						
						}
					} //end of if(Counter[Index] <4)
				}//end of 	if(Default_RawData_with_FRC_Flag)
			#endif	//end of #ifdef LOW_POWER_DEMO_ENABLE
		if (curRawData[Index]  > averageData[Index])
		{
			averageData[Index] = curRawData[Index];				// If curRawData is above Average, reset to high average.
		}

	break;

  }

	/* Dynamic trip value calculation . STD: */
	DynamicTripValueCalculation(Index);

}



/********************************************************************
 * Function		:    void DynamicTripValueCalculation(WORD Index)
 *
 * PreCondition	:    None
 *
 * Input		:    Index- Channel Number
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 calculate the dynamic trip value.
 *
 *
 * Note			: 
 *******************************************************************/
void DynamicTripValueCalculation(WORD Index)
{
    #ifndef USE_STATIC_TRIP_VALUE

    	#ifndef LOW_POWER_DEMO_ENABLE
			tripValue[Index] = (averageData[Index] / KEYTRIPDIV);
		 hystValue[Index] = (tripValue[Index] / HYSTERESIS_VALUE);
	
		#else
			//If the low power demo is selected, then the keytrip value will be different 
			//because of the switching of the clocks	
			if(Clock_Switch_Enable_Flag)
			{
				//if the clock is switched from Primary to FRC, then change the trip value to suit 
				//the lower clock frequency
				tripValue[Index] = (averageData[Index] / KEYTRIPDIV_FRC);
				 hystValue[Index] = (tripValue[Index] / HYSTERESIS_VALUE_FRC);
			}
			else
			{
				// If the Primary clock is selected, restore the Trip value.
   				tripValue[Index] = (averageData[Index] / KEYTRIPDIV);
				hystValue[Index] = (tripValue[Index] / HYSTERESIS_VALUE);

			}
				
		#endif
   
    
    #endif
}



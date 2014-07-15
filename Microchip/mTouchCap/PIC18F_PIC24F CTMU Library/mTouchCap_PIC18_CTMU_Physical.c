
/*****************************************************************************
 * FileName:        mTouchCap_PIC18_CTMU_Physical.c
 * Dependencies:
 * Processor:       PIC18
 * Compiler:       	C18
 * Linker:          MPLINK
 * Company:         Microchip Technology Incorporated
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
 * Author               		 Date        		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Naveen. M				     14 Apr 2009		Version 0.1 Initial Draft
 * Nithin Kumar. M			     14 May 2009		Version 0.2 Updates
 * Sasha. M	/ Naveen. M	      	 4 June 2009  		Version 0.3 Updates
 * Sasha. M	/ Naveen. M			 11 Nov 2009  		Version 1.0 Release
 * Sasha. M	/ Nithin. 			 10 April 2010  	Version 1.20 Release
 * Prasad A						 08 June 2011		Version 1.31 Release
 *****************************************************************************/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Includes	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "mTouchCap_PIC18_CTMU_Physical.h"


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Variables	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#pragma udata udata_mTouchCap_PIC18_CTMU_Physical_section1

WORD	curRawData[MAX_ADC_CHANNELS];	//Storage for CTMU channel values
WORD	tripValue[MAX_ADC_CHANNELS];	//Storage for the trip point for each channel	
WORD	hystValue[MAX_ADC_CHANNELS];	//Storage for the hysterisis value for each channel
WORD	pressedCount[MAX_ADC_CHANNELS];	//Storage for count of pressed value for each channel
WORD	unpressedCount[MAX_ADC_CHANNELS];//Storage for count of unpressed value for each channel
WORD	avg_delay[MAX_ADC_CHANNELS];	//Storage for count for average update for each channel		
WORD	averageData[MAX_ADC_CHANNELS];	// running average of CTMU channels	

#pragma udata udata_mTouchCap_PIC18_CTMU_Physical_section2
WORD	smallAvg[MAX_ADC_CHANNELS];		// current button smallavg
WORD	actualValue[MAX_ADC_CHANNELS];	// actual raw A/D counts for each channel
WORD	channel_TRIM_value[MAX_ADC_CHANNELS];// actual raw A/D counts for each channel
WORD	channel_IRNG_value[MAX_ADC_CHANNELS];// actual raw A/D counts for each channel
BYTE	chFilterType[MAX_ADC_CHANNELS];	// Channel filter type
BYTE    Press_State[MAX_ADC_CHANNELS]; // Storage of pressed value
BYTE 	Channel_Pressed_Status[MAX_ADC_CHANNELS];
#pragma udata

WORD		currentADCValue;			// current button value
WORD		scaledBigValue;				// current button bigval
SHORT		startupCount;				// variable to 'discard' first N samples
SHORT		loopCount;

extern CHAR	trimbitsReady;				// Variable to see the button is ready for press
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Function Prototypes  ~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Declarations  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */





/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Function Definitions  ~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


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
 * Note			: 	 Stablizes the CTMU channel data for the time defined by User. 
 *				     Change the count "INITIAL_STARTUP_COUNT" in mTouch_config.h file.
 *					
 *******************************************************************/
void mTouchCapPhy_StablizeChannelData(void)	 
{
  	BYTE Index;
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

/*********   	End of Function 	*********************/


/********************************************************************
 * Function		    :    void mTouchCapPhy_CTMUSetup(void)
 *
 * PreCondition	    :    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview		    : 	 This function will setup the CTMU control registers.
 *
 *
 * Note				: 	 Does the necessary CTMU setup. 
 *******************************************************************/

void mTouchCapPhy_CTMUSetup(void)
{


	Set_CTMU_ENABLE_Status(DISABLE);		//make sure CTMU is disabled 
	Idle_Mode_CTMU_Operation(DISABLE);		//CTMU continues to run in idle mode
	Set_CTMU_Time_Generation(DISABLE);		//disable edge delay generation mode of the CTMU
	Block_CTMU_Edge(DISABLE);				//edges are blocked
	Set_Edge_Sequence(DISABLE);				//edge sequence not needed
	Disable_CTMU_CurrentDischarge;	 		//Do not ground the current source
	Set_CTMU_Trigger_Control(DISABLE);		//Trigger Output is disabled
	Enable_CTMU_Negative_Edge2_Polarity;
	CTMU_Edge2_Source_Select(TRIGGER_SOURCE_EXTERNAL1);
	Enable_CTMU_Negative_Edge1_Polarity;
	CTMU_Edge1_Source_Select(TRIGGER_SOURCE_EXTERNAL1);
	

/***************************************************************************
--------------- Demo of API "mTouchCapAPI_CTMU_SetupCurrentSource" -----------------------		
		Setup the current source using the API "mTouchCapAPI_SetUpCTMU_Default". Pass the current source 
		range and Trim value as parameters.
***************************************************************************/
 
	mTouchCapAPI_CTMU_SetupCurrentSource(CURRENT_RANGE_BASE_CURRENT,0x00);

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
							CHANNEL_AN10,
							CHANNEL_AN11,
							CHANNEL_AN12,
							CHANNEL_AN13,
							CHANNEL_AN14,
							CHANNEL_AN15,
							CHANNEL_AN16,
							CHANNEL_AN17,
							CHANNEL_AN18,
							CHANNEL_AN19,
							CHANNEL_AN20,
							CHANNEL_AN21,
							CHANNEL_AN22,
							CHANNEL_AN23,
							CHANNEL_AN24,
							CHANNEL_AN25,
							CHANNEL_AN26,
							CHANNEL_AN27,
							
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 This function will setup the ADC channels that are used by the CTMU
 *				   		 Eval Board.
 *
 *
 * Note			:	Does the necessary CTMU port setup.
 *******************************************************************/


void mTouchCapPhy_ChannelSetup(WORD ChannelNum)
{
	#ifndef PIC18F_LOW_PIN_DEVICE
      
		#if defined ADC_ANCON_LEGACY || defined ADC_ANSEL_NEW ||  defined ADC_ANCON_NEW 
			//The controllers which has common type of Analog channel selection are grouped 																					
	        // Make sure that only selected channel is made input while the other port pins remain configured as output.
	        // configure the port pins of the respective channels as input and as Analog
	        // Update the current channel number which has to be processed by the ADC
	        // configurations for the Channel 0 as default
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
	
				#if defined(ADC_ANSEL_NEW) || defined(ADC_ANCON_NEW)	//Configure remaining channels as also Analog for controllers defined under this MACROs
		
					case CHANNEL_AN13:
			            TRIS_CHANNEL_AN13 = INPUT;               //make the channel 13 as input based on the Index
						ADC_CHANNEL_AN13(ANALOG);
			 		break;
			
					case CHANNEL_AN14:
			            TRIS_CHANNEL_AN14 = INPUT;               //make the channel 14 as input based on the Index
						ADC_CHANNEL_AN14(ANALOG);
					break;
			
					case CHANNEL_AN15:
			            TRIS_CHANNEL_AN15 = INPUT;               //make the channel 15 as input based on the Index
						ADC_CHANNEL_AN15(ANALOG);
					break;
			
					#ifndef _18F6XK22							//Configure 15 channels as Analog and skip all other. Since this series controllers have 15 Analog channel				
		
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
				            TRIS_CHANNEL_AN21 = INPUT;               //make the channel 21 as input based on the Index
							ADC_CHANNEL_AN21(ANALOG);
						break;
				
						case CHANNEL_AN22:
				            TRIS_CHANNEL_AN22 = INPUT;               //make the channel 22 as input based on the Index
							ADC_CHANNEL_AN22(ANALOG);
						break;
				
						case CHANNEL_AN23:
				            TRIS_CHANNEL_AN23 = INPUT;               //make the channel 23 as input based on the Index
							ADC_CHANNEL_AN23(ANALOG);
						break;
					
					#endif
				#endif
					#ifdef ADC_ANSEL_NEW								//Configure remaining channels as also Analog for controllers defined under this MACROs
	
						case CHANNEL_AN24:
				            TRIS_CHANNEL_AN24 = INPUT;               //make the channel 24 as input based on the Index
							ADC_CHANNEL_AN24(ANALOG);
						break;
				
						#ifndef _18XF2XK22								//Configure 15 channels as Analog and skip all other. Since this series controllers have 24 Analog channel
							case CHANNEL_AN25:
					            TRIS_CHANNEL_AN25 = INPUT;               //make the channel 25 as input based on the Index
								ADC_CHANNEL_AN25(ANALOG);
							break;
					
							case CHANNEL_AN26:
					            TRIS_CHANNEL_AN26 = INPUT;               //make the channel 26 as input based on the Index
								ADC_CHANNEL_AN26(ANALOG);
							break;
					
							case CHANNEL_AN27:
					            TRIS_CHANNEL_AN27 = INPUT;               //make the channel 27 as input based on the Index
								ADC_CHANNEL_AN27(ANALOG);
							break;
						#endif
	
					#endif
			//	#endif
			
				default:
		            TRIS_CHANNEL_AN0 = INPUT;               //make the channel 0 as input based on the Index
					ADC_CHANNEL_AN0(ANALOG);
		 		break;
	
			} 
				/* Connect the selected channel to ADC MUX */
	         	//load the channel number that has to be read
	       		 mTouchCapADC_SetChannelADC(ChannelNum);
					 
	
		#elif defined ADC_ADCON_LEGACY					//To configure port pin has Analog, combination of register value should be considered in the controllers defined
															//under these. 
			switch (ChannelNum)
			{
		
				case CHANNEL_AN0:
			          TRIS_CHANNEL_AN0 = INPUT;               //make the channel 0 as input based on the Index
					  ADC_COMBINE_CHANNEL(ADC_AN0);
			                      
			    break;
			
				case CHANNEL_AN1:
			    	TRIS_CHANNEL_AN1 = INPUT;               //make the channel 1 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN1);
				break;
			
				case CHANNEL_AN2:
			    	TRIS_CHANNEL_AN2 = INPUT;               //make the channel 2 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN2);
			 	break;
			
				case CHANNEL_AN3:
			        TRIS_CHANNEL_AN3 = INPUT;               //make the channel 3 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN3);
				break;
			
				case CHANNEL_AN4:
			        TRIS_CHANNEL_AN4 = INPUT;               //make the channel 4 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN4);
				break;
			
				case CHANNEL_AN5:
			        TRIS_CHANNEL_AN5 = INPUT;               //make the channel 5 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN5);
				break;
			
				case CHANNEL_AN6:
			        TRIS_CHANNEL_AN6 = INPUT;               //make the channel 6 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN6);
				break;
			
				case CHANNEL_AN7:
			        TRIS_CHANNEL_AN7 = INPUT;               //make the channel 7 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN7);
				break;
			
				case CHANNEL_AN8:
			        TRIS_CHANNEL_AN8 = INPUT;               //make the channel 8 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN8);
				break;
			
				case CHANNEL_AN9:
			        TRIS_CHANNEL_AN9 = INPUT;               //make the channel 9 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN9);
				break;
			
				case CHANNEL_AN10:
			        TRIS_CHANNEL_AN1O = INPUT;               //make the channel 10 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN10);
				break;
			
				case CHANNEL_AN11:
			        TRIS_CHANNEL_AN11 = INPUT;               //make the channel 11 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN11);
			 	break;	
			
				default:
			        TRIS_CHANNEL_AN0 = INPUT;               //make the channel 0 as input based on the Index
					ADC_COMBINE_CHANNEL(ADC_AN0);
			 	break;
		
			}
		 		 /* Connect the selected channel to ADC MUX */
	         	//load the channel number that has to be read
	       		 mTouchCapADC_SetChannelADC(ChannelNum);
	
		#endif
	#endif
}



 /*********   	End of Function 	*********************/

 

/********************************************************************
 * Function		:    void mTouchCapPhy_AdcSetup(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		:	 This function will setup the ADC module
 *
 *
 * Note			:    Does the necessary ADC peripheral setup.
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
 * Input			:    
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
 *
 *
 * Note				: 	 Sets up the CTMU current source.
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
 *
 * Note				: 	 Set the proper Trim  value of the current source as passed in the argument.
 *******************************************************************/

void	mTouchCapPhy_SetTrimValue( WORD TrimValue)
{

    CTMU_Trim_Value (TrimValue);

	
}



/********************************************************************
 * Function			:    void	mTouchCapPhy_Discharge_Current (void)
 * PreCondition		:    None
 *
 * Input			:    None
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Discharges the current source.
 *
 *
 * Note				:
 *******************************************************************/

void	mTouchCapPhy_Discharge_Current (void)
{
		Enable_CTMU_CurrentDischarge;
		Nop(); Nop();    Nop();    Nop();    Nop();
		Nop(); Nop();    Nop();    Nop();    Nop();
		Nop();	 // Delay for CTMU charge time
		Disable_CTMU_CurrentDischarge;  
	
}

/*********   	End of Function 	*********************/



/********************************************************************
 * Function		:    void	mTouchCapPhy_Charge_Current (void)
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 Charges the cusrrent for the time as per the value given to 
 *					"CTMU_CHARGE_TIME_COUNT" in mTouchCap_Config.h file.
 *
 *
 * Note			:
 *******************************************************************/

void	mTouchCapPhy_Charge_Current (void)
{		
		WORD loopIndex;
 
#ifdef CTMU_GND_ON
       Enable_CTMU_CurrentDischarge; //CTMUCONHbits.IDISSEN = 1; Drain any charge on the circuit
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();                      // Delay for CTMU charge time
       Disable_CTMU_CurrentDischarge;// CTMUCONHbits.IDISSEN = 0;
 #endif
		
		ADC1_Clear_Intr_Status_Bit;
		 
		Disbl_CTMUEdge2;
		Enable_CTMUEdge1;

	
		/* When we require to enaable EDGE1 or EDGE2, should we pass as a param*/
		for (loopIndex = 0; loopIndex < loopCount; loopIndex++)
		{ 
		 	Nop();	 // Delay for CTMU charge time  // PIC18 - 4 TCY to execute Nop(); //PIC24 - 2 TCY to execute Nop();		
		}     

   		Disbl_CTMUEdge1;

}




/*********************************************************************
 * Function		:    WORD mTouchCapPhy_Read_ADC(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    Resultant ADC Buffer value
 *
 * Side Effects	:    None
 *
 * Overview		:	Reads the ADC data of CTMU Channel.
 *
 *
 * Note			:
 *******************************************************************/

WORD mTouchCapPhy_Read_ADC(void)
{

	WORD Channel_Data;
	 
		ADC1_Clear_Intr_Status_Bit;
        Enable_ADC_Sampling;    //ADCON0bits.GO = 1;          // Manually start the conversion
        // Wait for the A/D conversion to finish
         
		while(ADC_GO_Bit );
      
       Channel_Data  = mTouchCapADC_ReadADCBuff(0);			   // Read the value from the A/D conversion
 
       ADC1_Clear_Intr_Status_Bit;                            // Disable the interrupt flag

/*
	Disable_ADC_Sampling;
	ADC1_Clear_Intr_Status_Bit;
	ADC_Done_Bit      = 0; */
	
	 
         #ifdef GROUND_TEST                              

        // A/D mux must connect to channel for CTMU to drain charge
         
        Enable_CTMU_CurrentDischarge;//CTMUCONHbits.IDISSEN = 1; Drain any charge on the circuit
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
         
        Disable_CTMU_CurrentDischarge;//CTMUCONHbits.IDISSEN = 0;
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
        Nop();
                
        #endif
	 return Channel_Data;
		
	
}


 /*********   	End of Function 	*********************/


/********************************************************************
 * Function			:   void mTouchCapPhy_InitCTMU(void) 
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
						 necessary for CTMU operation.
 *
 *
 * Note				:
 *******************************************************************/
/*....................................................................
. Init()
.
.		Initialization Routine to set up part's variables and
.	peripherals.
....................................................................*/
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
	
	Set_CTMU_ENABLE_Status(ENABLE);

	Enable_CTMU_CurrentDischarge;        // Drain any charge on the circuit
	Nop(); Nop();    Nop();    Nop();    Nop();	
	 
	Disable_CTMU_CurrentDischarge;
	Nop(); Nop();    Nop();    Nop();    Nop();	
					// Initialize Capacitive Sensing	
//	InitAvgDelay();				// Zero out the slow averaging function

}



/********************************************************************
 * Function		:   void mTouchCapPhy_ChargeTimeInit(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 Initializes the charge time. User can change this by 
 *				 	 modifying the value given for "CTMU_CHARGE_TIME_COUNT"
 * 					 in Config.h file.
 *
 *
 * Note			:
 *******************************************************************/
void mTouchCapPhy_ChargeTimeInit(void)
{
	loopCount = CTMU_CHARGE_TIME_COUNT;		//Loop counter - determines charge time for each 
}



/********************************************************************
 * Function			:    void mTouchCapPhy_ReadCTMU(WORD ChannelNumber) 
 *
 * PreCondition		:    None
 *
 * Input			:    Channel Number to be scanned. 				
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
 *
 * Output			:    None
 *
 * Side Effects		:    None
 *
 * Overview			: 	 Scans the CTMU channel for ADC voltage. 
 *						 It updates the "curRawData" and "actualValue" buffers.
 *
 *
 * Note				:
 *******************************************************************/		
void mTouchCapPhy_ReadCTMU(WORD ChannelNumber) 
{

	/***************************************************************************
	--------------- Demo of API "mTouchCapAPI_ScanChannelIterative" -----------------------
		
	Read the channel Rawdata in an iterative fashion for the mentioned sample count using the 
	API "mTouchCapAPI_ScanChannelIterative". 
	***************************************************************************/
	currentADCValue = mTouchCapAPI_ScanChannelIterative(ChannelNumber,NUM_HF_READS);


	//For debug - channels 6&7 are not enabled. They are clk & data lines for ICSP. use dummy values
	#ifdef DEBUG
	if(ScanChannels[ChannelNumber] == CHANNEL_AN6 || ScanChannels[ChannelNumber]  == CHANNEL_AN7)
	{
		currentADCValue = 0x240;
	}
	// ...
	#endif
	
#ifdef GUI_USB_INTERFACE 
	scaledBigValue   = currentADCValue  * 64; 	     // BIGVAL is current measurement left shifted 4 bits for GUI display
#else
	scaledBigValue   =  currentADCValue;			// Copy the ADC value of the channel
#endif

	curRawData[ChannelNumber] = scaledBigValue;		// curRawData array holds the most recent BIGVAL values
	actualValue[ChannelNumber] = currentADCValue;
	
} //end ReadCTMU()


/******************************************************************************
 * Function:       void CTMU_Current_trim_config(int channel_no)
 *
 * PreCondition:    None
 *
 * Input:           current Channel number that is being processed by ADC
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       	This function necessary to maintain a constant default voltage levels in all the channels.
 *
 * Note:            None
 *****************************************************************************/
void CTMU_Current_trim_config(int channel_no)
{

	if(trimbitsReady)
	{
		mTouchCapAPI_CTMU_SetupCurrentSource(channel_IRNG_value[channel_no],channel_TRIM_value[channel_no]);
	}

}  //end of void CTMU_Current_trim_config(int channel_no)
 


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
 *
 *
 *
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
		// 7. Average in the new value (channel based delay)
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
 * Overview		: calculate the dynamic trip value.
 *
 *
 * Note			: 
 *******************************************************************/
void DynamicTripValueCalculation(WORD Index)
{
#ifndef USE_STATIC_TRIP_VALUE 

   		tripValue[Index] = (averageData[Index] / KEYTRIPDIV);
   		hystValue[Index] = (tripValue[Index] / HYSTERESIS_VALUE);
#endif
}

/*****************************************************************************
 * FileName:        	mTouchCap_CtmuAPI.c
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
 * Naveen. M				    14 Apr 2009			Version 0.1 Initial Draft
 * Sasha. M	/ Naveen. M			4 May 2009  		Version 0.2 Updates
 * Bruce Bohn					10 Nov 2009  		Version 0.3 Updates
 * Sasha. M	/ Naveen. M			11 Nov 2009  		Version 1.0 Release
 * Sasha. M	/ Nithin. 			10 April 2010  		Version 1.20 Release
 * Nithin M						11 Aug 2010			Implemetation of Low Power Demo 
 *****************************************************************************/
 #ifndef _MTOUCHCAP_CTMU_API_C
#define  _MTOUCHCAP_CTMU_API_C


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Includes	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include 	"mTouchCap_CtmuAPI.h"
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* ~~~~~~~~~~~~~~~~~~~~~	Variables	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  	*/
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
WORD CapTouchkey_pressed_status =0;	//stores the count of the keys that were pressed
BYTE	trimbitsReady;
// array to store the active channel numbers that will be scanned in ISR
#ifdef PIC24F_LOW_PIN_DEVICE
WORD ScanChannels[MAX_ADC_CHANNELS]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
#elif defined  __PIC24F__
WORD ScanChannels[MAX_ADC_CHANNELS]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
#else
WORD ScanChannels[MAX_ADC_CHANNELS]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
#endif
BYTE ScanChannelIndex =0;  // store the index of the channels that has to be scanned
BYTE Previous_Pressed_Channel_Num =0; //stores the channel number of the previous key that was pressed	
BYTE Decode_Method_Used=DECODE_METHOD_PRESS_ASSERT;

#ifdef LOW_POWER_DEMO_ENABLE
	extern BYTE Clock_Switch_Enable_Flag;	//FRC clock mode enable

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
/********************************************************************
 Function			:   void mTouchCap_Init(void)
 
 PreCondition		:   None 
 
 Input         		:   None
  
 Output				:   None
 
 Side Effects		:   None

 Overview			: 	This function initializes the PORT, CTMU, TIMER and ADC modules.


 *******************************************************************/
void mTouchCap_Init(void)
{
	WORD Index=0;

	/* Initialize Physical layer */
	mTouchCapPhy_InitCTMU();				


	/***************************************************************************
	--------------- Demo of API "mTouchCapAPI_SetUpCTMU_Default" -----------------------		
	Initialize the CTMU module to defaults using the API "mTouchCapAPI_SetUpCTMU_Default".
	***************************************************************************/
	trimbitsReady = 0;
	for (Index=0; Index < ScanChannelIndex; Index++) 		  
	{
		mTouchCapAPI_SetUpCTMU_Default(ScanChannels[Index]);  // Set up defult channel index
	}

	trimbitsReady = 1;
	
	#ifdef USE_STATIC_TRIP_VALUE
	mTouchCap_UpdateStaticTripValue();			// Update the STATIC TRIP values defined in config.h
	#endif
	// Timer4 is not used as the interrupt source for creating the 1 msec interrupt
	#ifndef TIMER4_NOT_USED_FOR_CAP_SENSOR_PROCESSING
		mTouchCapPhy_TickInit();					// Initialize Timer for system tick
	#endif

    mTouchCapPhy_StablizeChannelData();  // Stabilize channel data
}	
	

/********************************************************************
 Function			:   void mTouchCapAPI_SetUpCTMU_Default(WORD ChannelNum)
 
 PreCondition		:   None

 Input          	:	ChannelNum-  Channel Number
   
 Output				:   None
 
 Side Effects		:   None

 Overview			: 	This API is for those who want to use the cap-touch application without having to tweak much.
 						The API sets up the channel in a predefined default method with known configuration settings.
 

 *******************************************************************/
void mTouchCapAPI_SetUpCTMU_Default(WORD ChannelNum)
{
	
	actualValue [ChannelNum] = 0;
	avg_delay [ChannelNum] = 0;	
	curRawData[ChannelNum] = 0;
	averageData[ChannelNum] = 0;

	hystValue[ChannelNum] = 0;	 	
	channel_TRIM_value[ChannelNum] = 0;	 
	channel_IRNG_value [ChannelNum] = 0;
	Press_State[ChannelNum] = 0;
	Channel_Pressed_Status[ChannelNum] = 0;
	//ScanChannels[ChannelNum]=0;
	/* Initialize Trip Value */
    #ifndef USE_STATIC_TRIP_VALUE
    tripValue[ChannelNum] = DEFAULT_TRIP_VALUE;
    #endif
	mTouchCapPhy_ChargeTimeInit();

	/***************************************************************************
	--------------- Demo of API "mTouchCapAPI_AutoAdjustChannel" -----------------------		
	Automatically adjust the charge voltage channel using the API "mTouchCapAPI_AutoAdjustChannel". 
	***************************************************************************/
	if (FAILED == mTouchCapAPI_AutoAdjustChannel(ChannelNum,CTMU_CHARGE_VOLTAGE_TO_ACHIEVE)  )  //0x2FF, 75% of charging of ADC Value  1024*0.75
	{
		// DO THE ERROR HANDLING HERE
	}
		
}


 /*********   	End of Function 	*********************/


/********************************************************************
  Function			:    WORD mTouchCapAPI_CTMU_GetChannelReading(WORD ChannelNum)
 
  PreCondition		:    Channel setup is complete 
 
  Input				:    ChannelNum - Channel number (must have enabled in "mTouchCAp_Config.h")
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
							CHANNEL_AN24 ,	
							CHANNEL_AN25 ,	
							CHANNEL_AN26 ,	
							CHANNEL_AN27,
							//Check the availability of channels in the PIC MCU being used							

							
 
  Output		   : 	ChannelData :ADC value (Range for 10 bit ADC is 0 : 0x3FF)
 
  Side Effects	   : 	None
 
  Overview	       : 	This API is used to get the channel reading. It initializes the CTMU and ADC module
						for the corresponding channel passed.	It reads ADC data from the channel and returns.
 				 	
 *******************************************************************/
WORD mTouchCapAPI_CTMU_GetChannelReading(WORD ChannelNum)
{
	WORD ChannelData;

	/* Check for valid channel number being passed */
	if ( (ChannelNum >= CHANNEL_AN0) &&  (ChannelNum < MAX_ADC_CHANNELS))
	{
	
       mTouchCapPhy_ChannelSetup (ChannelNum);
	   
       CTMU_Current_trim_config(ChannelNum);
		
	   mTouchCapPhy_Discharge_Current ();
		 
	   mTouchCapPhy_Charge_Current ();
		
		/* Disable CTMU Edge*/
	   Disbl_CTMUEdge1;

	   ChannelData = mTouchCapPhy_Read_ADC();
	}
	else
	{
		/* Error: exceeded available channel numbers */
		ChannelData = ADC_UNTOUCHED_VALUE_10_BIT;	//Indicates untouched value for 12-bit ADC
	 }
				
	return ChannelData;
 }

/*************************************************************************************
  Function:
        CHAR  mTouchCapAPI_AutoAdjustChannel (WORD ChannelNum, WORD AdcValueToAchieve)
    
    PreCondition   :    Channel setup is complete.
    
  Input:
    ChannelNum -         	CHANNEL_AN0 ,
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
							CHANNEL_AN24 ,	
							CHANNEL_AN25 ,	
							CHANNEL_AN26 ,	
							CHANNEL_AN27
							//Check the availability of channels in the PIC MCU being used	
							
    AdcValueToAchieve -  ADC Value to be achieved
  Return:
    CHAR ChannelAdjResult
      * -1 : FAILED
      * 1 : PASSED
  Side Effects:
    None
  Description:
    This API is used to automatically adjust the voltage reading(charge
    level) on a channel. This will read the ADC value after charging the
    channel, and then adjust the CTMU current source Trim bits to read the
    value defined as per AUTO_ADJUST_BAND_PERCENT.                                    
  *************************************************************************************/

CHAR  mTouchCapAPI_AutoAdjustChannel (WORD ChannelNum, WORD AdcValueToAchieve)
 {
	CHAR ChannelAdjResult = FAILED;  
	WORD AdcValueToAchieve_DataRangeMax;
	WORD AdcValueToAchieve_DataRangeMin;
	WORD ChannelAdcData;
	CHAR Trim_Value;
	CHAR Currrent_range_value;
	BYTE DelayLoop;

	
	Trim_Value = 0;


		Currrent_range_value = CURRENT_RANGE_100XBASE_CURRENT;

	
	/* Check for valid channel number being passed */
	if (! ( (ChannelNum >= CHANNEL_AN0) &&  (ChannelNum < MAX_ADC_CHANNELS)) )
	{
		/* return the error value if not a valid channel number */
		ChannelAdjResult = FAILED; 
	}
	else	 //Handle the Auto adjustment for a valid channel
	{

			do {

				mTouchCapAPI_CTMU_SetupCurrentSource(Currrent_range_value,Trim_Value);
 				for (DelayLoop=0; DelayLoop<= 20; DelayLoop++)
				{
					Nop();	 
					Nop();
					Nop();
					Nop();
				}	
				
				ChannelAdcData = mTouchCapAPI_CTMU_GetChannelReading(ChannelNum);

				AdcValueToAchieve_DataRangeMax = (AdcValueToAchieve + (AdcValueToAchieve*AUTO_ADJUST_BAND_PERCENT/100));	 
				AdcValueToAchieve_DataRangeMin = (AdcValueToAchieve - (AdcValueToAchieve*AUTO_ADJUST_BAND_PERCENT/100));
				
				if( ChannelAdcData>=AdcValueToAchieve_DataRangeMin && ChannelAdcData<=AdcValueToAchieve_DataRangeMax )
				 {
				 	ChannelAdjResult = PASSED;
				 	channel_TRIM_value[ChannelNum] =Trim_Value; 
					channel_IRNG_value[ChannelNum] = CTMU_Get_Current_Source;
					break;
				 }
				else
				{
				    if ( CTMU_Get_Current_Source == CURRENT_RANGE_100XBASE_CURRENT)
					   {
							if(Trim_Value<=CURRENT_SOURCE_TRIM_BITS_MAX_POSITIVE)
							{
								Trim_Value = Trim_Value+AUTO_ADJUST_STEP_SIZE;
								if(Trim_Value>CURRENT_SOURCE_TRIM_BITS_MAX_POSITIVE)
								{
								  Trim_Value = CURRENT_SOURCE_TRIM_BITS_MIN_NEGATIVE;
								}	
												
							}	
							
							if(Trim_Value>=CURRENT_SOURCE_TRIM_BITS_MAX_NEGATIVE)
							{
								Trim_Value = Trim_Value-AUTO_ADJUST_STEP_SIZE;
								if(Trim_Value<CURRENT_SOURCE_TRIM_BITS_MAX_NEGATIVE)
								{
								  Trim_Value = CURRENT_SOURCE_TRIM_BITS_NOMINAL;
								  Currrent_range_value = CURRENT_RANGE_10XBASE_CURRENT;
								}				
							}
				       }
				    else if ( CTMU_Get_Current_Source == CURRENT_RANGE_10XBASE_CURRENT)
					   {
							if(Trim_Value<=CURRENT_SOURCE_TRIM_BITS_MAX_POSITIVE)
							{
								Trim_Value = Trim_Value+AUTO_ADJUST_STEP_SIZE;
								if(Trim_Value>CURRENT_SOURCE_TRIM_BITS_MAX_POSITIVE)
								{
								  Trim_Value = CURRENT_SOURCE_TRIM_BITS_MIN_NEGATIVE;
								}	
												
							}	
							
							if(Trim_Value>=CURRENT_SOURCE_TRIM_BITS_MAX_NEGATIVE)
							{
								Trim_Value = Trim_Value-AUTO_ADJUST_STEP_SIZE;
								if(Trim_Value<CURRENT_SOURCE_TRIM_BITS_MAX_NEGATIVE)
								{
								  Trim_Value = CURRENT_SOURCE_TRIM_BITS_NOMINAL;
								  Currrent_range_value = CURRENT_RANGE_BASE_CURRENT;
								}				
							}
				       }
					    else 
					   {
							if(Trim_Value<=CURRENT_SOURCE_TRIM_BITS_MAX_POSITIVE)
							{
								Trim_Value = Trim_Value+AUTO_ADJUST_STEP_SIZE;
								if(Trim_Value>CURRENT_SOURCE_TRIM_BITS_MAX_POSITIVE)
								{
								  Trim_Value = CURRENT_SOURCE_TRIM_BITS_MIN_NEGATIVE;
								}	
												
							}	
							
							if(Trim_Value>=CURRENT_SOURCE_TRIM_BITS_MAX_NEGATIVE)
							{
								Trim_Value = Trim_Value-AUTO_ADJUST_STEP_SIZE;
								if(Trim_Value<CURRENT_SOURCE_TRIM_BITS_MAX_NEGATIVE)
								{
							 	  ChannelAdjResult = FAILED;
							      break;
								}				
							}
				       }			       
				       				       		
				}	
					
		    }while(1);
    }

	return ChannelAdjResult;
 }
 
 /*********   	End of Function 	*********************/




/******************************************************************************************
  Function:
        void mTouchCapAPI_CTMU_SetupCurrentSource (BYTE CurrentSourceRange, BYTE TrimValue)
    
    PreCondition  :    None
    
  Input:
    CurrentSourceRange -  Current source range
                          * CURRENT_RANGE_100XBASE_CURRENT //Current source
                            Range is 100*Base current (55uA)
                          * CURRENT_RANGE_10XBASE_CURRENT //Current source
                            Range is 10*Base current (5.5uA)
                          * CURRENT_RANGE_BASE_CURRENT //Current source
                            Range is Base current (0.55uA)
                          * CURRENT_SRC_DISABLED //Current source disabled
                          <p />
    TrimValue -           Trim settings
  Return:
    None
  Side Effects:
    None
  Description:
    This API sets the current source and trim level for a particular
    channel.                                                                               
  ******************************************************************************************/
void mTouchCapAPI_CTMU_SetupCurrentSource (BYTE CurrentSourceRange, BYTE TrimValue)
{

 mTouchCapPhy_SetupCurrentSourceRange(CurrentSourceRange);
 mTouchCapPhy_SetTrimValue( TrimValue);

}


 /*********   	End of Function 	*********************/

/********************************************************************
  Function			:    WORD mTouchCapAPI_ScanChannelIterative (WORD ChannelNum, BYTE SampleCount)
 
  PreCondition		:    Channel setup is complete.
 
  Input				:    ChannelNum - Channel number 
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
							CHANNEL_AN21,
							CHANNEL_AN22 ,	
							CHANNEL_AN23,
							CHANNEL_AN24 ,	
							CHANNEL_AN25 ,	
							CHANNEL_AN26 ,	
							CHANNEL_AN27,
							//Check the availability of channels in the PIC MCU being used							
							
							
						 SampleCount - Count of Samples to be taken per scan

 
  Output			:    RawData :Averaged ADC Value
 
  Side Effects		:    None
 
  Overview			: 	 This API is used for scanning the channels, one at a time. This should be called in the 
 				  		 Timer tick function, preferably in an interrupt. This will return the Averaged ADC value 
 				  		 based on the SampleCount passed.
 
 *******************************************************************/

WORD mTouchCapAPI_ScanChannelIterative (WORD ChannelNum, BYTE SampleCount)
{
	WORD RawData;
	WORD RawIterativeData = 0;

	DWORD total = 0;
	BYTE sample;

		//testing
//	AD1PCFGLbits.PCFG8 = 1;
//	TRISBbits.TRISB8 = 0;


	/* Check for valid channel number being passed */
	if (! ( (ChannelNum >= CHANNEL_AN0) &&  (ChannelNum < MAX_ADC_CHANNELS)) )
	{
		/* return the error value if not a valid channel number */
		RawData = ADC_UNTOUCHED_VALUE_10_BIT; 
	}
	else	 //Handle the scan for a valid channel
	{

		// Get the raw sensor reading.
		for(sample=0; sample< SampleCount; sample++)
		{
			RawIterativeData = mTouchCapAPI_CTMU_GetChannelReading(ChannelNum);
			
			total = total + RawIterativeData;
		}
		#if defined PROXIMITY
		curBigData[ChannelNum] = total;
		#endif	

		/* Error Check */
		if (total != 0)
		{
			RawData = total/ SampleCount;
		}
		else
		{
			RawData = 0;
		}
	}

	
	return RawData;
}



 /*********   	End of Function 	*********************/



/********************************************************************
  Function			:    BYTE mTouchCapAPI_getChannelTouchStatus(WORD ChIndex, BYTE Decode_Method)
 
  PreCondition		:    None
 
  Input				:    ChIndex - The Channel number. 
 					    Decode_Method - The type of Decode Method associated with that channel
 					    DECODE_METHOD_MOST_PRESSED,		
						DECODE_METHOD_MULTIPLE_PRESS,		
						DECODE_METHOD_PRESS_AND_RELEASE,	
						DECODE_METHOD_PRESS_ASSERT

 Output			:    	TouchStatus-Whether the key associated with the Channel is pressed or not
 					    KEY_NOT_PRESSED,
						KEY_PRESSED
 
  Side Effects		:    None
 
  Overview			:  	 This API will determine if the channel which is associated with a particular 
 				   		 key is touched or not. It will output the pressed or unpressed status of the channel based on the
 			             Decode method which is associated with the channel.
 
 
 
 *******************************************************************/

BYTE mTouchCapAPI_getChannelTouchStatus(WORD ChIndex, BYTE Decode_Method)
{
	BYTE temp;
	BYTE TouchStatus = KEY_NOT_PRESSED;

	/* Check for valid channel number being passed */
	if (! ( (ChIndex >= CHANNEL_AN0) &&  (ChIndex < MAX_ADC_CHANNELS)) )
	{
		/* return the error value if not a valid channel number */
		TouchStatus = KEY_NOT_PRESSED;
	}
	else	 //Handle the touch status for a valid channel
	{
		

	 switch (Decode_Method)
	 {
				 
		 case DECODE_METHOD_MOST_PRESSED:

			 
			    if (curRawData[ChIndex] < (averageData[ChIndex] - tripValue[ChIndex])) 
				{
					unpressedCount[ChIndex] = 0;
					pressedCount[ChIndex] = pressedCount[ChIndex] + 1;
					// Single-Button Only (Lockout other keys option)
					if(pressedCount[ChIndex] > DEBOUNCECOUNT) 
					{
					   Decode_Method_Used=DECODE_METHOD_MOST_PRESSED;
					   TouchStatus = KEY_PRESSED;
					    for(temp = 0; temp<ScanChannelIndex; temp++)
		                   {
			                  if(ScanChannels[temp]!=ChIndex)
			                    {
			    					if (curRawData[ScanChannels[temp]] < (averageData[ScanChannels[temp]] - tripValue[ScanChannels[temp]])) 
									{
				                     	   if ( curRawData[ChIndex] > curRawData[ScanChannels[temp]])    
			                               TouchStatus = KEY_NOT_PRESSED;
											//status of the key press for the low power demo
											CapTouchkey_pressed_status  = TouchStatus; 
			                       				                    
				                    }               
			                    }           
			               } 
								
					}
				}else
				{
					pressedCount[ChIndex] = 0;
					TouchStatus = KEY_NOT_PRESSED;
				}
				if (curRawData[ChIndex] > (averageData[ChIndex] - tripValue[ChIndex] + hystValue[ChIndex])) 
				{		// Is scaledBigValue above { average - (trip + hysteresis) } ? .. Yes
						// Button is up.
					unpressedCount[ChIndex] = unpressedCount[ChIndex] + 1;
					if (unpressedCount[ChIndex] > DEBOUNCECOUNT)
					{ 	
						TouchStatus = KEY_NOT_PRESSED;
						//status of the key press for the low power demo
						CapTouchkey_pressed_status  = TouchStatus; 
					}

				}
				else
				{
					unpressedCount[ChIndex] = 0;
				}		 
			 
		
		 break;

		 // This decode method will give the press status when the corresponding key is released after it is pressed
		 //The status TouchStatus is updated based on status of the key
		 //As long as the key is pressed, the status will remain as KEY_NOT_PRESSED. 
		 //The status will change to KEY_PRESSED as long as the key is released after it has been pressed
		 case DECODE_METHOD_PRESS_AND_RELEASE:

		 	    if (curRawData[ChIndex] < (averageData[ChIndex] - tripValue[ChIndex])) 
				{
					unpressedCount[ChIndex] = 0;
					pressedCount[ChIndex] = pressedCount[ChIndex] + 1;
					// Single-Button Only (Lockout other keys option)
					if(pressedCount[ChIndex] > DEBOUNCECOUNT) 
					{
					    TouchStatus = KEY_NOT_PRESSED;
						// This flag will indicate the channel that has been pressed so that the information about
						// the previous pressed key is stored.
						Press_State[ChIndex] = PRESS;		
						// the status of the previous pressed key should be reset when the new key is pressed
						Channel_Pressed_Status[Previous_Pressed_Channel_Num] =0; 
					}
				}else
				{ 
							//Check if the current channel has been pressed 
					if (Press_State[ChIndex])
					 { 					
					   if (curRawData[ChIndex] > (averageData[ChIndex] - tripValue[ChIndex] + hystValue[ChIndex])) 
					   {		// Is scaledBigValue above { average - (trip + hysteresis) } ? .. Yes
						// Button is up.
						pressedCount[ChIndex] = 0;
						unpressedCount[ChIndex] = unpressedCount[ChIndex] + 1;
						if (unpressedCount[ChIndex] > DEBOUNCECOUNT)
						{ 	
								
									pressedCount[ChIndex] = 0;
									//store the pressed status of the key so that the pressed status till
									//another key is pressed
									
									Channel_Pressed_Status[ChIndex] = KEY_PRESSED;

									TouchStatus = KEY_PRESSED ;
									//store the pressed key channel number so that it can be used in the application
									// to store the status of the channel of the pressed key
									Previous_Pressed_Channel_Num = ChIndex;
									//status of the key press for the low power demo
									CapTouchkey_pressed_status  =TouchStatus; 
									Decode_Method_Used=DECODE_METHOD_PRESS_AND_RELEASE;
						}
					  }
					 }//end of if (Press_State[ChIndex])
				    else
					{
						TouchStatus = KEY_NOT_PRESSED;
					    unpressedCount[ChIndex] = 0;
					    pressedCount[ChIndex] = 0;
					    Press_State[ChIndex] = RELEASE;
					}
               }         
				
			
		 break;
		 		 

		 case DECODE_METHOD_MULTIPLE_PRESS:
		 case DECODE_METHOD_PRESS_ASSERT:
		 	    if (curRawData[ChIndex] < (averageData[ChIndex] - tripValue[ChIndex])) 
				{
					unpressedCount[ChIndex] = 0;
					pressedCount[ChIndex] = pressedCount[ChIndex] + 1;
					// Single-Button Only (Lockout other keys option)
					if(pressedCount[ChIndex] > DEBOUNCECOUNT) 
					{
					   TouchStatus = KEY_PRESSED;
					   Decode_Method_Used=DECODE_METHOD_PRESS_ASSERT;
					//the variable  CapTouchkey_pressed_status will give the status of the keys
					//which will be used in the 10sec timeout logic which will reset the timeout and restart the timeout 
					//process
					CapTouchkey_pressed_status  = TouchStatus ; 
								
					}
				}else
				{
					pressedCount[ChIndex] = 0;
					TouchStatus = KEY_NOT_PRESSED;
				}
				if (curRawData[ChIndex] > (averageData[ChIndex] - tripValue[ChIndex] + hystValue[ChIndex])) 
				{		// Is scaledBigValue above { average - (trip + hysteresis) } ? .. Yes
						// Button is up.
					unpressedCount[ChIndex] = unpressedCount[ChIndex] + 1;
					if (unpressedCount[ChIndex] > DEBOUNCECOUNT)
					{ 	
						TouchStatus = KEY_NOT_PRESSED;
					}

				}
				else
				{
					unpressedCount[ChIndex] = 0;
				}

		 break;
		
		 default:
		 TouchStatus = KEY_NOT_PRESSED;
		 break;	
		
	 }	
	}
			 		// 6. Implement quick-release for a released button
		 		/* Untouched value - used as a average data */
	if (curRawData[ChIndex]  > averageData[ChIndex])
	   {
			averageData[ChIndex] = curRawData[ChIndex];				// If curRawData is above Average, reset to high average.
	   }
	
	return TouchStatus;
}	

/********************************************************************
  Function		:   CHAR mTouchCapAPI_SetUpChannelDirectKey(DirectKey *Button,CHAR Channel_number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
 
  PreCondition	:   None
 
  Input		       :   Button : Object of the Direct key structure
 					Channel number : channel number of the object Button associated with corresponding direct key.
 					Trip Value: Default Trip value for the channel specified by the channel number. 
 					Filter_Method: The filter method associated with the Direct Key. 
 						FILTER_METHOD_SLOWAVERAGE=0,
						FILTER_METHOD_GATEDAVERAGE,
						FILTER_METHOD_FASTAVERAGE

 					Decode_Method : The Decode method associated with the Direct Key.
 						DECODE_METHOD_MOST_PRESSED,		
						DECODE_METHOD_MULTIPLE_PRESS,		
						DECODE_METHOD_PRESS_AND_RELEASE,	
						DECODE_METHOD_PRESS_ASSERT
 
  Output		:     SetUpStatus - Status of the Direct key(TRUE or FALSE).
 
  Side Effects	:    None
 
  Overview		:    This API will setup the channel associated with the Direct key .
 					The channel number, filter type and decode method are stored in the structure associated with 
 					the Direct Key.
 
 
 *******************************************************************/
BYTE mTouchCapAPI_SetUpChannelDirectKey(DirectKey *Button,BYTE Channel_number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
{
	CHAR SetUpStatus = TRUE;

	Button->Channel_Num = Channel_number;                // Stores Channel Number
	Button->FilterType = Filter_Method;
	chFilterType [Channel_number] = Filter_Method;
	Button->DecodeMethod = Decode_Method;
	//NK_DIRKEY
	
	LoadChannelNumber(Channel_number);
	
	return SetUpStatus;
}	


/********************************************************************
  Function		:    BYTE mTouchCapAPI_GetStatusDirectButton(DirectKey *Button)
 
  PreCondition	:    The Channel associatd with the Direct key should  have been set up. 
 
  Input			:    Button - Object of the structure associated with the Direct Key
 
  Output		:    The Touch Status of the particular key.
  					  0 = KEY_NOT_PRESSED
 					  1 = KEY_PRESSED	
 
  Side Effects	:    None
 
  Overview		:    This API will provide the status of the Direct key passed which will be used 
 					 by the application to perform the related task.
 
  
 *******************************************************************/

BYTE mTouchCapAPI_GetStatusDirectButton (DirectKey *Button)
{
	BYTE DirectKey_Status = KEY_NOT_PRESSED;
		/* Check if the channel is used for Direct key */
		//Now check to see if the requested key is pressed
	if( KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Button->Channel_Num),(Button->DecodeMethod)))
	{
			Press_State[Button->Channel_Num]= RELEASE;  //reset the flag
			DirectKey_Status= KEY_PRESSED;
	}
      else
	{
			//Channel requested is not detected as pressed.
		DirectKey_Status= KEY_NOT_PRESSED;
	}
	
	return DirectKey_Status;

	
	
}	
/********************************************************************
  Function		:   BYTE mTouchCapAPI_SetUpChannelMatrixKey(MatrixKey *Button,BYTE Row_Channel_number,BYTE Col_Channel_number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
 
  PreCondition	:   None
 
  Input			:   Button - Object of the Matrix key structure
 					Row_Channel_number - channel number of the Row associated with corresponding matrix key.
 					Col_Channel_number - channel number of the Column associated with corresponding matrix key.
 					Trip Value - Default trip value for the channel associated with corresponding matrix key 
 					Filter_Method - The filter method associated with the corresponding Matrix key
 					Decode_Method - The Decode method associated with the corresponding Matrix key
 
  Output		:   SetUpStatus - Status of the Matrix key(TRUE or FALSE).
 
  Side Effects	:   None
 
  Overview		:  	This API will setup the channels of the Row and Column associated with the Matrix key.
 					The channel number of the Row and Column, filter type and decode method are stored in the structure associated with 
 					the corresponding channel.
 
 
  
 *******************************************************************/

BYTE mTouchCapAPI_SetUpChannelMatrixKey(MatrixKey *Button,BYTE Row_Channel_Number,BYTE Col_Channel_Number, WORD Trip_Value, WORD Decode_Method, WORD Filter_Method)
{
	BYTE SetUpStatus = TRUE;

	Button->Channel_Num[0] = Row_Channel_Number;  // Stores Channel Number of the Row asscoiated with the Matrix key
	Button->Channel_Num[1] = Col_Channel_Number;  // Stores Channel Number of the Column asscoiated with the Matrix key
	
	Button->FilterType = Filter_Method;   //Store the Filter type which will be common for both the multiplexed channel(Row and col)
	chFilterType [Row_Channel_Number] = Filter_Method;
	chFilterType [Col_Channel_Number] = Filter_Method;	
	Button->DecodeMethod = Decode_Method; //Store the Decode method which will be common for both the multiplexed channel(Row and col)
	
	//store the channels numbers in the global array for all the Matrix Key channels 
	
	//Store the Row and column channel numbers in the global array which will be used by the ISR
	// for reading the ADC value associated with the channels which are multiplxed to form a single key
	LoadChannelNumber(Row_Channel_Number);
	LoadChannelNumber(Col_Channel_Number);

	return SetUpStatus;
}	



/********************************************************************
  Function			:    BYTE mTouchCapAPI_GetStatusMatrixButton (MatrixKey *Button)
 
  PreCondition		:    The Channels of the Row and Column associatd with the Matrix key should  have been already set up 
 
  Input				:    Button - Object of the Structure associated with the Matrix Key
 
  Output		    :    The Touch Status of the particular key.
  							0 = KEY_NOT_PRESSED
 					 		1 = KEY_PRESSED	
 
  Side Effects		:    None
 
  Overview			:  	 This API will provide the status of the Matrix key passed which will be used 
 						 by the application to perform the related task.
 
 
 *******************************************************************/

	#ifdef	USE_MATRIX_KEYS
	BYTE mTouchCapAPI_GetStatusMatrixButton (MatrixKey *Button)
	{
		BYTE MatrixKey_Status = KEY_NOT_PRESSED;
		BYTE RowFlag = KEY_NOT_PRESSED;
		BYTE ColFlag = KEY_NOT_PRESSED;
	
		/* Check if the channel used as Row for corresponding Matrix key is pressed */
		//Now check to see if the requested row is pressed

		if( KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Button->Channel_Num[0]),(Button->DecodeMethod) ))
		{
			RowFlag=KEY_PRESSED;	// Used for determining ROW match
		}
		else
		{
			//Channel requested is not detected as pressed.
			RowFlag= KEY_NOT_PRESSED; 
		}

	 	//Now check to see if the channel used as column for corresponding Matrix key is pressed
		if( KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Button->Channel_Num[1]),(Button->DecodeMethod) ))
		{
		     ColFlag=KEY_PRESSED;	// Used for determining COL match
		}
		else
		{
			//Channel requested is not detected as pressed.
		    ColFlag= KEY_NOT_PRESSED; 
		}
		
		// check if both the row and column corresponding to the matrix key is pressed
		
				
		if( (ColFlag == KEY_PRESSED) && (RowFlag == KEY_PRESSED) )
			{
			MatrixKey_Status= KEY_PRESSED;		//matrix key is pressed
			Press_State[Button->Channel_Num[0]]= RELEASE;  //reset the flag
			Press_State[Button->Channel_Num[1]]= RELEASE;  //reset the flag
			}
		else
			{
			//If both channels are not pressed, the key is not pressed
			MatrixKey_Status= KEY_NOT_PRESSED; //key is not pressed
			}
	
		return MatrixKey_Status;
	}
	
	#endif
/********************************************************************
  Function		:    BYTE mTouchCapAPI_SetUpChannelSlider2Ch(Slider2Ch *Slider, BYTE Slider_Channel1_number,BYTE Slider_Channel2_number, WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)
 
 
  Input			:   Slider - Object of the Matrix key structure
 					Channel number - channel number of the object Slider associated with corresponding 2-channel slider.
 					Trip Value - Trip value for the channels associated with the  2-channel slider 
 					Filter_Method - One of the filter method for the 2-channel slider
 					Decode_Method -The Decode method for the 2-channel slider
 
  Output		:   SetUpStatus - Status of the 2-channel slider(TRUE or FALSE).
 
  Side Effects	:   None
 
  Overview		:  	This API will setup the 2 channels associated with the 2-channel Slider.
 					The 2 channel numbers, filter type and decode method are stored in the structure associated with
 					the corresponding 2-Channel Slider
 
 
 
 *******************************************************************/

BYTE mTouchCapAPI_SetUpChannelSlider2Ch(Slider2Ch *Slider, BYTE Slider_Channel1_Number,BYTE Slider_Channel2_Number, WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)


{
	BYTE SetUpStatus = TRUE;

	Slider->Channel_Num[0] = Slider_Channel1_Number;  // Stores first Channel Number of the 2-channel slider
	Slider->Channel_Num[1] = Slider_Channel2_Number;  // Stores Second Channel Number of the 2-channel slider
	
	Slider->FilterType = Filter_Method;   //Store the Filter type which will be common for both the channels in the 2-channel slider
	
	chFilterType [Slider_Channel1_Number] = Filter_Method;
	chFilterType [Slider_Channel2_Number] = Filter_Method;
	
	Slider->DecodeMethod = Decode_Method; //Store the Decode method which will be common for both the channels in the 2-channel slider
	
	//store the channels numbers in the global array which are associated with the 2-channel slider 
	
	//Store the Channel 1 and Channel 2 numbers of the 2-channel slider in the global array which will be used by the ISR
	// for reading the ADC value associated 
	
	LoadChannelNumber(Slider_Channel1_Number);
	LoadChannelNumber(Slider_Channel2_Number);

	return SetUpStatus;
}

/**********************************************************************************
 Function		:	SHORT  mTouchCapAPI_GetStatusSlider2Ch (Slider2Ch *Slider)
	 
 PreCondition	:	 2-channel Slider setup is complete.
	
 Input			:	Slider - Object of the 2-channel slider
						
 Output 		:	SliderLevel gives the Slider percent level of the touch.
	 
 Side Effects	:	None
	 
 Overview		: 	This API gets the percentage level of a particular 2-channel slider passed.
 					The output is ratio-metrically calculated from 0% to 100% proportional to the finger on the slider.

	 
 **********************************************************************************/
 #if defined(USE_SLIDER_2CHNL) 
	
SHORT  mTouchCapAPI_GetStatusSlider2Ch (Slider2Ch *Slider)
 {
			
		SHORT SliderState;
		WORD Percent_Level = 0;
	 
	
		#ifdef USE_SLIDER_2CHNL
				 
				if( (KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[0]),(Slider->DecodeMethod) )) || 
				    (KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[1]),(Slider->DecodeMethod) )) )
				{
					
					//#ifdef USE_SLIDER_2CHNL
						/* Use the 2ch slider algorithm to get the Slider Level */
						Percent_Level = mTouchCapApp_TwoChannelSliderLogic(Slider);
						SliderState = (SHORT)Percent_Level;
					//#endif
				}	
				else
				{
					SliderState = -1;
					
				}	
				
							
		#endif // USE_SLIDER_2CHNL
							
		/* Get the Level */
		
	
		return SliderState;
 }
 #endif // end of  #if defined(USE_SLIDER_2CHNL) 

/********************************************************************
  Function		:    BYTE mTouchCapAPI_SetUpChannelSlider4Ch(Slider4Ch *Slider, BYTE Slider_Channel1_Number,BYTE Slider_Channel2_Number, BYTE Slider_Channel3_Number,BYTE Slider_Channel4_Number,WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)
 
  PreCondition	:    None
 
  Input			:   Slider : object of the 4-channel Slider structure
 					Channel number : channel number of the object Slider associated with 4-channel slider.
 					Trip Value: Trip value for the channels associated with the 4-channel slider.
 					Filter_Method: The filter method for the 4-channel slider. 
 					Decode_Method:The Decode method for the 4-channel slider.
 
  Output		:   SetUpStatus - Status of the 4-channel slider(TRUE or FALSE).
 
  Side Effects	:   None
 
  Overview		:  	This API will setup the 4 channels associated with the 4-channel Slider.
 					The 4 channel numbers, filter type and decode method are stored in the structure associated with
 					the corresponding 4-Channel Slider


 *******************************************************************/

BYTE mTouchCapAPI_SetUpChannelSlider4Ch(Slider4Ch *Slider, BYTE Slider_Channel1_Number,BYTE Slider_Channel2_Number, BYTE Slider_Channel3_Number,BYTE Slider_Channel4_Number,WORD Trip_Value, BYTE Decode_Method, BYTE Filter_Method)

{
	BYTE SetUpStatus = TRUE;

	Slider->Channel_Num[0] = Slider_Channel1_Number;  // Stores first Channel Number of the 4-channel slider
	Slider->Channel_Num[1] = Slider_Channel2_Number;  // Stores Second Channel Number of the 4-channel slider
	Slider->Channel_Num[2] = Slider_Channel3_Number;  // Stores third  Channel Number of the 4-channel slider
	Slider->Channel_Num[3] = Slider_Channel4_Number;  // Stores fourth Channel Number of the 4-channel slider
	
	Slider->FilterType = Filter_Method;   //Store the Filter type which will be common for all the channels in the 4 channel slider
	
	chFilterType [Slider_Channel1_Number] = Filter_Method;
	chFilterType [Slider_Channel2_Number] = Filter_Method;
	chFilterType [Slider_Channel3_Number] = Filter_Method;
	chFilterType [Slider_Channel4_Number] = Filter_Method;
	
	
	Slider->DecodeMethod = Decode_Method; //Store the Decode method which will be common for all the channels in the 4 channel slider
	
	//store the channels numbers in the global array which are associated with the 4-channel slider 
	
	//Store the Channel 1,Channel 2,Channel 3 and Channel4  numbers of the 4-channel slider in the global array which will be used by the ISR
	// for reading the ADC value associated 
	
	LoadChannelNumber(Slider_Channel1_Number);
	LoadChannelNumber(Slider_Channel2_Number);
	LoadChannelNumber(Slider_Channel3_Number);
	LoadChannelNumber(Slider_Channel4_Number);
 
	return SetUpStatus;
}



/**********************************************************************************
 Function		:	SHORT	mTouchCapAPI_GetStatusSlider4Ch (Slider4Ch *Slider)
					  
 PreCondition	:	 4-channel Slider setup is complete
	 
 Input		 	:	Slider - Object of the 4-channel slider
						 
 Output		 	:	sliderLevel gives the Slider percent level of the touch.
	  
 Side Effects	:	None
	  
 Overview		: 	This API gets the percentage level of a particular 4-channel slider passed.
 					The output is ratio-metrically calculated from 0% to 100% proportional to the finger on the slider.	

					
 **********************************************************************************/
 #if defined(USE_SLIDER_4CHNL)	
	
	SHORT  mTouchCapAPI_GetStatusSlider4Ch (Slider4Ch *Slider)
	{
			
		SHORT SliderState;
		WORD Percent_Level = 0;
	
		
	#ifdef USE_SLIDER_4CHNL
					 
	if( (KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[0]),(Slider->DecodeMethod) )) || 
		(KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[1]),(Slider->DecodeMethod) )) ||
		(KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[2]),(Slider->DecodeMethod) )) || 
		(KEY_PRESSED == mTouchCapAPI_getChannelTouchStatus((Slider->Channel_Num[3]),(Slider->DecodeMethod) )) 
	  )
	  {
			/* Use the 4ch slider algorithm to get the Slider Level */
			Percent_Level = mTouchCapApp_FourChannelSliderLogic(Slider);
			SliderState = (SHORT)Percent_Level;
	   }
	else
	   {
			SliderState = -1; // For KEY_NOT_PRESSED
   	   }	
	
	#endif			
			
	  return SliderState;
 }
 #endif // end of  #if defined(USE_SLIDER_4CHNL)


/********************************************************************
 * Function		:    void LoadChannelNumber (BYTE Channel_Number_Passed)
 *
 * PreCondition	:    None
 *
 * Input		:    Channel Number
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		: 	 This function will load the active Touchkey Channel number in the 
 *				  	 global array ScanChannels[ScanChannelIndex++] and will also
 *  			  	 store the total channels used in the application
 *
 *  
 *******************************************************************/
void LoadChannelNumber (BYTE Channel_Number_Passed)
{
 BYTE index;
 BYTE result = FALSE;

   for(index = 0; index<ScanChannelIndex; index++)
   {
	   if(ScanChannels[index] == Channel_Number_Passed)
       {
	     result = TRUE;
	   }
    } 

	if(!result)
	{
	    ScanChannels[ScanChannelIndex++] = Channel_Number_Passed; 
	   	if(ScanChannelIndex > MAX_ADC_CHANNELS)// the ScanChannelIndex should not be incremented if the index is greater than the no of ADC channels used 
		// If the index is greater than the number of channels in the direct key board, then the index should
		// not be incremented. reset the index value
		ScanChannelIndex = 0;
		
	 }            	    
  
}


/********************************************************************
 * Function		:    void mTouchCap_UpdateStaticTripValue(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		:   It will load the static trip values for all the channels which   
 *				    cannot be changed in runtime.
 *
 *
 * Note			: 
 *******************************************************************/
void mTouchCap_UpdateStaticTripValue(void)
{
  #ifdef 	USE_STATIC_TRIP_VALUE
	tripValue[CHANNEL_AN0] = STATIC_TRIP_VALUE_CHANNEL0;
	tripValue[CHANNEL_AN1] = STATIC_TRIP_VALUE_CHANNEL1;
	tripValue[CHANNEL_AN2] = STATIC_TRIP_VALUE_CHANNEL2;
	tripValue[CHANNEL_AN3] = STATIC_TRIP_VALUE_CHANNEL3;
	tripValue[CHANNEL_AN4] = STATIC_TRIP_VALUE_CHANNEL4;
	tripValue[CHANNEL_AN5] = STATIC_TRIP_VALUE_CHANNEL5;
	tripValue[CHANNEL_AN6] = STATIC_TRIP_VALUE_CHANNEL6;
	tripValue[CHANNEL_AN7] = STATIC_TRIP_VALUE_CHANNEL7;
	tripValue[CHANNEL_AN8] = STATIC_TRIP_VALUE_CHANNEL8;
	tripValue[CHANNEL_AN9] = STATIC_TRIP_VALUE_CHANNEL9;
	tripValue[CHANNEL_AN10] = STATIC_TRIP_VALUE_CHANNEL10;
	tripValue[CHANNEL_AN11] = STATIC_TRIP_VALUE_CHANNEL11;
	tripValue[CHANNEL_AN12] = STATIC_TRIP_VALUE_CHANNEL12;
	tripValue[CHANNEL_AN13] = STATIC_TRIP_VALUE_CHANNEL13;
	tripValue[CHANNEL_AN14] = STATIC_TRIP_VALUE_CHANNEL14;
	tripValue[CHANNEL_AN15] = STATIC_TRIP_VALUE_CHANNEL15;

	/* Trip values are updated for below defined devices */
	#if defined(__PIC24FJ128DA110__) || defined(__PIC24FJ128DA210__)|| defined(__PIC24FJ256DA110__)\
 	|| defined(__PIC24FJ256DA210__) ||defined(__PIC24FJ128GB110__) || defined(__PIC24FJ128GB210__)\
	|| defined(__18F85K22)  || defined(__18F86K22) || defined(__18F87K22)|| defined(__18LF85K90)\
	|| defined(__18LF86K90) || defined(__18LF87K90)
	
		tripValue[CHANNEL_AN16] = STATIC_TRIP_VALUE_CHANNEL16;
		tripValue[CHANNEL_AN17] = STATIC_TRIP_VALUE_CHANNEL17;
		tripValue[CHANNEL_AN18] = STATIC_TRIP_VALUE_CHANNEL18;
		tripValue[CHANNEL_AN19] = STATIC_TRIP_VALUE_CHANNEL19;
		tripValue[CHANNEL_AN20] = STATIC_TRIP_VALUE_CHANNEL20;
		tripValue[CHANNEL_AN21] = STATIC_TRIP_VALUE_CHANNEL21;
		tripValue[CHANNEL_AN22] = STATIC_TRIP_VALUE_CHANNEL22;
		tripValue[CHANNEL_AN23] = STATIC_TRIP_VALUE_CHANNEL23;

	#elif defined(__18F43K22)  || defined(__18F44K22) || defined(__18F45K22) || defined(__18F46K22)\
  		||defined(__18LF43K22)  || defined(__18LF44K22) || defined(__18LF45K22) || defined(__18LF46K22)
			tripValue[CHANNEL_AN16] = STATIC_TRIP_VALUE_CHANNEL16;
			tripValue[CHANNEL_AN17] = STATIC_TRIP_VALUE_CHANNEL17;
			tripValue[CHANNEL_AN18] = STATIC_TRIP_VALUE_CHANNEL18;
			tripValue[CHANNEL_AN19] = STATIC_TRIP_VALUE_CHANNEL19;
			tripValue[CHANNEL_AN20] = STATIC_TRIP_VALUE_CHANNEL20;
			tripValue[CHANNEL_AN21] = STATIC_TRIP_VALUE_CHANNEL21;
			tripValue[CHANNEL_AN22] = STATIC_TRIP_VALUE_CHANNEL22;
			tripValue[CHANNEL_AN23] = STATIC_TRIP_VALUE_CHANNEL23;
			tripValue[CHANNEL_AN24] = STATIC_TRIP_VALUE_CHANNEL24;
			tripValue[CHANNEL_AN25] = STATIC_TRIP_VALUE_CHANNEL25;
			tripValue[CHANNEL_AN26] = STATIC_TRIP_VALUE_CHANNEL26;
			tripValue[CHANNEL_AN27] = STATIC_TRIP_VALUE_CHANNEL27;
	#endif


	
  #endif	
}	

/********************************************************************
 * Function		:    void InitAvgDelay(void)
 *
 * PreCondition	:    None
 *
 * Input		:    None
 *
 * Output		:    None
 *
 * Side Effects	:    None
 *
 * Overview		:    Initialize delay count to 0 for all assigned channels	  
 *
 *
 * Note			:
 *******************************************************************/

void InitAvgDelay(void)
{
	WORD Index;
	for(Index=0; Index<ScanChannelIndex; Index++)	//Initialize delay count to 0 for all channels	 
	{
		avg_delay[ScanChannels[Index]] = 0;
	}
}



#undef _MTOUCHCAP_CTMU_API_C
#endif 



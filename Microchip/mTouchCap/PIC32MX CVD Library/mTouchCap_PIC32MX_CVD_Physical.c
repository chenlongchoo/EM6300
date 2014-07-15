/*****************************************************************************
* FileName:        mTouchCap_PIC32MX_CVD_Physical.c
* Includes:
*   void mTouchCapPhy_UpdateData(UINT16 *MeasurementData )
*   void mTouchCapPhy_StablizeChannelData(void)
*   void DynamicTripValueCalculation(WORD Index)
*   void mTouchCapPhy_AdcSetup(void)
*   void mTouchCapPhy_TickInit(void)
*   void mTouchCapPhy_InitCVD(void)
*
* Software License Agreement
*
* Copyright © 2011 Microchip Technology Inc.
* Microchip licenses this software to you solely for use with Microchip
* products, according to the terms of the accompanying click-wrap software
* license. Microchip and its licensors retain all right, title and interest in
* and to the software.  All rights reserved. This software and any accompanying
* information is for suggestion only. It shall not be deemed to modify
* Microchip’s standard warranty for its products.  It is your responsibility to
* ensure that this software meets your requirements.
*
* SOFTWARE IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR
* IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
* NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL
* MICROCHIP OR ITS LICENSORS BE LIABLE FOR ANY DIRECT OR INDIRECT DAMAGES OR
* EXPENSES INCLUDING BUT NOT LIMITED TO INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
* OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
* SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, OR ANY CLAIMS BY THIRD PARTIES
* (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*
* The aggregate and cumulative liability of Microchip and its licensors for
* damages related to the use of the software will in no event exceed $1000 or
* the amount you paid Microchip for the software, whichever is greater.
*
* MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
* TERMS AND THE TERMS OF THE ACCOMPANYING CLICK-WRAP SOFTWARE LICENSE.
*
* Author                Date         Comment
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Naveen. M             14 Apr 2009  Version 0.1 Initial Draft
* Nithin Kumar. M       14 May 2009  Version 0.2 Updates
* Sasha. M / Naveen. M  4 June 2009  Version 0.3 Updates
* Sasha. M / Naveen. M  11 Nov 2009  Version 1.0 Release
* Sasha. M / Nithin.    10 Apr 2010  Version 1.20 Release
* MC                    10 Aug 2010  Version 1.21 Release
* MWM                   28 Mar 2011  Added mTouchCapPhy_UpdateData
*****************************************************************************/


////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "mTouchCap_CvdAPI.h"
#include "mTouchCap_PIC32MX_CVD_Physical.h"
#include "mTouchCap_Timers.h"


/* if sampling delay is 0, make sure that the conversion starts simultaneous */
#if (SAMPLING_DELAY == 0)
#   define BEGIN_CONVERSION_IMMEDIATELY
#else
#   define SAMPLING_DELAY_REF         SAMPLING_DELAY
#   undef BEGIN_CONVERSION_IMMEDIATELY
#endif//(SAMPLING_DELAY == 0)

// from PIC32MX CVD Library\mTouchCap_CvdAPI.c
extern  UINT8 HF_Read_Number;    //HF repeat number in current scan
extern  UINT8 NumberScanChannels;
extern UINT16 ScanChannels[MAX_ADC_CHANNELS];

extern UINT16 averageData[MAX_ADC_CHANNELS];   // running average of CVD channels
extern  UINT8 dataReadyCVD;//global indicating the reading of all channels finished
extern  UINT8 button_set_number; //button current scan
extern UINT16 hystValue  [MAX_ADC_CHANNELS];
extern UINT16 tripValue  [MAX_ADC_CHANNELS];


/* Global Variables referenced elsewhere *************************************/
UINT8  chFilterType[MAX_ADC_CHANNELS]; // Channel filter type
UINT16 CurFiltdData[MAX_ADC_CHANNELS]; // current filtered value for all channels
UINT16 ButtonWeight[MAX_ADC_CHANNELS]; // measurement of button presses
UINT16 MinButtonWeight;
INT8   iScanChanMinWeight;

#if   defined( UART_DUMP_RAW_COUNTS )
volatile UINT16 CurRawData[  NUM_HF_READS][MAX_ADC_CHANNELS]; // Scratch storage for UART dump
#elif defined( UART_DUMP_ALL_COUNTS )
volatile UINT16 CurRawData[3*NUM_HF_READS][MAX_ADC_CHANNELS]; // Scratch storage for UART dump
#endif

// Cumulative sum for average measurement calculation
// used to compute "current raw value" afer NUM_HF_READs
volatile UINT32 ButtonCumulativeSum[MAX_ADC_CHANNELS];

void mTouchCapPhy_UpdateData( UINT16 *CurrentMeasurementData,
                              UINT16 *AveragedData,
                              UINT16 *Weights)
{
    static UINT16 Log2AlphaInverse = 0;
    static UINT16 StartupCount = 0;
    UINT16 ButtonWeightTemp;
    UINT16 ButtonRawData;
    UINT8  iScanChan;
    UINT8  iButton; //Which button
    BYTE   FilterType;

    iScanChanMinWeight = -1; //Assume no buttons are being pressed
    MinButtonWeight = 1<<8;;
    // Compute average of all HF scans, update averages
    for(iButton=0; iButton<NumberScanChannels; iButton++)
    {
        CurrentMeasurementData[iButton] = ButtonCumulativeSum[iButton]/NUM_HF_READS;
        ButtonCumulativeSum[iButton] = 0;

        iScanChan = ScanChannels[iButton];
        CurFiltdData[iScanChan] = CurrentMeasurementData[iButton];

        FilterType = chFilterType[iScanChan];
        if ( (FilterType > FILTER_METHOD_FASTAVERAGE) )
        {
             FilterType = FILTER_METHOD_SLOWAVERAGE;
        }

        switch(FilterType)
        {
            case FILTER_METHOD_SLOWAVERAGE:
                // Average in raw value.
                averageData[iScanChan] = averageData[iScanChan] - (averageData[iScanChan]>>Log2AlphaInverse) + (CurFiltdData[iScanChan]>>Log2AlphaInverse);
                DynamicTripValueCalculation(iScanChan);

                break;


            case FILTER_METHOD_GATEDAVERAGE:
                if ( KEY_NOT_PRESSED == mTouchCapAPI_getChannelTouchStatus(iScanChan,DECODE_METHOD_PRESS_ASSERT ))
                {
                    // Average in raw value.
                    averageData[iScanChan] = averageData[iScanChan] - (averageData[iScanChan]>>Log2AlphaInverse) + (CurFiltdData[iScanChan]>>Log2AlphaInverse);
                    DynamicTripValueCalculation(iScanChan);
                }

                break;

            case FILTER_METHOD_FASTAVERAGE:
                if (CurFiltdData[iScanChan]  > averageData[iScanChan])
                {
                    averageData[iScanChan] = CurFiltdData[iScanChan];             // If CurFiltdData is above Average, reset to high average.
                }
                DynamicTripValueCalculation(iScanChan);

                break;

        }//end switch(FilterType)

        AveragedData[iButton] = averageData[iScanChan];

        // Calculate button weight as scaled fraction of averageData.
        // 2^8 = 256 => full scale, i.e. equal to averageData
        // The lower the  number the more the button is being pressed

        // Make sure ratio is never bigger than 1
        ButtonRawData = CurFiltdData[iScanChan] < averageData[iScanChan] ? CurFiltdData[iScanChan] : averageData[iScanChan] ;
        if ( averageData[iScanChan] > 0 )
        {
             // Calculate button weight as a binary percentage, where 1/1 = 256 counts
             ButtonWeightTemp = (UINT16)( ((UINT32)ButtonRawData<<8)/averageData[iScanChan] );
             ButtonWeight[iButton] = ButtonWeightTemp;
             if ( ButtonWeightTemp < MinButtonWeight )
             {
                 MinButtonWeight = ButtonWeightTemp;
                 iScanChanMinWeight = iScanChan;
             }
        }
        else
        {
            ButtonWeight[iButton] = 0XFFFF;
        }

        Weights[iButton] = ButtonWeight[iButton];

    }//end for(iButton=0; iButton<NumberScanChannels; iButton++)

    StartupCount++;
    if ( StartupCount >= INITIAL_STARTUP_COUNT )
    {
        StartupCount = INITIAL_STARTUP_COUNT;
        Log2AlphaInverse++;
        Log2AlphaInverse = Log2AlphaInverse < LOG2_ALPHA_INVERSE ? Log2AlphaInverse : LOG2_ALPHA_INVERSE;
    }
}


/******************************************************************************
 * Function:        void mTouchCapPhy_StablizeChannelData(void)
 *
 *****************************************************************************/
void mTouchCapPhy_StablizeChannelData(void)
{

    WORD i;

    static int             startupCount;    // variable to 'discard' first N samples

    startupCount    = INITIAL_STARTUP_COUNT;

    while (startupCount > 0)
    {
        while(!dataReadyCVD);

        dataReadyCVD = 0;           //clear flag

        Set_ScanTimer_IF_Bit_State(DISABLE);              //Clear timer 4 SHORT flag
        Set_ScanTimer_IE_Bit_State(DISABLE);              //Disable interrupt
        Set_ScanTimer_ON_Bit_State(DISABLE);              //Stop timer

        startupCount--;                         // Decr. N # times to establish startup

        Set_ScanTimer_IF_Bit_State(DISABLE);              //Clear flag
        Set_ScanTimer_IE_Bit_State(ENABLE);              //Enable interrupt
        Set_ScanTimer_ON_Bit_State(ENABLE);              //Run timer
    }

    for(i=0; i<NumberScanChannels; i++)
    {
        // During start up time, set Average each pass.
        averageData[ScanChannels[i]] = CurFiltdData[ScanChannels[i]];
    }

}


/********************************************************************
 * Function     :    void DynamicTripValueCalculation(WORD Index)
 *
 * PreCondition :    None
 *
 * Input        :    Index- Channel Number
 *
 * Output       :    None
 *
 * Side Effects :    None
 *
 * Overview     :    calculate the dynamic trip value.
 *
 *
 * Note         :
 *******************************************************************/
void DynamicTripValueCalculation(WORD Index)
{

    tripValue[Index] = (averageData[Index] / KEYTRIPDIV);
    hystValue[Index] = (tripValue[Index] / HYSTERESIS_VALUE);

}

/********************************************************************
 * Function         :    void mTouchCapPhy_AdcSetup(void)
 *
 * PreCondition     :    None
 *
 * Input            :    None
 *
 * Output           :    None
 *
 * Side Effects     :    None
 *
 * Overview         :   This function will setup the ADC module
 *
 * Note             :   Does the necessary ADC peripheral setup.
 *******************************************************************/
void mTouchCapPhy_AdcSetup(void)
{
    InitADC2();
}

/********************************************************************
 * Function         :    void mTouchCapPhy_TickInit(void)
 *
 * PreCondition     :    None
 *
 * Input            :    None
 *
 * Output           :    None
 *
 * Side Effects     :    None
 *
 * Overview         :   This function will setup the Timer4 module
 *
 * Note             :   Does the necessary Timer4 peripheral setup except period.
 *******************************************************************/
void mTouchCapPhy_TickInit(void)
{
    InitTimer4();
}

/********************************************************************
 * Function         :    void mTouchCapPhy_InitCVD(void)
 *
 * PreCondition     :    None
 *
 * Input            :    None
 *
 * Output           :    None
 *
 * Side Effects     :    None
 *
 * Overview         :    Does the initialization of all peripherals/ports necessary for CVD operation.
 *
 *
 * Note             :
 *******************************************************************/
void mTouchCapPhy_InitCVD(void)
{

    /* Initialize PORTs  */
    mTouchCapApp_PortSetup();

    /* Initialize ADC  */
    mTouchCapPhy_AdcSetup();

    Set_Adc_Enable_State(ENABLE);

}

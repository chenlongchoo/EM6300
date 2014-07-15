/******************************************************************************
* File Name:       mTouchCapAdc.c
* Includes:
*   void InitADC2(void *ADC1CallBackISR)
*   void mTouchCapADC_SetChannelADC(WORD channel_select)
*
* Dependencies:    None
* Processor:       PIC32MX795F512H
* Compiler:        C32
* Company:         Microchip Technology, Inc.
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
* Author    Date          Comments
*--------------------------------------------------------------------------------
* MC        22-Jan-2010   First release of ADCDriver.c, ADCDriver.h
*                         Porting for PIC32MX795F512H
* MWM       28 Mar 2011   Removed ADC ISR, capture now in Timer 4 ISR.
********************************************************************************/
#include "config.h"
#include "HardwareProfile.h"
#include "GenericTypeDefs.h"
#include "mTouchCap_Adc.h"
#include "mTouchCap_CvdAPI.h"

void InitADC2(void)
{
//*****************************************************************************
// ADC Setup
//*****************************************************************************

    // configure and enable the ADC
    AD1CON1 = 0x0;

    /* do not scan inputs */
    /* interrupt at completion of each conversion */
    /* always use MUXA */
    AD1CON2 = 0x0;

    /* clock derived from periph clock */
    /* Tadc = 2*(AD1CON<7:0>+1)*Tpbus */
    /* Tadc = 2*(   2       +1)*Tpbus */
    /* Tadc = 6 * Tpbus = 6 * 12.5 ns = 75 ns > 65 ns required */
    AD1CON3 = 0x0002;

    /* reset value for channel select at this point */
    AD1CHS = 0x0;

#if  defined(USE_DIRECT_KEYS)

 // Port configuration defined in HardwareProfile.h
    AD1PCFG = ~( (1<<DIRECTKEY1_CHANNEL) |
                 (1<<DIRECTKEY2_CHANNEL) |
                 (1<<DIRECTKEY3_CHANNEL) |
                 (1<<DIRECTKEY4_CHANNEL) |
                 (1<<DIRECTKEY5_CHANNEL) |
                 (1<<DIRECTKEY6_CHANNEL) |
                 (1<<DIRECTKEY7_CHANNEL) |
                 (1<<DIRECTKEY8_CHANNEL) );

#elif defined(USE_MATRIX_KEYS)

 // Port configuration defined in HardwareProfile.h
    AD1PCFG =~((1<<MATRIXKEY0_ROW_CHANNEL)  | (1<<MATRIXKEY0_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY1_ROW_CHANNEL)  | (1<<MATRIXKEY1_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY2_ROW_CHANNEL)  | (1<<MATRIXKEY2_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY3_ROW_CHANNEL)  | (1<<MATRIXKEY3_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY4_ROW_CHANNEL)  | (1<<MATRIXKEY4_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY5_ROW_CHANNEL)  | (1<<MATRIXKEY5_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY6_ROW_CHANNEL)  | (1<<MATRIXKEY6_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY7_ROW_CHANNEL)  | (1<<MATRIXKEY7_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY8_ROW_CHANNEL)  | (1<<MATRIXKEY8_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY9_ROW_CHANNEL)  | (1<<MATRIXKEY9_COLUMN_CHANNEL)  |
               (1<<MATRIXKEY10_ROW_CHANNEL) | (1<<MATRIXKEY10_COLUMN_CHANNEL) |
               (1<<MATRIXKEY11_ROW_CHANNEL) | (1<<MATRIXKEY11_COLUMN_CHANNEL) );

#elif defined(USE_SLIDER_4CHNL)

 // Port configuration defined in HardwareProfile.h
    AD1PCFG = ~( (1<<FOUR_CH_SLIDER1_CHANNEL0) |
                 (1<<FOUR_CH_SLIDER1_CHANNEL1) |
                 (1<<FOUR_CH_SLIDER1_CHANNEL2) |
                 (1<<FOUR_CH_SLIDER1_CHANNEL3) );

#elif defined(USE_SLIDER_2CHNL)

 // Port configuration defined in HardwareProfile.h
    AD1PCFG = ~( (1<<TWO_CH_SLIDER1_CHANNEL0) |
                 (1<<TWO_CH_SLIDER1_CHANNEL1) );

#else
#   error("Don't recognize demo type!")
#endif

    /* no channel is scanned */
    AD1CSSL = 0x0;

    /* DIS-ENABLE interrupt */
    IEC1bits.AD1IE = 0;

}


/********************************************************************
 * Function         :    void mTouchCapADC_SetChannelADC(WORD chan_param)
 *
 * PreCondition     :    None
 *
 * Input            :    channel_select - This contains the input select parameter to be
                         configured into the ADCHS register as defined below
 *
 * Output           :    None
 *
 * Side Effects     :    None
 *
 * Overview         :    This function sets the positive and negative inputs for
                         the sample multiplexers A and B.
 *
 * Note             :
 *******************************************************************/
void mTouchCapADC_SetChannelADC(WORD channel_select)
{
    ADC_CHANNEL_SELECT = channel_select;
}

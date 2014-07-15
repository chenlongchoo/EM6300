/******************************************************************************
* File Name:       mTouchCap_Timers.c
* Includes:
*   void InitTimer4(void)
*   void SetPeriodTimer4(unsigned int time)
*   void __ISR(_TIMER_4_VECTOR, IPL2SOFT) Tmr4Handler0(void)
*   void Timer4CallbackFunc(void)
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
*
* Author    Date     Comments
*-------------------------------------------------------------------------
* BDB    26-Jun-2008 First release
* NK     24-Apr-2009 Porting for 18F46J50 Eval Board
* MC     22-Jan-2010 First release of TMRDriver.c, TMRDriver.h
*                    Porting for PIC32MX795F512H
* MWM    17 May 2011 Modified algorithm to implement Dieter Peter's approach.
******************************************************************************/
#include "config.h"
#include "mTouchCap_Timers.h"
#include "mTouchCap_CvdAPI.h"
#include "p32xxxx.h"

// Check that part supports this differential measurement approach
#if defined(__32MX534F064H__) || \
    defined(__32MX534F064L__) || \
    defined(__32MX564F064H__) || \
    defined(__32MX564F064L__) || \
    defined(__32MX564F128H__) || \
    defined(__32MX564F128L__) || \
    defined(__32MX664F064H__) || \
    defined(__32MX664F064L__) || \
    defined(__32MX664F128H__) || \
    defined(__32MX664F128L__) || \
    defined(__32MX764F128H__) || \
    defined(__32MX764F128L__)
#   warning('This part supports using mTouchCap_Timers.c. Try it instead.')
#endif

/* timer 4 callback ISR local - init in Initialization function */
static void *T4CallBackISR;

// from PIC32MX CVD Library\mTouchCap_CvdAPI.c
extern  UINT8 HF_Read_Number;                 //# of current HF scan
extern UINT16 ScanChannels[MAX_ADC_CHANNELS]; //ADC channels for each button
extern  UINT8 NumberScanChannels;             // store the index of the channels that has to be scanned
extern  UINT8 dataReadyCVD;                   //global indicating the reading of all channels finished
extern  UINT8 button_set_number;              //# of current button in scan

// from PIC32MX CVD Library\mTouchCap_PIC32MX_CVD_Physical.c
extern UINT32 ButtonCumulativeSum[MAX_ADC_CHANNELS]; // to compute average
#if   defined( UART_DUMP_RAW_COUNTS )
extern UINT16 CurRawData[  NUM_HF_READS][MAX_ADC_CHANNELS]; // Scratch storage for UART dump
#elif defined( UART_DUMP_ALL_COUNTS )
extern UINT16 CurRawData[3*NUM_HF_READS][MAX_ADC_CHANNELS]; // Scratch storage for UART dump
#endif

// Locals
#ifdef LIMIT_SLEW_RATE
static UINT16 OldRawData[MAX_ADC_CHANNELS] = { 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
                                               0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
                                               0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
                                               0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF }; // raw value from previous set of ADC scans
#endif
static UINT32 AD1CHS_Mask; //Used to change CHOSA mux bits,
//Declared as static instead of local to allow in line assembly when needed.

/******************************************************************************
 * Function:       void InitTimer(void)
 *
 * PreCondition:    None
 *
 * Input:           unsigned int time
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Set the period for timer 4 depending on input param time
 *
 * Note:            Stops and restarts the timer when setting the new period
 *****************************************************************************/
void InitTimer4(void)
{

    // set up the timer interrupt with a priority of 2
    IPC4bits.T4IP = 2;
    // interrupt subpriority is 0
    IPC4bits.T4IS = 0;
    // clear the interrupt flag
    IFS0bits.T4IF = 0;

    // initialize the callback pointer with the value passed as argument
    T4CallBackISR = Timer4CallbackFunc3;

    /* enable the interrupt */
    IEC0bits.T4IE = 1;

    TMR4 = 0;
    PR4 = TIMER4_PERIOD;
    // restart the timer
    T4CONbits.TON = 1;

    return;
}

/******************************************************************************
 * Function:       void SetPeriodTimer4(unsigned int time)
 *
 * PreCondition:    None
 *
 * Input:           unsigned int time
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Set the period for timer 4 depending on input param time
 *
 * Note:            Stops and restarts the timer when setting the new period
 *****************************************************************************/
void SetPeriodTimer4(unsigned int time)
{
    // stop the timer
    T4CONbits.TON = 0;
    /* load the time to higher and lower byte of the timer register */
    TMR4 = 0;
    PR4 = time;
    // restart the timer
    T4CONbits.TON = 1;

}

/******************************************************************************
 * Function:       void __ISR(_TIMER_4_VECTOR, IPL2SOFT) Tmr4Handler0(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This is timer 4 interrupt handler - acknoledge and callback
 *
 * Note:            None
 *****************************************************************************/
void __ISR(_TIMER_4_VECTOR, IPL2SOFT) Tmr4Handler0(void)
{
    void (*ptr_fcn)(void);
    /* init local wiht the callback function ptr */
    ptr_fcn = (void (*) ())T4CallBackISR;
    if (ptr_fcn != 0)
    {
        // call the callback isr
        (*ptr_fcn)();
    }
    /* acknowlege */
    IFS0bits.T4IF = 0;
}

/*
  This version of the callback function replaces the pull-up resistors used in
  TimerCallBackFunc2 with 10 to 30 charges from the ADC's sample and hold (SAH)
  capacitor.  This improves noise performance at the cost of longer execution.
 */
void Timer4CallbackFunc3(void)
{
    UINT16 Vpos, Vneg, Vmeas, Vupdate;
    UINT16 iChan, iChanBit;
    UINT16 iRefChan, iRefBit;

    iRefChan = REF_CHANNEL;
    iRefBit = 1<<iRefChan;

    iChan = ScanChannels[button_set_number];
    iChanBit = 1<<iChan;

    //Mask shows which bits need changing to toggle between reference and sensor
    AD1CHS_Mask = (iRefChan<<16)^(iChan<<16); //XOR = bits that are different

//*****************************************************************************
// Measure Vpos
//*****************************************************************************
    //Bring up ADC in known state
    AD1CON1bits.SAMP = 1;        //Start Chold charging Process
    LATBCLR  = iRefBit;          //Ref pin now low
    TRISBCLR = iRefBit;          //Make reference pin output
    AD1CHSbits.CH0SA = iRefChan; //Switch ADC to grounded reference pin

    LATBSET  = iRefBit; //Make reference pin high

    TRISBSET = iChanBit; //Tri-state sensor port
    LATBCLR  = iChanBit; //Sensor port low

    //This charge/discharge cycle is necessary to start the process
    AD1CHSbits.CH0SA = iRefChan; //Charge ADC Sample and Hold capacitor
    TRISBSET = iChanBit;         //Tri-state sensor port pin again
    AD1CHSINV = AD1CHS_Mask;     //Switch to the sensor to charge button

#   if defined(CHARGE_DISCHARGE_CYCLE_2)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button
#   endif//defined(CHARGE_DISCHARGE_CYCLE_2)

#   if defined(CHARGE_DISCHARGE_CYCLE_3)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button
#   endif//defined(CHARGE_DISCHARGE_CYCLE_3)

#   if defined(CHARGE_DISCHARGE_CYCLE_3)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button
#   endif//defined(CHARGE_DISCHARGE_CYCLE_4)

    AD1CON1bits.SAMP = 0; //Start ADC conversion

    //Wait for ADC conversion complete
    while ( !AD1CON1bits.DONE )
    {
        //Do Nothing
    }
    AD1CON1bits.DONE = 0;

    Vpos = ReadADC10(ReadActiveBufferADC10())<<5;  //ADC measurement

//*****************************************************************************
// Measure Vneg
//*****************************************************************************
#if !defined(USE_DIFFERENTIAL_MEASUREMENTS)
#   error('This file supports differential measurements only!  Use mTouchCap_Timers.c instead.')
#endif//!defined(USE_DIFFERENTIAL_MEASUREMENTS)

    AD1CON1bits.SAMP=1; //Enable sampling again

// Repeat charging to bring button capacitor to near Vdd in voltage, using ADC
#if defined(BUTTON_CHARGE_CYCLE_1)
// Charge cycles 1 - 10
    // Button charge #01
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #02
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #03
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #04
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #05
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #06
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #07
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #08
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #09
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #10
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

#endif//defined(BUTTON_CHARGE_CYCLE_1)

#if defined(BUTTON_CHARGE_CYCLE_2)
// Charge cycles 11 - 20
    // Button Charge #11
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #12
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #13
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #14
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #15
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #16
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #17
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #18
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #19
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #20
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

#endif//defined(BUTTON_CHARGE_CYCLE_2)

#if defined(BUTTON_CHARGE_CYCLE_3)
// Charge cycles 21 - 30
    // Button Charge #21
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #22
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #23
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #24
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #25
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #26
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #27
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #28
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #29
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #30
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

#endif//defined(BUTTON_CHARGE_CYCLE_3)

#if defined(BUTTON_CHARGE_CYCLE_4)
// Charge cycles 31 - 40
    // Button Charge #31
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #32
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #33
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #34
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #35
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #36
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #37
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #38
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button Charge #39
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

    // Button charge #40
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to charge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge button

#endif//defined(BUTTON_CHARGE_CYCLE_4)

//Button cap is charged, now start dumping charge with ADC's SAH cap
    LATBCLR  = iRefBit; //Make reference pin low

#   if defined(CHARGE_DISCHARGE_CYCLE_1)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to discharge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge SAH cap
#   endif//defined(CHARGE_DISCHARGE_CYCLE_1)

#   if defined(CHARGE_DISCHARGE_CYCLE_2)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to discharge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge SAH cap
#   endif//defined(CHARGE_DISCHARGE_CYCLE_2)

#   if defined(CHARGE_DISCHARGE_CYCLE_3)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to discharge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge SAH cap
#   endif//defined(CHARGE_DISCHARGE_CYCLE_3)

#   if defined(CHARGE_DISCHARGE_CYCLE_4)
    AD1CHSINV = AD1CHS_Mask; //Switch to reference to discharge SAH cap
    AD1CHSINV = AD1CHS_Mask; //Switch to the sensor to charge SAH cap
#   endif//defined(CHARGE_DISCHARGE_CYCLE_4)

    AD1CON1bits.SAMP = 0; //Start ADC conversion

    //Wait for ADC conversion complete
    while ( !AD1CON1bits.DONE )
    {
        // Do Nothing
    }
    AD1CON1bits.DONE = 0;
    TRISBCLR = iChanBit; //Remove tri-state on sensor port pin, make it output

//  Read new Vneg
    Vneg = ReadADC10(ReadActiveBufferADC10())<<5;

//  Calculate new Vmeas
    Vmeas = Vpos + 32768 - Vneg;

#if defined(LIMIT_SLEW_RATE)
#   define VMEAS_DELTA (1<<(5))
    if     (Vmeas > OldRawData[button_set_number])
    {
       OldRawData[button_set_number] += VMEAS_DELTA;
    }
    else if(Vmeas < OldRawData[button_set_number])
    {
       OldRawData[button_set_number] -= VMEAS_DELTA;
    }
    Vupdate = OldRawData[button_set_number];
#else
    Vupdate = Vmeas;
#endif//defined(LIMIT_SLEW_RATE)

    ButtonCumulativeSum[button_set_number] += Vupdate;

#if defined( UART_DUMP_RAW_COUNTS )
    CurRawData[  HF_Read_Number  ][button_set_number] = Vmeas;
#elif defined( UART_DUMP_ALL_COUNTS )
    CurRawData[3*HF_Read_Number+0][button_set_number] = Vpos;
    CurRawData[3*HF_Read_Number+1][button_set_number] = Vneg;
    CurRawData[3*HF_Read_Number+2][button_set_number] = Vmeas;
#endif//defined( UART_DUMP_RAW_COUNTS )


    //Next Measurement??
    button_set_number++; //Increment the button number to move on to the next button
    if(button_set_number == NumberScanChannels) //if scanned all the buttons
    {
        HF_Read_Number++;      //Start the next set of HF Scans
        button_set_number = 0; //Start over at the first button
        if ( HF_Read_Number < NUM_HF_READS ) // Not done with scans
        {
            dataReadyCVD = FALSE;
        }
        else  //All HF scans done, compute average, signal we're done
        {
            dataReadyCVD = TRUE; // We're done with complete scan
            HF_Read_Number = 0;  // Start next set of HF scans
        }
    }
    else
    {
        dataReadyCVD = FALSE;
    }

    return; // we're done!
}

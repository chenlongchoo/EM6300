/******************************************************************************

  TMRDriver.h (Header File)

Description:
    This file contains function prototypes that are used in TMRDriver.c
******************************************************************************/

/******************************************************************************
* FileName:        TMRDriver.h
* Dependencies:    See included files, below.
* Processor:       PIC32MX795F512H
* Compiler:        C32
* Company:         Microchip Technology, Inc.
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
*
* Change History:
* Author          Date       Comments
*------------------------------------------------------------------------------
* BDB           26-Jun-2008 Initial release
* NK            24-Apr-2009 Porting for 18F46J50 Eval Board
* MC            22-Ian-2010 First release of TMRDriver.c, TMRDriver.h
*                           Porting for PIC32MX795F512H
*
*******************************************************************************/
#ifndef __MTOUCHCAP_TIMER_H
#define __MTOUCHCAP_TIMER_H

//==========================================================
//--- PROTOTYPES
//==============================
void Timer4CallbackFunc1(void); //found in mTouchCap_Timers.c
void Timer4CallbackFunc2(void); //found in mTouchCap_Timers_Rev2.c
void Timer4CallbackFunc3(void); //found in mTouchCap_Timers_Rev3.c
void InitTimer4(void);
void SetPeriodTimer4(unsigned int time);

/************************************************************************
Macro       : Set_Timer4_IF_Bit_State(State)
Overview    : setting or clearing T4 Interrupt Flag Bit
Parameters  : ENABLE '1' or DISABLE '0'
Remarks     : None.
**************************************************************************/

#define Set_ScanTimer_IF_Bit_State(State)               (IFS0bits.T4IF = State)


/************************************************************************
Macro       : Set_T4_IE_Bit_State(State)
Overview    : setting or clearing T4 Interrupt Enable Bit
Parameters  : ENABLE '1' or DISABLE '0'
Remarks     : None.
**************************************************************************/

#define Set_ScanTimer_IE_Bit_State(State)               (IEC0bits.T4IE = State)


/************************************************************************
Macro       : Set_T4_ON_Bit_State(State)
Overview    : setting or clearing T4 ON bit
Parameters  : ENABLE '1' or DISABLE '0'
Remarks     : None.
**************************************************************************/

#define Set_ScanTimer_ON_Bit_State(State)               (T4CONbits.TON = State)

#endif //end of __MTOUCHCAP_TIMER_H

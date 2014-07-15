/*************************************************************************
 *  © 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD_HardwareProfile_12F61x.h
 *  Dependencies:    mTouchCVD.h
 *  Processor:       See documentation for supported PIC® microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB® IDE v8.50 (or later) or MPLAB® X                        
 *  Hardware:         
 *  Company:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     mTouch CVD Framework Hardware Profile Header File
 *                   - Provides information about correct SFR addresses,
 *                     pin assignments, module availability, and any other
 *                     part-specific data.
 *                   - See the documentation for more information.
 *************************************************************************/
 /**************************************************************************
 * MICROCHIP SOFTWARE NOTICE AND DISCLAIMER: You may use this software, and 
 * any derivatives created by any person or entity by or on your behalf, 
 * exclusively with Microchip's products in accordance with applicable
 * software license terms and conditions, a copy of which is provided for
 * your referencein accompanying documentation. Microchip and its licensors 
 * retain all ownership and intellectual property rights in the 
 * accompanying software and in all derivatives hereto. 
 * 
 * This software and any accompanying information is for suggestion only. 
 * It does not modify Microchip's standard warranty for its products. You 
 * agree that you are solely responsible for testing the software and 
 * determining its suitability. Microchip has no obligation to modify, 
 * test, certify, or support the software. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE, ITS INTERACTION WITH 
 * MICROCHIP'S PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY 
 * APPLICATION. 
 * 
 * IN NO EVENT, WILL MICROCHIP BE LIABLE, WHETHER IN CONTRACT, WARRANTY, 
 * TORT (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), STRICT 
 * LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, 
 * SPECIAL, PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, 
 * FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, 
 * HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY 
 * OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWABLE BY LAW, 
 * MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS 
 * SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID 
 * DIRECTLY TO MICROCHIP FOR THIS SOFTWARE. 
 * 
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF 
 * THESE TERMS. 
 *************************************************************************/
/** @file mTouchCVD_HardwareProfile_12F61x.h
* @brief Support for: PIC12F615, PIC12HV615, PIC12F617
*/
/// @cond
#if !defined(__MTOUCHCVD_HARDWARE_12F61X_H)
#define __MTOUCHCVD_HARDWARE_12F61X_H

#if !defined(_12F615) && !defined(_12HV615)
#if !defined(_12F617) 
    #error The current mTouch hardware include file does not support this PIC microcontroller.
#endif
#endif

#if defined(_12F615) || defined(_12HV615)
    #if CVD_NUMBER_SENSORS > 2
        #error The PIC12F/HV615 is not able to support more than 2 sensors on the mTouch Framework due to memory limitations.
    #endif
    #if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
        #error The PIC12F/HV615 is not able to support debug communications at this time. 
    #endif
#endif
#if defined(_12F617)
    #if CVD_NUMBER_SENSORS > 4
        #error The PIC12F617 is not able to support more than 4 sensors due to pinout limitations.
    #endif
#endif

// No UART
//#define CVD_DEBUG_UART_ENABLED

#define CVD_DEBUG_COMM_INIT()                                       \
    do {                                                            \
    } while (0)

#define CVD_DEBUG_COMM_TXIF             TXIF
#define CVD_DEBUG_COMM_TXREG            TXREG

#define CVD_SET_TMR0IE()                TMR0IE = 1
#define CVD_SET_GIE()                   GIE    = 1
    
#define CVD_SET_ADC_CLK_8MHZ()          ANSEL = 0b01010000 // 16
#define CVD_SET_ADC_CLK_4MHZ()          ANSEL = 0b00010000 // 8
#define CVD_SET_ADC_CLK_1MHZ()          ANSEL = 0b00000000 // 2

#if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
#if !(CVD_DEBUG_SPEED == 9600 || CVD_DEBUG_SPEED == 115200)
    #error CVD_DEBUG_SPEED must be set to either 9600 or 115200 in the configuration file.
#endif
#endif

#if     _XTAL_FREQ == 8000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_8MHZ()   
    #define CVD_TAD             2 
#elif   _XTAL_FREQ == 4000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_4MHZ()    
    #define CVD_TAD             2
#elif   _XTAL_FREQ == 1000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_1MHZ()    
    #define CVD_TAD             2
#else
    #error Chosen Fosc frequency is not supported by the mTouch framework on this device.
#endif

#define CVD_ADCON0_BANK     0
#define CVD_VRCON_BANK      0
#define CVD_UNIMPLEMENTED_AVAILABLE     1
#define CVD_VRCON_MAX       0x8F
#define CVD_VRCON_MIN       0xB0
    
// A/D MUX selection for each A/D button, do not change this, refer to datasheet if curious how these are derived
// Right Justified, Vdd as reference, A/D on, Go/Done  asserted, do not change this, refer to datasheet if curious how these are derived 
#define	CVD_AD_AN0	        0x81	
#define	CVD_AD_AN1	        0x85
#define	CVD_AD_AN2	        0x89
#define	CVD_AD_AN3	        0x8D

#define CVD_AD_CVREF_NOGO   0x91
#define CVD_AD_CVREF_AND_GO 0x93
#define CVD_AD_ISO_AND_GO   0x9F // Selects an unimplemented, isolated ADC mux option and sets the GO/DONE bit.
#define CVD_AD_ISO_NOGO     0x9D // Selects an unimplemented, isolated ADC mux option w/o setting the GO/DONE bit.

#define CVD_SELECT_SENSOR0	    __paste(CVD_AD_, CVD_SENSOR0)
#define CVD_SELECT_SENSOR1	    __paste(CVD_AD_, CVD_SENSOR1)
#define CVD_SELECT_SENSOR2	    __paste(CVD_AD_, CVD_SENSOR2)
#define CVD_SELECT_SENSOR3	    __paste(CVD_AD_, CVD_SENSOR3)
#define CVD_SELECT_CVREF_AND_GO __paste(CVD_AD_, CVREF_AND_GO)
#define CVD_SELECT_CVREF_NOGO   __paste(CVD_AD_, CVREF_NOGO)
#define CVD_SELECT_ISO_AND_GO   __paste(CVD_AD_, ISO_AND_GO) 
#define CVD_SELECT_ISO_NOGO     __paste(CVD_AD_, ISO_NOGO) 
#define CVD_SELECT_REFERENCE    __paste(CVD_AD_, CVD_REFERENCE)
	
#define CVD_PIN_AN0		0
#define CVD_PIN_AN1		1
#define CVD_PIN_AN2		2
#define CVD_PIN_AN3		4

#define CVD_PIN_SENSOR0     __paste(CVD_PIN_,CVD_SENSOR0)
#define CVD_PIN_SENSOR1     __paste(CVD_PIN_,CVD_SENSOR1)
#define CVD_PIN_SENSOR2     __paste(CVD_PIN_,CVD_SENSOR2)
#define CVD_PIN_SENSOR3     __paste(CVD_PIN_,CVD_SENSOR3)
#define CVD_PIN_REFERENCE   __paste(CVD_PIN_,CVD_REFERENCE)

#define CVD_PORT_AN0 	_GPIO
#define CVD_PORT_AN1 	_GPIO
#define CVD_PORT_AN2 	_GPIO	
#define CVD_PORT_AN3 	_GPIO

#define CVD_PORT_SENSOR0    __paste(CVD_PORT_,CVD_SENSOR0)
#define CVD_PORT_SENSOR1    __paste(CVD_PORT_,CVD_SENSOR1)
#define CVD_PORT_SENSOR2    __paste(CVD_PORT_,CVD_SENSOR2)
#define CVD_PORT_SENSOR3    __paste(CVD_PORT_,CVD_SENSOR3)
#define CVD_PORT_REFERENCE  __paste(CVD_PORT_,CVD_REFERENCE)

#endif
/// @endcond

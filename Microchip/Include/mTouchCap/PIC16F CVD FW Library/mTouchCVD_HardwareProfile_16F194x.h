/*************************************************************************
 *  � 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD_HardwareProfile_16F194x.h
 *  Dependencies:    mTouchCVD.h
 *  Processor:       See documentation for supported PIC� microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB� IDE v8.50 (or later) or MPLAB� X                        
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
/** @file mTouchCVD_HardwareProfile_16F194x.h
* @brief Support for: PIC16F1946, PIC16F1947
*/
/// @cond
#if !defined(__MTOUCHCVD_HARDWARE_16F194X_H)
#define __MTOUCHCVD_HARDWARE_16F194X_H

#if !defined(_16F1946) && !defined(_16LF1946)
#if !defined(_16F1947) && !defined(_16LF1947)
    #error The current mTouch hardware include file does not support this PIC microcontroller.
#endif
#endif

#if defined(_16F1946) || defined(_16LF1946)
    #if CVD_NUMBER_SENSORS > 17
        #error The PIC16F/LF1946 is not able to support more than 17 sensors due to pinout limitations.
    #endif
#endif
#if defined(_16F1947) || defined(_16LF1947)
    #if CVD_NUMBER_SENSORS > 17
        #error The PIC16F/LF1947 is not able to support more than 17 sensors due to pinout limitations.
    #endif
#endif

#define CVD_DEBUG_UART_ENABLED

#define CVD_DEBUG_COMM_INIT()                                       \
    do {                                                            \
        TX1STA       = CVD_DEBUG_TXSTA;                             \
        RC1STA       = CVD_DEBUG_RCSTA;                             \
        BAUD1CON     = CVD_DEBUG_BAUDCON;                           \
        SP1BRGL      = CVD_DEBUG_SPBRGL;                            \
        SP1BRGH      = CVD_DEBUG_SPBRGH;                            \
    } while(0)

#define CVD_DEBUG_COMM_TXIF             TXIF
#define CVD_DEBUG_COMM_TXREG            TX1REG
    
#define CVD_SET_TMR0IE()                TMR0IE = 1
#define CVD_SET_GIE()                   GIE    = 1  
    
#define CVD_SET_ADC_CLK_32MHZ()         ADCON1 = 0b10100000 // 32
#define CVD_SET_ADC_CLK_16MHZ()         ADCON1 = 0b11010000 // 16
#define CVD_SET_ADC_CLK_8MHZ()          ADCON1 = 0b10010000 // 8
#define CVD_SET_ADC_CLK_4MHZ()          ADCON1 = 0b11000000 // 4
#define CVD_SET_ADC_CLK_2MHZ()          ADCON1 = 0b10000000 // 2
#define CVD_SET_ADC_CLK_1MHZ()          ADCON1 = 0b10000000 // 2

#define CVD_UNIMPLEMENTED_AVAILABLE     1

#if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
#if !(CVD_DEBUG_SPEED == 9600 || CVD_DEBUG_SPEED == 115200)
    #error CVD_DEBUG_SPEED must be set to either 9600 or 115200 in the configuration file.
#endif
#endif

#if     _XTAL_FREQ == 32000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_32MHZ() 
    #define CVD_TAD             1     
    #if     CVD_DEBUG_SPEED == 115200
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    68
        #define CVD_DEBUG_SPBRGH    0
    #elif   CVD_DEBUG_SPEED == 9600
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00000000
        #define CVD_DEBUG_SPBRGL    207
        #define CVD_DEBUG_SPBRGH    0
    #endif
#elif   _XTAL_FREQ == 16000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_16MHZ()
    #define CVD_TAD             1  
    #if     CVD_DEBUG_SPEED == 115200
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    33
        #define CVD_DEBUG_SPBRGH    0
    #elif   CVD_DEBUG_SPEED == 9600
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00000000
        #define CVD_DEBUG_SPBRGL    103
        #define CVD_DEBUG_SPBRGH    0
    #endif
#elif   _XTAL_FREQ ==  8000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_8MHZ()
    #define CVD_TAD             1  
    #if     CVD_DEBUG_SPEED == 115200
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    16
        #define CVD_DEBUG_SPBRGH    0
    #elif   CVD_DEBUG_SPEED == 9600
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    207
        #define CVD_DEBUG_SPBRGH    0
    #endif
#elif   _XTAL_FREQ ==  4000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_4MHZ()
    #define CVD_TAD             1  
    #if     CVD_DEBUG_SPEED == 115200
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    8
        #define CVD_DEBUG_SPBRGH    0
    #elif   CVD_DEBUG_SPEED == 9600
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    103
        #define CVD_DEBUG_SPBRGH    0
    #endif
#elif   _XTAL_FREQ ==  2000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_2MHZ()
    #define CVD_TAD             1  
    #if     CVD_DEBUG_SPEED == 115200
        #error The 115.2kbps UART baudrate option cannot be used with a 2MHz Fosc. Please select '9600'.
    #elif   CVD_DEBUG_SPEED == 9600
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    51
        #define CVD_DEBUG_SPBRGH    0
    #endif
#elif   _XTAL_FREQ ==  1000000 
    #define CVD_SET_ADC_CLK()   CVD_SET_ADC_CLK_1MHZ()
    #define CVD_TAD             2
    #if     CVD_DEBUG_SPEED == 115200
        #error The 115.2kbps UART baudrate option cannot be used with a 1MHz Fosc. Please select '9600'.
    #elif   CVD_DEBUG_SPEED == 9600
        #define CVD_DEBUG_TXSTA     0b00100100
        #define CVD_DEBUG_RCSTA     0b10010000
        #define CVD_DEBUG_BAUDCON   0b00001000
        #define CVD_DEBUG_SPBRGL    25
        #define CVD_DEBUG_SPBRGH    0
    #endif
#else
    #error _XTAL_FREQ is not set to a valid value for this processor.
#endif

#define CVD_ADCON0_BANK     1
#define CVD_DACCON0_VDD         0xC0
#define CVD_DACCON1_VDD         0x1F
#define CVD_DACCON0_VSS         0x80
#define CVD_DACCON1_VSS         0x00

// A/D MUX selection for each A/D button, do not change this, refer to datasheet if curious how these are derived
// Right Justified, Vdd as reference, A/D on, Go/Done  asserted, do not change this, refer to datasheet if curious how these are derived 
#define	CVD_AD_AN0	        0x01	
#define	CVD_AD_AN1	        0x05
#define	CVD_AD_AN2	        0x09
#define	CVD_AD_AN3	        0x0D
#define CVD_AD_AN4	        0x11
#define CVD_AD_AN5	        0x14
#define CVD_AD_AN6	        0x19
#define CVD_AD_AN7	        0x1D
#define CVD_AD_AN8	        0x21
#define CVD_AD_AN9	        0x24
#define CVD_AD_AN10         0x29
#define CVD_AD_AN11         0x2D
#define CVD_AD_AN12         0x31
#define CVD_AD_AN13         0x34
#define CVD_AD_AN14         0x39
#define CVD_AD_AN15         0x3D
#define CVD_AD_AN16         0x41

#define CVD_AD_FVR_AND_GO   0x7F // Selects the FVR as the ADC mux option and sets the GO/DONE bit.
#define CVD_AD_DAC_AND_GO   0x7B // Selects the DAC as the ADC mux option and sets the GO/DONE bit.
#define CVD_AD_DAC_NOGO     0x79 // Selects the DAC as the ADC mux option and sets the GO/DONE bit.
#define CVD_AD_ISO_AND_GO   0x47 // Selects an unimplemented, isolated ADC mux option and sets the GO/DONE bit.
#define CVD_AD_ISO_NOGO     0x45 // Selects an unimplemented, isolated ADC mux option w/o setting the GO/DONE bit.

//====================================================================
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT    
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ 
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __|
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |   
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |   
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |   
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|   
//
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT 
//====================================================================

#define CVD_SELECT_SENSOR0	    __paste(CVD_AD_, CVD_SENSOR0)
#define CVD_SELECT_SENSOR1	    __paste(CVD_AD_, CVD_SENSOR1)
#define CVD_SELECT_SENSOR2	    __paste(CVD_AD_, CVD_SENSOR2)
#define CVD_SELECT_SENSOR3	    __paste(CVD_AD_, CVD_SENSOR3)
#define CVD_SELECT_SENSOR4	    __paste(CVD_AD_, CVD_SENSOR4)
#define CVD_SELECT_SENSOR5	    __paste(CVD_AD_, CVD_SENSOR5)
#define CVD_SELECT_SENSOR6	    __paste(CVD_AD_, CVD_SENSOR6)
#define CVD_SELECT_SENSOR7	    __paste(CVD_AD_, CVD_SENSOR7)
#define CVD_SELECT_SENSOR8	    __paste(CVD_AD_, CVD_SENSOR8)
#define CVD_SELECT_SENSOR9	    __paste(CVD_AD_, CVD_SENSOR9)
#define CVD_SELECT_SENSOR10     __paste(CVD_AD_, CVD_SENSOR10)
#define CVD_SELECT_SENSOR11     __paste(CVD_AD_, CVD_SENSOR11)
#define CVD_SELECT_SENSOR12     __paste(CVD_AD_, CVD_SENSOR12)
#define CVD_SELECT_SENSOR13     __paste(CVD_AD_, CVD_SENSOR13)
#define CVD_SELECT_SENSOR14     __paste(CVD_AD_, CVD_SENSOR14)
#define CVD_SELECT_SENSOR15     __paste(CVD_AD_, CVD_SENSOR15)
#define CVD_SELECT_SENSOR16     __paste(CVD_AD_, CVD_SENSOR16)
#define CVD_SELECT_FVR_AND_GO   __paste(CVD_AD_, FVR_AND_GO)
#define CVD_SELECT_DAC_AND_GO   __paste(CVD_AD_, DAC_AND_GO)
#define CVD_SELECT_DAC_NOGO     __paste(CVD_AD_, DAC_NOGO)
#define CVD_SELECT_ISO_AND_GO   __paste(CVD_AD_, ISO_AND_GO) 
#define CVD_SELECT_ISO_NOGO     __paste(CVD_AD_, ISO_NOGO) 
#define CVD_SELECT_REFERENCE    __paste(CVD_AD_, CVD_REFERENCE)

//====================================================================
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT    
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ 
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __|
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |   
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |   
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |   
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|   
//
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT 
//====================================================================
	
#define CVD_PIN_AN0		0
#define CVD_PIN_AN1		1
#define CVD_PIN_AN2		2
#define CVD_PIN_AN3		3
#define CVD_PIN_AN4		5
#define	CVD_PIN_AN5	    7        
#define	CVD_PIN_AN6	    1       
#define	CVD_PIN_AN7	    2        
#define CVD_PIN_AN8		3	
#define CVD_PIN_AN9		4
#define CVD_PIN_AN10	5
#define CVD_PIN_AN11	6
#define CVD_PIN_AN12	4
#define CVD_PIN_AN13	3
#define CVD_PIN_AN14	2
#define CVD_PIN_AN15	1
#define CVD_PIN_AN16	0

//====================================================================
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT    
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ 
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __|
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |   
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |   
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |   
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|   
//
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT 
//====================================================================

#define CVD_PIN_SENSOR0     __paste(CVD_PIN_,CVD_SENSOR0)
#define CVD_PIN_SENSOR1     __paste(CVD_PIN_,CVD_SENSOR1)
#define CVD_PIN_SENSOR2     __paste(CVD_PIN_,CVD_SENSOR2)
#define CVD_PIN_SENSOR3     __paste(CVD_PIN_,CVD_SENSOR3)
#define CVD_PIN_SENSOR4     __paste(CVD_PIN_,CVD_SENSOR4)
#define CVD_PIN_SENSOR5     __paste(CVD_PIN_,CVD_SENSOR5)
#define CVD_PIN_SENSOR6     __paste(CVD_PIN_,CVD_SENSOR6)
#define CVD_PIN_SENSOR7     __paste(CVD_PIN_,CVD_SENSOR7)
#define CVD_PIN_SENSOR8     __paste(CVD_PIN_,CVD_SENSOR8)
#define CVD_PIN_SENSOR9     __paste(CVD_PIN_,CVD_SENSOR9)
#define CVD_PIN_SENSOR10    __paste(CVD_PIN_,CVD_SENSOR10)
#define CVD_PIN_SENSOR11    __paste(CVD_PIN_,CVD_SENSOR11)
#define CVD_PIN_SENSOR12    __paste(CVD_PIN_,CVD_SENSOR12)
#define CVD_PIN_SENSOR13    __paste(CVD_PIN_,CVD_SENSOR13)
#define CVD_PIN_SENSOR14    __paste(CVD_PIN_,CVD_SENSOR14)
#define CVD_PIN_SENSOR15    __paste(CVD_PIN_,CVD_SENSOR15)
#define CVD_PIN_SENSOR16    __paste(CVD_PIN_,CVD_SENSOR16)
#define CVD_PIN_REFERENCE   __paste(CVD_PIN_,CVD_REFERENCE)

//====================================================================
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT    
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ 
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __|
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |   
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |   
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |   
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|   
//
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT 
//====================================================================

#define CVD_PORT_AN0 	_PORTA	
#define CVD_PORT_AN1 	_PORTA	
#define CVD_PORT_AN2 	_PORTA	
#define CVD_PORT_AN3 	_PORTA
#define CVD_PORT_AN4 	_PORTA
#define CVD_PORT_AN5 	_PORTF	
#define CVD_PORT_AN6 	_PORTF	
#define CVD_PORT_AN7 	_PORTF	
#define CVD_PORT_AN8 	_PORTF
#define CVD_PORT_AN9 	_PORTF
#define CVD_PORT_AN10	_PORTF	
#define CVD_PORT_AN11	_PORTF	
#define CVD_PORT_AN12	_PORTG	
#define CVD_PORT_AN13	_PORTG
#define CVD_PORT_AN14   _PORTG
#define CVD_PORT_AN15   _PORTG
#define CVD_PORT_AN16   _PORTF

//====================================================================
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT    
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ 
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __|
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |   
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |   
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |   
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|   
//
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT 
//====================================================================

#define CVD_PORT_SENSOR0    __paste(CVD_PORT_,CVD_SENSOR0)
#define CVD_PORT_SENSOR1    __paste(CVD_PORT_,CVD_SENSOR1)
#define CVD_PORT_SENSOR2    __paste(CVD_PORT_,CVD_SENSOR2)
#define CVD_PORT_SENSOR3    __paste(CVD_PORT_,CVD_SENSOR3)
#define CVD_PORT_SENSOR4    __paste(CVD_PORT_,CVD_SENSOR4)
#define CVD_PORT_SENSOR5    __paste(CVD_PORT_,CVD_SENSOR5)
#define CVD_PORT_SENSOR6    __paste(CVD_PORT_,CVD_SENSOR6)
#define CVD_PORT_SENSOR7    __paste(CVD_PORT_,CVD_SENSOR7)
#define CVD_PORT_SENSOR8    __paste(CVD_PORT_,CVD_SENSOR8)
#define CVD_PORT_SENSOR9    __paste(CVD_PORT_,CVD_SENSOR9)
#define CVD_PORT_SENSOR10    __paste(CVD_PORT_,CVD_SENSOR10)
#define CVD_PORT_SENSOR11    __paste(CVD_PORT_,CVD_SENSOR11)
#define CVD_PORT_SENSOR12    __paste(CVD_PORT_,CVD_SENSOR12)
#define CVD_PORT_SENSOR13    __paste(CVD_PORT_,CVD_SENSOR13)
#define CVD_PORT_SENSOR14    __paste(CVD_PORT_,CVD_SENSOR14)
#define CVD_PORT_SENSOR15    __paste(CVD_PORT_,CVD_SENSOR15)
#define CVD_PORT_SENSOR16    __paste(CVD_PORT_,CVD_SENSOR16)
#define CVD_PORT_REFERENCE  __paste(CVD_PORT_,CVD_REFERENCE)

//====================================================================
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT    
//   _____   ____    _   _  ____ _______   ______ _____ _____ _______ 
//  |  __ \ / __ \  | \ | |/ __ \__   __| |  ____|  __ \_   _|__   __|
//  | |  | | |  | | |  \| | |  | | | |    | |__  | |  | || |    | |   
//  | |  | | |  | | | . ` | |  | | | |    |  __| | |  | || |    | |   
//  | |__| | |__| | | |\  | |__| | | |    | |____| |__| || |_   | |   
//  |_____/ \____/  |_| \_|\____/  |_|    |______|_____/_____|  |_|   
//
// DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT - DO NOT EDIT
//==================================================================== 

#endif
/// @endcond


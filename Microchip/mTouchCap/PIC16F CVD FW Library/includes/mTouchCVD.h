/*************************************************************************
 *  © 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD.h
 *  Dependencies:    mTouchCVD_Config.h
 *                   includes/mTouchCVD_HardwareProfile_*******.h
 *                      (Based on current PIC® configuration)
 *                   includes/mTouchCVD_macroLibrary_PIC****.h
 *                      (Based on current PIC® processor core)
 *                   See documentation for more information about which
 *                   files are required for compiling.
 *
 *  Processor:       See documentation for supported PIC® microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB® IDE v8.50 (or later) or MPLAB® X                        
 *  Hardware:         
 *  Company:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     mTouch CVD Framework Include File
 *                   - No application code should be implemented in this
 *                     or any other mTouch framework file. This will allow
 *                     for better customer support and easier upgrades to
 *                     later firmware versions. Use the main.c and user-
 *                     generated files to implement your application.
 *                   - See the documentation located in the docs/ folder
 *                     for a more information about how the framework is
 *                     implemented.
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
/** @file mTouchCVD.h
* @brief Main framework header file used by all mTouchCVD files.
*
* Also, performs setup and error checking operations at compile time.
*/
#ifndef __MTOUCHCVD_H
#define __MTOUCHCVD_H

///////////////////////////////
// INCLUDES 
///////////////////////////////
#include <htc.h>
#include <includes/mTouchCVD_Config.h>
 
///////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////
extern       unsigned char  mTouchCVD_dataReady;                        ///< GLOBAL: Flag to determine if new data is ready for processing
extern       unsigned char  ButtonState         [CVD_NUMBER_SENSORS];   ///< GLOBAL: Tracks the current state of the sensor
extern       unsigned int   Average             [CVD_NUMBER_SENSORS];   ///< GLOBAL: Tracks environmental changes in the system
extern       unsigned int   CVDSensorData       [CVD_NUMBER_SENSORS];   ///< GLOBAL: Latch register storing the latest acquisition data
extern       unsigned int   sensor_data         [CVD_NUMBER_SENSORS];   ///< GLOBAL: Internal mTouch acquisition variable - do not modify

///////////////////////////////
// ENUMERATIONS
///////////////////////////////

/// GLOBAL: ButtonState valid values
extern  enum    CVD_ButtonState 
                {
                    CVD_INITIALIZING,   ///< Sensor is still initializing
                    CVD_RELEASED,       ///< Sensor is currently released
                    CVD_PRESSED         ///< Sensor is currently pressed
                };    

///////////////////////////////
// GLOBAL PROTOTYPES
///////////////////////////////
void    CVD_Decode              (void);
void    mTouchCVD_Init          (void);
void    CVD_Filter_UpdateAverage(unsigned int, unsigned char);
void    CVD_Filter_Init         (void);

// Communication Functions
#if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
    void    mTouchCVD_Comm_Init     (void);
    void    send_Data               (void);
#endif

///////////////////////////////
// MACROS
///////////////////////////////

#define     CVD_GetButtonState(i)           ButtonState[i]                          ///< Abstraction macro for the current button state
#define     mTouchCVD_Service()             mTouchCVD_dataReady = 0; CVD_Decode()   ///< Abstraction macro for servicing new mTouch data
#define     mTouchCVD_isDataReady()         mTouchCVD_dataReady                     ///< Abstraction macro for checking if new data is ready for processing
#define     CVD_SetButtonState(state, i)    ButtonState[i] = state                  ///< Abstraction macro for changing button state
#define     CVD_GetAverage(i)               Average[i]                              ///< Abstraction macro for array access
#define     CVDGetSensor(i)                 CVDSensorData[i]                        ///< Abstraction macro for array access                     


//////////////////////////////////////////////////////////////////////
//                                                                  //
//                   PREPROCESSOR FRAMEWORK CODE                    //
//                                                                  //
//  This code assigns the correct hardware include file for your    //
//  processor, defines a few important macros for the system, and   //
//  checks to make sure that the configuration file is set with     //
//  all valid options.                                              //
//                                                                  //
//====================================================================
/// @cond

// Include the hardware profile header file that corresponds to the currently selected PIC microcontroller.
#if     defined(_16F1827) || defined(_16LF1827) || defined(_16F1826) || defined(_16LF1826) || defined(_16F1823) || defined(_16LF1823) || defined(_12F1822) || defined(_12LF1822) || defined(_16F1824) || defined(_16LF1824) || defined(_16F1825) || defined(_16LF1825) || defined(_16F1828) || defined(_16LF1828) || defined(_16F1829) || defined(_16LF1829)
    #include <includes/mTouchCVD_HardwareProfile_16F182x.h>
    #define _16F182x
#elif   defined(_16F1933) || defined(_16LF1933) || defined(_16F1934) || defined(_16LF1934) || defined(_16F1936) || defined(_16LF1936) || defined(_16F1937) || defined(_16LF1937) || defined(_16F1938) || defined(_16LF1938) || defined(_16F1939) || defined(_16LF1939)
    #include <includes/mTouchCVD_HardwareProfile_16F193x.h>
    #define _16F193x
#elif   defined(_16F1946) || defined(_16LF1946) || defined(_16F1947) || defined(_16LF1947) 
    #include <includes/mTouchCVD_HardwareProfile_16F194x.h>
    #define _16F194x
#elif   defined(_16F1526) || defined(_16LF1526) || defined(_16F1527) || defined(_16LF1527) 
    #include <includes/mTouchCVD_HardwareProfile_16F152x.h>
    #define _16F152x
#elif   defined(_16F1516) || defined(_16LF1516) || defined(_16F1517) || defined(_16LF1517) || defined(_16F1518) || defined(_16LF1518) || defined(_16F1519) || defined(_16LF1519)
    #include <includes/mTouchCVD_HardwareProfile_16F151x.h> 
    #define _16F151x
#elif   defined(_12F615) || defined(_12HV615) || defined(_12F617)
    #include <includes/mTouchCVD_HardwareProfile_12F61x.h>
    #define _12F61x
#else
    #error The currently chosen processor is not yet supported by the mTouch CVD Framework.
#endif 

// Include the macro library header file that corresponds to the currently selected PIC microcontroller's core.
#if defined(_PIC14E)
    // Enhanced Mid-range
    #include <includes/mTouchCVD_macroLibrary_PIC16F1.h>
#elif defined(_PIC14)
    // Normal Mid-range
    #include <includes/mTouchCVD_macroLibrary_PIC16F.h>
#else
    #error The currently chosen processor core does not have a matching macroLibrary file associated with it.
#endif

// Include the framework library's generic header file to perform checks and definitions common to all microcontrollers.
#include <includes/mTouchCVD_processConfiguration.h>
/// @endcond

#endif



/*************************************************************************
 *  © 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD.c
 *  Dependencies:    mTouchCVD.h
 *  Processor:       See documentation for supported PIC® microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB® IDE v8.50 (or later) or MPLAB® X                        
 *  Hardware:         
 *  Company:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     mTouch CVD Framework filtering and decoding modules
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
/** @file mTouchCVD.c
* @brief This module provides a group of functions performing the post
*  acquisition filtering and decoding for all the sensors.
*/

///////////////////////////////
// INCLUDES 
///////////////////////////////
#include <includes/mTouchCVD.h>
  
///////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////
/** @name Global Sensor Variables
* These variables store information about the current state of the sensors.
* They should only ever be updated by the mTouch framework, but may be read
* globally so that special actions can be taken based on the sensors' status.
*/ 
//@{
 unsigned char       ButtonState   [CVD_NUMBER_SENSORS];    ///< Tracks the current state of each sensor                    @ingroup Decoding
 unsigned int        Average       [CVD_NUMBER_SENSORS];    ///< Tracks environmental changes in the system                 @ingroup Decoding
//@}
///////////////////////////////
// LOCAL VARIABLES
///////////////////////////////
/** @name Local Variables
* These variables are used internally by the filtering and decoding framework modules.
* They should never be accessed outside of the mTouchCVD.c file.
*/ 
//@{
#if defined(AVG_ENABLE)
 unsigned int        AverageCount  [CVD_NUMBER_SENSORS];    ///< Counter variable for updating averages                     @ingroup Decoding
#endif

#if defined(BUTTON_TIMEOUT)
#if BUTTON_TIMEOUT > 0
 unsigned char       PressTimer    [CVD_NUMBER_SENSORS];    ///< Counter variable for button timeout implementation         @ingroup Decoding
#endif
#endif

unsigned char DebounceCount [CVD_NUMBER_SENSORS];           ///< Counter variable for state transition debouncing           @ingroup Decoding
unsigned int  initCounter;                                  ///< Counter variable for initialization                        @ingroup Decoding
//@}
////////////////////////////////////////////////////////////////
//               CREATE THRESHOLD CONSTANT ARRAY              //
//============================================================//
/** @name Constant Threshold Arrays
* These arrays are stored in program memory at compile time, so they do not require
* RAM. The downside to this implementation is that their values cannot be updated at
* runtime. If this is something your application requires, further editing will be
* needed.
*/
//@{
const unsigned int pressThreshold[]     = PRESS_THRESHOLD_INIT;     ///< Constant array containing the press thresholds     @ingroup Decoding
const unsigned int releaseThreshold[] = RELEASE_THRESHOLD_INIT;     ///< Constant array containing the release thresholds   @ingroup Decoding
//@}

//*************************************************************************
// mTouchCVD_Init() - mTouch Framework's Initialization Routine
//*************************************************************************
/** @name Required Initialization Call
* This function must be called at power up in order to correctly configure
* the mTouch framework.
*/
//@{
/**
*   @brief Initializes the mTouch CVD firmware
*   
*   This routine is the main initialization routine for all mTouch CVD 
*   modules including the built-in communication module, if enabled.
*
*   @ingroup mTouchAPI
*/
void mTouchCVD_Init(void)
{
   
    /** @section CVDInit Algorithm Implementation Notes
    * 1. Loop through all sensors and initialize array variables
    */   
    for (unsigned char i = 0; i < CVD_NUMBER_SENSORS; i++)
    {
        ButtonState     [i] = CVD_INITIALIZING;
        DebounceCount   [i] = 0;
        
        #if defined(BUTTON_TIMEOUT)
        #if BUTTON_TIMEOUT > 0
        PressTimer      [i] = 0;
        #endif
        #endif
        
        sensor_data     [i] = 0x47F;
    }
    
    /// 2. Initialize non-array system variables
    initCounter = POWER_UP_SAMPLES;
    
    /// 3. Initialize the ADC module for correct scanning operation
    CVD_SET_ADC_CLK();
    
    /// 4. Initialize the framework's filtering module
    CVD_Filter_Init();
    
    /// 5. Initialize the framework's communication module, if enabled by configuration options
    #if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
    mTouchCVD_Comm_Init();          
    #endif
    
    /// 6. Initialization complete: Enable Interrupts
    CVD_SET_TMR0IE();
    CVD_SET_GIE();
    
    /// <br><br>
    
    /** @section CVDInitRegs Affected Special Function Registers
    * @li ADCON1 or ANSEL (based on PIC) - sets ADC clock
    */
    
    /// <br><br>
}
//@}

//*************************************************************************
// CVD_Filter_Init() - Filtering Module's Initialization Routine
//*************************************************************************
/**
*   @brief Initializes the filtering variables
*   
*   The CVD filtering module's initialization routine initializes the
*   filtering variables.
*   @callergraph
*/
void CVD_Filter_Init(void)
{
    /** @section FilterInit Algorithm Implementation Notes
    * 1. Loop through all sensors and initialize array variables
    */  
    #if defined(AVG_ENABLE)
    for (unsigned char i = 0; i < CVD_NUMBER_SENSORS; i++)
    {        
        Average[i]      = 0;
        AverageCount[i] = 0;        
    } 
    #endif
    
    /// <br><br><br>
}

//*************************************************************************
// CVD_Decode() - Decoding Module's Service Routine
//*************************************************************************
/**
*   @ingroup Decoding
* 
*   @brief Updates the state of the sensor using the current CVDSensorData value
*   
*   This is the service routine that performs the functions of the
*   decoding stage and should only be called once the #mTouchCVD_dataReady
*   variable has been set to a non-0 value. 
*/
void CVD_Decode(void)
{
    /** @section DecodeImp Algorithm Implementation Notes
    * This function loops through each sensor and performs the following algorithm:
    */

    signed int delta;
    
    #if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
    unsigned char areInitialized = 1;
    #endif
    
    //#if defined(_PIC14)
    GIE = 0;        // If non-enhanced core is used, prevent stack overflow by disabling
                    // interrupts during decoding.
    //#endif
    
    // For each active sensor...
    for(unsigned char i = 0; i < CVD_NUMBER_SENSORS; i++)
    {
        /** 
        * 1. Get the current shift amount by comparing the new reading with the average
        * and store it in the variable 'delta'. If delta is negative, clear the result
        * by forcing it to '0'.
        */
        delta   = CVDGetSensor(i) - CVD_GetAverage(i); 

        // If a negative shift is detected, clear delta value.
        if (delta < 0)                         
        {
            delta = 0;  // Shifts should increase.
                        // Setting to 0 helps to prevent triggering
                        // on negative spikes.
        }
        
        /**
        * 2. Based on the sensor's current state, perform one of the following procedures:
        */
        switch(CVD_GetButtonState(i))                           // Begin state machine
        {
            /** @subsection InitState Initialization State
            *
            * This is the default state of all sensors on power up. Once sensors leave the
            * initialization state, they do not return. In this state, it begins by
            * decrementing the #initCounter variable. If #initCounter has not yet reached
            * 0, the average is updated to equal the current reading and a flag is set
            * to stop any communication data from being transmitted. If #initCounter has
            * reached 0: the sensor transitions to the #CVD_RELEASED state, the debounce
            * counter is reset to 0, and the average is updated one final time to be 
            * equal to the current reading's value.
            */
            case CVD_INITIALIZING:    
            {
                if (--initCounter == 0)                         // If initialization delay has elapsed
                {
                    // State Transition -> Released
                    initCounter = 1;                            // Sets the initCounter variable so that
                                                                // all future sensors will be immediately
                                                                // transitioned to 'released'.
                                                                
                    CVD_SetButtonState(CVD_RELEASED,i);         // Sensor is now released and stable                    
                    DebounceCount[i]    = 0;                    // Initialize sensor's debounce counter
                    Average[i] = CVDGetSensor(i);               // Initialize sensor's average
                }
                else                                            // If initialization delay is not complete
                {
                    #if defined(CVD_DEBUG) && (CVD_DEBUG == 1)  // (Only if CVD_DEBUG is enabled)
                    areInitialized = 0;                         // Disable data transfer until initialized
                    #endif
                    Average[i] = CVDGetSensor(i);               // Initialize sensor's average
                }
                break;                                          
            }                                                   
            
            /** @subsection ReleaseState Released State
            * 
            * This state can be entered from either the #CVD_INITIALIZING or #CVD_PRESSED 
            * state. The only way to leave this state is by creating enough consecutive
            * positive shifts to overflow the DebounceCount[i] variable. Once this is done,
            * the sensor will enter the pressed state.<br>
            * The code for this section begins by checking to see if the latest reading
            * is greater than the average by at least the press threshold. If it is, it
            * performs the debouncing logic. Once the debounce count has been reached, the
            * sensor will change to the CVD_PRESSED state, reset the debounce counter, and
            * initialize the press timer (if enabled).<br>
            * If the threshold has not been crossed, the debounce counter is reset and 
            * the average is updated.
            *
            * @note This is the only state and condition where the average is able to be 
            * updated, and we only use the latest reading to update the average once we 
            * know it's not crossing any thresholds.
            */
            case CVD_RELEASED:                                          // Sensor is released and stable
            {                  

                if(delta > pressThreshold[i])                           // If reading has crossed the press threshold
                {
                    if (++DebounceCount[i] > DEBOUNCE_COUNT_PRESS)      // Increment the debounce counter
                    {                                                   // and check if it has crossed its limit
                        // State Transition -> Pressed
                        CVD_SetButtonState(CVD_PRESSED, i);             // Sensor is now pressed
                        DebounceCount[i]    = DEBOUNCE_COUNT_RELEASE;   // Initialize the pressed state's debounce variable
                        
                        #if defined(BUTTON_TIMEOUT)                     // If the press timer is enabled
                        #if BUTTON_TIMEOUT > 0
                        PressTimer[i]       = BUTTON_TIMEOUT;           // Initialize the press timer counter
                        #endif
                        #endif
                    }
                }
                else                                                    // If the reading has not crossed the press threshold
                {
                    DebounceCount[i] = 0;                               // Reset the debounce variable                    
                    CVD_Filter_UpdateAverage(CVDGetSensor(i), i);       // Update the average
                }
                break;
                
            }

            /** @subsection PressState Pressed State
            * 
            * A sensor can only enter this state from the #CVD_RELEASED state, but there
            * are two ways to leave: First, by being in the state for too long and allowing
            * the press timer to run out. Second, by having a the delta value smaller than
            * the release threshold on enough consecutive scans to cause the debounce
            * counter to reach 0. <br>
            * The code for this section begins by updating and checking the press timer
            * counter variable. If it has reached 0, the sensor transfers to the #CVD_RELEASED
            * state, the debounce counter is reset and the average is re-initialized to
            * the current reading. This is done to allow for a quick recovery if some
            * environmental condition caused the sensor to become stuck.<br>
            * Next, the reading is checked against the release threshold. If it has 
            * crossed the threshold, the debouncing and then a state transition to 
            * #CVD_RELEASED is performed. If it has not crossed the threshold, the
            * debounce counter is reset.
            * @note The average is never updated while the sensor is pressed so the system
            * can remember where the unpressed value should end up. Because of this, the
            * average can be better thought of as a released-state average.
            */
            case CVD_PRESSED:
            {

                #if defined(BUTTON_TIMEOUT)                     // (Only if the press timer has been enabled)
                #if BUTTON_TIMEOUT > 0   

                if (--PressTimer[i] <= 0)                       // Decrement the press timer counter
                {                                               // and check if it has reached 0.
                    // State Transition -> Released
                    CVD_SetButtonState(CVD_RELEASED, i);        // Sensor is now released
                    DebounceCount[i] = 0;                       // Initialize the debounce counter for the CVD_PRESSED state
                    Average[i] = CVDGetSensor(i);               // Re-initialize the average to the current reading value
                    break;
                }
                
                #endif
                #endif
            
            
                if(delta < releaseThreshold[i])                 // If the reading has crossed the release threshold
                {
                    if (--DebounceCount[i] == 0)                // Decrement the debounce counter
                    {                                           // and check if it has reached 0.
                        CVD_SetButtonState(CVD_RELEASED, i);    // Sensor is now released
                    }
                }
                else                                            // If the reading has not crossed the release threshold
                {
                    DebounceCount[i] = DEBOUNCE_COUNT_RELEASE;  // Reset the debounce counter
                }
                break;
            }
            
            default: break; 
            
        } // End state machine switch command
    } // End for loop iterating over each sensor
    
    /**
    * 3. Now that all state machine decoding is complete, transmit fresh data information.
    */
    #if defined(CVD_DEBUG) && (CVD_DEBUG == 1)      // (Only if debugging communications are enabled)
    if (areInitialized)                             // Only trasmit data if all sensors are initialized
    {
        send_Data();                                // Transmit data packet using mTouch Communication module
    }
    #endif
    
    //#if defined(_PIC14)
    GIE = 1;                // If non-enhanced core is used, re-enable interrupts.
    //#endif
    
    /// <br><br><br>
} 

//*************************************************************************
// CVD_Filter_UpdateAverage() - Filtering Module
//*************************************************************************
/**
*   @ingroup Decoding
*   @param[in] reading The current reading value of the sensor
*   @param[in] index The index of the sensor to update
*
*   @brief Updates the average for the given sensor
*   
*   Updates a sensor's average (or baseline) in order to track the
*   environmental changes of the system. 
*
*   @callergraph
*/
void CVD_Filter_UpdateAverage(unsigned int reading, unsigned char index)
{
    /** @section UAvgImp Algorithm Implementation Notes
    *
    * This function has several possible implementations based on the current 
    * configuration options of the framework. The important configuration
    * options are:
    * @li #NEGATIVE_CAPACITANCE - determines how the average handles a negative reading shift
    * @li #AVG_RATE - acts as a debounce counter for updating only every other scan, etc
    * @li #AVG_UPDATE - determines what weight to give the reading when updating the average
    *
    * The implementation begins by checking if #NEGATIVE_CAPACITANCE is set to 1.
    * If so, the average counter variable is set so that the average will perform its
    * update regardless of the previous count. If #NEGATIVE_CAPACITANCE is instead 
    * equal to 2, the average is immediately updated to follow the reading and the 
    * function is exited.<br>
    * Next, the average updates the counter variable and compares it with #AVG_RATE
    * to see if it should continue or return. If the counter allows it to continue,
    * the average is then updated using one of four possible implementations
    * which is deteremined by the #AVG_UPDATE option.
    */
    #if     NEGATIVE_CAPACITANCE == 1
    if ((signed int)(reading - Average[index]) < 0)     // If there was a negative shift
    {
        AverageCount[index] = AVG_RATE;                 // Set the average counter to allow an immediate average update
    }
    #elif   NEGATIVE_CAPACITANCE == 2                       
    if ((signed int)(reading - Average[index]) < 0)     // If there was a negative shift
    {
        Average[index] = reading;                       // Re-initialize the average to the current reading value
        return;                                         // and return from the function - not performing the update algorithm
    }
    #endif

    if (++AverageCount[index] >= AVG_RATE)                          // If the average counter has been exceeded
    {
        AverageCount[index] = 0;                                    // Reset the average counter

        #if     (AVG_UPDATE == 2)                                   // Calculate average update
        
            // avg = (avg + reading) / 2   
            Average[index]  += reading;                             
            Average[index] >>= 1;
            
        #elif   (AVG_UPDATE == 4)                                   
        
            // avg = avg - (25% * avg) + (25% * reading)        
            Average[index]  -= (unsigned int)(Average[index] >> 2); 
            Average[index]  += (unsigned int)(reading >> 2);
            
        #elif   (AVG_UPDATE == 8)                                   
        
            // avg = avg - (12.5% * avg) + (12.5% * reading)   
            Average[index]  -= (unsigned int)(Average[index] >> 3); 
            Average[index]  += (unsigned int)(reading >> 3);
            
        #elif   (AVG_UPDATE == 16)                                  
        
            // avg = avg - (6.25% * avg) + (6.25% * reading)
            Average[index]  -= (unsigned int)(Average[index] >> 4); 
            Average[index]  += (unsigned int)(reading >> 4);
            
        #endif
    }
    /// <br><br><br>    
}




/*************************************************************************
 *  © 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD_Config.h
 *  Dependencies:    
 *  Processor:       See documentation for supported PIC® microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB® IDE v8.50 (or later) or MPLAB® X                        
 *  Hardware:         
 *  Company:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     mTouch CVD Framework Configuration File
 *                   - See documentation for better explanations of all
 *                     configuration options.
 *************************************************************************/
/***********************************************************************
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
 /****************************************************************************
 * Revision History
 * Date		Rev	 Author  Comments
 * 09.20.10 1.00         Public Release
 ****************************************************************************/
/** @file mTouchCVD_Config.h
* @brief Main mTouch application configuration file
*/

//**********************************************
//
// System Configuration
//
//**********************************************

                                        //====================================================
#define _XTAL_FREQ      32000000        // REQUIRED: The Fosc frequency of your system in Hz      
#define TMR0_PRESCALER  2               // REQUIRED: The TMR0 prescaler value of your system   


#define CVD_NUMBER_SENSORS          7       // Number of CVD Sensor Inputs

    #define CVD_SENSOR0             AN0
/// @cond    
    #define CVD_SENSOR1             AN1    
    #define CVD_SENSOR2             AN2
    #define CVD_SENSOR3             AN3
    
    #define CVD_SENSOR4             AN4    
    #define CVD_SENSOR5             AN5     
    #define CVD_SENSOR6             AN6  
    #define CVD_SENSOR7             AN7       
    #define CVD_SENSOR8             AN8     
    #define CVD_SENSOR9             AN9     
    #define CVD_SENSOR10            AN10    
    #define CVD_SENSOR11            AN11    
    #define CVD_SENSOR12            AN12    
    #define CVD_SENSOR13            AN13     
    #define CVD_SENSOR14            AN14    
    #define CVD_SENSOR15            AN15   
    #define CVD_SENSOR16            AN16   
    #define CVD_SENSOR17            AN17   
    #define CVD_SENSOR18            AN18   
    #define CVD_SENSOR19            AN19   
    #define CVD_SENSOR20            AN20   
    #define CVD_SENSOR21            AN21   
    #define CVD_SENSOR22            AN22   
    #define CVD_SENSOR23            AN23   
    #define CVD_SENSOR24            AN24   
    #define CVD_SENSOR25            AN25   
    #define CVD_SENSOR26            AN26   
    #define CVD_SENSOR27            AN27   
    #define CVD_SENSOR28            AN28   
    #define CVD_SENSOR29            AN29  
/// @endcond
    
//**********************************************
//
// Typical-Use Configuration Options
//
//**********************************************

// Do not edit these three timing definitions.
#define CVD_100ms_Timing    (unsigned int)(((_XTAL_FREQ / 4) * 0.1)   / (224 * TMR0_PRESCALER * CVD_NUMBER_SENSORS))
#define CVD_20ms_Timing     (unsigned int)(((_XTAL_FREQ / 4) * 0.02)  / (224 * TMR0_PRESCALER * CVD_NUMBER_SENSORS)) 
#define CVD_17ms_Timing     (unsigned int)(((_XTAL_FREQ / 4) * 0.017) / (224 * TMR0_PRESCALER * CVD_NUMBER_SENSORS)) 



                                                    //====================================================
#define CVD_SAMPLES_PER_SCAN        30
                                                    // This is the number of times each sensor will be 
                                                    //   scanned before asserting the mTouchCVD_dataReady() 
                                                    //   bit. If the application requires a specific
                                                    //   response time, there are several variables that 
                                                    //   will be important to the calculation of this
                                                    //   parameter.
                                                    //
                                                    // _XTAL_FREQ is the PICs oscillation frequency and
                                                    //   will determine how quickly the TMR0 counter will
                                                    //   increment.
                                                    //
                                                    // TMR0_PRESCALER also determines how quickly the 
                                                    //   TMR0 counter will increment. This value, located
                                                    //   in the OPTION register, is not set by the framework.
                                                    //
                                                    // CVD_NUMBER_SENSORS defines how many times the ISR
                                                    //   will interrupt between each decrement of the 
                                                    //   sample counter. The sample counter is initialized
                                                    //   with CVD_SAMPLES_PER_SCAN as its value.
                                                    //
                                                    // Allowable Range :: [ 1 to 65535 ]
                                                    //
                                                    // To help choose the appropriate number of samples,
                                                    //   we have provided some example timing calculations
                                                    //   to achieve 100, 20, and 17ms. You may either use
                                                    //   the defined label (ex: CVD_100ms_Timing) as the
                                                    //   value of CVD_SAMPLES_PER_SCAN or you can evaluate
                                                    //   the equation and place the integer result as the
                                                    //   value of this configuration option.

                                            
                                            
                                            //====================================================
#define THRESHOLD_PRESS_SENSOR0         30  // These values determine how far away from             
/// @cond
#define THRESHOLD_PRESS_SENSOR1         30  // the average the current reading must be in
#define THRESHOLD_PRESS_SENSOR2         30  // order to trigger a press.
#define THRESHOLD_PRESS_SENSOR3         30  //
#define THRESHOLD_PRESS_SENSOR4         30  // Allowable Range :: [ 1 to 65535 ]
#define THRESHOLD_PRESS_SENSOR5         30  //
#define THRESHOLD_PRESS_SENSOR6         30  
#define THRESHOLD_PRESS_SENSOR7         30  
#define THRESHOLD_PRESS_SENSOR8         65  
#define THRESHOLD_PRESS_SENSOR9         65  
#define THRESHOLD_PRESS_SENSOR10        65
#define THRESHOLD_PRESS_SENSOR11        65
#define THRESHOLD_PRESS_SENSOR12        65
#define THRESHOLD_PRESS_SENSOR13        65
#define THRESHOLD_PRESS_SENSOR14        65
#define THRESHOLD_PRESS_SENSOR15        65
#define THRESHOLD_PRESS_SENSOR16        65
#define THRESHOLD_PRESS_SENSOR17        65
#define THRESHOLD_PRESS_SENSOR18        65
#define THRESHOLD_PRESS_SENSOR19        65
#define THRESHOLD_PRESS_SENSOR20        65
#define THRESHOLD_PRESS_SENSOR21        65
#define THRESHOLD_PRESS_SENSOR22        65
#define THRESHOLD_PRESS_SENSOR23        65
#define THRESHOLD_PRESS_SENSOR24        65
/// @endcond

                                            //====================================================
#define CVD_SCALING                 1       // This option will determine the amount of post-scaling
                                            //  that is implemented on the acquisition's accumulator
                                            //  register before being stored in the results register.
                                            // The scaling factor should be set so that it is not
                                            //  possible for the result register to overflow 
                                            //      (Max output value is 65535.)
                                            // When in doubt, 16 is usually the best option.
                                            //
                                            //
                                            // Options:
                                            //
                                            //   1 - No scaling is performed.
                                            //       1:1 relationship between accumulator and result
                                            //
                                            //  16 - Accumulator is divided by 16.
                                            //       16:1 relationship between accumulator and result
                                            //
                                            // 256 - Accumulator is divided by 256.
                                            //       256:1 relationship between accumulator and result
                                            //

//**********************************************
//
// Communication Configuration
//
//**********************************************

                                            //====================================================
//#define CVD_DEBUG 1                       // Enable CVD Data Logging through RS232, comment 
                                            //   this out to disable RS232 output.
                                            
    //#define CVD_DEBUG_OUTPUT_HEX          // Enabled: Output in Hex notation
                                            // Disabled: Output in Decimal notation
                                            
    #define CVD_DEBUG_SPEED     115200      // 115200 or 9600
    #define CVD_DEBUG_PIN       GP5         // When using a PIC that does not have a hardware UART
                                            //   module, this value determines which pin is used for
                                            //   the software implementation of a UART.

    #define CVD_DEBUG_OUTPUT_RAW            // You may output the raw sensor values, the sensor average
    //#define CVD_DEBUG_OUTPUT_AVG          // values, or both. Simply uncomment/comment one or more options.
                                                
                                                //====================================================
                                                // System Version Control
                                                //
                                                //  These values are used only when communicating
                                                //  with the mTouch PC software to relay
                                                //  information about the current version of
                                                //  firmware being used.
                                                //
        #define CVD_Frame_Revision      01      //  Not yet implemented.
                                                //
        #define CVD_Board_Revision      0001    //  Not yet implemented.
                                                //
        #define CVD_Software_Revision   0002    //  Sets the current version of the GUI you are using
                                                //  so the framework will know what communication
                                                //  protocol is required.
                                                //
                                                //  Options:
                                                //  
                                                //      0001 - Sends raw data only
                                                //      0002 - Sends raw data and button state
                                                
                                            
//**********************************************
//
// Advanced Configuration Options
//
//**********************************************

                                            //====================================================
#define POWER_UP_SAMPLES            90      // This is the number of total scans that should be
                                            //   taken across ALL SENSORS before they will be 
                                            //   considered initialized.
                                            //
                                            // For example:
                                            //      Let's assume the dataReady flag is set once
                                            //      every 100ms due to the CVD_SAMPLES_PER_SCAN
                                            //      option and there are 8 sensors in our system.
                                            //      If we want a 3 second initialization time,
                                            //      we set POWER_UP_SAMPLES to 240.
                                            //
                                            //  (dataReady Toggle Rate / CVD_NUMBER_SENSORS) * POWER_UP_SAMPLES
                                            //  = Initialization Duration
                                            //
                                            //   Allowable Range :: [ 1 to 65535 ]

                                            //====================================================
#define	BUTTON_TIMEOUT              150     // This feature will reset the sensor's state if it
                                            //   remains in the 'pressed' state for too long.
                                            // This number is based off the amount of time between
                                            //   dataReady flag toggles.
                                            //
                                            // For example: 
                                            //      Let's assume the dataReady flag is set once
                                            //      every 100ms due to the CVD_SAMPLES_PER_SCAN
                                            //      option. If we want a timeout of 10sec, the
                                            //      BUTTON_TIMEOUT value should be set to 100.
                                            //      If we want a timeout of 7.4 seconds, then
                                            //      BUTTON_TIMEOUT should be set to 74.
                                            //
                                            // dataReady Toggle Rate * BUTTON_TIMEOUT = Timeout Duration
                                            //
                                            //
                                            //   Allowable Range :: [ 2 to 255 ]
                                            //
                                            // Setting to '0' or commenting the #define will
                                            //   disable the timeout feature.
                                            
                                            //====================================================
#define AVG_ENABLE                          // Leaving this uncommented will enable 
                                            //   implementation of an average (or 'baseline')
                                            //   to track environmental changes in the system.
                                            // NOTE!: Turning this off is highly discouraged.
                                            
                                            
                                            //====================================================
    #define AVG_UPDATE              8       // When the average updates itself using a new 
                                            //   reading, this value determines what weight is 
                                            //   given to the reading variable in the 
                                            //   calculation of the new average.
                                            // The reading variable will have a weight of 
                                            //   1/AVG_UPDATE in the average calculation. 
                                            // Ex: If it is 4, the average calculation would be 
                                            //   average = (1/4)*reading + (3/4)*average
                                            // Allowed Values :: 1, 2, 4, 8, 16

                                            
                                            
                                            //====================================================
    #define AVG_RATE                4       // The update rate of the sensors' average values 
                                            //   when the current sensor value is farther away 
                                            //   from the average value than AVG_RATE_THRESHOLD. 
                                            // (Fastest = 1)
                                            // Allowable Range :: [ 1 to 255 ]

//---------

                                            //====================================================
#define RELEASE_THRESHOLD_FACTOR    0.5     // Defines the hysteresis between press and release
                                            //   In other words, '0.5' would result in a release
                                            //   threshold equal to half of the press threshold.
                                            //
                                            // Release Thresholds are calculated using the equation:
                                            //
                                            // Release Threshold = Press Threshold * Release Threshold Factor
                                            //
                                            // Example: Press Threshold             = 100
                                            //          Release Threshold Factor    = 0.5
                                            //          Release Threshold         --> 50
                                            //
                                            //   Allowable Range :: [ 0.30 to 1.00 ]
                                            
                                            //====================================================
#define CVD_JITTER_ENABLE                   // This option should always be left on to maximize
                                            // noise immunity. 
                                            // 
                                            // The only time you may want to comment this out is if
                                            // you are looking at the sensor's waveform on a scope.
                                            // In that case, turning this off will allow you to 
                                            // see a clear waveform. 

                                            //====================================================
#define CVD_JITTER_TYPE             1       // This option will choose between two 'random' seeds
                                            //   for a random number generator that is used to
                                            //   implement the jittering functionality in the
                                            //   acquisition process.
                                            //
                                            // Options:
                                            //
                                            //   0 - The least significant bits of the ADRESL
                                            //       register are used. This is the smallest
                                            //       and fastest jittering option but may not
                                            //       provide the same amount of randomness that
                                            //       the other option can provide.
                                            //
                                            //   1 - For a slightly higher processing and memory
                                            //       cost, this jittering routine implements
                                            //       a linear feedback shift register to maximize
                                            //       the randomness. Requires one byte of memory
                                            //       and 5 additional instruction cycles to execute.

                                            
                                            //====================================================
#define	DEBOUNCE_COUNT_RELEASE      1       // Number of consecutive scans a button must be seen
                                            //   as released before a 'released state' is declared
                                            //
                                            //   Allowable Range :: [ 1 to 255 ]
 
 
                                            //====================================================
#define	DEBOUNCE_COUNT_PRESS    	1       // Number of consecutive scans a button must be seen 
                                            //   as pressed before a 'pressed state' is declared
                                            //
                                            //   Allowable Range :: [ 1 to 255 ]                                            

                                            
#define NEGATIVE_CAPACITANCE        0       //====================================================
                                            // This option determines the method used to handle
                                            //   a negative capacitance shift. In other words, how
                                            //   the system will respond to a reading value shift
                                            //   in the opposite direction of a normal shift
                                            //   caused by a finger.
                                            // 
                                            // These shifts occur for a variety of reasons and you 
                                            //   may want to adjust this behavior based on your 
                                            //   system.
                                            //
                                            // Options:
                                            //
                                            //   0 - Normal Average Behavior
                                            //       (DEFAULT) No special behavior. The average 
                                            //       will update at the normal rate, as always.
                                            //       For Metal-over-Capacitive systems, a negative
                                            //       shift usually means a neighboring sensor is
                                            //       being pressed, so this is the best option to
                                            //       keep the current state of the sensor and
                                            //       allow it to be pressed quickly after a
                                            //       neighbor has been pressed.
                                            //
                                            //   1 - Increase Average Speed
                                            //       The average will ignore the AVG_RATE counter
                                            //       and update itself after each new reading
                                            //       value. This has the effect of speeding up
                                            //       the average until it catches up with the
                                            //       negative shift. 
                                            //
                                            //   2 - Average Directly Follows Reading
                                            //       The average will instantly follow a negative
                                            //       shift. This behavior is best for a system
                                            //       that needs a very fast response time. It
                                            //       will help prevent quick, repeated presses 
                                            //       from being missed due to the average not
                                            //       updating quickly enough.

//**********************************************
//
// Advanced CVD Scan Routine Configuration Options
//
//**********************************************
                                            
#define CVD_SETTLING_DELAY          5       //====================================================
                                            // The larger this value is, the more susceptible to
                                            //   noise the application will be. However, there is
                                            //   a minimum amount of time required to get full
                                            //   sensitivity. 
                                            //
                                            // The correct amount of delay is dependant on VDD,
                                            //   the PIC's oscillation frequency, and the hardware
                                            //   design. Because of this, the correct amount of
                                            //   delay cannot be determined by this framework.
                                            //
                                            // Correctly choosing the delay is easy once you can
                                            //   see the live sensor data on the PC's GUI. 
                                            //
                                            // DESIGN RULE:
                                            // Increase CVD_BALANCE_DELAY until changes in VDD 
                                            //   no longer cause changes in the sensor's reading.
                                            // 
                                            // For low-voltage applications, maximum sensitivity
                                            //   can be attained in a shorter amount of time.
                                            //   Just be sure to verify that the sensor's value
                                            //   does not change across the possible VDD range
                                            //   for your application.


#define CVD_CHOLD_CHARGE_DELAY      5       //====================================================
                                            // The correct value for this delay is dependant on
                                            //   the reference source used to charge the internal 
                                            //   ADC capacitor.
                                            //
                                            // If the source is another sensor's I/O pin, this 
                                            //   delay will should be set so that CHOLD fully 
                                            //   reaches VDD despite changes in the reference
                                            //   sensor I/O's capacitance.
                                            //   
                                            //   For example: if SensorA uses SensorB as it's
                                            //   reference sensor, this delay should be set so
                                            //   so that there's enough time to fully charge
                                            //   to VDD even if the sensor being strongly pressed.
                                            //   (ie. when the sensor is at it's maximum amount of
                                            //   capacitance)
                                            //
                                            // If the source is the DAC, the delay time could be
                                            //   dependant on VDD.
                                            //   
                                            // DESIGN RULE:
                                            // Increase CVD_CHOLD_CHARGE_DELAY until pressing on
                                            //   one sensor no longer causes another sensor to
                                            //   decrease in value. ("negative crosstalk")
                                            //
                                            // Also, increase CVD_CHOLD_CHARGE_DELAY until the
                                            //   voltage change during the balancing step of the
                                            //   scan is maximized.
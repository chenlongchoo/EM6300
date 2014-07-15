/*************************************************************************
 *  © 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD_Acquisition.c
 *  Dependencies:    mTouchCVD.h
 *  Processor:       See documentation for supported PIC® microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB® IDE v8.50 (or later) or MPLAB® X                        
 *  Hardware:         
 *  Company:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     mTouch CVD Framework Acquisition Module
 *                   - Implements the CVD acquisition process through use
 *                     of the TMR0 ISR. Additional ISR functions should be
 *                     carefully designed to not interfere with the mTouch
 *                     interrupt service.
 *                   - See the documentation for more information about
 *                     implementing the framework with your application.
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
 /****************************************************************************
 * NOTE:  This is written in assembly to guarantee proper timing control for noise robustness.
 * PORTING THIS CODE TO 'C' COULD MAKE IT LESS ROBUST TO NOISE ENVIRONMENTS
 * Revision History
 * Date		Rev	 Author  Comments
 * 09.20.10    1.00         Public Release
 ****************************************************************************/
/** 
* @file mTouchCVD_Acquistion.c
* @brief Implements the ISR for the framework which performs the sensor scanning
*/
#include <includes/mTouchCVD.h>

/** @name mTouch Acquisition ISR Variables
* These variables are used by the ISR of the mTouch Framework and should not be directly
* accessed by the main loop application.
*/
//@{

      unsigned short long   CVD_Acq_Filtered_Data [CVD_NUMBER_SENSORS]; ///< Accumulator array for sensor oversampling              @ingroup Acquisition
bank0 unsigned int          last_result;                                ///< Stores ADC results for differential calculation        @ingroup Acquisition
bank0 unsigned int          sample_counter = CVD_SAMPLES_PER_SCAN;      ///< Counter variable for oversampling                      @ingroup Acquisition

      unsigned char mTouchCVD_dataReady;                                
      unsigned int  sensor_data                 [CVD_NUMBER_SENSORS];   ///< Decimate-filtered differential ADC result register     @ingroup Acquisition
      unsigned int  CVDSensorData               [CVD_NUMBER_SENSORS];   ///< Latch register storing the latest acquisition data     @ingroup Acquisition

#if defined(CVD_JITTER_ENABLE) && (CVD_JITTER_TYPE == 1)
      unsigned char mTouchCVD_jitter = 0x55;                            ///< Stores the current random seed value for jittering     @ingroup Acquisition
#endif
#if CVD_NUMBER_SENSORS > 1
bank0 unsigned char current_sensor;                                     ///< Currently scanned sensor index value                   @ingroup Acquisition
#endif
#if defined(CVD_DELAY_VARIABLE_REQUIRED)
      unsigned char mTouchCVD_delayCount;                               ///< Delay counter variable used in CVD scan sequence       @ingroup Acquisition
#endif

#if defined(_PIC14)
// These state-saving variables are only required in non-enhanced core PIC microcontrollers. They are automatically omitted if not needed.
near  unsigned char int_w;                                              ///< Saves the state of the W register, if required         @ingroup Acquisition
near  unsigned char int_status;                                         ///< Saves the state of the STATUS register, if required    @ingroup Acquisition
near  unsigned char int_fsr;                                            ///< Saves the state of the FSR register, if required       @ingroup Acquisition
near  unsigned char int_pclath;                                         ///< Saves the state of the PCLATH register, if required    @ingroup Acquisition
// TIP: The 'near' qualifier tells the compiler to place these variables in common memory so they may be accessed from any bank.
#endif
//@}


/**
*   @ingroup Acquisition
*   @brief mTouch CVD Framework Interrupt Service Routine
*
*   Implements the acquisition process for the mTouch CVD Framework. 
*   Adding code to the beginning of this function could significantly 
*   affect the noise performance of your final system. To avoid 
*   complications, add any necessary ISR functionality to the end of 
*   the ISR - after the mTouch scanning process is complete. Be careful 
*   that you are leaving enough time outside of the ISR to process the 
*   main line application.
*/
void interrupt ISR(void)
{
#asm

#if defined(_12F617) && (_HTC_VER_MAJOR_ == 9 && _HTC_VER_MINOR_ == 80)
    #include <cas12F617.h>      // HiTech's 9.80 compiler's caspic.h includes the 617 header file twice, causing compiling 
                                // errors. This will by-pass the mistake by directly including the correct file only once.
                                // This issue has been fixed as of the 9.81 release.
#else
    #include <caspic.h>
#endif

mTouchCVD_ISR:
#endasm 

    /**
    * @section AcqImp Algorithm Implementation Notes
    *
    * 1. If required, save the current state of the main loop application.
    */
    SAVE_STATE();

    /**
    * 2. Check and clear the TMR0IF flag. If it has not been set, jump 
    * to END_ISR.
    */
    CHECK_AND_CLEAR_TMR0IF();
    
    /**
    * 3. Reload TMR0 with a jittered reload value to prepare for the next 
    * mTouch scan after this one.
    */
    JITTER_START_TIME(); 

    /// 4. Check that the Go/Done bit of the ADC has been cleared.
    WAIT_FOR_GODONE_BIT();
    // TODO: Update this check to perform an action if the Go/Done flag has
    //       not cleared. When called off an interrupt, the Go/Done flag
    //       should always have enough time to be cleared. If it isn't, it
    //       means something has broken and action needs to be taken.
    //       PIC16F193x (Mask A1) devices' ADC temp issue, for example.

    /**
    * 5. Store the result of the previous sensor's ADC scan by subtracting
    * it from the value stored in last_result. This creates the 'differential'
    * value that will be accumulated in the next step.
    */
    STORE_LAST_RESULT();
    
    /**
    * 6. Implement and use a jump table based on which sensor should be
    * scanned next.
    */ 
    #pragma switch rangetable
    // TIP: The 'switch rangetable' option tells the compiler to implement
    //      the following switch statement as a jump table. It is still not as
    //      efficient as hand-coded assembly, however.
    GENERATE_JUMP_TABLE();    
   
    /**
    * 6. Generate the correct macro functions to implement the mTouch CVD
    * scanning algorithms. This step uses the number of sensors to implement
    * a 'previous sensor used as the reference sensor' configuration. The
    * last sensor uses the EXIT_LAST_SENSOR() macro instead of the EXIT_SENSOR()
    * macro. This allows the storage function to be performed instead of
    * immediately moving to the END_ISR label.
    */
#asm
sensor_0:                                               // This label is used by the jump table, above.
#endasm
    #if CVD_NUMBER_SENSORS > 0
        READ_SENSOR(0, CVD_SENSOR0_REFERENCE);          // WARNING: Only in the single-sensor configuration 
                                                        //          may a sensor use itself as the reference. 
                                                        //          Sending the same number as both arguments
                                                        //          of the READ_SENSOR macro in any other
                                                        //          configuration will cause the CVD scan to
                                                        //          no longer function correctly.
                                                        //
                                                        //          By default, sensors use the previously
                                                        //          scanned sensor as their reference.
                                                        
        #if CVD_NUMBER_SENSORS > 1                      // If there is more than one sensor,
        EXIT_SENSOR(current_sensor);                    // Use the normal ISR exiting method
        #else                                           // If there is only one sensor,
        EXIT_LAST_SENSOR(0);                            // Use the 'last sensor' exiting method
        #endif
    #endif
#asm
sensor_1:
#endasm
    #if CVD_NUMBER_SENSORS > 1
        READ_SENSOR(1, 0);                              // Scan sensor1 using sensor0 as the reference
        #if CVD_NUMBER_SENSORS > 2                      // If this is not the last sensor,
        EXIT_SENSOR(current_sensor);                    // Use the normal ISR exiting method
        #else                                           // If this is the last sensor,
        EXIT_LAST_SENSOR(current_sensor);               // Use the 'last sensor' exiting method
        #endif
    #endif
#asm
sensor_2:
#endasm
    #if CVD_NUMBER_SENSORS > 2
        READ_SENSOR(2, 1);                              // etc
        #if CVD_NUMBER_SENSORS > 3
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_3:
#endasm
    #if CVD_NUMBER_SENSORS > 3
        READ_SENSOR(3, 2);                              
        #if CVD_NUMBER_SENSORS > 4
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_4:
#endasm
    #if CVD_NUMBER_SENSORS > 4
        READ_SENSOR(4, 3);
        #if CVD_NUMBER_SENSORS > 5
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_5:
#endasm
    #if CVD_NUMBER_SENSORS > 5
        READ_SENSOR(5, 4);
        #if CVD_NUMBER_SENSORS > 6
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_6:
#endasm
    #if CVD_NUMBER_SENSORS > 6
        READ_SENSOR(6, 5);
        #if CVD_NUMBER_SENSORS > 7
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_7:
#endasm
    #if CVD_NUMBER_SENSORS > 7
        READ_SENSOR(7, 6);
        #if CVD_NUMBER_SENSORS > 8
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_8:
#endasm
    #if CVD_NUMBER_SENSORS > 8
        READ_SENSOR(8, 7);
        #if CVD_NUMBER_SENSORS > 9
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_9:
#endasm
    #if CVD_NUMBER_SENSORS > 9
        READ_SENSOR(9, 8);
        #if CVD_NUMBER_SENSORS > 10
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_10:
#endasm
    #if CVD_NUMBER_SENSORS > 10
        READ_SENSOR(10, 9);
        #if CVD_NUMBER_SENSORS > 11
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_11:
#endasm
    #if CVD_NUMBER_SENSORS > 11
        READ_SENSOR(11, 10);
        #if CVD_NUMBER_SENSORS > 12
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_12:
#endasm
    #if CVD_NUMBER_SENSORS > 12
        READ_SENSOR(12, 11);
        #if CVD_NUMBER_SENSORS > 13
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_13:
#endasm
    #if CVD_NUMBER_SENSORS > 13
        READ_SENSOR(13, 12);
        #if CVD_NUMBER_SENSORS > 14
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_14:
#endasm
    #if CVD_NUMBER_SENSORS > 14
        READ_SENSOR(14, 13);
        #if CVD_NUMBER_SENSORS > 15
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_15:
#endasm
    #if CVD_NUMBER_SENSORS > 15
        READ_SENSOR(15, 14);
        #if CVD_NUMBER_SENSORS > 16
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_16:
#endasm
    #if CVD_NUMBER_SENSORS > 16
        READ_SENSOR(16, 15);
        #if CVD_NUMBER_SENSORS > 17
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_17:
#endasm
    #if CVD_NUMBER_SENSORS > 17
        READ_SENSOR(17, 16);
        #if CVD_NUMBER_SENSORS > 18
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_18:
#endasm
    #if CVD_NUMBER_SENSORS > 18
        READ_SENSOR(18, 17);
        #if CVD_NUMBER_SENSORS > 19
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_19:
#endasm
    #if CVD_NUMBER_SENSORS > 19
        READ_SENSOR(19, 18);
        #if CVD_NUMBER_SENSORS > 20
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_20:
#endasm
    #if CVD_NUMBER_SENSORS > 20
        READ_SENSOR(20, 19);
        #if CVD_NUMBER_SENSORS > 21
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_21:
#endasm
    #if CVD_NUMBER_SENSORS > 21
        READ_SENSOR(21, 20);
        #if CVD_NUMBER_SENSORS > 22
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_22:
#endasm
    #if CVD_NUMBER_SENSORS > 22
        READ_SENSOR(22, 21);
        #if CVD_NUMBER_SENSORS > 23
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_23:
#endasm
    #if CVD_NUMBER_SENSORS > 23
        READ_SENSOR(23, 22);
        #if CVD_NUMBER_SENSORS > 24
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_24:
#endasm
    #if CVD_NUMBER_SENSORS > 24
        READ_SENSOR(24, 23);
        #if CVD_NUMBER_SENSORS > 25
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_25:
#endasm
    #if CVD_NUMBER_SENSORS > 25
        READ_SENSOR(25, 24);
        #if CVD_NUMBER_SENSORS > 26
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_26:
#endasm
    #if CVD_NUMBER_SENSORS > 26
        READ_SENSOR(26, 25);
        #if CVD_NUMBER_SENSORS > 27
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_27:
#endasm
    #if CVD_NUMBER_SENSORS > 27
        READ_SENSOR(27, 26);
        #if CVD_NUMBER_SENSORS > 28
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_28:
#endasm
    #if CVD_NUMBER_SENSORS > 28
        READ_SENSOR(28, 27);
        #if CVD_NUMBER_SENSORS > 29
        EXIT_SENSOR(current_sensor);
        #else
        EXIT_LAST_SENSOR(current_sensor);
        #endif
    #endif
#asm
sensor_29:
#endasm
    #if CVD_NUMBER_SENSORS > 29
        READ_SENSOR(29, 28);
        EXIT_LAST_SENSOR(current_sensor);
    #endif

    /**
    * 8. Uses the number of sensors to generate a storage macro for each 
    * sensor. This step is only reached after a decrementing counter,
    * initialized to #CVD_SAMPLES_PER_SCAN, reaches 0.
    */    
    GENERATE_STORAGE_FUNCTION();    
    
    /**
    * 9. Sets the mTouchCVD_dataReady flag so the main application knows
    * to process the new data.
    */   
    SET_DATA_READY_FLAG();

    /// <br><br><br>
    
#asm
END_ISR:
#endasm

    /**
    * 10. Sets the mTouchCVD_dataReady flag so the main application knows
    * to process the new data.
    */   
    RESTORE_STATE();

    /**
    * @section AcqEndISR Adding Additional ISR Checks and Functionality
    *
    * END_ISR is the place the mTouchCVD_ISR will jump to when its finished
    * performing its service.
    * 
    * Add additional application ISR functions/checks above the RESTORE_STATE() 
    * macro. You may program in C.
    */
    
    #asm    
    retfie
    #endasm
    
    /// <br><br><br>
    /**
    * @section AcqDec Decimation Helper Function
    *
    * The 'decimate' function is used in each of the READ_SENSOR() calls to
    * perform some common math calculations on the previous sensor scan's
    * result. It has been implemented as a macro to allow for different
    * implementations based on the chosen PIC microcontroller's core.
    */    
    /// <br><br><br>
#asm
decimate:
#endasm
    CVD_DECIMATE();         // Compare the previous sensor's scan result with
                            // its decimated latch register. If the current 
                            // result is larger, increment the latch register.
#asm
sensor_smaller:
#endasm
    CVD_SENSOR_SMALLER();   // If the current result is smaller, decrement the
                            // latch register.
#asm
decimate_end:
#endasm
    CVD_DECIMATE_END();     // Now add the new value of the latch register into
                            // the accumulater register. Finally, grab the
                            // result from the ADC scan that was performed in
                            // the first half of the READ_SENSOR() macro 
                            // currently under execution.
    

}

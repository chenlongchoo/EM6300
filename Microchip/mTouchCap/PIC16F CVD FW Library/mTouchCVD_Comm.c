/*************************************************************************
 *  © 2011 Microchip Technology Inc.                                       
 *  
 *  Project Name:    mTouch CVD Framework v1.1
 *  FileName:        mTouchCVD_Comm.c
 *  Dependencies:    mTouchCVD.h
 *  Processor:       See documentation for supported PIC® microcontrollers 
 *  Compiler:        HI-TECH Ver. 9.81 or later
 *  IDE:             MPLAB® IDE v8.50 (or later) or MPLAB® X                        
 *  Hardware:         
 *  Company:         
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Description:     mTouch CVD Framework Communication Module
 *                   - Implements the communication between the mTouch
 *                     PC GUI program and the firmware. May also be used
 *                     to output directly to a PC's terminal.
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
/** @file mTouchCVD_Comm.c
* @brief Implements the communication function of the framework used for debugging
*/
//*****************************************************************************
// Include and Header files
//*****************************************************************************
#include <stdlib.h>
#include <includes/mTouchCVD.h>

#if defined(CVD_DEBUG) && (CVD_DEBUG == 1)
//*****************************************************************************
// Local Function Prototypes
//*****************************************************************************
    ////////////////////////////////////////////////////
    /* System Support Functions */
    ////////////////////////////////////////////////////
void           mTouchCVD_Comm_Init(void);

    ////////////////////////////////////////////////////
    /* Communication Functions */
    ////////////////////////////////////////////////////
void           send_Data(void);

    ////////////////////////////////////////////////////
    /* Communication Support Functions */
    ////////////////////////////////////////////////////
void           put_Char     (unsigned char);
void           str_Int      (unsigned int);
void           str_Char     (unsigned char);


///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
// SUPPORT FUNCTIONS
//*****************************************************************************
///////////////////////////////////////////////////////////////////////////////

/****************************************************************
* Function: void Init(void)
*
* Overview: This function configures the peripherals and presets
*           the system variables
*
* Input:    None
*
* Output:   None
*
****************************************************************/
/**
*   @brief This function configures the communiation peripherals and presets the system variables
*/
void mTouchCVD_Comm_Init(void)
{
    // Turn on the Hardware UART - this is defined in each HardwareProfile header file
    CVD_DEBUG_COMM_INIT();
}


///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
// Communications Functions
//*****************************************************************************
///////////////////////////////////////////////////////////////////////////////

/**
*   @brief Performs a loop and outputs data to the PC according to the set protocol
*/
void send_Data(void)
{    
    #if CVD_Software_Revision == 0002
    // If using the 0002 version of the GUI, sensor state information should be
    // sent before outputting the current reading values.
        unsigned int btnMsk = 0;
        unsigned int bufMsk = 1;
        for (unsigned char i = 0; i < CVD_NUMBER_SENSORS; i++)
        {
            if (CVD_GetButtonState(i) < CVD_PRESSED)
            {
                btnMsk |= bufMsk;
            }
            bufMsk <<= 1;
        }
        btnMsk = ~btnMsk;            // Bitwise 'NOT' because bits are inverted 
        
        str_Int(btnMsk);             // Output sensor state data
    #endif
    
    // For all versions of the GUI, output the current reading values.
    for (unsigned char i = 0; i < CVD_NUMBER_SENSORS; i++)
    { 
        #if defined(CVD_DEBUG_OUTPUT_RAW)
        str_Int(CVDGetSensor(i));
        #endif
        #if defined(CVD_DEBUG_OUTPUT_AVG)
        str_Int(Average[i]);
        #endif
    }
    
    // Carriage return, line feed
    put_Char(0x0D);
    put_Char(0x0A);    
}

///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
// Communications Support functions
//*****************************************************************************
///////////////////////////////////////////////////////////////////////////////

/****************************************************************
* Function: void str_Int(unsigned int i)
*
* Overview: This function sends an INT as a space and 5 digits
*
* Input:    Din is the value to be converted
*           base is the radix of the value
*
* Output:   None
*
****************************************************************/
/**
*   @brief Sends an integer through the serial port
*
*   @param i The integer to send
*/
void str_Int(unsigned int i)
{
	unsigned char ctr = 0;
    
    #if defined(CVD_DEBUG_OUTPUT_HEX)
    
    // Hex Output
    ctr = (unsigned char)(i >> 12);
    if (ctr <= 9) { put_Char(ctr+0x30); } else { put_Char(ctr+0x37); }
    ctr = (unsigned char)(i >> 8) & 0x0F;
    if (ctr <= 9) { put_Char(ctr+0x30); } else { put_Char(ctr+0x37); }
    ctr = (unsigned char)(i >> 4) & 0x0F;
    if (ctr <= 9) { put_Char(ctr+0x30); } else { put_Char(ctr+0x37); }
    ctr = (unsigned char)(i & 0x0F);
    if (ctr <= 9) { put_Char(ctr+0x30); } else { put_Char(ctr+0x37); }
    
    #else
    
    // Decimal Output
 	while (i >= 10000)  { i -= 10000; ctr++; } put_Char(ctr+0x30); ctr=0;
 	while (i >=  1000)  { i -=  1000; ctr++; } put_Char(ctr+0x30); ctr=0;
	while (i >=   100)  { i -=   100; ctr++; } put_Char(ctr+0x30); ctr=0;
	while (i >=    10)  { i -=    10; ctr++; } put_Char(ctr+0x30); ctr=0;
	while (i >=     1)  { i -=     1; ctr++; } put_Char(ctr+0x30);	
    
    #endif
    
    // A semi-colon delimiter is used for the GUI. This can be changed if
    // you are not using the GUI to process the output.
    put_Char(';');
}


/**
*   @brief Sends a single character through the serial port or bit-bangs
*          a UART implementation if no UART is available.
*
*   @param Dout The character to send
*/
void put_Char(unsigned char Dout)
{
    #if defined(CVD_DEBUG_UART_ENABLED)
        while(CVD_DEBUG_COMM_TXIF == 0);
        CVD_DEBUG_COMM_TXREG = Dout;
    #else
        #define RS232__RATE         CVD_DEBUG_SPEED  
        #define RS232__PORT         CVD_DEBUG_PIN     
        #if (_XTAL_FREQ == 8000000)
            #if (RS232__RATE == 9600)
                #define RS232__DELAY		22 
                #define RS232__DELAYCODE()  for (j = RS232__DELAY; j > 0; j--) { NOP(); NOP(); NOP(); NOP(); NOP(); } NOP(); NOP(); NOP(); NOP(); 
            #elif (RS232__RATE == 56700)
                #define RS232__DELAY		3   
                #define RS232__DELAYCODE()  for (j = RS232__DELAY; j > 0; j--) { NOP(); NOP(); NOP(); } NOP();
            #elif (RS232__RATE == 115200)
                #define RS232__DELAY		1
                #define RS232__DELAYCODE()  NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
            #else
                #error The chosen RS232 baud rate is not supported. Choose either 9600, 57600, or 115200.
            #endif
        #elif (_XTAL_FREQ == 4000000)
            #if (RS232__RATE == 9600)
                #define RS232__DELAY		10 
                #define RS232__DELAYCODE()  for (j = RS232__DELAY; j > 0; j--) { NOP(); NOP(); NOP(); NOP(); NOP(); } NOP(); NOP(); NOP();
            #elif (RS232__RATE == 56700)
                #define RS232__DELAY		1   
                #define RS232__DELAYCODE()  NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); NOP();
            #elif (RS232__RATE == 115200)
                #error 115.2k baud rate is not available with a 4MHz oscillator. Choose 9600 or 57600, or move to a 8MHz oscillator.
            #else
                #error The chosen RS232 baud rate is not supported. Choose either 9600 or 57600.
            #endif
        #else
            #warning Current clock frequency is not a supported option for this version of RS232 code.
        #endif
        
        //DO NOT EDIT! DO NOT EDIT! DO NOT EDIT! DO NOT EDIT! DO NOT EDIT! DO NOT EDIT
        ///////////////////////////////////////////////////////////////////////////////
        // Software UART Documentation
        //                                                                 
        //              b0  b1  b2  b3  b4  b5  b6  b7                   
        // _ _ _ _     _ _                     _ _         _ _ _ _ _ _ _         
        //  IDLE  |_S_| 1 |_0_ _0_ _0_ _0_ _0_| 1 |_0_ _0_| P    IDLE           
        //  |       |                                   |   |    |                
        //  IDLE    START                            PARITY STOP  IDLE          
        //                                                                 
        //             b<7:0> = 0x41 = 65 = 'A'                                                                                                              
        //
        // Design Spec:
        // 	 Use 8-bit transmissions
        //	 Parity none  {Will use "No Parity" for Parity, this bit is not sent.  (Example shows parity=0)}
        //   Delay tailored to 4MHz, Tbit=17.2usec,  baud=58139 @ -0.94% of 57.6k     
        ///////////////////////////////////////////////////////////////////////////////
        //DO NOT EDIT! DO NOT EDIT! DO NOT EDIT! DO NOT EDIT! DO NOT EDIT! DO NOT EDIT

        //  START BIT
        RS232__PORT = 0;
        if (!(Dout & 0x00)) { NOP(); NOP(); NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();

        //  BIT 0
        if   (Dout & 0x01)  { RS232__PORT = 1; } else { RS232__PORT = 0; }    
        if (!(Dout & 0x01)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  BIT 1
        if   (Dout & 0x02)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 
        if (!(Dout & 0x02)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  BIT 2
        if   (Dout & 0x04)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 
        if (!(Dout & 0x04)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  BIT 3
        if   (Dout & 0x08)  { RS232__PORT = 1; } else { RS232__PORT = 0; }   
        if (!(Dout & 0x08)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  BIT 4
        if   (Dout & 0x10)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 
        if (!(Dout & 0x10)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();

        //  BIT 5
        if   (Dout & 0x20)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 
        if (!(Dout & 0x20)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  BIT 6
        if   (Dout & 0x40)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 	
        if (!(Dout & 0x40)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  BIT 7
        if   (Dout & 0x80)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 
        if (!(Dout & 0x80)) { NOP(); NOP(); NOP(); NOP(); }
        RS232__DELAYCODE();
        
        //  NO PARITY BIT
        
        //  STOP BIT
        NOP(); NOP();
        if   (Dout | 0xFF)  { RS232__PORT = 1; } else { RS232__PORT = 0; } 
        if (!(Dout | 0xFF)) { NOP(); NOP(); NOP(); NOP(); } 
        RS232__DELAYCODE();

        //  IDLE STATE
        RS232__PORT = 1;
        
    #endif
}

#endif
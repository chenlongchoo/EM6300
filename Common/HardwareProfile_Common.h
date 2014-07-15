

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include "GenericTypeDefs.h"
#include "Compiler.h"


#if defined(__18F26K20)
	#include <p18f26k20.h>
	#define CLOCK_SPEED						32000000
#endif
#if defined(__18F87J50)
	#include <p18f87J50.h>

	#define TRANSCEIVER_MCU
	#define CLOCK_SPEED						48000000
#endif
#if defined(__18F2455)
	#include <p18f2455.h>
#endif



#define		SINGLEPHASEMETER_MCU1_BOARD
//#define		SINGLEPHASEMETER_MCU2_BOARD
//#define		TRANSCEIVER_BOARD


//#define APP_USE_MANUAL_READING
//#define APP_USE_RTCC

	
	#define FIRMWARE_VERSION				"v.3A.001"

	
	//#define	DEVICE_TYPE_ROUTER			//CPU_SPEED_16MHZ		
	#define		DEVICE_TYPE_END_DEVICE		//CPU_SPEED_48MHZ

	    
    
//    // USART pins.
//    #if defined(SINGLEPHASEMETER_MCU1_BOARD)
//    	#define USART_RX_EN						RCSTA1bits.CREN
//		#define USART_OVERRUN_ERROR				RCSTA1bits.OERR
//		#define ZIGBEE_RX_INTERRUPT_EN			PIE1bits.RC1IE
//		#define ZIGBEE_RX_INTERRUPT_FLAG		PIR1bits.RC1IF
//		#define ZIGBEE_RX_INTERRUPT_PRIORITY	IPR1bits.RC1IP
//		//#define	ZIGBEE_HIGH_BAUDRATE			TXSTAbits.BRGH
//		#define ZIGBEE_AUTO_BAUDRATE			BAUDCON1bits.ABDEN
//		#define ZIGBEE_16BIT_BAUDRATE			BAUDCON1bits.BRG16
//		#define ZIGBEE_READ						Read1USART()
//		#define ZIGBEE_DATARDY					DataRdy1USART()
//	#endif
	
	
	
	
	
	/*******************************************************************/
    /************************* RTCC ********************************/
    /*******************************************************************/
	
	#define RTCC_SCLK		LATCbits.LATC0
	#define RTCC_SCLK_TRIS	TRISCbits.TRISC0

	#define RTCC_IO_PORT	PORTCbits.RC1
	#define RTCC_IO_TRIS	TRISCbits.TRISC1
	#define RTCC_IO_LAT		LATCbits.LATC1

	#define RTCC_CE			LATAbits.LATA4
	#define RTCC_CE_TRIS	TRISAbits.TRISA4
	
	
	
	/*******************************************************************/
    /********************* MDD ********************************/
    /*******************************************************************/
	// Description: Macro used to enable the SD-SPI physical layer (SD-SPI.c and .h)
	#define DEMO_BOARD PIC18F26K20_PIM
    #define PIC18F26K20_PIM

    #if defined(__18F87J50)
    	#define GetSystemClock()		(48000000ul)      // Hz
		#define GetInstructionClock()	(GetSystemClock()/4)
		#define GetPeripheralClock()	GetInstructionClock()
    #elif defined(__18F26K20) || defined(__18F46K20)
    	#define GetSystemClock()        32000000                        // System clock frequency (Hz)
    	#define GetPeripheralClock()    GetSystemClock()                // Peripheral clock freq.
    	#define GetInstructionClock()   GetSystemClock()                // Instruction clock freq.
    #else
    	#error "System clock frequency not defined."
    #endif
    
    #define USE_SD_INTERFACE_WITH_SPI

	    #define USE_PIC18
            
        #define INPUT_PIN           1
        #define OUTPUT_PIN          0
    
        // Chip Select Signal
        #define SD_CS               PORTBbits.RB2
        #define SD_CS_TRIS          TRISBbits.TRISB2
        
        // Card detect signal
        #define SD_CD               PORTBbits.RB0
        #define SD_CD_TRIS          TRISBbits.TRISB0
        
        // Write protect signal
        #define SD_WE               PORTBbits.RB1
        #define SD_WE_TRIS          TRISBbits.TRISB1
        
        // Registers for the SPI module you want to use
        #define SPICON1             SSPCON1
        #define SPISTAT             SSPSTAT
        #define SPIBUF              SSPBUF
        #define SPISTAT_RBF         SSPSTATbits.BF
        #define SPICON1bits         SSP1CON1bits
        #define SPISTATbits         SSPSTATbits
    
        #define SPI_INTERRUPT_FLAG  PIR1bits.SSPIF   
    
        // Defines for the HPC Explorer board
        #define SPICLOCK            TRISCbits.TRISC3
        #define SPIIN               TRISCbits.TRISC4
        #define SPIOUT              TRISCbits.TRISC5
    
        // Latch pins for SCK/SDI/SDO lines
        #define SPICLOCKLAT         LATCbits.LATC3
        #define SPIINLAT            LATCbits.LATC4
        #define SPIOUTLAT           LATCbits.LATC5
    
        // Port pins for SCK/SDI/SDO lines
        #define SPICLOCKPORT        PORTCbits.RC3
        #define SPIINPORT           PORTCbits.RC4
        #define SPIOUTPORT          PORTCbits.RC5
    
        #define SPIENABLE           SSPCON1bits.SSPEN

        // Will generate an error if the clock speed is too low to interface to the card
        #if (GetSystemClock() < 400000)
            #error System clock speed must exceed 400 kHz
        #endif
        
        #include <uart2.h>


	/*******************************************************************/
    /********************* MAXQ3180 ********************************/
    /*******************************************************************/
	#if defined(__18F87J50)
		#define POWER_SCK_TRIS		TRISDbits.TRISD6
		#define POWER_MOSI_TRIS		TRISDbits.TRISD4
		#define POWER_MISO_TRIS		TRISDbits.TRISD5
		#define POWER_CS_LAT		LATDbits.LATD7
		#define POWER_CS_TRIS		TRISDbits.TRISD7
	
		#define POWER_INT_FLAG		PIR1bits.SSP1IF
		#define POWER_RX_BUFFER		SSP1BUF
	#endif
	#if defined(__18F26K20) || defined(__18F46K20)
		#define POWER_SCK_TRIS		TRISCbits.TRISC3
		#define POWER_MOSI_TRIS		TRISCbits.TRISC5
		#define POWER_MISO_TRIS		TRISCbits.TRISC4
		#define POWER_CS_LAT		LATCbits.LATC2
		#define POWER_CS_TRIS		TRISCbits.TRISC2
		
		#define POWER_INT_FLAG		PIR1bits.SSPIF
		#define POWER_RX_BUFFER		SSPBUF
	#endif
	#if defined(__18F2455)
		#define POWER_SCK_TRIS		TRISBbits.TRISB1
		#define POWER_MOSI_TRIS		TRISCbits.TRISC7
		#define POWER_MISO_TRIS		TRISBbits.TRISB0
		#define POWER_CS_LAT		LATBbits.LATB5
		#define POWER_CS_TRIS		TRISBbits.TRISB5
		
		#define POWER_INT_FLAG		PIR1bits.SSPIF
		#define POWER_RX_BUFFER		SSPBUF
	#endif
//	#define SPIRXBUFFER			SSP2BUF
//	#define SPIBUSYFLAG			SSP2STATbits.BF
//	#define SPIWRITEBYTE(a)		WriteSPI2(a)
	
	/*******************************************************************/
    /********************* UNIO EEPROM ********************************/
    /*******************************************************************/
	#define SCIO					PORTHbits.RH4
	#define SCIO_LAT				LATHbits.LATH4
	#define SCIO_TRIS				TRISHbits.TRISH4


    //*************************************************//
    //************** Comms to MCU2.
    //*************************************************//
    //#define BAUDRATE_9600
    
    #if defined( __18F87J50 )
    #define MCU_RX_INTERRUPT_FLAG		PIR1bits.RCIF
	#define MCU_RX_INTERRUPT_PRIORITY	IPR1bits.RCIP
//	#define	ZIGBEE_HIGH_BAUDRATE			TXSTAbits.BRGH
	#define MCU_AUTO_BAUDRATE			BAUDCONbits.ABDEN
	#define MCU_16BIT_BAUDRATE			BAUDCONbits.BRG16

	#define MCU_READDATA			ReadUSART()
    #define MCU_DATAREADY			DataRdy1USART()
    
    //#define MCUIsBusy()				Busy1USART()
    //#define MCU2_READDATA			ReadUSART()
    //#define MCU2_DATARDY			DataRdy1USART()
#elif defined( __18F26K20 )
    #define MCU_RX_INTERRUPT_FLAG		PIR1bits.RCIF
	#define MCU_RX_INTERRUPT_PRIORITY	IPR1bits.RCIP
//	#define	ZIGBEE_HIGH_BAUDRATE			TXSTAbits.BRGH
	#define MCU_AUTO_BAUDRATE			BAUDCONbits.ABDEN
	#define MCU_16BIT_BAUDRATE			BAUDCONbits.BRG16

	#define MCU_READDATA			ReadUSART()
    #define MCU_DATAREADY			DataRdyUSART()
    
    //#define MCUIsBusy()				BusyUSART()
    #define MCU2_READDATA			ReadUSART()
    #define MCU2_DATARDY			DataRdyUSART()
#endif
    
    //*************************************************//
    //************** Comms to ZIGBEE.
    //*************************************************//
    #if defined(__18F87J50)
    #define ZIGBEE_RX_INTERRUPT_FLAG		PIR1bits.RC1IF
	#define ZIGBEE_RX_INTERRUPT_PRIORITY	IPR1bits.RC1IP
//	#define	ZIGBEE_HIGH_BAUDRATE			TXSTAbits.BRGH
	#define ZIGBEE_AUTO_BAUDRATE			BAUDCON1bits.ABDEN
	#define ZIGBEE_16BIT_BAUDRATE			BAUDCON1bits.BRG16
	#endif
	
    

    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Definitions for TCPIP STACK *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/


    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Application specific definitions *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/


#endif  //HARDWARE_PROFILE_PIC18F87J50_PIM_H

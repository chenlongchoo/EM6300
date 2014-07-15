

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#define		CLOCK_SPEED		32000000
//#define		CLOCK_SPEED		16000000

#if defined(__18F26K20)
#include <p18f26k20.h>
#endif
#if defined(__18F87J50)
#include <p18f87J50.h>
#endif
#if defined(__18F2455)
#include <p18f2455.h>
#endif



#define		SINGLEPHASEMETER_MCU1_BOARD
//#define		SINGLEPHASEMETER_MCU2_BOARD
//#define		TRANSCEIVER_BOARD


//#define APP_USE_MANUAL_READING
//#define APP_USE_RTCC

	
	#define FIRMWARE_VERSION				"v.2B.003"

	//#define		___18F87J50
	
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
    /********************* MAXQ3180 ********************************/
    /*******************************************************************/
	#if defined(__18F87J50)
		#define POWER_SCK_TRIS		TRISDbits.TRISD6
		#define POWER_MOSI_TRIS		TRISDbits.TRISD4
		#define POWER_MISO_TRIS		TRISDbits.TRISD5
		#define POWER_CS_LAT		LATDbits.LATD7
		#define POWER_CS_TRIS		TRISDbits.TRISD7
	#endif
	#if defined(__18F26K20)
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
    		
    #define ZIGBEE_RX_INTERRUPT_FLAG		PIRbits.RC1IF
	#define ZIGBEE_RX_INTERRUPT_PRIORITY	IPR1bits.RCIP
//	#define	ZIGBEE_HIGH_BAUDRATE			TXSTAbits.BRGH
	#define ZIGBEE_AUTO_BAUDRATE			BAUDCONbits.ABDEN
	#define ZIGBEE_16BIT_BAUDRATE			BAUDCONbits.BRG16

	#define MCU_READDATA			ReadUSART()
    #define MCU_DATARDY				DataRdyUSART()
    
    #define mTxRdyUSART()   		TXSTAbits.TRMT
    #define MCU2_READDATA			ReadUSART()
    #define MCU2_DATARDY			DataRdyUSART()
    

    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Application specific definitions *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/

#endif  //HARDWARE_PROFILE_PIC18F87J50_PIM_H

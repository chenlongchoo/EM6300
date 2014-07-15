/*********************************************************************
 *
 *	Hardware specific definitions
 *
 *********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    None
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.10 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.34 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		10/03/06	Original, copied from Compiler.h
 * Ken Hesky            07/01/08    Added ZG2100-specific features
 ********************************************************************/
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

//#include "AppConfig.h"  // We cannot call this here because HardwareProfile.h must be processed before AppConfig.h.
#include "GenericTypeDefs.h"
#include "Compiler.h"


#if defined( METER_TOP_BOARD )	
	#define FIRMWARE_VERSION		"v4.6.2a"
	#define DEFAULT_USER_PASSWORD	"1010"
	#define EEPROM_VERSION			0
#elif defined( COMMUNICATOR_BOARD )
	//
#endif


#define BOARD_TYPE				(PORTE & 0x03)


// If no hardware profiles are defined, assume that we are using 
// a Microchip demo board and try to auto-select the correct profile
// based on processor selected in MPLAB
#if !defined(PICDEMNET2) && !defined(PIC18_EXPLORER) && !defined(HPC_EXPLORER) && !defined(EXPLORER_16) && !defined(PIC24FJ64GA004_PIM) && !defined(DSPICDEM11) && !defined(PICDEMNET2) && !defined(INTERNET_RADIO) && !defined(YOUR_BOARD) && !defined(__PIC24FJ128GA006__) && !defined(PIC32_STARTER_KIT) && !defined(PIC32_ETH_STARTER_KIT)
	#if defined(__18F97J60) || defined(_18F97J60)
		#define PICDEMNET2
	#elif defined(__18F8722) || defined(__18F87J10) || defined(_18F8722) || defined(_18F87J10) || defined(__18F87J11) || defined(_18F87J11)|| defined(__18F87J50) || defined(_18F87J50)
		#define PIC18_EXPLORER
	#endif
#endif

// Set configuration fuses (but only once)
#if defined(THIS_IS_STACK_APPLICATION)
	#if defined(__18CXX)
		#if defined(__EXTENDED18__)
			#pragma config XINST=ON
		#elif !defined(HI_TECH_C)
			#pragma config XINST=OFF
		#endif
	
		#if defined(__18F8722) && !defined(HI_TECH_C)
			#error 1
		#elif defined(__18F87J50) && !defined(HI_TECH_C)
			// Reduce WDT postscalar to 4096. Approx. 15 seconds.
			#pragma config WDTEN=ON, WDTPS=4096, FOSC=HSPLL, PLLDIV=2, CPUDIV=OSC1
//			#pragma config WDTEN=ON, WDTPS=8192, FOSC=HSPLL, PLLDIV=2, CPUDIV=OSC1
//			#pragma config WDTEN=OFF, FOSC=HSPLL, PLLDIV=2, CPUDIV=OSC1
		#endif
	#endif
#endif // Prevent more than one set of config fuse definitions

// Clock frequency value.
// This value is used to calculate Tick Counter value
#if defined(__18CXX)
	// All PIC18 processors
	#if defined(__18F87J50)
		#define BOOTLOAD_DEVID			0xFF0010FFul
		#define GetSystemClock()		(48000000ul)      // Hz
		#define GetInstructionClock()	(GetSystemClock()/4)
		#define GetPeripheralClock()	GetInstructionClock()
	//#elif defined(__18F26K20)
	#else
		#error "System clock frequency value not defined."
	#endif
#endif

    /*************************************************/
    /************** BUTTON *****************************/
    /*************************************************/
    #if defined( METER_TOP_BOARD )
    	#define BUTTON_UP						PORTBbits.RB3
		#define BUTTON_UP_TRIS					TRISBbits.TRISB3
	
		#define BUTTON_DOWN						PORTBbits.RB2
		#define BUTTON_DOWN_TRIS				TRISBbits.TRISB2

		#define BUTTON_ENTER					PORTBbits.RB1
		#define BUTTON_ENTER_TRIS				TRISBbits.TRISB1

		#define BUTTON_EXIT						PORTBbits.RB0
		#define BUTTON_EXIT_TRIS				TRISBbits.TRISB0
		
		#define BUTTON_RESET					PORTJbits.RJ3
		#define BUTTON_RESET_TRIS				TRISJbits.TRISJ3
	#endif
	
	#if defined( COMMUNICATOR_BOARD )
		#define BUTTON_INT						PORTBbits.RB0
		#define BUTTON_INT_TRIS					TRISBbits.TRISB0
	#endif
    /*************************************************/
    /************** LCD ******************************/
    /*************************************************/
	#if defined ( METER_TOP_BOARD )

		#define LCD_LED			LATDbits.LATD0   // Backlight
		#define LCD_LED_TRIS	TRISDbits.TRISD0

		#define LCD_EN 			LATGbits.LATG0   // Enable 
		#define LCD_EN_TRIS 	TRISGbits.TRISG0
		
		#define LCD_RS 			LATGbits.LATG1   // Register Select
		#define LCD_RS_TRIS 	TRISGbits.TRISG1
		
		#define LCD_DATA 		LATE             // Data Pins
		#define LCD_DT_TRIS 	TRISE
		
	#endif
	
	/*************************************************/
    /************** BUZZER ***************************/
    /*************************************************/
    #if defined( METER_TOP_BOARD )
    	#define BUZZER							LATCbits.LATC2
    	#define BUZZER_TRIS						TRISCbits.TRISC2
    #endif
    

	/*************************************************/
    /************** LEDs ***************************/
    /*************************************************/

	#if defined( COMMUNICATOR_BOARD )	
		#define LED0_TRIS			(TRISBbits.TRISB3)
		#define LED0_IO				(LATBbits.LATB3)
		#define LED1_TRIS			(TRISBbits.TRISB2)
		#define LED1_IO				(LATBbits.LATB2)
		#define LED2_TRIS			(TRISBbits.TRISB1)
		#define LED2_IO				(LATBbits.LATB1)
	#elif defined( METER_TOP_BOARD )		
		#define LED0_TRIS			(TRISHbits.TRISH3)
		#define LED0_IO				(LATHbits.LATH3)
		#define LED1_TRIS			(TRISHbits.TRISH2)
		#define LED1_IO				(LATHbits.LATH2)
	#else
		#error "No board defined"
	#endif
	
    /*************************************************/
    /************** CLOCK SPEED **********************/
    /*************************************************/
	#if defined(__18F26K20)
		#include <p18f26k20.h>
		#define CLOCK_SPEED						32000000
	#elif defined(__18F46K20)
		#include <p18f46k20.h>
		#define CLOCK_SPEED						32000000
	#elif defined(__18F87J50)
		#include <p18f87J50.h>
		#define CLOCK_SPEED						48000000
	#elif defined(__18F2455)
		#include <p18f2455.h>
	#endif
		
    /*************************************************/
    /********** Comms to ZIGBEE. *********************/
    /*************************************************/
    #if defined ( COMMUNICATOR_BOARD )	
		//XBee pins
		#define ZB_DTR				LATHbits.LATH3		// ->
		#define ZB_DTR_TRIS			TRISHbits.TRISH3
		#define ZB_CTS				LATEbits.LATE1		// <-
		#define ZB_CTS_TRIS			TRISEbits.TRISE1
		#define ZB_RTS				LATGbits.LATG0		// <-
		#define ZB_RTS_TRIS			TRISGbits.TRISG0
		#define ZB_STA				LATEbits.LATE0
		#define ZB_STA_TRIS			TRISEbits.TRISE0
		#define ZB_RESET			LATHbits.LATH2		// ->
		#define ZB_RESET_TRIS		TRISHbits.TRISH2
	#endif

    //*************************************************//
    //************** Comms to MCU2.
    //*************************************************//
    //#define BAUDRATE_9600

	#if defined( __18F87J50 )
		#if defined( METER_TOP_BOARD )
			#define MCU_RX_INTERRUPT_FLAG		PIR1bits.RC1IF
			#define MCU_RX_INTERRUPT_PRIORITY	IPR1bits.RC1IP
			#define MCU_AUTO_BAUDRATE			BAUDCON1bits.ABDEN
			#define MCU_16BIT_BAUDRATE			BAUDCON1bits.BRG16
		
			#define MCU_READDATA				Read1USART()
		    #define MCU_DATAREADY				DataRdy1USART()
			#define mTxRdyUSART()   			TXSTA1bits.TRMT
		#endif
	#elif defined( __18F26K20 )
	    #define MCU_RX_INTERRUPT_FLAG		PIR1bits.RCIF
		#define MCU_RX_INTERRUPT_PRIORITY	IPR1bits.RCIP
	//	#define	ZIGBEE_HIGH_BAUDRATE			TXSTAbits.BRGH
		#define MCU_AUTO_BAUDRATE			BAUDCONbits.ABDEN
		#define MCU_16BIT_BAUDRATE			BAUDCONbits.BRG16

		#define MCU_READDATA			ReadUSART()
	    #define MCU_DATARDY				DataRdyUSART()
    
	    #define mTxRdyUSART()   		TXSTA2bits.TRMT
	    #define MCU2_READDATA			ReadUSART()
	    #define MCU2_DATARDY			DataRdyUSART()
	#endif

    /** Board definition ***********************************************/
    //These defintions will tell the main() function which board is
    //  currently selected.  This will allow the application to add
    //  the correct configuration bits as wells use the correct
    //  initialization functions for the board.  These defitions are only
    //  required in the stack provided demos.  They are not required in
    //  final application design.
    #define DEMO_BOARD PICDEM_FS_USB
    #define PICDEM_FS_USB
    #define CLOCK_FREQ 48000000
    //#define GetSystemClock() (CLOCK_FREQ)
    
    /** USB ************************************************************/
    //The PIC18F87J50 FS USB Plug-In Module supports the USE_USB_BUS_SENSE_IO
    //feature.  The USE_SELF_POWER_SENSE_IO feature is not implemented on the
    //circuit board, so the USE_SELF_POWER_SENSE_IO define should always be
    //commented for this hardware platform.


    //The PICDEM FS USB Demo Board platform supports the USE_SELF_POWER_SENSE_IO
    //and USE_USB_BUS_SENSE_IO features.  Uncomment the below line(s) if
    //it is desireable to use one or both of the features.
    //#define USE_SELF_POWER_SENSE_IO
    #if defined ( COMMUNICATOR_BOARD )

		#define tris_self_power     TRISAbits.TRISA2    // Input
    	#if defined(USE_SELF_POWER_SENSE_IO)
    		#define self_power          PORTAbits.RA2
    	#else
    		#define self_power          1
    	#endif

    	#define USE_USB_BUS_SENSE_IO
    	#define tris_usb_bus_sense  TRISJbits.TRISJ5    // Input
    	#if defined(USE_USB_BUS_SENSE_IO)
    		#define USB_BUS_SENSE		PORTJbits.RJ5
    	#else
    		#define USB_BUS_SENSE       1
    	#endif
	#endif

    //Uncomment the following line to make the output HEX of this  
    //  project work with the MCHPUSB Bootloader    
    //#define PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER
	
    //Uncomment the following line to make the output HEX of this 
    //  project work with the HID Bootloader
    //#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER	
    
    /** I/O pin definitions ********************************************/
    #define INPUT_PIN 1
    #define OUTPUT_PIN 0
    		
// Hardware mappings
#if defined(PIC18_EXPLORER) && !defined(HI_TECH_C)
// PIC18 Explorer + Fast 100Mbps Ethernet PICtail Plus or Ethernet PICtail
	// I/O pins
	
	// ENC424J600/624J600 Fast 100Mbps Ethernet PICtail Plus defines
	#define ENC100_INTERFACE_MODE			0		// Uncomment this line to use the ENC424J600/624J600 Ethernet controller (SPI mode) or comment it out to use some other network controller
	
	// ENC100_MDIX, ENC100_POR, and ENC100_INT are all optional.  Simply leave 
	// them commented out if you don't have such a hardware feature on your 
	// board.
//	#define ENC100_MDIX_TRIS				(TRISBbits.TRISB4)
//	#define ENC100_MDIX_IO					(LATBbits.LATB4)
//	#define ENC100_POR_TRIS					(TRISBbits.TRISB5)
//	#define ENC100_POR_IO					(LATBbits.LATB5)
//	#define ENC100_INT_TRIS					(TRISBbits.TRISB2)
//	#define ENC100_INT_IO					(PORTBbits.RB2)

	// ENC424J600/624J600 SPI pinout
	#ifdef METER_TOP_BOARD
		#define ETH_INT_TRIS					(TRISBbits.TRISB4)					
		#define	ETH_INT_IO						(LATBbits.LATB4)
		#define ENC100_CS_TRIS					(TRISBbits.TRISB5)
		#define ENC100_CS_IO					(LATBbits.LATB5)
		#define ENC100_SO_WR_B0SEL_EN_TRIS		(TRISCbits.TRISC4)	// NOTE: SO is ENC624J600 Serial Out, which needs to connect to the PIC SDI pin for SPI mode
		#define ENC100_SO_WR_B0SEL_EN_IO		(PORTCbits.RC4)
		#define ENC100_SI_RD_RW_TRIS			(TRISCbits.TRISC5)	// NOTE: SI is ENC624J600 Serial In, which needs to connect to the PIC SDO pin for SPI mode
		#define ENC100_SI_RD_RW_IO				(LATCbits.LATC5)
		#define ENC100_SCK_AL_TRIS				(TRISCbits.TRISC3)
		#define ENC100_SCK_AL_IO				(PORTCbits.RC3)		// NOTE: This must be the PORT, not the LATch like it is for the PSP interface.
	#elif defined( COMMUNICATOR_BOARD )
		#define ETH_INT_TRIS					(TRISBbits.TRISB4)	// <-					
		#define	ETH_INT_IO						(LATBbits.LATB4)
		#define ENC100_CS_TRIS					(TRISBbits.TRISB5)	// ->
		#define ENC100_CS_IO					(LATBbits.LATB5)
		#define ENC100_SO_WR_B0SEL_EN_TRIS		(TRISCbits.TRISC4)	// NOTE: SO is ENC624J600 Serial Out, which needs to connect to the PIC SDI pin for SPI mode
		#define ENC100_SO_WR_B0SEL_EN_IO		(PORTCbits.RC4)
		#define ENC100_SI_RD_RW_TRIS			(TRISCbits.TRISC5)	// NOTE: SI is ENC624J600 Serial In, which needs to connect to the PIC SDO pin for SPI mode
		#define ENC100_SI_RD_RW_IO				(LATCbits.LATC5)
		#define ENC100_SCK_AL_TRIS				(TRISCbits.TRISC3)
		#define ENC100_SCK_AL_IO				(PORTCbits.RC3)		// NOTE: This must be the PORT, not the LATch like it is for the PSP interface.
	#else
		#error "No board defined."
	#endif
	
	// ENC424J600/624J600 SPI SFR register selection (controls which SPI 
	// peripheral to use on PICs with multiple SPI peripherals).
//	#define ENC100_ISR_ENABLE		(INTCON3bits.INT2IE)
//	#define ENC100_ISR_FLAG			(INTCON3bits.INT2IF)
//	#define ENC100_ISR_POLARITY		(INTCON2bits.INTEDG2)
//	#define ENC100_ISR_PRIORITY		(INTCON3bits.INT2IP)
	#define ENC100_SPI_ENABLE		(ENC100_SPISTATbits.SPIEN)
	#define ENC100_SPI_IF			(PIR1bits.SSPIF)
	#define ENC100_SSPBUF			(SSP1BUF)
	#define ENC100_SPISTAT			(SSP1STAT)
	#define ENC100_SPISTATbits		(SSP1STATbits)
	#define ENC100_SPICON1			(SSP1CON1)
	#define ENC100_SPICON1bits		(SSP1CON1bits)
	#define ENC100_SPICON2			(SSP1CON2)

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISDbits.TRISD7)	// ->
	#define EEPROM_CS_IO		(LATDbits.LATD7)
	
	#define EEPROM_SCK_TRIS		(TRISDbits.TRISD6)	//
	#define EEPROM_SDI_TRIS		(TRISDbits.TRISD5)	// <-
	#define EEPROM_SDO_TRIS		(TRISDbits.TRISD4) 	// -> changed from LAT-TRIS
	
	#define EEPROM_HOLD_TRIS	(TRISJbits.TRISJ1)	//
	#define EEPROM_HOLD_IO		(LATJbits.LATJ1)
	
	#define EEPROM_WP_TRIS		(TRISDbits.TRISD3)	// ->
	#define EEPROM_WP_IO		(LATDbits.LATD3)
	
	#define EEPROM_SPI_IF		(PIR3bits.SSP2IF)
	#define EEPROM_SSPBUF		(SSP2BUF)
	#define EEPROM_SPICON1		(SSP2CON1)	
	#define EEPROM_SPICON1bits	(SSP2CON1bits)
	#define EEPROM_SPICON2		(SSP2CON2)
	#define EEPROM_SPISTAT		(SSP2STAT)
	#define EEPROM_SPISTATbits	(SSP2STATbits)
	
	// LCD I/O pins
	// TODO: Need to add support for LCD behind MCP23S17 I/O expander.  This 
	// requires code that isn't in the TCP/IP stack, not just a hardware 
	// profile change.

	// Serial Flash/SRAM/UART PICtail
//	#define SPIRAM_CS_TRIS			(TRISBbits.TRISB5)
//	#define SPIRAM_CS_IO			(LATBbits.LATB5)
//	#define SPIRAM_SCK_TRIS			(TRISCbits.TRISC3)
//	#define SPIRAM_SDI_TRIS			(TRISCbits.TRISC4)
//	#define SPIRAM_SDO_TRIS			(TRISCbits.TRISC5)
//	#define SPIRAM_SPI_IF			(PIR1bits.SSPIF)
//	#define SPIRAM_SSPBUF			(SSP1BUF)
//	#define SPIRAM_SPICON1			(SSP1CON1)
//	#define SPIRAM_SPICON1bits		(SSP1CON1bits)
//	#define SPIRAM_SPICON2			(SSP1CON2)
//	#define SPIRAM_SPISTAT			(SSP1STAT)
//	#define SPIRAM_SPISTATbits		(SSP1STATbits)
//
//	// NOTE: You must also set the SPI_FLASH_SST/SPI_FLASH_SPANSION, 
//	//       SPI_FLASH_SECTOR_SIZE, and SPI_FLASH_PAGE_SIZE macros in 
//	//       SPIFlash.h to match your particular Flash memory chip!!!
//	#define SPIFLASH_CS_TRIS		(TRISBbits.TRISB4)
//	#define SPIFLASH_CS_IO			(LATBbits.LATB4)
//	#define SPIFLASH_SCK_TRIS		(TRISCbits.TRISC3)
//	#define SPIFLASH_SDI_TRIS		(TRISCbits.TRISC4)
//	#define SPIFLASH_SDI_IO			(PORTCbits.RC4)
//	#define SPIFLASH_SDO_TRIS		(TRISCbits.TRISC5)
//	#define SPIFLASH_SPI_IF			(PIR1bits.SSPIF)
//	#define SPIFLASH_SSPBUF			(SSP1BUF)
//	#define SPIFLASH_SPICON1		(SSP1CON1)
//	#define SPIFLASH_SPICON1bits	(SSP1CON1bits)
//	#define SPIFLASH_SPICON2		(SSP1CON2)
//	#define SPIFLASH_SPISTAT		(SSP1STAT)
//	#define SPIFLASH_SPISTATbits	(SSP1STATbits)

	// Register name fix up for certain processors
	#define SPBRGH				SPBRGH1
	#if defined(__18F87J50) || defined(_18F87J50) || defined(__18F87J11) || defined(_18F87J11)
		#define ADCON2		ADCON1
	#endif

#elif defined(PIC18_EXPLORER) && defined(HI_TECH_C)

#elif defined(HPC_EXPLORER) && !defined(HI_TECH_C)
#elif defined(HPC_EXPLORER) && defined(HI_TECH_C)
#elif defined(PIC24FJ64GA004_PIM)
#elif defined(EXPLORER_16)
	#define ENC100_INTERFACE_MODE			0

	// If using a parallel interface, direct RAM addressing can be used (if all 
	// addresses wires are connected), or a reduced number of pins can be used 
	// for indirect addressing.  If using an SPI interface or PSP Mode 9 or 10 
	// (multiplexed 16-bit modes), which require all address lines to always be 
	// connected, then this option is ignored. Comment out or uncomment this 
	// macro to match your hardware connections.
	#define ENC100_PSP_USE_INDIRECT_RAM_ADDRESSING

	// ENC424J600/624J600 parallel indirect address remapping macro function.
	// This section translates SFR and RAM addresses presented to the 
	// ReadMemory() and WriteMemory() APIs in ENCX24J600.c to the actual 
	// addresses that must be presented on the parallel interface.  This macro 
	// must be modified to match your hardware if you are using an indirect PSP 
	// addressing mode (ENC100_PSP_USE_INDIRECT_RAM_ADDRESSING is defined) and 
	// have some of your address lines tied off to Vdd.  If you are using the 
	// SPI interface, then this section can be ignored or deleted.
	#if (ENC100_INTERFACE_MODE == 1) || (ENC100_INTERFACE_MODE == 2) || (ENC100_INTERFACE_MODE == 5) || (ENC100_INTERFACE_MODE == 6) // 8-bit PSP
		#define ENC100_TRANSLATE_TO_PIN_ADDR(a)		((((a)&0x0100)<<6) | ((a)&0x00FF))
	#elif (ENC100_INTERFACE_MODE == 3) || (ENC100_INTERFACE_MODE == 4) // 16-bit PSP
		#define ENC100_TRANSLATE_TO_PIN_ADDR(a)		(a)
	#endif

	// Auto-crossover pins on Fast 100Mbps Ethernet PICtail/PICtail Plus.  If 
	// your circuit doesn't have such a feature, delete these two defines.
	#define ENC100_MDIX_TRIS				(TRISBbits.TRISB3)
	#define ENC100_MDIX_IO					(LATBbits.LATB3)

	// ENC624J600 I/O control and status pins
	// If a pin is not required for your selected ENC100_INTERFACE_MODE 
	// interface selection (ex: WRH/B1SEL for PSP modes 1, 2, 5, and 6), then 
	// you can ignore, delete, or put anything for the pin definition.  Also, 
	// the INT and POR pins are entirely optional.  If not connected, comment 
	// them out.
	#if defined(__dsPIC33FJ256GP710__) || defined(__PIC24HJ256GP610__)
		#define ENC100_INT_TRIS				(TRISAbits.TRISA13)		// INT signal is optional and currently unused in the Microchip TCP/IP Stack.  Leave this pin disconnected and comment out this pin definition if you don't want it.
		#define ENC100_INT_IO				(PORTAbits.RA13)
	#else
		#define ENC100_INT_TRIS				(TRISEbits.TRISE9)		// INT signal is optional and currently unused in the Microchip TCP/IP Stack.  Leave this pin disconnected and comment out this pin definition if you don't want it.
		#define ENC100_INT_IO				(PORTEbits.RE9)
	#endif
	#if (ENC100_INTERFACE_MODE >= 1)	// Parallel mode
		// PSP control signal pinout
		#define ENC100_CS_TRIS					(TRISAbits.TRISA5)	// CS is optional in PSP mode.  If you are not sharing the parallel bus with another device, tie CS to Vdd and comment out this pin definition.
		#define ENC100_CS_IO					(LATAbits.LATA5)
		#define ENC100_POR_TRIS					(TRISCbits.TRISC1)	// POR signal is optional.  If your application doesn't have a power disconnect feature, comment out this pin definition.
		#define ENC100_POR_IO					(LATCbits.LATC1)
		#define ENC100_SO_WR_B0SEL_EN_TRIS		(TRISDbits.TRISD4)
		#define ENC100_SO_WR_B0SEL_EN_IO		(LATDbits.LATD4)
		#define ENC100_SI_RD_RW_TRIS			(TRISDbits.TRISD5)
		#define ENC100_SI_RD_RW_IO				(LATDbits.LATD5)
		#define ENC100_SCK_AL_TRIS				(TRISBbits.TRISB15)
		#define ENC100_SCK_AL_IO				(LATBbits.LATB15)
	#else	
		// SPI pinout
		#define ENC100_CS_TRIS					(TRISDbits.TRISD14)	// CS is mandatory when using the SPI interface
		#define ENC100_CS_IO					(LATDbits.LATD14)
		#define ENC100_POR_TRIS					(TRISDbits.TRISD15)	// POR signal is optional.  If your application doesn't have a power disconnect feature, comment out this pin definition.
		#define ENC100_POR_IO					(LATDbits.LATD15)
		#define ENC100_SO_WR_B0SEL_EN_TRIS		(TRISFbits.TRISF7)	// SO is ENCX24J600 Serial Out, which needs to connect to the PIC SDI pin for SPI mode
		#define ENC100_SO_WR_B0SEL_EN_IO		(PORTFbits.RF7)
		#define ENC100_SI_RD_RW_TRIS			(TRISFbits.TRISF8)	// SI is ENCX24J600 Serial In, which needs to connect to the PIC SDO pin for SPI mode
		#define ENC100_SI_RD_RW_IO				(LATFbits.LATF8)
		#define ENC100_SCK_AL_TRIS				(TRISFbits.TRISF6)
	#endif


	// ENC624J600 SPI SFR register selection (controls which SPI peripheral to 
	// use on PICs with multiple SPI peripherals).  If a parallel interface is 
	// used (ENC100_INTERFACE_MODE is >= 1), then the SPI is not used and this 
	// section can be ignored or deleted.
	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
	#else					// PIC32MX
		#define ENC100_ISR_ENABLE		(IEC0bits.INT2IE)
		#define ENC100_ISR_FLAG			(IFS0bits.INT2IF)
		#define ENC100_ISR_POLARITY		(INTCONbits.INT2EP)	
		#define ENC100_ISR_PRIORITY		(IPC2bits.INT2IP)	
		#define ENC100_SPI_ENABLE		(ENC100_SPICON1bits.ON)
		#define ENC100_SPI_IF			(IFS0bits.SPI1RXIF)
		#define ENC100_SSPBUF			(SPI1BUF)
		#define ENC100_SPICON1			(SPI1CON)
		#define ENC100_SPISTATbits		(SPI1STATbits)
		#define ENC100_SPICON1bits		(SPI1CONbits)
		#define ENC100_SPIBRG			(SPI1BRG)
	#endif

	// 25LC256 I/O pins
	#define EEPROM_CS_TRIS		(TRISDbits.TRISD12)
	#define EEPROM_CS_IO		(PORTDbits.RD12)
	#define EEPROM_SCK_TRIS		(TRISGbits.TRISG6)
	#define EEPROM_SDI_TRIS		(TRISGbits.TRISG7)
	#define EEPROM_SDO_TRIS		(TRISGbits.TRISG8)
	#if defined(__C30__)	// PIC24F, PIC24H, dsPIC30, dsPIC33
	#else					// PIC32
		#define EEPROM_SPI_IF		(IFS1bits.SPI2RXIF)
		#define EEPROM_SSPBUF		(SPI2BUF)
		#define EEPROM_SPICON1		(SPI2CON)
		#define EEPROM_SPICON1bits	(SPI2CONbits)
		#define EEPROM_SPIBRG		(SPI2BRG)
		#define EEPROM_SPISTAT		(SPI2STAT)
		#define EEPROM_SPISTATbits	(SPI2STATbits)
	#endif

	// LCD Module I/O pins.  NOTE: On the Explorer 16, the LCD is wired to the 
	// same PMP lines required to communicate with an ENCX24J600 in parallel 
	// mode.  Since the LCD does not have a chip select wire, if you are using 
	// the ENC424J600/624J600 in parallel mode, the LCD cannot be used.
	#if !defined(ENC100_INTERFACE_MODE) || (ENC100_INTERFACE_MODE == 0)	// SPI only
		#define LCD_DATA_TRIS		(*((volatile BYTE*)&TRISE))
		#define LCD_DATA_IO			(*((volatile BYTE*)&LATE))
		#define LCD_RD_WR_TRIS		(TRISDbits.TRISD5)
		#define LCD_RD_WR_IO		(LATDbits.LATD5)
		#define LCD_RS_TRIS			(TRISBbits.TRISB15)
		#define LCD_RS_IO			(LATBbits.LATB15)
		#define LCD_E_TRIS			(TRISDbits.TRISD4)
		#define LCD_E_IO			(LATDbits.LATD4)
	#endif


#elif defined(DSPICDEM11)
#elif defined(PICDEMNET2) && !defined(HI_TECH_C)
#elif defined(PICDEMNET2) && defined(HI_TECH_C)
#elif defined(INTERNET_RADIO) // Internet Radio board
#elif defined(PIC32_STARTER_KIT)
#elif defined(PIC32_ETH_STARTER_KIT)
#elif defined(YOUR_BOARD)
// Define your own board hardware profile here
#else
	#error "Hardware profile not defined.  See available profiles in HardwareProfile.h and modify or create one."
#endif

#if defined(__18CXX)	// PIC18
	// UART mapping functions for consistent API names across 8-bit and 16 or 
	// 32 bit compilers.  For simplicity, everything will use "UART" instead 
	// of USART/EUSART/etc.
	#define BusyUART()				BusyUSART()
	#define CloseUART()				CloseUSART()
	#define ConfigIntUART(a)		ConfigIntUSART(a)
	#define DataRdyUART()			DataRdyUSART()
	#define OpenUART(a,b,c)			OpenUSART(a,b,c)
	#define ReadUART()				ReadUSART()
	#define WriteUART(a)			WriteUSART(a)
	#define getsUART(a,b,c)			getsUSART(b,a)
	#define putsUART(a)				putsUSART(a)
	#define getcUART()				ReadUSART()
	#define putcUART(a)				WriteUSART(a)
	#define putrsUART(a)			putrsUSART((far rom char*)a)

#else	 // PIC24F, PIC24H, dsPIC30, dsPIC33, PIC32
	// Select which UART the STACK_USE_UART and STACK_USE_UART2TCP_BRIDGE 
	// options will use.  You can change these to U1BRG, U1MODE, etc. if you 
	// want to use the UART1 module instead of UART2.
	#define UBRG					U2BRG
	#define UMODE					U2MODE
	#define USTA					U2STA
	#define BusyUART()				BusyUART2()
	#define CloseUART()				CloseUART2()
	#define ConfigIntUART(a)		ConfigIntUART2(a)
	#define DataRdyUART()			DataRdyUART2()
	#define OpenUART(a,b,c)			OpenUART2(a,b,c)
	#define ReadUART()				ReadUART2()
	#define WriteUART(a)			WriteUART2(a)
	#define getsUART(a,b,c)			getsUART2(a,b,c)
	#if defined(__C32__)
	#else
		#define putsUART(a)			putsUART2((unsigned int*)a)
	#endif
	#define getcUART()				getcUART2()
	#define putcUART(a)				do{while(BusyUART()); WriteUART(a); while(BusyUART()); }while(0)
	#define putrsUART(a)			putrsUART2(a)
#endif


#endif

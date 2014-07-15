/*********************************************************************
 *
 *  Main Application Entry Point and TCP/IP Stack Demo
 *  Module for Microchip TCP/IP Stack
 *   -Demonstrates how to call and use the Microchip TCP/IP stack
 *	 -Reference: AN833
 *
 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
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
 *qqwertyerqwqfffffggfdfgfdgfdfg
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
 * Author              Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti		4/19/01		Original (Rev. 1.0)
 * Nilesh Rajbharti		2/09/02		Cleanup
 * Nilesh Rajbharti		5/22/02		Rev 2.0 (See version.log for detail)
 * Nilesh Rajbharti		7/9/02		Rev 2.1 (See version.log for detail)
 * Nilesh Rajbharti		4/7/03		Rev 2.11.01 (See version log for detail)
 * Howard Schlunder		10/1/04		Beta Rev 0.9 (See version log for detail)
 * Howard Schlunder		10/8/04		Beta Rev 0.9.1 Announce support added
 * Howard Schlunder		11/29/04	Beta Rev 0.9.2 (See version log for detail)
 * Howard Schlunder		2/10/05		Rev 2.5.0
 * Howard Schlunder		1/5/06		Rev 3.00
 * Howard Schlunder		1/18/06		Rev 3.01 ENC28J60 fixes to TCP, 
 *									UDP and ENC28J60 files
 * Howard Schlunder		3/01/06		Rev. 3.16 including 16-bit micro support
 * Howard Schlunder		4/12/06		Rev. 3.50 added LCD for Explorer 16
 * Howard Schlunder		6/19/06		Rev. 3.60 finished dsPIC30F support, added PICDEM.net 2 support
 * Howard Schlunder		8/02/06		Rev. 3.75 added beta DNS, NBNS, and HTTP client (GenericTCPClient.c) services
 * Howard Schlunder		12/28/06	Rev. 4.00RC added SMTP, Telnet, substantially modified TCP layer
 * Howard Schlunder		04/09/07	Rev. 4.02 added TCPPerformanceTest, UDPPerformanceTest, Reboot and fixed some bugs
 * Howard Schlunder		xx/xx/07	Rev. 4.03
 * HSchlunder & EWood	08/27/07	Rev. 4.11
 * HSchlunder & EWood	10/08/07	Rev. 4.13
 * HSchlunder & EWood	11/06/07	Rev. 4.16
 * HSchlunder & EWood	11/08/07	Rev. 4.17
 * HSchlunder & EWood	11/12/07	Rev. 4.18
 * HSchlunder & EWood	02/11/08	Rev. 4.19
 * HSchlunder & EWood   04/26/08    Rev. 4.50 Moved most code to other files for clarity
 * KHesky               07/07/08    Added ZG2100-specific support
 * HSchlunder & EWood   07/24/08    Rev. 4.51
 * Howard Schlunder		11/10/08    Rev. 4.55
 * Howard Schlunder		04/14/09    Rev. 5.00
 * Howard Schlunder		07/10/09    Rev. 5.10
 * Howard Schlunder		11/18/09    Rev. 5.20
 ********************************************************************/
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION

// Uncomment this to set the hardware to use the correct settings.
#define APP_USE_TCPIP

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"

#include "HardwareProfile.h"
#include "AppConfig.h"
#include "ModbusTCPClient.h"
#include "eeprom_task.h"

#ifdef APP_USE_ZIGBEE
	#include "zigbee.h"
#endif

// Include functions specific to this stack application
#include "Main_Transceiver.h"

/** V A R I A B L E S ********************************************************/
//#ifdef APP_USE_USB
#pragma udata
unsigned char    LastRS232Out;  // Number of characters in the buffer
unsigned char    RS232cp;       // current position within the buffer
unsigned char 	 RS232_Out_Data_Rdy = 0;
CONNECTED_METERS_LIST   iplist;
//#endif

#ifdef APP_USE_ROUTER_CODE
	static	unsigned int		GENERAL_COUNTER = 0;
	char				IS_CONNECTED_TO_SERVER;
#endif

static unsigned char is_MAC_powered_up;

// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
BYTE AN0String[8];


/** P R O T O T Y P E s ********************************************************/
void InitializeUSART(void);
unsigned char getcUSART(void);

// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitializeBoard(void);
void InitIPLIST(void);
void SaveIPLIST(void);

#if defined(__18CXX)
    #define mDataRdyUSART() PIR1bits.RC1IF
    #define mTxRdyUSART()   TXSTA2bits.TRMT
#elif defined(__C30__) || defined(__C32__)
    #define mDataRdyUSART() UART2IsPressed()
    #define mTxRdyUSART()   U2STAbits.TRMT
#endif



//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#if defined(__18CXX)
	#pragma interruptlow LowISR
	void LowISR(void)
	{
	    TickUpdate();
	}
	
	#pragma interrupt HighISR
	void HighISR(void)
	{
		#ifdef APP_USE_ZIGBEE
		if( ZIGBEE_RX_INTERRUPT_FLAG ) ZigbeeUnloadData();
		#endif
	}

	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif
#endif

//
// Main application entry point.
//
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
	unsigned char counter = 0;	
	static DWORD t = 0, t2 = 0, t3 = TICK_MINUTE, t4 = 0;
	static DWORD dwLastIP = 0;
	
	// Enable PLL.
	{
        unsigned int pll_startup_counter = 1000;
        OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
        while(pll_startup_counter--);
    }
	
	// Light on BLUE LED
	{
		long count = 0;
		LED0_IO = 1;
		while(count++ < 200000);
		LED0_IO = 0;
	}
	
	// Initialize application specific hardware
	InitializeBoard();
	InitEEPROMSavePower();
	ResetMemory();

	// Initialize stack-related hardware components that may be 
	// required by the UART configuration routines
    TickInit();
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
	MPFSInit();
	#endif

	// Initialize Stack and application related NV variables into AppConfig.
	InitAppConfig();
	InitIPLIST();

    // Initiates board setup process if button is depressed 
	// on startup
    if(BUTTON0_IO == 0u)
    {
		#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
		// Invalidate the EEPROM contents if BUTTON0 is held down for more than 4 seconds
		DWORD StartTime = TickGet();
//		LED_PUT(0x00);

		#ifdef COMMUNICATOR_BOARD
		#elif defined( SINGLEPHASEMETER_MCU1 )
		while(BUTTON0_IO == 0u)
		{
			if(TickGet() - StartTime > 4*TICK_SECOND)
			{
				#if defined(EEPROM_CS_TRIS)
			    XEEBeginWrite(0x0000);
			    XEEWrite(0xFF);
			    XEEEndWrite();
			    #elif defined(SPIFLASH_CS_TRIS)
			    SPIFlashBeginWrite(0x0000);
			    SPIFlashWrite(0xFF);
			    #endif
			    
				#if defined(STACK_USE_UART)
				putrsUART("\r\n\r\nBUTTON0 held for more than 4 seconds.  Default settings restored.\r\n\r\n");
				#endif

//				LED_PUT(0x0F);
				while((LONG)(TickGet() - StartTime) <= (LONG)(9*TICK_SECOND/2));
//				LED_PUT(0x00);
				while(BUTTON0_IO == 0u);
				Reset();
				break;
			}
		}
		#else
			#error "No board defined."
		#endif
		#endif
    }

	// Initialize core stack layers (MAC, ARP, TCP, UDP) and
	// application modules (HTTP, SNMP, etc.)
    StackInit();

	// Initialize any application-specific modules or functions/
	// For this demo application, this only includes the
	// UART 2 TCP Bridge
	#if defined(STACK_USE_UART2TCP_BRIDGE)
	UART2TCPBridgeInit();
	#endif
	
	// Critical Delaysince there is a timing issue between PIC and ZIGBEE speed.
	{
		ZB_RESET_TRIS = 0;
		ZB_RESET = 0;
		Delay100TCYx(1);
		ZB_RESET = 1;
	}
	
	Delay100TCYx(1);
//	Delay10KTCYx(10);
	
	#ifdef APP_USE_ZIGBEE
		ZigbeeOpen();
		ZigbeeSetup();
	#else
		//#error no zigbee.
	#endif

// ROUTER CODE.
#ifdef APP_USE_ROUTER_CODE
{
	GENERAL_COUNTER = 0;
	while( GENERAL_COUNTER++ < 2 )
	{
		Zigbee_UpdateNetworkInfo(&SERIAL_NUMBER, &NETWORK_ADDRESS, 0, &DEVICE_TYPE);
		if( NETWORK_ADDRESS[0] != 0 && strcmpram2pgm("FFFE", &NETWORK_ADDRESS) != 0 )
		{
			IS_CONNECTED_TO_SERVER = 1;
			break;
		}	
	}	
}	
#endif
// END ROUTER CODE.
	
	// Initialize variables
//	iRemoteNode.IPAddr.Val = 0;
	is_MAC_powered_up = 1;
	iplist.Val.repeat_interval = 1;
	
	#ifdef COMMUNICATOR_BOARD
//		XEEBeginRead(TELNET_INTERVAL);
//		if(XEERead() == 0x30)
//			telnet_timer = XEERead();
//		else
//		{
//			// Set time delay for telnet
//			telnet_timer = 1;
//									
//			// Save value to eeprom
//			XEEBeginWrite(TELNET_INTERVAL);
//	    	XEEWrite(0x30);
//	    	XEEWrite(0x01);	 
//	    	XEEEndWrite();
//		}	
	#endif

	// Now that all items are initialized, begin the co-operative
	// multitasking loop.  This infinite loop will continuously 
	// execute all stack-related tasks, as well as your own
	// application's functions.  Custom functions should be added
	// at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while(1)
    {
	    ClrWdt();
	     
       	/**********************************************/
	    /**** Handle Zigbee ******************************/
	    /**********************************************/
	    #ifdef APP_USE_ZIGBEE
	    	ZigbeeTasks();
	    #endif
	    
		#if defined(APP_USE_MODBUS_CLIENT)
		ModbusRequestTask();
		#endif
	    
	    // Blink LED0 (right most one) every second.
        if(TickGet() - t >= TICK_SECOND/2ul)
        {
            t = TickGet();
            LED0_IO ^= 1;
        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();
        
        // This tasks invokes each of the core stack application tasks
        StackApplications();
		
		#ifdef APP_USE_ZIGBEE
	   	ZigbeeTasks();
	    #endif
		
		// Process application specific tasks here.
		// For this demo app, this will include the Generic TCP 
		// client and servers, and the SNMP, Ping, and SNMP Trap
		// demos.  Following that, we will process any IO from
		// the inputs on the board itself.
		// Any custom modules or processing you need to do should
		// go here.

		#if defined(STACK_USE_SMTP_CLIENT)
		SMTPDemo();
		#endif
		
		#if defined(STACK_USE_ICMP_CLIENT)
		PingDemo();
		#endif
		
		#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
		SNMPTrapDemo();
		if(gSendTrapFlag)
			SNMPSendTrap();
		#endif
		
		#if defined(STACK_USE_BERKELEY_API)
		BerkeleyTCPClientDemo();
		BerkeleyTCPServerDemo();
		BerkeleyUDPClientDemo();
		#endif
		
		// Check to see if we need to save iplist.
		if( iplist.Val.Flags.bIsModified == 1u )
			SaveIPLIST();
		// END - Check to see if we need to save iplist.
		
		// If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce 
        // service
		if(dwLastIP != AppConfig.MyIPAddr.Val)
		{
			dwLastIP = AppConfig.MyIPAddr.Val;
			#if defined(STACK_USE_ANNOUNCE)
				AnnounceIP();
			#endif
		}
	}
}

/******************************************************************************
 * Function:        void putcUSART(char c)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          unsigned char c - character to received on the UART
 *
 * Side Effects:    None
 *
 * Overview:        Print the input character to the UART
 *
 * Note:            
 *
 *****************************************************************************/
unsigned char getcUSART ()
{
	char  c;

    #if defined(__18CXX)

	if (RCSTAbits.OERR)  // in case of overrun error
	{                    // we should never see an overrun error, but if we do, 
		RCSTAbits.CREN = 0;  // reset the port
		c = RCREG;
		RCSTAbits.CREN = 1;  // and keep going.
	}
	else
		c = RCREG;
// not necessary.  EUSART auto clears the flag when RCREG is cleared
//	PIR1bits.RCIF = 0;    // clear Flag

    #endif

    #if defined(__C30__) || defined(__C32__)
        c = UART2GetChar();
    #endif

	return c;
}
/******************************************************************************
 * Function:        void putcUSART(char c)
 *
 * PreCondition:    None
 *
 * Input:           char c - character to print to the UART
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Print the input character to the UART
 *
 * Note:            
 *
 *****************************************************************************/
void putcUSART(char c)  
{
    #if defined(__18CXX)
	    TXREG = c;
    #else
        UART2PutChar(c);
    #endif
}

/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/
static void InitializeBoard(void)
{
	// Board Type.
	TRISEbits.TRISE0 = TRISEbits.TRISE1 = 1;
	
	// LEDs
	LED0_TRIS = 0;
	LED1_TRIS = 0;
	LED2_TRIS = 0;
	LED0_IO = 0;
	LED1_IO = 0;
	LED2_IO = 0;
	
	// BUTTON
	BUTTON_INT_TRIS = 1;
	
	// Mini USB
	tris_usb_bus_sense = 1;
	
	// EEPROM
	EEPROM_HOLD_TRIS = 0;
	EEPROM_HOLD_IO = 1;
	EEPROM_WP_TRIS = 0;
	EEPROM_WP_IO = 1;
		
#if defined(__18CXX)
	// Enable 4x/5x/96MHz PLL on PIC18F87J10, PIC18F97J60, PIC18F87J50, etc.
    OSCTUNE = 0x40;

	// Set up analog features of PORTA

	// PICDEM.net 2 board has POT on AN2, Temp Sensor on AN3
	#if defined(__18F87J11) || defined(_18F87J11) || defined(__18F87J50) || defined(_18F87J50)
		ADCON0 = 0x01;		// ADON, Channel 0, Vdd/Vss is +/-REF
		WDTCONbits.ADSHR = 1;
		ANCON0 = 0xFC;		// AN0 (POT) and AN1 (temp sensor) are anlog
		ANCON1 = 0xFF;
		WDTCONbits.ADSHR = 0;		
	#endif
	ADCON2 = 0xBE;		// Right justify, 20TAD ACQ time, Fosc/64 (~21.0kHz)


    // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;

	// Configure USART (using EUSART2) for ZIGBEE
    BAUDCON2bits.DTRXP = 1;
    BAUDCON2bits.SCKP = 1;
    TXSTA2 = 0x20;
    RCSTA2 = 0x90;
	
	// See if we can use the high baud rate setting
	#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
		TXSTAbits.BRGH = 1;
	#else	// Use the low baud rate setting
		SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
	#endif


	// Enable Interrupts
	RCONbits.IPEN = 1;		// Enable interrupt priorities
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Do a calibration A/D conversion
	#if defined(__18F87J11) || defined(__18F86J16) || defined(__18F86J11) || defined(__18F67J11) || defined(__18F66J16) || defined(__18F66J11) || \
		   defined(_18F87J11) ||  defined(_18F86J16) ||  defined(_18F86J11) ||  defined(_18F67J11) ||  defined(_18F66J16) ||  defined(_18F66J11) || \
		  defined(__18F87J50) || defined(__18F86J55) || defined(__18F86J50) || defined(__18F67J50) || defined(__18F66J55) || defined(__18F66J50) || \
		   defined(_18F87J50) ||  defined(_18F86J55) ||  defined(_18F86J50) ||  defined(_18F67J50) ||  defined(_18F66J55) ||  defined(_18F66J50)
		   ADCON1bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON1bits.ADCAL = 0;
	#endif
#endif

#if defined(SPIRAM_CS_TRIS)
	SPIRAMInit();
#endif
#if defined(EEPROM_CS_TRIS)
	XEEInit();
#endif
#if defined(SPIFLASH_CS_TRIS)
	SPIFlashInit();
#endif

//	MACPowerUp();
}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and 
// Serialized Quick Turn Programming (SQTP). 
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling 
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C 
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
	AppConfig.Flags.bIsDHCPEnabled = FALSE;
	AppConfig.Flags.bInConfigMode = TRUE;
	memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//	{
//		_prog_addressT MACAddressAddress;
//		MACAddressAddress.next = 0x157F8;
//		_memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//	}
	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;


	// SNMP Community String configuration
	#if defined(STACK_USE_SNMP_SERVER)
	{
		BYTE i;
		static ROM char * ROM cReadCommunities[] = SNMP_READ_COMMUNITIES;
		static ROM char * ROM cWriteCommunities[] = SNMP_WRITE_COMMUNITIES;
		ROM char * strCommunity;
		
		for(i = 0; i < SNMP_MAX_COMMUNITY_SUPPORT; i++)
		{
			// Get a pointer to the next community string
			strCommunity = cReadCommunities[i];
			if(i >= sizeof(cReadCommunities)/sizeof(cReadCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_READ_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(AppConfig.readCommunity[0]))
				while(1);
			
			// Copy string into AppConfig
			strcpypgm2ram((char*)AppConfig.readCommunity[i], strCommunity);

			// Get a pointer to the next community string
			strCommunity = cWriteCommunities[i];
			if(i >= sizeof(cWriteCommunities)/sizeof(cWriteCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_WRITE_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(AppConfig.writeCommunity[0]))
				while(1);

			// Copy string into AppConfig
			strcpypgm2ram((char*)AppConfig.writeCommunity[i], strCommunity);
		}
	}
	#endif

	// Load the default NetBIOS Host Name
	memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	FormatNetBIOSName(AppConfig.NetBIOSName);

	#if defined(ZG_CS_TRIS)
		// Load the default SSID Name
		if (sizeof(MY_DEFAULT_SSID_NAME) > sizeof(AppConfig.MySSID))
		{
			ZGErrorHandler((ROM char *)"AppConfig.MySSID[] too small");
		}
		memcpypgm2ram(AppConfig.MySSID, (ROM void*)MY_DEFAULT_SSID_NAME, sizeof(MY_DEFAULT_SSID_NAME));
	#endif

	#if defined(EEPROM_CS_TRIS)
	{
		BYTE c;
		
	    // When a record is saved, first byte is written as 0x60 to indicate
	    // that a valid record was saved.  Note that older stack versions
		// used 0x57.  This change has been made to so old EEPROM contents
		// will get overwritten.  The AppConfig() structure has been changed,
		// resulting in parameter misalignment if still using old EEPROM
		// contents.
		
		// TCPIP configuration settings will be moved to start from 0x0050.
		// The first 80 bytes will be used for application settings.
		XEEReadArray(0x0000, &c, 1);
	    if(c == 0x60u)
	    {
		    APP_CONFIG newAppConfig;
		    XEEReadArray(0x0001, (BYTE*)&newAppConfig, sizeof(newAppConfig));
		    AppConfig = newAppConfig;
	    }
	    else
	        SaveAppConfig();
	}
	#elif defined(SPIFLASH_CS_TRIS)
	{
		BYTE c;
		
		SPIFlashReadArray(0x0000, &c, 1);
		if(c == 0x60u)
			SPIFlashReadArray(0x0001, (BYTE*)&AppConfig, sizeof(AppConfig));
		else
			SaveAppConfig();
	}
	#endif
}

#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
void SaveAppConfig(void)
{
	// Ensure adequate space has been reserved in non-volatile storage to 
	// store the entire AppConfig structure.  If you get stuck in this while(1) 
	// trap, it means you have a design time misconfiguration in TCPIPConfig.h.
	// You must increase MPFS_RESERVE_BLOCK to allocate more space.
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
		if(sizeof(AppConfig) > MPFS_RESERVE_BLOCK)
			while(1);
	#endif

	#if defined(EEPROM_CS_TRIS)
	    XEEBeginWrite(0x0000);
	    XEEWrite(0x60);
	    XEEWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
    #else
	    SPIFlashBeginWrite(0x0000);
	    SPIFlashWrite(0x60);
	    SPIFlashWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
    #endif
}

void InitIPLIST(void)
{
	BYTE * cnfg = (BYTE*)&iplist;
	DWORD cursor = IP_SPACE;
	unsigned char i = 0;
	
	#if defined(EEPROM_CS_TRIS)
	{
		XEEBeginRead(cursor);
		if(XEERead() == 0x60)	// If valid data is found, Read data from EEPROM.
			XEEReadArray(cursor+1, (BYTE*)&iplist, sizeof(iplist));
		else	// No valid data is found.  
		{
			for( i=0; i<sizeof(iplist); i++ )	// Initialise the IP_LIST variable and save it to EEPROM.
				cnfg[i] = 0;
			
			SaveIPLIST();
		}
	}	
	#else
	{
		BYTE c;
		
		SPIFlashReadArray(cursor, &c, 1);
		if(c == 0x60u)
			SPIFlashReadArray(cursor+1, (BYTE*)&iplist, sizeof(iplist));
		else
		{
			for( i=0; i<sizeof(iplist); i++ )	// Initialise the IP_LIST variable and save it to EEPROM.
				cnfg[i] = 0;
				
			SaveIPLIST();
		}
	}	
	#endif
}	

void SaveIPLIST(void)
{
	#if defined(EEPROM_CS_TRIS)
	    XEEBeginWrite(IP_SPACE);
	    XEEWrite(0x60);
	    XEEWriteArray((BYTE*)&iplist, sizeof(iplist));
    #else
	    SPIFlashBeginWrite(IP_SPACE);
	    SPIFlashWrite(0x60);
	    SPIFlashWriteArray((BYTE*)&iplist, sizeof(iplist));
    #endif
    
    iplist.Val.Flags.bIsModified = 0;
}	
#endif

/******************************************************************************
 * Function:        void InitializeUSART(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine initializes the UART to 19200
 *
 * Note:            
 *
 *****************************************************************************/
void InitializeUSART(void)
{
	// Initialise Zigbee here.
}//end InitializeUSART

//#if defined(__18CXX)
//    #define mDataRdyUSART() PIR1bits.RC1IF
//    #define mTxRdyUSART()   TXSTAbits.TRMT
//#elif defined(__C30__) || defined(__C32__)
//    #define mDataRdyUSART() UART2IsPressed()
//    #define mTxRdyUSART()   U2STAbits.TRMT
//#endif


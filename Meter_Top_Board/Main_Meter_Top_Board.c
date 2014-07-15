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
#include "Compiler.h"
#include "GenericTypeDefs.h"

#include "AppConfig.h"
#include "registers.h"
#include "HardwareProfile.h"
#include "ModbusTCPServer_Meter.h"
#include "utility.h"
//#include "CommReboot.h"	//2012-06-23 Liz: Reboot comm
#include "eeprom_task.h"	//2012-09-24 Liz: Eeprom record 

#ifdef APP_USE_LCD
	#include "lcd.h"
#endif
#ifdef APP_USE_MCU
	#include "mcu.h"
#endif
#ifdef APP_USE_MENU
	#include "menu.h"
#endif

// Include functions specific to this stack application
#include "Main_Meter_Top_Board.h"

// Provides a way for the C# bootloader software to identify the device the hex file was compiled for.
// This will appear in the hex file as :04010000<dwDeviceID><checksum>
// where the first 2 and last 2 hex digits of dwDeviceID will always be FF.
// dwDeviceID = 0xFF0001FF is for PIC18F46K20, bottom board.
// dwDeviceID = 0xFF0010FF is for PIC18F87J50, top board.
#pragma romdata DevID = 0x100
far rom DWORD dwDeviceID = BOOTLOAD_DEVID;	// 0xF000EF02 is a goto 0x000004 instruction
#pragma romdata

/** V A R I A B L E S ********************************************************/
// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
METER_STATUS_FLAG meter_flag;	//2012-10-02 Liz added

char strIPAddress[16] = "";
char strMACAddress[16] = "";
static short int button_reset_old_state;
char Credit_Balance[17] = "NA";
long EEPROM_INTERVAL = 0;	// 2013-09-26 Liz added

// 2012-06-10(Eric) - Variable for force WDT reset.
static DWORD dwForce_wdt = 0;
static BOOL Do_Reset = FALSE;	// 2013-09-18 Liz moved from CustomClearWdt() function.
//static BOOL Save_Flag_to_EEP = FALSE;	// 2013-09-20 Liz. Save flag in eeprom to mark that meter reset dut to 12-hour-reset

//char DOWNLOAD_IN_PROGRESS = 0;
/** P R O T O T Y P E s ********************************************************/
void UserInit(void);
void InitializeUSART(void);
void ResetControl(void);
void CustomClearWdt(void);

// Private helper functions.
// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitializeBoard(void);
void LowISR(void);
void HighISR(void);

//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#if defined(__18CXX)

	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif
	
	#pragma interruptlow LowISR
	void LowISR(void)
	{
	    TickUpdate();
	}
	#pragma code
	
	#pragma interrupt HighISR
	void HighISR(void)
	{
		char backup = INTCON;
		INTCON = 0;
		
		#ifdef APP_USE_ZIGBEE
		if( ZIGBEE_RX_INTERRUPT_FLAG )
		{
			char i=0;	
		}
		#endif
		
		#ifdef APP_USE_MCU
		if( MCU_RX_INTERRUPT_FLAG )
			MCUUnloadData();
		#endif

		INTCON = backup;
	}
	#pragma code
	
#endif

//
// Main application entry point.
//
void main(void)
{
	unsigned char counter = 0;

	static DWORD t = 0, EEPROM_RECORD_TIMER = 0;
	static DWORD dwLastIP = 0;
	
	static BYTE c_cb = 0;
	static BYTE verifyStr_cb[20] = "";
	static BYTE retries_cb = 0;
	
	BUZZER = 0;
	Delay10KTCYx(70);

	// Initialize application specific hardware
	InitializeBoard();
	
	// Initialize stack-related hardware components that may be 
	// required by the UART configuration routines
    TickInit();
	
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
		MPFSInit();
	#endif

	// Initialize Stack and application related NV variables into AppConfig.
	InitAppConfig();

	#ifdef APP_USE_BUZZER
		// 2012-06-10(Eric) - Do not sound buzzer if we are coming back from a WDT reset.
		if(RCONbits.NOT_TO == 1)
		{
			BUZZER = 1;
			{ unsigned long i=0; while( i++ < 106454 ); }
			BUZZER = 0;
		}
	#endif	

	// Initialize core stack layers (MAC, ARP, TCP, UDP) and
	// application modules (HTTP, SNMP, etc.)
    StackInit();

	// 2012-06-10(Eric) - Variable for force WDT reset.
	dwForce_wdt = TickGetDiv256();

	// Initialize any application-specific modules or functions/
	// For this demo application, this only includes the
	// UART 2 TCP Bridge
	#if defined(STACK_USE_UART2TCP_BRIDGE)
		UART2TCPBridgeInit();
	#endif
	
	#ifdef METER_TOP_BOARD
		MCUOpen();
	#endif
	
	// Initialize global variables
    #ifdef APP_USE_MENU
    	setMenuCurrentState(MENU_RESET_STATE);
    	set_current_menu(0);
    	set_button_state();
	#endif
	button_reset_old_state = 0;
	
	// Init reading storage
	{ 
		unsigned char i, j, length;
		
		for(i=0; i<4; i++)
		{
			for(j=0; j<POWER_REGISTERS_QUEUE_SIZE; j++)
			{	
				int * phase_storage = POWER_READINGS[i];
				long * kk = phase_storage[j];

				length = POWER_REGISTERS_SIZE_QUEUE[i][j];
				
				// Init variables
				if( (void*)kk != 0 )
				{	
					kk[0] = 0;	
					if(length >= 8)	kk[1] = 0;
				}		
			}	
			
		}					
	}

	// Init eeprom
	InitEEPROMSavePower();
//	ResetMemory();

	/////////////////////////////////////////////////////////////////////////////////
	//2013-02-21 Liz Need to check if it is auto reset every 12 hours.
		if(RCONbits.NOT_TO == 0)
		{
			//2013-03-28 Liz. Added to check if MCU reset because of watchdog timeout	
			meter_flag.Flags.Flag2.bTOPTOReset = 1;	//2013-03-28 Liz
			meter_flag.Flags.Flag1.bTOPReset = 0;
			meter_flag.Flags.Flag2.bTOPAutoReset = 0;
			meter_flag.Flags.Flag2.bBOTAutoReset = 0;
		}
		else if(RCONbits.NOT_RI == 0)
		{
			meter_flag.Flags.Flag2.bTOPTOReset = 0;
			meter_flag.Flags.Flag1.bTOPReset = 0;
			meter_flag.Flags.Flag2.bTOPAutoReset = 1;
			meter_flag.Flags.Flag2.bBOTAutoReset = 1;	
		}	 	
		else
		{
			meter_flag.Flags.Flag2.bTOPTOReset = 0;	//2013-03-28 Liz
			meter_flag.Flags.Flag1.bTOPReset = 1;
			meter_flag.Flags.Flag2.bTOPAutoReset = 0;	
			meter_flag.Flags.Flag2.bBOTAutoReset = 0;
		}	
	///////////////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////////////
	// 2012-07-18 Liz: stored credit balance in eeprom
	// Init value for credit balance
	// 2012-10-16 Liz removed. not in used
	// 2013-09-25 Liz added back.
	{
		BYTE c = 0;
		
		XEEBeginRead(CREDIT_BALANCE_EEP_POS);
		c = XEERead();
		XEEEndRead();
		
		// Simple credit balance value verifying
		if(c == 0x80)
			XEEReadArray(CREDIT_BALANCE_EEP_POS+1, &Credit_Balance[0], sizeof(Credit_Balance));
		else
			memcpypgm(&Credit_Balance[0], "NA", 2);
	}
	//////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// 2013-09-20 Liz added. Check if meter reset due to 12-hour-auto-reset
	{
		BYTE c = 0;
		
		// Get value stored in eeprom
		XEEBeginRead(TWELVE_HOUR_RESET_FLAG);
		c = XEERead();
		XEEEndRead();
		
		// Check if meter is reset due to 12-hour-auto-reset
		if(c == 0x52) 
		{
			meter_flag.Flags.Flag2.bTOPAutoReset = 1;
			meter_flag.Flags.Flag2.bBOTAutoReset = 1;
			meter_flag.Flags.Flag1.bTOPReset = 0;
		}
		else
		{
			meter_flag.Flags.Flag2.bTOPAutoReset = 0;
			meter_flag.Flags.Flag2.bBOTAutoReset = 0;
		}
		
		// Clear the flag in eeprom
		XEEBeginWrite(TWELVE_HOUR_RESET_FLAG);
		XEEWrite(0x00);	// Write 'R' to indicate that meter had been reset due to 12-hour-auto-reset
		XEEEndWrite();		
	}	
	///////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// 2013-09-25 Liz added. 
	//	Load the setting of eeprom record interval from eeprom
	{
		BYTE c = 0;
		
		// Get value stored in eeprom
		XEEBeginRead(EEPROM_INTERVAL_POS);
		c = XEERead();
		XEEEndRead();
		
		if(c == 0x80)
		{
			XEEBeginRead(EEPROM_INTERVAL_POS+1);
			XEEReadArray(EEPROM_INTERVAL_POS+1, &EEPROM_INTERVAL, 4);
			XEEEndRead();
		}	
		else
			EEPROM_INTERVAL = 30ul;
	//		EEPROM_INTERVAL = 2ul; // 2014-07-20_cl test_for debugging purpose, change the EEPROM interval to one minute
	}	
	///////////////////////////////////////////////////////////////////////
	LED0_IO = 1;
	///////////////////////////////////////////////////////////////////////
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
	    //CustomClearWdt();
	    ResetControl();
	    
	    #if defined( APP_USE_MENU )
		menu_task();
	    #endif

		#if defined(APP_METER_USE_MODBUS_SERVER)
		Modbus_MeterServerTask();
		#endif

		#if defined(APP_METER_USE_MODBUS_CLIENT)
		Modbus_MeterClientTask();
		#endif

	    #ifdef METER_TOP_BOARD
	    MCUTasks();
	    #endif
	    	    	    	    	    
	    // Blink LED0 (right most one) every second.
        //if(TickGet() - t >= TICK_SECOND)
        if(TickGet() - t >= LED_INTERVAL*TICK_SECOND/2ul)
	    {
            t = TickGet();
            LED0_IO ^= 1;
        }

		// 2013-09-18 (Liz). Dont request for eeprom if meter prepare for 12-hour-auto-reset
		if(!Do_Reset)
		{
			//if(TickGet() - EEPROM_RECORD_TIMER >= TICK_HOUR/2ul)
			// 2013-09-12 Liz debug
			//if(TickGet() - EEPROM_RECORD_TIMER >= TICK_SECOND*5ul)
			//if(TickGet() - EEPROM_RECORD_TIMER >= EEPROM_INTERVAL*TICK_MINUTE)
			if(EEP_RECORD_FLAG && TickGet() - EEPROM_RECORD_TIMER >= TICK_SECOND)
			{
				BOOL i = 0;
			
				// 2012-10-19 Liz added: Skip if there is no space left to write
				if(free_space < EEP_RECORD_LENGTH)	
					//EEPROM_RECORD_TIMER =  TickGet(); // Stop writing if there is no space to store
					EEP_RECORD_FLAG = FALSE;
				else
				{
					i = EEPROMEnergyRecord();
					if(!i || !is_cycle_completed)
						//EEPROM_RECORD_TIMER = TickGet() - EEPROM_INTERVAL*TICK_MINUTE + TICK_SECOND;
						//EEPROM_RECORD_TIMER = TickGet() - TICK_HOUR/2ul + TICK_SECOND;
						//EEPROM_RECORD_TIMER = TickGet() - TICK_SECOND*4ul;	// 2013-09-12 Liz debug
						EEPROM_RECORD_TIMER = TickGet();
					else
					{
						EEPROM_RECORD_TIMER =  TickGet();
						is_cycle_completed = FALSE;
						EEP_RECORD_FLAG = FALSE;
					}
				}
			}
		}
		
        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();
        
        #ifdef METER_TOP_BOARD
		MCUTasks();
	   	#endif
	   	
        // This tasks invokes each of the core stack application tasks
        StackApplications();

		// Process application specific tasks here.
		// For this demo app, this will include the Generic TCP 
		// client and servers, and the SNMP, Ping, and SNMP Trap
		// demos.  Following that, we will process any IO from
		// the inputs on the board itself.
		// Any custom modules or processing you need to do should
		// go here.
		
		#if defined( APP_USE_MENU )
		menu_task();
	    #endif
		
		// 2014-01-22 liz. Update Datetime if receive new datetime from server 
		if(UPDATE_DATETIME)
		{
//			// Need to check if datetime is invalid
			if(DateTime[0] == 0 || DateTime[1] == 0 || DateTime[2] == 0)
			{
				UPDATE_DATETIME = FALSE;
			}			
			else if(MCURequestToBOTTOMBoard(MMT_SET_DATETIME, &DateTime[0], 6, FALSE, FALSE))
			{
			//	 Send valid datetime to bottom board
			//	 Clear Flag after sending
				UPDATE_DATETIME = FALSE;
			//	Update DateTime flag 
			//	meter_flag.Flags.Flag1.bDateTimeUpdated = 1;	// 2014-06-10 Liz removed.
				SetMCUIsBusy(FALSE);						
			}
		}	
		
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
		
		#ifdef METER_TOP_BOARD
    	MCUTasks();
    	#endif
		
		#if defined(STACK_USE_BERKELEY_API)
		BerkeleyTCPClientDemo();
		BerkeleyTCPServerDemo();
		BerkeleyUDPClientDemo();
		#endif
		
        // If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce 
        // service
		if(dwLastIP != AppConfig.MyIPAddr.Val)
		{
			dwLastIP = AppConfig.MyIPAddr.Val;

			#if defined(STACK_USE_ANNOUNCE)
				AnnounceIP();
			#endif
				
			// Get MAC address
			MACAddressToString(AppConfig.MyMACAddr, &strMACAddress[0]);
			
			// Get IP address
			IPAddressToString(AppConfig.MyIPAddr, &strIPAddress[0]);	//IPAddress
		}
	}
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
	// Init Meter Flag
	meter_flag.v[0] = 0;
	meter_flag.v[1] = 0;
//	meter_flag.Flags.Flag1.bTOPReset = 1;	// 2012-11-09 Liz added
//	meter_flag.Flags.Flag1.bHaveEepRecords = 0;
//	meter_flag.Flags.Flag1.bDateTimeUpdated = 0;
	
	// LCD
	LCD_Init();
	
	// 2012-06-10(Eric) - Do not show "initialising" text if we are coming back from a WDT reset.
	if(RCONbits.NOT_TO == 1)
		LCD_PrintStringPGM("Initialising...", LCD_ROW0);
	else
		LCD_PrintStringPGM("Total kWh", LCD_ROW0);		// 2014-06-16 Liz
//		LCD_PrintStringPGM("Anacle Systems", LCD_ROW0);

	// Buttons.
	BUTTON_UP_TRIS = 1;
	BUTTON_DOWN_TRIS = 1;
	BUTTON_ENTER_TRIS = 1;
	BUTTON_EXIT_TRIS = 1;
	BUTTON_RESET_TRIS = 1;
	
	// Buzzer.
	BUZZER_TRIS = 0;
	
	// LEDs
	LED0_TRIS = 0;
	LED1_TRIS = 0;
	LED0_IO = 1;
	LED1_IO = 1;
	
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
	#if defined(PICDEMNET2)
		ADCON0 = 0x09;		// ADON, Channel 2
		ADCON1 = 0x0B;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are analog
	#elif defined(PICDEMZ)
		ADCON0 = 0x81;		// ADON, Channel 0, Fosc/32
		ADCON1 = 0x0F;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are all digital
	#elif defined(__18F87J11) || defined(_18F87J11) || defined(__18F87J50) || defined(_18F87J50)
		ADCON0 = 0x01;		// ADON, Channel 0, Vdd/Vss is +/-REF
		WDTCONbits.ADSHR = 1;
		ANCON0 = 0xFC;		// AN0 (POT) and AN1 (temp sensor) are anlog
		ANCON1 = 0xFF;
		WDTCONbits.ADSHR = 0;		
	#else
		ADCON0 = 0x01;		// ADON, Channel 0
		ADCON1 = 0x0E;		// Vdd/Vss is +/-REF, AN0 is analog
	#endif
	ADCON2 = 0xBE;		// Right justify, 20TAD ACQ time, Fosc/64 (~21.0kHz)


    // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;

	// Configure USART
   	// $$$ Initialize UART
	TRISCbits.TRISC6 = 0;
	TRISCbits.TRISC7 = 1;
    BAUDCON1bits.DTRXP = 1;
    BAUDCON1bits.SCKP = 1;
    TXSTA1 = 0x20;
    RCSTA1 = 0x90;

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
	#if defined(__18F87J10) || defined(__18F86J15) || defined(__18F86J10) || defined(__18F85J15) || defined(__18F85J10) || defined(__18F67J10) || defined(__18F66J15) || defined(__18F66J10) || defined(__18F65J15) || defined(__18F65J10) || defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) || \
	     defined(_18F87J10) ||  defined(_18F86J15) || defined(_18F86J10)  ||  defined(_18F85J15) ||  defined(_18F85J10) ||  defined(_18F67J10) ||  defined(_18F66J15) ||  defined(_18F66J10) ||  defined(_18F65J15) ||  defined(_18F65J10) ||  defined(_18F97J60) ||  defined(_18F96J65) ||  defined(_18F96J60) ||  defined(_18F87J60) ||  defined(_18F86J65) ||  defined(_18F86J60) ||  defined(_18F67J60) ||  defined(_18F66J65) ||  defined(_18F66J60)
		ADCON0bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON0bits.ADCAL = 0;
	#elif defined(__18F87J11) || defined(__18F86J16) || defined(__18F86J11) || defined(__18F67J11) || defined(__18F66J16) || defined(__18F66J11) || \
		   defined(_18F87J11) ||  defined(_18F86J16) ||  defined(_18F86J11) ||  defined(_18F67J11) ||  defined(_18F66J16) ||  defined(_18F66J11) || \
		  defined(__18F87J50) || defined(__18F86J55) || defined(__18F86J50) || defined(__18F67J50) || defined(__18F66J55) || defined(__18F66J50) || \
		   defined(_18F87J50) ||  defined(_18F86J55) ||  defined(_18F86J50) ||  defined(_18F67J50) ||  defined(_18F66J55) ||  defined(_18F66J50)
		ADCON1bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON1bits.ADCAL = 0;
	#endif

#else	// 16-bit C30 and and 32-bit C32
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
	AppConfig.Flags.bIsPwSet = FALSE;
	//AppConfig.Flags.bComputeWithNegativeEnergy = FALSE;
	memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));

	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;
	memcpypgm2ram((void*)AppConfig.Password, (ROM void*)DEFAULT_USER_PASSWORD, 4);
	
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

	#if defined(EEPROM_CS_TRIS)
	{
		BYTE c;
		APP_CONFIG tempAppConfig;	
	    // When a record is saved, first byte is written as 0x60 to indicate
	    // that a valid record was saved.  Note that older stack versions
		// used 0x57.  This change has been made to so old EEPROM contents
		// will get overwritten.  The AppConfig() structure has been changed,
		// resulting in parameter misalignment if still using old EEPROM
		// contents.
		
		// TCPIP configuration settings will be moved to start from 0x0050.
		// The first 80 bytes will be used for application settings.
		XEEReadArray(0x0000, &c, 1);
	    if(c == 0x70u)	// 2012-05-11 Liz: Changed start byte of eeprom from 0x60 to 0x70
		{
			// Load AppConfig from the EEPROM. If it exists, the default values populated above will be overwritten
			// with what is in the EEPROM.
			XEEReadArray(0x0001, (BYTE*)&tempAppConfig, sizeof(AppConfig));
			AppConfig = tempAppConfig;
			// Check if password field has been changed by user at least once and 
			// check if all chracters are numbers.
			if(!AppConfig.Flags.bIsPwSet ||
				!(AppConfig.Password[0] >= '0' && AppConfig.Password[0] <='9') ||
				!(AppConfig.Password[1] >= '0' && AppConfig.Password[1] <='9') ||
				!(AppConfig.Password[2] >= '0' && AppConfig.Password[2] <='9') ||
				!(AppConfig.Password[3] >= '0' && AppConfig.Password[3] <='9') )
				memcpypgm2ram((void*)AppConfig.Password, (ROM void*)DEFAULT_USER_PASSWORD, 4);
		}    
	    else
	        SaveAppConfig();

		// 2012-05-07 Liz: Fixed MAC address mess up
		memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
	}
	#endif
}

/*-------------------------------------------------------------------------
	Save AppConfig to eeprom
-------------------------------------------------------------------------*/
#if defined(EEPROM_CS_TRIS)
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
	// 2012-04-30(Eric) - If we are endlessly stuck in here, it means EEPROM is unstable.
	while(1)
	{
    	BOOL bIsErrorFound = FALSE;
    	APP_CONFIG verifyAppConfig;
    	WORD vCount;
		
	    XEEBeginWrite(0x0000);
	    XEEWrite(0x70);	//2012-05-11 Liz: Changed start byte of eeprom from 0x60 to 0x70
	    XEEWriteArray((BYTE*)&AppConfig, sizeof(AppConfig));
	    
	    // 2012-04-26(Eric) - Need to verify written data.
    	XEEReadArray(0x0001, (BYTE*)&verifyAppConfig, sizeof(APP_CONFIG));

		// 2012 -05-07 Liz: used memcmp to verify data instead of FOR loop    
		if(memcmp((BYTE*)&AppConfig, (BYTE*)&verifyAppConfig, sizeof(AppConfig)) != 0 )
			bIsErrorFound = TRUE;	

    	if(!bIsErrorFound)
    		break;
    }    
    #endif
}
#endif

/*----------------------------------------------------------------------
	Handle RESET button. If Reset button is pressed, send Reset command 
		to reset Bototm board before reset Top board. 
----------------------------------------------------------------------*/
void ResetControl(void)
{
	// If BUTTON_RESET is pressed, reset microcontroller.	
	if (BUTTON_RESET == 0) button_reset_old_state = 1;
	if ((BUTTON_RESET == 1) && (button_reset_old_state==1))
	{
		if((BUTTON_DOWN == 0) && (BUTTON_UP == 0))
		{
			Reset();
		}
		else
		{
			char t[5] = {'T'};

			MCURequestToBOTTOMBoard(MMT_BOT_NORMAL_RESET, t, 1, TRUE, TRUE);
				
			if( t[0] == 'T' ) 
			{
				// Not reset successfully.
				Reset();
			}
			else if( t[0] == 'O' && t[1] == 'K' )
			{
				Reset();
			}		
			button_reset_old_state = 0;
		}
	}
}	

/*-------------------------------------------------------------------
	This function will reset meter and communicator ervery 12hours
	Note: This function is temporaly use to prevent Bot_tob comm hangs
		and unable to detect IP. Better to remove once problem are fixed.
-------------------------------------------------------------------*/
void CustomClearWdt(void)
{
	//static BOOL Do_Reset = FALSE;	// 2013-09-18 Liz changed to global varialble.
	static unsigned int Retries = 0;
	static BOOL Save_Flag_to_EEP = FALSE;	// 2013-09-20 Liz. Save flag in eeprom to mark that meter reset dut to 12-hour-reset
	
	// Below is for 12 hour.
	//if(TickGetDiv256() - dwForce_wdt < (DWORD)(12u*TICK_HOUR/256u))	
	// 2013-09-12 Liz debug
	//if(TickGetDiv256() - dwForce_wdt < (DWORD)(11u*TICK_MINUTE/256u))
	// 2013-09-18 (Liz). Added another 2 minutes to allow top board get eepmrom record before auto-reset
	if(TickGetDiv256() - dwForce_wdt < (DWORD)((12ul*TICK_HOUR + 2u*TICK_MINUTE)/256u))
	{
		ClrWdt();
	}
	else
	{
		// 2012-11-14 Liz added: Reset Bot before reset Top board
		char t[5] = {'T', 0, 0, 0, 0};
		
		if(Retries < 3 && !Do_Reset)
		{
				
			MCURequestToBOTTOMBoard(MMT_BOT_NORMAL_RESET, t, 1, TRUE, TRUE);
				
			if( t[0] == 'T' ) 
			{
				// Not reset successfully. Try again
				ClrWdt();
				Retries++;
			}
			else if( t[0] == 'O' && t[1] == 'K' )
			{
				// Successfully reset Bot, wait until time out and reset Top 
				Do_Reset = TRUE;
			}
		}
		else
		{
			if(!Save_Flag_to_EEP)
			{
				XEEBeginWrite(TWELVE_HOUR_RESET_FLAG);
				XEEWrite(0x52);	// Write 'R' to indicate that meter had been reset due to 12-hour-auto-reset
				XEEEndWrite();
				Save_Flag_to_EEP = TRUE;
			}	
			
			// Wait for time out to reset
		}	
	}
	// Below is for 24 hours.
//	if(TickGetDiv256() - dwForce_wdt < (DWORD)(24u*TICK_HOUR/256u))
//		ClrWdt();
}	

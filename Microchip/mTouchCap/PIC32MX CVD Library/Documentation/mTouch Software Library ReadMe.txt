*******************************************************************************
********************** mTouch Capacitive Library Readme File **********************
*******************************************************************************

Copyright (c) Microchip Technology Inc.  All rights reserved.

This file contains information related to the mTouch Capacitive Touch Software Library.

*******************************************************************************
    Release Information
*******************************************************************************
mTouch Cap Library v1.21       Oct-2010
MPLAB version:                 8.56 or later
C18 Compiler Version      3.34 or later  
C30 Compiler Version      3.23 or later
C32 Compiler Version      1.10b or later`

*******************************************************************************
    mTouch Capacitive Touch Library testing Details
*******************************************************************************
The mTouch Capacitive Touch Library software is tested using PIC18F, PIC24F and
PIC32 enhanced mTouch Capacitive Touch Evaluation Kit (DM183026-2).


*******************************************************************************
    mTouch Capacitive Touch Library Installation Details
*******************************************************************************

The default installation directory for the mTouch software library files is
C:\Microchip Solutions\Microchip\mTouchCap.

Please see the help file "mTouchCap Library Help.chm" in the 
C:\Microchip Solutions\Microchip\Help for details on using the mTouch Software Library.  


*******************************************************************************
                Software Folder and File structure
*******************************************************************************

<<Microchip Solutions>>
          |
          | 
          |
          |------> <<Microchip>>
          |          |
          |          |
          |          |
          |          |------> <<mTouchCap>>
          |          |          |
          |          |          |
          |          |          |------> << PIC16F CSM & CVD Library >>(Core mTouch Capacitive Touch Library files for PIC18F and PIC24F)
               |             |          |
          |          |          |                       
          |          |                   |------> << PIC18F & PIC24F CTMU Library >> (Core mTouch Capacitive Touch Library files for PIC18F and PIC24F)
          |          |          |
          |          |          |
          |          |          |------> << PIC32MX CVD Library >>(Core mTouch Capacitive Touch Library files for PIC32MX)
          |          |          |
          |          |          |         
          |          |          |------> << Documentation >> (mTouchSoftwareLibraryReadMe.txt)
                  |          |
          |          |
               |              |------> <<Include>>
          |          |          |
          |          |          |
          |          |          |------> .h (common files)
          |          |          |       
          |          |
          |          |
          |          |------> <<Help>> (mTouchCap Library Help)
          |                                                     
          |          
          |------> <<mTouchCapDemos>>
          |          |
          |          |------> <<PIC18F_Demos>>
          |          |          |                               
          |          |          |          
          |          |          |------> <<mTouchCap_2Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 2ch slider Demo)                     |          |          |           
          |          |          |------> <<mTouchCap_4Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 4ch slider Demo)                     |          |          |          
          |          |          |------> <<mTouchCap_DirectKeyDemo>> (config,hardware and  Demo Source files/MPLAB files for Direct Key Demo)                     |          |          |          
          |          |          |------> <<mTouchCap_MatrixKeyDemo>> (config,hardware and Demo Source files/MPLAB files for Matrix Key Demo)                      |          |          |          
          |          |          |------> <<mTouchCap_Combo_Demo>> (config,hardware and Demo Source files/MPLAB files for all demos ) 
          |          |          |          
          |          |          |------> <<mTouchCap_GUI_Demo>> (config,hardware and GUI related Demo Source files/MPLAB files for all Demos)                     |          |          |          
          |          |          |------> <<mTouchCap_AppCommonFiles>> (Demo Application related source and header files) 
          |          |          |               
          |          |------> <<PIC24F_Demos>>          
          |          |          |
          |          |          |          
          |          |          |------> <<mTouchCap_2Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 2ch slider Demo)                     |          |          |           
          |          |          |------> <<mTouchCap_4Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 4ch slider Demo)                     |          |          |          
          |          |          |------> <<mTouchCap_DirectKeyDemo>> (config,hardware and  Demo Source files/MPLAB files for Direct Key Demo)                     |          |          |          
          |          |          |------> <<mTouchCap_MatrixKeyDemo>> (config,hardware and Demo Source files/MPLAB files for Matrix Key Demo)                      |          |          |          
          |          |          |------> <<mTouchCap_Combo_Demo>> (config,hardware and Demo Source files/MPLAB files for all Demos ) 
          |          |          |          
          |          |          |------> <<mTouchCap_GUI_Demo>> (config,hardware and GUI related Demo Source files/MPLAB files for all Demos)                     |          |          |          
          |          |          |------> <<mTouchCap_DA210_GraphicsDemo>> (config,hardware and Demo Source files/MPLAB files for Graphics                         |          |          |                                                     Demo)     
          |          |          |          
          |          |          |------> <<mTouchCap_GestureDemo>> (config,hardware and Demo Source files/MPLAB files for Gesture Demo ) 
          |          |          |          
          |          |          |------> <<mTouchCap_ProximityDemo>> (config,hardware and Demo Source files/MPLAB files for Proximity Demo)                               |          |          |          
          |          |          |------> <<mTouchCap_AppCommonFiles>> (Demo Application related source and header files) 
          |          |          |               
          |          |------> <<PIC32MX_Demos>>          
          |          |          |
          |          |          |          
          |          |          |------> <<mTouchCap_2Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 2ch slider Demo)                     |          |          |           
          |          |          |------> <<mTouchCap_4Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 4ch slider Demo)                     |          |          |          
          |          |          |------> <<mTouchCap_DirectKeyDemo>> (config,hardware and  Demo Source files/MPLAB files for Direct Key Demo)                     |          |          |          
          |          |          |------> <<mTouchCap_MatrixKeyDemo>> (config,hardware and Demo Source files/MPLAB files for Matrix Key Demo)                      |          |          |          
          |          |          |------> <<mTouchCap_AppCommonFiles>> (Demo Application related source and header files) 
                
               


*******************************************************************************
    mTouch Capacitive Touch Library Setup Details
*******************************************************************************
There are many Capacitive touch Demo projects supplied along with this library. 
These demo projects can be configured to user's application by
modifying two header files "HardwareProfile.h" and "Config.h".


"HardwareProfile.h" file defines the macros that are related to the hardware used. 
Make changes to this file according to the target hardware.
This file is located in each demo folder. It configures 
system hardware related settings like  system clock, ports, max ADC channels.

"Config.h" file defines the configurations releated to the library. 
This file is located in each demo folder.

The detailed setup details for all the demo boards are mentioned in the respective Readme.txt files which 
are placed in the corresponding demo folders.

*******************************************************************************


*******************************************************************************
        Demo projects for the mTouchCap Software Library
*******************************************************************************
The software installed includes the complete source code for several demo
applications that can be loaded onto the CTMU Eval Board.  The default directory 
location is: C:\Microchip Solutions\mTouchCapDemos. 

*******************************************************************************

The latest release of MPLAB (Version 8.56 or higher) should be used with the 
mTouch Capaitive Touch Library files. The latest release is available at:
http://www.microchip.com 

Please note that the MPLAB C30/C18/C32 C compiler is required to build the project 
is available for download/purchase from the Microchip website.

*******************************************************************************


*******************************************************************************
        Help file
*******************************************************************************
More mTouch Software Library specific information is located in the
"mTouchCap Library Help" file.

*******************************************************************************



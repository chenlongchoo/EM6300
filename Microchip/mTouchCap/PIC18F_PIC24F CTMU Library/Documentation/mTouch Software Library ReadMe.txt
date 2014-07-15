*******************************************************************************
********************** mTouch Capacitive Library Readme File **********************
*******************************************************************************

Copyright (c) Microchip Technology Inc.  All rights reserved.

This file contains information related to the mTouch Capacitive Touch Software Library.

*******************************************************************************
    Release Information
*******************************************************************************
mTouch Cap Library v1.31:      	  June-2011
MPLAB version:                    	  8.70 or Later
C30 Compiler Version:	               3.25 or Later 
C18 Compiler Version:		  3.38 or Later
MPLAB X version: 			  6.00.01 Beta 
 


*******************************************************************************
    mTouch Capacitive Touch Library testing Details
*******************************************************************************
The mTouch Capacitive Touch Library software is tested using PIC18F and PIC24F 
CTMU Evaluation boards(DM183026-2).


*******************************************************************************
    mTouch Capacitive Touch Library Installation Details
*******************************************************************************

The default installation directory for the mTouch software library files is
C:\Microchip Solutionsv2011-06-02\Microchip\mTouchCap.

Please see the help file "mTouchCap Library Help.chm" in the 
C:\Microchip Solutionsv2011-06-02\Microchip\Help for details on using the mTouch Software Library.  


*******************************************************************************
		Software Folder and File structure
*******************************************************************************

<<Microchip Solutionsv2011-06-02>>
	  |
	  | 
	  |
	  |------> <<Microchip>>
	  |	     |
	  |	     |
	  |	     |
	  |	     |------> <<mTouchCap>>
	  |	     |		|
	  |	     |		|
	  |	     |		|------> << PIC32MX CVD Library>>(Core mTouch Capacitive Touch Library files for PIC32)
               |	     |		|
	  |	     |		|            		
	  |	     |                   |------> << PIC18F_PIC24F CTMU Library >> (Core mTouch Capacitive Touch Library files for PIC18F and PIC24F)
	  |	     |		|		|
	  |	     |		 	  	|
	  |	     |				|------> << Documentation >> (mTouchSoftwareLibraryReadMe.txt)
          	  |	     |
 	  |	     |
               |              |------> <<Include>>
	  |	     |		|
	  |	     |		|
	  |	     |		|------> .h (common files)
	  |	     |		|	
	  |	     |
	  |	     |
	  |	     |------> <<Help>> (mTouchCap Library Help)
	  |	     			  	     		
	  |	     
	  |------> <<mTouchCapDemos>>
               |	     |		|	   			
	  |	     |		|
	  |	     |------> <<PIC16F_CVD_Demos>>
	  |	     |		|	   			
	  |	     |		|	   
	  |	     |		|------> <<Cap Touch CSM-CVD Eval Board 04-02091 Rev D1>> (Code-no cover and Code-with cover folders )
	  |	     |		|	    
	  |	     |		|------> <<Cap Touch CVD Eval Board 233-04-2028 Rev B>> (Code-no cover and Code-with cover folders )
	  |	     |		|	   
	  |	     |		|------> <<PIC12F1822 Example - 4 sensors>> (Code-no cover folder) 	        	  
	  |	     |		|	   
	  |	     |		|------> <<PIC16F617 Example - 4 Sensors>> (Code-no cover folder)
	  |	     |		|	   
	  |	     |		|------> <<PIC16F1936 Example - 4 sensors>> (Code-2mm cover, Code-metal over capacitive, Code-no Cover folders ) 
	  |	     |		|	   
	  |	     |		|------> <<Utilities>> (config,hardware and GUI related Demo Source files/MPLAB 8 and MPLAB X files for all Demos) 		  
	  |	     |		|	   	  
	  |	     |		|	
	  |	     |------> <<PIC18FDemos>>
	  |	     |		|	   			
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_2ChSlider>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for 2ch slider Demo) 	  	  
	  |	     |		|	    
	  |	     |		|------> <<mTouchCap_4ChSlider>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for 4ch slider Demo) 		  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_DirectKey>> (config,hardware and  Demo Source files/MPLAB 8 and MPLAB X files for Direct Key Demo) 	        	  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_MatrixKey>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB XB files for Matrix Key Demo) 		  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_Combo_>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for all demos ) 
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_GUI>> (config,hardware and GUI related Demo Source files/MPLAB 8 and MPLAB X files for all Demos) 		  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_AppCommonFiles>> (Demo Application related source and header files) 
	  |	     |		|	   	
	  |	     |------> <<PIC24FDemos>>		
	  |	     |		|
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_2ChSlider>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for 2ch slider Demo)  	  	  
	  |	     |		|	    
	  |	     |		|------> <<mTouchCap_4ChSlidero>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for 4ch slider Demo) 		  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_DirectKey>> (config,hardware and  Demo Source files/MPLAB 8 and MPLAB X files for Direct Key Demo)	 	  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_MatrixKey>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for Matrix Key Demo) 		  
 	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_Comboo>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for all Demos ) 
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_GUI>> (config,hardware and GUI related Demo Source files/MPLAB 8 and MPLAB X files for all Demos)		  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_DA210Graphics>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for Graphics 		  
	  |	     |		|	   					      Demo)	
	  |	     |        	|	   
	  |	     |		|------> <<mTouchCap_Gesture>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for Gesture Demo ) 
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_Proximity>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for Proximity Demo)  		  
	  |	     |		|
	  |	     |		|------> <<mTouchCap_LowPower>> (config,hardware and Demo Source files/MPLAB 8 and MPLAB X files for LowPower Demo)   
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_AppCommonFiles>> (Demo Application related source and header files) 
	  |	     |		|
	  |	     |		|
	  |	     |------> <<PIC32MX_Demos>>
	  |	     |		|	   			
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_2Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 2ch slider Demo) 		  	  
	  |	     |		|	    
	  |	     |		|------> <<mTouchCap_4Ch_SliderDemo>> (config,hardware and Demo Source files/MPLAB files for 4ch slider Demo) 			  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_DirectKeyDemo>> (config,hardware and  Demo Source files/MPLAB files for Direct Key Demo) 	        	  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_MatrixKeyDemo>> (config,hardware and Demo Source files/MPLAB files for Matrix Key Demo) 	 		  
	  |	     |		|	   
	  |	     |		|------> <<mTouchCap_AppCommonFiles>> (Demo Application related source and header files) 
          	
	       


*******************************************************************************
    mTouch Capacitive Touch Library Setup Details
*******************************************************************************
There are many Capacitive touch Demo projects supplied along with this library. 
These demo projects can be configured to user's application by
modifying two header files "HardwareProfile.h" and "Config.h".


"HardwareProfile.h" file defines the macros that are related to the hardware used. 
Controllers with CTMU module are defined in this file
Make changes to this file according to the target hardware.
This file is located in each demo folder. It configures 
system hardware related settings like  system clock, ports, max ADC channels.

"Config.h" file defines the configurations releated to the application.
This file is located in each demo folder.

The detailed setup details for all the demo boards are mentioned in the respective Readme.txt files which 
are placed in the corresponding demo folders.

*******************************************************************************


*******************************************************************************
	Demo projects for the mTouchCap Software Library
*******************************************************************************
The software installed includes the complete source code for several demo
applications that can be loaded onto the CTMU Eval Board.
The default directory location is: C:\Microchip Solutions v2011-06-02\mTouchCapDemos. 

*******************************************************************************

The latest release of MPLAB 8 (Version 8.70 or higher) and MPLAB X(6.00.01 Beta version)should be used 
with the mTouch Capaitive Touch Library files. The latest release is available at:
http://www.microchip.com 

Please note that the MPLAB C30/C18 C compiler is required to build the project 
is available for download/purchase from the Microchip website.

*******************************************************************************


*******************************************************************************
	Help file
*******************************************************************************
More mTouch Software Library specific information is located in the
"mTouchCap Library Help" file.

*******************************************************************************


Note: This text file can be best viewed with the following settings
	Font: 		Verdana
	Font style:	Regular
             Size: 		10
	Format:               Word Wrap

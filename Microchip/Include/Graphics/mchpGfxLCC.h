/*********************************************************************
 *                  LCC Graphics Driver (mchpGfxLCC)
 *********************************************************************
 * FileName:        mchpGfxLCC.c
 * Dependencies:    plib.h
 *
 * Processor:       PIC32
 *
 * Complier:        MPLAB C32 v1 or higher
 *                  MPLAB IDE v8 or higher
 * Company:         Microchip Technology Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC Microcontroller is intended
 * and supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ********************************************************************/

#ifndef _mchpGfxLCC_H
    #define _mchpGfxLCC_H

    #if defined(__PIC32MX__)
        #include <plib.h>
        #define PMDIN1  PMDIN
    #endif
    
    #include "GraphicsConfig.h"
    #include "GenericTypeDefs.h"

    #ifdef USE_PALETTE
        #include "Graphics/Palette.h"
    #endif

    #ifndef DISP_HOR_RESOLUTION
        #error DISP_HOR_RESOLUTION must be defined in HardwareProfile.h
    #endif
    #ifndef DISP_VER_RESOLUTION
        #error DISP_VER_RESOLUTION must be defined in HardwareProfile.h
    #endif
    #ifndef COLOR_DEPTH
        #error COLOR_DEPTH must be defined in HardwareProfile.h
    #endif
    #ifndef DISP_ORIENTATION
        #error DISP_ORIENTATION must be defined in HardwareProfile.h
    #endif

/*********************************************************************
* Overview: Horizontal synchronization timing in pixels
*                  (from the glass datasheet).
*********************************************************************/

    #ifndef DISP_HOR_PULSE_WIDTH
        #error DISP_HOR_PULSE_WIDTH must be defined in HardwareProfile.h
    #endif
    #ifndef DISP_HOR_BACK_PORCH
        #error DISP_HOR_BACK_PORCH must be defined in HardwareProfile.h
    #endif
    #ifndef DISP_HOR_FRONT_PORCH
        #error DISP_HOR_FRONT_PORCH must be defined in HardwareProfile.h
    #endif

/*********************************************************************
* Overview: Vertical synchronization timing in lines
*                  (from the glass datasheet).
*********************************************************************/
    #ifndef DISP_VER_PULSE_WIDTH
        #error DISP_VER_PULSE_WIDTH must be defined in HardwareProfile.h
    #endif
    #ifndef DISP_VER_BACK_PORCH
        #error DISP_VER_BACK_PORCH must be defined in HardwareProfile.h
    #endif
    #ifndef DISP_VER_FRONT_PORCH
        #error DISP_VER_FRONT_PORCH must be defined in HardwareProfile.h
    #endif

/*********************************************************************
* PARAMETERS VALIDATION
*********************************************************************/
    #if (DISP_HOR_RESOLUTION % 8) != 0
        #error Horizontal resolution must be divisible by 8.
    #endif
    #if (DISP_ORIENTATION != 0) && (DISP_ORIENTATION != 180) && (DISP_ORIENTATION != 90) && (DISP_ORIENTATION != 270)
        #error The display orientation selected is not supported. It can be only 0,90,180 or 270.
    #endif

/*********************************************************************
* Overview: Clipping region control codes to be used with SetClip(...)
*           function. 
*********************************************************************/
    #define CLIP_DISABLE    0   // Disables clipping.
    #define CLIP_ENABLE     1   // Enables clipping.

/*********************************************************************
* Overview: Some basic colors definitions.
*********************************************************************/
    #ifdef USE_PALETTE
    
        #include "PaletteColorDefines.h"

    #endif

/*********************************************************************
* Overview: Clipping region control and border settings.
*********************************************************************/

// Clipping region enable control
extern SHORT    _clipRgn;

// Left clipping region border
extern SHORT    _clipLeft;

// Top clipping region border
extern SHORT    _clipTop;

// Right clipping region border
extern SHORT    _clipRight;

// Bottom clipping region border
extern SHORT    _clipBottom;

/*********************************************************************
* Function:  BYTE GetReg(WORD index)
*
* PreCondition: none
*
* Input: index - register number
*
* Output: none
*
* Side Effects: none
*
* Overview: returns graphics controller register value (byte access)
*
* Note: none
*
********************************************************************/
BYTE            GetReg(WORD index);

/*********************************************************************
* Function:  void ResetDevice()
*
* Overview: Initializes LCD module.
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void            ResetDevice(void);

/*********************************************************************
* Macros:  GetMaxX()
*
* Overview: Returns maximum horizontal coordinate.
*
* PreCondition: none
*
* Input: none
*
* Output: Maximum horizontal coordinate.
*
*
* Example:
*   <CODE> 
*
*	// Create a window that will occupy the whole screen.
*	WndCreate(0xFF,				    	// ID
*             0,0,
*			  GetMaxX(),GetMaxY(),		// dimension
*             WND_DRAW,					// will be dislayed after creation
*             (void*)&mchpIcon,         // use icon used
*             pText,	   				// set to text pointed to by pText
*             NULL);					// use default scheme 
*
*  </CODE>
*
* Side Effects: none
*
********************************************************************/
    #if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
        #define GetMaxX()   (DISP_VER_RESOLUTION - 1)
    #elif (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180)
        #define GetMaxX()   (DISP_HOR_RESOLUTION - 1)
    #endif

/*********************************************************************
* Macros:  GetMaxY()
*
* Overview: Returns maximum vertical coordinate.
*
* PreCondition: none
*
* Input: none
*
* Output: Maximum vertical coordinate.
*
* Example: (see GetMaxX()) example.
*
* Side Effects: none
*
********************************************************************/
    #if (DISP_ORIENTATION == 90) || (DISP_ORIENTATION == 270)
        #define GetMaxY()   (DISP_HOR_RESOLUTION - 1)
    #elif (DISP_ORIENTATION == 0) || (DISP_ORIENTATION == 180)
        #define GetMaxY()   (DISP_VER_RESOLUTION - 1)
    #endif


/*********************************************************************
* Macros:  GetColor()
*
* Overview: Returns current drawing color.
*
* PreCondition: none
*
* Input: none
*
* Output: Color coded in 5:6:5 RGB format.
*
* Side Effects: none
*
********************************************************************/
    #define GetColor()  _color

/*********************************************************************
* Macros:  SetActivePage(page)
*
* Overview: Sets active graphic page.
*
* PreCondition: none
*
* Input: page - Graphic page number.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
    #define SetActivePage(page)

/*********************************************************************
* Macros: SetVisualPage(page)
*
* Overview: Sets graphic page to display.
*
* PreCondition: none
*
* Input: page - Graphic page number
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
    #define SetVisualPage(page)

/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* Overview: Puts pixel with the given x,y coordinate position.
*
* PreCondition: none
*
* Input: x - x position of the pixel.
*		 y - y position of the pixel.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
//void PutPixel(SHORT x, SHORT y);

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* Overview: Returns pixel color at the given x,y coordinate position.
*
* PreCondition: none
*
* Input: x - x position of the pixel.
*		 y - y position of the pixel.
*
* Output: pixel color
*
* Side Effects: none
*
********************************************************************/
GFX_COLOR    GetPixel(SHORT x, SHORT y);


/*********************************************************************
* Macros: GetClipLeft()
*
* Overview: Returns left clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Left clipping border.
*
* Side Effects: none
*
********************************************************************/
    #define GetClipLeft()   _clipLeft

/*********************************************************************
* Macros: GetClipRight()
*
* Overview: Returns right clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Right clipping border.
*
* Side Effects: none
*
********************************************************************/
    #define GetClipRight()  _clipRight

/*********************************************************************
* Macros: GetClipTop()
*
* Overview: Returns top clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Top clipping border.
*
* Side Effects: none
*
********************************************************************/
    #define GetClipTop()    _clipTop

/*********************************************************************
* Macros: GetClipBottom()
*
* Overview: Returns bottom clipping border.
*
* PreCondition: none
*
* Input: none
*
* Output: Bottom clipping border.
*
* Side Effects: none
*
********************************************************************/
    #define GetClipBottom() _clipBottom


#endif // _mchpGfxLCC_H

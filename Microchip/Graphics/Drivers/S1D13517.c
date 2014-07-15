/*****************************************************************************
 *  Module for Microchip Graphics Library
 *  Epson S1D13517 LCD controller driver
 *  to be used with GFX 3E PICtail board
 *****************************************************************************
 * FileName:        S1D13517.c
 * Dependencies:    Graphics.h
 * Processor:       PIC32
 * Compiler:       	MPLAB C32
 * Linker:          MPLAB LINK30, MPLAB LINK32
 * Company:         Microchip Technology Incorporated
 *
 * Software License Agreement
 *
 * Copyright © 2008 Microchip Technology Inc.  All rights reserved.
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital
 * signal controller, which is integrated into your product or third party
 * product (pursuant to the sublicense terms in the accompanying license
 * agreement).  
 *
 * You should refer to the license agreement accompanying this Software
 * for additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR
 * OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 * BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT
 * DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL,
 * INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA,
 * COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY
 * CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
 * OR OTHER SIMILAR COSTS.
 *****************************************************************************/
#include "HardwareProfile.h"

#ifdef GFX_USE_DISPLAY_CONTROLLER_S1D13517

#include "Compiler.h"
#include "TimeDelay.h"
#include "Graphics/gfxcolors.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/S1D13517.h"
#include "Graphics/gfxtcon.h"
#include "Graphics/Primitive.h"
#include "Graphics/Transitions.h"

#if defined (USE_GFX_PMP)
    #include "Graphics/gfxpmp.h"
#elif defined (USE_GFX_EPMP)
    #include "Graphics/gfxepmp.h"
#endif  


// Current Page
WORD    _page;

// Color
GFX_COLOR    _color;

// Clipping region control
SHORT       _clipRgn;

// Clipping region borders
SHORT       _clipLeft;
SHORT       _clipTop;
SHORT       _clipRight;
SHORT       _clipBottom;

// Alpha Blending Variables
SHORT _GFXForegroundPage;
SHORT _GFXBackgroundPage;
SHORT _GFXDestinationPage;

#if (COLOR_DEPTH == 24)
#define RED8(color24)   ((color24 & 0x00FF0000) >> 16)
#define GREEN8(color24) ((color24 & 0x0000FF00) >> 8)
#define BLUE8(color24)  ((color24 & 0x000000FF))

#define RED5(color16)   (BYTE) ((color16 & 0xF800) >> 8)
#define GREEN6(color16) (BYTE) ((color16 & 0x07E0) >> 3)
#define BLUE5(color16)  (BYTE) ((color16 & 0x001F) << 3)

#else
#define RED8(color16)   (BYTE) ((color16 & 0xF800) >> 8)
#define GREEN8(color16) (BYTE) ((color16 & 0x07E0) >> 3)
#define BLUE8(color16)  (BYTE) ((color16 & 0x001F) << 3)
#endif 


#ifndef GFX_DEFAULT_FOREGROUND_PAGE
#define GFX_DEFAULT_FOREGROUND_PAGE		GFX_PAGE9
#endif

#ifndef GFX_DEFAULT_BACKGROUND_PAGE
#define GFX_DEFAULT_BACKGROUND_PAGE 	GFX_PAGE10
#endif

#ifndef GFX_DEFAULT_DESTINATION_PAGE
#define GFX_DEFAULT_DESTINATION_PAGE 	GFX_PAGE0
#endif

typedef struct{
	DWORD page0BaseAddress;
} GFX_PAGE_INFO;

typedef struct{
	GFX_PAGE windowPage;
	WORD xOrigin; 
	WORD yOrigin;
	WORD width;
	WORD height;
} GFX_WINDOW_INFO;


#ifndef TRANSPARENTCOLOR
#define TRANSPARENTCOLOR                RGBConvert(0xFF, 0x00, 0xCC)          //Transparent Color chosen for the application
#endif

GFX_COLOR    pixelWindow[8] = 
{   
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR,
    TRANSPARENTCOLOR
};


//Internal S1D13517 Driver functions	
DWORD 		GFXGetPageOriginAddress(SHORT pageNumber);
DWORD 		GFXGetPageXYAddress(SHORT pageNumber, WORD x, WORD y);
DWORD 		GFXGetWindowOriginAddress(GFX_WINDOW_INFO* GFXWindowInfo);
DWORD 		GFXGetWindowXYAddress(GFX_WINDOW_INFO* GFXWindowInfo, WORD relX, WORD relY);
DWORD 		GetPositionAddress(SHORT page, WORD left, WORD top);

void        PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch);
void        PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch);
void        PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch);
void        PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch);
void        PutImage24BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch);

void        PutImage1BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch);
void        PutImage4BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch);
void        PutImage8BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch);
void        PutImage16BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch);
void 		PutImage24BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch);

#if (DISPLAY_PANEL == TFT_G240320LTSW_118W_E)
    #include "TCON_SSD1289.c"

#elif (DISPLAY_PANEL == TFT_G320240DTSW_69W_TP_E)
    #include "TCON_HX8238.c"

#elif (DISPLAY_PANEL == PH480272T_005_I06Q)
    #include "TCON_HX8257.c"

#else
    #include "TCON_Custom.c"
#endif

#ifdef USE_PALETTE
extern void *_palette;
static BYTE PaletteBpp = 16;
extern BYTE blPaletteChangeError;
extern void *pPendingPalette;
extern WORD PendingStartEntry;
extern WORD PendingLength;
#endif

/*********************************************************************
* Macro:  WritePixel(color)
*
* PreCondition: none
*
* Input: color 
*
* Output: none
*
* Side Effects: none
*
* Overview: writes pixel at the current address
*
* Note: chip select should be enabled
*
********************************************************************/
#ifdef USE_16BIT_PMP

#if (COLOR_DEPTH == 16)
#define WritePixel(color)	    DeviceWrite(color)
#elif (COLOR_DEPTH == 24)
#define WritePixel(color)   { DeviceWrite((WORD)((color) >> 16)); DeviceWrite((WORD)(color));}
#endif

#else

#ifdef USE_PALETTE
#define WritePixel(color)	DeviceWrite(color)
#else

#if (COLOR_DEPTH == 16)
#define WritePixel(color)	{ DeviceWrite(((WORD_VAL)color).v[1]); DeviceWrite(((WORD_VAL)color).v[0]);}
#elif (COLOR_DEPTH == 24)
#define WritePixel(color){ DeviceWrite(((DWORD_VAL)color).v[2]);DeviceWrite(((DWORD_VAL)color).v[1]); DeviceWrite(((DWORD_VAL)color).v[0]);}

#endif

#endif

#endif

/*********************************************************************
* Function:  void SetAddress(DWORD address)
*
* PreCondition: none
*
* Input: address - address 
*
* Output: none
*
* Side Effects: none
*
* Overview: sets the address for read/write operations
*
* Note: chip select should be enabled
*
********************************************************************/
void SetAddress(DWORD address)
{
    #ifdef USE_16BIT_PMP
	WORD_VAL    temp;

    DisplaySetCommand(); // set RS line to low for command

    temp.v[0] = ((DWORD_VAL) address).v[1];
    temp.v[1] = ((DWORD_VAL) address).v[2] | 0x80;
	DeviceWrite(temp.Val);
	temp.v[0] = 0x01;
    temp.v[1] = ((DWORD_VAL) address).v[0];
	DeviceWrite(temp.Val);

	DisplaySetData();    // set RS line to high for data

    #else

    DisplaySetCommand(); // set RS line to low for command

    DeviceWrite(((DWORD_VAL) address).v[2] | 0x80);
    DeviceWrite(((DWORD_VAL) address).v[1]);
    DeviceWrite(((DWORD_VAL) address).v[0]);

    DisplaySetData();    // set RS line to high for data

    #endif
}

/*********************************************************************
* Function:  void  SetReg(WORD index, BYTE value)
*
* PreCondition: none
*
* Input: index - register number
*        value - value to be set
*
* Output: none
*
* Side Effects: none
*
* Overview: sets graphics controller register (byte access)
*
* Note: none
*
********************************************************************/
void SetReg(WORD index, BYTE value)
{
    DisplayEnable();     // enable S1D13517
 
    DisplaySetCommand(); // set RS line to low for command
   
    DeviceWrite(index);

    DisplaySetData();    // set RS line to high for data

    DeviceWrite(value);

    DisplayDisable();   // disable S1D13517
}

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
BYTE GetReg(WORD index)
{
    BYTE value;
 
    DisplayEnable();     // enable S1D13517

    DisplaySetCommand(); // set RS line to low for command

	DeviceWrite(index);

	DisplaySetData();    // set RS line to high for data

	value = DeviceRead();

	DisplayDisable();   // disable S1D13517

	return (value);

}

/*********************************************************************
* Function:  void ResetDevice()
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: resets LCD, initializes PMP
*
* Note: none
*
********************************************************************/
void ResetDevice(void)
{

   _GFXForegroundPage = GFX_DEFAULT_DESTINATION_PAGE;   //Initialize Alpha Values
   _GFXBackgroundPage = GFX_DEFAULT_BACKGROUND_PAGE;
   _GFXDestinationPage = GFX_DEFAULT_FOREGROUND_PAGE;


    /////////////////////////////////////////////////////////////////////
    // Initialize the device
    /////////////////////////////////////////////////////////////////////
	DriverInterfaceInit();

	SetReg(REG2A_DSP_MODE,0x00);
	  
	SetReg(REG68_POWER_SAVE,0x00);		
	SetReg(REG04_PLL_DDIVIDER,0x17);  	//PLLCLK = 1MHz (=24MHz/24=CLKI/PLL_D)
	SetReg(REG06_PLL_0,0x29);		 	//VCO = 142-180MHz
	SetReg(REG08_PLL_1,0x01);  			//PLLCLK is divided by 2
	SetReg(REG0A_PLL_2,0x08);  			//REG0A must be progremmed with value 08h

	SetReg(REG0C_PLL_NDIVIDER,0x59);	//Target Frequency = 90MHz; NN = 180; CLKI = 24MHz; PLL_D = 24:1
 
      #if defined(GFX_USE_DISPLAY_PANEL_PH480272T_005_I11Q) || defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
            SetReg(REG12_CLK_SRC_SELECT,0x02); 
       #else
		SetReg(REG12_CLK_SRC_SELECT,0x92);  //SYSCLK is the source of SS clock; Internal PLL is the source of SDCLK
       #endif 	

	SetReg(REG04_PLL_DDIVIDER,0x97);	//Enable PLL
	SetReg(REG0E_SS_CONTROL_0,0x8F);  	//Spread Spectrum modulation enabled with default spread of +/-0.25ns
	DelayMs(100);
		
	#if (COLOR_DEPTH == 24)
		SetReg(REG14_LCD_PANEL_TYPE,0x02);  //8:8:8 color format 24BPP Panel
	#else
		SetReg(REG14_LCD_PANEL_TYPE,0x05);  //5:6:5 color format
	#endif

	SetReg(REG16_HDISP_WIDTH,(DISP_HOR_RESOLUTION>>3)-1);          
	SetReg(REG18_HNDP_PERIOD,((DISP_HOR_PULSE_WIDTH+DISP_HOR_FRONT_PORCH+DISP_HOR_BACK_PORCH)>>1)-1);          
	SetReg(REG1A_VDISP_HEIGHT_0,(BYTE)DISP_VER_RESOLUTION-1);
	SetReg(REG1C_VDISP_HEIGHT_1,(DISP_VER_RESOLUTION-1)>>8);
	SetReg(REG1E_VNDP_PERIOD,((DISP_VER_PULSE_WIDTH+DISP_VER_FRONT_PORCH+DISP_VER_BACK_PORCH)>>1)-1);          
	SetReg(REG20_PHS_PULSE_WIDTH,DISP_HOR_PULSE_WIDTH -1);    
	SetReg(REG22_PHS_PULSE_START,DISP_HOR_FRONT_PORCH);     
	SetReg(REG24_PVS_PULSE_WIDTH,DISP_VER_PULSE_WIDTH -1);     
	SetReg(REG26_PVS_PULSE_START,DISP_VER_FRONT_PORCH);    

#ifdef DISP_INV_LSHIFT
        SetReg(REG28_PCLK_POLARITY,0x80);       //Polarity
#else
        SetReg(REG28_PCLK_POLARITY,0x00);       //Polarity
#endif
           	
	SetReg(REG82_SDRAM_CONTROL_0,0x03);		//SDRAM memory size = 128MBits
	SetReg(REG8C_SDRAM_RFS_CNT_0,0xFF);		//SDRAM Refresh Rate = 
	SetReg(REG8E_SDRAM_RFS_CNT_1,0x03);
	SetReg(REG90_SDRAM_BUF_SIZE,0x50);		//SDRAM Write Buffer Memory Size = 1.75MHz (max)	
	SetReg(REG68_POWER_SAVE,0xE8);			//Reset modules
	SetReg(REG68_POWER_SAVE,0x00);
	SetReg(REG68_POWER_SAVE,0x01);			//Enable SDCLK
	SetReg(REG84_SDRAM_STATUS_0,0x86);		// Enable SDRAM, SDRAM auto refresh and powersave

	SetReg(REG52_INPUT_MODE,0x08);        	//Transparency On = 0x08
    SetReg(REG54_TRANSP_KEY_RED, RED8(TRANSPARENTCOLOR));   	
    SetReg(REG56_TRANSP_KEY_GREEN, GREEN8(TRANSPARENTCOLOR)); 
    SetReg(REG58_TRANSP_KEY_BLUE, BLUE8(TRANSPARENTCOLOR)); 
	SetReg(REG6E_GPO_1,0x07);   			//GPO All High

	#if defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
        DisplayBrightness(0);       //Sets Backlight Brightness Level - PWM
    #else
        DisplayBrightness(100);      //Sets Backlight Brightness Level - PWM
    #endif

	SetReg(REG2A_DSP_MODE,0x01);   			//Enable display interface

	SetReg(REG50_DISPLAY_CONTROL,0x80);   	//Update registers 0x2A-0x4E

	DelayMs(500);             				//Delay of 500ms for SDRAM to stabilize

    /////////////////////////////////////////////////////////////////////
    // Panel TCON Programming
    /////////////////////////////////////////////////////////////////////
//    GfxTconInit();

}


/*********************************************************************
* Function: void PutPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates
*
* Output: none
*
* Side Effects: none
*
* Overview: puts pixel
*
* Note: none
*
********************************************************************/
void PutPixel(SHORT x, SHORT y)
{
	static  BYTE xc=0;

#if (DISP_ORIENTATION == 90)

     WORD templ = x; 

     x = y;
     y = (DISP_VER_RESOLUTION-1) - templ;

#endif

#if (DISP_ORIENTATION == 270)
 
    WORD templ = x;

     x = (DISP_HOR_RESOLUTION-1)- y;
     y = templ;

#endif

#if (DISP_ORIENTATION == 180)

     x = (DISP_HOR_RESOLUTION-1)- x;
     y = (DISP_VER_RESOLUTION-1)- y;

#endif

    xc = x & 7;        
  
	DisplayEnable();
	
    DisplaySetCommand(); 
    DeviceWrite(REG5A_WRITE_WIN_X_SP);   
    
	DisplaySetData();    
 
    DeviceWrite(x >> 2);    //Setup the start window size/position    
    DeviceWrite(y >> 2);   
    DeviceWrite(y);   
    DeviceWrite(GetMaxX() >> 2);    //Setup the end window size/position  
    DeviceWrite(GetMaxY() >> 2);   
    DeviceWrite(GetMaxY());   

    pixelWindow[xc] = _color;

    WritePixel(pixelWindow[0]);    //Output the pixelWindow
    WritePixel(pixelWindow[1]);    //Output the pixelWindow
    WritePixel(pixelWindow[2]);    //Output the pixelWindow
    WritePixel(pixelWindow[3]);    //Output the pixelWindow
    WritePixel(pixelWindow[4]);    //Output the pixelWindow
    WritePixel(pixelWindow[5]);    //Output the pixelWindow
    WritePixel(pixelWindow[6]);    //Output the pixelWindow
    WritePixel(pixelWindow[7]);    //Output the pixelWindow
    
    pixelWindow[xc] = TRANSPARENTCOLOR;      
    
    DisplayDisable();     
}

/*********************************************************************
* Function: WORD GetPixel(SHORT x, SHORT y)
*
* PreCondition: none
*
* Input: x,y - pixel coordinates 
*
* Output: pixel color
*
* Side Effects: none
*
* Overview: returns pixel color at x,y position
*
* Note: none
*
********************************************************************/
GFX_COLOR GetPixel(SHORT x, SHORT y)
{
    return (0);
}

/*********************************************************************
* Function: WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
*
* PreCondition: none
*
* Input: left,top - top left corner coordinates,
*        right,bottom - bottom right corner coordinates
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the shape is not yet completely drawn.
*         - Returns 1 when the shape is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: draws rectangle filled with current color
*
* Note: none
*
********************************************************************/
WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
		
    DWORD xdelta,ydelta;
    DWORD loffset,roffset;


    if(left > right) 
        return(1);        //Added to make sure function call is accurate
    
    if(bottom < top) 
        return(1);        //Added to make sure function call is accurate


#if (DISP_ORIENTATION == 90)       //Added for rotation capabilities
    DWORD templ,tempr;
    templ = left; tempr = right;

    left = top;
    right = bottom;

    top = (DISP_VER_RESOLUTION-1) - tempr;
    bottom = (DISP_VER_RESOLUTION-1) - templ;
#endif

#if (DISP_ORIENTATION == 270)         //Added for rotation capabilities
    DWORD templ,tempr;
    templ = left; tempr = right;

    left = (DISP_HOR_RESOLUTION-1) - bottom;
    right = (DISP_HOR_RESOLUTION-1) - top;

    top = templ;
    bottom = tempr;
#endif

#if (DISP_ORIENTATION == 180)         //Added for rotation capabilities
    DWORD templ,tempr;
    templ = left; tempr = top;

    left = (DISP_HOR_RESOLUTION-1) - right;
    right = (DISP_HOR_RESOLUTION-1) - templ;

    top = (DISP_VER_RESOLUTION-1) - bottom;
    bottom = (DISP_VER_RESOLUTION-1) - tempr;
#endif

	if(left>0) 
        loffset = left & 7;
	else 
        loffset = 0;
	
    roffset = right & 7;
	roffset = 8 - roffset;


	DisplayEnable();     
	DisplaySetCommand();       
	DeviceWrite(REG5A_WRITE_WIN_X_SP);   

	DisplaySetData();    
    
    DeviceWrite(left >> 2);     
    DeviceWrite(top >> 2);     
    DeviceWrite(top);     
    DeviceWrite(((right+roffset)-8) >> 2);     
    DeviceWrite(bottom >> 2);     
    DeviceWrite(bottom); 
   
    WORD j,i;

	xdelta = (right - left) + loffset;
    roffset += xdelta;
	ydelta = bottom - top;

    for(i=0;i <= ydelta; i++)  
    {  
        j = 0;
        while(j < loffset)
        {
            WritePixel(TRANSPARENTCOLOR);
            j++;
        }

        while(j <= xdelta)
        {
            WritePixel(_color);
            j++;
        }

        while(j < roffset)
        {
            WritePixel(TRANSPARENTCOLOR);
            j++;
        }
    }

    DisplayDisable();     // disable S1D13517

    return (1);		
}

/*********************************************************************
* Function: IsDeviceBusy()
*
* Overview: Returns non-zero if LCD controller is busy 
*           (previous drawing operation is not completed).
*
* PreCondition: none
*
* Input: none
*
* Output: Busy status.
*
* Side Effects: none
*
********************************************************************/
WORD IsDeviceBusy(void)
{  
    return 0;
}

/*********************************************************************
* Function: SetClipRgn(left, top, right, bottom)
*
* Overview: Sets clipping region.
*
* PreCondition: none
*
* Input: left - Defines the left clipping region border.
*		 top - Defines the top clipping region border.
*		 right - Defines the right clipping region border.
*	     bottom - Defines the bottom clipping region border.
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void SetClipRgn(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
    _clipLeft=left;
    _clipTop=top;
    _clipRight=right;
    _clipBottom=bottom;

}

/*********************************************************************
* Function: SetClip(control)
*
* Overview: Enables/disables clipping.
*
* PreCondition: none
*
* Input: control - Enables or disables the clipping.
*			- 0: Disable clipping
*			- 1: Enable clipping
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void SetClip(BYTE control)
{
    _clipRgn=control;
}

#ifdef USE_DRV_OUTCHAR
/*********************************************************************
* Function: WORD OutChar(XCHAR ch)
*
* PreCondition: none
*
* Input: character code
*
* Output: For NON-Blocking configuration:
*         - Returns 0 when device is busy and the character is not yet completely drawn.
*         - Returns 1 when the character is completely drawn.
*         For Blocking configuration:
*         - Always return 1.
*
* Side Effects: none
*
* Overview: outputs a character
*
* Note: none
*
********************************************************************/
WORD OutChar(XCHAR ch)
{
	#ifdef USE_FONT_FLASH	
    GLYPH_ENTRY *pChTable = NULL;
	#endif
    BYTE        *pChImage = NULL;

    #ifdef USE_FONT_EXTERNAL
    GLYPH_ENTRY chTable;
    BYTE        chImage[EXTERNAL_FONT_BUFFER_SIZE];
    WORD        imageSize;
    DWORD_VAL   glyphOffset;
    #endif
    SHORT       chWidth = 0;
    SHORT       xCnt, yCnt, x = 0, y;
    BYTE        temp = 0, mask;

    #ifndef USE_NONBLOCKING_CONFIG
    while(IsDeviceBusy() != 0) 
        Nop();
    #else
    if(IsDeviceBusy() != 0)
        return (0);
    #endif

    if((XCHAR)ch < (XCHAR)_fontFirstChar)
        return (-1);

    if((XCHAR)ch > (XCHAR)_fontLastChar)
        return (-1);

    switch(*((SHORT *)_font))
    {
#ifdef USE_FONT_FLASH
        case FLASH:
            pChTable = (GLYPH_ENTRY *) (((FONT_FLASH *)_font)->address + sizeof(FONT_HEADER)) + ((XCHAR)ch - (XCHAR)_fontFirstChar);

            pChImage = (BYTE *) (((FONT_FLASH *)_font)->address + ((DWORD)(pChTable->offsetMSB) << 8) + pChTable->offsetLSB);

            chWidth = pChTable->width;

            break;
#endif
#ifdef USE_FONT_EXTERNAL
        case EXTERNAL:

            // get glyph entry
            ExternalMemoryCallback
            (
                _font,
                sizeof(FONT_HEADER) + ((XCHAR)ch - (XCHAR)_fontFirstChar) * sizeof(GLYPH_ENTRY),
                sizeof(GLYPH_ENTRY),
                &chTable
            );

            chWidth = chTable.width;

            // width of glyph in bytes
            imageSize = 0;
            if(chWidth & 0x0007)
                imageSize = 1;
            imageSize += (chWidth >> 3);

            // glyph image size
            imageSize *= _fontHeight;

            // get glyph image
            glyphOffset.w[1] = (chTable.offsetMSB >> 8);
            glyphOffset.w[0] = (chTable.offsetMSB << 8) + (chTable.offsetLSB);

            ExternalMemoryCallback(_font, glyphOffset.Val, imageSize, &chImage);
            pChImage = (BYTE *) &chImage;

            break;
#endif

        default:
            return (-1);
    }

    if(_fontOrientation == ORIENT_HOR)
    {
        SHORT left, right;
        SHORT top, bottom;
        DWORD leftOffset, rightOffset;
        WORD charDelta;

#if (DISP_ORIENTATION == 90)
        left = GetY();
        right = left + _fontHeight;

        bottom = (DISP_VER_RESOLUTION - 1) - GetX();
        top = bottom + chWidth;

#elif (DISP_ORIENTATION == 270)
        top = GetX();
        bottom = left + chWidth;

        right = (DISP_HOR_RESOLUTION - 1) - Get();
        left = right + _fontHeight;

#elif (DISP_ORIENTATION == 180)
        right = (DISP_HOR_RESOLUTION - 1) - GetX();
        left = right + chWidth;

        bottom = (DISP_VER_RESOLUTION - 1) - GetY();
        top = bottom + _fontHeight;
#else
        left = GetX();
        right = left + chWidth;
        top = GetY();
        bottom = top + _fontHeight;
#endif
        if(left > 0)
            leftOffset = left & 0x07;
        else
            leftOffset = 0;

        rightOffset = right & 0x07;
        rightOffset = 8 - rightOffset;

        DisplayEnable();     
	    DisplaySetCommand();       
	    DeviceWrite(REG5A_WRITE_WIN_X_SP);   

	    DisplaySetData();    
        
        DeviceWrite(left >> 2);     
        DeviceWrite(top >> 2);     
        DeviceWrite(top);     
        DeviceWrite(((right + rightOffset) - 8) >> 2);     
        DeviceWrite(bottom >> 2);     
        DeviceWrite(bottom); 

        charDelta = chWidth + leftOffset;
        rightOffset += charDelta;

        for(yCnt = 0; yCnt < _fontHeight; yCnt++)
        {
            xCnt = 0;

            while(xCnt < leftOffset)
            {
                WritePixel(TRANSPARENTCOLOR);
                xCnt++;
            }

            mask = 0;

            while(xCnt < charDelta)
            {
                if(mask == 0)
                {
                    temp = *pChImage++;
                    mask = 1;
                }

                if(temp & mask)
                {
                    WritePixel(_color);
                }else
                {
                    WritePixel(TRANSPARENTCOLOR);
                }
                
                xCnt++;
                mask <<= 1;
            }

            while(xCnt < rightOffset)
            {
                WritePixel(TRANSPARENTCOLOR);
                xCnt++;
            }
        }
        _cursorX += chWidth;
        DisplayDisable();     // disable S1D13517

    }
    else
    {
        y = GetX();
        for(yCnt = 0; yCnt < _fontHeight; yCnt++)
        {
            x = GetY();
            mask = 0; 
            for(xCnt = 0; xCnt < chWidth; xCnt++)
            {
                if(mask == 0)
                {
                    temp = *pChImage++;
                    mask = 0x01; 
                }

                if(temp & mask)
                {
                    PutPixel(y, x);
                }

                x--;
                mask <<= 1;
            }

            y++;
        }

        // move cursor
        _cursorY = x;
    }

    return (1);
}
#endif 

/*********************************************************************
* Function: void ClearDevice(void)
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
* Overview: clears screen with current color 
*
* Note: none
*
********************************************************************/
void ClearDevice(void)
{
	
    DisplayEnable();     
    DisplaySetCommand();      
    DeviceWrite(REG5A_WRITE_WIN_X_SP);

	DisplaySetData();   
	DeviceWrite(0x00);                  // X Start Position  
    DeviceWrite(0x00);                  // Y Start Position
    DeviceWrite(0x00);                  // Y Start Position
    DeviceWrite((DISP_HOR_RESOLUTION-1) >>2);     // X End Position 
    DeviceWrite((DISP_VER_RESOLUTION-1) >>2);     // Y End Position
    DeviceWrite(DISP_VER_RESOLUTION-1);         // Y End Position


    DWORD i;

    for(i = 0; i < ((DWORD)DISP_VER_RESOLUTION * (DWORD)DISP_HOR_RESOLUTION); i++)
        WritePixel(_color);

	DisplayDisable();
		
}

#ifdef USE_BITMAP_FLASH
/*********************************************************************
* Function: void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner,
*        bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage1BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch)
{

    register DWORD      address;
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    BYTE                temp = 0;
    WORD                sizeX, sizeY;
    WORD                x, y;
    BYTE                stretchX, stretchY;

    GFX_COLOR                pallete[2];

    BYTE                mask;
	WORD 				loffset,roffset;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

    #if (COLOR_DEPTH == 24)
    flashAddress += 2;
	#endif

#if (COLOR_DEPTH == 16)	
    pallete[0] = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    pallete[1] = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
#elif (COLOR_DEPTH == 24)	
    pallete[0] = *((FLASH_DWORD *)flashAddress);
    flashAddress += 4;
    pallete[1] = *((FLASH_DWORD *)flashAddress);
    flashAddress += 4;
#endif

 	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data
	
    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            //SetAddress(address);
            mask = 0;
            for(x = 0; x < sizeX + loffset + roffset; x++)
            {

			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
					WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {
                // Read 8 pixels from flash
                if(mask == 0)
                {
                    temp = *flashAddress;
                    flashAddress++;
                    mask = 0x80;
                }

                // Set color
                if(mask & temp)
                {
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(1);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[1]);
                    }
                }
                else
                {
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(0);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[0]);
                    }
                }

				// Write pixel to screen
				for(stretchX = 0; stretchX < stretch; stretchX++)
				{
					WritePixel(_color);
			    }

                // Shift to the next pixel
                mask >>= 1;
            }
		  }
		  
                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
    }


    DisplayDisable();
}

/*********************************************************************
* Function: void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs 16 color image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage4BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch)
{
    register DWORD      address;
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    WORD                sizeX, sizeY;
    register WORD       x, y;
    BYTE                temp = 0;
    register BYTE       stretchX, stretchY;
	WORD                image_x = 0;
	GFX_COLOR               pallete[16];

    WORD                counter;
	WORD 				loffset,roffset;
	
    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
	
	#if (COLOR_DEPTH == 24)
    flashAddress += 2;
	#endif
	
    // Read pallete
    for(counter = 0; counter < 16; counter++)
    {
	#if (COLOR_DEPTH == 16)	
        pallete[counter] = *((FLASH_WORD *)flashAddress);
        flashAddress += 2;
    #elif (COLOR_DEPTH == 24)	
		pallete[counter] = *((FLASH_DWORD *)flashAddress);
		flashAddress += 4;
	#endif
    }

 	if(left>0)
	  loffset = left % 8;
	else
	loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data
	
    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            //SetAddress(address);
            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {

			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
                image_x = 0;
              }
			  else
			  {
                // Read 2 pixels from flash
                if(image_x & 0x0001)
                {

                    // second pixel in byte
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(temp >> 4);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[temp >> 4]);
                    }
                }
                else
                {
                    temp = *flashAddress;
                    flashAddress++;
                        
                    // first pixel in byte
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(temp & 0x0f);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[temp & 0x0f]);
                    }
                }
                image_x++;

				// Write pixel to screen
				for(stretchX = 0; stretchX < stretch; stretchX++)
				{
					WritePixel(_color);
			      }
     		  }           
            }

                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
    }

    DisplayDisable();
}

/*********************************************************************
* Function: void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs 256 color image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch)
{
    register DWORD      address;
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    WORD                sizeX, sizeY;
    WORD                x, y;
    BYTE                temp;
    BYTE                stretchX, stretchY;
	
    GFX_COLOR               pallete[256];

    WORD                counter;
	WORD 				loffset,roffset;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

	#if (COLOR_DEPTH == 24)
    flashAddress += 2;
	#endif
	
    // Read pallete
    for(counter = 0; counter < 256; counter++)
    {
	#if (COLOR_DEPTH == 16)	
        pallete[counter] = *((FLASH_WORD *)flashAddress);
        flashAddress += 2;       
    #elif (COLOR_DEPTH == 24)	
		pallete[counter] = *((FLASH_DWORD *)flashAddress);
		flashAddress += 4;
	#endif
    }

 	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data


    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            //SetAddress(address);
            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {
			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {

					// Read pixels from flash
					temp = *flashAddress;
					flashAddress++;

	               // Set color
	                #ifdef USE_PALETTE
	                if(IsPaletteEnabled())
	                {
	                    SetColor(temp);
	                }
	                else
	                #endif
	                {
	                    SetColor(pallete[temp]);
	                }

					// Write pixel to screen
					for(stretchX = 0; stretchX < stretch; stretchX++)
					{
						WritePixel(_color);
				    }
			  }
            }

                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
    }

    DisplayDisable();
}

/*********************************************************************
* Function: void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs hicolor image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage16BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch)
{
    register DWORD      address;
    register FLASH_WORD *flashAddress;
    register FLASH_WORD *tempFlashAddress;
    
    WORD                sizeX, sizeY;
    register WORD       x, y;

    GFX_COLOR                	temp;

    register BYTE       stretchX, stretchY;
	WORD 				loffset,roffset;
	
    // Move pointer to size information
    flashAddress = (FLASH_WORD *)bitmap + 1;

    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

	// Read image size
    sizeY = *flashAddress;
    flashAddress++;
    sizeX = *flashAddress;
    flashAddress++;
	
	if(left>0)
	  loffset = left % 8;
	else
	loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data
	
    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;

            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {

				if((x<=loffset) || (x > (sizeX+loffset)))
				{
					WritePixel(TRANSPARENTCOLOR);
					}
					else
					{
						// Read pixels from flash
						temp = *flashAddress;
						flashAddress++;

                      #if (COLOR_DEPTH == 24) 
                        temp = RGBConvert(RED5(temp),GREEN6(temp),BLUE5(temp)); 
                      #endif
						// Set color
						SetColor(temp);

						// Write pixel to screen
						for(stretchX = 0; stretchX < stretch; stretchX++)
						{
							WritePixel(_color);
						}
					}
			}
                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
    }

    DisplayDisable();
}


#if (COLOR_DEPTH == 24)	
/*********************************************************************
* Function: void PutImage24BPP(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs hicolor image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage24BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch)
{

	register DWORD 	*flashAddress;
	register DWORD 	*tempFlashAddress;
	WORD                	sizeX, sizeY;
	register WORD       	x, y;
	DWORD                	temp;
	register BYTE       	stretchX, stretchY;
	WORD 					loffset,roffset;
	
	// Move pointer to size information
	flashAddress = (DWORD *)bitmap + 1;

	// Read image size
	sizeY = *((FLASH_WORD *)flashAddress);
	((FLASH_WORD *)flashAddress)++;
	sizeX = *((FLASH_WORD *)flashAddress);
	((FLASH_WORD *)flashAddress)++;
	((FLASH_WORD *)flashAddress)++;

	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data

    for(y = 0; y <= sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {
			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {
					// Read pixels from flash
					temp = *flashAddress;
					flashAddress++;

					// Set color
					SetColor(temp);

					// Write pixel to screen
					for(stretchX = 0; stretchX < stretch; stretchX++)
					{
						WritePixel(_color);
				    }
			  }
            }

        }
    }
    
    DisplayDisable();
}
#endif

#endif
#ifdef USE_BITMAP_EXTERNAL

/*********************************************************************
* Function: void PutImage1BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage1BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch)
{
    register DWORD  address;
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;	
    GFX_COLOR       pallete[2];

    BYTE            lineBuffer[((GetMaxX() + 1) / 8) + 1];
    BYTE            *pData;
    SHORT           byteWidth;

    BYTE            temp = 0;
    BYTE            mask;
    WORD            sizeX, sizeY;
    WORD            x, y;
    BYTE            stretchX, stretchY;
	WORD 			loffset,roffset;
	
    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);
	
    // Get pallete (2 entries)
    ExternalMemoryCallback(bitmap, sizeof(BITMAP_HEADER), 2 * sizeof(GFX_COLOR), pallete);
    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER) + 2 * sizeof(GFX_COLOR);


    // Line width in bytes
    byteWidth = bmp.width >> 3;
    if(bmp.width & 0x0007)
        byteWidth++;

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data
	
    for(y = 0; y < sizeY; y++)
    {

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            pData = lineBuffer;
            //SetAddress(address);
            mask = 0;
            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {
			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {
                // Read 8 pixels from flash
                if(mask == 0)
                {
                    temp = *pData++;
                    mask = 0x80;
                }

                // Set color
                if(mask & temp)
                {
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(1);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[1]);
                    }
                }
                else
                {
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(0);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[0]);
                    }
                }

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
					WritePixel(_color);
                }

                // Shift to the next pixel
                mask >>= 1;
     		 }           
            }

                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
    }
    DisplayDisable();
}

/*********************************************************************
* Function: void PutImage4BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage4BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch)
{
    register DWORD  address;
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
	
	GFX_COLOR       pallete[16];

    BYTE            lineBuffer[((GetMaxX() + 1) / 2) + 1];
    BYTE            *pData;
    SHORT           byteWidth;

    BYTE            temp = 0;
    WORD            sizeX, sizeY;
    WORD            x, y;
    BYTE            stretchX, stretchY;
	WORD 			loffset,roffset;
	
    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);
	
    // Get pallete (16 entries)
    ExternalMemoryCallback(bitmap, sizeof(BITMAP_HEADER), 16 * sizeof(GFX_COLOR), pallete);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER) + 16 * sizeof(GFX_COLOR);


    // Line width in bytes
    byteWidth = bmp.width >> 1;
    if(bmp.width & 0x0001)
        byteWidth++;

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;


	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data
	
    for(y = 0; y < sizeY; y++)
    {

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;

        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            pData = lineBuffer;
            //SetAddress(address);

            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {
			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {

                // Read 2 pixels from flash
                if(x & 0x0001)
                {
                    // second pixel in byte
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(temp >> 4);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[temp >> 4]);
                    }
                }
                else
                {
                    temp = *pData++;

                    // first pixel in byte
                    #ifdef USE_PALETTE
                    if(IsPaletteEnabled())
                    {
                        SetColor(temp & 0x0f);
                    }
                    else
                    #endif
                    {
                        SetColor(pallete[temp & 0x0f]);
                    }
                }

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
					WritePixel(_color);
                }
            }

                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
		}
        
    }
    DisplayDisable();
}

/*********************************************************************
* Function: void PutImage8BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage8BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch)
{
    register DWORD  address;
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;

    GFX_COLOR                pallete[256];

    BYTE            lineBuffer[(GetMaxX() + 1)];
    BYTE            *pData;

    BYTE            temp;
    WORD            sizeX, sizeY;
    WORD            x, y;
    BYTE            stretchX, stretchY;
    WORD 				loffset,roffset;

    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);
	
    // Get pallete (256 entries)
    ExternalMemoryCallback(bitmap, sizeof(BITMAP_HEADER), 256 * sizeof(GFX_COLOR), pallete);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER) + 256 * sizeof(GFX_COLOR);


    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data
	
    for(y = 0; y < sizeY; y++)
    {

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, sizeX, lineBuffer);
        memOffset += sizeX;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            pData = lineBuffer;
            //SetAddress(address);

           for(x = 0; x < (sizeX + loffset + roffset); x++)
            {
			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {
                temp = *pData++;

                #ifdef USE_PALETTE
                if(IsPaletteEnabled())
                {
                    SetColor(temp);
                }
                else
                #endif
                {
                    SetColor(pallete[temp]);
                }

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
					WritePixel(_color);
                }
            }

                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }
		}
        
    }
    DisplayDisable();
}

/*********************************************************************
* Function: void PutImage16BPPExt(SHORT left, SHORT top, void* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs monochrome image starting from left,top coordinates
*
* Note: image must be located in external memory
*
********************************************************************/
void PutImage16BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch)
{
    register DWORD  address;
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
    WORD            lineBuffer[(GetMaxX() + 1)];
    WORD            *pData;
    WORD            byteWidth;

    WORD            temp;
    WORD            sizeX, sizeY;
    WORD            x, y;
    BYTE            stretchX, stretchY;

    // Set start address
        #ifndef USE_PALETTE
    address = ((DWORD) (GetMaxX() + 1) * top + left) << 1;
        #else
    address = (((DWORD) (GetMaxX() + 1) * top + left) * PaletteBpp) >> 3;
        #endif

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);

    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER);

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

    byteWidth = sizeX << 1;

    for(y = 0; y < sizeY; y++)
    {

        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;
        DisplayEnable();      // enable SSD1926
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            pData = lineBuffer;
            SetAddress(address);

            for(x = 0; x < sizeX; x++)
            {
                temp = *pData++;

                #if (COLOR_DEPTH == 24) 
                temp = RGBConvert(RED5(temp),GREEN6(temp),BLUE5(temp)); 
                #endif

                #ifdef USE_PALETTE
                if(IsPaletteEnabled())
                {
                    SetColor(0); /* Paint first value of Palette instead of junk */
                }
                else
                #endif
                {
                    SetColor(temp);
                }

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
                    WritePixel(_color);
                }
            }

                #ifndef USE_PALETTE
            address += (GetMaxX() + 1) << 1;
                #else
            address += ((GetMaxX() + 1) * PaletteBpp) >> 3;
                #endif
        }

        DisplayDisable();
    }
}



#if (COLOR_DEPTH == 24)	
/*********************************************************************
* Function: void PutImage24BPPExt(SHORT left, SHORT top, FLASH_BYTE* bitmap, BYTE stretch)
*
* PreCondition: none
*
* Input: left,top - left top image corner, bitmap - image pointer,
*        stretch - image stretch factor
*
* Output: none
*
* Side Effects: none
*
* Overview: outputs hicolor image starting from left,top coordinates
*
* Note: image must be located in flash
*
********************************************************************/
void PutImage24BPPExt(SHORT left, SHORT top, void *bitmap, BYTE stretch)
{


    register DWORD  address;
    register DWORD  memOffset;
    BITMAP_HEADER   bmp;
    DWORD            lineBuffer[(GetMaxX() + 1)];
    DWORD            *pData;
    WORD            byteWidth;
    

	WORD                	sizeX, sizeY;
	register WORD       	x, y;
	DWORD                	temp;
	register BYTE       	stretchX, stretchY;
	WORD 					loffset,roffset;

    // Get bitmap header
    ExternalMemoryCallback(bitmap, 0, sizeof(BITMAP_HEADER), &bmp);
    // Set offset to the image data
    memOffset = sizeof(BITMAP_HEADER);

    // Get size
    sizeX = bmp.width;
    sizeY = bmp.height;

    byteWidth = sizeX << 2;

	

	if(left>0)
	  loffset = left % 8;
	else
	  loffset = 0;
	roffset = (left + (sizeX*stretch)) % 8;
	roffset = 8 - roffset;

	DisplayEnable();     // enable S1D13517
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(REG5A_WRITE_WIN_X_SP);
	DisplaySetData();    // set RS line to high for data
	DeviceWrite((left>>3)<<1);
	DeviceWrite(top>>2);
	DeviceWrite(top);
	DeviceWrite(((left+(sizeX*stretch)+roffset-8)>>3)<<1);
	DeviceWrite((top+(sizeY*stretch))>>2);
	DeviceWrite((top+(sizeY*stretch)));
	DisplaySetCommand(); // set RS line to low for command
	DeviceWrite(0x66);
	DisplaySetData();    // set RS line to high for data

    for(y = 0; y <= sizeY; y++)
    {
        // Get line
        ExternalMemoryCallback(bitmap, memOffset, byteWidth, lineBuffer);
        memOffset += byteWidth;
        
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
	        pData = lineBuffer;
            
            for(x = 0; x < (sizeX + loffset + roffset); x++)
            {
			  if((x<=loffset) || (x > (sizeX+loffset)))
              {
				WritePixel(TRANSPARENTCOLOR);
              }
			  else
			  {

					// Read pixels from flash
					temp = *pData++;

					// Set color
					SetColor(temp);

					// Write pixel to screen
					for(stretchX = 0; stretchX < stretch; stretchX++)
					{
						WritePixel(_color);
				    }
			  }
            }

        }
    }
    
    DisplayDisable();
}
#endif

#endif

#ifdef USE_PALETTE

/*********************************************************************
* Function: void PaletteInit(void)
*
* Overview: Initializes the CLUT.
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects: Drawing mode will change to support palettes
*
********************************************************************/
void PaletteInit(void)
{
    PaletteBpp = 16;
    blPaletteChangeError = 0;
    pPendingPalette = NULL;
    PendingStartEntry = 0;
    PendingLength = 0;
}

/*********************************************************************
* Function: BYTE SetPaletteBpp(BYTE bpp)
*
* Overview: Sets the CLUT's number of valid entries.
*
* PreCondition: PaletteInit() must be called before.
*
* Input: bpp -> Bits per pixel
*
* Output: Status: Zero -> Success, Non-zero -> Error.
*
* Side Effects: Drawing mode will change to support palettes
*
********************************************************************/
BYTE SetPaletteBpp(BYTE bpp)
{
    switch(bpp)
    {
        /*case 1: SetReg(REG_DISPLAY_MODE,0x00);
                break;

        case 2: SetReg(REG_DISPLAY_MODE,0x01);
                break;

        case 4: SetReg(REG_DISPLAY_MODE,0x02);
                break;*/
        case 8:     SetReg(REG_DISPLAY_MODE, 0x03); break;
        case 16:    SetReg(REG_DISPLAY_MODE, 0x04); break;
        default:    SetReg(REG_DISPLAY_MODE, 0x04); PaletteBpp = 16; return (-1);
    }

    PaletteBpp = bpp;

    /////////////////////////////////////////////////////////////////////
    // ROTATION MODE
    /////////////////////////////////////////////////////////////////////
        #if (DISP_ORIENTATION == 0)
            #define WIN_START_ADDR  0ul
            #define ROTATION        0

        #elif (DISP_ORIENTATION == 90)
            #ifndef USE_PALETTE
                #define WIN_START_ADDR  ((((DWORD) GetMaxX() + 1) >> 1) - 1)
            #else
                #define WIN_START_ADDR  (((((DWORD) GetMaxX() + 1) * PaletteBpp) >> 5) - 1)
            #endif
            #define ROTATION    1

        #elif (DISP_ORIENTATION == 180)
            #ifndef USE_PALETTE
                #define WIN_START_ADDR  (((((DWORD) GetMaxX() + 1) * (GetMaxY() + 1)) >> 1) - 1)
            #else
                #define WIN_START_ADDR  (((((DWORD) GetMaxX() + 1) * (GetMaxY() + 1) * PaletteBpp) >> 5) - 1)
            #endif
            #define ROTATION    2

        #elif (DISP_ORIENTATION == 270)
            #ifndef USE_PALETTE
                #define WIN_START_ADDR  ((((DWORD) GetMaxX() + 1) * GetMaxY()) >> 1)
            #else
                #define WIN_START_ADDR  ((((DWORD) GetMaxX() + 1) * GetMaxY() * PaletteBpp) >> 5)
            #endif
            #define ROTATION    3
        #endif

    /////////////////////////////////////////////////////////////////////
    // Special Effects Register (reg 71h)
    /////////////////////////////////////////////////////////////////////
        #ifndef USE_PALETTE
    SetReg(REG_SPECIAL_EFFECTS, 0x40 | ROTATION);
        #else
    SetReg(REG_SPECIAL_EFFECTS, 0x00 | ROTATION);
        #endif

    /////////////////////////////////////////////////////////////////////
    // Main Window Display Start Address (regs 74h, 75h, 76h)
    /////////////////////////////////////////////////////////////////////
    SetReg(REG_MAIN_WIN_DISP_START_ADDR0, WIN_START_ADDR & 0x00FF);
    SetReg(REG_MAIN_WIN_DISP_START_ADDR1, (WIN_START_ADDR >> 8) & 0x00FF);
    SetReg(REG_MAIN_WIN_DISP_START_ADDR2, (WIN_START_ADDR >> 16) & 0x00FF);

    /////////////////////////////////////////////////////////////////////
    // Main Window Display Offset (regs 78h, 79h)
    /////////////////////////////////////////////////////////////////////
        #ifndef USE_PALETTE
            #define WIN_OFFSET  ((GetMaxX() + 1) >> 1)
        #else
            #define WIN_OFFSET  (((GetMaxX() + 1) * PaletteBpp) >> 5)
        #endif
    SetReg(REG_MAIN_WIN_ADDR_OFFSET0, WIN_OFFSET & 0x00FF);
    SetReg(REG_MAIN_WIN_ADDR_OFFSET1, (WIN_OFFSET >> 8) & 0x00FF);

    return (0);
}

/*********************************************************************
* Function: void EnablePalette(void)
*
* Overview: Enables the Palette mode
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects:
*
********************************************************************/
void EnablePalette(void)
{
    SetPaletteBpp(PaletteBpp);
}

/*********************************************************************
* Function: void DisablePalette(void)
*
* Overview: Disables the Palette mode
*
* PreCondition: none
*
* Input: none
*
* Output: none
*
* Side Effects:
*
********************************************************************/
void DisablePalette(void)
{
    SetPaletteBpp(16);
}

/*********************************************************************
* Function: BYTE IsPaletteEnabled(void)
*
* Overview: Returns if the Palette mode is enabled or not
*
* PreCondition: none
*
* Input: none
*
* Output: Enabled -> 1, Disabled -> 0
*
* Side Effects:
*
********************************************************************/
BYTE IsPaletteEnabled(void)
{
    return ((PaletteBpp == 16) ? 0 : 1);
}

/*********************************************************************
* Function: void StartVBlankInterrupt(void)
*
* Overview: Sets up the Vertical Blanking Interrupt
*
* PreCondition: Interrupts must be enabled
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void StartVBlankInterrupt(void)
{

    /* We don't use Vertical Blanking Interrupt in SSD1926 */
    if(pPendingPalette != NULL)
    {
        blPaletteChangeError = SetPalette(pPendingPalette, PendingStartEntry, PendingLength);
        if(!blPaletteChangeError)
        {
            _palette = pPendingPalette;
        }
    }
}

/*********************************************************************
* Function: BYTE SetPaletteFlash(PALETTE_ENTRY *pPaletteEntry, WORD startEntry, WORD length)
*
* Overview: Loads the palettes from the flash immediately.
*
* PreCondition: PaletteInit() must be called before.
*
* Input: pPaletteEntry   - Pointer to the palette table in ROM
*        startEntry      - Start entry to load (inclusive)
*        length          - Number of entries
*
* Output: Status: Zero -> Success, Non-zero -> Error.
*
* Side Effects: There may be a slight flicker when the Palette entries
*               are getting loaded one by one.
*
********************************************************************/
BYTE SetPaletteFlash(PALETTE_ENTRY *pPaletteEntry, WORD startEntry, WORD length)
{
    WORD    counter;

    if((pPaletteEntry == NULL) || ((startEntry + length) > 256))
    {
        return (-1);
    }

    for(counter = 0; counter < length; counter++)
    {
        SetReg(REG_LUT_RED_WRITE_DATA, RED8(pPaletteEntry[counter].value));
        SetReg(REG_LUT_GREEN_WRITE_DATA, GREEN8(pPaletteEntry[counter].value));
        SetReg(REG_LUT_BLUE_WRITE_DATA, BLUE8(pPaletteEntry[counter].value));
        SetReg(REG_LUT_WRITE_ADDR, startEntry + counter);
    }

    return (0);
}

#endif


/*********************************************************************
* Function: void DisplayBrightness(WORD level)
********************************************************************/
void DisplayBrightness(WORD level)
{

   if(level == 100)
   {
       SetReg(REG70_PWM_CONTROL,0x85);          //Turn on Backlight 
   }else if (level == 0) 
   {    
       SetReg(REG70_PWM_CONTROL,0x84);       //Turn off Backlight 
   }else if (level <=50)
   {
       level >>= 1;
       level *=  5;  //Sets the value from (0-250)

       SetReg(REG72_PWM_HIGH_DC_0,0xff);
       SetReg(REG74_PWM_HIGH_DC_1,level);
       SetReg(REG7A_PWM_LOW_DC_0,0xff);
       SetReg(REG7C_PWM_LOW_DC_1,0xff);
     
       SetReg(REG70_PWM_CONTROL,0x86);   //Turn off Backlight PWM
       }
       else
       {
       level >>= 1;
       level *=  5;  //Sets the value from (0-250)
       SetReg(REG72_PWM_HIGH_DC_0,level);
       SetReg(REG74_PWM_HIGH_DC_1,0xff);
       SetReg(REG7A_PWM_LOW_DC_0,0xff);
       SetReg(REG7C_PWM_LOW_DC_1,0xff);
     
       SetReg(REG70_PWM_CONTROL,0x86);   //Turn off Backlight PWM
   } 

  
}


#define BUFFER_SIZE 0x140000l  //WVGA


/*********************************************************************
* Function: DWORD GFXGetPageOriginAddress(SHORT pageNumber)
********************************************************************/
DWORD GFXGetPageOriginAddress(SHORT pageNumber)
{
	return(BUFFER_SIZE * (DWORD)pageNumber);
}

/*********************************************************************
* Function: DWORD GFXGetPageXYAddress(SHORT pageNumber, WORD x, WORD y)
********************************************************************/
DWORD GFXGetPageXYAddress(SHORT pageNumber, WORD x, WORD y)
{
    DWORD address;

    address = (DWORD)y * (DWORD)DISP_HOR_RESOLUTION;
    address += (DWORD)x;
    address *= 3;
    address += GFXGetPageOriginAddress(pageNumber);

    return address;
}

/*********************************************************************
* Function: DWORD GFXGetWindowOriginAddress(GFX_WINDOW_INFO* GFXWindowInfo)
********************************************************************/
DWORD GFXGetWindowOriginAddress(GFX_WINDOW_INFO* GFXWindowInfo)
{
    return (0);
}

/*********************************************************************
* Function: DWORD GFXGetWindowXYAddress(GFX_WINDOW_INFO* GFXWindowInfo, WORD relX, WORD relY)
********************************************************************/
DWORD GFXGetWindowXYAddress(GFX_WINDOW_INFO* GFXWindowInfo, WORD relX, WORD relY)
{
    return (0);
}



#define 	NO_ALPHA_BLENDING 	0

 #ifdef USE_ALPHABLEND
/*********************************************************************
* Function: void AlphaBlendWindow(DWORD foregroundWindowAddr, DWORD backgroundWindowAddr,
					  DWORD destinationWindowAddr,		            
					  WORD  width, WORD height,  	
					  BYTE  alphaPercentage)
********************************************************************/
void AlphaBlendWindow(DWORD foregroundWindowAddr, DWORD backgroundWindowAddr,
					  DWORD destinationWindowAddr,		            
					  WORD  width, WORD height,  	
					  BYTE  alphaPercentage)
{
	
	SetReg(REG98_ALP_HR_SIZE,((width-1)>>3)); 
    SetReg(REG9A_ALP_VR_SIZE_0,height); 
    SetReg(REG9C_ALP_VR_SIZE_1,(height>>8)); 

    foregroundWindowAddr  -=(foregroundWindowAddr % 8);
    backgroundWindowAddr  -=(backgroundWindowAddr % 8);
    destinationWindowAddr -=(destinationWindowAddr % 8);
	
    SetReg(REGA0_ALP_IN_IMG1_SA_0,foregroundWindowAddr);    
    SetReg(REGA2_ALP_IN_IMG1_SA_1,(foregroundWindowAddr>>8)); 
    SetReg(REGA4_ALP_IN_IMG1_SA_2,(foregroundWindowAddr>>16)); 
    SetReg(REGA6_ALP_IN_IMG2_SA_0,backgroundWindowAddr);    
    SetReg(REGA8_ALP_IN_IMG2_SA_1,(backgroundWindowAddr>>8)); 
    SetReg(REGAA_ALP_IN_IMG2_SA_2,(backgroundWindowAddr>>16)); 
    SetReg(REGAC_ALP_OUT_IMG_SA_0,destinationWindowAddr);  
    SetReg(REGAE_ALP_OUT_IMG_SA_1,(destinationWindowAddr>>8)); 
    SetReg(REGB0_ALP_OUT_IMG_SA_2,(destinationWindowAddr>>16)); 
	
	SetReg(REG9E_ALP_VALUE,0x80 | Percentage2Alpha(alphaPercentage));  
	SetReg(REGB2_INTERRUPT_CTRL,0x01);				//Enable Alpha Blend Interrupt 
    SetReg(REG94_ALP_CONTROL,0x01); 				//Enable Alpha Blend  
	SetReg(REG94_ALP_CONTROL,0x00);                 // Added From DataSheet

 	while(GetReg(REGB4_INTERRUPT_STAT) != 1);

	SetReg(REGB6_INTERRUPT_CLEAR,0x01);              //Added for DEBUG
	SetReg(REGB6_INTERRUPT_CLEAR,0x00);				//Clear Interrupt
	SetReg(REGB2_INTERRUPT_CTRL,0x00);              //Enable Alpha Blend Interrupt 
	SetReg(REG94_ALP_CONTROL,0x00); 				//Disable Alpha Blend  

}			

void CopyPageWindow( BYTE srcPage, BYTE dstPage,       
                        WORD srcX, WORD srcY, WORD dstX, WORD dstY, 
                        WORD width, WORD height)			
{
			AlphaBlendWindow(GFXGetPageXYAddress(srcPage,srcX,srcY), GFXGetPageXYAddress(srcPage,srcX,srcY), GFXGetPageXYAddress(dstPage,dstX,dstY), width, height,                               
                             NO_ALPHA_BLENDING);	
}

#endif
#ifdef USE_TRANSITION_EFFECTS
/*********************************************************************
* Function: WORD CopyBlock(DWORD srcAddr, DWORD dstAddr, DWORD srcOffset, DWORD dstOffset, WORD width, WORD height)
*
* Overview: Moves a block of data from source specified by srcAddr 
*           and srcOffset to the destination specified by dstAddr 
*           and dstOffset.
*
* PreCondition: none
*
* Input: srcAddr - the base address of the data to be moved
*        dstAddr - the base address of the new location of the moved data 
*        srcOffset - offset of the data to be moved with respect to the 
*                    source base address.
*        dstOffset - offset of the new location of the moved data respect 
*                    to the source base address.
*        width - width of the block of data to be moved
*        height - height of the block of data to be moved
*
* Output: none
* Side Effects: none
* Note: none
********************************************************************/
WORD CopyBlock(DWORD srcAddr, DWORD dstAddr, DWORD srcOffset, DWORD dstOffset, WORD width, WORD height)
{

    DWORD source = srcAddr + 3*srcOffset;
    DWORD destination = dstAddr + 3*dstOffset;
    
    
    AlphaBlendWindow(source, source, destination, width, height,                               
                                 NO_ALPHA_BLENDING);

    return (0);
}

/*********************************************************************
* Function:  SetActivePage(page)
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
void SetActivePage(WORD page)
{
    SetReg(REG52_INPUT_MODE, 0x08 | (page<<4));
}

/*********************************************************************
* Function: SetVisualPage(page)
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
void SetVisualPage(WORD page)
{	
    SetReg(REG2A_DSP_MODE, (0x01 | (page<<4))); 
    SetReg(REG50_DISPLAY_CONTROL,0x80);
}  

#ifdef USE_TRANSITION_EFFECTS
extern WORD _transitionpending, _left, _top, _right, _bottom, _type, _delay_ms, _param1, _param2;
extern DWORD _srcpageaddr, _destpageaddr;

extern WORD Startx;
extern WORD Starty;
extern WORD Width;
extern WORD Height;
/*********************************************************************
* Function: void PushRectangle(void)
*
* Overview: Moves window in a certain direction
*
* PreCondition: direction and window parameters defined
*
* Input: none
*
* Output: none
*
* Side Effects: none
*
********************************************************************/
void PushRectangle(void)
{
WORD direction = _param2;
int i;
long shift;


    SetReg(REG2A_DSP_MODE,0x09);      //Turn on both PIP1 + PIP2
 
   //Shift PIP1 screen (which is the original screen user was on)///
	SetReg(REG32_PIP1_WIN_X_SP, Startx);
    SetReg(REG38_PIP1_WIN_X_EP, (Startx +(Width-1)) >> 2   );
	SetReg(REG34_PIP1_WIN_Y_SP_0, Starty >> 2);  
	SetReg(REG36_PIP1_WIN_Y_SP_1, Starty);
	SetReg(REG3A_PIP1_WIN_Y_EP_0, (Starty+Height) >> 2  );
	SetReg(REG3C_PIP1_WIN_Y_EP_1, Starty + Height );

    SetReg(REG44_PIP2_WIN_X_SP, Startx);
    SetReg(REG4A_PIP2_WIN_X_EP, (Startx + (Width - 1)) >> 2   );
	SetReg(REG46_PIP2_WIN_Y_SP_0, Starty >> 2);  
	SetReg(REG48_PIP2_WIN_Y_SP_1, Starty);
	SetReg(REG4C_PIP2_WIN_Y_EP_0, (Starty + Height) >> 2  );
	SetReg(REG4E_PIP2_WIN_Y_EP_1, (Starty + Height) );


    switch(direction)
    {

    case LEFT_TO_RIGHT:
            for(i=0; i < (Width-9); i=i+8)
            {
                 shift = _destpageaddr;    //Page 2

                 SetReg(REG2C_PIP1_DSP_SA_0,shift); 
	             SetReg(REG2E_PIP1_DSP_SA_1,shift >> 8);
	             SetReg(REG30_PIP1_DSP_SA_2,shift >> 16);
                 SetReg(REG32_PIP1_WIN_X_SP, (Startx+i) >> 2  );

                 shift = _srcpageaddr;
                 shift += (DISP_HOR_RESOLUTION-i-8)*3;

                 SetReg(REG3E_PIP2_DSP_SA_0,shift); 
	             SetReg(REG40_PIP2_DSP_SA_1,shift >> 8);
	             SetReg(REG42_PIP2_DSP_SA_2,shift >> 16); 
                 SetReg(REG32_PIP1_WIN_X_SP, (Startx+i) >> 2  ); 
                 
                 SetReg(REG50_DISPLAY_CONTROL,0x80);          //Refresh must be callsed after swapping screens
                 DelayMs(_delay_ms);
                 
            }
        break;
    case RIGHT_TO_LEFT:
            for(i=0; i < (Width-9); i=i+8)
            {
                 shift = _destpageaddr;    //Page 2
                 shift += 3 * i;
                 SetReg(REG2C_PIP1_DSP_SA_0,shift); 
	             SetReg(REG2E_PIP1_DSP_SA_1,shift >> 8);
	             SetReg(REG30_PIP1_DSP_SA_2,shift >> 16);
                 SetReg(REG38_PIP1_WIN_X_EP, ((Startx+Width-1)- 8-i) >> 2 );

                 shift = _srcpageaddr;

                 SetReg(REG3E_PIP2_DSP_SA_0,shift); 
	             SetReg(REG40_PIP2_DSP_SA_1,shift >> 8);
	             SetReg(REG42_PIP2_DSP_SA_2,shift >> 16); 
                 SetReg(REG44_PIP2_WIN_X_SP,((Startx+Width-1) - i) >> 2  ); 
                 
                 SetReg(REG50_DISPLAY_CONTROL,0x80);          //Refresh must be callsed after swapping screens
                 DelayMs(_delay_ms);

            }
        break;
    case TOP_TO_BOTTOM:
            for(i=0; i < Height; i=i+8)
            {
                 shift = _destpageaddr;    //Page 2

                 SetReg(REG2C_PIP1_DSP_SA_0,shift); 
	             SetReg(REG2E_PIP1_DSP_SA_1,shift >> 8);
	             SetReg(REG30_PIP1_DSP_SA_2,shift >> 16);
                 SetReg(REG34_PIP1_WIN_Y_SP_0, (Starty+i) >> 2);  
                 SetReg(REG36_PIP1_WIN_Y_SP_1, (Starty+i));

                 shift = (long)DISP_VER_RESOLUTION - (long)i;
                 shift *= (long)DISP_HOR_RESOLUTION;
                 shift *= 3;
                 shift += _srcpageaddr;

                 SetReg(REG3E_PIP2_DSP_SA_0,shift); 
	             SetReg(REG40_PIP2_DSP_SA_1,shift >> 8);
	             SetReg(REG42_PIP2_DSP_SA_2,shift >> 16); 
                 SetReg(REG4C_PIP2_WIN_Y_EP_0, (Starty+i) >> 2  );
	             SetReg(REG4E_PIP2_WIN_Y_EP_1, (Starty+i) ); 
                 
                 SetReg(REG50_DISPLAY_CONTROL,0x80);          //Refresh must be callsed after swapping screens
                 DelayMs(_delay_ms);
            }
        break;
    case BOTTOM_TO_TOP:
            for(i=0; i < Height; i=i+8)
            {
                 shift = (long)DISP_HOR_RESOLUTION * (long)i;
                 shift *= 3;
                 shift += _destpageaddr;

                 SetReg(REG2C_PIP1_DSP_SA_0,shift); 
	             SetReg(REG2E_PIP1_DSP_SA_1,shift >> 8);
	             SetReg(REG30_PIP1_DSP_SA_2,shift >> 16);
                 SetReg(REG3A_PIP1_WIN_Y_EP_0, ((Starty+Height) - i) >> 2  );
	             SetReg(REG3C_PIP1_WIN_Y_EP_1, ((Starty+Height) - i) );

                 shift = _srcpageaddr;

                 SetReg(REG3E_PIP2_DSP_SA_0,shift); 
	             SetReg(REG40_PIP2_DSP_SA_1,shift >> 8);
	             SetReg(REG42_PIP2_DSP_SA_2,shift >> 16); 
                 SetReg(REG46_PIP2_WIN_Y_SP_0,((Starty+Height) - i) >> 2);  
                 SetReg(REG48_PIP2_WIN_Y_SP_1, ((Starty+Height)) - i); 
                 
                 SetReg(REG50_DISPLAY_CONTROL,0x80);          //Refresh must be callsed after swapping screens
                 DelayMs(_delay_ms);


            }
        break;
    default: 
        break;
    }

    PlainCopyRectangle();

    SetReg(REG2A_DSP_MODE,0x01);                           //Turn off PIP
    SetReg(REG50_DISPLAY_CONTROL,0x80);                    //Refresh must be callsed after swapping screens 

}
#endif
#endif
#endif //GFX_USE_DISPLAY_CONTROLLER_S1D13517

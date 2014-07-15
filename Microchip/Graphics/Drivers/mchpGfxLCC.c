/*********************************************************************
 *                  LCC Graphics Dirver (MicrochipGraphicsDMA)
 *********************************************************************
 * FileName:        MicrochipGraphicsDMA.c
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
#include "HardwareProfile.h"

#if defined (GFX_USE_DISPLAY_CONTROLLER_DMA)
#warning "The LCC Demo board works with an explorer 16 as a proof of concept, but noise is sometimes seen on the LCD panel"
#include "Compiler.h"
#include "TimeDelay.h"
#include "Graphics/DisplayDriver.h"
#include "Graphics/mchpGfxLCC.h"
#include "Graphics/gfxtcon.h"
#include "Graphics/Primitive.h"

#if defined(__32MX460F512L__) || defined (__32MX360F512L__) 
#define LEGACY_MODE
#define MAX_DMA_TRANSFER 254
#endif

#ifdef USE_PALETTE
    #include "Graphics/Palette.h"

    extern void *_palette;
    extern BYTE PaletteBpp;
    extern BYTE blPaletteChangeError;
    extern void *pPendingPalette;
    extern WORD PendingStartEntry;
    extern WORD PendingLength;
    WORD LUT[256];
#endif

/*This defines the color of the system*/
GFX_COLOR _color;

/* Clipping region control */
SHORT   _clipRgn;

/* Clipping region borders */
SHORT   _clipLeft;
SHORT   _clipTop;
SHORT   _clipRight;
SHORT   _clipBottom;

/*Functions*/
int _VirtToPhys(const void* p);
//void PutPixel(short x, short y);
//void ResetDevice(void);

/*This stabilizes the pixel refresh when drawing pixels*/
#ifdef LEGACY_MODE
#define PIXEL_DRAW_PER_DMA_TX          10  
#else
#define PIXEL_DRAW_PER_DMA_TX          20 
#endif

/*Macros for timing signals*/
#define DATA_ENABLE      LATDbits.LATD2
#define DATA_ENABLE_TRIS TRISDbits.TRISD2
#define HSYNC            LATBbits.LATB8
#define HSYNC_TRIS       TRISBbits.TRISB8
#define VSYNC            LATCbits.LATC3 
#define VSYNC_TRIS       TRISCbits.TRISC3

/*Macros for LCD IO*/ 
#define BACKLIGHT      LATDbits.LATD3
#define BACKLIGHT_TRIS TRISDbits.TRISD3
#define LCD_RESET      LATCbits.LATC1
#define LCD_RESET_TRIS TRISCbits.TRISC1
#define LCD_CS         LATCbits.LATC2
#define LCD_CS_TRIS    TRISCbits.TRISC2
#define LCD_DC         LATBbits.LATB3
#define LCD_DC_TRIS    TRISBbits.TRISB3

/*Macros for External SRAM*/
#define SRAM_CS       LATFbits.LATF13      
#define SRAM_TRIS     TRISFbits.TRISF13
#define ADDR15        LATAbits.LATA15
#define ADDR15_TRIS   TRISAbits.TRISA15
#define ADDR16        LATDbits.LATD8
#define ADDR16_TRIS   TRISDbits.TRISD8
#define ADDR17        LATEbits.LATE9
#define ADDR17_TRIS   TRISEbits.TRISE9
#define ADDR18        LATFbits.LATF12
#define ADDR18_TRIS   TRISFbits.TRISF12

#define PIXELCLOCK        LATDbits.LATD5
#define PIXELCLOCK_TRIS   TRISDbits.TRISD5

/*This defines the polarity of the pixel clock as defined in LCD specs*/
#ifdef DISP_INV_LSHIFT
#define PCLK_POLARITY PMP_READ_POL_LO
#else
#define PCLK_POLARITY PMP_READ_POL_HI
#endif

/*These define the size (in resolution) of the LCD being used*/
#define LINE_LENGTH              DISP_HOR_RESOLUTION
#define FRAME_HEIGHT             DISP_VER_RESOLUTION

#ifdef LCC_INTERNAL_MEMORY
#ifdef USE_PALETTE
BYTE HBackPorch = 2*((DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH)-1);     
#else
BYTE HBackPorch = (DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH)-1;     
#endif
BYTE HFrontPorch = 1;     
#else

WORD HBackPorch = 2*((DISP_HOR_PULSE_WIDTH+DISP_HOR_BACK_PORCH)-1);    
BYTE HFrontPorch = 2;     
#endif

#define VER_BLANK                 (DISP_VER_PULSE_WIDTH+DISP_VER_BACK_PORCH+DISP_VER_FRONT_PORCH-1)    

#define  PMADDR_OVERFLOW               32768          /* Set for 2^15 because that is how many address lines are connected from the PIC32*/
volatile BYTE DrawCount =0;                          /* The current status of how many pixels have been drawn inbetween a DMA IR*/
volatile BYTE overflowcount =0;                      /* The count for the amount of overflows that have happened in the PMP Adress*/

#ifdef LCC_INTERNAL_MEMORY
char GraphicsFrame[FRAME_HEIGHT][LINE_LENGTH];                   //8 BPP QVGA graphics frame
#else 
char GraphicsFrame[LINE_LENGTH<<1];
#endif 

#ifdef LCC_INTERNAL_MEMORY
     #define PMP_ADDRESS_LINES PMP_PEN_OFF 

     #ifdef USE_PALETTE
         #define PMP_DATA_LENGTH PMP_DATA_BUS_16
     #else
         #define PMP_DATA_LENGTH PMP_DATA_BUS_8
#endif
 
#else

     #define PMP_ADDRESS_LINES PMP_PEN_ALL
     #define PMP_DATA_LENGTH PMP_DATA_BUS_16
#endif

#define PMP_CONTROL	(PMP_ON | PMP_MUX_OFF | PMP_READ_WRITE_EN|\
             		PMP_CS2_EN | PMP_CS2_POL_LO | PMP_WRITE_POL_LO | PCLK_POLARITY)

#define PMP_MODE     (PMP_DATA_LENGTH | PMP_MODE_MASTER2 |\
                     PMP_WAIT_BEG_1 | PMP_WAIT_MID_1 | PMP_WAIT_END_1 )


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

    DMACONbits.SUSPEND =1;    //Suspend ALL DMA transfers

    BMXCONbits.BMXARB = 0x02; //Faster Refresh Rate
    BMXCONbits.BMXCHEDMA = 1;

    LCD_DC_TRIS =0;
    HSYNC_TRIS =0;
    LCD_CS_TRIS =0;
    VSYNC_TRIS =0;
    LCD_RESET_TRIS =0;

    BACKLIGHT_TRIS=0;
    DATA_ENABLE_TRIS=0;

    SRAM_TRIS =0;

    ADDR15_TRIS=0;
    ADDR16_TRIS=0;
    ADDR17_TRIS =0;
    ADDR18_TRIS =0;

    LCD_RESET =1;  
    LCD_CS    =1;      
    LCD_DC    =1;       

    SRAM_CS   =0;
    ADDR17    =0;     
    ADDR18    =0;

    PIXELCLOCK_TRIS =0;

#ifdef DISP_INV_LSHIFT
PIXELCLOCK =1;
#else
PIXELCLOCK =0;
#endif

    #if defined(USE_TCON_MODULE)
    GfxTconInit();
    #endif

    // setup the PMP
    mPMPOpen(PMP_CONTROL, PMP_MODE, PMP_ADDRESS_LINES, PMP_INT_ON);
    PMADDR = 0x0000;

	// Open the desired DMA channel.
	DmaChnOpen(1, 0, DMA_OPEN_DEFAULT);

    // set the transfer event control: what event is to start the DMA transfer
     DmaChnSetEventControl(1, DMA_EV_START_IRQ(_TIMER_2_IRQ)); 

   	// set the transfer parameters: source & destination address, source & destination size, number of bytes per event
    #ifdef LCC_INTERNAL_MEMORY
    BACKLIGHT =0;     //Turn Backlight on

    #ifdef USE_PALETTE
    DmaChnSetTxfer(1, &GraphicsFrame[0], (void*)&PMDIN, HBackPorch, 2, 2); 
    #else
    DmaChnSetTxfer(1, &GraphicsFrame[0], (void*)&PMDIN, HBackPorch, 1, 2); 
    #endif

    #else    
    #if defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
    BACKLIGHT =0;     //Turn Backlight on
    DmaChnSetTxfer(1, (void*)&PMDIN ,&GraphicsFrame[0] , 2, HBackPorch, 2);  
    #else
    BACKLIGHT =1;
    DmaChnSetTxfer(1, (void*)&PMDIN ,&GraphicsFrame[0] , 2, HBackPorch, 16);  
    #endif 
    #endif

	INTSetVectorPriority(INT_VECTOR_DMA(1), INT_PRIORITY_LEVEL_7);		        // set INT controller priority
	INTSetVectorSubPriority(INT_VECTOR_DMA(1), INT_SUB_PRIORITY_LEVEL_3);		// set INT controller sub-priority

    DmaChnSetEvEnableFlags(1, DMA_EV_BLOCK_DONE);	// enable the transfer done interrupt, when all buffer transferred
    INTEnable(INT_SOURCE_DMA(1), INT_ENABLED);		// enable the chn interrupt in the INT controller

    DCH1CONbits.CHPRI = 0b11;  //DMA channel has highest priority  

    // once we configured the DMA channel we can enable it
	DmaChnEnable(1);
    
   #ifdef LCC_INTERNAL_MEMORY
   #ifdef USE_PALETTE 
   OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 4); //Start Timer
   #else
   OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 27); //Start Timer
   #endif
   #else  //External Memory
   OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 10); //Start Timer
   #endif

   DMACONbits.SUSPEND = 0;

}

#ifdef LCC_EXTERNAL_MEMORY

//#ifdef LEGACY_MODE

//#endif

// handler for the DMA channel 1 interrupt
void __ISR(_DMA1_VECTOR, ipl7) DmaHandler1(void)
{
  static WORD remaining=0;
  static short line =0;
  static BYTE GraphicsState=1;
  static WORD dmatransfersremaining=0;
 
#ifdef LEGACY_MODE
        if(dmatransfersremaining != 0)
         {

            if(dmatransfersremaining > MAX_DMA_TRANSFER)
             {
                   //Setup DMA Transfer
                   DCH1DSIZ =  MAX_DMA_TRANSFER;
                   dmatransfersremaining -= MAX_DMA_TRANSFER;
             }
             else
             {
                   //Setup DMA Transfer
                   DCH1DSIZ =  dmatransfersremaining;
                   dmatransfersremaining -= DCH1DSIZ;
             }

         DrawCount=0;
         DCH1INTCLR = 0x08;  //CHBCIF = 0;
         IFS1CLR = 0x20000;  //DMA1IF =0;
         DCH1CONSET =0x80;   //CHEN =1; 
         return;
        }   
#endif

       switch(GraphicsState)
       {
         case 1:
 
            if(line++ >= -DISP_VER_FRONT_PORCH)
            {                   
           
              if(line == (FRAME_HEIGHT)) 
              {         
                VSYNC =0;  
                line=  -VER_BLANK; 
                PMADDR = 0;
                ADDR15=0; ADDR16=0;
                #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                ADDR17=0; ADDR18=0;
                #endif
                overflowcount=0;  
               }

             else
             { 
               
               PMMODESET = 0x0800;   //INCM = 0b01;
               VSYNC =1; 
               DATA_ENABLE =1;

               if(((PMADDR_OVERFLOW - PMADDR) < (LINE_LENGTH)) 
                    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E 
                           || ((PMADDR ==0) && (overflowcount>0))) 
                    #else 
                           ) 
                    #endif       
               {   

               GraphicsState = 3;      //Do Overflow routine
               remaining = ((PMADDR_OVERFLOW)-PMADDR);
 
                    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        if(PMADDR==0)
                        {
                            remaining = LINE_LENGTH;
                            ADDR15 = ++overflowcount;          //count holds the additional address line count
                            ADDR16 = overflowcount>>1; 
                            ADDR17 = overflowcount>>2;
                            ADDR18 = overflowcount>>3;
                            GraphicsState=2;
                        }
                    #endif
 
               //Setup DMA Transfer
             #ifdef LEGACY_MODE
               if((remaining<<1) > MAX_DMA_TRANSFER)
               {        
               DCH1DSIZ =  MAX_DMA_TRANSFER;
               dmatransfersremaining = (remaining <<1)- MAX_DMA_TRANSFER; 
               }
                else
               {
               DCH1DSIZ =  (WORD)(remaining<<1);
               } 
              #else
               DCH1DSIZ =  (WORD)(remaining<<1);
              #endif
               
               break;
               } 
            
              }
             }

            //Setup DMA Transfer
            #ifdef LEGACY_MODE
            DCH1DSIZ =  MAX_DMA_TRANSFER;
            dmatransfersremaining = (LINE_LENGTH<<1) - MAX_DMA_TRANSFER; 
            #else
            DCH1DSIZ =  (WORD)(LINE_LENGTH<<1);
            #endif

            GraphicsState++;
            break;

       case 3:                    //Adjust the address lines that aren't part of PMP
             remaining = (LINE_LENGTH-remaining);
 
             ADDR15 = ++overflowcount;          //count holds the additional address line count
             ADDR16 = overflowcount>>1; 
             #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
             ADDR17 = overflowcount>>2;
             ADDR18 = overflowcount >>3;
             #endif
             

               //Setup DMA Transfer
              #ifdef LEGACY_MODE
               if((remaining<<1) > MAX_DMA_TRANSFER)
               {        
               DCH1DSIZ =  MAX_DMA_TRANSFER;
               dmatransfersremaining = (remaining <<1)- MAX_DMA_TRANSFER; 
               }
                else
               {
               DCH1DSIZ =  (WORD)(remaining<<1);
               } 
              #else
               DCH1DSIZ =  (WORD)(remaining<<1);
              #endif

             GraphicsState=2; //goto Front Porch
             break;
 
  case 2:   //Front Porch then Back Porch Start
           HSYNC =0; 
           DATA_ENABLE =0; 
           PMMODECLR = 0x0800; //  INCM = 0b00; No Address Incrementing here

           GraphicsState= PMDIN;
    
            HSYNC = 1;
            //Setup DMA Back Porch
            DCH1DSIZ =  HBackPorch;

            GraphicsState=1;         
           break;

   default: 
           break;
}

    DrawCount=0;
    DCH1INTCLR = 0x08;  //CHBCIF = 0;
    IFS1CLR = 0x20000;  //DMA1IF =0;
    DCH1CONSET =0x80;   //CHEN =1; 

}
#endif
#ifdef LCC_INTERNAL_MEMORY

void __ISR(_DMA1_VECTOR, ipl7) DmaHandler1(void)
{
  static BYTE GraphicsState = 1;
  static short line =0;

#ifdef USE_PALETTE
  static WORD ColorLUTLine[DISP_HOR_RESOLUTION];
         WORD *ColorLUTAddr = &ColorLUTLine[0];;
  static BYTE *LUTAddress= &GraphicsFrame[0][0];
#endif

    if(GraphicsState ==1)
    {
            #ifdef USE_PALETTE
            DCH1SSIZ =  LINE_LENGTH<<1;
            #else
            DCH1SSIZ =  LINE_LENGTH;
            #endif

           GraphicsState++;

           if(line++ >= -1)       
           { 
            VSYNC =1; 
            DATA_ENABLE =1;
           
            #ifdef USE_PALETTE
            DCH1SSA = _VirtToPhys(&ColorLUTLine[0]);
            #else
            DCH1SSA = _VirtToPhys(&GraphicsFrame[line][0]);
            #endif

            if(line == (FRAME_HEIGHT)) 
            {
                 VSYNC =0;
                 line= 0 - VER_BLANK;
                #ifdef USE_PALETTE
                 LUTAddress = &GraphicsFrame[0][0];
                 line--;
                #endif
            }

           #ifdef USE_PALETTE
           BYTE i=0;
           DCH1CONSET =0x80;  //CHEN =1; 
           DCH1INTCLR = 0x08; //CHBCIF = 0;
           IFS1CLR = 0x20000; //DMA1IF =0;

          /*Do color LUT Here. Each line is 240 bytes*/
           while(i++<10)  
           {  
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];
            *ColorLUTAddr++ = LUT[*LUTAddress++];       
           }
           return;
         #endif

           }        
    }
    else
    {     
           HSYNC =0; 
           DATA_ENABLE =0;

           //Perform Back Porch Clock Signal
           PMDINSET=1;
           
           HSYNC = 1;
           DCH1SSIZ =  HBackPorch;
           GraphicsState= 1;
    }
 
    DCH1INTCLR = 0x08; //CHBCIF = 0;
    IFS1CLR = 0x20000; //DMA1IF =0;
    DCH1CONSET =0x80;  //CHEN =1;
}


#endif

void PutPixel(short x, short y)
{
 
    if(_clipRgn)
    {
        if(x < _clipLeft)
            return;
        if(x > _clipRight)
            return;
        if(y < _clipTop)
            return;
        if(y > _clipBottom)
            return;
    }

#ifdef LCC_INTERNAL_MEMORY
GraphicsFrame[(GetMaxX()-x)][y] = _color;
#else

static BYTE pixelcount =0;
static GFX_COLOR color1;
static DWORD address1; 
static BYTE address1bit0, address1bit1;
static DWORD prevaddr;
static DWORD address; 
static BYTE addressbit0, addressbit1;

#ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
static BYTE address1bit2,address1bit3,addressbit2,addressbit3;
#endif 

if(pixelcount++ == 0)
{
    #if defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
    address1 = (DWORD)(76800-(x*240)+(y));  
    #else //(DISPLAY_PANEL == PH480272T_005_I11Q)
    address1 = (DWORD)(((y)*(DISP_HOR_RESOLUTION))+(x));
    #endif
    address1bit0 = address1 >> 15;
    address1bit1 = address1 >> 16;
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    address1bit2 = address1 >> 17;
    address1bit3 = address1 >> 18;
    #endif
    color1 =  _color;
    return;
}
else
{

   pixelcount =0;   //Toggle between saving the pixel information and sending two pixels to the SRAM 

    #if defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
    address = (DWORD)(76800-(x*240)+(y));  
    #else //(DISPLAY_PANEL == PH480272T_005_I11Q)
    address = (DWORD)(((y)*DISP_HOR_RESOLUTION+(x)));
    #endif
    addressbit0 = address>>15;
    addressbit1 = address>>16;
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    addressbit2 = address >> 17;
    addressbit3 = address >> 18;
    #endif

    DrawCount++;

    while(DrawCount>PIXEL_DRAW_PER_DMA_TX){}   //Added in WQVGA Driver to stabilize refresh rate

//Suspend DMA
    DMACONSET = 0x1000;
    while(PMMODEbits.BUSY ==1);

//Perform Write
    ADDR15 = addressbit0;
    ADDR16 = addressbit1;
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    ADDR17 = addressbit2;
    ADDR18 = addressbit3;
    #endif

//Save previous address value
    prevaddr = PMADDR;     
    PMADDR = address;             
    PMDIN = _color;   

//Setup Write 2
    ADDR15 = address1bit0;
    ADDR16 = address1bit1;
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    ADDR17 = address1bit2;
    ADDR18 = address1bit3;
    #endif 
  
    PMADDR = address1;             
    PMDIN  = (WORD)color1;

//Clean-up Address Lines
    ADDR15 = overflowcount;          //count holds the additional address line count
    ADDR16 = overflowcount>>1; 
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    ADDR17 = overflowcount>>2;
    ADDR18 = overflowcount>>3;
    #endif
    PMADDR = prevaddr; 

//ReStart DMA
    DMACONCLR = 0x1000;

}
#endif
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

/*********************************************************************
 * Function:        unsigned int _VirtToPhys(const void* p)
 * PreCondition:    None
 * Input:			p	- pointer to be converted.
 * Output:          a physical address corresponding to the virtual input pointer
 * Side Effects:    None
 * Overview:		Virtual to physical translation helper.
 * Note:            None.
 ********************************************************************/
int _VirtToPhys(const void* p)
{
	return (int)p<0?((int)p&0x1fffffffL):(unsigned int)((unsigned char*)p+0x40000000L);
}

#ifdef USE_PALETTE
void StartVBlankInterrupt(void){}
void  EnablePalette(void){}
BYTE SetPaletteBpp(BYTE bpp){}
BYTE SetPaletteFlash(PALETTE_ENTRY *pPaletteEntry, WORD startEntry, WORD length)
{
    WORD counter;

    if((pPaletteEntry == NULL) || ((startEntry + length) > 256))
    {
        return -1;
    }

    for(counter = 0; counter < length; counter++)
    {
        
        LUT[counter]  = pPaletteEntry[counter].value;

    }

    return 0;
}
#endif


GFX_COLOR GetPixel(short x, short y)
{

#ifdef LCC_INTERNAL_MEMORY
return GraphicsFrame[(GetMaxX()-x)][y];
#else
static DWORD prevaddr;
static DWORD address; 
static BYTE addressbit0, addressbit1;
static GFX_COLOR getcolor;
#ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
static BYTE addressbit2,addressbit3;
#endif 

    #if defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
    address = (DWORD)(76800-(x*240)+(y));  
    #else //(DISPLAY_PANEL == PH480272T_005_I11Q)
    address = (DWORD)(((y)*DISP_HOR_RESOLUTION+(x)));
    #endif
    addressbit0 = address>>15;
    addressbit1 = address>>16;
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    addressbit2 = address >> 17;
    addressbit3 = address >> 18;
    #endif

    DrawCount++;

    while(DrawCount>PIXEL_DRAW_PER_DMA_TX){}   //Added in WQVGA Driver to stabilize refresh rate

//Suspend DMA
    DMACONSET = 0x1000;

    while(PMMODEbits.BUSY ==1);

   PMCONbits.PTRDEN=0;

//Perform Write
    ADDR15 = addressbit0;
    ADDR16 = addressbit1;
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    ADDR17 = addressbit2;
    ADDR18 = addressbit3;
    #endif

//Save previous address value
    prevaddr = PMADDR;     

    PMADDR =   address;
           
    getcolor = PMDIN; 
    getcolor = PMDIN;

    while(PMMODEbits.BUSY ==1);

  PMCONbits.PTRDEN=1;

//Clean-up Address Lines
    ADDR15 = overflowcount;          //count holds the additional address line count
    ADDR16 = overflowcount>>1; 
    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
    ADDR17 = overflowcount>>2;
    ADDR18 = overflowcount>>3;
    #endif

    PMADDR = prevaddr; 

//ReStart DMA
    DMACONCLR = 0x1000;

return getcolor;
#endif
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
    SHORT   x, y;
    static DWORD address; 
    static WORD  overflowamount=0;
    static int addresschange;
       #if !defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
       addresschange = 1;
       #else
       addresschange = -240;
       #endif

   if(_clipRgn)
   {
      if(left < _clipLeft)
      left = _clipLeft;
      if(right > _clipRight)
      right = _clipRight;
      if(top < _clipTop)
      top = _clipTop;
      if(bottom > _clipBottom)
      bottom = _clipBottom;
    }

#ifdef LCC_INTERNAL_MEMORY

for(y = top; y <= bottom; y++)
   for(x = left; x <= right; x++)
        GraphicsFrame[(GetMaxX()-x)][y] = _color;
#else

    for(y = top; y <= bottom; y++)
      {  
       #if !defined(GFX_USE_DISPLAY_PANEL_TFT_G240320LTSW_118W_E)
       address = (DWORD)(((y)*(DISP_HOR_RESOLUTION))+(left));
       #else
       address = (DWORD)(76800-(left*240)+(y));
       #endif
  
       for(x = left; x <= right; x+= 8)
            {
                static DWORD prevaddr;

                    DrawCount++;
                    while(DrawCount>PIXEL_DRAW_PER_DMA_TX){}   //Added in WQVGA Driver to stabilize refresh rate

                    if(((right-x) <= 8))  //Draw less than 8 at a time if x<8
                    {
                    //Suspend DMA
                    DMACONSET = 0x1000;
                    while(PMMODEbits.BUSY ==1);
                    //Perform Write
                    ADDR15 = address>>15;
                    ADDR16 = address>>16;
                    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                    ADDR17 = address>>17;
                    ADDR18 = address>>18;
                    #endif

                    //Save previous address value
                    prevaddr = PMADDR;     
                    PMADDR = address;             
                    PMDIN = _color;   
                    x-=7;
                    }
                    else    //Draw 8 pixels at a time
                    {
                    //Suspend DMA
                    DMACONSET = 0x1000;
                    while(PMMODEbits.BUSY ==1);
                    //Setup Write 1
                     overflowamount = (address/PMADDR_OVERFLOW);
                     ADDR15 = overflowamount;
                     ADDR16 = overflowamount>>1;
                     #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                     ADDR17 = overflowamount>>2;
                     ADDR18 = overflowamount>>3;
                     #endif

                    //Save previous address value
                    prevaddr = PMADDR;     
                    PMADDR = address; 
                    address  +=  addresschange;            
                    PMDIN = _color;   

                    //Setup Write 2
                    if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address;
                    address  +=  addresschange;             
                    PMDINSET = 0;            //Draw pixel 2

                    //Setup Write 3
                     if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address;  
                    address  +=  addresschange;           
                    PMDINSET = 0;            //Draw pixel 3

                    //Setup Write 4
                    if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address;   
                    address  +=  addresschange;          
                    PMDINSET = 0;            //Draw pixel 4

                    //Setup Write 5
                    if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address;
                    address  +=  addresschange;            
                    PMDINSET = 0;            //Draw pixel 5

                    //Setup Write 6
                    if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address; 
                    address  +=  addresschange;            
                    PMDINSET = 0;            //Draw pixel 6

                    //Setup Write 7
                    if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address; 
                    address  +=  addresschange;            
                    PMDINSET = 0;            //Draw pixel 7

                    //Setup Write 8
                    if(overflowamount != (address/PMADDR_OVERFLOW))
                    {
                        overflowamount = (address/PMADDR_OVERFLOW);
                        ADDR15 = overflowamount;
                        ADDR16 = overflowamount>>1;
                        #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                        ADDR17 = overflowamount>>2;
                        ADDR18 = overflowamount>>3;
                        #endif
                    }

                    PMADDR = address;           
                    PMDINSET = 0;            //Draw pixel 8
                    }

                    //Clean-up Address Lines
                    ADDR15 = overflowcount;          //count holds the additional address line count
                    ADDR16 = overflowcount>>1; 
                    #ifdef GFX_USE_DISPLAY_PANEL_TFT_640480_8_E
                    ADDR17 = overflowcount>>2;
                    ADDR18 = overflowcount>>3;
                    #endif
                    PMADDR = prevaddr; 

                    //ReStart DMA
                    DMACONCLR = 0x1000;
                    address  +=  addresschange;
             }

      }
#endif
    return (1);
}

#if defined USE_BITMAP_FLASH
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
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    BYTE                temp = 0;
    WORD                sizeX, sizeY;
    WORD                x, y;
    WORD                xc, yc;
    BYTE                stretchX, stretchY;
#if (COLOR_DEPTH != 24)	
    WORD                pallete[2];
#elif (COLOR_DEPTH == 24)	
    DWORD                pallete[2];
#endif
    BYTE                mask;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    
    #if (COLOR_DEPTH == 24)
	((FLASH_WORD *)flashAddress)++;
	#endif

#if (COLOR_DEPTH != 24)	
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

    yc = top;
    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            mask = 0;
            xc = left;
            for(x = 0; x < sizeX; x++)
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
                // Set color
                #ifdef USE_PALETTE
                    SetColor(1);
                #else
                     #ifdef LCC_INTERNAL_MEMORY
                    SetColor(RGBConvert((((pallete[1]&0xF800)>>8)),(((pallete[1]&0x07FF)>>3)),(((pallete[1]&0x001F)<<3))));
                    #else
                    SetColor(pallete[1]);
                    #endif
                #endif                
                }
                else
                {
                // Set color
                #ifdef USE_PALETTE
                    SetColor(0);
                #else
                   
                    #ifdef LCC_INTERNAL_MEMORY
                    SetColor(RGBConvert((((pallete[0]&0xF800)>>8)),(((pallete[0]&0x07FF)>>3)),(((pallete[0]&0x001F)<<3))));
                    #else
                    SetColor(pallete[0]);
                    #endif
                #endif
                }

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
                    PutPixel(xc++, yc);
                }

                // Shift to the next pixel
                mask >>= 1;
            }

            yc++;
        }
    }
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
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    WORD                sizeX, sizeY;
    register WORD       x, y;
    WORD                xc, yc;
    BYTE                temp = 0;
    register BYTE       stretchX, stretchY;
#if (COLOR_DEPTH != 24)	
    WORD                pallete[16];
#elif (COLOR_DEPTH == 24)	
	DWORD               pallete[16];
#endif
    WORD                counter;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

	#if (COLOR_DEPTH == 24)
	((FLASH_WORD *)flashAddress)++;
	#endif
	
    // Read pallete
    for(counter = 0; counter < 16; counter++)
    {
	#if (COLOR_DEPTH != 24)	
        pallete[counter] = *((FLASH_WORD *)flashAddress);
        flashAddress += 2;
    #elif (COLOR_DEPTH == 24)	
		pallete[counter] = *((FLASH_DWORD *)flashAddress);
		flashAddress += 4;
	#endif
    }

    yc = top;
    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            xc = left;
            for(x = 0; x < sizeX; x++)
            {

                // Read 2 pixels from flash
                if(x & 0x0001)
                {
                    // second pixel in byte
                // Set color
                #ifdef USE_PALETTE
                    SetColor(temp >> 4);
                #else
                  
                    #ifdef LCC_INTERNAL_MEMORY
                    SetColor(RGBConvert((((pallete[temp >> 4]&0xF800)>>8)),(((pallete[temp >> 4]&0x07FF)>>3)),(((pallete[temp >> 4]&0x001F)<<3)))); 
                    #else
                    SetColor(pallete[temp >> 4]);
                    #endif
                #endif
                }
                else
                {
                    temp = *flashAddress;
                    flashAddress++;

                    // first pixel in byte
                // Set color
                #ifdef USE_PALETTE
                    SetColor(temp & 0x0f);
                #else
                
                #ifdef LCC_INTERNAL_MEMORY
                SetColor(RGBConvert((((pallete[temp & 0x0f]&0xF800)>>8)),(((pallete[temp & 0x0f]&0x07E0)>>3)),(((pallete[temp & 0x0f]&0x001F)<<3))));
                #else
                SetColor(pallete[temp & 0x0f]);
                #endif
                #endif
                }

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
                    PutPixel(xc++, yc);
                }
            }

            yc++;
        }
    }
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

/* */
void PutImage8BPP(SHORT left, SHORT top, FLASH_BYTE *bitmap, BYTE stretch)
{
    register FLASH_BYTE *flashAddress;
    register FLASH_BYTE *tempFlashAddress;
    WORD                sizeX, sizeY;
    WORD                x, y;
    WORD                xc, yc;
    BYTE                temp;
    BYTE                stretchX, stretchY;
#if (COLOR_DEPTH != 24)	
    WORD                pallete[256];
#elif (COLOR_DEPTH == 24)	
    DWORD               pallete[256];
#endif
    WORD                counter;

    // Move pointer to size information
    flashAddress = bitmap + 2;

    // Read image size
    sizeY = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;
    sizeX = *((FLASH_WORD *)flashAddress);
    flashAddress += 2;

	#if (COLOR_DEPTH == 24)
	((FLASH_WORD *)flashAddress)++;
	#endif
	
    // Read pallete
    for(counter = 0; counter < 256; counter++)
    {
	#if (COLOR_DEPTH != 24)	
        pallete[counter] = *((FLASH_WORD *)flashAddress);
        flashAddress += 2;       
    #elif (COLOR_DEPTH == 24)	
		pallete[counter] = *((FLASH_DWORD *)flashAddress);
		flashAddress += 4;
	#endif
    }
    
    yc = top;
    for(y = 0; y < sizeY; y++)
    {
        tempFlashAddress = flashAddress;
        for(stretchY = 0; stretchY < stretch; stretchY++)
        {
            flashAddress = tempFlashAddress;
            xc = left;
            for(x = 0; x < sizeX; x++)
            {

                // Read pixels from flash
                temp = *flashAddress;
                flashAddress++;

                // Set color
            #ifdef USE_PALETTE
                SetColor(temp);
            #else
               #ifdef LCC_INTERNAL_MEMORY
                SetColor(RGBConvert((((pallete[temp]&0xF800)>>8)),(((pallete[temp]&0x07E0)>>3)),(((pallete[temp]&0x001F)<<3)))); 
               #else
                SetColor(pallete[temp]);
               #endif
            #endif

                // Write pixel to screen
                for(stretchX = 0; stretchX < stretch; stretchX++)
                {
                    PutPixel(xc++, yc);
                }
            }

            yc++;
        }
    }
}
#endif


#endif    //if defined (GFX_USE_DISPLAY_CONTROLLER_DMA)


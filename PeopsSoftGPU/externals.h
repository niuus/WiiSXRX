/***************************************************************************
                        externals.h -  description
                             -------------------
    begin                : Sun Oct 28 2001
    copyright            : (C) 2001 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//
// History of changes:
//
// 2005/04/15 - Pete
// - Changed user frame limit to floating point value
//
// 2004/01/31 - Pete
// - added zn stuff
//
// 2002/04/20 - linuzappz
// - added iFastFwd var
//
// 2001/12/22 - syo
// - added vsync & transparent vars
//
// 2001/12/16 - Pete
// - added iFPSEInterface variable
//
// 2001/12/05 - syo
// - added iSysMemory and iStopSaver
//
// 2001/10/28 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

/////////////////////////////////////////////////////////////////////////////

#define INFO_TW        0
#define INFO_DRAWSTART 1
#define INFO_DRAWEND   2
#define INFO_DRAWOFF   3

#define SHADETEXBIT(x) ((x>>24) & 0x1)
#define SEMITRANSBIT(x) ((x>>25) & 0x1)
#define PSXRGB(r,g,b) ((g<<10)|(b<<5)|r)

#define DATAREGISTERMODES unsigned short

#define DR_NORMAL        0
#define DR_VRAMTRANSFER  1


#define GPUSTATUS_ODDLINES            0x80000000
#define GPUSTATUS_DMABITS             0x60000000 // Two bits
#define GPUSTATUS_READYFORCOMMANDS    0x10000000
#define GPUSTATUS_READYFORVRAM        0x08000000
#define GPUSTATUS_IDLE                0x04000000
#define GPUSTATUS_DISPLAYDISABLED     0x00800000
#define GPUSTATUS_INTERLACED          0x00400000
#define GPUSTATUS_RGB24               0x00200000
#define GPUSTATUS_PAL                 0x00100000
#define GPUSTATUS_DOUBLEHEIGHT        0x00080000
#define GPUSTATUS_WIDTHBITS           0x00070000 // Three bits
#define GPUSTATUS_MASKENABLED         0x00001000
#define GPUSTATUS_MASKDRAWN           0x00000800
#define GPUSTATUS_DRAWINGALLOWED      0x00000400
#define GPUSTATUS_DITHER              0x00000200

#define GPUIsBusy (lGPUstatusRet &= ~GPUSTATUS_IDLE)
#define GPUIsIdle (lGPUstatusRet |= GPUSTATUS_IDLE)

#define GPUIsNotReadyForCommands (lGPUstatusRet &= ~GPUSTATUS_READYFORCOMMANDS)
#define GPUIsReadyForCommands (lGPUstatusRet |= GPUSTATUS_READYFORCOMMANDS)

/////////////////////////////////////////////////////////////////////////////

typedef struct VRAMLOADTTAG
{
 short x;
 short y;
 short Width;
 short Height;
 short RowsRemaining;
 short ColsRemaining;
 unsigned short *ImagePtr;
} VRAMLoad_t;

/////////////////////////////////////////////////////////////////////////////

typedef struct PSXPOINTTAG
{
 long x;
 long y;
} PSXPoint_t;

typedef struct PSXSPOINTTAG
{
 short x;
 short y;
} PSXSPoint_t;

typedef struct PSXRECTTAG
{
 short x0;
 short x1;
 short y0;
 short y1;
} PSXRect_t;


// linux defines for some windows stuff

#define FALSE 0
#define TRUE 1
#define BOOL unsigned short
#define LOWORD(l)           ((unsigned short)(l))
#define HIWORD(l)           ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define DWORD unsigned long
#define __int64 long long int

typedef struct RECTTAG
{
 int left;
 int top;
 int right;
 int bottom;
}RECT;

/////////////////////////////////////////////////////////////////////////////

typedef struct TWINTAG
{
 PSXRect_t  Position;
} TWin_t;

/////////////////////////////////////////////////////////////////////////////

typedef struct PSXDISPLAYTAG
{
 PSXPoint_t  DisplayModeNew;
 PSXPoint_t  DisplayMode;
 PSXPoint_t  DisplayPosition;
 PSXPoint_t  DisplayEnd;

 long        Double;
 long        Height;
 long        PAL;
 long        InterlacedNew;
 long        Interlaced;
 long        RGB24New;
 long        RGB24;
 PSXSPoint_t DrawOffset;
 long        Disabled;
 PSXRect_t   Range;

} PSXDisplay_t;

/////////////////////////////////////////////////////////////////////////////

// draw.c

#ifndef _IN_DRAW


extern char *         pCaptionText;

extern int            iResX;
extern int            iResY;
extern long           GlobalTextAddrX,GlobalTextAddrY,GlobalTextTP;
extern long           GlobalTextREST,GlobalTextABR,GlobalTextPAGE;
extern short          ly0,lx0,ly1,lx1,ly2,lx2,ly3,lx3;
extern long           lLowerpart;
extern BOOL           bIsFirstFrame;
extern int            iWinSize;
extern BOOL           bCheckMask;
extern unsigned short sSetMask;
extern unsigned long  lSetMask;
extern BOOL           bDeviceOK;
extern short          g_m1;
extern short          g_m2;
extern short          g_m3;
extern short          DrawSemiTrans;
extern int            iUseGammaVal;
extern int            iUseScanLines;
extern int            iDesktopCol;
extern int            iUseNoStretchBlt;
extern int            iShowFPS;
extern int            iFastFwd;
extern int            iDebugMode;
extern int            iFVDisplay;
extern PSXPoint_t     ptCursorPoint[];
extern unsigned short usCursorActive;

#endif

// prim.c

#ifndef _IN_PRIMDRAW

extern BOOL           bUsingTWin;
extern TWin_t         TWin;
extern unsigned long  clutid;
extern void (*primTableJ[256])(unsigned char *);
extern void (*primTableSkip[256])(unsigned char *);
extern unsigned short  usMirror;
extern int            iDither;
extern unsigned long  dwCfgFixes;
extern unsigned long  dwActFixes;
extern unsigned long  dwEmuFixes;
extern int            iUseFixes;
extern int            iUseDither;
extern BOOL           bDoVSyncUpdate;
extern long           drawX;
extern long           drawY;
extern long           drawW;
extern long           drawH;

#endif

// gpu.c

#ifndef _IN_GPU

extern VRAMLoad_t     VRAMWrite;
extern VRAMLoad_t     VRAMRead;
extern DATAREGISTERMODES DataWriteMode;
extern DATAREGISTERMODES DataReadMode;
extern int            iColDepth;
extern int            iWindowMode;
extern char           szDispBuf[];
extern char           szMenuBuf[];
extern char           szDebugText[];
extern short          sDispWidths[];
extern BOOL           bDebugText;
//extern unsigned int   iMaxDMACommandCounter;
//extern unsigned long  dwDMAChainStop;
extern PSXDisplay_t   PSXDisplay;
extern PSXDisplay_t   PreviousPSXDisplay;
extern BOOL           bSkipNextFrame;
extern long           lGPUstatusRet;
extern long           drawingLines;
extern unsigned char  * psxVSecure;
extern unsigned char  * psxVub;
extern signed char    * psxVsb;
extern unsigned short * psxVuw;
extern signed short   * psxVsw;
extern unsigned long  * psxVul;
extern signed long    * psxVsl;
extern unsigned short * psxVuw_eom;
extern BOOL           bChangeWinMode;
extern long           lSelectedSlot;
extern DWORD          dwLaceCnt;
extern unsigned long  lGPUInfoVals[];
extern unsigned long  ulStatusControl[];
extern int            iRumbleVal;
extern int            iRumbleTime;

#endif

// menu.c

#ifndef _IN_MENU

extern unsigned long dwCoreFlags;

#endif

// key.c

#ifndef _IN_KEY

extern unsigned long  ulKeybits;

#endif

// fps.c

#ifndef _IN_FPS

extern BOOL           bInitCap;
extern int            UseFrameLimit;
extern int            UseFrameSkip;
extern float          fFrameRate;
extern int            iFrameLimit;
extern float          fFrameRateHz;
extern float          fps_skip;
extern float          fps_cur;
extern BOOL           bSSSPSXLimit;

#endif

// key.c

#ifndef _IN_KEY

#endif

// cfg.c

#ifndef _IN_CFG

extern char * pConfigFile;

#endif

// zn.c

#ifndef _IN_ZN

#define dwGPUVersion 0
#define iGPUHeight 512
#define iGPUHeightMask 511
#define GlobalTextIL 0
#define iTileCheat 0

#endif



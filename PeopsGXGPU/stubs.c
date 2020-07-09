//TODO These are variables are "stubbed"
//TODO Delete these after implementation

/////////////////////////////////////////////////////////////////////////////

#include "externals.h"

// draw.c

#define _IN_DRAW

char *         pCaptionText;

int            iResX;
int            iResY;
long           GlobalTextAddrX,GlobalTextAddrY,GlobalTextTP;
long           GlobalTextREST,GlobalTextABR,GlobalTextPAGE;
short          ly0,lx0,ly1,lx1,ly2,lx2,ly3,lx3;
long           lLowerpart;
BOOL           bIsFirstFrame;
int            iWinSize;
BOOL           bCheckMask;
unsigned short sSetMask;
unsigned long  lSetMask;
BOOL           bDeviceOK;
short          g_m1;
short          g_m2;
short          g_m3;
short          DrawSemiTrans;
int            iUseGammaVal;
int            iUseScanLines;
int            iDesktopCol;
int            iUseNoStretchBlt;
int            iShowFPS;
int            iFastFwd;
int            iDebugMode;
int            iFVDisplay;
PSXPoint_t     ptCursorPoint[];
unsigned short usCursorActive;

// prim.c

#define _IN_PRIMDRAW

BOOL           bUsingTWin;
TWin_t         TWin;
unsigned long  clutid;
void (*primTableJ[256])(unsigned char *);
void (*primTableSkip[256])(unsigned char *);
unsigned short  usMirror;
int            iDither;
unsigned long  dwCfgFixes;
unsigned long  dwActFixes;
unsigned long  dwEmuFixes;
int            iUseFixes;
int            iUseDither;
BOOL           bDoVSyncUpdate;
long           drawX;
long           drawY;
long           drawW;
long           drawH;

// menu.c

#define _IN_MENU

unsigned long dwCoreFlags;

// key.c

#define _IN_KEY
//TODO Replace all instances of this with 0, key.c is useless
unsigned long ulKeybits=0;

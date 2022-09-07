/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#ifndef __PSXMEMORY_H__
#define __PSXMEMORY_H__

#include "psxcommon.h"

#if defined(HW_RVL) || defined(HW_DOL) || defined(BIG_ENDIAN)

//#define _SWAP16(b) ((((unsigned char*)&(b))[0]&0xff) | (((unsigned char*)&(b))[1]&0xff)<<8)
//#define _SWAP32(b) ((((unsigned char*)&(b))[0]&0xff) | ((((unsigned char*)&(b))[1]&0xff)<<8) | ((((unsigned char*)&(b))[2]&0xff)<<16) | (((unsigned char*)&(b))[3]<<24))

#define SWAP16(v) ((((v)&0xff00)>>8) | (((v)&0xff)<<8))
#define SWAP32(v) ((((v)&0xff000000ul)>>24) | (((v)&0xff0000ul)>>8) | (((v)&0xff00ul)<<8) | (((v)&0xfful)<<24))
#define SWAPu32(v) SWAP32((u32)(v))
//#define SWAPs32(v) SWAP32((s32)(v))
#define SWAPu16(v) SWAP16((u16)(v))
//#define SWAPs16(v) SWAP16((s16)(v))

// add xjsxjs197 start
#define psxHAddr(mem) (psxH + (((mem) << 16) >> 16))
// add xjsxjs197 end

#define SWAP16p(ptr) ({u16 __ret, *__ptr=(ptr); __asm__ ("lhbrx %0, 0, %1" : "=r" (__ret) : "r" (__ptr)); __ret;})
#define SWAP32p(ptr) ({u32 __ret, *__ptr=(ptr); __asm__ ("lwbrx %0, 0, %1" : "=r" (__ret) : "r" (__ptr)); __ret;})
#define SWAP32wp(ptr,val) ({u32 __val=(val), *__ptr=(ptr); __asm__ ("stwbrx %0, 0, %1" : : "r" (__val), "r" (__ptr) : "memory");})

#else

#define SWAP16(b) (b)
#define SWAP32(b) (b)

#define SWAPu16(b) (b)
#define SWAPu32(b) (b)

#endif

// upd xjsxjs197 start
extern s8 psxM[0x00220000] __attribute__((aligned(32)));
//#define psxMs8(mem)		psxM[(mem) & 0x1fffff]
//#define psxMs16(mem)	(SWAP16(*(s16*)&psxM[(mem) & 0x1fffff]))
//#define psxMs32(mem)	(SWAP32(*(s32*)&psxM[(mem) & 0x1fffff]))
#define psxMu8(mem)		(*(u8*)&psxM[(mem) & 0x1fffff])
//#define psxMu16(mem)	(SWAP16(*(u16*)&psxM[(mem) & 0x1fffff]))
//#define psxMu32(mem)	(SWAP32(*(u32*)&psxM[(mem) & 0x1fffff]))
#define psxMu32(mem)	(LOAD_SWAP32p(psxM + (((mem) << 11) >> 11)))

//#define psxMs8ref(mem)	psxM[((mem) << 11) >> 11]
//#define psxMs16ref(mem)	(*(s16*)&psxM[((mem) << 11) >> 11])
//#define psxMs32ref(mem)	(*(s32*)&psxM[((mem) << 11) >> 11])
//#define psxMu8ref(mem)	(*(u8*) &psxM[((mem) << 11) >> 11])
//#define psxMu16ref(mem)	(*(u16*)&psxM[((mem) << 11) >> 11])
//#define psxMu32ref(mem)	(*(u32*)&psxM[((mem) << 11) >> 11])
#define psxMu32ref(mem)	(*(u32*)(psxM + (((mem) << 11) >> 11)))

s8 *psxP;
//#define psxPs8(mem)	    psxP[(mem) & 0xffff]
//#define psxPs16(mem)	(SWAP16(*(s16*)&psxP[(mem) & 0xffff]))
//#define psxPs32(mem)	(SWAP32(*(s32*)&psxP[(mem) & 0xffff]))
//#define psxPu8(mem)		(*(u8*) &psxP[(mem) & 0xffff])
//#define psxPu16(mem)	(SWAP16(*(u16*)&psxP[(mem) & 0xffff]))
//#define psxPu32(mem)	(SWAP32(*(u32*)&psxP[(mem) & 0xffff]))

//#define psxPs8ref(mem)	psxP[((mem) << 16) >> 16]
//#define psxPs16ref(mem)	(*(s16*)&psxP[((mem) << 16) >> 16])
//#define psxPs32ref(mem)	(*(s32*)&psxP[((mem) << 16) >> 16])
//#define psxPu8ref(mem)	(*(u8*) &psxP[((mem) << 16) >> 16])
//#define psxPu16ref(mem)	(*(u16*)&psxP[((mem) << 16) >> 16])
//#define psxPu32ref(mem)	(*(u32*)&psxP[((mem) << 16) >> 16])

extern s8 psxR[0x00080000] __attribute__((aligned(32)));
#define psxRs8(mem)		psxR[(mem) & 0x7ffff]
//#define psxRs16(mem)	(SWAP16(*(s16*)&psxR[(mem) & 0x7ffff]))
#define psxRs16(mem)	(LOAD_SWAP16p(psxR + (((mem) << 13) >> 13)))
//#define psxRs32(mem)	(SWAP32(*(s32*)&psxR[(mem) & 0x7ffff]))

//#define psxRu8(mem)		(*(u8* )&psxR[(mem) & 0x7ffff])
//#define psxRu16(mem)	(SWAP16(*(u16*)&psxR[(mem) & 0x7ffff]))
#define psxRu16(mem)	(LOAD_SWAP16p(psxR + (((mem) << 13) >> 13)))
//#define psxRu32(mem)	(SWAP32(*(u32*)&psxR[(mem) & 0x7ffff]))
#define psxRu32(mem)	(LOAD_SWAP32p(psxR + (((mem) << 13) >> 13)))

//#define psxRs8ref(mem)	(psxR + (((mem) << 13) >> 13))
//#define psxRs16ref(mem)	(*(s16*)&psxR[((mem) << 13) >> 13])
//#define psxRs32ref(mem)	(*(s32*)&psxR[((mem) << 13) >> 13])
//#define psxRu8ref(mem)	(*(u8* )&psxR[((mem) << 13) >> 13])
//#define psxRu16ref(mem)	(*(u16*)&psxR[((mem) << 13) >> 13])
#define psxRu32ref(mem)	(*(u32*)(psxR + (((mem) << 13) >> 13)))

s8 *psxH;
//#define psxHs8(mem)		psxH[(mem) & 0xffff]
//#define psxHs16(mem)	(SWAP16(*(s16*)&psxH[(mem) & 0xffff]))
//#define psxHs32(mem)	(SWAP32(*(s32*)&psxH[(mem) & 0xffff]))

#define psxHu8(mem)		(*(u8*) &psxH[(mem) & 0xffff])
#define psxHu16(mem)	(LOAD_SWAP16p(psxH[(mem) & 0xffff]))
//#define psxHu32(mem)	(SWAP32(*(u32*)&psxH[(mem) & 0xffff]))
#define psxHu32(mem)	(LOAD_SWAP32p(psxH + (((mem) << 16) >> 16)))

//#define psxHs8ref(mem)	psxH[((mem) << 16) >> 16]
//#define psxHs16ref(mem)	(*(s16*)&psxH[((mem) << 16) >> 16])
//#define psxHs32ref(mem)	(*(s32*)&psxH[((mem) << 16) >> 16])
//#define psxHu8ref(mem)	(*(u8*) &psxH[((mem) << 16) >> 16])
#define psxHu16ref(mem)	(*(u16*)(psxH + (((mem) << 16) >> 16)))
#define psxHu32ref(mem)	(*(u32*)(psxH + (((mem) << 16) >> 16)))

extern u8* psxMemWLUT[0x10000] __attribute__((aligned(32)));
extern u8* psxMemRLUT[0x10000] __attribute__((aligned(32)));

#define PSXM(mem)		(psxMemRLUT[(mem) >> 16] == 0 ? NULL : (u8*)(psxMemRLUT[(mem) >> 16] + ((mem) & 0xffff)))
//#define PSXMs8(mem)		(*(s8 *)PSXM(mem))
//#define PSXMs16(mem)	(SWAP16(*(s16*)PSXM(mem)))
//#define PSXMs32(mem)	(SWAP32(*(s32*)PSXM(mem)))
//#define PSXMu8(mem)		(*(u8 *)PSXM(mem))
//#define PSXMu16(mem)	(SWAP16(*(u16*)PSXM(mem)))
//#define PSXMu32(mem)	(SWAP32(*(u32*)PSXM(mem)))
#define PSXMu32(mem)	(psxMemRLUT[(mem) >> 16] == 0 ? NULL : LOAD_SWAP32p(psxMemRLUT[(mem) >> 16] + ((mem) & 0xffff)))

#define PSXMu32ref(mem)	(*(u32*)PSXM(mem))


#if !defined PSXREC && (defined(__x86_64__) || defined(__i386__) || defined(__sh__) || defined(__ppc__) || defined(HW_RVL)) || defined(HW_DOL)
#define PSXREC
#endif

int  psxMemInit();
void psxMemReset();
void psxMemShutdown();

u8   psxMemRead8 (u32 mem);
u16  psxMemRead16(u32 mem);
u32  psxMemRead32(u32 mem);
void psxMemWrite8 (u32 mem, u8 value);
void psxMemWrite16(u32 mem, u16 value);
void psxMemWrite32(u32 mem, u32 value);
void *psxMemPointer(u32 mem);

#endif /* __PSXMEMORY_H__ */


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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA.           *
 ***************************************************************************/

/*
* Functions for PSX hardware control.
*/

#include "psxhw.h"
#include "mdec.h"
#include "cdrom.h"
#include "gpu.h"
#include "Gamecube/DEBUG.h"

// add xjsxjs197 start
u32 tmpVal;
u32 tmpAddr[1];
u16 tmpVal16;
u16 tmpAddr16[1];
#ifdef DISP_DEBUG
char debug[256];
#endif // DISP_DEBUG
// add xjsxjs197 end

void psxHwReset() {
	if (Config.Sio) psxHu32ref(0x1070) |= SWAP32(0x80);
	if (Config.SpuIrq) psxHu32ref(0x1070) |= SWAP32(0x200);

	memset(psxH, 0, 0x10000);

	mdecInit(); // initialize mdec decoder
	cdrReset();
	psxRcntInit();
	HW_GPU_STATUS = SWAP32(0x14802000);
}

u8 psxHwRead8(u32 add) {
	unsigned char hard;

	switch (add & 0x1fffffff) {
		case 0x1f801040: hard = sioRead8();break;
      //  case 0x1f801050: hard = serial_read8(); break;//for use of serial port ignore for now
		case 0x1f801800: hard = cdrRead0(); break;
		case 0x1f801801: hard = cdrRead1(); break;
		case 0x1f801802: hard = cdrRead2(); break;
		case 0x1f801803: hard = cdrRead3(); break;
		default:
			hard = psxHu8(add);
#ifdef PSXHW_LOG
			PSXHW_LOG("*Unkwnown 8bit read at address %lx\n", add);
#endif
            //PRINT_LOG1("*psxHwRead8 err:  0x%-08x\n", add);
			return hard;
	}

#ifdef PSXHW_LOG
	PSXHW_LOG("*Known 8bit read at address %lx value %x\n", add, hard);
#endif
	return hard;
}

u16 psxHwRead16(u32 add) {
	unsigned short hard;

	switch (add & 0x1fffffff) {
#ifdef PSXHW_LOG
		case 0x1f801070: PSXHW_LOG("IREG 16bit read %x\n", psxHu16(0x1070));
			return psxHu16(0x1070);
#endif
#ifdef PSXHW_LOG
		case 0x1f801074: PSXHW_LOG("IMASK 16bit read %x\n", psxHu16(0x1074));
			return psxHu16(0x1074);
#endif

		case 0x1f801040:
			hard = sioRead8();
			hard|= sioRead8() << 8;
#ifdef PAD_LOG
			PAD_LOG("sio read16 %lx; ret = %x\n", add&0xf, hard);
#endif
			return hard;
		case 0x1f801044:
			hard = StatReg;
#ifdef PAD_LOG
			PAD_LOG("sio read16 %lx; ret = %x\n", add&0xf, hard);
#endif
			return hard;
		case 0x1f801048:
			hard = ModeReg;
#ifdef PAD_LOG
			PAD_LOG("sio read16 %lx; ret = %x\n", add&0xf, hard);
#endif
			return hard;
		case 0x1f80104a:
			hard = CtrlReg;
#ifdef PAD_LOG
			PAD_LOG("sio read16 %lx; ret = %x\n", add&0xf, hard);
#endif
			return hard;
		case 0x1f80104e:
			hard = BaudReg;
#ifdef PAD_LOG
			PAD_LOG("sio read16 %lx; ret = %x\n", add&0xf, hard);
#endif
			return hard;

		//Serial port stuff not support now ;P
	 // case 0x1f801050: hard = serial_read16(); break;
	 //	case 0x1f801054: hard = serial_status_read(); break;
	 //	case 0x1f80105a: hard = serial_control_read(); break;
	 //	case 0x1f80105e: hard = serial_baud_read(); break;

		case 0x1f801100:
			hard = psxRcntRcount(0);
#ifdef PSXHW_LOG
			PSXHW_LOG("T0 count read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801104:
			hard = psxRcntRmode(0);
#ifdef PSXHW_LOG
			PSXHW_LOG("T0 mode read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801108:
			hard = psxRcntRtarget(0);
#ifdef PSXHW_LOG
			PSXHW_LOG("T0 target read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801110:
			hard = psxRcntRcount(1);
#ifdef PSXHW_LOG
			PSXHW_LOG("T1 count read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801114:
			hard = psxRcntRmode(1);
#ifdef PSXHW_LOG
			PSXHW_LOG("T1 mode read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801118:
			hard = psxRcntRtarget(1);
#ifdef PSXHW_LOG
			PSXHW_LOG("T1 target read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801120:
			hard = psxRcntRcount(2);
#ifdef PSXHW_LOG
			PSXHW_LOG("T2 count read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801124:
			hard = psxRcntRmode(2);
#ifdef PSXHW_LOG
			PSXHW_LOG("T2 mode read16: %x\n", hard);
#endif
			return hard;
		case 0x1f801128:
			hard = psxRcntRtarget(2);
#ifdef PSXHW_LOG
			PSXHW_LOG("T2 target read16: %x\n", hard);
#endif
			return hard;

		//case 0x1f802030: hard =   //int_2000????
		//case 0x1f802040: hard =//dip switches...??

		default:
			if (add>=0x1f801c00 && add<0x1f801e00) {
            	hard = SPU_readRegister(add);
			} else {
			    // upd xjsxjs197 start
				//hard = psxHu16(add);
				hard = LOAD_SWAP16p(psxHAddr(add));
				// upd xjsxjs197 end
#ifdef PSXHW_LOG
				PSXHW_LOG("*Unkwnown 16bit read at address %lx\n", add);
#endif
			    //PRINT_LOG1("*psxHwRead16 err:  0x%-08x\n", add);
			}
            return hard;
	}

#ifdef PSXHW_LOG
	PSXHW_LOG("*Known 16bit read at address %lx value %x\n", add, hard);
#endif
	return hard;
}

u32 psxHwRead32(u32 add) {
	u32 hard;

	switch (add & 0x1fffffff) {
		case 0x1f801040:
			hard = sioRead8();
			hard|= sioRead8() << 8;
			hard|= sioRead8() << 16;
			hard|= sioRead8() << 24;
#ifdef PAD_LOG
			PAD_LOG("sio read32 ;ret = %lx\n", hard);
#endif
			return hard;

	//	case 0x1f801050: hard = serial_read32(); break;//serial port
#ifdef PSXHW_LOG
		case 0x1f801060:
			PSXHW_LOG("RAM size read %lx\n", psxHu32(0x1060));
			return psxHu32(0x1060);
#endif
#ifdef PSXHW_LOG
		case 0x1f801070: PSXHW_LOG("IREG 32bit read %x\n", psxHu32(0x1070));
			return psxHu32(0x1070);
#endif
#ifdef PSXHW_LOG
		case 0x1f801074: PSXHW_LOG("IMASK 32bit read %x\n", psxHu32(0x1074));
			return psxHu32(0x1074);
#endif

		case 0x1f801810:
			hard = GPU_readData();
#ifdef PSXHW_LOG
			PSXHW_LOG("GPU DATA 32bit read %lx\n", hard);
#endif
			return hard;
		case 0x1f801814:
			hard = GPU_readStatus();
			//gpuSyncPluginSR();
			//HW_GPU_STATUS &= SWAP32(PSXGPU_TIMING_BITS);
			// Gameshark Lite - wants to see VRAM busy
            // - Must enable GPU 'Fake Busy States' hack
            if( (hard & GPUSTATUS_IDLE) == 0 )
            {
                #ifdef SHOW_DEBUG
                sprintf(txtbuffer, "Read GPU_STATUS Fake Busy \n");
                DEBUG_print(txtbuffer, DBG_CORE2);
                writeLogFile(txtbuffer);
                #endif // DISP_DEBUG
                hard &= ~GPUSTATUS_READYFORVRAM;
                //HW_GPU_STATUS &= SWAP32(~GPUSTATUS_READYFORVRAM);
            }

	        //HW_GPU_STATUS |= SWAP32(hard & ~PSXGPU_TIMING_BITS);
			//hard = SWAP32(HW_GPU_STATUS);
			if (hSyncCount < 240 && (hard & PSXGPU_ILACE_BITS) != PSXGPU_ILACE_BITS)
				hard |= PSXGPU_LCF & (psxRegs.cycle << 20);
#ifdef PSXHW_LOG
			PSXHW_LOG("GPU STATUS 32bit read %lx\n", hard);
#endif
			return hard;

		case 0x1f801820: hard = mdecRead0(); break;
		case 0x1f801824: hard = mdecRead1(); break;

#ifdef PSXHW_LOG
		case 0x1f8010a0:
			PSXHW_LOG("DMA2 MADR 32bit read %x\n", psxHu32(0x10a0));
			return SWAPu32(HW_DMA2_MADR);
		case 0x1f8010a4:
			PSXHW_LOG("DMA2 BCR 32bit read %x\n", psxHu32(0x10a4));
			return SWAPu32(HW_DMA2_BCR);
		case 0x1f8010a8:
			PSXHW_LOG("DMA2 CHCR 32bit read %x\n", psxHu32(0x10a8));
			return SWAPu32(HW_DMA2_CHCR);
#endif

#ifdef PSXHW_LOG
		case 0x1f8010b0:
			PSXHW_LOG("DMA3 MADR 32bit read %x\n", psxHu32(0x10b0));
			return SWAPu32(HW_DMA3_MADR);
		case 0x1f8010b4:
			PSXHW_LOG("DMA3 BCR 32bit read %x\n", psxHu32(0x10b4));
			return SWAPu32(HW_DMA3_BCR);
		case 0x1f8010b8:
			PSXHW_LOG("DMA3 CHCR 32bit read %x\n", psxHu32(0x10b8));
			return SWAPu32(HW_DMA3_CHCR);
#endif

#ifdef PSXHW_LOG
/*		case 0x1f8010f0:
			PSXHW_LOG("DMA PCR 32bit read %x\n", psxHu32(0x10f0));
			return SWAPu32(HW_DMA_PCR); // dma rest channel
		case 0x1f8010f4:
			PSXHW_LOG("DMA ICR 32bit read %x\n", psxHu32(0x10f4));
			return SWAPu32(HW_DMA_ICR); // interrupt enabler?*/
#endif

		// time for rootcounters :)
		case 0x1f801100:
			hard = psxRcntRcount(0);
#ifdef PSXHW_LOG
			PSXHW_LOG("T0 count read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801104:
			hard = psxRcntRmode(0);
#ifdef PSXHW_LOG
			PSXHW_LOG("T0 mode read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801108:
			hard = psxRcntRtarget(0);
#ifdef PSXHW_LOG
			PSXHW_LOG("T0 target read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801110:
			hard = psxRcntRcount(1);
#ifdef PSXHW_LOG
			PSXHW_LOG("T1 count read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801114:
			hard = psxRcntRmode(1);
#ifdef PSXHW_LOG
			PSXHW_LOG("T1 mode read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801118:
			hard = psxRcntRtarget(1);
#ifdef PSXHW_LOG
			PSXHW_LOG("T1 target read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801120:
			hard = psxRcntRcount(2);
#ifdef PSXHW_LOG
			PSXHW_LOG("T2 count read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801124:
			hard = psxRcntRmode(2);
#ifdef PSXHW_LOG
			PSXHW_LOG("T2 mode read32: %lx\n", hard);
#endif
			return hard;
		case 0x1f801128:
			hard = psxRcntRtarget(2);
#ifdef PSXHW_LOG
			PSXHW_LOG("T2 target read32: %lx\n", hard);
#endif
			return hard;

		default:
            // upd xjsxjs197 start
			//hard = psxHu32(add);
			hard = LOAD_SWAP32p(psxHAddr(add));
			// upd xjsxjs197 end
#ifdef PSXHW_LOG
			PSXHW_LOG("*Unkwnown 32bit read at address %lx\n", add);
#endif
            //PRINT_LOG1("*psxHwRead32 err:  0x%-08x\n", add);
			return hard;
	}
#ifdef PSXHW_LOG
	PSXHW_LOG("*Known 32bit read at address %lx\n", add);
#endif
	return hard;
}

void psxHwWrite8(u32 add, u8 value) {
	switch (add & 0x1fffffff) {
		case 0x1f801040: sioWrite8(value); break;
	//	case 0x1f801050: serial_write8(value); break;//serial port
		case 0x1f801800: cdrWrite0(value); break;
		case 0x1f801801: cdrWrite1(value); break;
		case 0x1f801802: cdrWrite2(value); break;
		case 0x1f801803: cdrWrite3(value); break;

		default:
			psxHu8(add) = value;
#ifdef PSXHW_LOG
			PSXHW_LOG("*Unknown 8bit write at address %lx value %x\n", add, value);
#endif
            //PRINT_LOG2("psxHwWrite8 err: 0x%-08x 0x%-08x", add, value);
			return;
	}
	psxHu8(add) = value;
#ifdef PSXHW_LOG
	PSXHW_LOG("*Known 8bit write at address %lx value %x\n", add, value);
#endif
}

void psxHwWrite16(u32 add, u16 value) {
	switch (add & 0x1fffffff) {
		case 0x1f801040:
			sioWrite8((unsigned char)value);
			sioWrite8((unsigned char)(value>>8));
#ifdef PAD_LOG
			PAD_LOG ("sio write16 %lx, %x\n", add&0xf, value);
#endif
			return;
		case 0x1f801044:
#ifdef PAD_LOG
			PAD_LOG ("sio write16 %lx, %x\n", add&0xf, value);
#endif
			return;
		case 0x1f801048:
			ModeReg = value;
#ifdef PAD_LOG
			PAD_LOG ("sio write16 %lx, %x\n", add&0xf, value);
#endif
			return;
		case 0x1f80104a: // control register
			sioWriteCtrl16(value);
#ifdef PAD_LOG
			PAD_LOG ("sio write16 %lx, %x\n", add&0xf, value);
#endif
			return;
		case 0x1f80104e: // baudrate register
			BaudReg = value;
#ifdef PAD_LOG
			PAD_LOG ("sio write16 %lx, %x\n", add&0xf, value);
#endif
			return;

		//serial port ;P
	//  case 0x1f801050: serial_write16(value); break;
	//	case 0x1f80105a: serial_control_write(value);break;
	//	case 0x1f80105e: serial_baud_write(value); break;
	//	case 0x1f801054: serial_status_write(value); break;

		case 0x1f801070:
#ifdef PSXHW_LOG
			PSXHW_LOG("IREG 16bit write %x\n", value);
#endif
			if (Config.Sio) psxHu16ref(0x1070) |= SWAPu16(0x80);
			if (Config.SpuIrq) psxHu16ref(0x1070) |= SWAPu16(0x200);
			// upd xjsxjs197 start
			//psxHu16ref(0x1070) &= SWAPu16(value);
            STORE_SWAP16p(tmpAddr16, (value));
            psxHu16ref(0x1070) &= tmpAddr16[0];
			// upd xjsxjs197 end
			return;

		case 0x1f801074:
#ifdef PSXHW_LOG
			PSXHW_LOG("IMASK 16bit write %x\n", value);
#endif
            // upd xjsxjs197 start
			//psxHu16ref(0x1074) = SWAPu16(value);
			STORE_SWAP16p(psxHAddr(0x1074), value);
			// upd xjsxjs197 end
			psxRegs.interrupt|= 0x80000000;
			return;

		case 0x1f801100:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 0 COUNT 16bit write %x\n", value);
#endif
			psxRcntWcount(0, value); return;
		case 0x1f801104:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 0 MODE 16bit write %x\n", value);
#endif
			psxRcntWmode(0, value); return;
		case 0x1f801108:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 0 TARGET 16bit write %x\n", value);
#endif
			psxRcntWtarget(0, value); return;

		case 0x1f801110:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 1 COUNT 16bit write %x\n", value);
#endif
			psxRcntWcount(1, value); return;
		case 0x1f801114:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 1 MODE 16bit write %x\n", value);
#endif
			psxRcntWmode(1, value); return;
		case 0x1f801118:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 1 TARGET 16bit write %x\n", value);
#endif
			psxRcntWtarget(1, value); return;

		case 0x1f801120:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 2 COUNT 16bit write %x\n", value);
#endif
			psxRcntWcount(2, value); return;
		case 0x1f801124:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 2 MODE 16bit write %x\n", value);
#endif
			psxRcntWmode(2, value); return;
		case 0x1f801128:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 2 TARGET 16bit write %x\n", value);
#endif
			psxRcntWtarget(2, value); return;

		default:
			if (add>=0x1f801c00 && add<0x1f801e00) {
            	SPU_writeRegister(add, value, psxRegs.cycle);
				return;
			}

            // upd xjsxjs197 start
			//psxHu16ref(add) = SWAPu16(value);
			STORE_SWAP16p(psxHAddr(add), value);
			// upd xjsxjs197 end
#ifdef PSXHW_LOG
			PSXHW_LOG("*Unknown 16bit write at address %lx value %x\n", add, value);
#endif
            //PRINT_LOG2("psxHwWrite16 err: 0x%-08x 0x%-08x", add, value);
			return;
	}
	// upd xjsxjs197 start
	//psxHu16ref(add) = SWAPu16(value);
	STORE_SWAP16p(psxHAddr(add), value);
	// upd xjsxjs197 end
#ifdef PSXHW_LOG
	PSXHW_LOG("*Known 16bit write at address %lx value %x\n", add, value);
#endif
}

// upd xjsxjs197 start
/*#define DmaExec(n) { \
	if (SWAPu32(HW_DMA##n##_CHCR) & 0x01000000) return; \
	HW_DMA##n##_CHCR = SWAPu32(value); \
 \
	if (SWAPu32(HW_DMA##n##_CHCR) & 0x01000000 && SWAPu32(HW_DMA_PCR) & (8 << (n * 4))) { \
		psxDma##n(SWAPu32(HW_DMA##n##_MADR), SWAPu32(HW_DMA##n##_BCR), SWAPu32(HW_DMA##n##_CHCR)); \
	} \
}*/
#define DmaExec(char, bcr, madr, n) { \
	if (LOAD_SWAP32p(psxHAddr(char)) & 0x01000000) return; \
	STORE_SWAP32p(psxHAddr(char), value); \
 \
    tmpVal = LOAD_SWAP32p(psxHAddr(char)); \
	if (tmpVal & 0x01000000 && LOAD_SWAP32p(psxHAddr(0x10f0)) & (8 << (n * 4))) { \
		psxDma##n(LOAD_SWAP32p(psxHAddr(madr)), LOAD_SWAP32p(psxHAddr(bcr)), tmpVal); \
	} \
}
// upd xjsxjs197 end

void psxHwWrite32(u32 add, u32 value) {
	switch (add & 0x1fffffff) {
	    case 0x1f801040:
			sioWrite8((unsigned char)value);
			sioWrite8((unsigned char)((value&0xff) >>  8));
			sioWrite8((unsigned char)((value&0xff) >> 16));
			sioWrite8((unsigned char)((value&0xff) >> 24));
#ifdef PAD_LOG
			PAD_LOG("sio write32 %lx\n", value);
#endif
			return;
	//	case 0x1f801050: serial_write32(value); break;//serial port
#ifdef PSXHW_LOG
		case 0x1f801060:
			PSXHW_LOG("RAM size write %lx\n", value);
			psxHu32ref(add) = SWAPu32(value);
			return; // Ram size
#endif

		case 0x1f801070:
#ifdef PSXHW_LOG
			PSXHW_LOG("IREG 32bit write %lx\n", value);
#endif
			if (Config.Sio) psxHu32ref(0x1070) |= SWAPu32(0x80);
			if (Config.SpuIrq) psxHu32ref(0x1070) |= SWAPu32(0x200);
			// upd xjsxjs197 start
			//psxHu32ref(0x1070) &= SWAPu32(value);
            STORE_SWAP32p(tmpAddr, (value));
            psxHu32ref(0x1070) &= (u32)(tmpAddr[0]);
			// upd xjsxjs197 end
			return;
		case 0x1f801074:
#ifdef PSXHW_LOG
			PSXHW_LOG("IMASK 32bit write %lx\n", value);
#endif
            // upd xjsxjs197 start
			//psxHu32ref(0x1074) = SWAPu32(value);
			STORE_SWAP32p(psxHAddr(0x1074), value);
			// upd xjsxjs197 end
			psxRegs.interrupt|= 0x80000000;
			return;

#ifdef PSXHW_LOG
		case 0x1f801080:
			PSXHW_LOG("DMA0 MADR 32bit write %lx\n", value);
			HW_DMA0_MADR = SWAPu32(value); return; // DMA0 madr
		case 0x1f801084:
			PSXHW_LOG("DMA0 BCR 32bit write %lx\n", value);
			HW_DMA0_BCR  = SWAPu32(value); return; // DMA0 bcr
#endif
		case 0x1f801088:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA0 CHCR 32bit write %lx\n", value);
#endif
			//DmaExec(0);	                 // DMA0 chcr (MDEC in DMA)
			DmaExec(0x1088, 0x1084, 0x1080, 0);
			return;

#ifdef PSXHW_LOG
		case 0x1f801090:
			PSXHW_LOG("DMA1 MADR 32bit write %lx\n", value);
			HW_DMA1_MADR = SWAPu32(value); return; // DMA1 madr
		case 0x1f801094:
			PSXHW_LOG("DMA1 BCR 32bit write %lx\n", value);
			HW_DMA1_BCR  = SWAPu32(value); return; // DMA1 bcr
#endif
		case 0x1f801098:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA1 CHCR 32bit write %lx\n", value);
#endif
			//DmaExec(1);                  // DMA1 chcr (MDEC out DMA)
			DmaExec(0x1098, 0x1094, 0x1090, 1);
			return;

#ifdef PSXHW_LOG
		case 0x1f8010a0:
			PSXHW_LOG("DMA2 MADR 32bit write %lx\n", value);
			HW_DMA2_MADR = SWAPu32(value); return; // DMA2 madr
		case 0x1f8010a4:
			PSXHW_LOG("DMA2 BCR 32bit write %lx\n", value);
			HW_DMA2_BCR  = SWAPu32(value); return; // DMA2 bcr
#endif
		case 0x1f8010a8:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA2 CHCR 32bit write %lx\n", value);
#endif
			//DmaExec(2);                  // DMA2 chcr (GPU DMA)
			DmaExec(0x10a8, 0x10a4, 0x10a0, 2);
			return;

#ifdef PSXHW_LOG
		case 0x1f8010b0:
			PSXHW_LOG("DMA3 MADR 32bit write %lx\n", value);
			HW_DMA3_MADR = SWAPu32(value); return; // DMA3 madr
		case 0x1f8010b4:
			PSXHW_LOG("DMA3 BCR 32bit write %lx\n", value);
			HW_DMA3_BCR  = SWAPu32(value); return; // DMA3 bcr
#endif
		case 0x1f8010b8:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA3 CHCR 32bit write %lx\n", value);
#endif
			//DmaExec(3);                  // DMA3 chcr (CDROM DMA)
            DmaExec(0x10b8, 0x10b4, 0x10b0, 3);
			return;

#ifdef PSXHW_LOG
		case 0x1f8010c0:
			PSXHW_LOG("DMA4 MADR 32bit write %lx\n", value);
			HW_DMA4_MADR = SWAPu32(value); return; // DMA4 madr
		case 0x1f8010c4:
			PSXHW_LOG("DMA4 BCR 32bit write %lx\n", value);
			HW_DMA4_BCR  = SWAPu32(value); return; // DMA4 bcr
#endif
		case 0x1f8010c8:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA4 CHCR 32bit write %lx\n", value);
#endif
			//DmaExec(4);                  // DMA4 chcr (SPU DMA)
			DmaExec(0x10c8, 0x10c4, 0x10c0, 4);
			return;

#if 0
		case 0x1f8010d0: break; //DMA5write_madr();
		case 0x1f8010d4: break; //DMA5write_bcr();
		case 0x1f8010d8: break; //DMA5write_chcr(); // Not needed
#endif

#ifdef PSXHW_LOG
		case 0x1f8010e0:
			PSXHW_LOG("DMA6 MADR 32bit write %lx\n", value);
			HW_DMA6_MADR = SWAPu32(value); return; // DMA6 bcr
		case 0x1f8010e4:
			PSXHW_LOG("DMA6 BCR 32bit write %lx\n", value);
			HW_DMA6_BCR  = SWAPu32(value); return; // DMA6 bcr
#endif
		case 0x1f8010e8:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA6 CHCR 32bit write %lx\n", value);
#endif
			//DmaExec(6);                   // DMA6 chcr (OT clear)
			DmaExec(0x10e8, 0x10e4, 0x10e0, 6);
			return;

#ifdef PSXHW_LOG
		case 0x1f8010f0:
			PSXHW_LOG("DMA PCR 32bit write %lx\n", value);
			HW_DMA_PCR = SWAPu32(value);
			return;
#endif

		case 0x1f8010f4:
#ifdef PSXHW_LOG
			PSXHW_LOG("DMA ICR 32bit write %lx\n", value);
#endif
		{
		    // upd xjsxjs197 start
			//u32 tmp = (~value) & SWAPu32(HW_DMA_ICR);
			//HW_DMA_ICR = SWAPu32(((tmp ^ value) & 0xffffff) ^ tmp);
			//u32 tmp = (~value) & LOAD_SWAP32p(psxHAddr(0x10f4));
			//STORE_SWAP32p(psxHAddr(0x10f4), (u32)(((tmp ^ value) & 0xffffff) ^ tmp));

			/*u32 tmp = value & 0x00ff803f;
			tmp |= (SWAPu32(HW_DMA_ICR) & ~value) & 0x7f000000;
			if ((tmp & HW_DMA_ICR_GLOBAL_ENABLE && tmp & 0x7f000000)
			    || tmp & HW_DMA_ICR_BUS_ERROR) {
				if (!(SWAPu32(HW_DMA_ICR) & HW_DMA_ICR_IRQ_SENT))
					psxHu32ref(0x1070) |= SWAP32(8);
				tmp |= HW_DMA_ICR_IRQ_SENT;
			}
			HW_DMA_ICR = SWAPu32(tmp);*/
			u32 tmp = value & 0x00ff803f;
			tmpVal = LOAD_SWAP32p(psxHAddr(0x10f4));
			tmp |= (tmpVal & ~value) & 0x7f000000;
			if ((tmp & HW_DMA_ICR_GLOBAL_ENABLE && tmp & 0x7f000000)
			    || tmp & HW_DMA_ICR_BUS_ERROR) {
				if (!(tmpVal & HW_DMA_ICR_IRQ_SENT))
					psxHu32ref(0x1070) |= SWAP32(8);
				tmp |= HW_DMA_ICR_IRQ_SENT;
			}
			STORE_SWAP32p(psxHAddr(0x10f4), tmp);
			// upd xjsxjs197 end
			return;
		}

		case 0x1f801810:
#ifdef PSXHW_LOG
			PSXHW_LOG("GPU DATA 32bit write %lx\n", value);
#endif
			GPU_writeData(value); return;
		case 0x1f801814:
#ifdef PSXHW_LOG
			PSXHW_LOG("GPU STATUS 32bit write %lx\n", value);
#endif
			GPU_writeStatus(value);
			gpuSyncPluginSR();
			return;

		case 0x1f801820:
			mdecWrite0(value); break;
		case 0x1f801824:
			mdecWrite1(value); break;

		case 0x1f801100:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 0 COUNT 32bit write %lx\n", value);
#endif
			psxRcntWcount(0, value & 0xffff); return;
		case 0x1f801104:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 0 MODE 32bit write %lx\n", value);
#endif
			psxRcntWmode(0, value); return;
		case 0x1f801108:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 0 TARGET 32bit write %lx\n", value);
#endif
			psxRcntWtarget(0, value & 0xffff); return; //  HW_DMA_ICR&= SWAP32((~value)&0xff000000);

		case 0x1f801110:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 1 COUNT 32bit write %lx\n", value);
#endif
			psxRcntWcount(1, value & 0xffff); return;
		case 0x1f801114:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 1 MODE 32bit write %lx\n", value);
#endif
			psxRcntWmode(1, value); return;
		case 0x1f801118:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 1 TARGET 32bit write %lx\n", value);
#endif
			psxRcntWtarget(1, value & 0xffff); return;

		case 0x1f801120:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 2 COUNT 32bit write %lx\n", value);
#endif
			psxRcntWcount(2, value & 0xffff); return;
		case 0x1f801124:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 2 MODE 32bit write %lx\n", value);
#endif
			psxRcntWmode(2, value); return;
		case 0x1f801128:
#ifdef PSXHW_LOG
			PSXHW_LOG("COUNTER 2 TARGET 32bit write %lx\n", value);
#endif
			psxRcntWtarget(2, value & 0xffff); return;

		default:
		    // Dukes of Hazard 2 - car engine noise
			if (add>=0x1f801c00 && add<0x1f801e00) {
				SPU_writeRegister(add, value&0xffff, psxRegs.cycle);
				SPU_writeRegister(add + 2, value>>16, psxRegs.cycle);
				return;
			}

		    // upd xjsxjs197 start
			//psxHu32ref(add) = SWAPu32(value);
			STORE_SWAP32p(psxHAddr(add), value);
			// upd xjsxjs197 end
#ifdef PSXHW_LOG
			PSXHW_LOG("*Unknown 32bit write at address %lx value %lx\n", add, value);
#endif
            //PRINT_LOG2("psxHwWrite32 err: 0x%-08x 0x%-08x", add, value);
			return;
	}
	// upd xjsxjs197 start
	//psxHu32ref(add) = SWAPu32(value);
	STORE_SWAP32p(psxHAddr(add), value);
	// upd xjsxjs197 end
#ifdef PSXHW_LOG
	PSXHW_LOG("*Known 32bit write at address %lx value %lx\n", add, value);
#endif
}

int psxHwFreeze(gzFile f, int Mode) {
	return 0;
}

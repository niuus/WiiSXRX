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
* R3000A CPU functions.
*/

#include "r3000a.h"
#include "psxhw.h"
#include "psxdma.h"
#include "cdrom.h"
#include "mdec.h"
R3000Acpu *psxCpu;
psxRegisters psxRegs;

int psxInit() {

	if(Config.Cpu) {
		if(Config.Dbg) psxCpu = &psxIntDbg;
		else 	psxCpu = &psxInt;
	}
#if defined(__x86_64__) || defined(__i386__) || defined(__sh__) || defined(__ppc__) || defined(HW_RVL) || defined(HW_DOL)
	if (!Config.Cpu) psxCpu = &psxRec;
#endif
	Log=0;

	if (psxMemInit() == -1) return -1;

	return psxCpu->Init();
}

void psxReset() {
	psxMemReset();

	memset(&psxRegs, 0, sizeof(psxRegs));

	psxRegs.pc = 0xbfc00000; // Start in bootstrap

	psxRegs.CP0.r[12] = 0x10900000; // COP0 enabled | BEV = 1 | TS = 1
	psxRegs.CP0.r[15] = 0x00000002; // PRevID = Revision ID, same as R3000A

	psxCpu->Reset();

	psxHwReset();
	psxBiosInit();

	if (!Config.HLE)
		psxExecuteBios();

#ifdef EMU_LOG
	EMU_LOG("*BIOS END*\n");
#endif
	Log = 0;
}

void psxShutdown() {
	psxMemShutdown();
	psxBiosShutdown();

    if (psxCpu && psxCpu->Shutdown)
    {
        psxCpu->Shutdown();
    }
}

void psxException(u32 code, u32 bd) {
	// Set the Cause
	// upd xjsxjs197 start
	//psxRegs.CP0.n.Cause = code;
	psxRegs.CP0.n.Cause = (psxRegs.CP0.n.Cause & 0x300) | code;
	// upd xjsxjs197 end

	// Set the EPC & PC
	if (bd) {
#ifdef PSXCPU_LOG
		PSXCPU_LOG("bd set!!!\n");
#endif
        //SysPrintf("bd set!!!\n");
		//PRINT_LOG("=====bd set!!!");
		psxRegs.CP0.n.Cause |= 0x80000000;
		psxRegs.CP0.n.EPC = (psxRegs.pc - 4);
	} else
		psxRegs.CP0.n.EPC = (psxRegs.pc);

	if (psxRegs.CP0.n.Status & 0x400000)
		psxRegs.pc = 0xbfc00180;
	else
		psxRegs.pc = 0x80000080;

	// Set the Status
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status &~0x3f) |
						  ((psxRegs.CP0.n.Status & 0xf) << 2);

	// upd xjsxjs197 start
	/*if (!Config.HLE && (((SWAP32(*(u32*)PSXM(psxRegs.CP0.n.EPC)) >> 24) & 0xfe) == 0x4a)) {
	    // "hokuto no ken" / "Crash Bandicot 2" ... fix
	    PSXMu32ref(psxRegs.CP0.n.EPC)&= SWAP32(~0x02000000);

      if (Config.HLE) psxBiosException();
    }*/
    if (Config.HLE)
    {
        psxBiosException();
    }
    else
    {
	    // "hokuto no ken" / "Crash Bandicot 2" ...
		// BIOS does not allow to return to GTE instructions
		// (just skips it, supposedly because it's scheduled already)
		// so we execute it here
		u32 tmp = PSXMu32(psxRegs.CP0.n.EPC);
		psxRegs.code = tmp;
		if (tmp != NULL && ((tmp >> 24) & 0xfe) == 0x4a) {
            #ifdef DISP_DEBUG
            PRINT_LOG("========hokuto no ken Fix ");
            #endif // DISP_DEBUG
		    PSXMu32ref(psxRegs.CP0.n.EPC) &= SWAP32(~0x02000000);

            psxRegs.code = PSXMu32(psxRegs.CP0.n.EPC);
			extern void (*psxCP2[64])();
		    psxCP2[psxRegs.code & 0x3f]();
		}
		#ifdef DISP_DEBUG
		else if (tmp == NULL )
        {
            tmp = PSXMu32(psxRegs.CP0.n.EPC + 4);
            if (tmp == NULL)
            {
                //PRINT_LOG("===psxException NULL Pointer, [EPC + 4] also NULL");
            }
            else
            {
                //PRINT_LOG1("===psxException NULL Pointer, [EPC + 4]: %x", tmp);
            }
        }
        #endif
	}
	// upd xjsxjs197 end
}

extern u32 psxNextCounter, psxNextsCounter;
void psxBranchTest() {

	if ((psxRegs.cycle - psxNextsCounter) >= psxNextCounter)
		psxRcntUpdate();

	if (psxRegs.interrupt) {
		if ((psxRegs.interrupt & (1 << PSXINT_SIO)) && (!Config.Sio)) { // sio
            //if ((psxRegs.cycle - psxRegs.intCycle[7]) >= psxRegs.intCycle[7+1]) {
            if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_SIO].sCycle) >= psxRegs.intCycle[PSXINT_SIO].cycle) {
				psxRegs.interrupt&=~(1 << PSXINT_SIO);
				sioInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_CDR)) { // cdr
            //if ((psxRegs.cycle - psxRegs.intCycle[2]) >= psxRegs.intCycle[2+1]) {
            if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_CDR].sCycle) >= psxRegs.intCycle[PSXINT_CDR].cycle) {
				psxRegs.interrupt&=~(1 << PSXINT_CDR);
				cdrInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_CDREAD)) { // cdr read
		    //if ((psxRegs.cycle - psxRegs.intCycle[2+16]) >= psxRegs.intCycle[2+16+1]) {
		    if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_CDREAD].sCycle) >= psxRegs.intCycle[PSXINT_CDREAD].cycle) {
				psxRegs.interrupt&=~(1 << PSXINT_CDREAD);
				cdrReadInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_GPUDMA)) { // gpu dma
		    //if ((psxRegs.cycle - psxRegs.intCycle[3+24]) >= psxRegs.intCycle[3+24+1]) {
		    if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_GPUDMA].sCycle) >= psxRegs.intCycle[PSXINT_GPUDMA].cycle) {
				psxRegs.interrupt&=~(1 << PSXINT_GPUDMA);
				gpuInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_MDECOUTDMA)) { // mdec out dma
		    //if ((psxRegs.cycle - psxRegs.intCycle[5+24]) >= psxRegs.intCycle[5+24+1]) {
		    if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_MDECOUTDMA].sCycle) >= psxRegs.intCycle[PSXINT_MDECOUTDMA].cycle) {
				psxRegs.interrupt&=~(1 << PSXINT_MDECOUTDMA);
				mdec1Interrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_SPUDMA)) { // spu dma
			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_SPUDMA].sCycle) >= psxRegs.intCycle[PSXINT_SPUDMA].cycle) {
				psxRegs.interrupt &= ~(1 << PSXINT_SPUDMA);
				spuInterrupt();
			}
		}
//		if (psxRegs.interrupt & (1 << PSXINT_MDECINDMA)) { // mdec in
//			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_MDECINDMA].sCycle) >= psxRegs.intCycle[PSXINT_MDECINDMA].cycle) {
//				psxRegs.interrupt &= ~(1 << PSXINT_MDECINDMA);
//				mdec0Interrupt();
//			}
//		}
		if (psxRegs.interrupt & (1 << PSXINT_GPUOTCDMA)) { // gpu otc
			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_GPUOTCDMA].sCycle) >= psxRegs.intCycle[PSXINT_GPUOTCDMA].cycle) {
				psxRegs.interrupt &= ~(1 << PSXINT_GPUOTCDMA);
				gpuotcInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_CDRDMA)) { // cdrom
			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_CDRDMA].sCycle) >= psxRegs.intCycle[PSXINT_CDRDMA].cycle) {
				psxRegs.interrupt &= ~(1 << PSXINT_CDRDMA);
				cdrDmaInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_CDRPLAY)) { // cdr play timing
			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_CDRPLAY].sCycle) >= psxRegs.intCycle[PSXINT_CDRPLAY].cycle) {
				psxRegs.interrupt &= ~(1 << PSXINT_CDRPLAY);
				cdrPlayInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_CDRLID)) { // cdr lid states
			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_CDRLID].sCycle) >= psxRegs.intCycle[PSXINT_CDRLID].cycle) {
				psxRegs.interrupt &= ~(1 << PSXINT_CDRLID);
				cdrLidSeekInterrupt();
			}
		}
		if (psxRegs.interrupt & (1 << PSXINT_SPU_UPDATE)) { // scheduled spu update
			if ((psxRegs.cycle - psxRegs.intCycle[PSXINT_SPU_UPDATE].sCycle) >= psxRegs.intCycle[PSXINT_SPU_UPDATE].cycle) {
				psxRegs.interrupt &= ~(1 << PSXINT_SPU_UPDATE);
				spuUpdate();
			}
		}

		//if (psxRegs.interrupt & 0x80000000) {
		//	psxRegs.interrupt&=~0x80000000;
			psxTestHWInts();
		//}
	}
//	if (psxRegs.cycle > 0xd29c6500) Log=1;
}

inline void psxTestHWInts() {
    // upd xjsxjs197 start
	//if (psxHu32(0x1070) & psxHu32(0x1074)) {
	//if (LOAD_SWAP32p(psxHAddr(0x1070)) & LOAD_SWAP32p(psxHAddr(0x1074))) {
	if (*((u32*)psxHAddr(0x1070)) & *((u32*)psxHAddr(0x1074))) {
    // upd xjsxjs197 end
		if ((psxRegs.CP0.n.Status & 0x401) == 0x401) {
            u32 opcode;

			// Crash Bandicoot 2: Don't run exceptions when GTE in pipeline
			opcode = SWAP32(*Read_ICache(psxRegs.pc, TRUE));
			if( ((opcode >> 24) & 0xfe) != 0x4a ) {
			    psxException(0x400, 0);
			}
#ifdef PSXCPU_LOG
			PSXCPU_LOG("Interrupt: %x %x\n", psxHu32(0x1070), psxHu32(0x1074));
#endif
//			SysPrintf("Interrupt (%x): %x %x\n", psxRegs.cycle, psxHu32(0x1070), psxHu32(0x1074));
			//psxException(0x400, 0);
		}
	}
}

void psxJumpTest() {
	if (!Config.HLE && Config.PsxOut) {
		u32 call = psxRegs.GPR.n.t1 & 0xff;
		switch (psxRegs.pc & 0x1fffff) {
			case 0xa0:
#ifdef PSXBIOS_LOG
				if (call != 0x28 && call != 0xe) {
					PSXBIOS_LOG("Bios call a0: %s (%x) %x,%x,%x,%x\n", biosA0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3); }
#endif
				if (biosA0[call])
					biosA0[call]();
				break;
			case 0xb0:
#ifdef PSXBIOS_LOG
				if (call != 0x17 && call != 0xb) {
					PSXBIOS_LOG("Bios call b0: %s (%x) %x,%x,%x,%x\n", biosB0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3); }
#endif
				if (biosB0[call])
					biosB0[call]();
				break;
			case 0xc0:
#ifdef PSXBIOS_LOG
				PSXBIOS_LOG("Bios call c0: %s (%x) %x,%x,%x,%x\n", biosC0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3);
#endif
				if (biosC0[call])
					biosC0[call]();
				break;
		}
	}
}

void psxExecuteBios() {
	while (psxRegs.pc != 0x80030000)
		psxCpu->ExecuteBlock();
}


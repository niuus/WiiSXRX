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
* Miscellaneous functions, including savestates and CD-ROM loading.
*/

#include "misc.h"
#include "cdrom.h"
#include "psxhw.h"
#include "mdec.h"
#include "ppf.h"
#include "Gamecube/wiiSXconfig.h"
#include "Gamecube/fileBrowser/fileBrowser-libfat.h"

int Log = 0;

/* PSX Executable types */
#define PSX_EXE     1
#define CPE_EXE     2
#define COFF_EXE    3
#define INVALID_EXE 4

#define ISODCL(from, to) (to - from + 1)

struct iso_directory_record {
	char length			[ISODCL (1, 1)]; /* 711 */
	char ext_attr_length		[ISODCL (2, 2)]; /* 711 */
	char extent			[ISODCL (3, 10)]; /* 733 */
	char size			[ISODCL (11, 18)]; /* 733 */
	char date			[ISODCL (19, 25)]; /* 7 by 711 */
	char flags			[ISODCL (26, 26)];
	char file_unit_size		[ISODCL (27, 27)]; /* 711 */
	char interleave			[ISODCL (28, 28)]; /* 711 */
	char volume_sequence_number	[ISODCL (29, 32)]; /* 723 */
	unsigned char name_len		[ISODCL (33, 33)]; /* 711 */
	char name			[1];
};

void mmssdd( char *b, char *p )
{
	int m, s, d;
	int block = SWAP32(*((uint32_t*) b));

	block += 150;
	m = block / 4500;			// minutes
	block = block - m * 4500;	// minutes rest
	s = block / 75;				// seconds
	d = block - s * 75;			// seconds rest

	m = ((m / 10) << 4) | m % 10;
	s = ((s / 10) << 4) | s % 10;
	d = ((d / 10) << 4) | d % 10;

	p[0] = m;
	p[1] = s;
	p[2] = d;
}

#define incTime() \
	time[0] = btoi(time[0]); time[1] = btoi(time[1]); time[2] = btoi(time[2]); \
	time[2]++; \
	if(time[2] == 75) { \
		time[2] = 0; \
		time[1]++; \
		if (time[1] == 60) { \
			time[1] = 0; \
			time[0]++; \
		} \
	} \
	time[0] = itob(time[0]); time[1] = itob(time[1]); time[2] = itob(time[2]);

#define READTRACK() \
	if (CDR_readTrack(time) == 0) return -1; \
	buf = (void *)CDR_getBuffer(); \
	if (buf == NULL) return -1; \
	/*else CheckPPFCache((u8 *)buf, time[0], time[1], time[2]);*/

#define READDIR(_dir) \
	READTRACK(); \
	memcpy(_dir, buf + 12, 2048); \
 \
	incTime(); \
	READTRACK(); \
	memcpy(_dir + 2048, buf + 12, 2048);

int GetCdromFile(u8 *mdir, u8 *time, char *filename) {
	struct iso_directory_record *dir;
	int retval = -1;
	u8 ddir[4096];
	u8 *buf;
	int i;

	// only try to scan if a filename is given
	if (!strlen(filename)) return -1;

	i = 0;
	while (i < 4096) {
		dir = (struct iso_directory_record*) &mdir[i];
		if (dir->length[0] == 0) {
			return -1;
		}
		i += (u8)dir->length[0];

		if (dir->flags[0] & 0x2) { // it's a dir
			if (!strnicmp((char *)&dir->name[0], filename, dir->name_len[0])) {
				if (filename[dir->name_len[0]] != '\\') continue;

				filename += dir->name_len[0] + 1;

				mmssdd(dir->extent, (char *)time);
				READDIR(ddir);
				i = 0;
				mdir = ddir;
			}
		} else {
			if (!strnicmp((char *)&dir->name[0], filename, strlen(filename))) {
				mmssdd(dir->extent, (char *)time);
				retval = 0;
				break;
			}
		}
	}
	return retval;
}

static const unsigned int gpu_ctl_def[] = {
	0x00000000, 0x01000000, 0x03000000, 0x04000000,
	0x05000800, 0x06c60260, 0x0703fc10, 0x08000027,
};

static const unsigned int gpu_data_def[] = {
	0xe100360b, 0xe2000000, 0xe3000800, 0xe4077e7f,
	0xe5001000, 0xe6000000,
	0x02000000, 0x00000000, 0x01ff03ff,
};

static void fake_bios_gpu_setup(void)
{
	int i;

	for (i = 0; i < sizeof(gpu_ctl_def) / sizeof(gpu_ctl_def[0]); i++)
		GPU_writeStatus(gpu_ctl_def[i]);

	for (i = 0; i < sizeof(gpu_data_def) / sizeof(gpu_data_def[0]); i++)
		GPU_writeData(gpu_data_def[i]);
}

int LoadCdrom() {
	EXE_HEADER tmpHead;
	struct iso_directory_record *dir;
	u8 time[4], *buf;
	u8 mdir[4096];

    if (!swapIso) {
        // not the best place to do it, but since BIOS boot logo killer
	    // is just below, do it here
	    fake_bios_gpu_setup();
    } else {
        swapIso = false;
    }

	if (!Config.HLE) {
		if(!LoadCdBios)
  	  psxRegs.pc = psxRegs.GPR.n.ra;
    return 0;
	}

	time[0] = itob(0); time[1] = itob(2); time[2] = itob(0x10);

	READTRACK();

	// skip head and sub, and go to the root directory record
	dir = (struct iso_directory_record*) &buf[12+156];

	mmssdd(dir->extent, (char*)time);

	READDIR(mdir);

	// Load SYSTEM.CNF and scan for the main executable
	if (GetCdromFile(mdir, time, "SYSTEM.CNF;1") == -1) {
		// if SYSTEM.CNF is missing, start an existing PSX.EXE
		if (GetCdromFile(mdir, time, "PSX.EXE;1") == -1) return -1;

		READTRACK();
	}
	else {
		// read the SYSTEM.CNF
		READTRACK();
		char exename[256];

		sscanf((char *)buf + 12, "BOOT = cdrom:\\%255s", exename);
		if (GetCdromFile(mdir, time, exename) == -1) {
			sscanf((char *)buf + 12, "BOOT = cdrom:%255s", exename);
			if (GetCdromFile(mdir, time, exename) == -1) {
				char *ptr = strstr((char *)buf + 12, "cdrom:");
				if (ptr != NULL) {
					ptr += 6;
					while (*ptr == '\\' || *ptr == '/') ptr++;
					strncpy(exename, ptr, 255);
					exename[255] = '\0';
					ptr = exename;
					while (*ptr != '\0' && *ptr != '\r' && *ptr != '\n') ptr++;
					*ptr = '\0';
					if (GetCdromFile(mdir, time, exename) == -1)
						return -1;
				} else
					return -1;
			}
		}

		// Read the EXE-Header
		READTRACK();
	}

	memcpy(&tmpHead, buf + 12, sizeof(EXE_HEADER));

	psxRegs.pc = SWAP32(tmpHead.pc0);
	psxRegs.GPR.n.gp = SWAP32(tmpHead.gp0);
	psxRegs.GPR.n.sp = SWAP32(tmpHead.s_addr);
	if (psxRegs.GPR.n.sp == 0) psxRegs.GPR.n.sp = 0x801fff00;

	tmpHead.t_size = SWAP32(tmpHead.t_size);
	tmpHead.t_addr = SWAP32(tmpHead.t_addr);

	psxCpu->Clear(tmpHead.t_addr, tmpHead.t_size / 4);

	// Read the rest of the main executable
	while (tmpHead.t_size & ~2047) {
		void *ptr = (void *)PSXM(tmpHead.t_addr);

		incTime();
		READTRACK();

		if (ptr != NULL) memcpy(ptr, buf+12, 2048);

		tmpHead.t_size -= 2048;
		tmpHead.t_addr += 2048;
	}

	return 0;
}

int LoadCdromFile(char *filename, EXE_HEADER *head) {
	struct iso_directory_record *dir;
	u8 time[4],*buf;
	u8 mdir[4096];
	char exename[256];
	u32 size, addr;
	void *mem;

	sscanf(filename, "cdrom:\\%255s", exename);

	time[0] = itob(0); time[1] = itob(2); time[2] = itob(0x10);

	READTRACK();

	// skip head and sub, and go to the root directory record
	dir = (struct iso_directory_record *)&buf[12 + 156];

	mmssdd(dir->extent, (char*)time);

	READDIR(mdir);

	if (GetCdromFile(mdir, time, exename) == -1) return -1;

	READTRACK();

	memcpy(head, buf + 12, sizeof(EXE_HEADER));
	size = head->t_size;
	addr = head->t_addr;

	psxCpu->Clear(addr, size / 4);

	while (size & ~2047) {
		incTime();
		READTRACK();

		mem = PSXM(addr);
		if (mem)
			memcpy(mem, buf + 12, 2048);

		size -= 2048;
		addr += 2048;
	}

	return 0;
}

int CheckCdrom() {
	struct iso_directory_record *dir;
	unsigned char time[4];
	char *buf;
	unsigned char mdir[4096];
	char exename[256];
	int i, len, c;

	FreePPFCache();

	time[0] = itob(0);
	time[1] = itob(2);
	time[2] = itob(0x10);

	READTRACK();

	memset(CdromLabel, 0, sizeof(CdromLabel));
	memset(CdromId, 0, sizeof(CdromId));
	memset(exename, 0, sizeof(exename));

	strncpy(CdromLabel, buf + 52, 32);

	// skip head and sub, and go to the root directory record
	dir = (struct iso_directory_record *)&buf[12 + 156];

	mmssdd(dir->extent, (char *)time);

	READDIR(mdir);

	if (GetCdromFile(mdir, time, "SYSTEM.CNF;1") != -1) {
		READTRACK();

		sscanf(buf + 12, "BOOT = cdrom:\\%255s", exename);
		if (GetCdromFile(mdir, time, exename) == -1) {
			sscanf(buf + 12, "BOOT = cdrom:%255s", exename);
			if (GetCdromFile(mdir, time, exename) == -1) {
				char *ptr = strstr(buf + 12, "cdrom:");			// possibly the executable is in some subdir
				if (ptr != NULL) {
					ptr += 6;
					while (*ptr == '\\' || *ptr == '/') ptr++;
					strncpy(exename, ptr, 255);
					exename[255] = '\0';
					ptr = exename;
					while (*ptr != '\0' && *ptr != '\r' && *ptr != '\n') ptr++;
					*ptr = '\0';
					if (GetCdromFile(mdir, time, exename) == -1)
					 	return -1;		// main executable not found
				} else
					return -1;
			}
		}
		/* Workaround for Wild Arms EU/US which has non-standard string causing incorrect region detection */
		if (exename[0] == 'E' && exename[1] == 'X' && exename[2] == 'E' && exename[3] == '\\') {
			size_t offset = 4;
			size_t i, len = strlen(exename) - offset;
			for (i = 0; i < len; i++)
				exename[i] = exename[i + offset];
			exename[i] = '\0';
		}
	} else if (GetCdromFile(mdir, time, "PSX.EXE;1") != -1) {
		strcpy(exename, "PSX.EXE;1");
		strcpy(CdromId, "SLUS99999");
	} else
		return -1;		// SYSTEM.CNF and PSX.EXE not found

	if (CdromId[0] == '\0') {
		len = strlen(exename);
		c = 0;
		for (i = 0; i < len; ++i) {
			if (exename[i] == ';' || c >= sizeof(CdromId) - 1)
				break;
			if (isalnum(exename[i]))
				CdromId[c++] = exename[i];
		}
	}

	if (CdromId[0] == '\0')
		strcpy(CdromId, "SLUS99999");

	if (Config.PsxAuto) { // autodetect system (pal or ntsc)
		if (
			/* Make sure Wild Arms SCUS-94608 is not detected as a PAL game. */
			((CdromId[0] == 's' || CdromId[0] == 'S') && (CdromId[2] == 'e' || CdromId[2] == 'E')) ||
			!strncmp(CdromId, "DTLS3035", 8) ||
			!strncmp(CdromId, "PBPX95001", 9) || // according to redump.org, these PAL
			!strncmp(CdromId, "PBPX95007", 9) || // discs have a non-standard ID;
			!strncmp(CdromId, "PBPX95008", 9))   // add more serials if they are discovered.
			Config.PsxType = PSX_TYPE_PAL; // pal
		else Config.PsxType = PSX_TYPE_NTSC; // ntsc
	}
	psxUpdateVSyncRate();
	if (CdromLabel[0] == ' ') {
		memcpy(CdromLabel, CdromId, 9);
	}
	SysPrintf(_("CD-ROM Label: %.32s\n"), CdromLabel);
	SysPrintf(_("CD-ROM ID: %.9s\n"), CdromId);
  for(i = 32; i>0; i--) {
    if(CdromLabel[i]==' ') {
      CdromLabel[i]=0;
    }
  }
  for(i = 9; i>0; i--) {
    if(CdromId[i]==' ') {
      CdromId[i]=0;
    }
  }
    // xjsxjs197 mem too small
	// BuildPPFCache();

	return 0;
}

static int PSXGetFileType(fileBrowser_file *f) {
    unsigned long current;
    u32 mybuf[2048];
    EXE_HEADER *exe_hdr;
    FILHDR *coff_hdr;

    current = f->offset;
    isoFile_seekFile(f, 0, FILE_BROWSER_SEEK_SET);
    isoFile_readFile(f, mybuf, 2048);
    isoFile_seekFile(f, current, FILE_BROWSER_SEEK_SET);

    exe_hdr = (EXE_HEADER *)mybuf;
    if (memcmp(exe_hdr->id,"PS-X EXE",8)==0)
        return PSX_EXE;

    if (mybuf[0]=='C' && mybuf[1]=='P' && mybuf[2]=='E')
        return CPE_EXE;

    coff_hdr = (FILHDR*)mybuf;
    if (coff_hdr->f_magic == 0x0162)
        return COFF_EXE;

    return INVALID_EXE;
}

/* TODO Error handling - return integer for each error case below, defined in an enum. Pass variable on return */
int Load(fileBrowser_file *exe) {

	EXE_HEADER tmpHead;
	int temp;
	int retval = 0;

	strncpy(CdromId, "Homebrew\0", 9);
	strncpy(CdromLabel, "Demo\0",  5);

	if (isoFile_readFile(exe, &temp, 4) != 4) {
		SysMessage(_("Error opening file: %s"), exe->name);
		retval = 0;
	} else {
  	exe->offset = 0;  //reset the offset back to 0
		int type = PSXGetFileType(exe);
		switch (type) {
			case PSX_EXE:
				isoFile_readFile(exe, &tmpHead, sizeof(EXE_HEADER));
				isoFile_seekFile(exe, 0x800, FILE_BROWSER_SEEK_SET);
				isoFile_readFile(exe, (void *)PSXM(SWAP32(tmpHead.t_addr)), SWAP32(tmpHead.t_size));
				psxRegs.pc = SWAP32(tmpHead.pc0);
				psxRegs.GPR.n.gp = SWAP32(tmpHead.gp0);
				psxRegs.GPR.n.sp = SWAP32(tmpHead.s_addr);
				if (psxRegs.GPR.n.sp == 0)
					psxRegs.GPR.n.sp = 0x801fff00;
				retval = 0;
				break;
			case CPE_EXE:
				SysMessage(_("CPE files not supported."));
				retval = -1;
				break;
			case COFF_EXE:
				SysMessage(_("COFF files not supported."));
				retval = -1;
				break;
			case INVALID_EXE:
				SysMessage(_("This file does not appear to be a valid PSX file."));
				retval = -1;
				break;
		}
	}
	return retval;
}

// STATES
void LoadingBar_showBar(float percent, const char* string);
const char PcsxHeader[32] = "STv3 PCSX v";
char* statespath = "/wiisxrx/savestates/";
static unsigned int savestates_slot = 0;
extern unsigned char  *psxVub;
// upd xjsxjs197 start
//extern unsigned short  spuMem[256*1024];
extern unsigned char  spuMem[512 * 1024];
// upd xjsxjs197 end
#define iGPUHeight 512
#define SAVE_STATE_MSG "Saving State .."
#define LOAD_STATE_MSG "Loading State .."

void savestates_select_slot(unsigned int s)
{
   if (s > 9) {
     return;
   }
   savestates_slot = s;
}

int SaveState() {

  gzFile f;
  GPUFreeze_t *gpufP;
	SPUFreeze_t *spufP;
	int Size;
	unsigned char *pMem;
	char *filename;

  /* fix the filename to %s.st%u format */
	filename = malloc(1024);

#ifdef HW_RVL
  sprintf(filename, "%s%s%s.st%u",(saveStateDevice==SAVESTATEDEVICE_USB)?"usb:":"sd:",
                           statespath, CdromId, savestates_slot);
#else
  sprintf(filename, "sd:%s%s.st%u", statespath, CdromId, savestates_slot);
#endif

	f = gzopen(filename, "wb");
  free(filename);

  if(!f) {
  	return 0;
	}

  LoadingBar_showBar(0.0f, SAVE_STATE_MSG);
  pauseRemovalThread();
  GPU_updateLace();

	gzwrite(f, (void*)PcsxHeader, 32);

	pMem = (unsigned char *) malloc(128*96*3);
	if (pMem == NULL) return -1;
	GPU_getScreenPic(pMem);
	gzwrite(f, pMem, 128*96*3);
	free(pMem);

	if (Config.HLE) {
		psxBiosFreeze(1);
	}
  LoadingBar_showBar(0.10f, SAVE_STATE_MSG);
	gzwrite(f, psxM, 0x00200000);
	LoadingBar_showBar(0.40f, SAVE_STATE_MSG);
	gzwrite(f, psxR, 0x00080000);
	LoadingBar_showBar(0.60f, SAVE_STATE_MSG);
	gzwrite(f, psxH, 0x00010000);
	gzwrite(f, (void*)&psxRegs, sizeof(psxRegs));
  LoadingBar_showBar(0.70f, SAVE_STATE_MSG);
	// gpu
	gpufP = (GPUFreeze_t *) malloc(sizeof(GPUFreeze_t));
	gpufP->ulFreezeVersion = 1;
	GPU_freeze(1, gpufP);
	gzwrite(f, gpufP, sizeof(GPUFreeze_t));
	free(gpufP);
	// gpu VRAM save (save directly to save memory)
	gzwrite(f, &psxVub[0], 1024*iGPUHeight*2);
  LoadingBar_showBar(0.80f, SAVE_STATE_MSG);
	// spu
	spufP = (SPUFreeze_t *) malloc(16);
	SPU_freeze(2, spufP, psxRegs.cycle);
	Size = spufP->ulFreezeSize; gzwrite(f, &Size, 4);
	free(spufP);
	spufP = (SPUFreeze_t *) malloc(Size);
	SPU_freeze(1, spufP, psxRegs.cycle);
	gzwrite(f, spufP, Size);
	free(spufP);
  // spu spuMem save (save directly to save memory)
  //gzwrite(f, spu.spuMemC, 0x80000);
  LoadingBar_showBar(0.90f, SAVE_STATE_MSG);

	sioFreeze(f, 1);
	cdrFreeze(f, 1);
	psxHwFreeze(f, 1);
	psxRcntFreeze(f, 1);
	mdecFreeze(f, 1);
  LoadingBar_showBar(0.99f, SAVE_STATE_MSG);
	gzclose(f);

	continueRemovalThread();
  LoadingBar_showBar(1.0f, SAVE_STATE_MSG);
	return 1; //ok
}

int LoadState() {
	gzFile f;
	GPUFreeze_t *gpufP;
	SPUFreeze_t *spufP;
	int Size;
	char header[32];
	char *filename;

  /* fix the filename to %s.st%u format */
	filename = malloc(1024);
#ifdef HW_RVL
  sprintf(filename, "%s%s%s.st%u",(saveStateDevice==SAVESTATEDEVICE_USB)?"usb:":"sd:",
                           statespath, CdromId, savestates_slot);
#else
  sprintf(filename, "sd:%s%s.st%u", statespath, CdromId, savestates_slot);
#endif

	f = gzopen(filename, "rb");
  free(filename);

  if(!f) {
  	return 0;
	}

	pauseRemovalThread();
	LoadingBar_showBar(0.0f, LOAD_STATE_MSG);
	//SysReset();

	psxCpu->Reset();
  LoadingBar_showBar(0.10f, LOAD_STATE_MSG);
	gzread(f, header, 32);

	if (strncmp("STv3 PCSX", header, 9)) { gzclose(f); return -1; }

	gzseek(f, 128*96*3, SEEK_CUR);

	gzread(f, psxM, 0x00200000);
	LoadingBar_showBar(0.40f, LOAD_STATE_MSG);
	gzread(f, psxR, 0x00080000);
	LoadingBar_showBar(0.60f, LOAD_STATE_MSG);
	gzread(f, psxH, 0x00010000);
	gzread(f, (void*)&psxRegs, sizeof(psxRegs));
  LoadingBar_showBar(0.70f, LOAD_STATE_MSG);
	if (Config.HLE)
		psxBiosFreeze(0);

	// gpu
	gpufP = (GPUFreeze_t *) malloc (sizeof(GPUFreeze_t));
	gzread(f, gpufP, sizeof(GPUFreeze_t));
	GPU_freeze(0, gpufP);
	free(gpufP);
	// gpu VRAM load (load directly to save memory)
	gzread(f, &psxVub[0], 1024*iGPUHeight*2);
	LoadingBar_showBar(0.80f, LOAD_STATE_MSG);

	// spu
	gzread(f, &Size, 4);
	spufP = (SPUFreeze_t *) malloc (Size);
	gzread(f, spufP, Size);
	SPU_freeze(0, spufP, psxRegs.cycle);
	free(spufP);
  // spu spuMem save (save directly to save memory)
  //gzread(f, spu.spuMemC, 0x80000);
  LoadingBar_showBar(0.99f, LOAD_STATE_MSG);

	sioFreeze(f, 0);
	cdrFreeze(f, 0);
	psxHwFreeze(f, 0);
	psxRcntFreeze(f, 0);
	mdecFreeze(f, 0);

	gzclose(f);
  continueRemovalThread();
  LoadingBar_showBar(1.0f, LOAD_STATE_MSG);

	return 1;
}

int CheckState(char *file) {
	gzFile f;
	char header[32];

	f = gzopen(file, "rb");
	if (f == NULL) return -1;

	psxCpu->Reset();

	gzread(f, header, 32);

	gzclose(f);

	if (strncmp("STv3 PCSX", header, 9)) return -1;

	return 0;
}

// NET Function Helpers

int SendPcsxInfo() {
	if (NET_recvData == NULL || NET_sendData == NULL)
		return 0;

//	SysPrintf("SendPcsxInfo\n");

	NET_sendData(&Config.Xa, sizeof(Config.Xa), PSE_NET_BLOCKING);
	NET_sendData(&Config.Sio, sizeof(Config.Sio), PSE_NET_BLOCKING);
	NET_sendData(&Config.SpuIrq, sizeof(Config.SpuIrq), PSE_NET_BLOCKING);
	NET_sendData(&Config.RCntFix, sizeof(Config.RCntFix), PSE_NET_BLOCKING);
	NET_sendData(&Config.PsxType, sizeof(Config.PsxType), PSE_NET_BLOCKING);
	NET_sendData(&Config.Cpu, sizeof(Config.Cpu), PSE_NET_BLOCKING);

//	SysPrintf("Send OK\n");

	return 0;
}

int RecvPcsxInfo() {
	int tmp;

	if (NET_recvData == NULL || NET_sendData == NULL)
		return 0;

//	SysPrintf("RecvPcsxInfo\n");

	NET_recvData(&Config.Xa, sizeof(Config.Xa), PSE_NET_BLOCKING);
	NET_recvData(&Config.Sio, sizeof(Config.Sio), PSE_NET_BLOCKING);
	NET_recvData(&Config.SpuIrq, sizeof(Config.SpuIrq), PSE_NET_BLOCKING);
	NET_recvData(&Config.RCntFix, sizeof(Config.RCntFix), PSE_NET_BLOCKING);
	NET_recvData(&Config.PsxType, sizeof(Config.PsxType), PSE_NET_BLOCKING);
	psxUpdateVSyncRate();

	SysUpdate();

	tmp = Config.Cpu;
	NET_recvData(&Config.Cpu, sizeof(Config.Cpu), PSE_NET_BLOCKING);
	if (tmp != Config.Cpu) {
		psxCpu->Shutdown();
#ifdef PSXREC
		if (Config.Cpu)
			 psxCpu = &psxInt;
		else psxCpu = &psxRec;
#else
		psxCpu = &psxInt;
#endif
		if (psxCpu->Init() == -1) {
			SysClose(); return -1;
		}
		psxCpu->Reset();
	}

//	SysPrintf("Recv OK\n");

	return 0;
}


void __Log(char *fmt, ...) {
	va_list list;
#ifdef LOG_STDOUT
	char tmp[1024];
#endif

	va_start(list, fmt);
#ifndef LOG_STDOUT
#if defined (CPU_LOG) || defined(DMA_LOG) || defined(CDR_LOG) || defined(HW_LOG) || \
	defined(PSXBIOS_LOG) || defined(GTE_LOG) || defined(PAD_LOG)
	vfprintf(emuLog, fmt, list);
#endif
#else
	vsprintf(tmp, fmt, list);
	SysPrintf(tmp);
#endif
	va_end(list);
}

// lookup table for crc calculation
static unsigned short crctab[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108,
	0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
	0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B,
	0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
	0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE,
	0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
	0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D,
	0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5,
	0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
	0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
	0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
	0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13,
	0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
	0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E,
	0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1,
	0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
	0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0,
	0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
	0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657,
	0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
	0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882,
	0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E,
	0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
	0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D,
	0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
	0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

u16 calcCrc(u8 *d, int len) {
	u16 crc = 0;
	int i;

	for (i = 0; i < len; i++) {
		crc = crctab[(crc >> 8) ^ d[i]] ^ (crc << 8);
	}

	return (u16)(~crc);
}

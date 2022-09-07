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

/*
* SIO functions.
*/

#include "sio.h"
#include "Gamecube/fileBrowser/fileBrowser.h"
#include "Gamecube/fileBrowser/fileBrowser-libfat.h"
#include <sys/stat.h>

// *** FOR WORKS ON PADS AND MEMORY CARDS *****

static unsigned char buf[256];
unsigned char cardh[4] = { 0x00, 0x00, 0x5a, 0x5d };

//static unsigned short StatReg = 0x002b;
// Transfer Ready and the Buffer is Empty
unsigned short StatReg = TX_RDY | TX_EMPTY;
unsigned short ModeReg;
unsigned short CtrlReg;
unsigned short BaudReg;

static unsigned int bufcount;
static unsigned int parp;
static unsigned int mcdst,rdwr;
static unsigned char adrH,adrL;
static unsigned int padst;

char mcd1Written = 0;
char mcd2Written = 0;

PadDataS pad;

#ifdef HW_RVL
#include "Gamecube/MEM2.h"
char *Mcd1Data = (char*)MCD1_LO;
char *Mcd2Data = (char*)MCD2_LO;
#else
char Mcd1Data[MCD_SIZE], Mcd2Data[MCD_SIZE];
#endif

// clk cycle byte
// 4us * 8bits = ((PSXCLK / 1000000) * 32) / BIAS; (linuzappz)
#define SIO_INT() { \
	if (!Config.Sio) { \
		psxRegs.interrupt |= (1 << PSXINT_SIO); \
		psxRegs.intCycle[PSXINT_SIO].cycle = 200; \
		psxRegs.intCycle[PSXINT_SIO].sCycle = psxRegs.cycle; \
	} \
}

unsigned char sioRead8() {
	unsigned char ret = 0;

	if ((StatReg & RX_RDY)/* && (CtrlReg & RX_PERM)*/) {
//		StatReg &= ~RX_OVERRUN;
		ret = buf[parp];
		if (parp == bufcount) {
			StatReg &= ~RX_RDY;		// Receive is not Ready now
			if (mcdst == 5) {
				mcdst = 0;
				if (rdwr == 2) {
					switch (CtrlReg&0x2002) {
						case 0x0002:
							memcpy(Mcd1Data + (adrL | (adrH << 8)) * 128, &buf[1], 128);
							mcd1Written = 1;
							break;
						case 0x2002:
							memcpy(Mcd2Data + (adrL | (adrH << 8)) * 128, &buf[1], 128);
							mcd2Written = 1;
							break;
					}
				}
			}
			if (padst == 2) padst = 0;
			if (mcdst == 1) {
				mcdst = 2;
				StatReg|= RX_RDY;
			}
		}
	}

#ifdef PAD_LOG
	PAD_LOG("sio read8 ;ret = %x\n", ret);
#endif
	return ret;
}

void netError() {
	ClosePlugins();
	SysMessage(_("Connection closed!\n"));
	SysRunGui();
}

void sioWrite8(unsigned char value) {
#ifdef PAD_LOG
	PAD_LOG("sio write8 %x\n", value);
#endif
	switch (padst) {
		case 1: SIO_INT();
			if ((value&0x40) == 0x40) {
				padst = 2; parp = 1;
				if (!Config.UseNet) {
					switch (CtrlReg&0x2002) {
						case 0x0002:
							buf[parp] = PAD1_poll(value);
							break;
						case 0x2002:
							buf[parp] = PAD2_poll(value);
							break;
					}
				}/* else {
//					SysPrintf("%x: %x, %x, %x, %x\n", CtrlReg&0x2002, buf[2], buf[3], buf[4], buf[5]);
				}*/

				if (!(buf[parp] & 0x0f)) {
					bufcount = 2 + 32;
				} else {
					bufcount = 2 + (buf[parp] & 0x0f) * 2;
				}
				if (buf[parp] == 0x41) {
					switch (value) {
						case 0x43:
							buf[1] = 0x43;
							break;
						case 0x45:
							buf[1] = 0xf3;
							break;
					}
				}
			}
			else padst = 0;
			return;
		case 2:
			parp++;
/*			if (buf[1] == 0x45) {
				buf[parp] = 0;
				SIO_INT();
				return;
			}*/
			if (!Config.UseNet) {
				switch (CtrlReg&0x2002) {
					case 0x0002: buf[parp] = PAD1_poll(value); break;
					case 0x2002: buf[parp] = PAD2_poll(value); break;
				}
			}

			if (parp == bufcount) { padst = 0; return; }
			SIO_INT();
			return;
	}

    #ifdef DISP_DEBUG
    //PRINT_LOG3("sioWrite8====%d=%d=%x", mcdst, rdwr, CtrlReg);
    #endif // DISP_DEBUG*/
	switch (mcdst) {
		case 1:
			SIO_INT();
			if (rdwr) { parp++; return; }
			parp = 1;
			switch (value) {
				case 0x52: rdwr = 1; break;
				case 0x57: rdwr = 2; break;
				default: mcdst = 0;
			}
			return;
		case 2: // address H
			SIO_INT();
			adrH = value;
			*buf = 0;
			parp = 0;
			bufcount = 1;
			mcdst = 3;
			return;
		case 3: // address L
			SIO_INT();
			adrL = value;
			*buf = adrH;
			parp = 0;
			bufcount = 1;
			mcdst = 4;
			return;
		case 4:
			SIO_INT();
			parp = 0;
			switch (rdwr) {
				case 1: // read
					buf[0] = 0x5c;
					buf[1] = 0x5d;
					buf[2] = adrH;
					buf[3] = adrL;
					switch (CtrlReg&0x2002) {
						case 0x0002:
							memcpy(&buf[4], Mcd1Data + (adrL | (adrH << 8)) * 128, 128);
							mcd1Written = 1;
							break;
						case 0x2002:
							memcpy(&buf[4], Mcd2Data + (adrL | (adrH << 8)) * 128, 128);
							mcd2Written = 1;
							break;
					}
					{
					char xor = 0;
					int i;
					for (i=2;i<128+4;i++)
						xor^=buf[i];
					buf[132] = xor;
					}
					buf[133] = 0x47;
					bufcount = 133;
					break;
				case 2: // write
					buf[0] = adrL;
					buf[1] = value;
					buf[129] = 0x5c;
					buf[130] = 0x5d;
					buf[131] = 0x47;
					bufcount = 131;
					break;
			}
			mcdst = 5;
			return;
		case 5:
			parp++;
			if (rdwr == 2) {
				if (parp < 128) buf[parp+1] = value;
			}
			SIO_INT();
			return;
	}

	switch (value) {
		case 0x01: // start pad
			StatReg |= RX_RDY;		// Transfer is Ready

			if (!Config.UseNet) {
				switch (CtrlReg&0x2002) {
					case 0x0002: buf[0] = PAD1_startPoll(1); break;
					case 0x2002: buf[0] = PAD2_startPoll(2); break;
				}
			} else {
				if ((CtrlReg & 0x2002) == 0x0002) {
					int i, j;

					PAD1_startPoll(1);
					buf[0] = 0;
					buf[1] = PAD1_poll(0x42);
					if (!(buf[1] & 0x0f)) {
						bufcount = 32;
					} else {
						bufcount = (buf[1] & 0x0f) * 2;
					}
					buf[2] = PAD1_poll(0);
					i = 3;
					j = bufcount;
					while (j--) {
						buf[i++] = PAD1_poll(0);
					}
					bufcount+= 3;

					if (NET_sendPadData(buf, bufcount) == -1)
						netError();

					if (NET_recvPadData(buf, 1) == -1)
						netError();
					if (NET_recvPadData(buf+128, 2) == -1)
						netError();
				} else {
					memcpy(buf, buf+128, 32);
				}
			}

			bufcount = 2;
			parp = 0;
			padst = 1;
			SIO_INT();
			return;
		case 0x81: // start memcard
			StatReg |= RX_RDY;
			memcpy(buf, cardh, 4);
			parp = 0;
			bufcount = 3;
			mcdst = 1;
			rdwr = 0;
			SIO_INT();
			return;
	}
}

void sioWriteCtrl16(unsigned short value) {
	CtrlReg = value & ~RESET_ERR;
	if (value & RESET_ERR) StatReg &= ~IRQ;
	if ((CtrlReg & SIO_RESET) || !(CtrlReg & DTR)) {
		padst = 0; mcdst = 0; parp = 0;
		StatReg = TX_RDY | TX_EMPTY;
		psxRegs.interrupt&=~(1 << PSXINT_SIO);
	}
}

void sioInterrupt() {
#ifdef PAD_LOG
	PAD_LOG("Sio Interrupt (CP0.Status = %x)\n", psxRegs.CP0.n.Status);
#endif
//	SysPrintf("Sio Interrupt\n");
    if (!(StatReg & IRQ)) {
	    StatReg|= IRQ;
	    psxHu32ref(0x1070)|= SWAPu32(0x80);
	    psxRegs.interrupt|= 0x80000000;
    }
}

//call me from menu, takes slot and save path as args
int LoadMcd(int mcd, fileBrowser_file *savepath) {
	int temp = 0;
	bool ret = 0;
	char *data = NULL;
  fileBrowser_file saveFile;
	memcpy(&saveFile, savepath, sizeof(fileBrowser_file));
	memset(&saveFile.name[0],0,FILE_BROWSER_MAX_PATH_LEN);

	if(mcd == 1) {
	  sprintf((char*)saveFile.name,"%s/%s.mcd",savepath->name,CdromId);
	  data = &Mcd1Data[0];
  }
	if (mcd == 2) {
  	sprintf((char*)saveFile.name,"%s/slot2.mcd",savepath->name);
  	data = &Mcd2Data[0];
	}

	if(saveFile_readFile(&saveFile, &temp, 4) == 4) {  //file exists
		saveFile.offset = 0;
		if(saveFile_readFile(&saveFile, data, MCD_SIZE)==MCD_SIZE)
		  ret = 1;
	}
	else {
		if(CreateMcd(mcd, &saveFile)) {  //created ok
		  saveFile.offset = 0;
			if(saveFile_readFile(&saveFile, data, MCD_SIZE)==MCD_SIZE)
			  ret = 1;
		}
	}
	return ret;
}

//we need to get rid of this joint function and start using the individual versions
int LoadMcds(fileBrowser_file *mcd1, fileBrowser_file *mcd2) {
  if((LoadMcd(1, mcd1)) && (LoadMcd(2, mcd2)))
    return 1;
  return 0;
}

// add xjsxjs197 start
int SaveMcdByNum(int mcd) {
    if (saveFile_dir)
	{
	    return SaveMcd(mcd, saveFile_dir);
	}
    else
	{
	    return -1;
	}
}
// add xjsxjs197 end

//call me from menu, takes slot and save path as args
int SaveMcd(int mcd, fileBrowser_file *savepath) {
  bool ret = 0;
  char *data = NULL;
  fileBrowser_file saveFile;

	memcpy(&saveFile, savepath, sizeof(fileBrowser_file));
	memset(&saveFile.name[0],0,FILE_BROWSER_MAX_PATH_LEN);

	if(mcd == 1) {
	  sprintf((char*)saveFile.name,"%s/%s.mcd",savepath->name,CdromId);
	  data = &Mcd1Data[0];
  }
	if (mcd == 2) {
  	sprintf((char*)saveFile.name,"%s/slot2.mcd",savepath->name);
  	data = &Mcd2Data[0];
	}

  if(saveFile_writeFile(&saveFile, data, MCD_SIZE)==MCD_SIZE)
    ret = 1;

  return ret;
}

//we need to get rid of this joint function and start using the individual versions
int SaveMcds(fileBrowser_file *mcd1, fileBrowser_file *mcd2) {
  if((SaveMcd(1, mcd1)) && (SaveMcd(2, mcd2)))
    return 1;
  return 0;
}

bool CreateMcd(int slot, fileBrowser_file *mcd) {
	char *cardData;
	if (slot == 1) cardData = Mcd1Data;
	else /*(slot == 2)*/ cardData = Mcd2Data;

	int i=0, j=0, curPos =0;

	// setup header
	cardData[curPos++] = 'M';
	cardData[curPos++] = 'C';
	for(i=0; i<125; i++)
	  cardData[curPos++] = 0;
	cardData[curPos++] = 0x0E;

	// 15 blocks
	for(i=0;i<15;i++) {
		cardData[curPos++] = 0xA0;
		for(j=0;j<126;j++) {
			cardData[curPos++] = 0;
		}
		cardData[curPos++] = 0xA0;
	}

	//blank out the rest
	for(i = curPos; i < MCD_SIZE; i++)
	  cardData[i] = 0;
	if(saveFile_writeFile(mcd, cardData, MCD_SIZE)==MCD_SIZE)
	  return 1;
	return 0;
}

void ConvertMcd(char *mcd, char *data) {
	/*FILE *f;
	int i=0;
	int s = MCD_SIZE;

	if (strstr(mcd, ".gme")) {
		f = fopen(mcd, "wb");
		if (f != NULL) {
			fwrite(data-3904, 1, MCD_SIZE+3904, f);
			fclose(f);
		}
		f = fopen(mcd, "r+");
		s = s + 3904;
		fputc('1', f); s--;
		fputc('2', f); s--;
		fputc('3', f); s--;
		fputc('-', f); s--;
		fputc('4', f); s--;
		fputc('5', f); s--;
		fputc('6', f); s--;
		fputc('-', f); s--;
		fputc('S', f); s--;
		fputc('T', f); s--;
		fputc('D', f); s--;
		for(i=0;i<7;i++) {
			fputc(0, f); s--;
		}
		fputc(1, f); s--;
		fputc(0, f); s--;
		fputc(1, f); s--;
		fputc('M', f); s--;
		fputc('Q', f); s--;
		for(i=0;i<14;i++) {
			fputc(0xa0, f); s--;
		}
		fputc(0, f); s--;
		fputc(0xff, f);
		while (s-- > (MCD_SIZE+1)) fputc(0, f);
		fclose(f);
	} else if(strstr(mcd, ".mem") || strstr(mcd,".vgs")) {
		f = fopen(mcd, "wb");
		if (f != NULL) {
			fwrite(data-64, 1, MCD_SIZE+64, f);
			fclose(f);
		}
		f = fopen(mcd, "r+");
		s = s + 64;
		fputc('V', f); s--;
		fputc('g', f); s--;
		fputc('s', f); s--;
		fputc('M', f); s--;
		for(i=0;i<3;i++) {
			fputc(1, f); s--;
			fputc(0, f); s--;
			fputc(0, f); s--;
			fputc(0, f); s--;
		}
		fputc(0, f); s--;
		fputc(2, f);
		while (s-- > (MCD_SIZE+1)) fputc(0, f);
		fclose(f);
	} else {
		f = fopen(mcd, "wb");
		if (f != NULL) {
			fwrite(data, 1, MCD_SIZE, f);
			fclose(f);
		}
	}*/
}

//void GetMcdBlockInfo(int mcd, int block, McdBlock *Info) {
//	char *data = NULL, *ptr, *str;
//	unsigned short clut[16];
//	int i, x;
//
//	memset(Info, 0, sizeof(McdBlock));
//
//	str = Info->Title;
//
//	if (mcd == 1) data = Mcd1Data;
//	if (mcd == 2) data = Mcd2Data;
//
//	ptr = data + block * 8192 + 2;
//
//	Info->IconCount = *ptr & 0x3;
//
//	ptr+= 2;
//
//	i=0;
//	memcpy(Info->sTitle, ptr, 48*2);
//
//	for (i=0; i < 48; i++) {
//		unsigned short c = *(ptr) << 8;
//		c|= *(ptr+1);
//		if (!c) break;
//
//		// Convert ASCII characters to half-width
//		if (c >= 0x8281 && c <= 0x829A)
//			c = (c - 0x8281) + 'a';
//		else if (c >= 0x824F && c <= 0x827A)
//			c = (c - 0x824F) + '0';
//		else if (c == 0x8140) c = ' ';
//		else if (c == 0x8143) c = ',';
//		else if (c == 0x8144) c = '.';
//		else if (c == 0x8146) c = ':';
//		else if (c == 0x8147) c = ';';
//		else if (c == 0x8148) c = '?';
//		else if (c == 0x8149) c = '!';
//		else if (c == 0x815E) c = '/';
//		else if (c == 0x8168) c = '"';
//		else if (c == 0x8169) c = '(';
//		else if (c == 0x816A) c = ')';
//		else if (c == 0x816D) c = '[';
//		else if (c == 0x816E) c = ']';
//		else if (c == 0x817C) c = '-';
//		else {
//			c = ' ';
//		}
//
//		str[i] = c;
//		ptr+=2;
//	}
//	str[i] = 0;
//
//	ptr = data + block * 8192 + 0x60; // icon palete data
//
//	for (i=0; i<16; i++) {
//		clut[i] = *((unsigned short*)ptr);
//		ptr+=2;
//	}
//
//	for (i=0; i<Info->IconCount; i++) {
//		short *icon = &Info->Icon[i*16*16];
//
//		ptr = data + block * 8192 + 128 + 128 * i; // icon data
//
//		for (x=0; x<16*16; x++) {
//			icon[x++] = clut[*ptr & 0xf];
//			icon[x]   = clut[*ptr >> 4];
//			ptr++;
//		}
//	}
//
//	ptr = data + block * 128;
//
//	Info->Flags = *ptr;
//
//	ptr+= 0xa;
//	strncpy(Info->ID, ptr, 12);
//	Info->ID[12] = 0;
//	ptr+= 12;
//	strncpy(Info->Name, ptr, 16);
//}

int sioFreeze(gzFile f, int Mode) {
	char Unused[4096];

	gzfreezel(buf);
	gzfreezel(&StatReg);
	gzfreezel(&ModeReg);
	gzfreezel(&CtrlReg);
	gzfreezel(&BaudReg);
	gzfreezel(&bufcount);
	gzfreezel(&parp);
	gzfreezel(&mcdst);
	gzfreezel(&rdwr);
	gzfreezel(&adrH);
	gzfreezel(&adrL);
	gzfreezel(&padst);
	gzfreezel(Unused);

	return 0;
}

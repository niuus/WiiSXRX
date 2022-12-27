/**
 * WiiSX - MainFrame.cpp
 * Copyright (C) 2009 sepp256
 *
 * WiiSX homepage: http://www.emulatemii.com
 * email address: sepp256@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
**/

#include "MenuContext.h"
#include "MainFrame.h"
#include "SettingsFrame.h"
#include "../libgui/Button.h"
#include "../libgui/Gui.h"
#include "../libgui/InputStatusBar.h"
#include "../libgui/resources.h"
//#include "../libgui/InputManager.h"
#include "../libgui/FocusManager.h"
#include "../libgui/CursorManager.h"
#include "../libgui/MessageBox.h"
//#include "../main/wii64config.h"
#ifdef DEBUGON
# include <debug.h>
#endif
extern "C" {
#ifdef WII
#include <di/di.h>
#endif
/*#include "../gc_memory/memory.h"
#include "../gc_memory/Saves.h"
#include "../main/plugin.h"
#include "../main/savestates.h"*/
#include "../fileBrowser/fileBrowser.h"
#include "../fileBrowser/fileBrowser-libfat.h"
#include "../fileBrowser/fileBrowser-CARD.h"
#include "../fileBrowser/fileBrowser-SMB.h"
//#include "../main/gc_dvd.h"
}
#include <ogc/dvd.h>

void Func_LoadROM();
void Func_CurrentROM();
void Func_Settings();
void Func_Credits();
void Func_ExitToLoader();
void Func_PlayGame();

#define NUM_MAIN_BUTTONS 6
#define FRAME_BUTTONS mainFrameButtons
#define FRAME_STRINGS mainFrameStrings

char FRAME_STRINGS[7][20] =
	{ "Load CD",
	  "Current CD",
	  "Settings",
	  "Credits",
	  "Quit",
	  "Play Game",
	  "Resume Game"};


struct ButtonInfo
{
	menu::Button	*button;
	int				buttonStyle;
	char*			buttonString;
	float			x;
	float			y;
	float			width;
	float			height;
	int				focusUp;
	int				focusDown;
	int				focusLeft;
	int				focusRight;
	ButtonFunc		clickedFunc;
	ButtonFunc		returnFunc;
} FRAME_BUTTONS[NUM_MAIN_BUTTONS] =
{ //	button	buttonStyle	buttonString		x		y		width	height	Up	Dwn	Lft	Rt	clickFunc				returnFunc
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[0],	315.0,	 60.0,	200.0,	56.0,	 5,	 1,	-1,	-1,	Func_LoadROM,			NULL }, // Load ROM
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[1],	315.0,	120.0,	200.0,	56.0,	 0,	 2,	-1,	-1,	Func_CurrentROM,		NULL }, // Current ROM
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[2],	315.0,	180.0,	200.0,	56.0,	 1,	 3,	-1,	-1,	Func_Settings,			NULL }, // Settings
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[3],	315.0,	240.0,	200.0,	56.0,	 2,	 4,	-1,	-1,	Func_Credits,			NULL }, // Credits
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[4],	315.0,	300.0,	200.0,	56.0,	 3,	 5,	-1,	-1,	Func_ExitToLoader,		NULL }, // Exit to Loader
	{	NULL,	BTN_A_NRM,	FRAME_STRINGS[5],	315.0,	360.0,	200.0,	56.0,	 4,	 0,	-1,	-1,	Func_PlayGame,			NULL }, // Play/Resume Game
};

MainFrame::MainFrame()
{
	inputStatusBar = new menu::InputStatusBar(450,100);
	add(inputStatusBar);

	for (int i = 0; i < NUM_MAIN_BUTTONS; i++)
		FRAME_BUTTONS[i].button = new menu::Button(FRAME_BUTTONS[i].buttonStyle, &FRAME_BUTTONS[i].buttonString,
										FRAME_BUTTONS[i].x, FRAME_BUTTONS[i].y,
										FRAME_BUTTONS[i].width, FRAME_BUTTONS[i].height);

	for (int i = 0; i < NUM_MAIN_BUTTONS; i++)
	{
		if (FRAME_BUTTONS[i].focusUp != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_UP, FRAME_BUTTONS[FRAME_BUTTONS[i].focusUp].button);
		if (FRAME_BUTTONS[i].focusDown != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_DOWN, FRAME_BUTTONS[FRAME_BUTTONS[i].focusDown].button);
		if (FRAME_BUTTONS[i].focusLeft != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_LEFT, FRAME_BUTTONS[FRAME_BUTTONS[i].focusLeft].button);
		if (FRAME_BUTTONS[i].focusRight != -1) FRAME_BUTTONS[i].button->setNextFocus(menu::Focus::DIRECTION_RIGHT, FRAME_BUTTONS[FRAME_BUTTONS[i].focusRight].button);
		FRAME_BUTTONS[i].button->setActive(true);
		if (FRAME_BUTTONS[i].clickedFunc) FRAME_BUTTONS[i].button->setClicked(FRAME_BUTTONS[i].clickedFunc);
		if (FRAME_BUTTONS[i].returnFunc) FRAME_BUTTONS[i].button->setReturn(FRAME_BUTTONS[i].returnFunc);
		add(FRAME_BUTTONS[i].button);
		menu::Cursor::getInstance().addComponent(this, FRAME_BUTTONS[i].button, FRAME_BUTTONS[i].x,
												FRAME_BUTTONS[i].x+FRAME_BUTTONS[i].width, FRAME_BUTTONS[i].y,
												FRAME_BUTTONS[i].y+FRAME_BUTTONS[i].height);
	}
	setDefaultFocus(FRAME_BUTTONS[0].button);
	setEnabled(true);

}

MainFrame::~MainFrame()
{
	for (int i = 0; i < NUM_MAIN_BUTTONS; i++)
	{
		menu::Cursor::getInstance().removeComponent(this, FRAME_BUTTONS[i].button);
		delete FRAME_BUTTONS[i].button;
	}
	delete inputStatusBar;
}

extern MenuContext *pMenuContext;

void Func_LoadROM()
{
	pMenuContext->setActiveFrame(MenuContext::FRAME_LOADROM,FileBrowserFrame::FILEBROWSER_LOADISO);
}

extern BOOL hasLoadedISO;

void Func_CurrentROM()
{
	if(!hasLoadedISO)
	{
		menu::MessageBox::getInstance().setMessage("Please load a CD first");
		return;
	}

	pMenuContext->setActiveFrame(MenuContext::FRAME_CURRENTROM);
}

void Func_Settings()
{
	menu::Gui::getInstance().menuLogo->setLocation(580.0, 410.0, -50.0);
	pMenuContext->setActiveFrame(MenuContext::FRAME_SETTINGS,SettingsFrame::SUBMENU_GENERAL);
}

void Func_Credits()
{
	char CreditsInfo[512] = "";
#ifdef HW_RVL
	sprintf(CreditsInfo,"WiiSX RX Beta 3.3-FS\n");
#else
	sprintf(CreditsInfo,"CubeSX RX Beta 3.3-FS\n");
#endif
	strcat(CreditsInfo,"www.github.com/niuus/WiiSXRX\n");
	strcat(CreditsInfo,"WiiSX RX & logo: NiuuS\n");
	strcat(CreditsInfo,"\n");
	strcat(CreditsInfo,"Original WiiSX team:\n");
	strcat(CreditsInfo,"emu_kidid - general coding\n");
	strcat(CreditsInfo,"sepp256 - graphics & menu\n");
	strcat(CreditsInfo,"tehpola - audio\n");
	strcat(CreditsInfo,"PCSX/-df/-r/-Revolution/ReARMed\n");
	strcat(CreditsInfo,"\n");

	char wiiDetails[30];
	char wiiInfo[20];
#ifdef HW_RVL
	if(!IsWiiU()) {
		sprintf(wiiInfo, "Wii (729 MHz)");
	}
	else if(IsWiiUFastCPU()) {
		sprintf(wiiInfo, "vWii (1.215 GHz)");
	}
	else {
		sprintf(wiiInfo, "vWii (729 MHz)");
	}
	sprintf(wiiDetails, "IOS: %d / %s", IOS_GetVersion(), wiiInfo);
#endif
#ifdef HW_RVL
	strcat(CreditsInfo,"FIX94 - Wii U gamepad support\n");
	strcat(CreditsInfo,"matguitarist - USB 2.0 support\n");
	strcat(CreditsInfo,"Daxtsu - libwupc support\n");
	strcat(CreditsInfo,"Mystro256 (WiiSXR) xjsxjs197 (WiiStation)\n");
	strcat(CreditsInfo,"\n");
	strcat(CreditsInfo, wiiDetails);
#endif

	menu::MessageBox::getInstance().setMessage(CreditsInfo);
}

extern char shutdown;
extern "C" void SysClose();

void Func_ExitToLoader()
{
	if (menu::MessageBox::getInstance().askMessage("Are you sure you want to exit to loader?"))
    {
        shutdown = 2;
        if (hasLoadedISO)
        {
            SysClose();
        }
    }
}

extern "C" {
void cpu_init();
void cpu_deinit();
}

extern "C" {
extern int SaveMcd(int mcd, fileBrowser_file *savepath);
void pauseAudio(void);  void pauseInput(void);
void resumeAudio(void); void resumeInput(void);
void go(void);
}

//void control_info_init();

extern char menuActive;
extern char autoSave;
extern "C" char mcd1Written;
extern "C" char mcd2Written;
extern "C" unsigned int usleep(unsigned int us);

void Func_PlayGame()
{
	if(!hasLoadedISO)
	{
		menu::MessageBox::getInstance().setMessage("Please load a CD first");
		return;
	}

	//Wait until 'A' button released before play/resume game
	menu::Cursor::getInstance().setFreezeAction(true);
	menu::Focus::getInstance().setFreezeAction(true);
	int buttonHeld = 1;
	while(buttonHeld)
	{
		buttonHeld = 0;
		menu::Gui::getInstance().draw();
		for (int i=0; i<4; i++)
		{
			if(PAD_ButtonsHeld(i) & PAD_BUTTON_A) buttonHeld++;
#ifdef HW_RVL
			WPADData* wiiPad = WPAD_Data(i);
			if(wiiPad->err == WPAD_ERR_NONE && wiiPad->btns_h & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) buttonHeld++;

			WUPCData* data = WUPC_Data(i);
			if (data != NULL && data->button & WPAD_CLASSIC_BUTTON_A) buttonHeld++;

			if(i == 0)
			{
				const WiiDRCData* data = WiiDRC_Data();
				if (data != NULL && data->button & WIIDRC_BUTTON_A) buttonHeld++;
			}
#endif
		}
	}
	menu::Cursor::getInstance().setFreezeAction(false);
	menu::Focus::getInstance().setFreezeAction(false);

	menu::Gui::getInstance().gfx->clearEFB((GXColor){0, 0, 0, 0xFF}, 0x000000);
#ifdef HW_RVL
	pause_netinit_thread();
	pauseRemovalThread();
#endif
	resumeAudio();
	resumeInput();
	menuActive = 0;
#ifdef DEBUGON
	_break();
#endif
	go();
#ifdef DEBUGON
	_break();
#endif
	menuActive = 1;
	pauseInput();
	pauseAudio();

  if(autoSave==AUTOSAVE_ENABLE) {
    if(mcd1Written || mcd2Written) {  //something needs saving
      switch (nativeSaveDevice)
    	{
    		case NATIVESAVEDEVICE_SD:
    		case NATIVESAVEDEVICE_USB:
    			// Adjust saveFile pointers
    			saveFile_dir = (nativeSaveDevice==NATIVESAVEDEVICE_SD) ? &saveDir_libfat_Default:&saveDir_libfat_USB;
    			saveFile_readFile  = fileBrowser_libfat_readFile;
    			saveFile_writeFile = fileBrowser_libfat_writeFile;
    			saveFile_init      = fileBrowser_libfat_init;
    			saveFile_deinit    = fileBrowser_libfat_deinit;
    			break;
    		case NATIVESAVEDEVICE_CARDA:
    		case NATIVESAVEDEVICE_CARDB:
    			// Adjust saveFile pointers
    			saveFile_dir       = (nativeSaveDevice==NATIVESAVEDEVICE_CARDA) ? &saveDir_CARD_SlotA:&saveDir_CARD_SlotB;
    			saveFile_readFile  = fileBrowser_CARD_readFile;
    			saveFile_writeFile = fileBrowser_CARD_writeFile;
    			saveFile_init      = fileBrowser_CARD_init;
    			saveFile_deinit    = fileBrowser_CARD_deinit;
    			break;
    	}
      // Try saving everything
    	int amountSaves = mcd1Written + mcd2Written;
    	int result = 0;
      saveFile_init(saveFile_dir);
      result += SaveMcd(1,saveFile_dir);
      result += SaveMcd(2,saveFile_dir);
      saveFile_deinit(saveFile_dir);
    	if (result>=amountSaves) {  //saved all of them ok
    		switch (nativeSaveDevice)
    		{
    			case NATIVESAVEDEVICE_SD:
    				menu::MessageBox::getInstance().fadeMessage("Automatically saved to SD card");
    				break;
    			case NATIVESAVEDEVICE_USB:
    				menu::MessageBox::getInstance().fadeMessage("Automatically saved to USB device");
    				break;
    			case NATIVESAVEDEVICE_CARDA:
    				menu::MessageBox::getInstance().fadeMessage("Automatically saved to memcard in Slot A");
    				break;
    			case NATIVESAVEDEVICE_CARDB:
    				menu::MessageBox::getInstance().fadeMessage("Automatically saved to memcard in Slot B");
    				break;
    		}
    		mcd1Written = mcd2Written = 0;  //nothing new written since save
  		}
  	  else		{
  	    menu::MessageBox::getInstance().setMessage("Failed to save game"); //one or more failed to save
	    }

    }
  }
#ifdef HW_RVL
  continueRemovalThread();
  resume_netinit_thread();
#endif
	FRAME_BUTTONS[5].buttonString = FRAME_STRINGS[6];
	menu::Cursor::getInstance().clearCursorFocus();
}

void Func_SetPlayGame()
{
	FRAME_BUTTONS[5].buttonString = FRAME_STRINGS[5];
}

void Func_SetResumeGame()
{
	FRAME_BUTTONS[5].buttonString = FRAME_STRINGS[6];
}

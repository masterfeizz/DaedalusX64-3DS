#include <3ds.h>
#include <GL/picaGL.h>
#include <stdio.h>

#include "UserInterface.h"
#include "InGameMenu.h"

#include "BuildOptions.h"
#include "Config/ConfigOptions.h"
#include "Core/Cheats.h"
#include "Core/CPU.h"
#include "Core/Memory.h"
#include "Core/PIF.h"
#include "Core/RomSettings.h"
#include "Core/Save.h"
#include "Debug/DBGConsole.h"
#include "Debug/DebugLog.h"
#include "Graphics/GraphicsContext.h"
#include "HLEGraphics/TextureCache.h"
#include "Input/InputManager.h"
#include "Interface/RomDB.h"
#include "System/Paths.h"
#include "System/System.h"
#include "Test/BatchTest.h"
#include "Utility/IO.h"
#include "Utility/Preferences.h"
#include "Utility/Profiler.h"
#include "Utility/Thread.h"
#include "Utility/Translate.h"
#include "Utility/ROMFile.h"
#include "Utility/Timer.h"

extern float gCurrentFramerate;
extern EFrameskipValue gFrameskipValue;

static uint64_t timer;
static uint8_t currentPage = 0;

static void ExecSaveState(int slot)
{
	IO::Filename full_path;
	sprintf(full_path, "%s%s.ss%ld", DAEDALUS_CTR_PATH("SaveStates/"), g_ROM.settings.GameName.c_str(), slot);

	CPU_RequestSaveState(full_path);
}

static void LoadSaveState(int slot)
{
	IO::Filename full_path;
	sprintf(full_path, "%s%s.ss%ld", DAEDALUS_CTR_PATH("SaveStates/"), g_ROM.settings.GameName.c_str(), slot);

	CPU_RequestLoadState(full_path);
}

static void DrawSaveStatePage()
{
	char buttonString[20];

	UI::DrawHeader("Save state");

	for(int i = 0; i < 3; i++)
	{
		sprintf(buttonString, "Save slot: %i", i);

		if(UI::DrawButton(10, 22 + (54 * i), 300, 44, buttonString))
		{
			ExecSaveState(i);
		}
	}

	if(UI::DrawButton(10, 184, 300, 44, "Back"))
		currentPage = 0;
}

static void DrawLoadStatePage()
{
	char buttonString[20];

	UI::DrawHeader("Load state");

	for(int i = 0; i < 3; i++)
	{
		sprintf(buttonString, "Load slot: %i", i);

		if(UI::DrawButton(10, 22 + (54 * i), 300, 44, buttonString))
		{
			LoadSaveState(i);
		}
	}

	if(UI::DrawButton(10, 184, 300, 44, "Back"))
		currentPage = 0;
}

static void DrawConfirmPage()
{
	UI::DrawHeader("Close ROM: Are you sure?");

	if(UI::DrawToggle(10,  22, 300, 99, "YES", false))
	{
		currentPage = 0;
		 CPU_Halt("Exiting");
	}

	if(UI::DrawButton(10, 131, 300, 99, "NO"))
	{
		currentPage = 0;
	}
	
}

static void DrawOptionsPage()
{
	static uint32_t frameskip = 0;

	char frameskipString[30];

	sprintf(frameskipString, "Frameskip: %s", Preferences_GetFrameskipDescription( (EFrameskipValue)frameskip ));

	UI::DrawHeader("Options");

	if(UI::DrawToggle(10,  22, 300, 62, "Toggle Audio", gAudioPluginEnabled == APM_ENABLED_ASYNC))
	{
		gAudioPluginEnabled = (gAudioPluginEnabled == APM_ENABLED_ASYNC ? APM_DISABLED : APM_ENABLED_ASYNC);
		gSpeedSyncEnabled   = (gAudioPluginEnabled == APM_ENABLED_ASYNC ? false : true);
	}

	if(UI::DrawButton(10,  94, 300, 62, frameskipString))
	{
		if(++frameskip > 6)
			frameskip = 0;

		gFrameskipValue = (EFrameskipValue)frameskip;
	}

	if(UI::DrawButton(10, 166, 300, 62, "Back"))
	{
		currentPage = 0;
	}
	
}

static void DrawMainPage()
{
	char titleString[20];

	sprintf(titleString, "FPS: %.2f", gCurrentFramerate);
	UI::DrawHeader(titleString);

	if((osGetTime() - timer) > 5000)
	{
		if(keysHeld() & KEY_TOUCH)
		{
			timer = osGetTime();
		}
		return;
	}

	if(UI::DrawButton(10,  22, 300, 62, "Save State")) currentPage = 1;
	if(UI::DrawButton(10,  94, 300, 62, "Load State")) currentPage = 2;
	if(UI::DrawButton(10,  166, 145, 62, "Close ROM")) currentPage = 3;
	if(UI::DrawButton(165, 166, 145, 62, "Options"))   currentPage = 4;
}

void UI::DrawInGameMenu()
{
	UI::RestoreRenderState();
	glClear(GL_COLOR_BUFFER_BIT);

	switch(currentPage)
	{
		case 0: DrawMainPage(); break;
		case 1: DrawSaveStatePage(); break;
		case 2: DrawLoadStatePage(); break;
		case 3: DrawConfirmPage(); break;
		case 4: DrawOptionsPage(); break;
	}

	pglSwapBuffers();
	pglSelectScreen(GFX_TOP, GFX_LEFT);
}
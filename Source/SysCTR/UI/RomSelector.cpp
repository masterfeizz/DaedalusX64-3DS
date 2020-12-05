#include "UserInterface.h"
#include "RomSelector.h"

#include <3ds.h>
#include <GL/picaGL.h>
#include <string>
#include <algorithm>
#include <vector>

#include "BuildOptions.h"
#include "Config/ConfigOptions.h"
#include "Core/Cheats.h"
#include "Core/CPU.h"
#include "Core/PIF.h"
#include "Core/RomSettings.h"
#include "Core/Save.h"
#include "Interface/RomDB.h"
#include "System/Paths.h"
#include "System/System.h"
#include "Utility/IO.h"
#include "Utility/Preferences.h"
#include "Utility/ROMFile.h"

#define DAEDALUS_CTR_PATH(p)	"sdmc:/3ds/DaedalusX64/" p

struct SRomInfo
{
	std::string		mFilename;

	RomID			mRomID;
	u32				mRomSize;
	ECicType		mCicType;

	RomSettings		mSettings;
};

bool operator<(const SRomInfo & lhs, const SRomInfo & rhs)
{
    return std::string(lhs.mSettings.GameName) < std::string(rhs.mSettings.GameName);
}

static std::vector<SRomInfo> PopulateRomList()
{
	std::vector<SRomInfo> roms = {};

	std::string			full_path;
	IO::FindHandleT		find_handle;
	IO::FindDataT		find_data;
	
	if(IO::FindFileOpen( DAEDALUS_CTR_PATH("Roms/"), &find_handle, find_data ))
	{
		do 
		{
			const char* rom_filename( find_data.Name );

			if(IsRomfilename( rom_filename ))
			{
				SRomInfo info;

				full_path = DAEDALUS_CTR_PATH("Roms/");
				full_path += rom_filename;

				info.mFilename = rom_filename;

				if (ROM_GetRomDetailsByFilename(full_path.c_str(), &info.mRomID, &info.mRomSize, &info.mCicType))
				{
					if (!CRomSettingsDB::Get()->GetSettings(info.mRomID, &info.mSettings ))
					{
						std::string game_name;

						info.mSettings.Reset();
						info.mSettings.Comment = "Unknown";

						if (!ROM_GetRomName(full_path.c_str(), game_name )) game_name = full_path;

						game_name = game_name.substr(0, 63);
						info.mSettings.GameName = game_name.c_str();
						CRomSettingsDB::Get()->SetSettings(info.mRomID, info.mSettings);
					}
				}
				else
				{
					info.mSettings.GameName = "Unknown";
				} 

				roms.push_back(info);
			}

		} while(IO::FindFileNext( find_handle, find_data ));

		IO::FindFileClose( find_handle );
	}

	std::stable_sort(roms.begin(), roms.end());

	return roms;
}

std::string UI::DrawRomSelector()
{
	int cursor = 0, scroll = 0;

	std::vector<SRomInfo> roms = PopulateRomList();	
	
	UI::RestoreRenderState();

	if(roms.empty())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		DrawText( 4, 12, "No ROMs found!");

		pglSwapBuffers();
		UI::ClearSecondScreen(GFX_TOP);

		while(aptMainLoop())
		{
			hidScanInput();

			if(hidKeysDown() == KEY_START)
				exit(1);
		}
	}


	while(aptMainLoop())
	{

		hidScanInput();

		glClear(GL_COLOR_BUFFER_BIT);
		glColor4f(0.5f, 0.5f, 0.5f, 1.0f);

		for(int i = 0; i < roms.size(); i++)
		{
			if(cursor == i)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawText( 4, 24 + ((i - scroll) * 10), roms.at(i).mSettings.GameName.c_str());
				glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
			}
			else
				DrawText( 4, 24 + ((i - scroll) * 10), roms.at(i).mSettings.GameName.c_str());
		}

		UI::DrawHeader("Rom Selector");

		pglSwapBuffers();
		UI::ClearSecondScreen(GFX_TOP);

		if(hidKeysDown() & KEY_A)
			return roms.at(cursor).mFilename;

		if((hidKeysDown() & KEY_DOWN) && cursor != roms.size() - 1)
			cursor++;

		if((hidKeysDown() & KEY_UP) && cursor != 0)
			cursor--;

		if((cursor - scroll) > 21)
			scroll++;

		if((cursor - scroll) < 0)
			scroll--;
	}

	return nullptr;
}
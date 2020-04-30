#include "UserInterface.h"
#include "RomSelector.h"

#include <3ds.h>
#include <GL/picaGL.h>
#include <string>
#include <vector>

#include "Utility/ROMFile.h"

#define DAEDALUS_CTR_PATH(p)	"sdmc:/3ds/DaedalusX64/" p

static std::vector<std::string> PopulateRomList()
{
	std::vector<std::string> files = {};

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
				files.push_back(rom_filename);
			}

		} while(IO::FindFileNext( find_handle, find_data ));

		IO::FindFileClose( find_handle );
	}

	return files;
}

std::string UI::DrawRomSelector()
{
	int cursor = 0, scroll = 0;

	std::vector<std::string> files = PopulateRomList();	
	
	UI::RestoreRenderState();

	if(files.empty())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		DrawText( 4, 12, "No ROMs found!");

		pglSwapBuffers();
		UI::ClearSecondScreen(GFX_TOP);
		gfxSwapBuffersGpu();

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

		for(int i = 0; i < files.size(); i++)
		{
			if(cursor == i)
			{
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				DrawText( 4, 24 + ((i - scroll) * 10), files.at(i).c_str());
				glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
			}
			else
				DrawText( 4, 24 + ((i - scroll) * 10), files.at(i).c_str());
		}

		UI::DrawHeader("Rom Selector");

		pglSwapBuffers();
		UI::ClearSecondScreen(GFX_TOP);
		gfxSwapBuffersGpu();

		if(hidKeysDown() & KEY_A)
			return files.at(cursor);

		if((hidKeysDown() & KEY_DOWN) && cursor != files.size() - 1)
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
#pragma once
#include <stdint.h>
namespace UI
{
	void Initialize();
	void RestoreRenderState();
	void ClearSecondScreen(unsigned screen);
	void DrawHeader(const char *title);

	void DrawText(float x, float y, const char *text);
	bool DrawButton(float x, float y, float width, float height, const char *text);
	bool DrawToggle(float x, float y, float width, float height, const char *text, bool isToggled);
}
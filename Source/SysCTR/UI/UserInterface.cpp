#include <3ds.h>
#include <GL/picaGL.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "UserInterface.h"

static stbtt_bakedchar charData[96];
static GLuint fontTex;

static uint32_t GetStringWidth(const char* text)
{
	uint32_t stringWidth = 0;

	while (*text) 
	{
		if (*text >= 32 && *text < 128)
		{
			stbtt_bakedchar *b = charData + (*text-32);
			stringWidth += b->xadvance;

			if(*text == ' ')
				stringWidth += 3;
		}
		++text;
	}

	return stringWidth;
}

//Loads the font
void UI::Initialize()
{
	uint8_t* ttfBuffer = new uint8_t[1<<20];
	uint8_t* bitmap_u8 = new uint8_t[256*256];
	uint32_t* bitmap_u32 = new uint32_t[256*256];

	FILE *fontFile = fopen("romfs:/kenvector_future.ttf", "rb");

	if(fontFile == NULL)
		exit(0);

	fread(ttfBuffer, 1, 1<<20, fontFile);

	stbtt_BakeFontBitmap(ttfBuffer,0, 14.0, bitmap_u8, 256, 256, 32, 96, charData); // no guarantee this fits!

	for(uint32_t i =0; i < 256 * 256; i++)
	{
		bitmap_u32[i] = (0x00ffffff) | (bitmap_u8[i] << 24);
	}

	glGenTextures(1, &fontTex);
	glBindTexture(GL_TEXTURE_2D, fontTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256,256, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap_u32);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	delete [] ttfBuffer;
	delete [] bitmap_u8;
	delete [] bitmap_u32;
}

void UI::RestoreRenderState()
{
	pglSelectScreen(GFX_BOTTOM, GFX_LEFT);

	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	glViewport(0,0,320,240);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glMatrixMode(GL_PROJECTION);
	glOrtho(0, 320, 240, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void UI::ClearSecondScreen(unsigned screen)
{
	pglSelectScreen(screen, GFX_LEFT);
	glClear(GL_COLOR_BUFFER_BIT);
	pglSwapBuffers();
	pglSelectScreen(!screen, GFX_LEFT);
}

void UI::DrawHeader(const char *title)
{
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.3f, 0.6f, 0.4f);
		glVertex2f(0, 0);
		glVertex2f(320, 0);
		glVertex2f(0, 12);
		glVertex2f(320, 12);
	glEnd();

	glColor3f(0.9f, 0.9f, 0.9f);
	UI::DrawText(4, 10, title);
}

bool UI::DrawButton(float x, float y, float width, float height, const char *text)
{
	touchPosition touch;

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(0.3f, 0.6f, 0.4f);
		glVertex2f(x, y);
		glVertex2f(x+width, y);
		glVertex2f(x, y+height);
		glVertex2f(x+width, y+height);
	glEnd();

	glColor3f(0.9f, 0.9f, 0.9f);

	float tY = y + (height/2) + 6;
	float tX = x + (width/2) - (GetStringWidth(text) / 2);

	UI::DrawText(tX, tY, text);

	hidTouchRead(&touch);

	if(hidKeysDown() & KEY_TOUCH)
	{
		if(touch.px > x && touch.px < (x + width) && touch.py > y && touch.py < (y + height))
			return true;
	}

	return false;
}

void UI::DrawText(float x, float y, const char *text)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fontTex);

	while (*text) 
	{
		if (*text >= 32 && *text < 128)
		{
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(charData, 256, 256, *text-32, &x, &y, &q ,1);

			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
			glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
			glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);
			glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
			glEnd();
		}
		++text;
	}
}
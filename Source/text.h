#ifndef _TEXT_H_INCLUDED_
#define _TEXT_H_INCLUDED_

#include "main.h"

struct CharInfo
{
	int x, y, w, h, offsetX, offsetY, advance;
};

struct Font
{
	string face;
	int lineHeight;

	CharInfo chars[256];
};

enum Anchor
{
	ANCHOR_TOP = 0,
	ANCHOR_BOTTOM
};

void beginRenderText();
void endRenderText();
void drawText(const string &text, vec2 position, float scale = 1.0, Anchor anchor = ANCHOR_TOP);
void initFont(int screenWidth, int screenHeight);

#endif // _TEXT_H_INCLUDED_

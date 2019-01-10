#ifndef _LOADER_H_INCLUDED_
#define _LOADER_H_INCLUDED_

#include "main.h"

struct Texture
{
	GLuint tex = 0;
	int width = 0;
	int height = 0;
	int components = 0;
};

const Texture &getTexture(const string &filename);
void clearTextures();

#endif // _LOADER_H_INCLUDED_

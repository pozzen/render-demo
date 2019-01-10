#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unordered_map<string, Texture> textures;

const Texture &getTexture(const string &filename)
{
	if (textures.find(filename) != textures.end()) return textures[filename];
	else
	{
		Texture t;

		unsigned char *data = stbi_load(filename.c_str(), &t.width, &t.height, &t.components, 4);

		if (!data)
		{
			cerr<<"Texture loading failed for '"<<filename<<"'."<<endl;
		}
		else
		{
			glGenTextures(1, &t.tex);
			glBindTexture(GL_TEXTURE_2D, t.tex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

			glBindTexture(GL_TEXTURE_2D, 0);

			stbi_image_free(data);
		}

		textures[filename] = t;
		return textures[filename];
	}
}

void clearTextures()
{
	for (unordered_map<string, Texture>::iterator it = textures.begin(); it != textures.end(); it++)
	{
		if (it->second.tex != 0) glDeleteTextures(1, &it->second.tex);
	}
}

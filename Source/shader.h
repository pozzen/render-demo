#ifndef _SHADER_H_INCLUDED_
#define _SHADER_H_INCLUDED_

#include "main.h"

struct Shader
{
	GLuint program = 0;
	GLuint shaders[5] = {0};

	void setUniform(const char *name, float value);
	void setUniform(const char *name, int value);
	void setUniform(const char *name, bool value);
	void setUniform(const char *name, GLuint value);
	void setUniform(const char *name, vec2 value);
	void setUniform(const char *name, vec3 value);
	void setUniform(const char *name, vec4 value);
	void setUniform(const char *name, mat4 value);
};

const Shader &getShader(const string &shaderName, const vector<string> &defines = {});
void clearShaders();

#endif // _SHADER_H_INCLUDED_

#ifndef _MODEL_H_INCLUDED_
#define _MODEL_H_INCLUDED_

#include "main.h"
#include "texture.h"
#include "shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex
{
	vec3 position;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
};

struct Material
{
	vector<Texture> textures;
	vector<aiTextureType> textureTypes;
};

struct BoundingBox
{
	vec3 min = vec3(INFINITY);
	vec3 max = vec3(-INFINITY);
};

struct Mesh
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	int elements;
	mat4 transform;
	Material material;
	BoundingBox bb;
};

struct Model
{
	vector<Mesh> meshes;
	BoundingBox bb;
};

Mesh createMesh(vector<Vertex> vertices, vector<GLuint> indices, mat4 transform = mat4(1.0), Material material = Material(), BoundingBox bb = BoundingBox());
Model loadModel(const string &filename);
void clearMeshes();
void bindMaterial(const Material &m, Shader &shader);

#endif // _MODEL_H_INCLUDED_

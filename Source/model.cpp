#include "model.h"

vector<Mesh> meshes;

Mesh createMesh(vector<Vertex> vertices, vector<GLuint> indices, mat4 transform, Material material, BoundingBox bb)
{
	Mesh m;
	m.elements = indices.size();
	m.transform = transform;
	m.material = material;
	m.bb = bb;

	glGenVertexArrays(1, &m.vao);
	glGenBuffers(1, &m.vbo);
	glGenBuffers(1, &m.ebo);

	glBindVertexArray(m.vao);
	glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// Vertex Texture Coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoord));

	// Vertex Normals
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

	// Vertex Tangents
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));

	// Vertex Bitangents
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);

	meshes.push_back(m);
	return m;
}

void processTextures(const string &filepath, aiTextureType type, aiMaterial *material, Material &m)
{
	int nTextures = material->GetTextureCount(type);

	for(GLuint i = 0; i < nTextures; i++)
	{
		aiString str;
		material->GetTexture(type, i, &str);

		m.textures.push_back(getTexture(filepath+str.C_Str()));
		m.textureTypes.push_back(type);
	}
}

Material processMaterial(const string &filepath, aiMaterial *material)
{
	Material m;

	processTextures(filepath, aiTextureType_DIFFUSE, material, m);
	processTextures(filepath, aiTextureType_SPECULAR, material, m);
//	processTextures(filepath, aiTextureType_AMBIENT, material, m);
//	processTextures(filepath, aiTextureType_EMISSIVE, material, m);
	processTextures(filepath, aiTextureType_HEIGHT, material, m);
	processTextures(filepath, aiTextureType_NORMALS, material, m);
//	processTextures(filepath, aiTextureType_SHININESS, material, m);
//	processTextures(filepath, aiTextureType_OPACITY, material, m);
	processTextures(filepath, aiTextureType_DISPLACEMENT, material, m);

	return m;
}

Mesh processMesh(const string &filepath, aiMesh *mesh, const aiScene *scene, const mat4 &transform)
{
	vector<Vertex> vertices;
	vector<GLuint> indices;
	BoundingBox bb;

	for(GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		// Process vertex positions, normals, coordinates, tangents, and bitangents
		vertex.position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

		if (mesh->HasTextureCoords(0))
			vertex.texCoord = vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else
			vertex.texCoord = vec2(0.0f, 0.0f);

		if (mesh->HasTangentsAndBitangents())
		{
				vertex.tangent = vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
				vertex.bitangent = vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}

		vertices.push_back(vertex);

		// Update bounding box
		bb.min.x = glm::min(bb.min.x, vertex.position.x);
		bb.min.y = glm::min(bb.min.y, vertex.position.y);
		bb.min.z = glm::min(bb.min.z, vertex.position.z);

		bb.max.x = glm::max(bb.max.x, vertex.position.x);
		bb.max.y = glm::max(bb.max.y, vertex.position.y);
		bb.max.z = glm::max(bb.max.z, vertex.position.z);
	}

	// Process indices
	for(GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Process material
	Material material = processMaterial(filepath, scene->mMaterials[mesh->mMaterialIndex]);

	return createMesh(vertices, indices, transform, material, bb);
}

void processNode(const string &filepath, aiNode *node, const aiScene *scene, const mat4 &transform, Model &model)
{
	mat4 nodeTransform = transform * glm::transpose(glm::make_mat4(&node->mTransformation.a1));

	for(GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Mesh m = processMesh(filepath, mesh, scene, nodeTransform);
		model.meshes.push_back(m);
	}

	for(GLuint i = 0; i < node->mNumChildren; i++)
	{
		processNode(filepath, node->mChildren[i], scene, nodeTransform, model);
	}
}

Model loadModel(const string &filename)
{
	Model m;

	string filepath;
	if (unsigned int separator = filename.find_last_of("/"))
	{
		filepath += filename.substr(0, separator+1);
	}

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

	if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cerr << "Assimp: " << import.GetErrorString() << endl;
		return m;
	}

	processNode(filepath, scene->mRootNode, scene, mat4(1.0), m);

	for (const Mesh &mesh: m.meshes)
	{
		vec3 min = vec3(mesh.transform * vec4(mesh.bb.min, 1.0));
		vec3 max = vec3(mesh.transform * vec4(mesh.bb.max, 1.0));

		m.bb.min.x = glm::min(m.bb.min.x, min.x);
		m.bb.min.y = glm::min(m.bb.min.y, min.y);
		m.bb.min.z = glm::min(m.bb.min.z, min.z);

		m.bb.max.x = glm::max(m.bb.max.x, max.x);
		m.bb.max.y = glm::max(m.bb.max.y, max.y);
		m.bb.max.z = glm::max(m.bb.max.z, max.z);
	}

	return m;
}

void clearMeshes()
{
	for (vector<Mesh>::iterator it = meshes.begin(); it != meshes.end(); it++)
	{
		glDeleteBuffers(1, &(*it).ebo);
		glDeleteBuffers(1, &(*it).vbo);
		glDeleteVertexArrays(1, &(*it).vao);
	}
}

void bindMaterial(const Material &m, Shader &shader)
{
	static const char *textureTypeNames[] = {
		"none",
		"diffuse",
		"specular",
		"ambient",
		"emissive",
		"normal", // use height map as normal map
		"normal",
		"shininess",
		"opacity",
		"displacement"
	};

	int counters[aiTextureType_UNKNOWN] = {0};
	int counter = 0;

	for (unsigned int i = 0; i < m.textures.size(); i++)
	{
		string uniformName = "texture_";
		uniformName += textureTypeNames[m.textureTypes[i]];
		uniformName += std::to_string(++counters[m.textureTypes[i]]);

		glUniform1i(glGetUniformLocation(shader.program, uniformName.c_str()), counter);
		glActiveTexture(GL_TEXTURE0 + (counter++));
		glBindTexture(GL_TEXTURE_2D, m.textures[i].tex);
	}

	if (counters[aiTextureType_DIFFUSE] == 0) glUniform1i(glGetUniformLocation(shader.program, "texture_diffuse1"), 6);
	if (counters[aiTextureType_SPECULAR] == 0) glUniform1i(glGetUniformLocation(shader.program, "texture_specular1"), 6);
	if (counters[aiTextureType_HEIGHT] == 0) glUniform1i(glGetUniformLocation(shader.program, "texture_normal1"), 6);
}

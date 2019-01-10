#include "shader.h"
#include "sstream"

unordered_map<string, Shader> shaders;

string loadShader(const string& fileName, const vector<string> defines = {})
{
	std::ifstream file;
	file.open((fileName).c_str());

	string result;
	string line;

	if (file.is_open())
	{
		int headerLinesSkipped = 0;
		int curLine = 0;

		while (file.good())
		{
			getline(file, line);
			if (line.find("#include") == 0)
			{
				string fullPath;
				if (unsigned int separator = fileName.find_last_of("/"))
				{
					fullPath += fileName.substr(0, separator+1);
				}
				fullPath += line.substr(10, line.size()-11);
				result.append(loadShader(fullPath));
			}
			else
			{
				if(line.find("#version") == 0 || line.find("precision") == 0)
				{
					headerLinesSkipped++;
				}

				result.append(line + "\n");
				curLine++;

				if (headerLinesSkipped == 2 && curLine < 3)
				{
					for (int i = 0; i < defines.size(); i++)
					{
						result.append("#define ");
						result.append(defines[i]);
						result.append("\n");
					}
					curLine = 3;
				}
			}
		}
		file.close();
	}

	return result;
}

bool checkProgramError(GLuint shader, const string &errorMessage)
{
	GLint success = 0;
	GLchar error[1024];

	glGetProgramiv(shader, GL_LINK_STATUS, &success);

	if(success == GL_FALSE)
	{
		glGetProgramInfoLog(shader, sizeof(error), NULL, error);

		cerr<<errorMessage<<":\n"<<error<<endl;

		return false;
	}

	return true;
}

bool checkShaderError(GLuint shader, const string &errorMessage, const string &fileName, const string &source)
{
	GLint success = 0;
	GLchar error[1024];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(success == GL_FALSE)
	{
		glGetShaderInfoLog(shader, sizeof(error), NULL, error);

		cerr<<errorMessage<<":"<<endl;
		cerr<<"  "<<fileName<<endl;

		if (source.size() > 0)
		{
			int a;
			int errorLine;
			char c;
			if (std::sscanf(error, "%d%c%d", &a, &c, &errorLine) == 3)
			{
				std::stringstream ss(source);
				string line;
				int curLine = 1;
				while (std::getline(ss, line))
				{
					if (curLine == errorLine)
					{
						cerr<<" > |"<<line<<endl;
					}
					else if (curLine >= errorLine-2)
					{
						cerr<<"   |"<<line<<endl;
					}
					if (curLine > errorLine+2) break;
					curLine++;
				}
			}
		}

		cerr<<error<<endl;

		return false;
	}

	return true;
}

bool createShader(const string &fileName, unsigned int type, GLuint &shader, const vector<string> &defines)
{
	const string source = loadShader(fileName, defines);
	const GLchar *s = source.c_str();
	GLint length = source.length();

	if (source == "") return false;

	shader = glCreateShader(type);

	if(shader == 0)
	{
		cerr<<"Error creating shader type "<<type<<endl;
		return 0;
	}

	glShaderSource(shader, 1, &s, &length);
	glCompileShader(shader);

	if (!checkShaderError(shader, "Error compiling shader", fileName, source))
	{
		shader = 0;
		return false;
	}

	return true;
}

const Shader &getShader(const string &shaderName, const vector<string> &defines)
{
	string uid = shaderName;
	for (uint i = 0; i < defines.size(); i++)
	{
		uid += "#";
		uid += defines[i];
	}

	if (shaders.find(uid) != shaders.end()) return shaders[uid];
	else
	{
		Shader s;

		s.program = glCreateProgram();

		vector<string> vsDefines = defines;
		vector<string> fsDefines = defines;
		vector<string> csDefines = defines;
		vector<string> gsDefines = defines;
		vsDefines.push_back("VS");
		fsDefines.push_back("FS");
		csDefines.push_back("CS");
		gsDefines.push_back("GS");

		int counter = 0;

		if (createShader(shaderName + ".vs", GL_VERTEX_SHADER, s.shaders[counter], vsDefines)) counter++;
		if (createShader(shaderName + ".fs", GL_FRAGMENT_SHADER, s.shaders[counter], fsDefines)) counter++;
		if (createShader(shaderName + ".cs", GL_COMPUTE_SHADER, s.shaders[counter], csDefines)) counter++;
		if (createShader(shaderName + ".gs", GL_GEOMETRY_SHADER, s.shaders[counter], gsDefines)) counter++;

		for (int i = 0; s.shaders[i] != 0; i++)
		{
			glAttachShader(s.program, s.shaders[i]);
		}

		glLinkProgram(s.program);
		if (!checkProgramError(s.program, "Error linking shader program"))
		{
			for (int i = 0; s.shaders[i] != 0; i++)
			{
				glDeleteShader(s.shaders[i]);
			}
			glDeleteProgram(s.program);
			shaders[uid] = s;
			return shaders[uid];
		}

		glValidateProgram(s.program);
		if (!checkProgramError(s.program, "Invalid shader program"))
		{
			for (int i = 0; s.shaders[i] != 0; i++)
			{
				glDetachShader(s.program, s.shaders[i]);
				glDeleteShader(s.shaders[i]);
			}
			glDeleteProgram(s.program);
			shaders[uid] = s;
			return shaders[uid];
		}

//		GLuint ubiVP = glGetUniformBlockIndex(s.program, "matrices");
//		glUniformBlockBinding(s.program, ubiVP, 0);

		shaders[uid] = s;
		return shaders[uid];
	}
}

void Shader::setUniform(const char *name, float value)
{
	glUniform1f(glGetUniformLocation(program, name), value);
}

void Shader::setUniform(const char *name, int value)
{
	glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::setUniform(const char *name, bool value)
{
	glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::setUniform(const char *name, GLuint value)
{
	glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::setUniform(const char *name, vec2 value)
{
	glUniform2fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
}

void Shader::setUniform(const char *name, vec3 value)
{
	glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
}

void Shader::setUniform(const char *name, vec4 value)
{
	glUniform4fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
}

void Shader::setUniform(const char *name, mat4 value)
{
	glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(value));
}

void clearShaders()
{
	for (unordered_map<string, Shader>::iterator it = shaders.begin(); it != shaders.end(); it++)
	{
		for (int i = 0; it->second.shaders[i] != 0; i++)
		{
			glDetachShader(it->second.program, it->second.shaders[i]);
			glDeleteShader(it->second.shaders[i]);
		}
		glDeleteProgram(it->second.program);
	}
}

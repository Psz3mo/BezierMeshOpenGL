#pragma once
#include <glad/gl.h>
#include <string>

using namespace std;

class ShaderProgram {
public:
	GLuint ID;
	ShaderProgram(const char* vertexpath, const char* grafmentPath);
	void use();
	~ShaderProgram();
private:
	bool loadShaderSource(const char* path, string& shaderCode);
	GLuint compileShader(const char* shaderSource, GLenum shaderType);
};


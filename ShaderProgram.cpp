#include "ShaderProgram.h"
#include <iostream>
#include <fstream>
#include <sstream>

ShaderProgram::ShaderProgram(const char* vertexPath, const char* frgamentPath) {
	string vertexCode, fragmentCode;
	if (!loadShaderSource(vertexPath, vertexCode) || !loadShaderSource(frgamentPath, fragmentCode)) {
		cout << "Failed to load shader source" << endl;
		return;
	}

	GLuint vertexShader = compileShader(vertexCode.c_str(), GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentCode.c_str(), GL_FRAGMENT_SHADER);

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	GLint success;
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		cout << "Failed to link shader program" << endl << infoLog << endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void ShaderProgram::use() {
	glUseProgram(ID);
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(ID);
}

bool ShaderProgram::loadShaderSource(const char* path, string& shaderCode) {
	ifstream file(path);
	if (!file.is_open()) {
		cout << "Failed to open file: " << path << endl;
		return false;
	}

	stringstream stream;
	stream << file.rdbuf();
	shaderCode = stream.str();
	file.close();

	return true;
}

GLuint ShaderProgram::compileShader(const char* shaderSource, GLenum shaderType) {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		cout << "Failed to compile shader" << endl << infoLog << endl;
	}

	return shader;
}
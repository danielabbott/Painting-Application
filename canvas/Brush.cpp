#include <Brush.h>
#include <Shader.h>
#include <iostream>

using namespace std;

static GLuint vsId = 0;

void Brush::create(const char * fragmentShaderFile)
{
	if(!vsId) {
		vsId = load_shader("res/brush.vert", GL_VERTEX_SHADER);
	}

	GLuint fsId = load_shader(fragmentShaderFile, GL_FRAGMENT_SHADER);
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsId);
	glAttachShader(shaderProgram, fsId);
	link_shader_program(shaderProgram);
	glDeleteShader(fsId);
	matrixUniformLocation = glGetUniformLocation(shaderProgram, "matrix");
	if(matrixUniformLocation == -1) throw runtime_error("res/brush.vert does not define uniform mat4 matrix");
	opacityUniformLocation = glGetUniformLocation(shaderProgram, "opacity");
	if(opacityUniformLocation == -1) {
		cerr << "Error compiling " << fragmentShaderFile << endl;
	 	throw runtime_error("Brush fragment shader does not define uniform float opacity");
	}

	GLint uniLoc = glGetUniformLocation(shaderProgram, "brushTexture");
	if(uniLoc != -1) {
		glUniform1i(uniLoc, 0);
	}
}

void Brush::freeStaticResources()
{
	glDeleteShader(vsId);
}

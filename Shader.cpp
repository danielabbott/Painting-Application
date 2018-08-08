#include <Shader.h>
#include <vector>
#include <Files.h>

using namespace std;


GLuint load_shader(string shaderFilePath, GLenum type, const char * prepend)
{
	vector<unsigned char> source = load_file(shaderFilePath);
	source.push_back(0);

	GLuint id = glCreateShader(type);
	if(!id) {
		throw runtime_error("Error creating OpenGL shader object");
	}

	if(prepend) {
		const char * sources[2] = {prepend, (const char *) source.data()};
		glShaderSource(id, 2, sources, nullptr);
	}
	else {
		const char * shaderSource = (const char *) source.data();
		glShaderSource(id, 1, &shaderSource, nullptr);
	}

	glCompileShader(id);

	GLint status;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		/* Compilation error, output log and abort */

		cerr << "Error compiling " << 
		(type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
		<< " shader" << endl;

		GLint logSize = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logSize);

		if (logSize > 0) {
			char * log = new char[logSize + 1];
			glGetShaderInfoLog(id, logSize, nullptr, log);
			log[logSize - 1] = 0;

			cerr << "******" << endl << log << endl << "******" << endl;
			delete log;
		}

		glDeleteShader(id);

		throw runtime_error("Error compiling shader");
	}

	return id;
}

void print_program_log(GLuint id, ostream&)
{
	GLint logSize = 0;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);

	if (logSize) {
		char * log = new char[logSize + 1];
		glGetProgramInfoLog(id, logSize, NULL, log);
		log[logSize - 1] = 0;

		cerr << "******" << endl << log << endl << "******" << endl;
		delete log;
	}
}

// TODO: Cache shader programs with GL_ARB_get_program_binary

void link_shader_program(GLuint id)
{
	glLinkProgram(id);

	GLint status;
	glGetProgramiv(id, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		/* Linker error, output log and abort */

		print_program_log(id, cerr);

		throw runtime_error("Error linking shaders");
	}

	
}

void validate_shader_program(GLuint id)
{
	glValidateProgram(id);

	GLint status;
	glGetProgramiv(id, GL_VALIDATE_STATUS, &status);
	print_program_log(id, clog);

	if(status == GL_FALSE) {
		throw runtime_error("Error validating shaders");
	}
}

static GLuint bound = 0;

void bind_shader_program(GLuint programId)
{
	if(programId != bound) {
		glUseProgram(programId);
		bound = programId;
	}
}

GLuint get_bound_program()
{
	return bound;
}


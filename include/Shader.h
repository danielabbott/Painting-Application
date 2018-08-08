#pragma once

#include <glad/glad.h>
#include <iostream>

GLuint load_shader(std::string shaderFilePath, GLenum type, const char * prepend = nullptr);
void link_shader_program(GLuint id);

// Call this after linking and after setting sampler values
void validate_shader_program(GLuint id);

void bind_shader_program(GLuint programId);
GLuint get_bound_program();

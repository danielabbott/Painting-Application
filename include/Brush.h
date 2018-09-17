#pragma once

#include <glad/glad.h>

struct Brush {
	// 0 to 1. Turns a square brush into a rectangular brush, or a circular brush into a ovular brush, etc.
	float height = 1;

	// If true that value in 'colour' is used for brush strokes (active colour is ignored) and the user cannot change this colour
	// without changing the brush properties. If false, 'colour' is not used and the active colour is used
	bool fixedColour = false;
	float colour[4] = {0,0,0,1};

	float opacity = 1;
	float hardness = 0.9f; // 0 to 1

	enum class BlendMode {
		MAX, // Used for simple brushes
		ADD // Used for noise brush
	};

	BlendMode blendMode = BlendMode::MAX;

	GLuint shaderProgram;

	// Guaranteed to be valid
	GLint strokeAlphaUniformLocation;

	// These could be invalid (-1), in this case they should not be used
	GLint matrixUniformLocation;
	GLint seedUniformLocation;
	GLint hardnessUniformLocation;

	// GLuint texture;


	void create(const char * fragmentShaderFile);

	// Called before program termination to free OpenGL shader object
	static void freeStaticResources();
};

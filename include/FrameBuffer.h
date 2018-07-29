#pragma once

#include <ArrayTexture.h>

// Use glBindFragDataLocation to bind fragment shader outputs to a colour attachment 

class ArrayTextureFrameBuffer
{
	GLuint frameBufferName = 0;
	unsigned int size = 0;
	ArrayTexture * arrayTexture;
public:
	void create(ArrayTexture & arrayTexture, unsigned int arrayTextureIndex);

	// For drawing on the framebuffer
	void bindFrameBuffer();

	void destroy();

	bool isCreated() const { return frameBufferName != 0; }

	ArrayTextureFrameBuffer() {}
	ArrayTextureFrameBuffer(ArrayTextureFrameBuffer const&) = delete;
};

class FrameBuffer
{
	GLuint frameBufferName = 0;
	GLuint backingTextureId;
	unsigned int width = 0;
	unsigned int height = 0;
	ImageFormat type;
public:

	void create(ImageFormat type, unsigned int width, unsigned int height);

	// uses GL_ARB_clear_texture if available, otherwise will bind the framebuffer
	void clear();

	// For drawing on the framebuffer
	void bindFrameBuffer();

	// For using this framebuffer as a texture to draw with
	void bindTexture();

	// The texture MUST be bound when this function is called
	void getTexureData(void * outputBuffer);

	void destroy();
	
	FrameBuffer() {}
	FrameBuffer(FrameBuffer const&) = delete;
};

void bind_default_framebuffer();

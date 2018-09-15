#pragma once

#include <ArrayTexture.h>

// Use glBindFragDataLocation to bind fragment shader outputs to a colour attachment 

class ArrayTextureFrameBuffer
{
	GLuint frameBufferName = 0;
	unsigned int size = 0;
	ArrayTexture * arrayTexture;
public:
	ArrayTextureFrameBuffer(ArrayTexture & arrayTexture, unsigned int arrayTextureIndex);
	ArrayTextureFrameBuffer(ArrayTextureFrameBuffer const&) = delete;
	~ArrayTextureFrameBuffer();

	// For drawing on the framebuffer
	void bindFrameBuffer() const;

};

class FrameBuffer
{
	GLuint frameBufferName = 0;
	GLuint backingTextureId;
	ImageFormat type;
	unsigned int width = 0;
	unsigned int height = 0;
public:

	FrameBuffer(ImageFormat type, unsigned int width, unsigned int height);
	FrameBuffer(FrameBuffer const&) = delete;
	~FrameBuffer();

	// uses GL_ARB_clear_texture if available, otherwise will bind the framebuffer
	void clear() const;

	// For drawing on the framebuffer
	void bindFrameBuffer() const;

	// For using this framebuffer as a texture to draw with
	void bindTexture() const;

	// The texture MUST be bound when this function is called
	void getTexureData(void * outputBuffer) const;
	
};

void bind_default_framebuffer();

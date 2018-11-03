#include <FrameBuffer.h>
#include <stdexcept>
#include <cassert>
#include <Window.h>

#include <iostream>

using namespace std;


ArrayTextureFrameBuffer::ArrayTextureFrameBuffer(ArrayTexture & arrayTexture_, unsigned int arrayTextureIndex)
{
	arrayTexture = &arrayTexture_;

	glGenFramebuffers(1, &frameBufferName);

	if(!frameBufferName) {
		throw runtime_error("Error creating OpenGL framebuffer (glGenFramebuffers)");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);

	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, arrayTexture->id, 0, arrayTextureIndex);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(sizeof drawBuffers / sizeof(GLenum), drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		frameBufferName = 0;
		throw runtime_error("Error creating OpenGL framebuffer (with array texture)");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	arrayTexture->users++;
}

void ArrayTextureFrameBuffer::bindFrameBuffer() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
	glViewport(0, 0, arrayTexture->widthHeight, arrayTexture->widthHeight);
}

ArrayTextureFrameBuffer::~ArrayTextureFrameBuffer()
{
	arrayTexture->users--;
	glDeleteFramebuffers(1, &frameBufferName);
	frameBufferName = 0;
}

void FrameBuffer::create()
{
	glGenFramebuffers(1, &frameBufferName);

	if(!frameBufferName) {
		throw runtime_error("Error creating OpenGL framebuffer (glGenFramebuffers)");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);

	glGenTextures(1, &backingTextureId);
	glBindTexture(GL_TEXTURE_2D, backingTextureId);

	if(type == ImageFormat::FMT_RGBA) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}
	else if (type == ImageFormat::FMT_RG) {cout<<"nononon"<<endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
	}
	else if (type == ImageFormat::FMT_R) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}
	else {
		assert(0);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backingTextureId, 0);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(sizeof drawBuffers / sizeof(GLenum), drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		frameBufferName = 0;
		throw runtime_error("Error creating OpenGL framebuffer");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::FrameBuffer(ImageFormat type_, unsigned int width_, unsigned int height_, bool createWhenNeeded)
:type(type_), width(width_), height(height_)
{
	if(!createWhenNeeded) {
		create();
	}
}

void FrameBuffer::bindFrameBuffer()
{
	if(!frameBufferName) {
		create();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
	glViewport(0, 0, width, height);
}

void FrameBuffer::bindTexture()
{
	if(!frameBufferName) {
		create();
	}
	glBindTexture(GL_TEXTURE_2D, backingTextureId);
}

void FrameBuffer::getTexureData(void * outputBuffer)
{
	if(!frameBufferName) {
		create();
	}
	if(type == ImageFormat::FMT_RGBA) {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA8, GL_UNSIGNED_BYTE, outputBuffer);
	}
	else if (type == ImageFormat::FMT_RG) {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_UNSIGNED_BYTE, outputBuffer);
	}
	else if (type == ImageFormat::FMT_R) {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, outputBuffer);
	}
}

void FrameBuffer::clear()
{
	if(!frameBufferName) {
		create();
	}
	if(GLAD_GL_ARB_clear_texture) {
		if(type == ImageFormat::FMT_RGBA) {
			uint32_t colour = 0x00ffffff;
			glClearTexImage(backingTextureId, 0, GL_RGBA8, GL_UNSIGNED_BYTE, &colour);
		}
		else if (type == ImageFormat::FMT_RG) {
			uint16_t colour = 0x00ff;
			glClearTexImage(backingTextureId, 0, GL_RG, GL_UNSIGNED_BYTE, &colour);
		}
		else if (type == ImageFormat::FMT_R) {
			uint8_t colour = 0;
			glClearTexImage(backingTextureId, 0, GL_RED, GL_UNSIGNED_BYTE, &colour);
		}
	}
	else {
		bindFrameBuffer();
		if(type == ImageFormat::FMT_RGBA) {
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (type == ImageFormat::FMT_RG) {
			glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (type == ImageFormat::FMT_R) {
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

FrameBuffer::~FrameBuffer()
{
	if(!frameBufferName) {
		glDeleteTextures(1, &backingTextureId);
		glDeleteFramebuffers(1, &frameBufferName);
	}
}


void bind_default_framebuffer()
{
	unsigned int windowWidth, windowHeight;
	get_window_dimensions(windowWidth, windowHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

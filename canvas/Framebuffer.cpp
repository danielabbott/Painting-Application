#include <FrameBuffer.h>
#include <stdexcept>
#include <cassert>

namespace UI {
	void get_window_dimensions(unsigned int & windowWidth, unsigned int & windowHeight);
}

using namespace std;


void ArrayTextureFrameBuffer::create(ArrayTexture & arrayTexture_, unsigned int arrayTextureIndex)
{
	assert(!frameBufferName);
	assert(arrayTexture_.id);

	arrayTexture = &arrayTexture_;

	glGenFramebuffers(1, &frameBufferName);

	if(!frameBufferName) {
		throw runtime_error("Error creating OpenGL framebuffer(glGenFramebuffers)");
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

void ArrayTextureFrameBuffer::bindFrameBuffer()
{
	assert(frameBufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
	glViewport(0, 0, arrayTexture->widthHeight, arrayTexture->widthHeight);
}

void ArrayTextureFrameBuffer::destroy()
{
	assert(frameBufferName);
	arrayTexture->users--;
	glDeleteFramebuffers(1, &frameBufferName);
	frameBufferName = 0;
}

void FrameBuffer::create(ImageFormat type_, unsigned int width_, unsigned int height_)
{
	assert(!frameBufferName);
	type = type_;

	width = width_;
	height = height_;

	glGenFramebuffers(1, &frameBufferName);

	if(!frameBufferName) {
		throw runtime_error("Error creating OpenGL framebuffer (glGenFramebuffers)");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);

	glGenTextures(1, &backingTextureId);
	glBindTexture(GL_TEXTURE_2D, backingTextureId);

	if(type == FMT_RGBA) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}
	else if (type == FMT_RG) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
	}
	else if (type == FMT_R) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}
	else {
		assert(0);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backingTextureId, 0);

	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(sizeof drawBuffers / sizeof(GLenum), drawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		frameBufferName = 0;
		throw runtime_error("Error creating OpenGL framebuffer");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void FrameBuffer::bindFrameBuffer()
{
	assert(frameBufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferName);
	glViewport(0, 0, width, height);
}

void FrameBuffer::bindTexture()
{
	glBindTexture(GL_TEXTURE_2D, backingTextureId);
}

void FrameBuffer::getTexureData(void * outputBuffer)
{
	if(type == FMT_RGBA) {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE, outputBuffer);
	}
	else if (type == FMT_RG) {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_UNSIGNED_BYTE, outputBuffer);
	}
	else if (type == FMT_R) {
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, outputBuffer);
	}
}

void FrameBuffer::clear()
{
	if(GLAD_GL_ARB_clear_texture) {
		if(type == FMT_RGBA) {
			uint32_t colour = 0x00ffffff;
			glClearTexImage(backingTextureId, 0, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE, &colour);
		}
		else if (type == FMT_RG) {
			uint16_t colour = 0x00ff;
			glClearTexImage(backingTextureId, 0, GL_RG, GL_UNSIGNED_BYTE, &colour);
		}
		else if (type == FMT_R) {
			uint8_t colour = 0;
			glClearTexImage(backingTextureId, 0, GL_RED, GL_UNSIGNED_BYTE, &colour);
		}
	}
	else {
		bindFrameBuffer();
		if(type == FMT_RGBA) {
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (type == FMT_RG) {
			glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
		}
		else if (type == FMT_R) {
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

void FrameBuffer::destroy()
{
	assert(frameBufferName);
	glDeleteTextures(1, &backingTextureId);
	glDeleteFramebuffers(1, &frameBufferName);
	frameBufferName = 0;
}


void bind_default_framebuffer()
{
	unsigned int windowWidth, windowHeight;
	UI::get_window_dimensions(windowWidth, windowHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

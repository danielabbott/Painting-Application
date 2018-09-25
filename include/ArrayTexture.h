#pragma once

#include <glad/glad.h>
#include <ImageFile.h>

// Array textures are limited to 512x512 pixels and 256 images

class ArrayTextureFrameBuffer;
class ArrayTexture {
friend ArrayTextureFrameBuffer;

	GLuint id = 0;
	unsigned int widthHeight;
	unsigned int imageCount;
	ImageFormat type;

	// Number of framebuffers using this texture
	// TODO: Perhaps use shared_pointer instead
	unsigned int users = 0;

public:
	ArrayTexture(ImageFormat type, unsigned int widthHeight, unsigned int imageCount);
	ArrayTexture(ArrayTexture const&) = delete;
	~ArrayTexture();

	void bind() const;

	// Fills the texture with the given colour (all layers)
	// These 2 functions are only supported if GLAD_GL_ARB_clear_texture is true
	// If the extension is not supported then bind the framebuffer for each layer and call glClear
	void clear(uint32_t colour = 0) const;
	void clear(unsigned int firstImage, unsigned int imagesToFill, uint32_t colour = 0) const;

	// data MUST NOT be null
	// call bind() first
	void upload(void * data, unsigned int firstImage, unsigned int imagesToFill) const;
	void uploadImage(unsigned int layerIndex, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType) const;

	// Source data is the bound framebuffer
	// Assumes the bound framebuffer is the same width and height as this array texture
	void copy(unsigned int startIndex) const;

};

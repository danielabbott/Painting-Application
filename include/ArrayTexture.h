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
	unsigned int users = 0;

public:
	void create(ImageFormat type, unsigned int widthHeight, unsigned int imageCount);

	void bind() const;

	// Fills the texture with the given colour (all layers)
	// These 2 functions are only supported if GLAD_GL_ARB_clear_texture is true
	// If the extension is not supported then bind the framebuffer for each layer and call glClear
	void clear(uint32_t colour = 0);
	void clear(unsigned int firstImage, unsigned int imagesToFill, uint32_t colour = 0);

	// data MUST NOT be null
	// call bind() first
	void upload(void * data, unsigned int firstImage, unsigned int imagesToFill);
	void uploadImage(unsigned int layerIndex, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType);

	void destroy();

	bool isCreated() const { return id != 0; }

	// Source data is the bound framebuffer
	// Assumes the bound framebuffer is the same width and height as this array texture
	void copy(unsigned int startIndex);

	ArrayTexture() {}
	ArrayTexture(ArrayTexture const&) = delete;
};

#include <ArrayTexture.h>
#include <cassert>
#include <cstring>
#include <hwcaps.h>
#include <stdexcept>

using namespace std;

ArrayTexture::ArrayTexture(ImageFormat type_, unsigned int wh, unsigned int imageCount_)
{
	assert(wh);
	assert(wh <= max_texture_size());
	assert(imageCount_);
	assert(imageCount_ <= 256);

	widthHeight = wh;
	imageCount = imageCount_;
	type = type_;

	glGenTextures(1, &id);

	if(!id) {
		throw runtime_error("Error in glGenTextures");
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, id);

	if(GLAD_GL_ARB_texture_storage) {
		if(type == ImageFormat::FMT_RGBA) {
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, widthHeight, widthHeight, imageCount);
		}
		else if(type == ImageFormat::FMT_RG) {
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RG8, widthHeight, widthHeight, imageCount);
		}
		else if(type == ImageFormat::FMT_R) {
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, widthHeight, widthHeight, imageCount);	    	
		}
		else {
			assert(0);
		}
	}
	else {
		if(type == ImageFormat::FMT_RGBA) {
	    	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, widthHeight, widthHeight, imageCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		else if (type == ImageFormat::FMT_RG) {
	    	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RG8, widthHeight, widthHeight, imageCount, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
		}
		else if (type == ImageFormat::FMT_R) {
	    	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, widthHeight, widthHeight, imageCount, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		}
		else {
			assert(0);
		}
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

void ArrayTexture::bind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, id);
}

void ArrayTexture::clear(unsigned int firstImage, unsigned int imagesToFill, uint32_t colour) const
{
	assert(GLAD_GL_ARB_clear_texture);
	glClearTexSubImage(id, 0, 0, 0, firstImage, widthHeight, widthHeight, imagesToFill, GL_RGBA, GL_UNSIGNED_BYTE, &colour);
}

void ArrayTexture::clear(uint32_t colour) const
{
	assert(GLAD_GL_ARB_clear_texture);
	glClearTexImage(id, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colour);
}

void ArrayTexture::upload(void * data, unsigned int firstImage, unsigned int imagesToFill) const
{
	assert(data);
	if(type == ImageFormat::FMT_RGBA) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, firstImage, widthHeight, widthHeight, imagesToFill, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (type == ImageFormat::FMT_RG) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, firstImage, widthHeight, widthHeight, imagesToFill, GL_RG, GL_UNSIGNED_BYTE, data);
	}
	else if (type == ImageFormat::FMT_R) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, firstImage, widthHeight, widthHeight, imagesToFill, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	else {
		assert(0);
	}
}

// When a new layer is added or a layer is deleted, a new array texture is created for each layer and the data is copied across

void ArrayTexture::copy(unsigned int index) const
{
	bind();
	glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, 0, 0, widthHeight, widthHeight);
}

void ArrayTexture::copy(unsigned int index, unsigned int x, unsigned int y, unsigned int w, unsigned int h) const
{
	assert(x + w <= widthHeight);
	assert(y + h <= widthHeight);

	bind();
	glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, x, widthHeight - y - h, index, x, widthHeight - y - h, w, h);
}

ArrayTexture::~ArrayTexture()
{
	assert(!users);

	glDeleteTextures(1, &id);
	id = 0;
}

void ArrayTexture::uploadImage(unsigned int layerIndex, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType) const
{
	if(stride) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, stride);
	}

	if(sourceType == ImageFormat::FMT_RGBA) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, x, y, layerIndex, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (sourceType == ImageFormat::FMT_RG) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, x, y, layerIndex, width, height, 1, GL_RG, GL_UNSIGNED_BYTE, data);
	}
	else if (sourceType == ImageFormat::FMT_R) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, x, y, layerIndex, width, height, 1, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	else {
		assert(0);
	}

	if(stride) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
}

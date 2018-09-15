#pragma once

#include <FrameBuffer.h>
#include <deque>
#include <Brush.h>

// An image block is a 256x256 texture/framebuffer that stores image data for that region on all layers

class Layer;
class Canvas;


unsigned int image_block_size();

class ImageBlock {
// friend void Canvas::draw();

	// Keeps track of the region of this image block that is dirty
	unsigned int dirtyMinX = 0;
	unsigned int dirtyMinY = 0;
	unsigned int dirtyWidth = image_block_size();
	unsigned int dirtyHeight = image_block_size();

public:
	// Keeps track of whether anything has changed in this image bock since the last redraw..
	bool dirty = true;

	// Used for keeping track of which image blocks have been drawn on during the current stroke
	bool hasStrokeData = false;

	bool strokeDataFillsBlock = false;

	// Information about this image block on one particular layer
	struct LayerData {
		unsigned int layer; // Index into global layers std::vector

		enum class DataType {
			SOLID_COLOUR,
			ACTUAL_DATA
		};

		DataType dataType = DataType::SOLID_COLOUR;
		// unsigned int arrayTextureIndex; // if dataType == ACTUAL_DATA

		// If dataType == SOLID_COLOUR
		// If this is a greyscale image then only the most and least significant bytes are used (red and alpha)
		// If this is an alpha-only layer then only the most significant byte is used (alpha)
		uint32_t colour;

		ArrayTextureFrameBuffer frameBuffer;

		LayerData(ArrayTexture & arrayTexture, unsigned int arrayTextureIndex) : frameBuffer(arrayTexture, arrayTextureIndex) {}
	};

	unsigned int getDirtyMinX() const { return dirtyMinX; }
	unsigned int getDirtyMinY() const { return dirtyMinY; }
	unsigned int getDirtyWidth() const { return dirtyWidth; }
	unsigned int getDirtyHeight() const { return dirtyHeight; }

private:
	unsigned int x = -1;
	unsigned int y = -1;

	// RGBA, greyscale, and alpha-only images are kept in separate array textures

	ArrayTexture * arrayTextureRGBA = nullptr;
	ArrayTexture * arrayTextureRG = nullptr;
	ArrayTexture * arrayTextureR = nullptr;

	std::deque<LayerData> layersRGBA;
	std::deque<LayerData> layersRG;
	std::deque<LayerData> layersR;

public:
	ImageBlock(unsigned int x_, unsigned int y_, Canvas const& canvas);

	bool dirtyRegion(int x_, int y_, unsigned int width, unsigned int height);

	void setClean()
	{
		dirty = false;
	}

	unsigned int getX() const { return x; }
	unsigned int getY() const { return y; }

	void bindFrameBuffer(Layer * layer);
	void bindTexture(Layer * layer) const;
	void copyTo(Layer * layer);

	std::deque<LayerData> const& RGBALayers() { return layersRGBA; }
	std::deque<LayerData> const& RGLayers() { return layersRG; }
	std::deque<LayerData> const& RLayers() { return layersR; }

	const ArrayTexture * getArrayTextureRGBA() { return arrayTextureRGBA; }
	const ArrayTexture * getArrayTextureRG() { return arrayTextureRG; }
	const ArrayTexture * getArrayTextureR() { return arrayTextureR; }

	// Overwrites pixels, no blending is done
	void uploadImage(Layer * layer, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType);

	// Returns -1 if no video memory is allocated for that layer
	int indexOf(Layer * layer);

	void fillLayer(Layer * layer, uint32_t colour);

	~ImageBlock();
};




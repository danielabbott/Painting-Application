#pragma once

#include <FrameBuffer.h>
#include <deque>
#include <Brush.h>
#include <Layer.h>

// An image block is a square texture/framebuffer that stores image data for that region on all layers

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
	// Keeps track of whether anything has changed in this image bock since the last redraw.
	bool dirty = true;

	// Used for keeping track of which image blocks have been drawn on during the current stroke
	bool hasStrokeData = false;

	bool strokeDataFillsBlock = false;

	// Information about this image block on one particular layer
	struct LayerData {
		LayerPtr layer;

		enum class DataType {
			SOLID_COLOUR,
			ACTUAL_DATA
		};

		DataType dataType = DataType::SOLID_COLOUR;

		// If dataType == SOLID_COLOUR
		// If this is a greyscale image then only the most and least significant bytes are used (red and alpha)
		// If this is an alpha-only layer then only the most significant byte is used (alpha)
		uint32_t colour = 0;

		ArrayTextureFrameBuffer * frameBuffer = nullptr;

		// Value only valid if != -1
		int arrayTextureIndex = -1;

		LayerData(LayerPtr l) : layer(l) {}
		LayerData(LayerPtr l, ArrayTexture & arrayTexture, unsigned int arrayTextureIndex_) 
		: layer(l), frameBuffer(new ArrayTextureFrameBuffer(arrayTexture, arrayTextureIndex_)), arrayTextureIndex(arrayTextureIndex_) {}

		~LayerData() 
		{
			if(frameBuffer) {
				delete frameBuffer;
			}
		}
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

	std::deque<LayerData> layers;

public:
	ImageBlock(unsigned int x_, unsigned int y_, Canvas const& canvas);
	ImageBlock(ImageBlock const&) = delete;
	ImageBlock(ImageBlock &&) = delete;
	ImageBlock& operator=(const ImageBlock&&) = delete;

	bool dirtyRegion(int x_, int y_, unsigned int width, unsigned int height);

	void setClean()
	{
		dirty = false;
	}

	unsigned int getX() const { return x; }
	unsigned int getY() const { return y; }

	void bindFrameBuffer(LayerPtr layer);
	void bindTexture(LayerPtr layer) const;
	void copyTo(LayerPtr layer);
	void copyTo(LayerPtr layer, unsigned int x, unsigned int y, unsigned int w, unsigned int h);

	std::deque<LayerData> const& getLayerData() { return layers; }

	const ArrayTexture * getArrayTextureRGBA() { return arrayTextureRGBA; }
	const ArrayTexture * getArrayTextureRG() { return arrayTextureRG; }
	const ArrayTexture * getArrayTextureR() { return arrayTextureR; }

	// Overwrites pixels, no blending is done
	void uploadImage(LayerPtr layer, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType);

	// Returns -1 if no video memory is allocated for that layer
	int indexOf(LayerPtr layer);

	void fillLayer(LayerPtr layer, uint32_t colour);

	~ImageBlock();
};




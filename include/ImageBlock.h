#pragma once

#include <FrameBuffer.h>
#include <vector>
#include <Brush.h>
#include <Canvas.h>

// An image block is a 256x256 texture/framebuffer that stores image data for that region on all layers

struct ImageBlock {
friend void Canvas::draw();
public:
	// Keeps track of whether anything has changed in this image bock since the last redraw..
	bool dirty = true;

	// Used for keeping track of which image blocks have been drawn on during the current stroke
	bool hasStrokeData = false;

	// Information about this image block on one particular layer
	struct LayerData {
		unsigned int layer; // Index into global layers std::vector

		enum DataType {
			SOLID_COLOUR,
			ACTUAL_DATA
		};

		DataType dataType = SOLID_COLOUR;
		// unsigned int arrayTextureIndex; // if dataType == ACTUAL_DATA

		// If dataType == SOLID_COLOUR
		// If this is a greyscale image then only the most and least significant bytes are used (red and alpha)
		// If this is an alpha-only layer then only the most significant byte is used (alpha)
		uint32_t colour;

		ArrayTextureFrameBuffer frameBuffer;
	};

private:
	unsigned int x = -1;
	unsigned int y = -1;

	// RGBA, greyscale, and alpha-only images are kept in separate array textures

	ArrayTexture arrayTextureRGBA;
	ArrayTexture arrayTextureRG;
	ArrayTexture arrayTextureR;

	std::vector<LayerData> layersRGBA;
	std::vector<LayerData> layersRG;
	std::vector<LayerData> layersR;

public:
	ImageBlock(){}
	ImageBlock(unsigned int x_, unsigned int y_) : x(x_), y(y_) {}

	unsigned int getX() const { return x; }
	unsigned int getY() const { return y; }

	// Sets internal state to match global layers array
	void create();

	void bindFrameBuffer(Layer * layer);
	void bindTexture(Layer * layer);
	void copyTo(Layer * layer);

	// Overwrites pixels, no blending is done
	void uploadImage(Layer * layer, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType);

	// Returns -1 if no video memory is allocated for that layer
	int indexOf(Layer * layer);

	void fillLayer(Layer * layer, uint32_t colour);

	void destroy();
};

unsigned int image_block_size();



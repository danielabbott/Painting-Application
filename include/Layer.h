#pragma once

#include <string>
#include <ArrayTexture.h> // For ImageFormat enum

// Layers are stored in a fixed-size array in Canvas.cpp

struct Layer {
	enum Type {
		LAYER, LAYER_GROUP
	};

	Type type = LAYER;

	std::string name;
	ImageFormat imageFormat = FMT_RGBA;

	enum Mode {
		BLEND_MODE_NORMAL, BLEND_MODE_ADD, BLEND_MODE_SUB, BLEND_MODE_MUL, BLEND_MODE_DIV,
		FILTER_HSV_ADJUST, FILTER_GREYSCALE, FILTER_BLUR
	};

	Mode mode = BLEND_MODE_NORMAL;

	Layer * firstChild = nullptr;
	Layer * next = nullptr; // The layer above
	Layer * parent = nullptr;

	// Index into layersRGBA, layerRG, or layersR in ImageBlock
	// TODO: Maybe this won't be the same across all image blocks - save memory
	unsigned int imageFormatSpecificIndex = -1;

	Layer() {}
	Layer(std::string name_) : type(LAYER), name(name_) {}
};

// TODO Remove this. This is just for testing things
void create_layers();

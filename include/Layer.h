#pragma once

#include <string>
#include <ArrayTexture.h> // For ImageFormat enum

struct Layer {
	enum class Type {
		LAYER, LAYER_GROUP
	};

	Type type = Type::LAYER;

	std::string name;
	ImageFormat imageFormat = ImageFormat::FMT_RGBA;

	enum class Mode {
		BLEND_MODE_NORMAL, BLEND_MODE_ADD, BLEND_MODE_SUB, BLEND_MODE_MUL, BLEND_MODE_DIV,
		FILTER_HSV_ADJUST, FILTER_GREYSCALE, FILTER_BLUR
	};

	Mode mode = Mode::BLEND_MODE_NORMAL;

	Layer * firstChild = nullptr;
	Layer * next = nullptr; // The layer above
	Layer * parent = nullptr;

	Layer() {}
	Layer(std::string name_) : type(Type::LAYER), name(name_) {}
};

// TODO Remove this. This is just for testing things
void create_layers();

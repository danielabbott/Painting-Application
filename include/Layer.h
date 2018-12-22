#pragma once

#include <string>
#include <memory>
#include <ArrayTexture.h> // For ImageFormat enum

struct Layer;
typedef std::shared_ptr<Layer> LayerPtr;

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

	LayerPtr firstChild = nullptr;
	LayerPtr next = nullptr; // The layer above
	LayerPtr prev = nullptr;
	LayerPtr parent = nullptr;

	bool visible = true;

	Layer() {}
	Layer(std::string name_) : type(Type::LAYER), name(name_) {}

	LayerPtr getNext();
};


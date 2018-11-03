#include <ImageBlock.h>
#include <Layer.h>
#include <cassert>
#include <stdexcept>
#include <UI.h>
#include <Canvas.h>
#include <Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;


unsigned int image_block_size() { return 256; }

ImageBlock::ImageBlock(unsigned int x_, unsigned int y_, Canvas const& canvas)
: x(x_), y(y_)
{
	unsigned int numRGBA = 0;
	unsigned int numRG = 0;
	unsigned int numR = 0;

	Layer * layer = canvas.get_first_layer();

	if(!layer) {
		// No layers, use default state
		return;
	}

	// TODO replace while loops with iterators
	// TODO only allocate memory if it is needed (new canvases get memory for one layer (initially unused). allocate more later if needed)

	while(1) {
		if(layer->type == Layer::Type::LAYER) {
			if(layer->imageFormat == ImageFormat::FMT_RGBA) {
				numRGBA++;
			}
			else if(layer->imageFormat == ImageFormat::FMT_RG) {
				numRG++;
			}
			else if(layer->imageFormat == ImageFormat::FMT_R) {
				numR++;
			}
		}

		if(!(layer = layer->getNext())) {
			break;
		}
	}


	if(numRGBA) {
		arrayTextureRGBA = new ArrayTexture(ImageFormat::FMT_RGBA, image_block_size(), numRGBA);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRGBA->clear(0x00ffffff);
		}
	}
	if(numRG) {
		arrayTextureRG = new ArrayTexture(ImageFormat::FMT_RG, image_block_size(), numRG);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRG->clear(0x00ff);
		}		
	}
	if(numR) {
		arrayTextureR = new ArrayTexture(ImageFormat::FMT_R, image_block_size(), numR);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureR->clear();
		}		
	}

	numRGBA = numRG = numR = 0;
	layer = canvas.get_first_layer();

	while(1) {
		if(layer->type == Layer::Type::LAYER) {
			if(layer->imageFormat == ImageFormat::FMT_RGBA) {
				layers.emplace_back(layer, *arrayTextureRGBA, numRGBA);
				LayerData & layerData = *(layers.end()-1);
				layerData.colour = 0x00ffffff;

				if(!GLAD_GL_ARB_clear_texture) {
					layerData.frameBuffer->bindFrameBuffer();
					glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT);
				}

				numRGBA++;
			}
			else if(layer->imageFormat == ImageFormat::FMT_RG) {
				layers.emplace_back(layer, *arrayTextureRG, numRG);
				LayerData & layerData = *(layers.end()-1);
				layerData.colour = 0x00ffffff;

				if(!GLAD_GL_ARB_clear_texture) {
					layerData.frameBuffer->bindFrameBuffer();
					glClearColor(1.0f, 0.0f, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT);
				}
				numRG++;
			}
			else if(layer->imageFormat == ImageFormat::FMT_R) {
				layers.emplace_back(layer, *arrayTextureR, numR);
				LayerData & layerData = *(layers.end()-1);
				layerData.colour = 0xff; // effect layers default to 100% intensity for every pixel

				if(!GLAD_GL_ARB_clear_texture) {
					layerData.frameBuffer->bindFrameBuffer();
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT);
				}
				numR++;
			}
		}


		if(!(layer = layer->getNext())) {
			break;
		}
	}

}

ImageBlock::~ImageBlock()
{
	// Clear list to run deconstructors so that users field in array texture objects is decreased to 0
	// To prevent an assertion failure in the deconstructors of arrayTextureR* 
	layers.clear();

	delete arrayTextureR;
	delete arrayTextureRG;
	delete arrayTextureRGBA;
}

int ImageBlock::indexOf(Layer * layer)
{
	assert(layer);

	unsigned int i = 0;
	for(LayerData const& ld : layers) {
		if(ld.layer == layer) {
			return i;
		}
		i++;
	}
	
	return -1;
}

void ImageBlock::bindFrameBuffer(Layer * layer)
{
	assert(layer);

	int index = indexOf(layer);
	if(index == -1) {
		throw runtime_error("Attempted to bind framebuffer of layer with no video memory");
	}

	layers[index].frameBuffer->bindFrameBuffer();
	layers[index].dataType = LayerData::DataType::ACTUAL_DATA;
}

void ImageBlock::bindTexture(Layer * layer) const
{
	assert(layer);

	arrayTextureRGBA->bind();	
}


void ImageBlock::copyTo(Layer * layer)
{
	copyTo(layer, 0, 0, image_block_size(), image_block_size());
}


void ImageBlock::copyTo(Layer * layer, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	assert(layer);

	int index = indexOf(layer);
	if(index == -1) {
		throw runtime_error("Attempted to copy to layer with no video memory");
	}

	layers[index].dataType = LayerData::DataType::ACTUAL_DATA;
	
	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		arrayTextureRGBA->copy(index, x, y, w, h);
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		arrayTextureRG->copy(index, x, y, w, h);
	}
	else {
		arrayTextureR->copy(index, x, y, w, h);
	}
}

void ImageBlock::uploadImage(Layer * layer, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType)
{
	assert(layer);

	int index = indexOf(layer);
	if(index == -1) {
		throw runtime_error("Attempted to upload to layer with no video memory");
	}

	layers[index].dataType = LayerData::DataType::ACTUAL_DATA;
	
	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		arrayTextureRGBA->uploadImage(index, x, y, width, height, data, stride, sourceType);
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		arrayTextureRG->uploadImage(index, x, y, width, height, data, stride, sourceType);
	}
	else {
		arrayTextureR->uploadImage(index, x, y, width, height, data, stride, sourceType);
	}
}

void ImageBlock::fillLayer(Layer * layer, uint32_t colour)
{
	assert(layer);

	int index = indexOf(layer);
	if(index == -1) {
		throw runtime_error("Attempted to fill layer with no video memory");
	}

	layers[index].colour = colour;

	layers[index].dataType = LayerData::DataType::SOLID_COLOUR;
	
	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRGBA->clear(index, 1, colour);
		}
		else {
			layers[index].frameBuffer->bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, ((colour >> 16) & 0xff) / 255.0f, ((colour >> 24) & 0xff) / 255.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRG->clear(index, 1, colour);
		}
		else {
			layers[index].frameBuffer->bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else if(layer->imageFormat == ImageFormat::FMT_R) {
		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureR->clear(index, 1, colour);
		}
		else {
			layers[index].frameBuffer->bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	
}

bool ImageBlock::dirtyRegion(int x_, int y_, unsigned int width, unsigned int height)
{
	x_ -= (int)x;
	y_ -= (int)y;

	if(x_ < (int)image_block_size() && y_ < (int)image_block_size() &&
		x_ + (int)width >= 0 && y_ + (int)height >= 0) {


		if(x_ < 0) {
			width += x_;
			x_ = 0;
		}

		if(y_ < 0) {
			height += y_;
			y_ = 0;
		}

		if((unsigned)x_ + width > image_block_size()) {
			width = image_block_size() - (unsigned)x_;
		}

		if((unsigned)y_ + height > image_block_size()) {
			height = image_block_size() - (unsigned)y_;
		}

		if(dirty) {
			if(x_ < (int)dirtyMinX) {
				dirtyWidth += dirtyMinX-x_;
				dirtyMinX = x_;
			}
			if(y_ < (int)dirtyMinY) {
				dirtyHeight += dirtyMinY-y_;
				dirtyMinY = y_;
			}
			if(x_+width > dirtyMinX+dirtyWidth){
				dirtyWidth = x_+width - dirtyMinX;
			}
			if(y_+height > dirtyMinY+dirtyHeight){
				dirtyHeight = y_+height - dirtyMinY;
			}

		}
		else {
			dirtyMinX = x_;
			dirtyMinY = y_;
			dirtyWidth = width;
			dirtyHeight = height;
			dirty = true;
		}

		return true;
	}
	return false;
}

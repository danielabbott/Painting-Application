#include <ImageBlock.h>
#include <Layer.h>
#include <cassert>
#include <UI.h>
#include <Canvas.h>
#include <Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

extern Layer layers[64];
extern int firstLayer;


unsigned int image_block_size() { return 1024; }

void ImageBlock::create()
{
	assert(!layersRGBA.size());
	assert(!layersRG.size());
	assert(!layersR.size());

	if(firstLayer == -1) {
		// No layers, use default state
		return;
	}

	unsigned int numRGBA = 0;
	unsigned int numRG = 0;
	unsigned int numR = 0;

	for(unsigned int i = 0; i < 64; i++) {
		if(layers[i].type == Layer::LAYER) {
			if(layers[i].imageFormat == FMT_RGBA) {
				layers[i].imageFormatSpecificIndex = numRGBA;
				numRGBA++;
			}
			else if(layers[i].imageFormat == FMT_RG) {
				layers[i].imageFormatSpecificIndex = numRG;
				numRG++;
			}
			else if(layers[i].imageFormat == FMT_R) {
				layers[i].imageFormatSpecificIndex = numR;
				numR++;
			}
		}
	}

	layersRGBA = vector<LayerData>(numRGBA);
	layersRG = vector<LayerData>(numRG);
	layersR = vector<LayerData>(numR);

	if(numRGBA) {
		arrayTextureRGBA.create(FMT_RGBA, image_block_size(), numRGBA);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRGBA.clear(0x00ffffff);
		}

		for(unsigned int i = 0; i < numRGBA; i++) {
			layersRGBA[i].colour = 0x00ffffff;
			layersRGBA[i].frameBuffer.create(arrayTextureRGBA, layers[i].imageFormatSpecificIndex);

			if(!GLAD_GL_ARB_clear_texture) {
				layersRGBA[i].frameBuffer.bindFrameBuffer();
				glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}

	}
	if(numRG) {
		arrayTextureRG.create(FMT_RG, image_block_size(), numRG);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRG.clear(0x00ff);
		}

		for(unsigned int i = 0; i < numRG; i++) {
			layersRG[i].colour = 0x00ffffff;
			layersRG[i].frameBuffer.create(arrayTextureRG, layers[i].imageFormatSpecificIndex);

			if(!GLAD_GL_ARB_clear_texture) {
				layersRG[i].frameBuffer.bindFrameBuffer();
				glClearColor(1.0f, 0.0f, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
	}
	if(numR) {
		arrayTextureR.create(FMT_R, image_block_size(), numR);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureR.clear();
		}

		for(unsigned int i = 0; i < numR; i++) {
			layersR[i].colour = 0x00;
			layersR[i].frameBuffer.create(arrayTextureR, layers[i].imageFormatSpecificIndex);

			if(!GLAD_GL_ARB_clear_texture) {
				layersR[i].frameBuffer.bindFrameBuffer();
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
	}

}

void ImageBlock::destroy()
{
	for(LayerData & l : layersRGBA) {
		if(l.frameBuffer.isCreated()){
			l.frameBuffer.destroy();
		}
	}
	for(LayerData & l : layersRG) {
		if(l.frameBuffer.isCreated()){
			l.frameBuffer.destroy();
		}
	}
	for(LayerData & l : layersR) {
		if(l.frameBuffer.isCreated()){
			l.frameBuffer.destroy();
		}
	}


	if(arrayTextureRGBA.isCreated()) {
		arrayTextureRGBA.destroy();
	}

	if(arrayTextureRG.isCreated()) {
		arrayTextureRG.destroy();
	}

	if(arrayTextureR.isCreated()) {
		arrayTextureR.destroy();
	}
}

int ImageBlock::indexOf(unsigned int layerIndex)
{
	assert(layerIndex < 64);
	return layers[layerIndex].imageFormatSpecificIndex;
}

void ImageBlock::bindFrameBuffer(unsigned int layerIndex)
{
	assert(layerIndex < 64);
	Layer & layer = layers[layerIndex];

	if(layer.imageFormat == FMT_RGBA) {
		layersRGBA[layer.imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
		layersRGBA[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
	}
	else if(layer.imageFormat == FMT_RG) {
		layersRG[layer.imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
		layersRG[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
	}
	else if(layer.imageFormat == FMT_R) {
		layersR[layer.imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
		layersR[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
	}
	else {
		assert(0);
	}
}

void ImageBlock::bindTexture(unsigned int layerIndex)
{
	assert(layerIndex < 64);
	Layer & layer = layers[layerIndex];

	if(layer.imageFormat == FMT_RGBA) {
		arrayTextureRGBA.bind();
	}
	else if(layer.imageFormat == FMT_RG) {
		arrayTextureRG.bind();
	}
	else if(layer.imageFormat == FMT_R) {
		arrayTextureR.bind();
	}
	else {
		assert(0);
	}
}


void ImageBlock::copyTo(unsigned int layerIndex)
{
	assert(layerIndex < 64);
	
	Layer & layer = layers[layerIndex];

	if(layer.imageFormat == FMT_RGBA) {
		layersRGBA[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
		arrayTextureRGBA.copy(layer.imageFormatSpecificIndex);
	}
	else if(layer.imageFormat == FMT_RG) {
		layersRG[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
		arrayTextureRG.copy(layer.imageFormatSpecificIndex);
	}
	else if(layer.imageFormat == FMT_R) {
		layersR[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
		arrayTextureR.copy(layer.imageFormatSpecificIndex);
	}
	else {
		assert(0);
	}
}

void ImageBlock::uploadImage(unsigned int layerIndex, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType)
{
	assert(layerIndex < 64);
	
	Layer & layer = layers[layerIndex];

	if(layer.imageFormat == FMT_RGBA) {
		layersRGBA[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
		arrayTextureRGBA.uploadImage(layer.imageFormatSpecificIndex, x, y, width, height, data, stride, sourceType);
	}
	else if(layer.imageFormat == FMT_RG) {
		layersRG[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
		arrayTextureRG.uploadImage(layer.imageFormatSpecificIndex, x, y, width, height, data, stride, sourceType);
	}
	else if(layer.imageFormat == FMT_R) {
		layersR[layer.imageFormatSpecificIndex].dataType = LayerData::ACTUAL_DATA;
		arrayTextureR.uploadImage(layer.imageFormatSpecificIndex, x, y, width, height, data, stride, sourceType);
	}
	else {
		assert(0);
	}

}

void ImageBlock::fillLayer(unsigned int layerIndex, uint32_t colour)
{
	assert(layerIndex < 64);
	
	Layer & layer = layers[layerIndex];

	if(layer.imageFormat == FMT_RGBA) {
		layersRGBA[layer.imageFormatSpecificIndex].dataType = LayerData::SOLID_COLOUR;
		layersRGBA[layer.imageFormatSpecificIndex].colour = colour;

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRGBA.clear(layer.imageFormatSpecificIndex, 1, colour);
		}
		else {
			layersRGBA[layer.imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, ((colour >> 16) & 0xff) / 255.0f, ((colour >> 24) & 0xff) / 255.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else if(layer.imageFormat == FMT_RG) {
		layersRG[layer.imageFormatSpecificIndex].dataType = LayerData::SOLID_COLOUR;
		layersRG[layer.imageFormatSpecificIndex].colour = colour;

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRG.clear(layer.imageFormatSpecificIndex, 1, colour);
		}
		else {
			layersRG[layer.imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else if(layer.imageFormat == FMT_R) {
		layersR[layer.imageFormatSpecificIndex].dataType = LayerData::SOLID_COLOUR;
		layersR[layer.imageFormatSpecificIndex].colour = colour;

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureR.clear(layer.imageFormatSpecificIndex, 1, colour);
		}
		else {
			layersR[layer.imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else {
		assert(0);
	}
}

#include <Canvas.h>
#include <Layer.h>
#include <cassert>
#include "CanvasState.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.h>

using namespace std;

extern CanvasResources canvasResources;

void Canvas::draw() {
	unsigned int windowWidth, windowHeight;
	UI::get_window_dimensions(windowWidth, windowHeight);

	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	glBindVertexArray(canvasResources.vaoId);
	if(canvasResources.canvasDirty) {
		bind_shader_program(canvasResources.shaderProgram);
		glDisable(GL_BLEND);
		canvasResources.canvasFrameBuffer.bindFrameBuffer();

		glActiveTexture(GL_TEXTURE3);
		canvasResources.strokeLayer.bindTexture();

		for(ImageBlock & block : canvasResources.imageBlocks) {
			if(!block.dirty) {
				continue;
			}
			block.setClean();

			unsigned int rgbaIndex = 0;
			unsigned int rgIndex = 0;
			unsigned int rIndex = 0;

			assert(canvasResources.firstLayer);
			Layer * layer = canvasResources.firstLayer;
			vector<Op> ops;

			canvasResources.uniformData.baseColour[0] = canvasResources.uniformData.baseColour[1] = 
			canvasResources.uniformData.baseColour[2] = canvasResources.uniformData.baseColour[3] = 1;

			while(1) {
				if(layer->type == Layer::LAYER) {
					const ImageBlock::LayerData * layerData = nullptr;
					if(layer->imageFormat == FMT_RGBA) {
						layerData = &block.layersRGBA[rgbaIndex++];
					}
					else if(layer->imageFormat == FMT_RG) {
						layerData = &block.layersRG[rgIndex++];
					}
					else {
						layerData = &block.layersR[rIndex++];
					}

					if(layerData->dataType == ImageBlock::LayerData::SOLID_COLOUR) {
						if((layerData->colour >> 24) == 0xff) {
							ops.clear();
							canvasResources.uniformData.baseColour[0] = (layerData->colour & 0xff) / 255.0f;
							canvasResources.uniformData.baseColour[1] = ((layerData->colour >> 8) & 0xff) / 255.0f;
							canvasResources.uniformData.baseColour[2] = ((layerData->colour >> 16) & 0xff) / 255.0f;
							canvasResources.uniformData.baseColour[3] = ((layerData->colour >> 24) & 0xff) / 255.0f;
						}
						else {
							Op op;
							op.opType = 1; // Overlay colour (normal blend mode)

							op.colour[0] = (layerData->colour & 0xff) / 255.0f;
							op.colour[1] = ((layerData->colour >> 8) & 0xff) / 255.0f;
							op.colour[2] = ((layerData->colour >> 16) & 0xff) / 255.0f;
							op.colour[3] = ((layerData->colour >> 24) & 0xff) / 255.0f;
							ops.push_back(op);
						}
					}
					else {
						Op op;
						op.opType = 2 + layer->imageFormat;
						op.colour[0] = layer->imageFormatSpecificIndex + 0.1f;
						ops.push_back(op);
					}

					if(layer == canvasResources.activeLayer && canvasResources.penDown && block.hasStrokeData) {
						// Stroke must be overlayed
						Op op;
						op.opType = 4;

						if(canvasResources.activeLayer->imageFormat == FMT_RGBA) {
							op.colour[0] = canvasResources.activeColour[0];
							op.colour[1] = canvasResources.activeColour[1];
							op.colour[2] = canvasResources.activeColour[2];
						}
						else if(canvasResources.activeLayer->imageFormat == FMT_RG) {
							op.colour[0] = canvasResources.activeColour[0];
							op.colour[1] = canvasResources.activeColour[0];
							op.colour[2] = canvasResources.activeColour[0];
						}
						else {
							op.colour[0] = 1;
							op.colour[1] = 1;
							op.colour[2] = 1;
						}
						op.colour[3] = canvasResources.activeColour[3];

						ops.push_back(op);
					}
				}


				if(layer->firstChild) {
					assert(layer->type == Layer::LAYER);
					layer = layer->firstChild;
				}
				else {
					if(layer->next) {
						layer = layer->next;
					}
					else if(layer->parent && layer->parent->next) {
						layer = layer->parent->next;
					}
					else {
						break;
					}
				}
			}
			Op op = {};
			ops.push_back(op);

			memcpy(canvasResources.uniformData.ops, ops.data(), (ops.size() > 64 ? 64 : ops.size()) * sizeof(Op));

			canvasResources.uniformData.offsetX = (((block.getX() + block.dirtyMinX) / (float)canvasResources.canvasWidth) * 2.0f) - 1.0f;
			canvasResources.uniformData.offsetY = (((1.0f - (block.getY() + block.dirtyMinY + block.dirtyHeight) / (float)canvasResources.canvasHeight) * 2.0f) - 1.0f);
			canvasResources.uniformData.width   = ((block.dirtyWidth / (float)canvasResources.canvasWidth) * 2.0f);
			canvasResources.uniformData.height  = ((block.dirtyHeight / (float)canvasResources.canvasHeight) * 2.0f);

			canvasResources.uniformData.uvX = block.dirtyMinX / (float)image_block_size();
			canvasResources.uniformData.uvY = block.dirtyMinY / (float)image_block_size();
			canvasResources.uniformData.uvWidth = block.dirtyWidth / (float)image_block_size();
			canvasResources.uniformData.uvHeight = block.dirtyHeight / (float)image_block_size();

			glActiveTexture(GL_TEXTURE0);

			if(block.arrayTextureRGBA.isCreated()) {
				block.arrayTextureRGBA.bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glActiveTexture(GL_TEXTURE1);

			if(block.arrayTextureRG.isCreated()) {
				block.arrayTextureRG.bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glActiveTexture(GL_TEXTURE2);

			if(block.arrayTextureR.isCreated()) {
				block.arrayTextureR.bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glBindBuffer(GL_UNIFORM_BUFFER, canvasResources.ubo);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), &canvasResources.uniformData, GL_DYNAMIC_DRAW);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}

	// Draw canvas texture on screen


	bind_default_framebuffer();
	bind_shader_program(canvasResources.canvasTextureShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	canvasResources.canvasFrameBuffer.bindTexture();

	glm::mat4 m = 
		glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3(
			(float)((int)uiCanvasX + canvasResources.canvasX),
			(float)((int)uiCanvasY + canvasResources.canvasY),
		0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3((float)canvasResources.canvasWidth * canvasResources.canvasZoom, (float)canvasResources.canvasHeight * canvasResources.canvasZoom, 1.0))
		* glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f))
	;

	glUniformMatrix4fv(canvasResources.canvasTextureMatrixLocation, 1, GL_FALSE, &m[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	canvasResources.canvasDirty = false;

}
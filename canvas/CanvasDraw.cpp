#include <Canvas.h>
#include <Layer.h>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.h>
#include "CanvasState.h"

using namespace std;

extern CanvasResources canvasResources;

void Canvas::draw() {
	unsigned int windowWidth, windowHeight;
	UI::get_window_dimensions(windowWidth, windowHeight);

	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	glBindVertexArray(canvasResources.vaoId);
	if(canvasDirty) {
		bind_shader_program(canvasResources.shaderProgram);
		glDisable(GL_BLEND);
		canvasFrameBuffer->bindFrameBuffer();

		glActiveTexture(GL_TEXTURE3);
		canvasResources.strokeLayer->bindTexture();

		for(ImageBlock & block : imageBlocks) {
			if(!block.dirty) {
				continue;
			}
			block.setClean();

			assert(firstLayer);
			LayerPtr layer = firstLayer;
			vector<Op> ops;

			canvasResources.uniformData.baseColour[0] = canvasResources.uniformData.baseColour[1] = 
			canvasResources.uniformData.baseColour[2] = canvasResources.uniformData.baseColour[3] = 1;

			while(1) {
				if(layer->type == Layer::Type::LAYER && layer->visible) {
					const ImageBlock::LayerData * layerData = nullptr;
					for(ImageBlock::LayerData const& ld : block.getLayerData()) {
						if(ld.layer == layer) {
							layerData = &ld;
						}
					}

					assert(layerData);


					if(layerData->dataType == ImageBlock::LayerData::DataType::SOLID_COLOUR) {
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
						op.opType = 2 + (int)layer->imageFormat;
						op.colour[0] = layerData->arrayTextureIndex + 0.1f;
						ops.push_back(op);
					}

					if(layer == activeLayer && block.hasStrokeData) {
						// Stroke must be overlayed
						Op op;
						op.opType = 8;

						if(activeLayer->imageFormat == ImageFormat::FMT_R) {
							op.colour[0] = 1;
							op.colour[1] = 1;
							op.colour[2] = 1;
						}
						else {
							op.colour[0] = canvasResources.activeColour[0];
							op.colour[1] = canvasResources.activeColour[1];
							op.colour[2] = canvasResources.activeColour[2];
						}
						op.colour[3] = canvasResources.activeColour[3];

						ops.push_back(op);
					}
				}


				if(!(layer = layer->getNext())) {
					break;
				}
			}

			Op op = {};
			ops.push_back(op);

			if(block.hasStrokeData) {
				canvasResources.uniformData.ops[0].opType = 7;

				memcpy(&canvasResources.uniformData.ops[1], ops.data(), (ops.size() > 63 ? 63 : ops.size()) * sizeof(Op));
			}
			else {
				memcpy(canvasResources.uniformData.ops, ops.data(), (ops.size() > 64 ? 64 : ops.size()) * sizeof(Op));
			}


			canvasResources.uniformData.offsetX = (((block.getX() + block.getDirtyMinX()) / (float)canvasWidth) * 2.0f) - 1.0f;
			canvasResources.uniformData.offsetY = (((1.0f - (block.getY() + block.getDirtyMinY() + block.getDirtyHeight()) / (float)canvasHeight) * 2.0f) - 1.0f);
			canvasResources.uniformData.width   = ((block.getDirtyWidth() / (float)canvasWidth) * 2.0f);
			canvasResources.uniformData.height  = ((block.getDirtyHeight() / (float)canvasHeight) * 2.0f);

			canvasResources.uniformData.uvX = block.getDirtyMinX() / (float)image_block_size();
			canvasResources.uniformData.uvY = block.getDirtyMinY() / (float)image_block_size();
			canvasResources.uniformData.uvWidth = block.getDirtyWidth() / (float)image_block_size();
			canvasResources.uniformData.uvHeight = block.getDirtyHeight() / (float)image_block_size();

			glActiveTexture(GL_TEXTURE0);

			if(block.getArrayTextureRGBA()) {
				block.getArrayTextureRGBA()->bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glActiveTexture(GL_TEXTURE1);

			if(block.getArrayTextureRG()) {
				block.getArrayTextureRG()->bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glActiveTexture(GL_TEXTURE2);

			if(block.getArrayTextureR()) {
				block.getArrayTextureR()->bind();
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
	canvasFrameBuffer->bindTexture();

	glm::mat4 m = 
		glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3(
			(float)((int)uiCanvasX + canvasX),
			(float)((int)uiCanvasY + canvasY),
		0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3((float)canvasWidth * canvasZoom, (float)canvasHeight * canvasZoom, 1.0))
		* glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f))
	;

	glUniformMatrix4fv(canvasResources.canvasTextureMatrixLocation, 1, GL_FALSE, &m[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);


	canvasDirty = false;

}
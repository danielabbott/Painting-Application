#include <Canvas.h>
#include <Layer.h>
#include <vector>
#include <ImageBlock.h>
#include <Shader.h>
#include <cassert>
#include <UI.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Brush.h>
#include <Window.h>
#include <hwcaps.h>
#include "CanvasState.h"

using namespace std;

Brush testBrush;

CanvasResources canvasResources;

Layer * Layer::getNext()
{
	if(firstChild) {
		assert(type != Layer::Type::LAYER);
		return firstChild;
	}
	else {
		if(next) {
			return next;
		}
		else if(parent && parent->next) {
			return parent->next;
		}
		else {
			return nullptr;
		}
	}
}

Layer * Canvas::get_first_layer() const
{
	return firstLayer;
}

void Canvas::set_active_layer(Layer * layer)
{
	// TODO have layer groups selectable just don't allow drawing on them
	assert(layer->type == Layer::Type::LAYER);
	activeLayer = layer;
}

Layer * Canvas::get_active_layer() const
{
	return activeLayer;
}


void Canvas::widgetCoordsToCanvasCoords(unsigned int cursorX, unsigned int cursorY, int & x, int & y)
{
	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	float canvasOnscreenWidth = canvasWidth * canvasZoom;
	float canvasOnscreenHeight = canvasHeight * canvasZoom;

	x = ((((int)cursorX - canvasX) / canvasOnscreenWidth) + 0.5f) * (int)canvasWidth;
	y = ((((int)cursorY - canvasY) / canvasOnscreenHeight) + 0.5f) * (int)canvasHeight;
}


bool Canvas::onMouseButtonReleasedOutsideWidget(unsigned int button)
{
	return onMouseButtonReleased(button);
}

bool Canvas::onMouseButtonReleased(unsigned int button)
{
	if(button == 0) {
		clog << "Pen released" << endl;
		penDown = false;

		// Stylus was lifted up, merge the stroke layer with the active layer and clear the stroke layer

		// Bind the image-format-specific shader program and temporary framebuffer

		if(activeLayer->imageFormat == ImageFormat::FMT_RGBA) {
			bind_shader_program(canvasResources.strokeMergeShaderProgramRGBA);
			glUniform4f(canvasResources.strokeMergeColourLocationRGBA, canvasResources.activeColour[0], canvasResources.activeColour[1], 
				canvasResources.activeColour[2], canvasResources.activeColour[3]);
			canvasResources.imageBlockTempLayerRGBA->bindFrameBuffer();
		}
		else if(activeLayer->imageFormat == ImageFormat::FMT_RG) {
			bind_shader_program(canvasResources.strokeMergeShaderProgramRG);
			glUniform2f(canvasResources.strokeMergeColourLocationRG, canvasResources.activeColour[0], canvasResources.activeColour[3]);
			canvasResources.imageBlockTempLayerRG->bindFrameBuffer();
		}
		else {
			bind_shader_program(canvasResources.strokeMergeShaderProgramR);
			glUniform1f(canvasResources.strokeMergeColourLocationR, canvasResources.activeColour[3]);
			canvasResources.imageBlockTempLayerR->bindFrameBuffer();
		}

		glActiveTexture(GL_TEXTURE0);
		canvasResources.strokeLayer->bindTexture();

		// glDisable(GL_BLEND);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);

		glBindVertexArray(canvasResources.vaoId);

		glActiveTexture(GL_TEXTURE1);
		for(ImageBlock & block : imageBlocks) {
			if(block.hasStrokeData) {
				block.bindTexture(activeLayer);

				// Set area to merge and active layer index

				// TODO: Merge only dirty region then copy only dirty region

				// float strokeImageX = (((block.getX() + block.getDirtyMinX()) / (float)canvasWidth) * 2.0f) - 1.0f;
				// float strokeImageY = (((1.0f - (block.getY() + block.getDirtyMinY() + block.getDirtyHeight()) / (float)canvasHeight) * 2.0f) - 1.0f);
				// float strokeImageWidth  = ((block.getDirtyWidth() / (float)canvasWidth) * 2.0f);
				// float strokeImageHeight = ((block.getDirtyHeight() / (float)canvasHeight) * 2.0f);

				float strokeImageX = (block.getX() / (float)canvasWidth);
				float strokeImageY = 1.0f - (block.getY() + image_block_size()) / (float)canvasHeight;
				float strokeImageWidth  = image_block_size() / (float)canvasWidth;
				float strokeImageHeight = image_block_size() / (float)canvasHeight;




				if (activeLayer->imageFormat == ImageFormat::FMT_RGBA) {
					glUniform4f(canvasResources.strokeMergeCoordsLocationRGBA, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);
					glUniform1f(canvasResources.strokeMergeIndexLocationRGBA, block.indexOf(activeLayer));
				}
				else if (activeLayer->imageFormat == ImageFormat::FMT_RG) {
					glUniform4f(canvasResources.strokeMergeCoordsLocationRG, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);
					glUniform1f(canvasResources.strokeMergeIndexLocationRG, block.indexOf(activeLayer));
				}
				else {
					glUniform4f(canvasResources.strokeMergeCoordsLocationR, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);
					glUniform1f(canvasResources.strokeMergeIndexLocationR, block.indexOf(activeLayer));
				}

				// Merge stroke onto layer and store in temporary framebuffer

				glDrawArrays(GL_TRIANGLES, 0, 6);

				// Copy the data from the temporary framebuffer to the image block array texture

				// TODO: Copy only dirty region

				// block.copyTo(activeLayer, block.getDirtyMinX(), block.getDirtyMinY(), block.getDirtyWidth(), block.getDirtyHeight());

				block.copyTo(activeLayer);


				block.hasStrokeData = false;
			}
		}

		canvasResources.strokeLayer->clear();
	}
	else if (button == 2) {
		// Scroll wheel

		panning = false;
	}
	return true;
}



bool Canvas::onClicked(unsigned int button, unsigned int x, unsigned int y)
{
	if(button == 0) {
		clog << "Pen pressed" << endl;
		penDown = true;

		widgetCoordsToCanvasCoords(x, y, prevCanvasCoordX, prevCanvasCoordY);
	}
	else if (button == 2) {
		// Scroll wheel

		panning = true;
		panningPrevCursorX = x;
		panningPrevCursorY = y;
	}
	return false;
}

void Canvas::drawStroke(int canvasXcoord, int canvasYcoord, float pressure, unsigned int size, float alphaMultiply)
{
	// TODO: If brush is not textured and covers entire image block then set strokeDataFillsBlock to true and don't fill
	// remember to account for hardness setting

	glm::mat4 m = 
		glm::ortho(0.0f, (float)canvasWidth, (float)canvasHeight, 0.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3((float)canvasXcoord, (float)canvasYcoord, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3((float)size, (float)size, 1.0f))
	;

	glUniformMatrix4fv(testBrush.matrixUniformLocation, 1, GL_FALSE, &m[0][0]);
	glUniform1f(testBrush.strokeAlphaUniformLocation, pressure*canvasResources.activeColour[3] * alphaMultiply);

	if(testBrush.seedUniformLocation != -1) {
		glUniform1f(testBrush.seedUniformLocation, (rand() % 200) * 1.2f);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);


	// Set flag in appropriate image block(s)

	for(ImageBlock & block : imageBlocks) {
		if(block.dirtyRegion(canvasXcoord-size/2, canvasYcoord-size/2, size, size)) {
			block.hasStrokeData = true;
		}
	}

}

static bool useMouse = false;

void set_canvas_input_device(bool mouse)
{
	useMouse = mouse;
}

bool Canvas::onMouseMoved(unsigned int cursorX, unsigned int cursorY, float pressure)
{
	if(panning) {
		canvasX += (int)cursorX - (int)panningPrevCursorX;
		canvasY += (int)cursorY - (int)panningPrevCursorY;

		panningPrevCursorX = cursorX;
		panningPrevCursorY = cursorY;

		return true;
	}
	if(!penDown) {
		return false;
	}

	if(useMouse || !tablet_detected()) {
		pressure = 1;
	}
	else {
		if(pressure < 0) {
			// Ignore mouse input (tablet events may be duplicated as mouse events)
			return false;
		}
	}

	unsigned int size = 15;


	bind_shader_program(testBrush.shaderProgram);

	// Hardness value is divided by 2 because the hardness value in the shader is ranged 0-0.5 whereas
	// the hardness value in the brush struct is ranged 0-1
	glUniform1f(testBrush.hardnessUniformLocation, testBrush.hardness / 2.0f);


	glEnable(GL_BLEND);

	// if a stroke overlaps itself it will never make itself lighter
	glBlendFunc(GL_ONE, GL_ONE);

	if(testBrush.blendMode == Brush::BlendMode::MAX) {
		glBlendEquation(GL_MAX);
	}

	canvasResources.strokeLayer->bindFrameBuffer();
	glBindVertexArray(canvasResources.brushVaoId);

	//Reduce opacity when using add blend mode
	float alphaMultiply = testBrush.blendMode == Brush::BlendMode::ADD ? 0.2f : 1.0f;

	int canvasXcoord;
	int canvasYcoord;
	widgetCoordsToCanvasCoords(cursorX, cursorY, canvasXcoord, canvasYcoord);

	int diffX = canvasXcoord - prevCanvasCoordX;
	int diffY = canvasYcoord - prevCanvasCoordY;

	float distance = sqrt(diffX*diffX + diffY*diffY);
	float increment = size / 8 / distance;

	for(float mul = increment; mul < 1.0f; mul += increment) {
		drawStroke(prevCanvasCoordX + (int)(diffX * mul), prevCanvasCoordY + (int)(diffY * mul), pressure, size, alphaMultiply);
	}

	drawStroke(canvasXcoord, canvasYcoord, pressure, size, alphaMultiply);

	prevCanvasCoordX = canvasXcoord;
	prevCanvasCoordY = canvasYcoord;

	if(testBrush.blendMode != Brush::BlendMode::ADD) {
		glBlendEquation(GL_FUNC_ADD);
	}

	canvasDirty = true;
	return true;

}

bool Canvas::onScroll(unsigned int x, unsigned int y, int direction)
{	
	// Get canvas coordinates of mouse 
	int targetCanvasX, targetCanvasY;
	widgetCoordsToCanvasCoords(x, y, targetCanvasX, targetCanvasY);

	if(direction > 0) {
		canvasZoom *= direction*2;
	}
	else {
		canvasZoom /= -direction*2;
	}

	float smallestZoom = 8 / (float)min(canvasWidth, canvasHeight);

	if(canvasZoom < smallestZoom) {
		canvasZoom = smallestZoom;
	}

	if(canvasZoom > 30) {
		canvasZoom = 30;
	}

	// Position the canvas so that the point the mouse was over before zooming
	// is still under the mouse after zooming

	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	float distanceX = (targetCanvasX - (int)canvasWidth/2) * canvasZoom;
	float distanceY = (targetCanvasY - (int)canvasHeight/2) * canvasZoom;

	canvasX = (int)x - distanceX;
	canvasY = (int)y - distanceY;

	return true;
}

void Canvas::clear_layer(Layer * layer)
{
	for(ImageBlock & block : imageBlocks) {
		block.dirtyRegion(block.getX(), block.getY(), image_block_size(), image_block_size());
		block.fillLayer(layer, 0);
	}
	canvasDirty = true;
}


void Canvas::fill_layer(Layer * layer, uint32_t colour)
{
	for(ImageBlock & block : imageBlocks) {
		block.dirtyRegion(block.getX(), block.getY(), image_block_size(), image_block_size());
		block.fillLayer(layer, colour);
	}
	canvasDirty = true;
}

void set_active_colour(float r, float g, float b, float a)
{
	canvasResources.activeColour[0] = r;
	canvasResources.activeColour[1] = g;
	canvasResources.activeColour[2] = b;
	canvasResources.activeColour[3] = a;
}

void Canvas::forceRedraw()
{
	for(ImageBlock & block : imageBlocks) {
		block.dirtyRegion(block.getX(), block.getY(), image_block_size(), image_block_size());
	}
	canvasDirty = true;
}

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


CanvasResources canvasResources;

Brush testBrush;

static Layer layers[2];
void create_layers()
{


	layers[0].type = Layer::LAYER;
	layers[0].name = "bottom layer";
	layers[1].type = Layer::LAYER;
	layers[1].name = "top layer";

	canvasResources.firstLayer = &layers[0];
	canvasResources.activeLayer = &layers[1];
	layers[0].next = &layers[1];

	canvasResources.activeColour[0] = 1;
	canvasResources.activeColour[1] = 0;
	canvasResources.activeColour[2] = 0;
	canvasResources.activeColour[3] = 0.6f;
}

Layer * get_first_layer()
{
	return canvasResources.firstLayer;
}

void set_active_layer(Layer * layer)
{
	// TODO have layer groups selectable just don't allow drawing on them
	assert(layer->type == Layer::LAYER);
	canvasResources.activeLayer = layer;
}

Layer * get_active_layer()
{
	return canvasResources.activeLayer;
}


void Canvas::widgetCoordsToCanvasCoords(unsigned int cursorX, unsigned int cursorY, int & x, int & y)
{
	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	float canvasOnscreenWidth = canvasResources.canvasWidth * canvasResources.canvasZoom;
	float canvasOnscreenHeight = canvasResources.canvasHeight * canvasResources.canvasZoom;

	x = ((((int)cursorX - canvasResources.canvasX) / canvasOnscreenWidth) + 0.5f) * (int)canvasResources.canvasWidth;
	y = ((((int)cursorY - canvasResources.canvasY) / canvasOnscreenHeight) + 0.5f) * (int)canvasResources.canvasHeight;
}


bool Canvas::onMouseButtonReleasedOutsideWidget(unsigned int button)
{
	return onMouseButtonReleased(button);
}

bool Canvas::onMouseButtonReleased(unsigned int button)
{
	if(button == 0) {
		clog << "Pen released" << endl;
		canvasResources.penDown = false;

		// Stylus was lifted up, merge the stroke layer with the active layer and clear the stroke layer

		if(canvasResources.activeLayer->imageFormat == FMT_RGBA) {
			bind_shader_program(canvasResources.strokeMergeShaderProgramRGBA);
			glUniform4f(canvasResources.strokeMergeColourLocationRGBA, canvasResources.activeColour[0], canvasResources.activeColour[1], 
				canvasResources.activeColour[2], canvasResources.activeColour[3]);
			canvasResources.imageBlockTempLayerRGBA.bindFrameBuffer();
		}
		else if(canvasResources.activeLayer->imageFormat == FMT_RG) {
			bind_shader_program(canvasResources.strokeMergeShaderProgramRG);
			glUniform2f(canvasResources.strokeMergeColourLocationRG, canvasResources.activeColour[0], canvasResources.activeColour[3]);
			canvasResources.imageBlockTempLayerRG.bindFrameBuffer();
		}
		else {
			bind_shader_program(canvasResources.strokeMergeShaderProgramR);
			glUniform1f(canvasResources.strokeMergeColourLocationR, canvasResources.activeColour[3]);
			canvasResources.imageBlockTempLayerR.bindFrameBuffer();
		}

		glActiveTexture(GL_TEXTURE0);
		canvasResources.strokeLayer.bindTexture();
		glDisable(GL_BLEND);
		glBindVertexArray(canvasResources.vaoId);

		glActiveTexture(GL_TEXTURE1);
		for(ImageBlock & block : canvasResources.imageBlocks) {
			if(block.hasStrokeData) {
				block.bindTexture(canvasResources.activeLayer);

				float strokeImageX = (block.getX() / (float)canvasResources.canvasWidth);
				float strokeImageY = 1.0f - (block.getY() + image_block_size()) / (float)canvasResources.canvasHeight;
				float strokeImageWidth  = image_block_size() / (float)canvasResources.canvasWidth;
				float strokeImageHeight = image_block_size() / (float)canvasResources.canvasHeight;


				if(canvasResources.activeLayer->imageFormat == FMT_RGBA) {
					glUniform4f(canvasResources.strokeMergeCoordsLocationRGBA, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);
					glUniform1f(canvasResources.strokeMergeIndexLocationRGBA, block.indexOf(canvasResources.activeLayer));
				}
				else if(canvasResources.activeLayer->imageFormat == FMT_RG) {
					glUniform4f(canvasResources.strokeMergeCoordsLocationRG, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);
					glUniform1f(canvasResources.strokeMergeIndexLocationRG, block.indexOf(canvasResources.activeLayer));
				}
				else {
					glUniform4f(canvasResources.strokeMergeCoordsLocationR, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);
					glUniform1f(canvasResources.strokeMergeIndexLocationR, block.indexOf(canvasResources.activeLayer));
				}

				glDrawArrays(GL_TRIANGLES, 0, 6);

				block.copyTo(canvasResources.activeLayer);

				block.hasStrokeData = false;
			}
		}

		canvasResources.strokeLayer.clear();
	}
	else if (button == 2) {
		// Scroll wheel

		canvasResources.panning = false;
	}
	return true;
}



bool Canvas::onClicked(unsigned int button, unsigned int x, unsigned int y)
{
	if(button == 0) {
		clog << "Pen pressed" << endl;
		canvasResources.penDown = true;

		widgetCoordsToCanvasCoords(x, y, canvasResources.prevCanvasCoordX, canvasResources.prevCanvasCoordY);
	}
	else if (button == 2) {
		// Scroll wheel

		canvasResources.panning = true;
		canvasResources.panningPrevCursorX = x;
		canvasResources.panningPrevCursorY = y;
	}
	return false;
}

void drawStroke(int canvasXcoord, int canvasYcoord, float pressure, unsigned int size)
{
	glm::mat4 m = 
		glm::ortho(0.0f, (float)canvasResources.canvasWidth, (float)canvasResources.canvasHeight, 0.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3((float)canvasXcoord, (float)canvasYcoord, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3((float)size, (float)size, 1.0f))
	;

	glUniformMatrix4fv(testBrush.matrixUniformLocation, 1, GL_FALSE, &m[0][0]);
	glUniform1f(testBrush.strokeAlphaUniformLocation, pressure*canvasResources.activeColour[3]);

	glDrawArrays(GL_TRIANGLES, 0, 6);


	// Set flag in appropriate image block(s)

	for(ImageBlock & block : canvasResources.imageBlocks) {
		if(block.dirtyRegion(canvasXcoord-size/2, canvasYcoord-size/2, size, size)) {
			block.hasStrokeData = true;
		}
	}

}

bool useMouse = false;

void set_canvas_input_device(bool mouse)
{
	useMouse = mouse;
}

bool Canvas::onMouseMoved(unsigned int cursorX, unsigned int cursorY, float pressure)
{
	if(canvasResources.panning) {
		canvasResources.canvasX += (int)cursorX - (int)canvasResources.panningPrevCursorX;
		canvasResources.canvasY += (int)cursorY - (int)canvasResources.panningPrevCursorY;

		canvasResources.panningPrevCursorX = cursorX;
		canvasResources.panningPrevCursorY = cursorY;

		return true;
	}
	if(!canvasResources.penDown) {
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

	unsigned int size = 1000;


	bind_shader_program(testBrush.shaderProgram);


	glEnable(GL_BLEND);

	// if a stroke overlaps itself it will never make itself lighter
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_MAX);

	canvasResources.strokeLayer.bindFrameBuffer();
	glBindVertexArray(canvasResources.brushVaoId);

	int canvasXcoord;
	int canvasYcoord;
	widgetCoordsToCanvasCoords(cursorX, cursorY, canvasXcoord, canvasYcoord);


	int diffX = canvasXcoord - canvasResources.prevCanvasCoordX;
	int diffY = canvasYcoord - canvasResources.prevCanvasCoordY;

	float distance = sqrt(diffX*diffX + diffY*diffY);
	float increment = size / 8 / distance;

	for(float mul = increment; mul < 1.0f; mul += increment) {
		drawStroke(canvasResources.prevCanvasCoordX + (int)(diffX * mul), canvasResources.prevCanvasCoordY + (int)(diffY * mul), pressure, size);
	}

	drawStroke(canvasXcoord, canvasYcoord, pressure, size);

	canvasResources.prevCanvasCoordX = canvasXcoord;
	canvasResources.prevCanvasCoordY = canvasYcoord;


	glBlendEquation(GL_FUNC_ADD);

	canvasResources.canvasDirty = true;
	return true;

}

bool Canvas::onScroll(unsigned int x, unsigned int y, int direction)
{	
	// Get canvas coordinates of mouse 
	int targetCanvasX, targetCanvasY;
	widgetCoordsToCanvasCoords(x, y, targetCanvasX, targetCanvasY);

	if(direction > 0) {
		canvasResources.canvasZoom *= direction*2;
	}
	else {
		canvasResources.canvasZoom /= -direction*2;
	}

	float smallestZoom = 8 / (float)min(canvasResources.canvasWidth, canvasResources.canvasHeight);

	if(canvasResources.canvasZoom < smallestZoom) {
		canvasResources.canvasZoom = smallestZoom;
	}

	if(canvasResources.canvasZoom > 30) {
		canvasResources.canvasZoom = 30;
	}

	// Position the canvas so that the point the mouse was over before zooming
	// is still under the mouse after zooming

	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	float distanceX = (targetCanvasX - (int)canvasResources.canvasWidth/2) * canvasResources.canvasZoom;
	float distanceY = (targetCanvasY - (int)canvasResources.canvasHeight/2) * canvasResources.canvasZoom;

	canvasResources.canvasX = (int)x - distanceX;
	canvasResources.canvasY = (int)y - distanceY;

	return true;
}

void clear_layer(Layer * layer)
{
	for(ImageBlock & block : canvasResources.imageBlocks) {
		block.dirtyRegion(block.getX(), block.getY(), image_block_size(), image_block_size());
		block.fillLayer(layer, 0);
	}
	canvasResources.canvasDirty = true;
}

#pragma once

#include <glad/glad.h>
#include <UI.h>
#include <Layer.h>
#include <deque>
#include <memory>
#include <FrameBuffer.h>
#include <ImageBlock.h>


void free_canvas_resources();

GLuint get_brush_vao();

class Canvas;
class CanvasViewPort : public UI::Canvas
{
	::Canvas * canvas;

	int canvasX = 0;
	int canvasY = 0;

	float canvasZoom = 0.08f;
	
	bool panning = false;
	unsigned int panningPrevCursorX, panningPrevCursorY;

	bool penDown = false;

	// Coordinates of previous cursor position (in canvas coordinate space)
	int prevCanvasCoordX;
	int prevCanvasCoordY;

	void widgetCoordsToCanvasCoords(unsigned int cursorX, unsigned int cursorY, int & x, int & y);

public:

	CanvasViewPort(::Canvas * c, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

	::Canvas * getCanvas();

	void setCanvasPosition(int x, int y);
	void setCanvasZoom(float zoom);

	int getCanvasPositionX();
	int getCanvasPositionY();
	float getCanvasZoom();

	virtual void draw() override;
	virtual bool onClicked(unsigned int button, unsigned int x, unsigned int y) override;
	virtual bool onMouseButtonReleased(unsigned int button) override;
	virtual bool onMouseButtonReleasedOutsideWidget(unsigned int button) override;
	virtual bool onMouseMoved(unsigned int x, unsigned int y, float pressure) override;
	virtual bool onScroll(unsigned int x, unsigned int y, int direction) override;


};

class Canvas
{

	FrameBuffer * canvasFrameBuffer;

	std::deque<ImageBlock> imageBlocks;

	unsigned int canvasWidth = 7680;
	unsigned int canvasHeight = 4320;

	LayerPtr firstLayer = nullptr;
	LayerPtr activeLayer = nullptr;

	// True if the canvas texture needs to be regenerated
	bool canvasDirty = true;

	
	void drawStroke(int canvasXcoord, int canvasYcoord, float pressure, unsigned int size, float alphaMultiply);

	void createOpenGLImages();

	// TODO Remove this. This is just for testing things
	void create_layers();

	void initialiseCanvasDisplay(unsigned int x, unsigned int y);

public:
	Canvas(unsigned int width, unsigned int height);

	unsigned int getWidth();
	unsigned int getHeight();


	LayerPtr getFirstLayer() const;
	void setActiveLayer(LayerPtr layer);
	LayerPtr getActiveLayer() const;
	void clearLayer(LayerPtr layer);
	void fillLayer(LayerPtr layer, uint32_t colour);

	// Sets dirty flag on all image blocks
	// The canvas will therefore be completely redrawn when draw() is next called
	void forceRedraw();

	// 'draw' means to flatten the layers into an image that can then be used as a texture to show on-screen
	void draw();

	void bindFrameBufferTexture();


	// Returns true if canvas needs to be redrawn
	bool stroke(unsigned int prevY, unsigned int prevX, unsigned int x, unsigned int y, float pressure);
	bool finaliseStroke();


	// This does not delete the layer object or it's image data
	// The layer's children will also be removed
	void removeLayer(LayerPtr layer);

	void addLayerAfter(LayerPtr layer, LayerPtr newLayer);
	void addLayerBefore(LayerPtr layer, LayerPtr newLayer);
	// TODO: add_layer_as_first_child

	void addLayer(LayerPtr layer);


	void freeCanvasResources();
};


// false for tablet
void set_canvas_input_device(bool mouse);

void set_active_colour(float r, float g, float b, float a);

void free_canvas_resources();

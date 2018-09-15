#pragma once

#include <glad/glad.h>
#include <UI.h>
#include <Layer.h>
#include <deque>
#include <FrameBuffer.h>
#include <ImageBlock.h>


void free_canvas_resources();

GLuint get_brush_vao();

class Canvas : public UI::Canvas
{

	FrameBuffer * canvasFrameBuffer;

	std::deque<ImageBlock> imageBlocks;

	unsigned int canvasWidth = 7680;
	unsigned int canvasHeight = 4320;
	float canvasZoom = 0.08f;

	int canvasX = 0;
	int canvasY = 0;
	
	bool panning = false;
	unsigned int panningPrevCursorX, panningPrevCursorY;

	Layer * firstLayer = nullptr;
	Layer * activeLayer = nullptr;

	bool penDown = false;
	
	// Coordinates of previous cursor position (in canvas coordinate space)
	int prevCanvasCoordX;
	int prevCanvasCoordY;

	// True if the canvas texture needs to be regenerated
	bool canvasDirty = true;

	void widgetCoordsToCanvasCoords(unsigned int cursorX, unsigned int cursorY, int & x, int & y);
	void drawStroke(int canvasXcoord, int canvasYcoord, float pressure, unsigned int size, float alphaMultiply);

	void createOpenGLImages();


public:
	Canvas(unsigned int width, unsigned int height)
		: Canvas(0, 0, width, height) {}

	Canvas(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

	Layer * get_first_layer() const;
	void set_active_layer(Layer * layer);
	Layer * get_active_layer() const;
	void clear_layer(Layer * layer);
	void fill_layer(Layer * layer, uint32_t colour);

	void initialise_canvas_display(unsigned int x, unsigned int y);

	void freeCanvasResources();

	virtual void draw() override;
	virtual bool onClicked(unsigned int button, unsigned int x, unsigned int y) override;
	virtual bool onMouseButtonReleased(unsigned int button) override;
	virtual bool onMouseButtonReleasedOutsideWidget(unsigned int button) override;
	virtual bool onMouseMoved(unsigned int x, unsigned int y, float pressure) override;
	virtual bool onScroll(unsigned int x, unsigned int y, int direction) override;
};


// false for tablet
void set_canvas_input_device(bool mouse);

void set_active_colour(float r, float g, float b, float a);

void free_canvas_resources();

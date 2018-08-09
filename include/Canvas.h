#pragma once

#include <glad/glad.h>
#include <UI.h>
#include <Layer.h>

void initialise_canvas_display(unsigned int x, unsigned int y);

void free_canvas_resources();

GLuint get_brush_vao();

class Canvas : public UI::Canvas
{
	void cursorCoordsToCanvasCoords(unsigned int cursorX, unsigned int cursorY, int & x, int & y);

public:
	Canvas(unsigned int width, unsigned int height)
		: UI::Canvas(0, 0, width, height) {}

	Canvas(unsigned int x, unsigned int y, unsigned int width, unsigned int height) 
		: UI::Canvas(x, y, width, height) {}

	virtual void draw() override;
	virtual bool onClicked(unsigned int button, unsigned int x, unsigned int y) override;
	virtual bool onMouseButtonReleased(unsigned int button) override;
	virtual bool onMouseButtonReleasedOutsideWidget(unsigned int button) override;
	virtual bool onMouseMoved(unsigned int x, unsigned int y, float pressure) override;
	virtual bool onScroll(unsigned int x, unsigned int y, int direction) override;
};

void set_active_layer(Layer * layer);
Layer * get_active_layer();
Layer * get_first_layer();

// false for tablet
void set_canvas_input_device(bool mouse);

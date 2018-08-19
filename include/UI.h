#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace UI {

// All widgets are immutable - once created they cannot be modified.

class Widget;
class Container;
class Label;
class Button;
class Canvas;

class Widget {
	friend Container;
	friend bool draw_ui(bool);
	friend void setAbsWindowCoords(Container *, unsigned int, unsigned int);

public:

	enum class LeftRightAlignment {
		LEFT, LEFT_RIGHT_CENTRE, RIGHT
	};
	enum class TopBottomAlignment {
		TOP, TOP_BOTTOM_CENTRE, BOTTOM
	};

	// Relative to the top-left of the parent
	unsigned int actualX = 0, actualY = 0;

	// Relative to the top-left of the window
	unsigned int actualWindowX = 0, actualWindowY = 0;
	
	unsigned int actualWidth = 0, actualHeight = 0;

	Widget * parent = nullptr;

	LeftRightAlignment leftRightTextAlign;
	TopBottomAlignment topBottomTextAlign;

protected:
	// Preferred position
	// These may be taken into account depending on the widget arrangement being used
	unsigned int x = 0, y = 0;

	// Preferred size
	// Some widgets use these (containers, canvas)
	// Labels and buttons ignore these and set their size to match their text
	unsigned int w = 0, h = 0;

public:

	Widget();
	Widget(unsigned int x_, unsigned int y_, unsigned int w_, unsigned int h_, LeftRightAlignment leftRightTextAlign = LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment topBottomTextAlign = TopBottomAlignment::TOP_BOTTOM_CENTRE);

	uint32_t getActualX() { return actualX; }
	uint32_t getActualY() { return actualY; }
	uint32_t getActualWindowX() { return actualWindowX; }
	uint32_t getActualWindowY() { return actualWindowY; }
	uint32_t getActualWidth() { return actualWidth; }
	uint32_t getActualHeight() { return actualHeight; }

	Widget * getParent() { return parent; }
	
	// Call this after the widget has been added to a Container
	// Coordinates are relative to the top-left of the window
	void getArea(unsigned int & x_, unsigned int & y_, unsigned int & width, unsigned int & height)
	{
		x_ = actualX;
		y_ = actualY;

		Widget * next = parent;
		while(next) {
			x_ += next->actualX;
			y_ += next->actualY;

			next = next->parent;
		}

		width = actualWidth;
		height = actualHeight;
	}

	void getPosition(unsigned int & x_, unsigned int & y_)
	{
		x_ = actualX;
		y_ = actualY;
	}



	// RGBA (A is MSB, R LSB).
	// If alpha is 0 the background will not be drawn
	virtual uint32_t getBackGroundColour() { return 0; }

	// UTF-8 encoded
	virtual std::string const& getText(); 
	virtual uint32_t getTextColour() { return 0xffffffff; } 

	virtual ~Widget(){}

	// Returns the preferred/minimum dimensions of the widget
	virtual void getDimensions(unsigned int & width, unsigned int & height)
	{
		width = w;
		height = h;
	}


	// Event handles return true if the widget (or any other widget) must now be redrawn

	// Coordinates are relative to the top-left of the widget
	// Except for containers, where they are relative to the top-left of the window
	virtual bool onClicked(unsigned int button, unsigned int x, unsigned int y) { 
		(void)button; (void)x; (void)y;
		return false; 
	}

	virtual bool onMouseButtonReleased(unsigned int button) { (void)button; return false; }

	// Called if the mouse was moved outside of the boundaries of the widget before the button was released
	virtual bool onMouseButtonReleasedOutsideWidget(unsigned int button) { (void)button; return false; }

	// This event only fires when the mouse button is down
	// x,y is the new position of the cursor, relative to the top-left of the widget
	// virtual bool onMouseButtonMoved(unsigned int button, unsigned int x, unsigned int y) { (void)button; (void)x; (void)y; return false; }

	virtual bool onMouseMoved(unsigned int x, unsigned int y, float pressure) { (void)x; (void)y; (void)pressure; return false;}

	// direction value is positive when scrolling up
	virtual bool onScroll(unsigned int mouseX, unsigned int mouseY, int direction) { (void)mouseX; (void)mouseY; (void)direction; return false;}

};

class Container : public Widget {
	friend void setAbsWindowCoords(Container *, unsigned int, unsigned int);

public:

	enum class LayoutManager {
		// Preferred x,y,width,height of widgets are used directly.
		// Preferred width and height of container are used
		NONE,

		// Preferred width,height of widgets are used directly. Preferred x,y ignored.
		// Preferred width and height of container are ignored
		FLOW_ACCROSS,

		// Preferred width,height of widgets are used directly. Preferred x,y ignored.
		// Preferred width and height of container are ignored
		FLOW_DOWN,

		// Preferred x,y determine which border is to be used
		// x/y=0: left/top
		// x/y=1: centre
		// x/y=2: right/bottom
		// Preferred width and height of container are ignored if it is in the centre
		// Note that the order of widgets in the list may affect the positioning of the elements
		// If two widgets could exist in the same corner (e.g. widgets in the left-centre and top-centre positions compete for the top-left corner)
		// then the first widget in the list claims that corner
		BORDER
	};

private:

	friend bool draw_ui(bool);

	std::vector<Widget *> widgets;
	unsigned int colour = 0xff202020;
	std::vector<Container *> childContainers;
	LayoutManager layoutManager;

	void draw(std::vector<Canvas *> & canvases, unsigned int xOffset=0, unsigned int yOffset=0);
	void findCanvases(std::vector<Canvas *> & canvases);

public:
	bool active = true;

	Container(std::vector<Widget *> widgets_, unsigned int x_, unsigned int y_,
		unsigned int width_, unsigned int height_, unsigned int colour_, LayoutManager layoutManager_);

	virtual uint32_t getBackGroundColour() override { return colour; }

	// Returns either w/h variable if w/h is non-zero
	// If w/h is 0, returns minimum size required to fit widgets (depends on layout manager)
	virtual void getDimensions(unsigned int & width, unsigned int & height);

	enum class EventHandlerOutcome {
		NOTHING, CONTAINER_INTERACTED_WITH, WIDGET_INTERACTED_WITH
	};

	// ! x,y, relative to top-left of window, not container (coordinates are normal for the the regular on* functions in Widget)
	// These functions pass the event on to the child containers and child widgets
	bool onClicked_(unsigned int button, unsigned int x, unsigned int y, EventHandlerOutcome & outcome);
	bool onMouseMoved_(unsigned int x, unsigned int y, float pressure, EventHandlerOutcome & outcome);
	bool onScroll_(unsigned int mouseX, unsigned int mouseY, int direction, EventHandlerOutcome & outcome);

	void bake();
};

// For menus, etc. which need to be layered over the rest of the GUI
void set_root_container(Container *);
void set_menu_overlay_root_container(Container *, unsigned int x, unsigned int y);

class Label : public Widget {
protected:
	std::string widgetText;
	unsigned int textWidth;
public:
	Label(std::string text_, unsigned int x, unsigned int y, 
		LeftRightAlignment leftRightTextAlign = LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment topBottomTextAlign = TopBottomAlignment::TOP_BOTTOM_CENTRE);

	Label(std::string text_, LeftRightAlignment leftRightTextAlign = LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment topBottomTextAlign = TopBottomAlignment::TOP_BOTTOM_CENTRE);

	virtual std::string const& getText() override;
	virtual void getDimensions(unsigned int & width, unsigned int & height) override;
};

class Button : public Label {
bool beingClicked = false;
public:
	Button(std::string text_, LeftRightAlignment leftRightTextAlign = LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment topBottomTextAlign = TopBottomAlignment::TOP_BOTTOM_CENTRE);

	virtual uint32_t getBackGroundColour() override;
	virtual bool onClicked(unsigned int button, unsigned int x, unsigned int y) override;
	virtual bool onMouseButtonReleased(unsigned int button) override;
	virtual bool onMouseButtonReleasedOutsideWidget(unsigned int button) override;
};

// Widget that reserves an area of the window for custom drawing
// The application can get the position and dimensions of this area
// and draw directly to that position on the canvas by overriding
// the draw function

class Canvas : public Widget {
friend bool draw_ui(bool);
public:
	Canvas(unsigned int width, unsigned int height);

	Canvas(unsigned int x, unsigned int y, unsigned int width, unsigned int height);


	// Use getArea() or getPosition() to find the location of the canvas in the window
	// Use get_window_dimensions() to get the size of the window 
	// If the function changes the active texture unit it call glActiveTexture(GL_TEXTURE0) before returning
	virtual void draw() {};
};

typedef Container MenuBar;

class Menu : public Container
{
public:
	Menu(std::vector<Widget *> widgets);
};

// class SubMenu : public Menu
// {
// public:
// 	SubMenu(std::vector<Widget *> widgets);
// };

class MenuItem : public Button
{
	Menu * menu;
public:
	MenuItem(std::string text_, UI::Menu * menu_);
	virtual bool onMouseButtonReleased(unsigned int button) override;

};


void initialise_ui();

// Returns true if the UI was redrawn
bool draw_ui(bool forceRedraw);

void get_window_dimensions(unsigned int & width, unsigned int & height);

void free_ui_resources();

}

static inline bool x_in_region(unsigned int x, unsigned int y, unsigned int regionX, unsigned int regionY, unsigned int regionWidth, unsigned int regionHeight)
{
	return x >= regionX && x < regionX + regionWidth
		&& y >= regionY && y < regionY + regionHeight;
}



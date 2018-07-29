#include <UI.h>
#include <iostream>
#include <Font.h>

using namespace std;

namespace UI {

static string defaultText = "";
string const& Widget::getText() 
{ 
	return defaultText; 
}

Container::Container(std::vector<Widget *> widgets_, unsigned int x_, unsigned int y_,
		unsigned int width_, unsigned int height_, unsigned int colour_, LayoutManager layoutManager_) 
		: Widget(x_, y_, width_, height_), widgets(widgets_), colour(colour_), layoutManager(layoutManager_) {}


extern Widget * widgetBeingClicked[3];

bool Container::onClicked_(unsigned int button, unsigned int x, unsigned int y, EventHandlerOutcome & outcome)
{
	if(!active || button > 3) {
		return false;
	}

	onClicked(button, x-actualX, y-actualY);

	for(Container * container : childContainers) {
		if(container->active && x_in_region(x, y, container->actualX, container->actualY, container->actualWidth, container->actualHeight)) {
			outcome = CONTAINER_INTERACTED_WITH;
			if(container->onClicked_(button, x, y, outcome)) {
				return true;
			}
			return false;
		}
	}

	for(Widget * widget : widgets) {
		if(dynamic_cast<Container *>(widget))	continue;
			
		if(x_in_region(x, y, widget->actualX, widget->actualY, widget->actualWidth, widget->actualHeight)) {
			widgetBeingClicked[button] = widget;
			outcome = WIDGET_INTERACTED_WITH;

			if(widget->onClicked(button, x-widget->actualX, y-widget->actualY)) {
				return true;
			}
			return false;
		}
	}

	return false;
}

bool Container::onMouseMoved_(unsigned int x, unsigned int y, float pressure, EventHandlerOutcome & outcome)
{
	if(!active) {
		return false;
	}

	onMouseMoved(x-actualX, y-actualY, pressure);

	for(Container * container : childContainers) {
		if(container->active && x_in_region(x, y, container->actualX, container->actualY, container->actualWidth, container->actualHeight)) {
			outcome = CONTAINER_INTERACTED_WITH;
			if(container->onMouseMoved_(x, y, pressure, outcome)) {
				return true;
			}
			return false;
		}
	}

	for(Widget * widget : widgets) {
		if(dynamic_cast<Container *>(widget))	continue;

		if(x_in_region(x, y, widget->actualX, widget->actualY, widget->actualWidth, widget->actualHeight)) {
			outcome = WIDGET_INTERACTED_WITH;

			if(widget->onMouseMoved(x-widget->actualX, y-widget->actualY, pressure)) {
				return true;
			}
			return false;
		}
	}

	return false;
}

bool Container::onScroll_(unsigned int x, unsigned int y, int scrollY, EventHandlerOutcome & outcome)
{
	if(!active) {
		return false;
	}
	
	onMouseMoved(x-actualX, y-actualY, scrollY);

	for(Container * container : childContainers) {
		if(container->active && x_in_region(x, y, container->actualX, container->actualY, container->actualWidth, container->actualHeight)) {
			outcome = CONTAINER_INTERACTED_WITH;
			if(container->onScroll_(x, y, scrollY, outcome)) {
				return true;
			}
			return false;
		}
	}

	for(Widget * widget : widgets) {
		if(dynamic_cast<Container *>(widget))	continue;

		if(x_in_region(x, y, widget->actualX, widget->actualY, widget->actualWidth, widget->actualHeight)) {
			outcome = WIDGET_INTERACTED_WITH;

			if(widget->onScroll(x-widget->actualX, y-widget->actualY, scrollY)) {
				return true;
			}
			return false;
		}
	}

	return false;
}

Button::Button(std::string text_) : Label(text_) {}

Canvas::Canvas(unsigned int width, unsigned int height) : Widget(0, 0, width, height) {}

Canvas::Canvas(unsigned int x, unsigned int y, unsigned int width, unsigned int height) : Widget(x, y, width, height) {}

}


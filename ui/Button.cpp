#include <UI.h>
#include <iostream>

using namespace std;

namespace UI {

Button::Button(std::string text_, LeftRightAlignment leftRightTextAlign_, TopBottomAlignment topBottomTextAlign_)
: Label(text_, leftRightTextAlign_, topBottomTextAlign_) 
{}

uint32_t Button::getBackGroundColour()
{
	return beingClicked ? 0x30000000 : 0;
}

bool Button::onClicked(unsigned int button, unsigned int x, unsigned int y)
{
	(void)button;
	(void)x;
	(void)y;
	clog << "Button press: " << widgetText << endl;
	beingClicked = true;
	return true;
}


bool Button::onMouseButtonReleased(unsigned int button)
{
	(void)button;
	clog << "Button click released: " << widgetText << endl;
	beingClicked = false;
	return true;
}

bool Button::onMouseButtonReleasedOutsideWidget(unsigned int button)
{
	(void)button;
	clog << "Button click released (outside of widget): " << widgetText << endl;
	beingClicked = false;
	return true;
}

}

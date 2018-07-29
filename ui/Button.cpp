#include <UI.h>

namespace UI {

uint32_t Button::getBackGroundColour()
{
	return beingClicked ? 0x30000000 : 0;
}

bool Button::onClicked(unsigned int button, unsigned int x, unsigned int y)
{
	(void)button;
	(void)x;
	(void)y;
	beingClicked = true;
	return true;
}


bool Button::onMouseButtonReleased(unsigned int button)
{
	(void)button;
	beingClicked = false;
	return true;
}

bool Button::onMouseButtonReleasedOutsideWidget(unsigned int button)
{
	(void)button;
	beingClicked = false;
	return true;
}

}

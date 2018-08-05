#include <UI.h>

#include <iostream>
using namespace std;

namespace UI {

MenuItem::MenuItem(std::string text_, UI::Menu * menu_) : Button(text_), menu(menu_) 
{
}

bool MenuItem::onMouseButtonReleased(unsigned int button)
{
	// TODO: Shift menu left if it is going off the right edge of the window

	Button::onMouseButtonReleased(button);

	// Menu must be baked to get its height. It will be rebaked in set_menu_overlay_root_container .
	menu->bake();

	// Start by tying to fit the menu below the widget

	unsigned int windowWidth, windowHeight;
	get_window_dimensions(windowWidth, windowHeight);

	// Window coordinates and size of menu button widget that has been clicked
	unsigned int miX, miY, miWidth, miHeight;
	getArea(miX, miY, miWidth, miHeight);

	unsigned int bottomOfMenu = miY + miHeight + menu->getActualHeight();

	if(bottomOfMenu <= windowHeight) {
		// Menu fits below the widget
		set_menu_overlay_root_container(menu, miX, miY + miHeight);
		return true;
	}
	else {
		// Try putting the menu above the button widget

		int topOfMenu = (int)miY - (int)menu->getActualHeight();

		set_menu_overlay_root_container(menu, miX, topOfMenu);
		return true;
	}
}

Menu::Menu(std::vector<Widget *> widgets) : Container(widgets, 0,0,0,0,0xff505050, UI::Container::FLOW_DOWN) {}

// SubMenu::SubMenu(std::vector<Widget *> widgets) : Menu(widgets) {}

}
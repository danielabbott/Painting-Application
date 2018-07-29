#include <UI.h>

namespace UI {

MenuItem::MenuItem(std::string text_, UI::Menu * menu_) : Button(text_), menu(menu_) 
{
	menu->bake();
}

bool MenuItem::onMouseButtonReleased(unsigned int button)
{
	bool redraw = Button::onMouseButtonReleased(button);

	menu->bake();
	set_menu_overlay_root_container(menu);
	
	return redraw;
}

Menu::Menu(std::vector<Widget *> widgets) : Container(widgets, 0,0,100,200,0xffffffff, UI::Container::NONE) {}

SubMenu::SubMenu(std::vector<Widget *> widgets) : Menu(widgets) {}

}